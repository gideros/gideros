/*
** Copyright (C) 2004-2016 Mike Pall. All rights reserved.
**
** Permission is hereby granted, free of charge, to any person obtaining
** a copy of this software and associated documentation files (the
** "Software"), to deal in the Software without restriction, including
** without limitation the rights to use, copy, modify, merge, publish,
** distribute, sublicense, and/or sell copies of the Software, and to
** permit persons to whom the Software is furnished to do so, subject to
** the following conditions:
**
** The above copyright notice and this permission notice shall be
** included in all copies or substantial portions of the Software.
**
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
** EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
** MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
** IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
** CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
** TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
** SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**
** [ MIT license: http://www.opensource.org/licenses/mit-license.php ]
*/

/* Coco -- True C coroutines for Lua. http://luajit.org/coco.html */
#ifndef COCO_DISABLE

#define lcoco_c
#define LUA_CORE

#include "lua.h"

#include "lobject.h"
#include "lstate.h"
#include "ldo.h"
#include "lvm.h"
#include "lgc.h"


/*
** Define this if you want to run Coco with valgrind. You will get random
** errors about accessing memory from newly allocated C stacks if you don't.
** You need at least valgrind 3.0 for this to work.
**
** This macro evaluates to a no-op if not run with valgrind. I.e. you can
** use the same binary for regular runs, too (without a performance loss).
*/
#ifdef USE_VALGRIND
#include <valgrind/valgrind.h>
#define STACK_REG(coco, p, sz)	(coco)->vgid = VALGRIND_STACK_REGISTER(p, p+sz);
#define STACK_DEREG(coco)	VALGRIND_STACK_DEREGISTER((coco)->vgid);
#define STACK_VGID		unsigned int vgid;
#else
#define STACK_REG(coco, p, sz)
#define STACK_DEREG(id)
#define STACK_VGID
#endif

/* ------------------------------------------------------------------------ */

/* Use Windows Fibers. */
#if defined(COCO_USE_FIBERS)

#define _WIN32_WINNT 0x0400
#include <windows.h>

#define COCO_MAIN_DECL		CALLBACK

typedef LPFIBER_START_ROUTINE coco_MainFunc;

#define COCO_NEW(OL, NL, cstacksize, mainfunc) \
  if ((L2COCO(NL)->fib = CreateFiber(cstacksize, mainfunc, NL)) == NULL) \
    luaD_throw(OL, LUA_ERRMEM);

#define COCO_FREE(L) \
  DeleteFiber(L2COCO(L)->fib); \
  L2COCO(L)->fib = NULL;

/* See: http://blogs.msdn.com/oldnewthing/archive/2004/12/31/344799.aspx */
#define COCO_JUMPIN(coco) \
  { void *cur = GetCurrentFiber(); \
    coco->back = (cur == NULL || cur == (void *)0x1e00) ? \
      ConvertThreadToFiber(NULL) : cur; } \
  SwitchToFiber(coco->fib);

#define COCO_JUMPOUT(coco) \
  SwitchToFiber(coco->back);

/* CreateFiber() defaults to STACKSIZE from the Windows module .def file. */
#define COCO_DEFAULT_CSTACKSIZE		0

/* ------------------------------------------------------------------------ */

#else /* !COCO_USE_FIBERS */

#ifndef COCO_USE_UCONTEXT

/* Try inline asm first. */
#if __GNUC__ >= 3 && !defined(COCO_USE_SETJMP)

#if defined(__i386) || defined(__i386__)

#ifdef __PIC__
typedef void *coco_ctx[4];  /* eip, esp, ebp, ebx */
static inline void coco_switch(coco_ctx from, coco_ctx to)
{
  __asm__ __volatile__ (
    "call 1f\n" "1:\tpopl %%eax\n\t" "addl $(2f-1b),%%eax\n\t"
    "movl %%eax, (%0)\n\t" "movl %%esp, 4(%0)\n\t"
    "movl %%ebp, 8(%0)\n\t" "movl %%ebx, 12(%0)\n\t"
    "movl 12(%1), %%ebx\n\t" "movl 8(%1), %%ebp\n\t"
    "movl 4(%1), %%esp\n\t" "jmp *(%1)\n" "2:\n"
    : "+S" (from), "+D" (to) : : "eax", "ecx", "edx", "memory", "cc");
}
#else
typedef void *coco_ctx[3];  /* eip, esp, ebp */
static inline void coco_switch(coco_ctx from, coco_ctx to)
{
  __asm__ __volatile__ (
    "movl $1f, (%0)\n\t" "movl %%esp, 4(%0)\n\t" "movl %%ebp, 8(%0)\n\t"
    "movl 8(%1), %%ebp\n\t" "movl 4(%1), %%esp\n\t" "jmp *(%1)\n" "1:\n"
    : "+S" (from), "+D" (to) : : "eax", "ebx", "ecx", "edx", "memory", "cc");
}
#endif

