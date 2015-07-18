#ifndef MATRIXBINDER_H
#define MATRIXBINDER_H

#include "binder.h"

class MatrixBinder
{
public:
	MatrixBinder(lua_State* L);

private:
	static int create(lua_State* L);
	static int destruct(lua_State* L);

    static int getM11(lua_State* L);
    static int getM12(lua_State* L);
    static int getM21(lua_State* L);
    static int getM22(lua_State* L);
	static int getTx(lua_State* L);
	static int getTy(lua_State* L);

    static int setM11(lua_State* L);
    static int setM12(lua_State* L);
    static int setM21(lua_State* L);
    static int setM22(lua_State* L);
	static int setTx(lua_State* L);
	static int setTy(lua_State* L);

	static int setElements(lua_State* L);
	static int getElements(lua_State* L);
};


#endif
