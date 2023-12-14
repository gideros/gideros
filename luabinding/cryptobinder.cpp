#include <stdlib.h>
#include <string.h>
#include "platformutil.h"
#include "lua.hpp"

typedef unsigned int uint;
typedef unsigned char uchar;

//HASH
class Hash
{
public:
 virtual void Init()=0;
 virtual void Compute(const void *buffer,int size)=0;
 virtual int Result(void *buffer,unsigned int sz)=0;
 virtual int GetInputSize();
 int GetOutputSize() { return Result(NULL,0); };
 void HMAC(const void *input,uint insize,const void *key,uint keysize,void *output,uint outsize);
};

int Hash::GetInputSize()
{
 return 0;
}

void Hash::HMAC(const void *text,uint sz,const void *K,uint Ks,void *R,uint Rs)
{
 uint Ls=(uint) GetOutputSize();
 uint Bs=(uint) GetInputSize();
 uchar *Kb=(uchar *) malloc(Bs);
 uchar *Ib=(uchar *) malloc(Ls);
 uint i;

 Init();

 if (Ks>Bs)
 {
  Compute(K,Ks);
  Result(Kb,Ls);
  memset(Kb+Ls,0,Bs-Ls);
  Init();
 }
 else
 {
  memcpy(Kb,K,Ks);
  if (Ks<Bs) memset(Kb+Ks,0,Bs-Ks);
 }

 for (i=0;i<Bs;i++) Kb[i]^=0x36; //ipad

 Compute(Kb,Bs); // INNER
 Compute(text,sz);
 Result(Ib,Ls);
 Init();

 for (i=0;i<Bs;i++) Kb[i]^=(0x36^0x5C); //ipad -> opad

 Compute(Kb,Bs); // OUTER
 Compute(Ib,Ls);
 Result(R,Rs);

 free(Kb);
 free(Ib);
}


//SHA1
typedef uint32_t u32;
typedef uint8_t byte;

typedef struct {
    u32  h0,h1,h2,h3,h4;
    u32  nblocks;
    byte buf[64];
    int  count;
} SHA1_CONTEXT;

class Sha1 : public Hash
{
protected:
 SHA1_CONTEXT ctx;
 void Transform(unsigned char *data);
public:
 Sha1();
 void Init();
 void Compute(const void *buffer,int size);
 int Result(void *buffer,unsigned int sz);
 int GetInputSize();
};

static inline u32 rol(u32 n,int c)
{
 return (n<<c)|(n>>(32-c));
}


Sha1::Sha1()
{
 Init();
}

void Sha1::Init()
{
 SHA1_CONTEXT *hd=(SHA1_CONTEXT *)(&ctx);
 hd->h0 = 0x67452301;
 hd->h1 = 0xefcdab89;
 hd->h2 = 0x98badcfe;
 hd->h3 = 0x10325476;
 hd->h4 = 0xc3d2e1f0;
 hd->nblocks = 0;
 hd->count = 0;
}