#define COCO_CTX		coco_ctx
#define COCO_SWITCH(from, to)	coco_switch(from, to);
#define COCO_MAKECTX(coco, buf, func, stack, a0) \
  buf[0] = (void *)(func); \
  buf[1] = (void *)(stack); \
  buf[2] = (void *)0; \
  stack[0] = 0xdeadc0c0;  /* Dummy return address. */ \
  coco->arg0 = (size_t)(a0);
#define COCO_STATE_HEAD		size_t arg0;

#elif defined(__x86_64__)

static void coco_wrap_main(void)
{
  __asm__ __volatile__ ("\tmovq %r13, %rdi\n\tjmpq *%r12\n");
}

typedef void *coco_ctx[8];  /* rip, rsp, rbp, rbx, r12, r13, r14, r15 */
static inline void coco_switch(coco_ctx from, coco_ctx to)
{
  __asm__ __volatile__ (
    "leaq 1f(%%rip), %%rax\n\t"
    "movq %%rax, (%0)\n\t" "movq %%rsp, 8(%0)\n\t" "movq %%rbp, 16(%0)\n\t"
    "movq %%rbx, 24(%0)\n\t" "movq %%r12, 32(%0)\n\t" "movq %%r13, 40(%0)\n\t"
    "movq %%r14, 48(%0)\n\t" "movq %%r15, 56(%0)\n\t"
    "movq 56(%1), %%r15\n\t" "movq 48(%1), %%r14\n\t" "movq 40(%1), %%r13\n\t"
    "movq 32(%1), %%r12\n\t" "movq 24(%1), %%rbx\n\t" "movq 16(%1), %%rbp\n\t"
    "movq 8(%1), %%rsp\n\t" "jmpq *(%1)\n" "1:\n"
    : "+S" (from), "+D" (to) :
    : "rax", "rcx", "rdx", "r8", "r9", "r10", "r11", "memory", "cc");
}

#define COCO_CTX		coco_ctx
#define COCO_SWITCH(from, to)	coco_switch(from, to);
#define COCO_MAKECTX(coco, buf, func, stack, a0) \
  buf[0] = (void *)(coco_wrap_main); \
  buf[1] = (void *)(stack); \
  buf[2] = (void *)0; \
  buf[3] = (void *)0; \
  buf[4] = (void *)(func); \
  buf[5] = (void *)(a0); \
  buf[6] = (void *)0; \
  buf[7] = (void *)0; \
  stack[0] = 0xdeadc0c0deadc0c0;  /* Dummy return address. */ \

#elif __mips && !defined(__mips_eabi) && \
      ((defined(_ABIO32) && _MIPS_SIM == _ABIO32) || \
       (defined(_MIPS_SIM_ABI32) && _MIPS_SIM == _MIPS_SIM_ABI32))

/* No way to avoid the function prologue with inline assembler. So use this: */
static const unsigned int coco_switch[] = {
#ifdef __mips_soft_float
#define COCO_STACKSAVE		-10
  0x27bdffd8,  /* addiu sp, sp, -(10*4) */
#else
#define COCO_STACKSAVE		-22
  0x27bdffa8,  /* addiu sp, sp, -(10*4+6*8) */
  /* sdc1 {$f20-$f30}, offset(sp) */
  0xf7be0050, 0xf7bc0048, 0xf7ba0040, 0xf7b80038, 0xf7b60030, 0xf7b40028,
#endif
  /* sw {gp,s0-s8}, offset(sp) */
  0xafbe0024, 0xafb70020, 0xafb6001c, 0xafb50018, 0xafb40014, 0xafb30010,
  0xafb2000c, 0xafb10008, 0xafb00004, 0xafbc0000,
  /* sw sp, 4(a0); sw ra, 0(a0); lw ra, 0(a1); lw sp, 4(a1); move t9, ra */
  0xac9d0004, 0xac9f0000, 0x8cbf0000, 0x8cbd0004, 0x03e0c821,
  /* lw caller-saved-reg, offset(sp) */
  0x8fbe0024, 0x8fb70020, 0x8fb6001c, 0x8fb50018, 0x8fb40014, 0x8fb30010,
  0x8fb2000c, 0x8fb10008, 0x8fb00004, 0x8fbc0000,
#ifdef __mips_soft_float
  0x03e00008, 0x27bd0028  /* jr ra; addiu sp, sp, 10*4 */
#else
  /* ldc1 {$f20-$f30}, offset(sp) */
  0xd7be0050, 0xd7bc0048, 0xd7ba0040, 0xd7b80038, 0xd7b60030, 0xd7b40028,
  0x03e00008, 0x27bd0058  /* jr ra; addiu sp, sp, 10*4+6*8 */
#endif
};

