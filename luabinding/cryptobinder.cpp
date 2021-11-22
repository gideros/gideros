#include <stdlib.h>
#include <string.h>
#include "platformutil.h"
#include "lua.hpp"

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
		lua_pushfstring(L,"Invalid Key length for AES:%d",klen);
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
		lua_pushfstring(L,"Invalid Key length for AES:%d",klen);
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
            {"md5",      crypto_md5    },
	        {"aesEncrypt",      crypto_aes_encrypt    },
	        {"aesDecrypt",      crypto_aes_decrypt    },
	        {"b64",      		crypto_b64    },
	        {"unb64",      		crypto_unb64    },

        {NULL, NULL}
    };

    lua_newtable(L);
    luaL_register(L, NULL, crypto);
    lua_setglobal(L, "Cryptography");
}