int Sha1::Result(void *buffer,unsigned int sz)
{
 if ((!buffer)||(!sz)) return 20;
 SHA1_CONTEXT *hd=(SHA1_CONTEXT *)&ctx;

 u32 t, msb, lsb;
 byte *p;

 Compute(NULL,0); /* flush */;

 msb = 0;
 t=hd->nblocks;
 if ((lsb=t<<6)<t) /* multiply by 64 to make a byte count */
  msb++;
 msb+=t>>26;
 t=lsb;
 if ((lsb=t+hd->count)<t) /* add the count */
  msb++;
 t=lsb;
 if ((lsb=t<<3)<t) /* multiply by 8 to make a bit count */
  msb++;
 msb+=t>>29;

 if (hd->count<56)
 { /* enough room */
  hd->buf[hd->count++]=0x80; /* pad */
  while(hd->count<56)
  hd->buf[hd->count++]=0;  /* pad */
 }
 else
 { /* need one extra block */
  hd->buf[hd->count++]=0x80; /* pad character */
  while (hd->count<64)
  hd->buf[hd->count++]=0;
  Compute(NULL,0); /* flush */;
  memset(hd->buf,0,56); /* fill next block with zeroes */
 }
 /* append the 64 bit count */
 hd->buf[56]=msb>>24;
 hd->buf[57]=msb>>16;
 hd->buf[58]=msb>> 8;
 hd->buf[59]=msb;
 hd->buf[60]=lsb>> 24;
 hd->buf[61]=lsb>> 16;
 hd->buf[62]=lsb>>  8;
 hd->buf[63]=lsb;
 Transform(hd->buf);

 p=hd->buf;
 #ifdef BIG_ENDIAN
  #define X(a) do { *(u32*)p = hd->h##a ; p += 4; } while(0)
 #else /* little endian */
  #define X(a) do { *p++ = hd->h##a >> 24; *p++ = hd->h##a >> 16;  \
       *p++ = hd->h##a >> 8; *p++ = hd->h##a; } while(0)
 #endif
 X(0);
 X(1);
 X(2);
 X(3);
 X(4);
 #undef X

 if (sz>20) sz=20;
 if (buffer&&sz) memcpy(buffer,hd->buf,sz);
 return 20;
}

int Sha1::GetInputSize()
{
 return 64;
}

/*  Test vectors:
 *
 *  "abc"
 *  A999 3E36 4706 816A BA3E  2571 7850 C26C 9CD0 D89D
 *
 *  "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"
 *  8498 3E44 1C3B D26E BAAE  4AA1 F951 29E5 E546 70F1
 */


void Sha1::Transform(uchar *data)
{
 u32 tm;
 u32 x[16];
 u32 v[5];

 SHA1_CONTEXT *hd=(SHA1_CONTEXT *)&ctx;

 /* get values from the chaining vars */
 v[0]=hd->h0;
 v[1]=hd->h1;
 v[2]=hd->h2;
 v[3]=hd->h3;
 v[4]=hd->h4;

#ifdef BIG_ENDIAN
 memcpy( x, data, 64 );
#else
 {
  int i;
  byte *p2;
  for(i=0,p2=(byte*)x;i<16;i++,p2+=4 )
  {
   p2[3] = *data++;
   p2[2] = *data++;
   p2[1] = *data++;
   p2[0] = *data++;
  }
 }
#endif


#define K1  0x5A827999L
#define K2  0x6ED9EBA1L
#define K3  0x8F1BBCDCL
#define K4  0xCA62C1D6L
#define F1(x,y,z)   ( z ^ ( x & ( y ^ z ) ) )
#define F2(x,y,z)   ( x ^ y ^ z )
#define F3(x,y,z)   ( ( x & y ) | ( z & ( x | y ) ) )
#define F4(x,y,z)   ( x ^ y ^ z )


#define M(i) ( tm =   x[i&0x0f] ^ x[(i-14)&0x0f] \
      ^ x[(i-8)&0x0f] ^ x[(i-3)&0x0f] \
        , (x[i&0x0f] = (tm << 1) | (tm >> 31)) )