typedef void *coco_ctx[2];  /* ra, sp */
#define COCO_CTX		coco_ctx
#define COCO_SWITCH(from, to) \
  ((void (*)(coco_ctx, coco_ctx))coco_switch)(from, to);
#define COCO_MAKECTX(coco, buf, func, stack, a0) \
  buf[0] = (void *)(func); \
  buf[1] = (void *)&stack[COCO_STACKSAVE]; \
  stack[4] = (size_t)(a0);  /* Assumes o32 ABI. */
#define COCO_STACKADJUST	8
#define COCO_MAIN_PARAM		int _a, int _b, int _c, int _d, lua_State *L

#elif defined(__sparc__)

typedef void *coco_ctx[4];
#define COCO_CTX		coco_ctx
#define COCO_SWITCH(from, to)	coco_switch(from, to);
#define COCO_STACKADJUST	24

#if defined(__LP64__)
#define COCO_STACKBIAS		(2047UL)
#define COCO_PTR2SP(stack)	(((unsigned long)stack)-COCO_STACKBIAS)
static inline void coco_switch(coco_ctx from, coco_ctx to)
{
  void *__stack[16] __attribute__((aligned (16)));
  unsigned long __tmp_sp = COCO_PTR2SP(__stack);
  __asm__ __volatile__
    (/* Flush register window(s) to stack and save the previous stack
	pointer to capture the current registers, %l0-%l7 and %i0-%i7. */
     "ta 3\n\t"
     "stx %%sp,[%0+8]\n\t"
     /* Move to a temporary stack. If the register window is flushed
	for some reason (e.g. context switch), not the next stack
	but the temporary stack should be used so as not to break
	neither the previous nor next stack */
     "mov %2,%%sp\n\t"
     "sethi %%hh(1f),%%g1\n\t"		/* i.e. setx 1f,%%g1 */
     "or %%g1,%%hm(1f),%%g1\n\t"
     "sethi %%lm(1f),%%g2\n\t"
     "or %%g2,%%lo(1f),%%g2\n\t"
     "sllx %%g1,32,%%g1\n\t"
     "or %%g1,%%g2,%%g1\n\t"
     "stx %%g1,[%0]\n\t"
     /* Restore registers from stack. DO NOT load the next stack
	pointer directly to %sp. The register window can be possibly
	flushed and restored asynchronous (e.g. context switch). */
     "mov %1,%%o1\n\t"
     "ldx [%%o1+8],%%o2\n\t"
     "ldx [%%o2+%3],%%l0\n\t"
     "ldx [%%o2+%3+8],%%l1\n\t"
     "ldx [%%o2+%3+0x10],%%l2\n\t"
     "ldx [%%o2+%3+0x18],%%l3\n\t"
     "ldx [%%o2+%3+0x20],%%l4\n\t"
     "ldx [%%o2+%3+0x28],%%l5\n\t"
     "ldx [%%o2+%3+0x30],%%l6\n\t"
     "ldx [%%o2+%3+0x38],%%l7\n\t"
     "ldx [%%o2+%3+0x40],%%i0\n\t"
     "ldx [%%o2+%3+0x48],%%i1\n\t"
     "ldx [%%o2+%3+0x50],%%i2\n\t"
     "ldx [%%o2+%3+0x58],%%i3\n\t"
     "ldx [%%o2+%3+0x60],%%i4\n\t"
     "ldx [%%o2+%3+0x68],%%i5\n\t"
     "ldx [%%o2+%3+0x70],%%i6\n\t"
     "ldx [%%o2+%3+0x78],%%i7\n\t"
     /* Move to the next stack with the consistent registers atomically */
     "mov %%o2,%%sp\n\t"
     "ldx [%%o1],%%o2\n\t"
     /* Since %o0-%o7 are marked as clobbered, values are safely overwritten
	across the inline assembly.  %o0-%o7 will have meaningless values
	after leaving the inline assembly. The only exception is %o0, which
	serves as an argument to coco_main */
     "ldx [%%o1+16],%%o0\n\t"
     "jmpl %%o2,%%g0\n\t"
     "nop\n\t"
     "1:\n"
     /* An assumption is made here; no input operand is assigned to %g1
	nor %g2. It's the case for the currently avilable gcc's */
     : : "r"(from),"r"(to),"r"(__tmp_sp),"i"(COCO_STACKBIAS)
     : "g1","g2","o0","o1","o2","o3","o4","o5","o7","memory","cc");
}

