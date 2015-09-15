/*  $Id: LhA.c,v 1.16 2010/05/27 16:48:30 stoecker Exp $
    LhA file archiver client

    XAD library system for archive handling
    Copyright (C) 1998 and later by Dirk Stoecker <soft@dstoecker.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/*
    Modified for xmp by Claudio Matsuoka, 20120812
 */

#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "depacker.h"

#define LZHUFF0_METHOD          0x2D6C6830      /* -lh0- */
#define LZHUFF1_METHOD          0x2D6C6831      /* -lh1- */
#define LZHUFF2_METHOD          0x2D6C6832      /* -lh2- */
#define LZHUFF3_METHOD          0x2D6C6833      /* -lh3- */
#define LZHUFF4_METHOD          0x2D6C6834      /* -lh4- */
#define LZHUFF5_METHOD          0x2D6C6835      /* -lh5- */
#define LZHUFF6_METHOD          0x2D6C6836      /* -lh6- */
#define LZHUFF7_METHOD          0x2D6C6837      /* -lh7- */
#define LZHUFF8_METHOD          0x2D6C6838      /* -lh8- */
#define LARC_METHOD             0x2D6C7A73      /* -lzs- */
#define LARC5_METHOD            0x2D6C7A35      /* -lz5- */
#define LARC4_METHOD            0x2D6C7A34      /* -lz4- */
#define PMARC0_METHOD           0x2D706D30      /* -pm0- */
#define PMARC2_METHOD           0x2D706D32      /* -pm2- */

#undef UCHAR_MAX
#define UCHAR_MAX       ((1<<(sizeof(uint8)*8))-1)
#define MAX_DICBIT      16
#undef CHAR_BIT
#define CHAR_BIT        8
#define USHRT_BIT       16              /* (CHAR_BIT * sizeof(ushort)) */
#define MAXMATCH        256             /* not more than UCHAR_MAX + 1 */
#define NC              (UCHAR_MAX + MAXMATCH + 2 - THRESHOLD)
#define THRESHOLD       3               /* choose optimal value */
#define NPT             0x80
#define CBIT            9               /* $\lfloor \log_2 NC \rfloor + 1$ */
#define TBIT            5               /* smallest integer such that (1 << TBIT) > * NT */
#define NT              (USHRT_BIT + 3)
#define N_CHAR          (256 + 60 - THRESHOLD + 1)
#define TREESIZE_C      (N_CHAR * 2)
#define TREESIZE_P      (128 * 2)
#define TREESIZE        (TREESIZE_C + TREESIZE_P)
#define ROOT_C          0
#define ROOT_P          TREESIZE_C
#define N1              286             /* alphabet size */
#define EXTRABITS       8               /* >= log2(F-THRESHOLD+258-N1) */
#define BUFBITS         16              /* >= log2(MAXBUF) */
#define NP              (MAX_DICBIT + 1)
#define LENFIELD        4               /* bit size of length field for tree output */
#define MAGIC0          18
#define MAGIC5          19

#ifdef ENABLE_PMARC
#define PMARC2_OFFSET (0x100 - 2)
struct PMARC2_Tree {
  uint8 *leftarr;
  uint8 *rightarr;
  uint8 root;
};
#endif

struct LhADecrST {
  int32      pbit;
  int32      np;
  int32      nn;
  int32      n1;
  int32      most_p;
  int32      avail;
  uint32     n_max;
  uint16     maxmatch;
  uint16     total_p;
  uint16     blocksize;
  uint16     c_table[4096];
  uint16     pt_table[256];
  uint16     left[2 * NC - 1];
  uint16     right[2 * NC - 1];
  uint16     freq[TREESIZE];
  uint16     pt_code[NPT];
  int16      child[TREESIZE];
  int16      stock[TREESIZE];
  int16      s_node[TREESIZE / 2];
  int16      block[TREESIZE];
  int16      parent[TREESIZE];
  int16      edge[TREESIZE];
  uint8      c_len[NC];
  uint8      pt_len[NPT];
};

#ifdef ENABLE_PMARC
struct LhADecrPM {
  struct PMARC2_Tree tree1;
  struct PMARC2_Tree tree2;

  uint16     lastupdate;
  uint16     dicsiz1;
  uint8      gettree1;
  uint8      tree1left[32];
  uint8      tree1right[32];
  uint8      table1[32];

  uint8      tree2left[8];
  uint8      tree2right[8];
  uint8      table2[8];

  uint8      tree1bound;
  uint8      mindepth;

  /* Circular double-linked list. */
  uint8      prev[0x100];
  uint8      next[0x100];
  uint8      parentarr[0x100];
  uint8      lastbyte;
};
#endif

#ifdef ENABLE_LARC
struct LhADecrLZ {
  int32      matchpos;               /* LARC */
  int32      flag;                   /* LARC */
  int32      flagcnt;                /* LARC */
};
#endif

struct LhADecrData {
  int        error;
  FILE       *in;
  char       *text;
  uint16     DicBit;

  uint16     bitbuf;
  uint8      subbitbuf;
  uint8      bitcount;
  uint32     loc;
  uint32     count;
  uint32     nextcount;

  union {
    struct LhADecrST st;
#ifdef ENABLE_PMARC
    struct LhADecrPM pm;
#endif
#ifdef ENABLE_LARC
    struct LhADecrLZ lz;
#endif
  } d;
};

/* Shift bitbuf n bits left, read n bits */
static inline void fillbuf(struct LhADecrData *dat, uint8 n)
{
#if 0
  if(dat->error)
    return;
#endif

  while(n > dat->bitcount)
  {
    n -= dat->bitcount;
    dat->bitbuf = (dat->bitbuf << dat->bitcount) + (dat->subbitbuf >> (CHAR_BIT - dat->bitcount));
    dat->subbitbuf = fgetc(dat->in);

    dat->bitcount = CHAR_BIT;
  }
  dat->bitcount -= n;
  dat->bitbuf = (dat->bitbuf << n) + (dat->subbitbuf >> (CHAR_BIT - n));
  dat->subbitbuf <<= n;
}

static inline uint16 getbits(struct LhADecrData *dat, uint8 n)
{
  uint16 x;

  x = dat->bitbuf >> (2 * CHAR_BIT - n);
  fillbuf(dat, n);
  return x;
}

//#define init_getbits(a)      fillbuf((a), 2* CHAR_BIT)
/* this function can be replaced by a define! */
static void init_getbits(struct LhADecrData *dat)
{
  dat->bitbuf = 0;
  dat->subbitbuf = 0;
  dat->bitcount = 0;
  fillbuf(dat, 2 * CHAR_BIT);
}


/* ------------------------------------------------------------------------ */

