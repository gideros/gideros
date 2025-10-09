//FOR PLUGINS link
#include <sys/mman.h>
#include <gproxy.h>
#include "emscripten.h"
#include <signal.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <utime.h>

//Pull in these for plugins
extern "C" {
	/*
EMSCRIPTEN_KEEPALIVE double llvm_exp2_f64(double);
EMSCRIPTEN_KEEPALIVE double llvm_round_f64(double);
EMSCRIPTEN_KEEPALIVE double llvm_rint_f64(double);
EMSCRIPTEN_KEEPALIVE double llvm_exp2_f32(double);
EMSCRIPTEN_KEEPALIVE double llvm_round_f32(double);
EMSCRIPTEN_KEEPALIVE double llvm_rint_f32(double);
EMSCRIPTEN_KEEPALIVE uint32_t llvm_ctpop_i32(uint32_t);
EMSCRIPTEN_KEEPALIVE int settimeofday(const struct timeval *tv, const struct timezone *tz);
*/
EMSCRIPTEN_KEEPALIVE void *__mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
EMSCRIPTEN_KEEPALIVE int __munmap(void *addr, size_t length);
EMSCRIPTEN_KEEPALIVE char *strdup(const char *inStr);
EMSCRIPTEN_KEEPALIVE sighandler_t signal(int signum, sighandler_t handler);
EMSCRIPTEN_KEEPALIVE int gettimeofday(struct timeval *tv, void *tz);
EMSCRIPTEN_KEEPALIVE int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
EMSCRIPTEN_KEEPALIVE void freeaddrinfo(struct addrinfo *res);
EMSCRIPTEN_KEEPALIVE void setTempRet0(uint32_t r);
EMSCRIPTEN_KEEPALIVE uint32_t getTempRet0();
}

//The lines below are used to force linking of some system functions used by plugins
//This code is called when there is an issue, so no problem crashing even more
class L1: public GEventDispatcherProxy {
public:
	L1() { };
	~L1() { printf("%d",0); };
};
void linkCode() __attribute__((optnone)) {
	/*
	  printf("%f",llvm_round_f64(llvm_rint_f64(llvm_exp2_f64(12.0))));
	  printf("%f",llvm_round_f32(llvm_rint_f32(llvm_exp2_f32(12.0))));
	  printf("%d",llvm_ctpop_i32(d1));
	  settimeofday(NULL,NULL);
	  */
	  intptr_t d1=(intptr_t)__mmap(NULL,4096,0,MAP_ANONYMOUS,-1,0);
	  __munmap((void*)d1,4096);
	  L1 *l1=new L1();
	  delete l1;
	  signal(1,NULL);
	  gettimeofday(NULL,NULL);
	  getaddrinfo(NULL,NULL,NULL,NULL);
	  freeaddrinfo(NULL);
	  gethostbyname(NULL);
	  gethostbyaddr(NULL,0,0);
	  getnameinfo(NULL,0,NULL,0,NULL,0,0);
	  gai_strerror(0);
	  utime(NULL,NULL);
	  utimes(NULL,NULL);
	  setTempRet0(getTempRet0());

	  std::string __t="Dummy";
	  std::string __t2(__t);
	  std::string *__t3=new std::string(__t);

	  free(strdup(__t.c_str()));
	  delete __t3;
}