#define COCO_MAKECTX(coco, buf, func, stack, a0) \
  buf[0] = (void *)(func); \
  buf[1] = (void *)COCO_PTR2SP(&(stack)[0]); \
  buf[2] = (void *)(a0); \
  stack[0] = 0; \
  stack[1] = 0; \
  stack[2] = 0; \
  stack[3] = 0; \
  stack[4] = 0; \
  stack[5] = 0; \
  stack[6] = 0; \
  stack[7] = 0; \
  stack[8] = 0; \
  stack[9] = 0; \
  stack[10] = 0; \
  stack[11] = 0; \
  stack[12] = 0; \
  stack[13] = 0; \
  stack[14] = COCO_PTR2SP(&(stack)[COCO_STACKADJUST]); \
  stack[15] = 0xdeadc0c0deadc0c0; /* Dummy return address. */ \

#else
static inline void coco_switch(coco_ctx from, coco_ctx to)
{
  void *__tmp_stack[16] __attribute__((aligned (16)));
  __asm__ __volatile__ 
    ("ta 3\n\t"
     "st %%sp,[%0+4]\n\t"
     "mov %2,%%sp\n\t"
     "set 1f,%%g1\n\t"
     "st %%g1,[%0]\n\t"
     "mov %1,%%o1\n\t"
     "ld [%%o1+4],%%o2\n\t"
     "ldd [%%o2],%%l0\n\t"
     "ldd [%%o2+8],%%l2\n\t"
     "ldd [%%o2+0x10],%%l4\n\t"
     "ldd [%%o2+0x18],%%l6\n\t"
     "ldd [%%o2+0x20],%%i0\n\t"
     "ldd [%%o2+0x28],%%i2\n\t"
     "ldd [%%o2+0x30],%%i4\n\t"
     "ldd [%%o2+0x38],%%i6\n\t"
     "mov %%o2,%%sp\n\t"
     "ld [%%o1],%%o2\n\t"
     "ld [%%o1+8],%%o0\n\t"
     "jmpl %%o2,%%g0\n\t"
     "nop\n\t"
     "1:\n"
     : : "r"(from),"r"(to),"r"(__tmp_stack)
     : "g1","o0","o1","o2","o3","o4","o5","o7","memory","cc");
}

#define COCO_MAKECTX(coco, buf, func, stack, a0) \
  buf[0] = (void *)(func); \
  buf[1] = (void *)(stack); \
  buf[2] = (void *)(a0); \
  stack[0] = 0; \
  stack[1] = 0; \
  stack[2] = 0; \
  stack[3] = 0; \
  stack[4] = 0; \
  stack[5] = 0; \
  stack[6] = 0; \
  stack[7] = 0; \
  stack[8] = 0; \
  stack[9] = 0; \
  stack[10] = 0; \
  stack[11] = 0; \
  stack[12] = 0; \
  stack[13] = 0; \
  stack[14] = (size_t)&stack[COCO_STACKADJUST]; \
  stack[15] = 0xdeadc0c0; /* Dummy return address. */ \

#endif /* !define(__LP64__) */

#elif defined(__ARM_EABI__)

#if __SOFTFP__
#define COCO_FLOAT_SAVE		0
#else
#define COCO_FLOAT_SAVE		16
#endif

/* [d8-d15,] r4-r11, lr, sp */
typedef void *coco_ctx[COCO_FLOAT_SAVE + 10];

void coco_wrap_main(void);
int coco_switch(coco_ctx from, coco_ctx to);

__asm__(
  ".text\n"
  ".globl coco_switch\n"
  ".type coco_switch #function\n"
  ".hidden coco_switch\n"
  "coco_switch:\n"
#if COCO_FLOAT_SAVE
  "  vstmia r0!, {d8-d15}\n"
#endif
  "  stmia r0, {r4-r11, lr}\n"
  "  str sp, [r0, #9*4]\n"
#if COCO_FLOAT_SAVE
  "  vldmia r1!, {d8-d15}\n"
#endif
  "  ldr sp, [r1, #9*4]\n"
  "  ldmia r1, {r4-r11, pc}\n"
  ".size coco_switch, .-coco_switch\n"
);

__asm__(
  ".text\n"
  ".globl coco_wrap_main\n"
  ".type coco_wrap_main #function\n"
  ".hidden coco_wrap_main\n"
  "coco_wrap_main:\n"
  "  mov r0, r4\n"
  "  mov ip, r5\n"
  "  mov lr, r6\n"
  "  bx ip\n"
  ".size coco_wrap_main, .-coco_wrap_main\n"
);