static int make_table(struct LhADecrData *dat, int16 nchar, uint8 bitlen[], int16 tablebits, uint16 table[], int table_size)
{
  uint16 count[17];  /* count of bitlen */
  uint16 weight[17]; /* 0x10000ul >> bitlen */
  uint16 start[17];  /* first code of bitlen */
  uint16 total;
  uint32 i;
  int32  j, k, l, m, n, avail;
  uint16 *p;

#if 0
  if(dat->error)
    return;
#endif

  avail = nchar;

  memset(count, 0, 17*2);
  for(i = 1; i <= 16; i++)
    weight[i] = 1 << (16 - i);

  /* count */
  for(i = 0; i < nchar; i++)
    count[bitlen[i]]++;

  /* calculate first code */
  total = 0;
  for(i = 1; i <= 16; i++)
  {
    start[i] = total;
    total += weight[i] * count[i];
  }
  if(total & 0xFFFF)
  {
    dat->error = 1;
    return -1;
  }

  /* shift data for make table. */
  m = 16 - tablebits;
  for(i = 1; i <= tablebits; i++) {
    start[i] >>= m;
    weight[i] >>= m;
  }

  /* initialize */
  j = start[tablebits + 1] >> m;
  k = 1 << tablebits;
  if(j != 0) {

    /* Sanity check */
    if (k > table_size) {
      return -1;
    }

    for(i = j; i < k; i++)
      table[i] = 0;
  }

  /* create table and tree */
  for(j = 0; j < nchar; j++)
  {
    k = bitlen[j];
    if(k == 0)
      continue;
    l = start[k] + weight[k];

    if(k <= tablebits)
    {
      /* Sanity check */
      if (l > table_size) {
        return -1;
      }

      /* code in table */
      for(i = start[k]; i < l; i++)
        table[i] = j;
    }
    else
    {
      /* code not in table */
      p = &table[(i = start[k]) >> m];
      i <<= tablebits;
      n = k - tablebits;
      /* make tree (n length) */
      while(--n >= 0)
      {
        if(*p == 0)
        {
          dat->d.st.right[avail] = dat->d.st.left[avail] = 0;
          *p = avail++;
        }
        if(i & 0x8000)
          p = &dat->d.st.right[*p];
        else
          p = &dat->d.st.left[*p];
        i <<= 1;
      }
      *p = j;
    }
    start[k] = l;
  }
  
  return 0;
}

/* ------------------------------------------------------------------------ */

static int read_pt_len(struct LhADecrData *dat, int16 nn, int16 nbit, int16 i_special)
{
  int16 i, c, n;

  if(!(n = getbits(dat, nbit)))
  {
    c = getbits(dat, nbit);
    for(i = 0; i < nn; i++)
      dat->d.st.pt_len[i] = 0;
    for(i = 0; i < 256; i++)
      dat->d.st.pt_table[i] = c;
  }
  else
  {
    i = 0;
    while(i < n)
    {
      c = dat->bitbuf >> (16 - 3);
      if(c == 7)
      {
        uint16 mask;

        mask = 1 << (16 - 4);
        while(mask & dat->bitbuf)
        {
          mask >>= 1;
          c++;
        }
      }
      fillbuf(dat, (c < 7) ? 3 : c - 3);
      dat->d.st.pt_len[i++] = c;
      if(i == i_special)
      {
        c = getbits(dat, 2);
        while(--c >= 0)
          dat->d.st.pt_len[i++] = 0;
      }
    }
    while(i < nn)
      dat->d.st.pt_len[i++] = 0;
    if (make_table(dat, nn, dat->d.st.pt_len, 8, dat->d.st.pt_table, 256) < 0)
      return -1;
  }

  return 0;
}

static int read_c_len(struct LhADecrData *dat)
{
  int16 i, c, n;

  if(!(n = getbits(dat, CBIT)))
  {
    c = getbits(dat, CBIT);
    for(i = 0; i < NC; i++)
      dat->d.st.c_len[i] = 0;
    for(i = 0; i < 4096; i++)
      dat->d.st.c_table[i] = c;
  }
  else
  {
    i = 0;
    while(i < n)
    {
      c = dat->d.st.pt_table[dat->bitbuf >> (16 - 8)];
      if(c >= NT)
      {
        uint16 mask;

        mask = 1 << (16 - 9);
        do
        {
          if(dat->bitbuf & mask)
            c = dat->d.st.right[c];
          else
            c = dat->d.st.left[c];
          mask >>= 1;
        } while(c >= NT);
      }
      fillbuf(dat, dat->d.st.pt_len[c]);
      if(c <= 2)
      {
        if(!c)
          c = 1;
        else if(c == 1)
          c = getbits(dat, 4) + 3;
        else
          c = getbits(dat, CBIT) + 20;

	/* Sanity check */
	if (i + c >= NC)
	  return -1;

        while(--c >= 0)
          dat->d.st.c_len[i++] = 0;
      }
      else
        dat->d.st.c_len[i++] = c - 2;
    }
    while(i < NC)
      dat->d.st.c_len[i++] = 0;
    if (make_table(dat, NC, dat->d.st.c_len, 12, dat->d.st.c_table, 4096) < 0)
      return -1;
  }

  return 0;
}

static int decode_c_st1(struct LhADecrData *dat)
{
  uint16 j, mask;

  if(!dat->d.st.blocksize)
  {
    dat->d.st.blocksize = getbits(dat, 16);
    if (read_pt_len(dat, NT, TBIT, 3) < 0)
      return -1;
    if (read_c_len(dat) < 0)
      return -1;
    if (read_pt_len(dat, dat->d.st.np, dat->d.st.pbit, -1) < 0)
      return -1;
  }
  dat->d.st.blocksize--;
  j = dat->d.st.c_table[dat->bitbuf >> 4];
  if(j < NC)
    fillbuf(dat, dat->d.st.c_len[j]);
  else
  {
    fillbuf(dat, 12);
    mask = 1 << (16 - 1);
    do
    {
      if(dat->bitbuf & mask)
        j = dat->d.st.right[j];
      else
        j = dat->d.st.left[j];
      mask >>= 1;
    } while(j >= NC);
    fillbuf(dat, dat->d.st.c_len[j] - 12);
  }
  return j;
}

static uint16 decode_p_st1(struct LhADecrData *dat)
{
  uint16 j, mask;

  j = dat->d.st.pt_table[dat->bitbuf >> (16 - 8)];
  if(j < dat->d.st.np)
    fillbuf(dat, dat->d.st.pt_len[j]);
  else
  {
    fillbuf(dat, 8);
    mask = 1 << (16 - 1);
    do
    {
      if(dat->bitbuf & mask)
        j = dat->d.st.right[j];
      else
        j = dat->d.st.left[j];
      mask >>= 1;
    } while(j >= dat->d.st.np);
    fillbuf(dat, dat->d.st.pt_len[j] - 8);
  }
  if(j)
    j = (1 << (j - 1)) + getbits(dat, j - 1);
  return j;
}