#define R(a,b,c,d,e,f,k,m)  do { e += rol( a, 5 )     \
          + f( b, c, d )  \
          + k       \
          + m;       \
     b = rol( b, 30 );    \
          } while(0)

 int i;
 for (i=0;i<16;i++)
 {
  R(v[0],v[1],v[2],v[3],v[4],F1,K1,x[i]);
  u32 s=v[4]; v[4]=v[3]; v[3]=v[2]; v[2]=v[1]; v[1]=v[0]; v[0]=s;
 }
 for (i=16;i<20;i++)
 {
  R(v[0],v[1],v[2],v[3],v[4],F1,K1,M(i));
  u32 s=v[4]; v[4]=v[3]; v[3]=v[2]; v[2]=v[1]; v[1]=v[0]; v[0]=s;
 }
 for (i=20;i<40;i++)
 {
  R(v[0],v[1],v[2],v[3],v[4],F2,K2,M(i));
  u32 s=v[4]; v[4]=v[3]; v[3]=v[2]; v[2]=v[1]; v[1]=v[0]; v[0]=s;
 }
 for (i=40;i<60;i++)
 {
  R(v[0],v[1],v[2],v[3],v[4],F3,K3,M(i));
  u32 s=v[4]; v[4]=v[3]; v[3]=v[2]; v[2]=v[1]; v[1]=v[0]; v[0]=s;
 }
 for (i=60;i<80;i++)
 {
  R(v[0],v[1],v[2],v[3],v[4],F4,K4,M(i));
  u32 s=v[4]; v[4]=v[3]; v[3]=v[2]; v[2]=v[1]; v[1]=v[0]; v[0]=s;
 }

 /* update chainig vars */
 hd->h0 += v[0];
 hd->h1 += v[1];
 hd->h2 += v[2];
 hd->h3 += v[3];
 hd->h4 += v[4];
}


void Sha1::Compute(const void *ibuf,int inlen)
{
 uchar *inbuf=(uchar *)ibuf;
 SHA1_CONTEXT *hd=(SHA1_CONTEXT *)&ctx;

 if( hd->count == 64 )
 { /* flush the buffer */
  Transform(hd->buf);
  hd->count = 0;
  hd->nblocks++;
 }
 if( !inbuf ) return;
 if( hd->count )
 {
  for(;inlen&&hd->count<64;inlen-- )
   hd->buf[hd->count++]=*inbuf++;
  Compute(NULL,0);
  if (!inlen) return;
 }

 while (inlen>=64)
 {
  Transform(inbuf);
  hd->count = 0;
  hd->nblocks++;
  inlen -= 64;
  inbuf += 64;
 }
 for(;inlen&&hd->count<64;inlen--)
  hd->buf[hd->count++]=*inbuf++;
}

// PBKDF2
void PBKDF2_F(const uchar *password, int passSize, const uchar *salt,
        int saltlength, int iterations, int count, uchar *output) {
    if (saltlength>128) return; //Not allowed
    unsigned char digest[128+4], digest1[20];
    int i, j;
    Sha1 sha1;
    int mo=sha1.GetOutputSize();
    /* U1 = PRF(P, S || int(i)) */
    memcpy(digest, salt, saltlength);
    digest[saltlength] = (unsigned char) ((count >> 24) & 0xff);
    digest[saltlength + 1] = (unsigned char) ((count >> 16) & 0xff);
    digest[saltlength + 2] = (unsigned char) ((count >> 8) & 0xff);
    digest[saltlength + 3] = (unsigned char) (count & 0xff);
    sha1.HMAC(digest, saltlength + 4, password, passSize, digest1, mo);
    /* output = U1 */
    memcpy(output, digest1, mo);
    for (i = 1; i < iterations; i++) {
        /* Un = PRF(P, Un-1) */
        sha1.HMAC(digest1, mo, password, passSize, digest, mo);
        memcpy(digest1, digest, mo);
        /* output = output xor Un */
        for (j = 0; j < mo; j++)
            output[j] ^= digest[j];
    }
}

// Crypto API
static int crypto_pbkdf2(lua_State *L) //String=pbkdf2(String password,String salt,int count,int outlen)
{
    size_t pw_sz,s_sz;
    const unsigned char *pw_in = (const unsigned char *) luaL_checklstring(L, 1, &pw_sz);
    const unsigned char *s_in = (const unsigned char *) luaL_checklstring(L, 2, &s_sz);
    int iter=luaL_checkinteger(L,3);
    int outLen=luaL_checkinteger(L,4);
    int isz=20; //SHA1
    if (s_sz>128) {
        lua_pushfstring(L,"Salt must be no more than 128 bytes long, got %zu bytes",s_sz);
        lua_error(L);
        return 0;
    }
    if (iter<1) {
        lua_pushfstring(L,"Iteration count must be strictly positive, %d given",iter);
        lua_error(L);
        return 0;
    }
    if (outLen<0) {
        lua_pushfstring(L,"Output length must be positive, %d given",outLen);
        lua_error(L);
        return 0;
    }
    int oPass=(outLen+isz-1)/isz;
    char *out=(char *) malloc(oPass*isz); //20 because of SHA1
    for (int i=0;i<oPass;i++)
        PBKDF2_F(pw_in, pw_sz, s_in, s_sz, iter, i+1, (uchar *) out+i*isz);
    lua_pushlstring(L,out,outLen);
    free(out);
    return 1;
}

