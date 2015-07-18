#include <vector>
#include <numeric>
#include <algorithm>

//extern "C"
//{
//#include "lua.h"
//#include "lauxlib.h"
//#include "lualib.h"
//}
#include "lua.hpp"

enum Type
{
	eEmpty,
	eFilled,
	eCrossed,
};


static void calculate(	const std::vector<Type>& current,
						const std::vector<int>& counts,
						std::vector<bool>* slashes,
						std::vector<bool>* filleds)
{
	// check if std::accumulate(counts.begin(), counts.end(), 0) == number of filleds
	bool allFilleds = false;
	{
		int nFilleds = 0;
		for (std::size_t i = 0; i < current.size(); ++i)
			if (current[i] == eFilled)
				nFilleds++;

		if (nFilleds == std::accumulate(counts.begin(), counts.end(), 0))
			allFilleds = true;
	}

	int width = (int)current.size();

	std::vector<std::vector<int> > fills(counts.size(), std::vector<int>(width, 0));

	int size = (int)counts.size();
	int limit = width - std::accumulate(counts.begin(), counts.end(), 0);

	std::vector<int> a(size, 1);
	a[0] = 0;

	int ncount = 0;
	while (true)
	{
		int lasta = limit - std::accumulate(a.begin(), a.end(), 0);

		bool invalid = false;
		{
			int pos = 0;
			for (int i = 0; i < size; ++i)
			{
				for (int j = 0; j < a[i]; ++j)
					if (current[pos++] == eFilled)
						invalid = true;

				for (int j = 0; j < counts[i]; ++j)		
					if (current[pos++] == eCrossed)
					{
						/*invalid = true*/;				// currently we ignore CROSSes
					}
			}
			for (int j = 0; j < lasta; ++j)
				if (current[pos++] == eFilled)
					invalid = true;
		}

		if (invalid == false)
			ncount++;

		if (invalid == false)
		{
			int pos = 0;
			for (int i = 0; i < size; ++i)
			{
				for (int j = 0; j < a[i]; ++j)
					pos++;
				for (int j = 0; j < counts[i]; ++j)
					fills[i][pos++]++;
			}
			for (int j = 0; j < lasta; ++j)
				pos++;
		}

		bool last = false;
		int c = 0;	// carry bit
		for (int i = 0; i < size; ++i)
		{
			if (i == 0)
				a[i] = a[i] + 1 + c;
			else
				a[i] = a[i] + c;

			if (std::accumulate(a.begin(), a.end(), 0) > limit)
			{
				if (i == 0)
					a[i] = 0;
				else
					a[i] = 1;

				c = 1;
			}
			else
				c = 0;

			if (i == size - 1 && c == 1)
				last = true;
		}

		if (last == true)
			break;
	}

	slashes->resize(counts.size());
	filleds->resize(width, false);

	for (std::size_t i = 0; i < counts.size(); ++i)
	{
		bool allfilled = true;
		int count = 0;
		for (int j = 0; j < width; ++j)
			if (fills[i][j] == ncount)
			{
				count++;

				if (current[j] != eFilled)
					allfilled = false;
			}

		(*slashes)[i] = (count == counts[i] && allfilled == true);

		// check if both sides have X, if not set (*slashes)[i] as false
		if (allFilleds == false)
		{
			if ((*slashes)[i] == true)
			{
				// find start and end indices
				int start = -1;
				int end = width;

				for (int j = 0; j < width; ++j)
					if (fills[i][j] == ncount)
					{
						start = j;
						break;
					}
				
				for (int j = width - 1; j >= 0; --j)
					if (fills[i][j] == ncount)
					{
						end = j;
						break;
					}

				bool ok = (start == 0 || current[start - 1] == eCrossed) && (end == width - 1 || current[end + 1] == eCrossed);

				if (ok == false)
					(*slashes)[i] = false;
			}
		}

		if ((*slashes)[i] == true)
		{
			for (int j = 0; j < width; ++j)
				if (fills[i][j] == ncount)
					(*filleds)[j] = true;
		}
	}
}

static int calculate_bridge(lua_State* L)
{
	int begin = lua_gettop(L);

	int n;

	// populate current vector
	n = luaL_getn(L, 1);
	std::vector<Type> current(n);
	for (int i = 1; i <= n; i++)
	{
		lua_rawgeti(L, 1, i);
		current[i - 1] = static_cast<Type>(lua_tointeger(L, -1));
		lua_pop(L, 1);
	}


	// populate counts vector
	n = luaL_getn(L, 2);
	std::vector<int> counts(n);
	for (int i = 1; i <= n; i++)
	{
		lua_rawgeti(L, 2, i);
		counts[i - 1] = lua_tointeger(L, -1);
		lua_pop(L, 1);
	}


	std::vector<bool> slashes;
	std::vector<bool> filleds;

	calculate(current, counts, &slashes, &filleds);

	lua_createtable(L, slashes.size(), 0);
	for (std::size_t i = 0; i < slashes.size(); ++i)
	{
		lua_pushboolean(L, slashes[i]);
		lua_rawseti(L, -2, i + 1);
	}

	lua_createtable(L, filleds.size(), 0);
	for (std::size_t i = 0; i < filleds.size(); ++i)
	{
		lua_pushboolean(L, filleds[i]);
		lua_rawseti(L, -2, i + 1);
	}

	int end = lua_gettop(L);

	return 2;
}

void register_finishedGroups(lua_State* L)
{
	lua_newtable(L);

	lua_pushinteger(L, eEmpty);
	lua_setfield(L, -2, "eEmpty");
	lua_pushinteger(L, eFilled);
	lua_setfield(L, -2, "eFilled");
	lua_pushinteger(L, eCrossed);
	lua_setfield(L, -2, "eCrossed");

	lua_pushcfunction(L, calculate_bridge);
	lua_setfield(L, -2, "calculate");

	lua_setglobal(L, "finishedGroups");
}