static int decode_start_st1(struct LhADecrData *dat)
{
  if(dat->DicBit <= 13)
  {
    dat->d.st.np = 14;
    dat->d.st.pbit = 4;
  }
  else
  {
    if(dat->DicBit == 16)
      dat->d.st.np = 17; /* for -lh7- */
    else
      dat->d.st.np = 16;
    dat->d.st.pbit = 5;
  }
  init_getbits(dat);
//  dat->d.st.blocksize = 0; /* done automatically */

  return 0;
}

/* ------------------------------------------------------------------------ */

static void start_c_dyn(struct LhADecrData *dat)
{
  int32 i, j, f;

  dat->d.st.n1 = (dat->d.st.n_max >= 256 + dat->d.st.maxmatch - THRESHOLD + 1) ? 512 : dat->d.st.n_max - 1;
  for(i = 0; i < TREESIZE_C; i++)
  {
    dat->d.st.stock[i] = i;
    dat->d.st.block[i] = 0;
  }
  for(i = 0, j = dat->d.st.n_max * 2 - 2; i < (int32) dat->d.st.n_max; i++, j--)
  {
    dat->d.st.freq[j] = 1;
    dat->d.st.child[j] = ~i;
    dat->d.st.s_node[i] = j;
    dat->d.st.block[j] = 1;
  }
  dat->d.st.avail = 2;
  dat->d.st.edge[1] = dat->d.st.n_max - 1;
  i = dat->d.st.n_max * 2 - 2;
  while(j >= 0)
  {
    f = dat->d.st.freq[j] = dat->d.st.freq[i] + dat->d.st.freq[i - 1];
    dat->d.st.child[j] = i;
    dat->d.st.parent[i] = dat->d.st.parent[i - 1] = j;
    if(f == dat->d.st.freq[j + 1])
    {
      dat->d.st.edge[dat->d.st.block[j] = dat->d.st.block[j + 1]] = j;
    }
    else
    {
      dat->d.st.edge[dat->d.st.block[j] = dat->d.st.stock[dat->d.st.avail++]] = j;
    }
    i -= 2;
    j--;
  }
}

#ifdef ENABLE_LH2

static void start_p_dyn(struct LhADecrData *dat)
{
  dat->d.st.freq[ROOT_P] = 1;
  dat->d.st.child[ROOT_P] = ~(N_CHAR);
  dat->d.st.s_node[N_CHAR] = ROOT_P;
  dat->d.st.edge[dat->d.st.block[ROOT_P] = dat->d.st.stock[dat->d.st.avail++]] = ROOT_P;
  dat->d.st.most_p = ROOT_P;
  dat->d.st.total_p = 0;
  dat->d.st.nn = 1 << dat->DicBit;
  dat->nextcount = 64;
}

static void decode_start_dyn(struct LhADecrData *dat)
{
  dat->d.st.n_max = 286;
  dat->d.st.maxmatch = MAXMATCH;
  init_getbits(dat);
  start_c_dyn(dat);
  start_p_dyn(dat);
}

#endif

static void reconst(struct LhADecrData *dat, int32 start, int32 end)
{
  int32  i, j, k, l, b = 0;
  uint32 f, g;

  for(i = j = start; i < end; i++)
  {
    if((k = dat->d.st.child[i]) < 0)
    {
      dat->d.st.freq[j] = (dat->d.st.freq[i] + 1) / 2;
      dat->d.st.child[j] = k;
      j++;
    }
    if(dat->d.st.edge[b = dat->d.st.block[i]] == i)
    {
      dat->d.st.stock[--dat->d.st.avail] = b;
    }
  }
  j--;
  i = end - 1;
  l = end - 2;
  while(i >= start)
  {
    while(i >= l)
    {
      dat->d.st.freq[i] = dat->d.st.freq[j];
      dat->d.st.child[i] = dat->d.st.child[j];
      i--, j--;
    }
    f = dat->d.st.freq[l] + dat->d.st.freq[l + 1];
    for(k = start; f < dat->d.st.freq[k]; k++)
      ;
    while(j >= k)
    {
      dat->d.st.freq[i] = dat->d.st.freq[j];
      dat->d.st.child[i] = dat->d.st.child[j];
      i--, j--;
    }
    dat->d.st.freq[i] = f;
    dat->d.st.child[i] = l + 1;
    i--;
    l -= 2;
  }
  f = 0;
  for(i = start; i < end; i++)
  {
    if((j = dat->d.st.child[i]) < 0)
      dat->d.st.s_node[~j] = i;
    else
      dat->d.st.parent[j] = dat->d.st.parent[j - 1] = i;
    if((g = dat->d.st.freq[i]) == f) {
      dat->d.st.block[i] = b;
    }
    else
    {
      dat->d.st.edge[b = dat->d.st.block[i] = dat->d.st.stock[dat->d.st.avail++]] = i;
      f = g;
    }
  }
}

static int32 swap_inc(struct LhADecrData *dat, int32 p)
{
  int32 b, q, r, s;

  b = dat->d.st.block[p];
  if((q = dat->d.st.edge[b]) != p)
  { /* swap for leader */
    r = dat->d.st.child[p];
    s = dat->d.st.child[q];
    dat->d.st.child[p] = s;
    dat->d.st.child[q] = r;
    if(r >= 0)
      dat->d.st.parent[r] = dat->d.st.parent[r - 1] = q;
    else
      dat->d.st.s_node[~r] = q;
    if(s >= 0)
      dat->d.st.parent[s] = dat->d.st.parent[s - 1] = p;
    else
      dat->d.st.s_node[~s] = p;
    p = q;
    dat->d.st.edge[b]++;
    if(++dat->d.st.freq[p] == dat->d.st.freq[p - 1])
    {
      dat->d.st.block[p] = dat->d.st.block[p - 1];
    }
    else
    {
      dat->d.st.edge[dat->d.st.block[p] = dat->d.st.stock[dat->d.st.avail++]] = p;  /* create block */
    }
  }
  else if(b == dat->d.st.block[p + 1])
  {
    dat->d.st.edge[b]++;
    if(++dat->d.st.freq[p] == dat->d.st.freq[p - 1])
    {
      dat->d.st.block[p] = dat->d.st.block[p - 1];
    }
    else
    {
      dat->d.st.edge[dat->d.st.block[p] = dat->d.st.stock[dat->d.st.avail++]] = p;  /* create block */
    }
  }
  else if(++dat->d.st.freq[p] == dat->d.st.freq[p - 1])
  {
    dat->d.st.stock[--dat->d.st.avail] = b; /* delete block */
    dat->d.st.block[p] = dat->d.st.block[p - 1];
  }
  return dat->d.st.parent[p];
}

