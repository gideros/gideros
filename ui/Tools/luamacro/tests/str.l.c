// preprocess using luam -C -llc -o str.c str.l.c
#include <string.h>

module "str" {

  def at (Str s, Int i = 0) {
    lua_pushlstring(L,&s[i-1],1);
    return 1;
  }

  def upto (Str s, Str delim = " ") {
    lua_pushinteger(L, strcspn(s,delim) + 1);
    return 1;
  }

}



