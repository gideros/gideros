#include "meshbinder.h"
#include <gmesh.h>
#include "luaapplication.h"
#include <luautil.h>

MeshBinder::MeshBinder(lua_State *L)
{
    Binder binder(L);

    static const luaL_Reg functionList[] = {
        {"setVertex", setVertex},
        {"setIndex", setIndex},
        {"setColor", setColor},
        {"setTextureCoordinate", setTextureCoordinate},

        {"setVertices", setVertices},
        {"setIndices", setIndices},
        {"setColors", setColors},
        {"setTextureCoordinates", setTextureCoordinates},

        {"setVertexArray", setVertexArray},
        {"setIndexArray", setIndexArray},
        {"setColorArray", setColorArray},
        {"setTextureCoordinateArray", setTextureCoordinateArray},
        {"setGenericArray", setGenericArray},

        {"resizeVertexArray", resizeVertexArray},
        {"resizeIndexArray", resizeIndexArray},
        {"resizeColorArray", resizeColorArray},
        {"resizeTextureCoordinateArray", resizeTextureCoordinateArray},

        {"clearVertexArray", clearVertexArray},
        {"clearIndexArray", clearIndexArray},
        {"clearColorArray", clearColorArray},
        {"clearTextureCoordinateArray", clearTextureCoordinateArray},

        {"getVertexArraySize", getVertexArraySize},
        {"getIndexArraySize", getIndexArraySize},
        {"getColorArraySize", getColorArraySize},
        {"getTextureCoordinateArraySize", getTextureCoordinateArraySize},

        {"getVertex", getVertex},
        {"getIndex", getIndex},
        {"getColor", getColor},
        {"getTextureCoordinate", getTextureCoordinate},

        {"setTexture", setTexture},
        {"clearTexture", clearTexture},
        {"setPrimitiveType", setPrimitiveType},

        {NULL, NULL},
    };

    binder.createClass("Mesh", "Sprite", create, destruct, functionList);

    lua_getglobal(L, "Mesh");

    lua_pushinteger(L, ShaderProgram::Point);
    lua_setfield(L, -2, "PRIMITIVE_POINT");
    lua_pushinteger(L, ShaderProgram::Lines);
    lua_setfield(L, -2, "PRIMITIVE_LINES");
    lua_pushinteger(L, ShaderProgram::LineLoop);
    lua_setfield(L, -2, "PRIMITIVE_LINELOOP");
    lua_pushinteger(L, ShaderProgram::Triangles);
    lua_setfield(L, -2, "PRIMITIVE_TRIANGLES");
    lua_pushinteger(L, ShaderProgram::TriangleFan);
    lua_setfield(L, -2, "PRIMITIVE_TRIANGLEFAN");
    lua_pushinteger(L, ShaderProgram::TriangleStrip);
    lua_setfield(L, -2, "PRIMITIVE_TRIANGLESTRIP");

    lua_pop(L, 1);
}

int MeshBinder::create(lua_State *L)
{
    LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

    bool is3d = lua_toboolean(L, 1);

    Binder binder(L);

    binder.pushInstance("Mesh", new GMesh(application->getApplication(),is3d));

    return 1;
}

int MeshBinder::destruct(lua_State *L)
{
    void* ptr = *(void**)lua_touserdata(L, 1);
    GMesh* mesh = static_cast<GMesh*>(ptr);
    mesh->unref();

    return 0;
}

int MeshBinder::setVertex(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    int i = luaL_checkinteger(L, 2) - 1;
    float x = luaL_checknumber(L, 3);
    float y = luaL_checknumber(L, 4);
    float z = luaL_optnumber(L, 5, 0.0);

    mesh->setVertex(i, x, y, z);

    return 0;
}

int MeshBinder::setIndex(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    int i = luaL_checkinteger(L, 2) - 1;
    int index = luaL_checkinteger(L, 3) - 1;

    mesh->setIndex(i, index);

    return 0;
}

int MeshBinder::setColor(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    int i = luaL_checkinteger(L, 2) - 1;
    unsigned int color = luaL_checkinteger(L, 3);
    float alpha = luaL_optnumber(L, 4, 1.0);

    mesh->setColor(i, color, alpha);

    return 0;
}

int MeshBinder::setTextureCoordinate(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    int i = luaL_checkinteger(L, 2) - 1;
    float u = luaL_checknumber(L, 3);
    float v = luaL_checknumber(L, 4);

    mesh->setTextureCoordinate(i, u, v);

    return 0;
}