#ifdef ENABLE_LH2

static void update_p(struct LhADecrData *dat, int32 p)
{
  int32 q;

  if(dat->d.st.total_p == 0x8000)
  {
    reconst(dat, ROOT_P, dat->d.st.most_p + 1);
    dat->d.st.total_p = dat->d.st.freq[ROOT_P];
    dat->d.st.freq[ROOT_P] = 0xffff;
  }
  q = dat->d.st.s_node[p + N_CHAR];
  while(q != ROOT_P)
  {
    q = swap_inc(dat, q);
  }
  dat->d.st.total_p++;
}

static void make_new_node(struct LhADecrData *dat, int32 p)
{
  int32 q, r;

  r = dat->d.st.most_p + 1;
  q = r + 1;
  dat->d.st.s_node[~(dat->d.st.child[r] = dat->d.st.child[dat->d.st.most_p])] = r;
  dat->d.st.child[q] = ~(p + N_CHAR);
  dat->d.st.child[dat->d.st.most_p] = q;
  dat->d.st.freq[r] = dat->d.st.freq[dat->d.st.most_p];
  dat->d.st.freq[q] = 0;
  dat->d.st.block[r] = dat->d.st.block[dat->d.st.most_p];
  if(dat->d.st.most_p == ROOT_P)
  {
    dat->d.st.freq[ROOT_P] = 0xffff;
    dat->d.st.edge[dat->d.st.block[ROOT_P]]++;
  }
  dat->d.st.parent[r] = dat->d.st.parent[q] = dat->d.st.most_p;
  dat->d.st.edge[dat->d.st.block[q] = dat->d.st.stock[dat->d.st.avail++]] =
  dat->d.st.s_node[p + N_CHAR] = dat->d.st.most_p = q;
  update_p(dat, p);
}

#endif

static void update_c(struct LhADecrData *dat, int32 p)
{
  int32 q;

  if(dat->d.st.freq[ROOT_C] == 0x8000)
  {
    reconst(dat, 0, (int32) dat->d.st.n_max * 2 - 1);
  }
  dat->d.st.freq[ROOT_C]++;
  q = dat->d.st.s_node[p];
  do
  {
    q = swap_inc(dat, q);
  } while(q != ROOT_C);
}

static int decode_c_dyn(struct LhADecrData *dat)
{
  int32 c;
  int16 buf, cnt;

  c = dat->d.st.child[ROOT_C];
  buf = dat->bitbuf;
  cnt = 0;
  do
  {
    c = dat->d.st.child[c - (buf < 0)];
    buf <<= 1;
    if(++cnt == 16)
    {
      fillbuf(dat, 16);
      buf = dat->bitbuf;
      cnt = 0;
    }
  } while(c > 0);
  fillbuf(dat, cnt);
  c = ~c;
  update_c(dat, c);
  if(c == dat->d.st.n1)
    c += getbits(dat, 8);
  return (uint16) c;
}

#ifdef ENABLE_LH2

static uint16 decode_p_dyn(struct LhADecrData *dat)
{
  int32 c;
  int16 buf, cnt;

  while(dat->count > dat->nextcount)
  {
    make_new_node(dat, (int32) dat->nextcount / 64);
    if((dat->nextcount += 64) >= (uint32)dat->d.st.nn)
      dat->nextcount = 0xffffffff;
  }
  c = dat->d.st.child[ROOT_P];
  buf = dat->bitbuf;
  cnt = 0;
  while(c > 0)
  {
    c = dat->d.st.child[c - (buf < 0)];
    buf <<= 1;
    if(++cnt == 16)
    {
      fillbuf(dat, 16);
      buf = dat->bitbuf;
      cnt = 0;
    }
  }
  fillbuf(dat, cnt);
  c = (~c) - N_CHAR;
  update_p(dat, c);

  return (uint16) ((c << 6) + getbits(dat, 6));
}

#endif

/* ------------------------------------------------------------------------ */

static const int32 fixed[2][16] = {
  {3, 0x01, 0x04, 0x0c, 0x18, 0x30, 0}, /* old compatible */
  {2, 0x01, 0x01, 0x03, 0x06, 0x0D, 0x1F, 0x4E, 0}  /* 8K buf */
};

static void ready_made(struct LhADecrData *dat, int32 method)
{
  int32  i, j;
  uint32 code, weight;
  int32 *tbl;

  tbl = (int32 *) fixed[method];
  j = *tbl++;
  weight = 1 << (16 - j);
  code = 0;
  for(i = 0; i < dat->d.st.np; i++)
  {
    while(*tbl == i)
    {
      j++;
      tbl++;
      weight >>= 1;
    }
    dat->d.st.pt_len[i] = j;
    dat->d.st.pt_code[i] = code;
    code += weight;
  }
}

static int decode_start_fix(struct LhADecrData *dat)
{
  dat->d.st.n_max = 314;
  dat->d.st.maxmatch = 60;
  init_getbits(dat);
  dat->d.st.np = 1 << (12 - 6);
  start_c_dyn(dat);
  ready_made(dat, 0);
  if (make_table(dat, dat->d.st.np, dat->d.st.pt_len, 8, dat->d.st.pt_table, 256) < 0)
    return -1;

  return 0;
}

static uint16 decode_p_st0(struct LhADecrData *dat)
{
  int32 i, j;

  j = dat->d.st.pt_table[dat->bitbuf >> 8];
  if(j < dat->d.st.np)
  {
    fillbuf(dat, dat->d.st.pt_len[j]);
  }
  else
  {
    fillbuf(dat, 8);
    i = dat->bitbuf;
    do
    {
      if((int16) i < 0)
        j = dat->d.st.right[j];
      else
        j = dat->d.st.left[j];
      i <<= 1;
    } while(j >= dat->d.st.np);
    fillbuf(dat, dat->d.st.pt_len[j] - 8);
  }
  return (uint16)((j << 6) + getbits(dat, 6));
}

#ifdef ENABLE_LH3

static void decode_start_st0(struct LhADecrData *dat)
{
  dat->d.st.n_max = 286;
  dat->d.st.maxmatch = MAXMATCH;
  init_getbits(dat);
  dat->d.st.np = 1 << (MAX_DICBIT - 6);
}

