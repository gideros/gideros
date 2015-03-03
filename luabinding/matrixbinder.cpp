#include "matrixbinder.h"
#include "stackchecker.h"
#include <matrix.h>

MatrixBinder::MatrixBinder(lua_State* L)
{
	Binder binder(L);
	
	static const luaL_Reg functionList[] = {
        {"getM11", getM11},
        {"getM12", getM12},
        {"getM21", getM21},
        {"getM22", getM22},
		{"getTx", getTx},
		{"getTy", getTy},

        {"setM11", setM11},
        {"setM12", setM12},
        {"setM21", setM21},
        {"setM22", setM22},
		{"setTx", setTx},
		{"setTy", setTy},

		{"setElements", setElements},
		{"getElements", getElements},

		{NULL, NULL},
	};

	binder.createClass("Matrix", NULL, create, destruct, functionList);
}

int MatrixBinder::create(lua_State* L)
{
	StackChecker checker(L, "MatrixBinder::create", 1);

	Binder binder(L);

	lua_Number m11 = luaL_optnumber(L, 1, 1);
	lua_Number m12 = luaL_optnumber(L, 2, 0);
	lua_Number m21 = luaL_optnumber(L, 3, 0);
	lua_Number m22 = luaL_optnumber(L, 4, 1);
	lua_Number tx = luaL_optnumber(L, 5, 0);
	lua_Number ty = luaL_optnumber(L, 6, 0);

    binder.pushInstance("Matrix", new Matrix2D(m11, m12, m21, m22, tx, ty));

	return 1;
}

int MatrixBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	Matrix2D* matrix = static_cast<Matrix2D*>(ptr);
	delete matrix;

	return 0;
}


int MatrixBinder::getM11(lua_State* L)
{
	Binder binder(L);
	Matrix2D* matrix = static_cast<Matrix2D*>(binder.getInstance("Matrix", 1));

    lua_pushnumber(L, matrix->m11());

	return 1;
}
int MatrixBinder::getM12(lua_State* L)
{
	Binder binder(L);
	Matrix2D* matrix = static_cast<Matrix2D*>(binder.getInstance("Matrix", 1));

    lua_pushnumber(L, matrix->m12());

	return 1;
}
int MatrixBinder::getM21(lua_State* L)
{
	Binder binder(L);
	Matrix2D* matrix = static_cast<Matrix2D*>(binder.getInstance("Matrix", 1));

    lua_pushnumber(L, matrix->m21());

	return 1;
}
int MatrixBinder::getM22(lua_State* L)
{
	Binder binder(L);
	Matrix2D* matrix = static_cast<Matrix2D*>(binder.getInstance("Matrix", 1));

    lua_pushnumber(L, matrix->m22());

	return 1;
}
int MatrixBinder::getTx(lua_State* L)
{
	Binder binder(L);
	Matrix2D* matrix = static_cast<Matrix2D*>(binder.getInstance("Matrix", 1));

	lua_pushnumber(L, matrix->tx());

	return 1;
}
int MatrixBinder::getTy(lua_State* L)
{
	Binder binder(L);
	Matrix2D* matrix = static_cast<Matrix2D*>(binder.getInstance("Matrix", 1));

	lua_pushnumber(L, matrix->ty());

	return 1;
}


int MatrixBinder::setM11(lua_State* L)
{
	Binder binder(L);
	Matrix2D* matrix = static_cast<Matrix2D*>(binder.getInstance("Matrix", 1));

    matrix->setM11(luaL_checknumber(L, 2));

	return 0;
}
int MatrixBinder::setM12(lua_State* L)
{
	Binder binder(L);
	Matrix2D* matrix = static_cast<Matrix2D*>(binder.getInstance("Matrix", 1));

    matrix->setM12(luaL_checknumber(L, 2));

	return 0;
}
int MatrixBinder::setM21(lua_State* L)
{
	Binder binder(L);
	Matrix2D* matrix = static_cast<Matrix2D*>(binder.getInstance("Matrix", 1));

    matrix->setM21(luaL_checknumber(L, 2));

	return 0;
}
int MatrixBinder::setM22(lua_State* L)
{
	Binder binder(L);
	Matrix2D* matrix = static_cast<Matrix2D*>(binder.getInstance("Matrix", 1));

    matrix->setM22(luaL_checknumber(L, 2));

	return 0;
}
int MatrixBinder::setTx(lua_State* L)
{
	Binder binder(L);
	Matrix2D* matrix = static_cast<Matrix2D*>(binder.getInstance("Matrix", 1));

	matrix->setTx(luaL_checknumber(L, 2));

	return 0;
}
int MatrixBinder::setTy(lua_State* L)
{
	Binder binder(L);
	Matrix2D* matrix = static_cast<Matrix2D*>(binder.getInstance("Matrix", 1));

	matrix->setTy(luaL_checknumber(L, 2));

	return 0;
}

int MatrixBinder::getElements(lua_State* L)
{
	Binder binder(L);
	Matrix2D* matrix = static_cast<Matrix2D*>(binder.getInstance("Matrix", 1));

    lua_pushnumber(L, matrix->m11());
    lua_pushnumber(L, matrix->m12());
    lua_pushnumber(L, matrix->m21());
    lua_pushnumber(L, matrix->m22());
	lua_pushnumber(L, matrix->tx());
	lua_pushnumber(L, matrix->ty());

	return 6;
}

int MatrixBinder::setElements(lua_State* L)
{
	Binder binder(L);
	Matrix2D* matrix = static_cast<Matrix2D*>(binder.getInstance("Matrix", 1));

	lua_Number m11 = luaL_optnumber(L, 2, 1);
	lua_Number m12 = luaL_optnumber(L, 3, 0);
	lua_Number m21 = luaL_optnumber(L, 4, 0);
	lua_Number m22 = luaL_optnumber(L, 5, 1);
	lua_Number tx = luaL_optnumber(L, 6, 0);
	lua_Number ty = luaL_optnumber(L, 7, 0);

    matrix->set(m11, m12, m21, m22, tx, ty);

	return 0;
}
