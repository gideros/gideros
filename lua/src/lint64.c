/*
* lint64.c
* int64 nummbers for Lua
* Luiz Henrique de Figueiredo <lhf@tecgraf.puc-rio.br>
* 05 Aug 2013 21:04:25
* This code is hereby placed in the public domain.
*/

#include <stdlib.h>
#include <errno.h>

#define Int		long long
#define FMT		"%lld"
#define atoI		atoll

#include "lua.h"
#include "lauxlib.h"

#define MYNAME		"int64"
#define MYTYPE		MYNAME
#define MYVERSION	MYTYPE " library for " LUA_VERSION " / Aug 2013"

#define Z(i)		Pget(L,i)
#define	I(x)		((Int)x)

static Int Pget(lua_State *L, int i)
{
 switch (lua_type(L,i))
 {
  case LUA_TNUMBER:
   return luaL_checknumber(L,i);
  case LUA_TSTRING:
   return atoI(luaL_checkstring(L,i));
  default:
   return *((Int*)luaL_checkudata(L,i,MYTYPE));
 }
}

static int pushInt(lua_State *L, Int z)
{
 Int *p=(Int*)lua_newuserdata(L,sizeof(Int));
 *p=z;
 luaL_getmetatable(L,MYTYPE);
 lua_setmetatable(L,-2);
 return 1;
}

static int Lpow(lua_State *L)			/** __pow(z,n) */
{
 Int z=Z(1);
 Int n=Z(2);
 Int r;
 if (z==I(2))
  r= (n>=8*sizeof(Int)) ? 0 : (I(1)<<n);
 else
 {
  for (r=1; n>0; n>>=1)
  {
   if (n&1) r*=z;
   z*=z;
  }
 }
 return pushInt(L,r);
}

static int Ltostring(lua_State *L)		/** __tostring(z) */
{
 char b[64];
 sprintf(b,FMT,Z(1));
 lua_pushstring(L,b);
 return 1;
}

static int Ltonumber(lua_State *L)		/** tonumber(z) */
{
 lua_pushnumber(L,(lua_Number)Z(1));
 return 1;
}

static int Lcall(lua_State *L)
{
 char b[64];
 switch (lua_type(L,2))
 {
  case LUA_TSTRING:
   sprintf(b,FMT,Z(1));
   lua_pushstring(L,b);
   break;
  default:
   lua_pushnumber(L,(lua_Number)Z(1));
 }
 return 1;
}

static int Lnind(lua_State *L)  /** _newindex(z, x) */
{
 Int z=Z(1);
 Int i=Z(2);
 Int v=Z(3);
 Int *p =(Int*) lua_touserdata(L, 1);
 *p = (z ^ (-v ^ z) & (1 << i));
 return 0;
}

#define add(z,w)	((z)+(w))
#define sub(z,w)	((z)-(w))
#define mod(z,w)	((z)%(w))
#define mul(z,w)	((z)*(w))
#define div(z,w)	((z)/(w))
#define neg(z)		(-(z))
#define new(z)		(z)
#define eq(z,w)		((z)==(w))
#define le(z,w)		((z)<=(w))
#define lt(z,w)		((z)<(w))
#define idiv(z,w)	(floor((z)/(w)))
#define band(z,w)	((z)&(w))
#define bor(z,w)	((z)|(w))
#define bxor(z,w)	((z)^(w))
#define bnot(z)         (~(z))
#define shl(z,w)	((z)<<(w))
#define shr(z,w)	((z)>>(w))
#define ind(z,w)        (((z)>>(w))&1)

#define A(f,e)	static int L##f(lua_State *L) { return pushInt(L,e); }
#define B(f)	A(f,f(Z(1),Z(2)))
#define F(f)	A(f,f(Z(1)))
#define T(f)	C(f,f(Z(1),Z(2)))
#define C(f,e)	static int L##f(lua_State *L) { lua_pushboolean(L,e); return 1; }

B(add)			/** __add(z,w) */
B(div)			/** __div(z,w) */
B(mod)			/** __mod(z,w) */
B(mul)			/** __mul(z,w) */
B(sub)			/** __sub(z,w) */
F(neg)			/** __unm(z) */
F(new)			/** new(z) */
T(eq)			/** __eq(z,w) */
T(le)			/** __le(z,w) */
T(lt)			/** __lt(z,w) */
B(idiv)         /** __idiv(z,w) */
B(band)         /** __band(z,w) */
B(bor)          /** __bor(z,w) */
B(bxor)         /** __bxor(z,w) */
F(bnot)         /** __unm(z) */
B(shl)          /** __shl(z,w) */
B(shr)          /** __shr(z,w) */
B(ind)          /** __index(z,w) */

static const luaL_Reg R[] =
{
	{ "__add",	    Ladd	},
	{ "__div",	    Ldiv	},
	{ "__eq",	    Leq		},
	{ "__le",	    Lle		},
	{ "__lt",	    Llt		},
	{ "__mod",	    Lmod	},
	{ "__mul",	    Lmul	},
	{ "__pow",	    Lpow	},
	{ "__sub",	    Lsub	},
	{ "__unm",	    Lneg	},
    { "__idiv",     Lidiv   },
	{ "__band",     Lband   },
	{ "__bor",      Lbor    },
	{ "__bxor",     Lbxor   },
	{ "__bnot",     Lbnot   },
	{ "__shl",      Lshl    },
	{ "__shr",      Lshr    },
	{ "__tostring",	Ltostring},
	{ "new",	Lnew	},
	{ "tonumber",	Ltonumber},
	{ "__index",    Lind    },
	{ "__newindex", Lnind   },
	{ "__call",     Lcall   },
	{ NULL,		NULL	}
};

LUALIB_API int luaopen_int64(lua_State *L)
{
 if (sizeof(Int)<8) luaL_error(L,"int64 cannot work with %d-byte values",sizeof(Int));

 luaL_newmetatable(L,MYTYPE);
 lua_setglobal(L,MYNAME);
 luaL_register(L,MYNAME,R);

 lua_createtable(L, 0, 1);  /* table to be type metatable for numbers */
 lua_pushvalue(L, -1);      /* copy table */
 lua_settypemt(L, LUA_TNUMBER);   /* set table as type metatable for numbers */
 lua_pushcfunction(L, Lnew);
 lua_setfield(L, -2, "__len");  /* metatable.__len = function */
 lua_pop(L, 1);

 return 1;
}
