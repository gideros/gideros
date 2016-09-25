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
	static int getTz(lua_State* L);

    static int setM11(lua_State* L);
    static int setM12(lua_State* L);
    static int setM21(lua_State* L);
    static int setM22(lua_State* L);
	static int setTx(lua_State* L);
	static int setTy(lua_State* L);
	static int setTz(lua_State* L);

	static int setElements(lua_State* L);
	static int getElements(lua_State* L);
	static int setMatrix(lua_State* L);
	static int getMatrix(lua_State* L);
	static int orthographicProjection(lua_State* L);
	static int perspectiveProjection(lua_State* L);
	static int scale(lua_State* L);
	static int translate(lua_State* L);
	static int rotate(lua_State* L);
	static int multiply(lua_State* L);
	static int transformPoint(lua_State* L);
	static int invert(lua_State* L);

	static int getX(lua_State* L);
	static int getY(lua_State* L);
	static int getZ(lua_State* L);
	static int getRotationZ(lua_State* L);
	static int getRotationX(lua_State* L);
	static int getRotationY(lua_State* L);
	static int getScaleX(lua_State* L);
	static int getScaleY(lua_State* L);
	static int getScaleZ(lua_State* L);
	static int setX(lua_State* L);
	static int setY(lua_State* L);
	static int setZ(lua_State* L);
	static int setRotationZ(lua_State* L);
	static int setRotationX(lua_State* L);
	static int setRotationY(lua_State* L);
	static int setScaleX(lua_State* L);
	static int setScaleY(lua_State* L);
	static int setScaleZ(lua_State* L);
	static int setPosition(lua_State* L);
	static int getPosition(lua_State* L);
    static int setAnchorPosition(lua_State* L);
    static int getAnchorPosition(lua_State* L);
	static int setScale(lua_State* L);
	static int getScale(lua_State* L);
};


#endif
