/*

    stackdoc.h:

        Output Lua stack logs to a file for examination.

    Author: Paul Reilly

*/

#pragma once
#include "lua.hpp"
#include <fstream>
#include <string>


class StackDoc
{
public:
    StackDoc(std::string filename, bool clear_file_with_new_session);
    ~StackDoc();
    bool isLogFileOpen();
    void logStack(lua_State* L, std::string msg);

private:
    std::ofstream file_out;
    std::string m_filename;
};