#define COCO_CTX               coco_ctx
#define COCO_SWITCH(from, to)  coco_switch(from, to);
#define COCO_MAKECTX(coco, buf, func, stack, a0) \
  buf[COCO_FLOAT_SAVE+0] = (void*)(a0); \
  buf[COCO_FLOAT_SAVE+1] = (void*)(func); \
  buf[COCO_FLOAT_SAVE+2] = (void*)(0xdeadc0c0); /* Dummy return address. */ \
  buf[COCO_FLOAT_SAVE+8] = (void*)(coco_wrap_main); \
  buf[COCO_FLOAT_SAVE+9] = (void*)(stack);

#elif defined(__aarch64__)

/* x19-x30, sp, lr, d8-d15 */
typedef void *coco_ctx[14 + 8];

void coco_wrap_main(void);
int coco_switch(coco_ctx from, coco_ctx to);

__asm__(
  ".text\n"
  ".globl coco_switch\n"
  ".type coco_switch #function\n"
  ".hidden coco_switch\n"
  "coco_switch:\n"
  "  mov x10, sp\n"
  "  mov x11, x30\n"
  "  stp x19, x20, [x0, #(0*16)]\n"
  "  stp x21, x22, [x0, #(1*16)]\n"
  "  stp d8, d9, [x0, #(7*16)]\n"
  "  stp x23, x24, [x0, #(2*16)]\n"
  "  stp d10, d11, [x0, #(8*16)]\n"
  "  stp x25, x26, [x0, #(3*16)]\n"
  "  stp d12, d13, [x0, #(9*16)]\n"
  "  stp x27, x28, [x0, #(4*16)]\n"
  "  stp d14, d15, [x0, #(10*16)]\n"
  "  stp x29, x30, [x0, #(5*16)]\n"
  "  stp x10, x11, [x0, #(6*16)]\n"
  "  ldp x19, x20, [x1, #(0*16)]\n"
  "  ldp x21, x22, [x1, #(1*16)]\n"
  "  ldp d8, d9, [x1, #(7*16)]\n"
  "  ldp x23, x24, [x1, #(2*16)]\n"
  "  ldp d10, d11, [x1, #(8*16)]\n"
  "  ldp x25, x26, [x1, #(3*16)]\n"
  "  ldp d12, d13, [x1, #(9*16)]\n"
  "  ldp x27, x28, [x1, #(4*16)]\n"
  "  ldp d14, d15, [x1, #(10*16)]\n"
  "  ldp x29, x30, [x1, #(5*16)]\n"
  "  ldp x10, x11, [x1, #(6*16)]\n"
  "  mov sp, x10\n"
  "  br x11\n"
  ".size coco_switch, .-coco_switch\n"
);

__asm__(
  ".text\n"
  ".globl coco_wrap_main\n"
  ".type coco_wrap_main #function\n"
  ".hidden coco_wrap_main\n"
  "coco_wrap_main:\n"
  "  mov x0, x19\n"
  "  mov x30, x21\n"
  "  br x20\n"
  ".size coco_wrap_main, .-coco_wrap_main\n"
);

#define COCO_CTX               coco_ctx
#define COCO_SWITCH(from, to)  coco_switch(from, to);
#define COCO_MAKECTX(coco, buf, func, stack, a0) \
  buf[0] = (void*)(a0); \
  buf[1] = (void*)(func); \
  buf[2] = (void*)(0xdeadc0c0deadc0c0); /* Dummy return address. */ \
  buf[12] = (void*)((size_t)stack & ~15); \
  buf[13] = (void*)(coco_wrap_main);

#endif /* arch check */

#endif /* !(__GNUC__ >= 3 && !defined(COCO_USE_SETJMP)) */

/* Try _setjmp/_longjmp with a patched jump buffer. */
#ifndef COCO_MAKECTX
#include <setjmp.h>

/* Check for supported CPU+OS combinations. */
#if defined(__i386) || defined(__i386__)

#define COCO_STATE_HEAD		size_t arg0;
#define COCO_SETJMP_X86(coco, stack, a0) \
  stack[COCO_STACKADJUST-1] = 0xdeadc0c0;  /* Dummy return address. */ \
  coco->arg0 = (size_t)(a0);

#if __GLIBC__ == 2 && defined(JB_SP)		/* x86-linux-glibc2 */
#define COCO_PATCHCTX(coco, buf, func, stack, a0) \
  buf->__jmpbuf[JB_PC] = (int)(func); \
  buf->__jmpbuf[JB_SP] = (int)(stack); \
  buf->__jmpbuf[JB_BP] = 0; \
  COCO_SETJMP_X86(coco, stack, a0)