static int read_tree_c(struct LhADecrData *dat) /* read tree from file */
{
  int32 i, c;

  i = 0;
  while(i < N1)
  {
    if(getbits(dat, 1))
      dat->d.st.c_len[i] = getbits(dat, LENFIELD) + 1;
    else
      dat->d.st.c_len[i] = 0;
    if(++i == 3 && dat->d.st.c_len[0] == 1 && dat->d.st.c_len[1] == 1 && dat->d.st.c_len[2] == 1)
    {
      c = getbits(dat, CBIT);
      memset(dat->d.st.c_len, 0, N1);
      for(i = 0; i < 4096; i++)
        dat->d.st.c_table[i] = c;
      return 0;
    }
  }
  if (make_table(dat, N1, dat->d.st.c_len, 12, dat->d.st.c_table, 4096) < 0)
    return -1;

  return 0;
}

static void read_tree_p(struct LhADecrData *dat) /* read tree from file */
{
  int32 i, c;

  i = 0;
  while(i < NP)
  {
    dat->d.st.pt_len[i] = getbits(dat, LENFIELD);
    if(++i == 3 && dat->d.st.pt_len[0] == 1 && dat->d.st.pt_len[1] == 1 && dat->d.st.pt_len[2] == 1)
    {
      c = getbits(dat, MAX_DICBIT - 6);
      for(i = 0; i < NP; i++)
        dat->d.st.c_len[i] = 0;
      for(i = 0; i < 256; i++)
        dat->d.st.c_table[i] = c;
      return;
    }
  }
}

static int decode_c_st0(struct LhADecrData *dat)
{
  int32 i, j;

  if(!dat->d.st.blocksize) /* read block head */
  {
    dat->d.st.blocksize = getbits(dat, BUFBITS); /* read block blocksize */
    if (read_tree_c(dat) < 0)
      return -1;
    if(getbits(dat, 1))
    {
      read_tree_p(dat);
    }
    else
    {
      ready_made(dat, 1);
    }
    if (make_table(dat, NP, dat->d.st.pt_len, 8, dat->d.st.pt_table, 256) < 0)
      return -1;
  }
  dat->d.st.blocksize--;
  j = dat->d.st.c_table[dat->bitbuf >> 4];
  if(j < N1)
    fillbuf(dat, dat->d.st.c_len[j]);
  else
  {
    fillbuf(dat, 12);
    i = dat->bitbuf;
    do
    {
      if((int16) i < 0)
        j = dat->d.st.right[j];
      else
        j = dat->d.st.left[j];
      i <<= 1;
    } while(j >= N1);
    fillbuf(dat, dat->d.st.c_len[j] - 12);
  }
  if (j == N1 - 1)
    j += getbits(dat, EXTRABITS);
  return (uint16) j;
}

#endif

/* ------------------------------------------------------------------------ */

#ifdef ENABLE_PMARC

static const int32 PMARC2_historyBits[8] = { 3,  3,  4,  5,  5,  5,  6,  6};
static const int32 PMARC2_historyBase[8] = { 0,  8, 16, 32, 64, 96,128,192};
static const int32 PMARC2_repeatBits[6]  = { 3,  3,  5,  6,  7,  0};
static const int32 PMARC2_repeatBase[6]  = {17, 25, 33, 65,129,256};

static void PMARC2_hist_update(struct LhADecrData *dat, uint8 data)
{
  if(data != dat->d.pm.lastbyte)
  {
    uint8 oldNext, oldPrev, newNext;

    /* detach from old position */
    oldNext = dat->d.pm.next[data];
    oldPrev = dat->d.pm.prev[data];
    dat->d.pm.prev[oldNext] = oldPrev;
    dat->d.pm.next[oldPrev] = oldNext;

    /* attach to new next */
    newNext = dat->d.pm.next[dat->d.pm.lastbyte];
    dat->d.pm.prev[newNext] = data;
    dat->d.pm.next[data] = newNext;

    /* attach to new prev */
    dat->d.pm.prev[data] = dat->d.pm.lastbyte;
    dat->d.pm.next[dat->d.pm.lastbyte] = data;

    dat->d.pm.lastbyte = data;
  }
}

static int32 PMARC2_tree_get(struct LhADecrData *dat, struct PMARC2_Tree *t)
{
  int32 i;
  i = t->root;

  while (i < 0x80)
  {
    i = (getbits(dat, 1) == 0 ? t->leftarr[i] : t->rightarr[i] );
  }
  return i & 0x7F;
}

static void PMARC2_tree_rebuild(struct LhADecrData *dat, struct PMARC2_Tree *t,
uint8 bound, uint8 mindepth, uint8 * table)
{
  uint8 d;
  int32 i, curr, empty, n;

  t->root = 0;
  memset(t->leftarr, 0, bound);
  memset(t->rightarr, 0, bound);
  memset(dat->d.pm.parentarr, 0, bound);

  for(i = 0; i < dat->d.pm.mindepth - 1; i++)
  {
    t->leftarr[i] = i + 1;
    dat->d.pm.parentarr[i+1] = i;
  }

  curr = dat->d.pm.mindepth - 1;
  empty = dat->d.pm.mindepth;
  for(d = dat->d.pm.mindepth; ; d++)
  {
    for(i = 0; i < bound; i++)
    {
      if(table[i] == d)
      {
        if(t->leftarr[curr] == 0)
          t->leftarr[curr] = i | 128;
        else
        {
          t->rightarr[curr] = i | 128;
          n = 0;
          while(t->rightarr[curr] != 0)
          {
            if(curr == 0) /* root? -> done */
              return;
            curr = dat->d.pm.parentarr[curr];
            n++;
          }
          t->rightarr[curr] = empty;
          for(;;)
          {
            dat->d.pm.parentarr[empty] = curr;
            curr = empty;
            empty++;

            n--;
            if(n == 0)
              break;
            t->leftarr[curr] = empty;
          }
        }
      }
    }
    if(t->leftarr[curr] == 0)
      t->leftarr[curr] = empty;
    else
      t->rightarr[curr] = empty;

    dat->d.pm.parentarr[empty] = curr;
    curr = empty;
    empty++;
  }
}

static uint8 PMARC2_hist_lookup(struct LhADecrData *dat, int32 n)
{
  uint8 i;
  uint8 *direction = dat->d.pm.prev;

  if(n >= 0x80)
  {
    /* Speedup: If you have to process more than half the ring,
                it's faster to walk the other way around. */
    direction = dat->d.pm.next;
    n = 0x100 - n;
  }
  for(i = dat->d.pm.lastbyte; n != 0; n--)
    i = direction[i];
  return i;
}

