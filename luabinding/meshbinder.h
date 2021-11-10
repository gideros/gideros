#ifndef MESHBINDER_H
#define MESHBINDER_H

#include "binder.h"

class MeshBinder
{
public:
    MeshBinder(lua_State *L);

private:
    static int create(lua_State *L);
    static int destruct(lua_State *L);

    static int setVertex(lua_State *L);
    static int setIndex(lua_State *L);
    static int setColor(lua_State *L);
    static int setTextureCoordinate(lua_State *L);

    static int setVertices(lua_State *L);
    static int setIndices(lua_State *L);
    static int setColors(lua_State *L);
    static int setTextureCoordinates(lua_State *L);

    static int setVertexArray(lua_State *L);
    static int setIndexArray(lua_State *L);
    static int setColorArray(lua_State *L);
    static int setTextureCoordinateArray(lua_State *L);
    static int setGenericArray(lua_State *L);

    static int resizeVertexArray(lua_State *L);
    static int resizeIndexArray(lua_State *L);
    static int resizeColorArray(lua_State *L);
    static int resizeTextureCoordinateArray(lua_State *L);

    static int clearVertexArray(lua_State *L);
    static int clearIndexArray(lua_State *L);
    static int clearColorArray(lua_State *L);
    static int clearTextureCoordinateArray(lua_State *L);

    static int getVertexArraySize(lua_State *L);
    static int getIndexArraySize(lua_State *L);
    static int getColorArraySize(lua_State *L);
    static int getTextureCoordinateArraySize(lua_State *L);

    static int getVertex(lua_State *L);
    static int getIndex(lua_State *L);
    static int getColor(lua_State *L);
    static int getTextureCoordinate(lua_State *L);

    static int setTexture(lua_State *L);
    static int clearTexture(lua_State *L);
    static int setTextureSlot(lua_State *L);
    static int setPrimitiveType(lua_State *L);
    static int setInstanceCount(lua_State *L);
    static int setCullMode(lua_State *L);
};


#endif