#elif defined(__linux__) && defined(_I386_JMP_BUF_H)	/* x86-linux-libc5 */
#define COCO_PATCHCTX(coco, buf, func, stack, a0) \
  buf->__pc = (func); \
  buf->__sp = (stack); \
  buf->__bp = NULL; \
  COCO_SETJMP_X86(coco, stack, a0)
#elif defined(__FreeBSD__)			/* x86-FreeBSD */
#define COCO_PATCHCTX(coco, buf, func, stack, a0) \
  buf->_jb[0] = (long)(func); \
  buf->_jb[2] = (long)(stack); \
  buf->_jb[3] = 0; /* ebp */ \
  COCO_SETJMP_X86(coco, stack, a0)
#define COCO_STACKADJUST	2
#elif defined(__NetBSD__) || defined(__OpenBSD__) /* x86-NetBSD, x86-OpenBSD */
#define COCO_PATCHCTX(coco, buf, func, stack, a0) \
  buf[0] = (long)(func); \
  buf[2] = (long)(stack); \
  buf[3] = 0; /* ebp */ \
  COCO_SETJMP_X86(coco, stack, a0)
#define COCO_STACKADJUST	2
#elif defined(__solaris__) && _JBLEN == 10	/* x86-solaris */
#define COCO_PATCHCTX(coco, buf, func, stack, a0) \
  buf[5] = (int)(func); \
  buf[4] = (int)(stack); \
  buf[3] = 0; \
  COCO_SETJMP_X86(coco, stack, a0)
#elif defined(__MACH__) && defined(_BSD_I386_SETJMP_H)	/* x86-macosx */
#define COCO_PATCHCTX(coco, buf, func, stack, a0) \
  buf[12] = (int)(func); \
  buf[9] = (int)(stack); \
  buf[8] = 0; /* ebp */ \
  COCO_SETJMP_X86(coco, stack, a0)
#endif

#elif defined(__x86_64__) || defined(__x86_64)

#define COCO_STATE_HEAD		size_t arg0;

#define COCO_MAIN_PARAM \
  int _a, int _b, int _c, int _d, int _e, int _f, lua_State *L

#if defined(__solaris__) && _JBLEN == 8			/* x64-solaris */
#define COCO_PATCHCTX(coco, buf, func, stack, a0) \
  buf[7] = (long)(func); \
  buf[6] = (long)(stack); \
  buf[5] = 0; \
  stack[0] = 0xdeadc0c0;  /* Dummy return address. */ \
  coco->arg0 = (size_t)(a0);
#endif

#elif defined(PPC) || defined(__ppc__) || defined(__PPC__) || \
      defined(__powerpc__) || defined(__POWERPC__) || defined(_ARCH_PPC)

#define COCO_STACKADJUST	16
#define COCO_MAIN_PARAM \
  int _a, int _b, int _c, int _d, int _e, int _f, int _g, int _h, lua_State *L

#if defined(__MACH__) && defined(_BSD_PPC_SETJMP_H_)	/* ppc32-macosx */
#define COCO_PATCHCTX(coco, buf, func, stack, a0) \
  buf[21] = (int)(func); \
  buf[0] = (int)(stack); \
  stack[6+8] = (size_t)(a0);
#endif

#elif (defined(MIPS) || defined(MIPSEL) || defined(__mips)) && \
  _MIPS_SIM == _MIPS_SIM_ABI32 && !defined(__mips_eabi)

/* Stack layout for o32 ABI. */
#define COCO_STACKADJUST	8
#define COCO_MAIN_PARAM		int _a, int _b, int _c, int _d, lua_State *L

#if __GLIBC__ == 2 || defined(__UCLIBC__)	/* mips32-linux-glibc2 */
#define COCO_PATCHCTX(coco, buf, func, stack, a0) \
  buf->__jmpbuf->__pc = (func); /* = t9 in _longjmp. Reqd. for -mabicalls. */ \
  buf->__jmpbuf->__sp = (stack); \
  buf->__jmpbuf->__fp = (void *)0; \
  stack[4] = (size_t)(a0);
#endif

#elif defined(__arm__) || defined(__ARM__)

#if __GLIBC__ == 2 || defined(__UCLIBC__)	/* arm-linux-glibc2 */
#ifndef __JMP_BUF_SP
#define __JMP_BUF_SP	((sizeof(__jmp_buf)/sizeof(int))-2)
#endif
#define COCO_PATCHCTX(coco, buf, func, stack, a0) \
  buf->__jmpbuf[__JMP_BUF_SP+1] = (int)(func); /* pc */ \
  buf->__jmpbuf[__JMP_BUF_SP] = (int)(stack); /* sp */ \
  buf->__jmpbuf[__JMP_BUF_SP-1] = 0; /* fp */ \
  stack[0] = (size_t)(a0);