static void PMARC2_maketree1(struct LhADecrData *dat)
{
  int32 i, nbits, x;

  dat->d.pm.tree1bound = getbits(dat, 5);
  dat->d.pm.mindepth = getbits(dat, 3);

  if(dat->d.pm.mindepth == 0)
    dat->d.pm.tree1.root = 128 | (dat->d.pm.tree1bound - 1);
  else
  {
    memset(dat->d.pm.table1, 0, 32);
    nbits = getbits(dat, 3);
    for(i = 0; i < dat->d.pm.tree1bound; i++)
    {
      if((x = getbits(dat, nbits)))
        dat->d.pm.table1[i] = x - 1 + dat->d.pm.mindepth;
    }
    PMARC2_tree_rebuild(dat, &dat->d.pm.tree1, dat->d.pm.tree1bound,
    dat->d.pm.mindepth, dat->d.pm.table1);
  }
}

static void PMARC2_maketree2(struct LhADecrData *dat, int32 par_b)
/* in use: 5 <= par_b <= 8 */
{
  int32 i, count, index;

  if(dat->d.pm.tree1bound < 10)
    return;
  if(dat->d.pm.tree1bound == 29 && dat->d.pm.mindepth == 0)
    return;

  for(i = 0; i < 8; i++)
    dat->d.pm.table2[i] = 0;
  for(i = 0; i < par_b; i++)
    dat->d.pm.table2[i] = getbits(dat, 3);
  index = 0;
  count = 0;
  for(i = 0; i < 8; i++)
  {
    if(dat->d.pm.table2[i] != 0)
    {
      index = i;
      count++;
    }
  }

  if(count == 1)
  {
    dat->d.pm.tree2.root = 128 | index;
  }
  else if (count > 1)
  {
    dat->d.pm.mindepth = 1;
    PMARC2_tree_rebuild(dat, &dat->d.pm.tree2, 8, dat->d.pm.mindepth, dat->d.pm.table2);
  }
  /* Note: count == 0 is possible! */
}

static void decode_start_pm2(struct LhADecrData *dat)
{
  int32 i;

  dat->d.pm.tree1.leftarr = dat->d.pm.tree1left;
  dat->d.pm.tree1.rightarr = dat->d.pm.tree1right;
/*  dat->d.pm.tree1.root = 0; */
  dat->d.pm.tree2.leftarr = dat->d.pm.tree2left;
  dat->d.pm.tree2.rightarr = dat->d.pm.tree2right;
/*  dat->d.pm.tree2.root = 0; */

  dat->d.pm.dicsiz1 = (1 << dat->DicBit) - 1;
  init_getbits(dat);

  /* history init */
  for(i = 0; i < 0x100; i++)
  {
    dat->d.pm.prev[(0xFF + i) & 0xFF] = i;
    dat->d.pm.next[(0x01 + i) & 0xFF] = i;
  }
  dat->d.pm.prev[0x7F] = 0x00; dat->d.pm.next[0x00] = 0x7F;
  dat->d.pm.prev[0xDF] = 0x80; dat->d.pm.next[0x80] = 0xDF;
  dat->d.pm.prev[0x9F] = 0xE0; dat->d.pm.next[0xE0] = 0x9F;
  dat->d.pm.prev[0x1F] = 0xA0; dat->d.pm.next[0xA0] = 0x1F;
  dat->d.pm.prev[0xFF] = 0x20; dat->d.pm.next[0x20] = 0xFF;
  dat->d.pm.lastbyte = 0x20;

/*  dat->nextcount = 0; */
/*  dat->d.pm.lastupdate = 0; */
  getbits(dat, 1); /* discard bit */
}

static uint16 decode_c_pm2(struct LhADecrData *dat)
{
  /* various admin: */
  while(dat->d.pm.lastupdate != dat->loc)
  {
    PMARC2_hist_update(dat, dat->text[dat->d.pm.lastupdate]);
    dat->d.pm.lastupdate = (dat->d.pm.lastupdate + 1) & dat->d.pm.dicsiz1;
  }
  while(dat->count >= dat->nextcount)
  /* Actually it will never loop, because count doesn't grow that fast.
     However, this is the way  does it.
     Probably other encoding methods can have repeats larger than 256 bytes.
     Note:  puts this code in decode_p...
  */
  {
    if(dat->nextcount == 0x0000)
    {
      PMARC2_maketree1(dat);
      PMARC2_maketree2(dat, 5);
      dat->nextcount = 0x0400;
    }
    else if(dat->nextcount == 0x0400)
    {
      PMARC2_maketree2(dat, 6);
      dat->nextcount = 0x0800;
    }
    else if(dat->nextcount == 0x0800)
    {
      PMARC2_maketree2(dat, 7);
      dat->nextcount = 0x1000;
    }
    else if(dat->nextcount == 0x1000)
    {
      if(getbits(dat, 1) != 0)
        PMARC2_maketree1(dat);
      PMARC2_maketree2(dat, 8);
      dat->nextcount = 0x2000;
    }
    else
    { /* 0x2000, 0x3000, 0x4000, ... */
      if(getbits(dat, 1) != 0)
      {
        PMARC2_maketree1(dat);
        PMARC2_maketree2(dat, 8);
      }
      dat->nextcount += 0x1000;
    }
  }
  dat->d.pm.gettree1 = PMARC2_tree_get(dat, &dat->d.pm.tree1); /* value preserved for decode_p */

  /* direct value (ret <= UCHAR_MAX) */
  if(dat->d.pm.gettree1 < 8)
  {
    return (uint16) (PMARC2_hist_lookup(dat, PMARC2_historyBase[dat->d.pm.gettree1]
    + getbits(dat, PMARC2_historyBits[dat->d.pm.gettree1])));
  }

  /* repeats: (ret > UCHAR_MAX) */
  if(dat->d.pm.gettree1 < 23)
  {
    return (uint16) (PMARC2_OFFSET + 2 + (dat->d.pm.gettree1 - 8));
  }

  return (uint16) (PMARC2_OFFSET + PMARC2_repeatBase[dat->d.pm.gettree1 - 23]
  + getbits(dat, PMARC2_repeatBits[dat->d.pm.gettree1 - 23]));
}

static uint16 decode_p_pm2(struct LhADecrData *dat)
{
  /* gettree1 value preserved from decode_c */
  int32 nbits, delta, gettree2;

  if(dat->d.pm.gettree1 == 8)
  { /* 2-byte repeat with offset 0..63 */
    nbits = 6; delta = 0;
  }
  else if(dat->d.pm.gettree1 < 28)
  { /* n-byte repeat with offset 0..8191 */
    if(!(gettree2 = PMARC2_tree_get(dat, &dat->d.pm.tree2)))
    {
      nbits = 6;
      delta = 0;
    }
    else
    { /* 1..7 */
      nbits = 5 + gettree2;
      delta = 1 << nbits;
    }
  }
  else
  { /* 256 bytes repeat with offset 0 */
    nbits = 0;
    delta = 0;
  }
  return (uint16) (delta + getbits(dat, nbits));
}