int MeshBinder::setVertices(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));
    
    bool is3d=mesh->is3d();
    int order=is3d?4:3;

    if (lua_type(L, 2) == LUA_TTABLE)
    {
        int n = lua_objlen(L, 2);
        for (int k = 0; k < n/order; ++k)
        {
            lua_rawgeti(L, 2, k * order + 1);
            int i = luaL_checkinteger(L, -1) - 1;
            lua_pop(L, 1);

            lua_rawgeti(L, 2, k * order + 2);
            float x = luaL_checknumber(L, -1);
            lua_pop(L, 1);

            lua_rawgeti(L, 2, k * order + 3);
            float y = luaL_checknumber(L, -1);
            lua_pop(L, 1);

            float z=0;
            if (is3d)
            {
            	lua_rawgeti(L, 2, k * order + 4);
            	z = luaL_checknumber(L, -1);
            	lua_pop(L, 1);
            }
            mesh->setVertex(i, x, y, z);
        }
    }
    else
    {
        int n = lua_gettop(L) - 1;
        for (int k = 0; k < n/order; ++k)
        {
            int i = luaL_checkinteger(L, k * order + 2) - 1;
            float x = luaL_checknumber(L, k * order + 3);
            float y = luaL_checknumber(L, k * order + 4);
            float z=0;
            if (is3d) 
            	z= luaL_checknumber(L, k * order + 5);
            mesh->setVertex(i, x, y, z);
        }
    }

    return 0;
}

int MeshBinder::setIndices(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    if (lua_type(L, 2) == LUA_TTABLE)
    {
        int n = lua_objlen(L, 2);
        for (int k = 0; k < n/2; ++k)
        {
            lua_rawgeti(L, 2, k * 2 + 1);
            int i = luaL_checkinteger(L, -1) - 1;
            lua_pop(L, 1);

            lua_rawgeti(L, 2, k * 2 + 2);
            int index = luaL_checknumber(L, -1) - 1;
            lua_pop(L, 1);

            mesh->setIndex(i, index);
        }
    }
    else
    {
        int n = lua_gettop(L) - 1;
        for (int k = 0; k < n/2; ++k)
        {
            int i = luaL_checkinteger(L, k * 2 + 2) - 1;
            int index = luaL_checknumber(L, k * 2 + 3) - 1;

            mesh->setIndex(i, index);
        }
    }

    return 0;
}

int MeshBinder::setColors(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    if (lua_type(L, 2) == LUA_TTABLE)
    {
        int n = lua_objlen(L, 2);
        for (int k = 0; k < n/3; ++k)
        {
            lua_rawgeti(L, 2, k * 3 + 1);
            int i = luaL_checkinteger(L, -1) - 1;
            lua_pop(L, 1);

            lua_rawgeti(L, 2, k * 3 + 2);
            unsigned int color = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_rawgeti(L, 2, k * 3 + 3);
            float alpha = luaL_checknumber(L, -1);
            lua_pop(L, 1);

            mesh->setColor(i, color, alpha);
        }
    }
    else
    {
        int n = lua_gettop(L) - 1;
        for (int k = 0; k < n/3; ++k)
        {
            int i = luaL_checkinteger(L, k * 3 + 2) - 1;
            unsigned int color = luaL_checkinteger(L, k * 3 + 3);
            float alpha = luaL_checknumber(L, k * 3 + 4);

            mesh->setColor(i, color, alpha);
        }
    }

    return 0;
}

int MeshBinder::setTextureCoordinates(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    if (lua_type(L, 2) == LUA_TTABLE)
    {
        int n = lua_objlen(L, 2);
        for (int k = 0; k < n/3; ++k)
        {
            lua_rawgeti(L, 2, k * 3 + 1);
            int i = luaL_checkinteger(L, -1) - 1;
            lua_pop(L, 1);

            lua_rawgeti(L, 2, k * 3 + 2);
            float u = luaL_checknumber(L, -1);
            lua_pop(L, 1);

            lua_rawgeti(L, 2, k * 3 + 3);
            float v = luaL_checknumber(L, -1);
            lua_pop(L, 1);

            mesh->setTextureCoordinate(i, u, v);
        }
    }
    else
    {
        int n = lua_gettop(L) - 1;
        for (int k = 0; k < n/3; ++k)
        {
            int i = luaL_checkinteger(L, k * 3 + 2) - 1;
            float u = luaL_checknumber(L, k * 3 + 3);
            float v = luaL_checknumber(L, k * 3 + 4);

            mesh->setTextureCoordinate(i, u, v);
        }
    }

    return 0;
}

