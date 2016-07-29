#ifndef TEXTFIELDBINDER_H
#define TEXTFIELDBINDER_H

#include "binder.h"

class TextFieldBinder
{
public:
	TextFieldBinder(lua_State* L);
	
private:
	static int create(lua_State* L);
	static int destruct(lua_State* L);

    static int setFont(lua_State* L);

	static int getText(lua_State* L);
	static int setText(lua_State* L);

	static int getTextColor(lua_State* L);
	static int setTextColor(lua_State* L);

	static int getLetterSpacing(lua_State* L);
	static int setLetterSpacing(lua_State* L);

    static int getLineHeight(lua_State* L);

    static int getSample(lua_State* L);
    static int setSample(lua_State* L);
};

#endif