#endif

/* ------------------------------------------------------------------------ */

#ifdef ENABLE_LARC

static uint16 decode_c_lzs(struct LhADecrData *dat)
{
  if(getbits(dat, 1))
  {
    return getbits(dat, 8);
  }
  else
  {
    dat->d.lz.matchpos = getbits(dat, 11);
    return (uint16) (getbits(dat, 4) + 0x100);
  }
}

static uint16 decode_p_lzs(struct LhADecrData *dat)
{
  return (uint16) ((dat->loc - dat->d.lz.matchpos - MAGIC0) & 0x7ff);
}

static void decode_start_lzs(struct LhADecrData *dat)
{
  init_getbits(dat);
}

static uint16 decode_c_lz5(struct LhADecrData *dat)
{
  int32 c;

  if(!dat->d.lz.flagcnt)
  {
    dat->d.lz.flagcnt = 8;
    dat->d.lz.flag = fgetc(dat->in);
  }
  dat->d.lz.flagcnt--;
  c = fgetc(dat->in);
  if((dat->d.lz.flag & 1) == 0)
  {
    dat->d.lz.matchpos = c;
    c = fgetc(dat->in);
    dat->d.lz.matchpos += (c & 0xf0) << 4;
    c &= 0x0f;
    c += 0x100;
  }
  dat->d.lz.flag >>= 1;
  return (uint16) c;
}

static uint16 decode_p_lz5(struct LhADecrData *dat)
{
  return (uint16) ((dat->loc - dat->d.lz.matchpos - MAGIC5) & 0xfff);
}

static void decode_start_lz5(struct LhADecrData *dat)
{
  int32 i;
  char *text;

  text = dat->text;

  dat->d.lz.flagcnt = 0;

  for(i = 0; i < 256; i++)
    memset(text + i * 13 + 18, i, 13);

  for(i = 0; i < 256; i++)
    text[256 * 13 + 18 + i] = i;

  for(i = 0; i < 256; i++)
    text[256 * 13 + 256 + 18 + i] = 255 - i;

  memset(text + 256 * 13 + 512 + 18, 0, 128);
  memset(text + 256 * 13 + 512 + 128 + 18, ' ', 128-18);
}

#endif

static int32 LhA_Decrunch(FILE *in, FILE *out, int size, uint32 Method)
{
  struct LhADecrData *dd;
  int32 err = 0;

  if((dd = calloc(sizeof(struct LhADecrData), 1))) {
    int (*DecodeStart)(struct LhADecrData *);
    int (*DecodeC)(struct LhADecrData *);
    uint16 (*DecodeP)(struct LhADecrData *);

    /* most often used stuff */
    dd->in = in;
    dd->DicBit = 13;
    DecodeStart = decode_start_st1;
    DecodeP = decode_p_st1;
    DecodeC = decode_c_st1;

    switch(Method)
    {
    case LZHUFF1_METHOD:
      dd->DicBit = 12;
      DecodeStart = decode_start_fix;
      DecodeC = decode_c_dyn;
      DecodeP = decode_p_st0;
      break;

#ifdef ENABLE_LH2
    case LZHUFF2_METHOD:
      DecodeStart = decode_start_dyn;
      DecodeC = decode_c_dyn;
      DecodeP = decode_p_dyn;
      break;
#endif

#ifdef ENABLE_LH3
    case LZHUFF3_METHOD:
      DecodeStart = decode_start_st0;
      DecodeP = decode_p_st0;
      DecodeC = decode_c_st0;
      break;
#endif

#ifdef ENABLE_PMARC
    case PMARC2_METHOD:
      DecodeStart = decode_start_pm2;
      DecodeP = decode_p_pm2;
      DecodeC = decode_c_pm2;
      break;
#endif

    case LZHUFF4_METHOD:
      dd->DicBit = 12;
//      break;
    case LZHUFF5_METHOD:
      break;
    case LZHUFF6_METHOD:
      dd->DicBit = 15;
      break;
    case LZHUFF7_METHOD:
      dd->DicBit = 16;
      break;
    case LZHUFF8_METHOD:
      dd->DicBit = 17;
      break;

#ifdef ENABLE_LARC
    case LARC_METHOD:
      dd->DicBit = 11;
      DecodeStart = decode_start_lzs;
      DecodeC = decode_c_lzs;
      DecodeP = decode_p_lzs;
      break;
    case LARC5_METHOD:
      dd->DicBit = 12;
      DecodeStart = decode_start_lz5;
      DecodeC = decode_c_lz5;
      DecodeP = decode_p_lz5;
      break;
#endif

    default:
      err = 1; break;
    }
    if(!err)
    {
      char *text;
      int32 i, c, offset;
      uint32 dicsiz;

      dicsiz = 1 << dd->DicBit;

      offset = (Method == LARC_METHOD || Method == PMARC2_METHOD) ? 0x100 - 2 : 0x100 - 3;

      if((text = dd->text = calloc(dicsiz, 1)))
      {
/*      if(Method == LZHUFF1_METHOD || Method == LZHUFF2_METHOD || Method == LZHUFF3_METHOD ||
        Method == LZHUFF6_METHOD || Method == LARC_METHOD || Method == LARC5_METHOD)
*/
          memset(text, ' ', (size_t) dicsiz);

        if (DecodeStart(dd) < 0) {
          free(dd->text);
          free(dd);
          return -1;
        }

        --dicsiz; /* now used with AND */
        while(1)
        {
          if (dd->count >= size)
            break;

          c = DecodeC(dd);
          if (c < 0) {
            free(dd->text);
            free(dd);
            return -1;  
          }

	  if (dd->error)
	    break;
	
          if(c <= UCHAR_MAX)
          {
            text[dd->loc++] = fputc(c, out);
            dd->loc &= dicsiz;
            dd->count++;
          }
          else
          {
            c -= offset;
            i = dd->loc - DecodeP(dd) - 1;
            dd->count += c;
            while(c--)
            {
              text[dd->loc++] = fputc(text[i++ & dicsiz], out);
              dd->loc &= dicsiz;
            }
          }
        }
        err = dd->error;
        free(text);
      }
      else
        err = -1;
    }
    free(dd);
  }
  else
    err = -1;
  return err;
}

/*
 * For xmp
 */

struct lha_data {
	int method;
	char name[256];
	int packed_size;
	int original_size;
	int crc;
};

