#include "stackchecker.h"
#include <glog.h>

PrintStackChecker::PrintStackChecker(lua_State* L, const char* pre, int delta) : L(L), delta_(delta), pre_(pre)
{
	begin_ = lua_gettop(L);
}

PrintStackChecker::~PrintStackChecker()
{
	int end = lua_gettop(L);
	if (begin_ + delta_ == end)
		;
	else
        glog_e("*%s* stack NOT ok begin:%d end:%d delta:%d", pre_, begin_, end, delta_);
}