static int crypto_sha1(lua_State *L) //String=sha1(String)
{
    size_t avail_in;
    const unsigned char *next_in = (const unsigned char *) luaL_checklstring(L, 1, &avail_in);
    unsigned char hres[20];
    Sha1 hash;
    hash.Compute(next_in,avail_in);
    hash.Result(hres, sizeof(hres));
    lua_pushlstring(L, (const char *) hres,sizeof(hres));
    return 1;
}

static int crypto_md5(lua_State *L) //String=md5(String)
{
    size_t avail_in;
    const unsigned char *next_in = (const unsigned char *) luaL_checklstring(L, 1, &avail_in);
    unsigned char md5res[16];
    md5_frombuffer(next_in,avail_in,md5res);
    lua_pushlstring(L, (const char *) md5res,16);
    return 1;
}

static int crypto_aes_encrypt(lua_State *L) //String=encrypt(String,String,String,bool)
{
    size_t avail_in,klen;
    const unsigned char *in = (const unsigned char *) luaL_checklstring(L, 1, &avail_in);
    const unsigned char *key = (const unsigned char *) luaL_checklstring(L, 2, &klen);
    const unsigned char *iv = (const unsigned char *) luaL_optstring(L, 3,NULL);
    bool mode=lua_toboolean(L,4);

    if (!((klen==16)||(klen==24)||(klen==32))) {
        lua_pushfstring(L,"Invalid Key length for AES:%zu",klen);
		lua_error(L);
    }

    size_t out_size=(avail_in+15)&(~15);
    if (mode&&iv) { //PKCS7 padding
    	if ((avail_in&15)==0) //room for extra 16 bytes
        	out_size+=16;
    }
    unsigned char *out=(unsigned char *) malloc(out_size);
	size_t rem=out_size-avail_in;
	memcpy(out,in,avail_in);
	memset(out+avail_in,(mode&&iv)?rem:0,rem);
    aes_encrypt(out,out_size,key,klen,iv);
    lua_pushlstring(L, (const char *)out,out_size);
    free(out);
    return 1;
}

static int crypto_aes_decrypt(lua_State *L) //String=encrypt(String,String,String,bool)
{
    size_t avail_in,klen;
    const unsigned char *in = (const unsigned char *) luaL_checklstring(L, 1, &avail_in);
    const unsigned char *key = (const unsigned char *) luaL_checklstring(L, 2, &klen);
    const unsigned char *iv = (const unsigned char *) luaL_optstring(L, 3,NULL);
    bool mode=lua_toboolean(L,4);

    if (!((klen==16)||(klen==24)||(klen==32))) {
        lua_pushfstring(L,"Invalid Key length for AES:%zu",klen);
		lua_error(L);
    }

    avail_in&=(~15);
    unsigned char *out=(unsigned char *) malloc(avail_in);
	memcpy(out,in,avail_in);
    aes_decrypt(out,avail_in,key,klen,iv);

    if (mode&&iv&&avail_in) //PKCS7: remove padding
    {
       unsigned char paddingval=out[avail_in-1];
       if ((paddingval==0)||(paddingval>0x10)) //Wrong padding byte
       {
   	    free(out);
		lua_pushstring(L,"Invalid PKCS#7 padding value");
		lua_error(L);
       }
       for (size_t p=avail_in-paddingval;p<avail_in;p++)
    	   if (out[p]!=paddingval)
    	   {
    	   	    free(out);
    			lua_pushstring(L,"Invalid PKCS#7 padding");
    			lua_error(L);
    	   }
       avail_in-=paddingval;
     }

    lua_pushlstring(L, (const char *)out,avail_in);
    free(out);
    return 1;
}