/*
 * level 0 header
 *
 *
 * offset  size  field name
 * ----------------------------------
 *     0      1  header size    [*1]
 *     1      1  header sum
 *            ---------------------------------------
 *     2      5  method ID                         ^
 *     7      4  packed size    [*2]               |
 *    11      4  original size                     |
 *    15      2  time                              |
 *    17      2  date                              |
 *    19      1  attribute                         | [*1] header size (X+Y+22)
 *    20      1  level (0x00 fixed)                |
 *    21      1  name length                       |
 *    22      X  pathname                          |
 * X +22      2  file crc (CRC-16)                 |
 * X +24      Y  ext-header(old style)             v
 * -------------------------------------------------
 * X+Y+24        data                              ^
 *                 :                               | [*2] packed size
 *                 :                               v
 * -------------------------------------------------
 *
 * ext-header(old style)
 *     0      1  ext-type ('U')
 *     1      1  minor version
 *     2      4  UNIX time
 *     6      2  mode
 *     8      2  uid
 *    10      2  gid
 *
 * attribute (MS-DOS)
 *    bit1  read only
 *    bit2  hidden
 *    bit3  system
 *    bit4  volume label
 *    bit5  directory
 *    bit6  archive bit (need to backup)
 *
 */

/*
 * level 1 header
 *
 *
 * offset   size  field name
 * -----------------------------------
 *     0       1  header size   [*1]
 *     1       1  header sum
 *             -------------------------------------
 *     2       5  method ID                        ^
 *     7       4  skip size     [*2]               |
 *    11       4  original size                    |
 *    15       2  time                             |
 *    17       2  date                             |
 *    19       1  attribute (0x20 fixed)           | [*1] header size (X+Y+25)
 *    20       1  level (0x01 fixed)               |
 *    21       1  name length                      |
 *    22       X  filename                         |
 * X+ 22       2  file crc (CRC-16)                |
 * X+ 24       1  OS ID                            |
 * X +25       Y  ???                              |
 * X+Y+25      2  next-header size                 v
 * -------------------------------------------------
 * X+Y+27      Z  ext-header                       ^
 *                 :                               |
 * -----------------------------------             | [*2] skip size
 * X+Y+Z+27       data                             |
 *                 :                               v
 * -------------------------------------------------
 *
 */

/*
 * level 2 header
 *
 *
 * offset   size  field name
 * --------------------------------------------------
 *     0       2  total header size [*1]           ^
 *             -----------------------             |
 *     2       5  method ID                        |
 *     7       4  packed size       [*2]           |
 *    11       4  original size                    |
 *    15       4  time                             |
 *    19       1  RESERVED (0x20 fixed)            | [*1] total header size
 *    20       1  level (0x02 fixed)               |      (X+26+(1))
 *    21       2  file crc (CRC-16)                |
 *    23       1  OS ID                            |
 *    24       2  next-header size                 |
 * -----------------------------------             |
 *    26       X  ext-header                       |
 *                 :                               |
 * -----------------------------------             |
 * X +26      (1) padding                          v
 * -------------------------------------------------
 * X +26+(1)      data                             ^
 *                 :                               | [*2] packed size
 *                 :                               v
 * -------------------------------------------------
 *
 */

/*
 * level 3 header
 *
 *
 * offset   size  field name
 * --------------------------------------------------
 *     0       2  size field length (4 fixed)      ^
 *     2       5  method ID                        |
 *     7       4  packed size       [*2]           |
 *    11       4  original size                    |
 *    15       4  time                             |
 *    19       1  RESERVED (0x20 fixed)            | [*1] total header size
 *    20       1  level (0x03 fixed)               |      (X+32)
 *    21       2  file crc (CRC-16)                |
 *    23       1  OS ID                            |
 *    24       4  total header size [*1]           |
 *    28       4  next-header size                 |
 * -----------------------------------             |
 *    32       X  ext-header                       |
 *                 :                               v
 * -------------------------------------------------
 * X +32          data                             ^
 *                 :                               | [*2] packed size
 *                 :                               v
 * -------------------------------------------------
 *
 */

static int get_header(FILE *f, struct lha_data *data)
{
	uint8 buf[21];
	int size, level, namelen;

	memset(data, 0, sizeof(struct lha_data));
	if (fread(buf, 1, 21, f) != 21)
		return -1;
	level = buf[20];

	switch (level) {
	case 0:
		size = buf[0];
		data->method = readmem32b(buf + 2);
		data->packed_size = readmem32l(buf + 7);
		data->original_size = readmem32l(buf + 11);
		namelen = read8(f);
		fread(data->name, 1, namelen, f);
		data->crc = read16l(f);
		fseek(f, size + 2 - 24 - namelen, SEEK_CUR);
		break;
	case 1:
		size = buf[0];
		data->method = readmem32b(buf + 2);
		data->packed_size = readmem32l(buf + 7);
		data->original_size = readmem32l(buf + 11);
		namelen = read8(f);
		fread(data->name, 1, namelen, f);
		data->crc = read16l(f);
		fseek(f, size - (22 + namelen) - 2, SEEK_CUR);
		while ((size = read16l(f)) != 0) {
			fseek(f, size - 2, SEEK_CUR);
			data->packed_size -= size;
		}
		break;
	case 2:
		size = readmem16l(buf);
		/* fall through */
	case 3:
		data->method = readmem32b(buf + 2);
		data->packed_size = readmem32l(buf + 7);
		data->original_size = readmem32l(buf + 11);
		data->crc = read16l(f);
		read8(f);		/* skip OS id */
		while ((size = read16l(f)) != 0) {
			int type = read8(f);
			int s = size - 3;
			if (type == 0x01) {
				/* Sanity check */
				if (s < 0 || s > 256)
					return -1;

				fread(data->name, 1, s, f);
			} else {
				fseek(f, s, SEEK_CUR);
			}
		}
		break;
	default:
		return -1;
	}

	return 0;
}

static int test_lha(unsigned char *b) {
	return b[2] == '-' && b[3] == 'l' && b[4] == 'h' && b[6] == '-' &&
		b[20] <= 3;
}

static int decrunch_lha(FILE *in, FILE *out)
{
	struct lha_data data;

	while (1) {
		if (get_header(in, &data) < 0)
			break;

#if 0
		printf("method = %x\n", data.method);
		printf("name = %s\n", data.name);
		printf("packed size = %d\n", data.packed_size);
		printf("original size = %d\n", data.original_size);
		printf("position = %lx\n", ftell(in));
#endif

		if (exclude_match(data.name)) {
			fseek(in, data.packed_size, SEEK_CUR);
			continue;
		}
		return LhA_Decrunch(in, out, data.original_size, data.method);
	}

	return -1;
}

struct depacker lha_depacker = {
	test_lha,
	decrunch_lha
};