int MeshBinder::setGenericArray(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));
    int index=luaL_checkinteger(L,2);
    ShaderProgram::DataType type=(ShaderProgram::DataType) luaL_checkinteger(L,3);
    int mult=luaL_checkinteger(L,4);
    int count=luaL_checkinteger(L,5);
    luaL_checktype(L, 6, LUA_TTABLE);

    int n = luaL_getn(L, 6);  /* get size of table */
    if (n!=(mult*count))
    {
    	lua_pushstring(L,"Actual array length doesn't match size multiple and count values");
    	lua_error(L);
    }

    void *ptr = 0;
	switch (type)
	{
	case ShaderProgram::DBYTE:
	case ShaderProgram::DUBYTE:
	{
	    char *p=(char *) malloc(mult*count);
	    ptr=p;
	    for (int i=1; i<=n; i++) {
	        lua_rawgeti(L, 6, i);  /* push t[i] */
	        *(p++)=luaL_checkinteger(L,-1);
	        lua_pop(L,1);
	      }
		break;
	}
	case ShaderProgram::DSHORT:
	case ShaderProgram::DUSHORT:
	{
	    short *p=(short *) malloc(mult*count*2);
	    ptr=p;
	    for (int i=1; i<=n; i++) {
	        lua_rawgeti(L, 6, i);  /* push t[i] */
	        *(p++)=luaL_checkinteger(L,-1);
	        lua_pop(L,1);
	      }
		break;
	}
	case ShaderProgram::DINT:
	{
		int *p=(int *) malloc(mult*count*4);
	    ptr=p;
	    for (int i=1; i<=n; i++) {
	        lua_rawgeti(L, 6, i);  /* push t[i] */
	        *(p++)=luaL_checkinteger(L,-1);
	        lua_pop(L,1);
	      }
		break;
	}
	case ShaderProgram::DFLOAT:
	{
	    float *p=(float *) malloc(mult*count*4);
	    ptr=p;
	    for (int i=1; i<=n; i++) {
	        lua_rawgeti(L, 6, i);  /* push t[i] */
	        *(p++)=luaL_checknumber(L,-1);
	        lua_pop(L,1);
	      }
		break;
	}
	}

    mesh->setGenericArray(index,ptr,type,mult,count);
	free(ptr);
	return 0;
}

int MeshBinder::setVertexArray(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    std::vector<float> vertices;
    
    int order=mesh->is3d()?3:2;

    if (lua_type(L, 2) == LUA_TTABLE)
    {
        int n = lua_objlen(L, 2);
        n = (n / order) * order;
        vertices.resize(n);
        for (int i = 0; i < n; ++i)
        {
            lua_rawgeti(L, 2, i + 1);
            vertices[i] = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
    }
    else
    {
        int n = lua_gettop(L) - 1;
        n = (n / order) * order;
        vertices.resize(n);
        for (int i = 0; i < n; ++i)
            vertices[i] = luaL_checknumber(L, i + 2);
    }

	if (vertices.size() > 0)
		mesh->setVertexArray(&vertices[0], vertices.size());
	else
		mesh->clearVertexArray();

    return 0;
}

int MeshBinder::setIndexArray(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    std::vector<unsigned short> indices;

    if (lua_type(L, 2) == LUA_TTABLE)
    {
        int n = lua_objlen(L, 2);
        indices.resize(n);
        for (int i = 0; i < n; ++i)
        {
            lua_rawgeti(L, 2, i + 1);
            indices[i] = luaL_checkinteger(L, -1) - 1;
            lua_pop(L, 1);
        }
    }
    else
    {
        int n = lua_gettop(L) - 1;
        indices.resize(n);
        for (int i = 0; i < n; ++i)
            indices[i] = luaL_checkinteger(L, i + 2) - 1;
    }

	if (indices.size() > 0)
		mesh->setIndexArray(&indices[0], indices.size());
	else
		mesh->clearIndexArray();

    return 0;
}

int MeshBinder::setColorArray(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    std::vector<unsigned int> colors;
    std::vector<float> alphas;

    if (lua_type(L, 2) == LUA_TTABLE)
    {
        int n = lua_objlen(L, 2);
        n /= 2;
        colors.resize(n);
        alphas.resize(n);
        for (int i = 0; i < n; ++i)
        {
            lua_rawgeti(L, 2, i * 2 + 1);
            colors[i] = luaL_checkinteger(L, -1);
            lua_pop(L, 1);

            lua_rawgeti(L, 2, i * 2 + 2);
            alphas[i] = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
    }
    else
    {
        int n = lua_gettop(L) - 1;
        n /= 2;
        colors.resize(n);
        alphas.resize(n);
        for (int i = 0; i < n; ++i)
        {
            colors[i] = luaL_checkinteger(L, i * 2 + 2);
            alphas[i] = luaL_checknumber(L, i * 2 + 3);
        }
    }

	if (colors.size() > 0)
		mesh->setColorArray(&colors[0], &alphas[0], colors.size());
	else
		mesh->clearColorArray();

    return 0;
}

int MeshBinder::setTextureCoordinateArray(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    std::vector<float> textureCoordinates;

    if (lua_type(L, 2) == LUA_TTABLE)
    {
        int n = lua_objlen(L, 2);
        n = (n / 2) * 2;
        textureCoordinates.resize(n);
        for (int i = 0; i < n; ++i)
        {
            lua_rawgeti(L, 2, i + 1);
            textureCoordinates[i] = luaL_checknumber(L, -1);
            lua_pop(L, 1);
        }
    }
    else
    {
        int n = lua_gettop(L) - 1;
        n = (n / 2) * 2;
        textureCoordinates.resize(n);
        for (int i = 0; i < n; ++i)
            textureCoordinates[i] = luaL_checknumber(L, i + 2);
    }

	if (textureCoordinates.size()>0)
		mesh->setTextureCoordinateArray(&textureCoordinates[0], textureCoordinates.size());
	else
		mesh->clearTextureCoordinateArray();

    return 0;
}


int MeshBinder::resizeVertexArray(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    mesh->resizeVertexArray(luaL_checkinteger(L, 2));

    return 0;
}

int MeshBinder::resizeIndexArray(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    mesh->resizeIndexArray(luaL_checkinteger(L, 2));

    return 0;
}

int MeshBinder::resizeColorArray(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    mesh->resizeColorArray(luaL_checkinteger(L, 2));

    return 0;
}

int MeshBinder::resizeTextureCoordinateArray(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    mesh->resizeTextureCoordinateArray(luaL_checkinteger(L, 2));

    return 0;
}


int MeshBinder::clearVertexArray(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    mesh->clearVertexArray();

    return 0;
}

int MeshBinder::clearIndexArray(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    mesh->clearIndexArray();

    return 0;
}

int MeshBinder::clearColorArray(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    mesh->clearColorArray();

    return 0;
}

int MeshBinder::clearTextureCoordinateArray(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    mesh->clearTextureCoordinateArray();

    return 0;
}

int MeshBinder::getVertexArraySize(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    lua_pushinteger(L, mesh->getVertexArraySize());

    return 1;
}

int MeshBinder::getIndexArraySize(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    lua_pushinteger(L, mesh->getIndexArraySize());

    return 1;
}

int MeshBinder::getColorArraySize(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    lua_pushinteger(L, mesh->getColorArraySize());

    return 1;
}

int MeshBinder::getTextureCoordinateArraySize(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));

    lua_pushinteger(L, mesh->getTextureCoordinateArraySize());

    return 1;
}