#define COCO_STACKADJUST	2
#define COCO_MAIN_PARAM		int _a, int _b, int _c, int _d, lua_State *L
#elif defined(__APPLE__)	/* arm-ios */
#define __JMP_BUF_SP  7   /* r4 r5 r6 r7 r8 r10 fp sp lr sig ... */
#define COCO_PATCHCTX(coco, buf, func, stack, a0) \
  buf[__JMP_BUF_SP+1] = (int)(func); /* lr */ \
  buf[__JMP_BUF_SP] = (int)(stack); /* sp */ \
  buf[__JMP_BUF_SP-1] = 0; /* fp */ \
  stack[0] = (size_t)(a0);
#define COCO_STACKADJUST 2
#define COCO_MAIN_PARAM int _a, int _b, int _c, int _d, lua_State *L
#endif

#endif /* arch check */

#ifdef COCO_PATCHCTX
#define COCO_CTX		jmp_buf
#define COCO_MAKECTX(coco, buf, func, stack, a0) \
  _setjmp(buf); COCO_PATCHCTX(coco, buf, func, stack, a0)
#define COCO_SWITCH(from, to)	if (!_setjmp(from)) _longjmp(to, 1);
#endif

#endif /* !defined(COCO_MAKECTX) */

#endif /* !defined(COCO_USE_UCONTEXT) */

/* ------------------------------------------------------------------------ */

/* Use inline asm or _setjmp/_longjmp if available. */
#ifdef COCO_MAKECTX

#ifndef COCO_STACKADJUST
#define COCO_STACKADJUST	1
#endif

#define COCO_FILL(coco, NL, mainfunc) \
{ /* Include the return address to get proper stack alignment. */ \
  size_t *stackptr = &((size_t *)coco)[-COCO_STACKADJUST]; \
  COCO_MAKECTX(coco, coco->ctx, mainfunc, stackptr, NL) \
}

/* ------------------------------------------------------------------------ */

/* Else fallback to ucontext. Slower, because it saves/restores signals. */
#else /* !defined(COCO_MAKECTX) */

#include <ucontext.h>

#define COCO_CTX		ucontext_t

/* Ugly workaround for makecontext() deficiencies on 64 bit CPUs. */
/* Note that WIN64 (which is LLP64) never comes here. See above. */
#if defined(__LP64__) || defined(_LP64) || INT_MAX != LONG_MAX
/* 64 bit CPU: split the pointer into two 32 bit ints. */
#define COCO_MAIN_PARAM		unsigned int lo, unsigned int hi
#define COCO_MAIN_GETL \
  lua_State *L = (lua_State *)((((unsigned long)hi)<<32)+(unsigned long)lo);
#define COCO_MAKECTX(coco, NL, mainfunc) \
  makecontext(&coco->ctx, mainfunc, 2, \
    (int)(ptrdiff_t)NL, (int)((ptrdiff_t)NL>>32));
#else
/* 32 bit CPU: a pointer fits into an int. */
#define COCO_MAKECTX(coco, NL, mainfunc) \
  makecontext(&coco->ctx, mainfunc, 1, (int)NL);
#endif

#define COCO_FILL(coco, NL, mainfunc) \
  getcontext(&coco->ctx); \
  coco->ctx.uc_link = NULL;  /* We never exit from coco_main. */ \
  coco->ctx.uc_stack.ss_sp = coco->allocptr; \
  coco->ctx.uc_stack.ss_size = (char *)coco - (char *)(coco->allocptr); \
  COCO_MAKECTX(coco, NL, mainfunc)

#define COCO_SWITCH(from, to)	swapcontext(&(from), &(to));

#endif /* !defined(COCO_MAKECTX) */


/* Common code for inline asm/setjmp/ucontext to allocate/free the stack. */

struct coco_State {
#ifdef COCO_STATE_HEAD
  COCO_STATE_HEAD
#endif
  COCO_CTX ctx;			/* Own context. */
  COCO_CTX back;		/* Context to switch back to. */
  void *allocptr;		/* Pointer to allocated memory. */
  int allocsize;		/* Size of allocated memory. */
  int nargs;			/* Number of arguments to pass. */
  STACK_VGID			/* Optional valgrind stack id. See above. */
};

typedef void (*coco_MainFunc)(void);

/* Put the Coco state at the end and align it downwards. */
#define ALIGNED_END(p, s, t) \
  ((t *)(((char *)0) + ((((char *)(p)-(char *)0)+(s)-sizeof(t)) & -16)))

