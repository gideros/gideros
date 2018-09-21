#include "stackdoc.h"
#include <ctime>
#include <iomanip>

StackDoc::StackDoc(std::string filename, bool clear_file_with_new_session)
    : m_filename(filename)
{
    time_t now = time(nullptr);
    const char* date_time = ctime(&now);
    int flags = std::ofstream::out;
    if (!clear_file_with_new_session)
        flags = flags | std::ofstream::app;
    file_out.open(m_filename);
    file_out << "\nNew log session: " << date_time << std::endl;
    file_out.close();
}

StackDoc::~StackDoc()
{
    if (file_out.is_open())
        file_out.close();
}

bool StackDoc::isLogFileOpen()
{
    return file_out.is_open();
}

void StackDoc::logStack(lua_State *L, std::string msg = "")
{
    file_out.open(m_filename, std::ofstream::out | std::ofstream::app);
    int i;
    int top = lua_gettop(L);
    file_out << "---------------------------------------\n";
    file_out << ":-> " << msg << "\n";
    file_out << "Stack size: " << top << "\nTop\n";
    for (i = top; i >= 1; i--)
    {
        int type = lua_type(L, i);
        switch (type)
        {
        case LUA_TSTRING:
            file_out << std::setw(3) << i << " -- " << std::setw(3) << (i - (top + 1)) << " ---- `" << lua_tostring(L, i) << "'";
            break;
        case LUA_TBOOLEAN:
            file_out << std::setw(3) << i << " -- " << std::setw(3) << (i - (top + 1)) << " ---- `" << (lua_toboolean(L, i) ? "true" : "false") << "'";
            break;
        case LUA_TNUMBER:
            file_out << std::setw(3) << i << " -- " << std::setw(3) << (i - (top + 1)) << " ---- `" << lua_tonumber(L, i) << "'";
            break;
        default:
            file_out << std::setw(3) << i << " -- " << std::setw(3) << (i - (top + 1)) << " ---- `" << lua_typename(L, type) << "'";
            break;
        }
        file_out << "\n";
    }
    file_out << "---------------------------------------\n\n";
    file_out << std::endl;
    file_out.close();
}
