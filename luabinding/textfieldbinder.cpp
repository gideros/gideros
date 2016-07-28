#include "textfieldbinder.h"
#include "textfield.h"
#include "tttextfield.h"
#include "font.h"
#include "ttfont.h"
#include "stackchecker.h"
#include "luaapplication.h"
#include "application.h"
#include <luautil.h>


TextFieldBinder::TextFieldBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
        {"setFont", setFont},
		{"getText", getText},
		{"setText", setText},
		{"getTextColor", getTextColor},
		{"setTextColor", setTextColor},
		{"getLetterSpacing", getLetterSpacing},
		{"setLetterSpacing", setLetterSpacing},
        {"getLineHeight", getLineHeight},
        {"getSample", getSample},
        {"setSample", setSample},
		{NULL, NULL},
	};

	binder.createClass("TextField", "Sprite", create, destruct, functionList);
}

int TextFieldBinder::create(lua_State* L)
{
	StackChecker checker(L, "TextFieldBinder::create", 1);

	LuaApplication* application = static_cast<LuaApplication*>(luaL_getdata(L));

	Binder binder(L);
	TextFieldBase* textField = NULL;

	FontBase* font = NULL;
	if (lua_isnoneornil(L, 1))
		font = application->getApplication()->getDefaultFont();
	else
		font = static_cast<FontBase*>(binder.getInstance("FontBase", 1));

	const char* str2 = lua_tostring(L, 2);
    const char* str3 = lua_tostring(L, 3);

    switch (font->getType())
    {
        case FontBase::eFont:
        case FontBase::eTTBMFont:
            if (str2)
                if (str3)
                    textField = new TextField(application->getApplication(), static_cast<BMFontBase*>(font), str2, str3);
                else
                    textField = new TextField(application->getApplication(), static_cast<BMFontBase*>(font), str2);
            else
                textField = new TextField(application->getApplication(), static_cast<BMFontBase*>(font));
            break;
        case FontBase::eTTFont:
            if (str2)
                if (str3)
                    textField = new TTTextField(application->getApplication(), static_cast<TTFont*>(font), str2, str3);
                else
                    textField = new TTTextField(application->getApplication(), static_cast<TTFont*>(font), str2);
            else
                textField = new TTTextField(application->getApplication(), static_cast<TTFont*>(font));
            break;
    }

/*
	if (lua_gettop(L) == 0)
	{
		textField = new TextField;
	}
	else if (lua_gettop(L) == 1)
	{
		Font* font = static_cast<Font*>(binder.getInstance("Font", 1));
		textField = new TextField(font);
	}
	else
	{
		Font* font = static_cast<Font*>(binder.getInstance("Font", 1));
		const char* text = luaL_checkstring(L, 2);
		textField = new TextField(font, text);
	}
*/

	binder.pushInstance("TextField", textField);

/*	if  (lua_gettop(L) == 1)
	{
		lua_pushvalue(L, 1);
		lua_setfield(L, -2, "__font");
	}
	else
	{
		lua_pushvalue(L, 1);
		lua_setfield(L, -2, "__font");

		lua_pushvalue(L, 2);
		lua_setfield(L, -2, "__text");
	} */

	return 1;
}

int TextFieldBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	TextFieldBase* textField = static_cast<TextFieldBase*>(ptr);
	textField->unref();

	return 0;
}


int TextFieldBinder::setFont(lua_State* L)
{
    StackChecker checker(L, "TextFieldBinder::setFont", 0);

    Binder binder(L);
    TextFieldBase* textField = static_cast<TextFieldBase*>(binder.getInstance("TextField", 1));
    FontBase* font = static_cast<FontBase*>(binder.getInstance("FontBase", 2));

    textField->setFont(font);

    return 0;
}


int TextFieldBinder::getText(lua_State* L)
{
	StackChecker checker(L, "TextFieldBinder::getText", 1);

	Binder binder(L);
	TextFieldBase* textField = static_cast<TextFieldBase*>(binder.getInstance("TextField", 1));

	lua_pushstring(L, textField->text());

	return 1;
}

int TextFieldBinder::setText(lua_State* L)
{
	StackChecker checker(L, "TextFieldBinder::setText", 0);

	Binder binder(L);
	TextFieldBase* textField = static_cast<TextFieldBase*>(binder.getInstance("TextField", 1));

	const char* text = luaL_checkstring(L, 2);
	textField->setText(text);

	return 0;
}

int TextFieldBinder::getTextColor(lua_State* L)
{
	StackChecker checker(L, "TextFieldBinder::getTextColor", 1);

	Binder binder(L);
	TextFieldBase* textField = static_cast<TextFieldBase*>(binder.getInstance("TextField", 1));

	lua_pushinteger(L, textField->textColor());

	return 1;
}

int TextFieldBinder::setTextColor(lua_State* L)
{
	StackChecker checker(L, "TextFieldBinder::setTextColor", 0);

	Binder binder(L);
	TextFieldBase* textField = static_cast<TextFieldBase*>(binder.getInstance("TextField", 1));

	unsigned int color = luaL_checkinteger(L, 2);
	textField->setTextColor(color);

	return 0;
}


int TextFieldBinder::getLetterSpacing(lua_State* L)
{
	StackChecker checker(L, "TextFieldBinder::getLetterSpacing", 1);

	Binder binder(L);
	TextFieldBase* textField = static_cast<TextFieldBase*>(binder.getInstance("TextField", 1));

    lua_pushnumber(L, textField->letterSpacing());

	return 1;
}

int TextFieldBinder::setLetterSpacing(lua_State* L)
{
	StackChecker checker(L, "TextFieldBinder::setLetterSpacing", 0);

	Binder binder(L);
	TextFieldBase* textField = static_cast<TextFieldBase*>(binder.getInstance("TextField", 1));

    textField->setLetterSpacing(luaL_checknumber(L, 2));

	return 0;
}

int TextFieldBinder::getLineHeight(lua_State* L)
{
    StackChecker checker(L, "TextFieldBinder::getLineHeight", 1);

    Binder binder(L);
    TextFieldBase* textField = static_cast<TextFieldBase*>(binder.getInstance("TextField", 1));

    lua_pushnumber(L, textField->lineHeight());

    return 1;
}

int TextFieldBinder::getSample(lua_State* L)
{
    StackChecker checker(L, "TextFieldBinder::getSample", 1);

    Binder binder(L);
    TextFieldBase* textField = static_cast<TextFieldBase*>(binder.getInstance("TextField", 1));

    lua_pushstring(L, textField->sample());

    return 1;
}

int TextFieldBinder::setSample(lua_State* L)
{
    StackChecker checker(L, "TextFieldBinder::setSample", 0);

    Binder binder(L);
    TextFieldBase* textField = static_cast<TextFieldBase*>(binder.getInstance("TextField", 1));

    const char* sample = luaL_checkstring(L, 2);
    textField->setSample(sample);

    return 0;
}