/* TODO: use mmap. */
#define COCO_NEW(OL, NL, cstacksize, mainfunc) \
{ \
  void *ptr = luaM_malloc(OL, cstacksize); \
  coco_State *coco = ALIGNED_END(ptr, cstacksize, coco_State); \
  STACK_REG(coco, ptr, cstacksize) \
  coco->allocptr = ptr; \
  coco->allocsize = cstacksize; \
  COCO_FILL(coco, NL, mainfunc) \
  L2COCO(NL) = coco; \
}

#define COCO_FREE(L) \
  STACK_DEREG(L2COCO(L)) \
  luaM_freemem(L, L2COCO(L)->allocptr, L2COCO(L)->allocsize); \
  L2COCO(L) = NULL;

#define COCO_JUMPIN(coco)	COCO_SWITCH(coco->back, coco->ctx)
#define COCO_JUMPOUT(coco)	COCO_SWITCH(coco->ctx, coco->back)

#endif /* !COCO_USE_FIBERS */

/* ------------------------------------------------------------------------ */

#ifndef COCO_MIN_CSTACKSIZE
#define COCO_MIN_CSTACKSIZE		(32768+4096)
#endif

/* Don't use multiples of 64K to avoid D-cache aliasing conflicts. */
#ifndef COCO_DEFAULT_CSTACKSIZE
#define COCO_DEFAULT_CSTACKSIZE		(65536-4096)
#endif

static int defaultcstacksize = COCO_DEFAULT_CSTACKSIZE;

/* Start the Lua or C function. */
static void coco_start(lua_State *L, void *ud)
{
  if (luaD_precall(L, (StkId)ud, LUA_MULTRET) == PCRLUA)
    luaV_execute(L, L->ci - L->base_ci);
}

#ifndef COCO_MAIN_PARAM
#define COCO_MAIN_PARAM		lua_State *L
#endif

#ifndef COCO_MAIN_DECL
#define COCO_MAIN_DECL
#endif

/* Toplevel function for the new coroutine stack. Never exits. */
static void COCO_MAIN_DECL coco_main(COCO_MAIN_PARAM)
{
#ifdef COCO_MAIN_GETL
  COCO_MAIN_GETL
#endif
  coco_State *coco = L2COCO(L);
  for (;;) {
    L->status = luaD_rawrunprotected(L, coco_start, L->top - (coco->nargs+1));
    if (L->status != 0) luaD_seterrorobj(L, L->status, L->top);
    COCO_JUMPOUT(coco)
  }
}

/* Add a C stack to a coroutine. */
lua_State *lua_newcthread(lua_State *OL, int cstacksize)
{
  lua_State *NL = lua_newthread(OL);

  if (cstacksize < 0)
    return NL;
  if (cstacksize == 0)
    cstacksize = defaultcstacksize;
  else if (cstacksize < COCO_MIN_CSTACKSIZE)
    cstacksize = COCO_MIN_CSTACKSIZE;
  cstacksize &= -16;

  COCO_NEW(OL, NL, cstacksize, ((coco_MainFunc)(coco_main)))

  return NL;
}

/* Free the C stack of a coroutine. Called from lstate.c. */
void luaCOCO_free(lua_State *L)
{
  COCO_FREE(L)
}

/* Resume a coroutine with a C stack. Called from ldo.c. */
int luaCOCO_resume(lua_State *L, int nargs)
{
  coco_State *coco = L2COCO(L);
  coco->nargs = nargs;
  COCO_JUMPIN(coco)
#ifndef COCO_DISABLE_EARLY_FREE
  if (L->status != LUA_YIELD) {
    COCO_FREE(L)
  }
#endif
  return L->status;
}

/* Yield from a coroutine with a C stack. Called from ldo.c. */
int luaCOCO_yield(lua_State *L)
{
  coco_State *coco = L2COCO(L);
  L->status = LUA_YIELD;
  COCO_JUMPOUT(coco)
  L->status = 0;
  {
    StkId base = L->top - coco->nargs;
    StkId rbase = L->base;
    if (rbase < base) {  /* Need to move args down? */
      while (base < L->top)
	setobjs2s(L, rbase++, base++);
      L->top = rbase;
    }
  }
  L->base = L->ci->base;  /* Restore invariant. */
  return coco->nargs;
}

/* Get/set the default C stack size. */
int luaCOCO_cstacksize(int cstacksize)
{
  int oldsz = defaultcstacksize;
  if (cstacksize >= 0) {
    if (cstacksize == 0)
      cstacksize = COCO_DEFAULT_CSTACKSIZE;
    else if (cstacksize < COCO_MIN_CSTACKSIZE)
      cstacksize = COCO_MIN_CSTACKSIZE;
    defaultcstacksize = cstacksize;
  }
  return oldsz;
}

#endif