int MeshBinder::getVertex(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));
    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getVertexArraySize())
        return luaL_error(L, "The supplied index is out of bounds.");

    int order=mesh->is3d()?3:2;
    float x, y, z;
    mesh->getVertex(i, &x, &y, &z);
    lua_pushnumber(L, x);
    lua_pushnumber(L, y);
    if (order==3)
    	lua_pushnumber(L, z);

    return order;
}

int MeshBinder::getIndex(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));
    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getIndexArraySize())
        return luaL_error(L, "The supplied index is out of bounds.");

    unsigned short index;
    mesh->getIndex(i, &index);
    lua_pushinteger(L, (int)index + 1);

    return 1;
}

int MeshBinder::getColor(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));
    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getColorArraySize())
        return luaL_error(L, "The supplied index is out of bounds.");

    unsigned int color;
    float alpha;
    mesh->getColor(i, &color, &alpha);
    lua_pushinteger(L, color);
    lua_pushnumber(L, alpha);

    return 2;
}

int MeshBinder::getTextureCoordinate(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));
    int i = luaL_checkinteger(L, 2) - 1;

    if (i < 0 || i >= mesh->getTextureCoordinateArraySize())
        return luaL_error(L, "The supplied index is out of bounds.");

    float u, v;
    mesh->getTextureCoordinate(i, &u, &v);
    lua_pushnumber(L, u);
    lua_pushnumber(L, v);

    return 2;
}

int MeshBinder::setPrimitiveType(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));
    ShaderProgram::ShapeType prim=(ShaderProgram::ShapeType) luaL_checkinteger(L,2);

    mesh->setPrimitiveType(prim);

    return 0;
}

int MeshBinder::setTexture(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));
    TextureBase* textureBase = static_cast<TextureBase*>(binder.getInstance("TextureBase", 2));
    int slot=luaL_optinteger(L,3,0);

    mesh->setTexture(textureBase,slot);

    return 0;
}

int MeshBinder::clearTexture(lua_State *L)
{
    Binder binder(L);
    GMesh *mesh = static_cast<GMesh*>(binder.getInstance("Mesh", 1));
    int slot=luaL_optinteger(L,2,0);

    mesh->clearTexture(slot);

    return 0;
}