static const char * const Base64keyStr =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";

static int crypto_b64(lua_State *L) //String=encrypt(String)
{
	size_t size;
	const char *data=luaL_checklstring(L,1,&size);
	size_t bsz = ((size + 2) / 3) * 4;
	char *buffer = (char *) malloc(bsz);
	char *obuffer=buffer;

	size_t i = 0, osz = 0;
	char enc[4];
	unsigned char *bd=(unsigned char *)data;

	while (i < size) {
		unsigned int m = bd[i++];
		enc[0] = Base64keyStr[((m >> 2) & 0x3f)];
		if (i < size) {
			m = ((m & 0x03) << 8) | (bd[i++] & 0xFF);
			enc[1] = Base64keyStr[(m >> 4) & 0x3f];
			if (i < size) {
				m = ((m & 0x0F) << 8) | (bd[i++] & 0xFF);
				enc[2] = Base64keyStr[(m >> 6) & 0x3f];
				enc[3] = Base64keyStr[m & 0x3f];
			} else {
				enc[2] = Base64keyStr[(m << 2) & 0x3f];
				enc[3] = '=';
			}
		} else {
			enc[1] = Base64keyStr[(m << 4) & 0x3f];
			enc[2] = '=';
			enc[3] = '=';
		}
		int cn = bsz;
		if (cn > 4)
			cn = 4;
		memcpy(buffer, enc, cn);
		buffer += cn;
		bsz -= cn;
		osz += cn;
	}

	lua_pushlstring(L,obuffer,osz);
	free(obuffer);

	return 1;
}

static int crypto_unb64(lua_State *L) //String=encrypt(String)
{
	size_t size;
	const char *str=luaL_checklstring(L,1,&size);
	int bsz = ((size + 3) / 4) * 3;
	unsigned char *b = (unsigned char *) malloc(bsz);

	unsigned char chr[3];
	unsigned char *db=b;
	unsigned char enc[4];
	size_t k;
	int ocnt = 0;
	while (bsz && (*str)) {
		k = 0;
		while ((k < 4) && (*str))
			enc[k++] = strchr(Base64keyStr, *(str++)) - Base64keyStr;
		chr[0] = (enc[0] << 2) | (enc[1] >> 4);
		chr[1] = ((enc[1] & 15) << 4) | (enc[2] >> 2);
		chr[2] = ((enc[2] & 3) << 6) | enc[3];
		size_t ct = 3;
		if (enc[3] == 64)
			ct--;
		if (enc[2] == 64)
			ct--;
		k = 0;
		while (bsz && (k < ct)) {
			*(db++) = chr[k++];
			bsz--;
			ocnt++;
		}
	}

	lua_pushlstring(L,(char *)b,ocnt);
	free(b);

	return 1;
}

/*
** =========================================================================
** Register functions
** =========================================================================
*/

#if (LUA_VERSION_NUM >= 502)

#define luaL_register(L,n,f)	luaL_setfuncs(L,f,0)

#endif

void register_crypto(lua_State *L)
{
    const luaL_Reg crypto[] =
    {
            {"md5",             crypto_md5    },
            {"sha1",            crypto_sha1    },
            {"aesEncrypt",      crypto_aes_encrypt    },
	        {"aesDecrypt",      crypto_aes_decrypt    },
	        {"b64",      		crypto_b64    },
	        {"unb64",      		crypto_unb64    },
            {"pbkdf2",          crypto_pbkdf2    },

        {NULL, NULL}
    };

    lua_newtable(L);
    luaL_register(L, NULL, crypto);
    lua_setglobal(L, "Cryptography");
}
