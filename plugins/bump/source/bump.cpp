#include "gideros.h"
#include "lua.h"
#include "lauxlib.h"
#include <math.h>
#include <map>
#include <set>
#include <algorithm>

#define MATH_HUGE HUGE_VAL
/*
 local bump = {
 _VERSION     = 'bump v3.1.7',
 _URL         = 'https://github.com/kikito/bump.lua',
 _DESCRIPTION = 'A collision detection library for Lua',
 _LICENSE     = [[
 MIT LICENSE

 Copyright (c) 2014 Enrique García Cota

 Permission is hereby granted, free of charge, to any person obtaining a
 copy of this software and associated documentation files (the
 "Software"), to deal in the Software without restriction, including
 without limitation the rights to use, copy, modify, merge, publish,
 distribute, sublicense, and/or sell copies of the Software, and to
 permit persons to whom the Software is furnished to do so, subject to
 the following conditions:

 The above copyright notice and this permission notice shall be included
 in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ]]
 }*/

#define DELTA 1e-10 // -- floating-point margin of error

static double sign(double x) {
	return (x > 0) ? 1 : ((x == 0) ? 0 : -1);
}
static double nearest(double x, double a, double b) {
	return fabs(a - x) < fabs(b - x) ? a : b;
}

static void assertNumber(lua_State *L, int narg, const char *name) {
	lua_Integer d = lua_tointeger(L, narg);
	if (d == 0 && !lua_isnumber(L, narg)) /* avoid extra test when d is not 0 */
	{
		lua_pushfstring(L, "%s must be a number, but was %s (a %s)", name,
				lua_tostring(L, narg), lua_typename(L, narg));
		lua_error(L);
	}
}

static void assertIsPositiveNumber(lua_State *L, int narg, const char *name) {
	lua_Integer d = lua_tointeger(L, narg);
	if (d <= 0) {
		lua_pushfstring(L, "%s must be a positive integer, but was %s (a %s)",
				name, lua_tostring(L, narg), lua_typename(L, narg));
		lua_error(L);
	}
}

void assertIsRect(lua_State *L, int x, int y, int w, int h) {
	assertNumber(L, x, "x");
	assertNumber(L, y, "y");
	assertIsPositiveNumber(L, w, "w");
	assertIsPositiveNumber(L, h, "h");
}

struct ColFilter {
	virtual const char *Filter(int item, int other)=0;
	virtual ~ColFilter() { };
};

struct defaultFilter: ColFilter {
	const char *Filter(int item, int other) {
		return "slide";
	}
	;
};

/*
 ------------------------------------------
 -- Rectangle functions
 ------------------------------------------
 */

static void rect_getNearestCorner(double x, double y, double w, double h,
		double px, double py, double &nx, double &ny) {
	nx = nearest(px, x, x + w);
	ny = nearest(py, y, y + h);
}

/*-- This is a generalized implementation of the liang-barsky algorithm, which also returns
 -- the normals of the sides where the segment intersects.
 -- Returns nil if the segment never touches the rect
 -- Notice that normals are only guaranteed to be accurate when initially ti1, ti2 == -math.huge, math.huge
 */
static bool rect_getSegmentIntersectionIndices(double x, double y, double w,
		double h, double x1, double y1, double x2, double y2, double &ti1,
		double &ti2, double &nx1, double &ny1, double &nx2, double &ny2) {
	//ti1, ti2 = ti1 or 0, ti2 or 1 TODO
	double dx = x2 - x1;
	double dy = y2 - y1;
	double nx, ny;
	double p, q, r;
	nx1 = nx2 = ny1 = ny2 = 0;

	for (int side = 1; side <= 4; side++) {
		switch (side) {
		case 1:
			nx = -1;
			ny = 0;
			p = -dx;
			q = x1 - x;
			break; //-- left
		case 2:
			nx = 1;
			ny = 0;
			p = dx;
			q = x + w - x1;
			break; //-- right
		case 3:
			nx = 0;
			ny = -1;
			p = -dy;
			q = y1 - y;
			break; //-- top
		case 4:
			nx = 0;
			ny = 1;
			p = dy;
			q = y + h - y1;
			break; //-- bottom
		}
		if (p == 0) {
			if (q <= 0)
				return false;
		} else {
			r = q / p;
			if (p < 0) {
				if (r > ti2)
					return false;
				if (r > ti1) {
					ti1 = r;
					nx1 = nx;
					ny1 = ny;
				}
			} else { //-- p > 0
				if (r < ti1)
					return false;
				if (r < ti2) {
					ti2 = r;
					nx2 = nx;
					ny2 = ny;
				}
			}
		}
	}
	return true;
}

//-- Calculates the minkowsky difference between 2 rects, which is another rect
static void rect_getDiff(double x1, double y1, double w1, double h1, double x2,
		double y2, double w2, double h2, double &rx, double &ry, double &rw,
		double &rh) {
	rx = x2 - x1 - w1;
	ry = y2 - y1 - h1;
	rw = w1 + w2;
	rh = h1 + h2;
}

static bool rect_containsPoint(double x, double y, double w, double h,
		double px, double py) {
	return ((px - x) > DELTA) && ((py - y) > DELTA) && ((x + w - px) > DELTA)
			&& ((y + h - py) > DELTA);
}

static bool rect_isIntersecting(double x1, double y1, double w1, double h1,
		double x2, double y2, double w2, double h2) {
	return (x1 < (x2 + w2)) && (x2 < (x1 + w1)) && (y1 < (y2 + h2))
			&& (y2 < (y1 + h1));
}

static double rect_getSquareDistance(double x1, double y1, double w1, double h1,
		double x2, double y2, double w2, double h2) {
	double dx = x1 - x2 + (w1 - w2) / 2;
	double dy = y1 - y2 + (h1 - h2) / 2;
	return dx * dx + dy * dy;
}

struct Point {
	double x, y;
};
struct Rect {
	double x, y, w, h;
};
struct Collision {
	bool overlaps;
	int item;
	int other;
	const char *type;
	double ti;
	Point move, normal, touch;
	Point response;
	Rect itemRect, otherRect;
};

static bool rect_detectCollision(double x1, double y1, double w1, double h1,
		double x2, double y2, double w2, double h2, double goalX, double goalY,
		Collision &col) {
	//goalX = goalX or x1 TODO
	//goalY = goalY or y1

	double dx = goalX - x1, dy = goalY - y1;
	double x, y, w, h;
	rect_getDiff(x1, y1, w1, h1, x2, y2, w2, h2, x, y, w, h);

	bool overlaps;
	double ti;
	bool cf = false;
	double nx, ny;

	if (rect_containsPoint(x, y, w, h, 0, 0)) { //-- item was intersecting other
		double px, py;
		rect_getNearestCorner(x, y, w, h, 0, 0, px, py);
		double wi = (w1 < abs(px)) ? w1 : abs(px);
		double hi = (h1 < abs(py)) ? h1 : abs(py);
		ti = -wi * hi; //-- ti is the negative area of intersection
		overlaps = true;
		cf = true;
	} else {
		double ti1 = -MATH_HUGE, ti2 = MATH_HUGE;
		double nx1, ny1, nx2, ny2;
		if (rect_getSegmentIntersectionIndices(x, y, w, h, 0, 0, dx, dy, ti1,
				ti2, nx1, ny1, nx2, ny2)) {
			//-- item tunnels into other
			if ((ti1 < 1) && (abs(ti1 - ti2) >= DELTA) //-- special case for rect going through another rect's corner
					&& ((0 < (ti1 + DELTA)) || ((0 == ti1) && (ti2 > 0)))) {
				ti = ti1;
				nx = nx1;
				ny = ny1;
				overlaps = false;
				cf = true;
			}
		}
	}

	if (!cf)
		return false;

	double tx, ty;

	if (overlaps) {
		if ((dx == 0) && (dy == 0)) {
			//-- intersecting and not moving - use minimum displacement vector
			double px, py;
			rect_getNearestCorner(x, y, w, h, 0, 0, px, py);
			if (abs(px) < abs(py))
				py = 0;
			else
				px = 0;
			nx = sign(px);
			ny = sign(py);
			tx = x1 + px;
			ty = y1 + py;
		} else {
			//-- intersecting and moving - move in the opposite direction
			double ti1 = -MATH_HUGE;
			double ti2 = 1;
			double nx1, ny1, nx2, ny2;
			if (!rect_getSegmentIntersectionIndices(x, y, w, h, 0, 0, dx, dy,
					ti1, ti2, nx1, ny1, nx2, ny2))
				return false;
			tx = x1 + dx * ti1;
			ty = y1 + dy * ti1;
		}
	} else //-- tunnel
	{
		tx = x1 + dx * ti;
		ty = y1 + dy * ti;
	}

	col.overlaps = overlaps;
	col.ti = ti;
	col.move.x = dx;
	col.move.y = dy;
	col.normal.x = nx;
	col.normal.y = ny;
	col.touch.x = tx;
	col.touch.y = ty;
	col.itemRect.x = x1;
	col.itemRect.y = y1;
	col.itemRect.w = w1;
	col.itemRect.h = h1;
	col.otherRect.x = x2;
	col.otherRect.y = y2;
	col.otherRect.w = w2;
	col.otherRect.h = h2;
	return true;
}

//------------------------------------------
//-- Grid functions
//------------------------------------------

static void grid_toWorld(int cellSize, int cx, int cy, double &wx, double &wy) {
	wx = (cx - 1) * cellSize;
	wy = (cy - 1) * cellSize;
}

static void grid_toCell(int cellSize, double x, double y, int &cx, int &cy) {
	cx = floor(x / cellSize) + 1;
	cy = floor(y / cellSize) + 1;
}

/*-- grid_traverse* functions are based on "A Fast Voxel Traversal Algorithm for Ray Tracing",
 -- by John Amanides and Andrew Woo - http://www.cse.yorku.ca/~amana/research/grid.pdf
 -- It has been modified to include both cells when the ray "touches a grid corner",
 -- and with a different exit condition*/

static int grid_traverse_initStep(int cellSize, int ct, double t1, double t2,
		double &rx, double &ry) {
	double v = t2 - t1;
	if (v > 0) {
		rx = cellSize / v;
		ry = ((ct + v) * cellSize - t1) / v;
		return 1;
	}
	if (v < 0) {
		rx = -cellSize / v;
		ry = ((ct + v - 1) * cellSize - t1) / v;
		return -1;
	}
	rx = HUGE_VAL;
	ry = HUGE_VAL;
	return 0;
}

typedef void (*pointFunc)(void *data, int, int);

static void grid_traverse(int cellSize, double x1, double y1, double x2,
		double y2, pointFunc f, void *data) {
	int cx1, cy1, cx2, cy2;
	double dx, tx, dy, ty;
	grid_toCell(cellSize, x1, y1, cx1, cy1);
	grid_toCell(cellSize, x2, y2, cx2, cy2);
	int stepX = grid_traverse_initStep(cellSize, cx1, x1, x2, dx, tx);
	int stepY = grid_traverse_initStep(cellSize, cy1, y1, y2, dy, ty);
	int cx = cx1;
	int cy = cy1;

	f(data, cx, cy);

	//-- The default implementation had an infinite loop problem when
	//-- approaching the last cell in some occassions. We finish iterating
	//-- when we are *next* to the last cell
	while ((fabs(cx - cx2) + fabs(cy - cy2)) > 1) {
		if (tx < ty) {
			tx += dx;
			cx += stepX;
			f(data, cx, cy);
		} else {
			//-- Addition: include both cells when going through corners
			if (tx == ty)
				f(data, cx + stepX, cy);
			ty += dy;
			cy += stepY;
			f(data, cx, cy);
		}
	}

	//-- If we have not arrived to the last cell, use it
	if ((cx != cx2) || (cy != cy2))
		f(data, cx2, cy2);
}

static void grid_toCellRect(int cellSize, double x, double y, double w,
		double h, int &cx, int &cy, int &cw, int &ch) {
	grid_toCell(cellSize, x, y, cx, cy);
	int cr = ceil((x + w) / cellSize);
	int cb = ceil((y + h) / cellSize);
	cw = cr - cx + 1;
	ch = cb - cy + 1;
}

struct World;
/*------------------------------------------
 -- Responses
 ------------------------------------------*/
struct Response {
	virtual std::vector<Collision> ComputeResponse(World *world, Collision col, double x,
			double y, double w, double h, double goalX, double goalY,
			ColFilter *filter, double &actualX, double &actualY)=0;
	virtual ~Response() {
	}
	;
};

/*------------------------------------------
 -- World
 ------------------------------------------*/

struct Cell {
	std::set<int> items;
	int x, y;
	Cell() :
			x(0), y(0) {
	}
};

struct ItemInfo {
	int item;
	double ti1, ti2, weight;
	double x1, y1, x2, y2;
};

struct ItemFilter {
	virtual bool Filter(int item)=0;
	virtual ~ItemFilter() {
	}
	;
};

struct World {
	int cellSize;
	std::map<int, Rect> rects;
	std::map<int, std::map<int, Cell>> rows;
	std::map<const char *, Response *> responses;
//-- Private functions and methods
	static bool sortByWeight(ItemInfo a, ItemInfo b) {
		return a.weight < b.weight;
	}

	static bool sortByTiAndDistance(Collision a, Collision b) {
		if (a.ti == b.ti) {
			double ad = rect_getSquareDistance(a.itemRect.x, a.itemRect.y,
					a.itemRect.w, a.itemRect.h, a.otherRect.x, a.otherRect.y,
					a.otherRect.w, a.otherRect.h);
			double bd = rect_getSquareDistance(a.itemRect.x, a.itemRect.y,
					a.itemRect.w, a.itemRect.h, b.otherRect.x, b.otherRect.y,
					b.otherRect.w, b.otherRect.h);
			return ad < bd;
		}
		return a.ti < b.ti;
	}

	void addItemToCell(int item, int cx, int cy) {
		rows[cy][cx].items.insert(item);
	}

	bool removeItemFromCell(int item, int cx, int cy) {
		std::map<int, std::map<int, Cell>>::iterator row = rows.find(cy);
		if (row == rows.end())
			return false;
		std::map<int, Cell>::iterator cell = row->second.find(cx);
		if (cell == row->second.end())
			return false;
		if (cell->second.items.find(item) == cell->second.items.end())
			return false;
		cell->second.items.erase(item);
		return true;
	}

	std::set<int> getDictItemsInCellRect(int cl, int ct, int cw, int ch) {
		std::set<int> items_dict;
		for (int cy = ct; cy < ct + ch; cy++) {
			std::map<int, std::map<int, Cell>>::iterator row = rows.find(cy);
			if (row == rows.end())
				continue;
			for (int cx = cl; cx < cl + cw; cx++) {
				std::map<int, Cell>::iterator cell = row->second.find(cx);
				if (cell == row->second.end())
					continue;
				if (cell->second.items.size() > 0) {
					for (std::set<int>::iterator it =
							cell->second.items.begin();
							it != cell->second.items.end(); it++)
						items_dict.insert(*it);
				}
			}
		}

		return items_dict;
	}
	struct _CellTraversal {
		World *world;
		std::set<Cell*> cells;
	};
	static void cellsTraversal_(void *ctx, int cx, int cy) {
		struct _CellTraversal *ct = (struct _CellTraversal *) ctx;
		std::map<int, std::map<int, Cell>>::iterator row = ct->world->rows.find(
				cy);
		if (row == ct->world->rows.end())
			return;
		std::map<int, Cell>::iterator cell = row->second.find(cx);
		if (cell == row->second.end())
			return;
		ct->cells.insert(&cell->second);
	}
	std::set<Cell*> getCellsTouchedBySegment(double x1, double y1, double x2,
			double y2) {
		struct _CellTraversal ct;
		ct.world = this;
		grid_traverse(cellSize, x1, y1, x2, y2, cellsTraversal_, &ct);
		return ct.cells;
	}

	std::vector<ItemInfo> getInfoAboutItemsTouchedBySegment(double x1,
			double y1, double x2, double y2, ItemFilter *filter) {
		std::set<Cell *> cells = getCellsTouchedBySegment(x1, y1, x2, y2);
		std::vector<ItemInfo> itemInfo;
		std::set<int> visited;

		for (std::set<Cell *>::iterator it = cells.begin(); it != cells.end();
				it++) {
			Cell *cell = (*it);
			for (std::set<int>::iterator i = cell->items.begin();
					i != cell->items.end(); i++) {
				if (visited.find(*i) != visited.end()) {
					visited.insert(*i);
					if ((!filter) || filter->Filter(*i)) {
						Rect r = rects[*i];
						double nx1, ny1, nx2, ny2;
						double ti1 = 0;
						double ti2 = 1;
						rect_getSegmentIntersectionIndices(r.x, r.y, r.w, r.h,
								x1, y1, x2, y2, ti1, ti2, nx1, ny1, nx2, ny2);
						if (ti1
								&& (((0 < ti1) && (ti1 < 1))
										|| ((0 < ti2) && (ti2 < 1)))) {
							//-- the sorting is according to the t of an infinite line, not the segment
							double tii0 = -MATH_HUGE;
							double tii1 = MATH_HUGE;
							rect_getSegmentIntersectionIndices(r.x, r.y, r.w,
									r.h, x1, y1, x2, y2, tii0, tii1, nx1, ny1,
									nx2, ny2);
							ItemInfo ii;
							ii.item = *i;
							ii.ti1 = ti1;
							ii.ti2 = ti2;
							ii.weight = tii0 < tii1 ? tii0 : tii1;
							itemInfo.push_back(ii);
						}
					}
				}
			}
		}
		std::sort(itemInfo.begin(), itemInfo.end(), sortByWeight);
		return itemInfo;
	}

	Response *getResponseByName(const char *name) {
		Response *response = responses[name];
		if (!response) {
			// TODO error(('Unknown collision type: %s (%s)'):format(name, type(name)))
			response = responses["slide"];
		}
		return response;
	}

//-- Misc Public Methods
	void addResponse(const char *name, Response *response) {
		responses[name] = response;
	}

	std::vector<Collision> project(int item, double x, double y, double w,
			double h, double goalX, double goalY, ColFilter *filter) {
		//assertIsRect(x,y,w,h) TODO

		//goalX = goalX or x
		//goalY = goalY or y
		//filter  = filter  or defaultFilter

		std::vector<Collision> collisions;
		std::set<int> visited;
		if (item)
			visited.insert(item);

		//-- This could probably be done with less cells using a polygon raster over the cells instead of a
		//-- bounding rect of the whole movement. Conditional to building a queryPolygon method
		double tl = (goalX < x) ? goalX : x;
		double tt = (goalY < y) ? goalY : y;
		double tr = ((goalX + w) > (x + w)) ? goalX + w : x + w;
		double tb = ((goalY + h) > (y + h)) ? goalY + h : y + h;
		double tw = tr - tl;
		double th = tb - tt;

		int cl, ct, cw, ch;
		grid_toCellRect(cellSize, tl, tt, tw, th, cl, ct, cw, ch);

		std::set<int> dictItemsInCellRect = getDictItemsInCellRect(cl, ct, cw,
				ch);

		for (std::set<int>::iterator it = dictItemsInCellRect.begin();
				it != dictItemsInCellRect.end(); it++) {
			int other = *it;
			if (visited.find(other) == visited.end()) {
				visited.insert(other);
				const char *responseName = filter->Filter(item, other);
				if (responseName) {
					double ox, oy, ow, oh;
					getRect(other, ox, oy, ow, oh);
					Collision col;
					if (rect_detectCollision(x, y, w, h, ox, oy, ow, oh, goalX,
							goalY, col)) {
						col.other = other;
						col.item = item;
						col.type = responseName;
						collisions.push_back(col);
					}
				}
			}
		}

		std::sort(collisions.begin(), collisions.end(), sortByTiAndDistance);

		return collisions;
	}

	int countCells() {
		int count = 0;
		for (std::map<int, std::map<int, Cell>>::iterator row = rows.begin();
				row != rows.end(); row++)
			count += row->second.size();
		return count;
	}

	bool hasItem(int item) {
		return rects.find(item) != rects.end();
	}

	std::set<int> getItems() {
		std::set<int> items;
		for (std::map<int, Rect>::iterator r = rects.begin(); r != rects.end();
				r++)
			items.insert(r->first);
		return items;
	}

	int countItems() {
		return rects.size();
	}

	void getRect(int item, double &x, double &y, double &w, double &h) {
		Rect r = rects[item];
		x = r.x;
		y = r.y;
		w = r.w;
		h = r.h;
	}

	void toWorld(int cx, int cy, double &x, double &y) {
		grid_toWorld(cellSize, cx, cy, x, y);
	}
	void toCell(double x, double y, int &cx, int &cy) {
		grid_toCell(cellSize, x, y, cx, cy);
	}

//--- Query methods

	std::set<int> queryRect(double x, double y, double w, double h,
			ItemFilter *filter) {

		int cl, ct, cw, ch;
		grid_toCellRect(cellSize, x, y, w, h, cl, ct, cw, ch);
		std::set<int> dictItemsInCellRect = getDictItemsInCellRect(cl, ct, cw,
				ch);
		for (std::set<int>::iterator it = dictItemsInCellRect.begin();
				it != dictItemsInCellRect.end();) {
			Rect rect = rects[*it];
			if ((filter && !filter->Filter(*it))
					|| !rect_isIntersecting(x, y, w, h, rect.x, rect.y, rect.w,
							rect.h))
				dictItemsInCellRect.erase(it++);
			else
				++it;
		}
		return dictItemsInCellRect;
	}

	std::set<int> queryPoint(double x, double y, ItemFilter *filter) {
		int cx, cy;
		toCell(x, y, cx, cy);
		std::set<int> dictItemsInCellRect = getDictItemsInCellRect(cx, cy, 1,
				1);
		for (std::set<int>::iterator it = dictItemsInCellRect.begin();
				it != dictItemsInCellRect.end();) {
			Rect rect = rects[*it];
			if ((filter && !filter->Filter(*it))
					|| !rect_containsPoint(rect.x, rect.y, rect.w, rect.h, x,
							y))
				dictItemsInCellRect.erase(it++);
			else
				++it;
		}
		return dictItemsInCellRect;
	}

	std::set<int> querySegment(double x1, double y1, double x2, double y2,
			ItemFilter *filter) {
		std::vector<ItemInfo> itemInfo = getInfoAboutItemsTouchedBySegment(x1, y1,
				x2, y2, filter);
		std::set<int> items;
		for (std::vector<ItemInfo>::iterator it = itemInfo.begin();
				it != itemInfo.end(); it++)
			items.insert((*it).item);
		return items;
	}

	std::vector<ItemInfo> querySegmentWithCoords(double x1, double y1, double x2,
			double y2, ItemFilter *filter) {
		std::vector<ItemInfo> itemInfo = getInfoAboutItemsTouchedBySegment(x1, y1,
				x2, y2, filter);
		double dx = x2 - x1, dy = y2 - y1;
		std::vector<ItemInfo> itemInfo2;
		for (std::vector<ItemInfo>::iterator it = itemInfo.begin();
				it != itemInfo.end(); it++) {
			ItemInfo i=*it;
			i.x1 = x1 + dx * i.ti1;
			i.y1 = y1 + dy * i.ti1;
			i.x2 = x1 + dx * i.ti2;
			i.y2 = y1 + dy * i.ti2;
			itemInfo2.push_back(i);
		}
		return itemInfo2;
	}

//--- Main methods

	void add(int item, double x, double y, double w, double h) {
		Rect r;
		r.x = x;
		r.y = y;
		r.w = w;
		r.h = h;
		rects[item] = r;
		int cl, ct, cw, ch;

		grid_toCellRect(cellSize, x, y, w, h, cl, ct, cw, ch);
		for (int cy = ct; cy < ct + ch; cy++)
			for (int cx = cl; cx < cl + cw; cx++)
				addItemToCell(item, cx, cy);
	}

	void remove(int item) {
		Rect r = rects[item];
		int cl, ct, cw, ch;

		grid_toCellRect(cellSize, r.x, r.y, r.w, r.h, cl, ct, cw, ch);
		for (int cy = ct; cy < ct + ch; cy++)
			for (int cx = cl; cx < cl + cw; cx++)
				removeItemFromCell(item, cx, cy);
	}

	void update(int item, double x2, double y2, double w2, double h2) {
		Rect r = rects[item];
		if (w2 <= 0)
			w2 = r.w;
		if (h2 <= 0)
			h2 = r.h;

		if ((r.x != x2) || (r.y != y2) || (r.w != w2) || (r.h != h2)) {
			int cl1, ct1, cw1, ch1;
			grid_toCellRect(cellSize, r.x, r.y, r.w, r.h, cl1, ct1, cw1, ch1);
			int cl2, ct2, cw2, ch2;
			grid_toCellRect(cellSize, x2, y2, w2, h2, cl2, ct2, cw2, ch2);

			if ((cl1 != cl2) || (ct1 != ct2) || (cw1 != cw2) || (ch1 != ch2)) {

				int cr1 = cl1 + cw1 - 1, cb1 = ct1 + ch1 - 1;
				int cr2 = cl2 + cw2 - 1, cb2 = ct2 + ch2 - 1;
				bool cyOut;

				for (int cy = ct1; cy <= cb1; cy++) {
					cyOut = (cy < ct2) || (cy > cb2);
					for (int cx = cl1; cx <= cr1; cx++) {
						if (cyOut || (cx < cl2) || (cx > cr2))
							removeItemFromCell(item, cx, cy);
					}
				}

				for (int cy = ct2; cy <= cb2; cy++) {
					cyOut = (cy < ct1) || (cy > cb1);
					for (int cx = cl2; cx <= cr2; cx++) {
						if (cyOut || (cx < cl1) || (cx > cr1))
							addItemToCell(item, cx, cy);
					}
				}
			}

			Rect r;
			r.x = x2;
			r.y = y2;
			r.w = w2;
			r.h = h2;
			rects[item] = r;
		}
	}

	std::vector<Collision> move(int item, double goalX, double goalY,
			ColFilter *filter, double &actualX, double &actualY) {
		std::vector<Collision> cols = check(item, goalX, goalY, filter, actualX,
				actualY);
		update(item, actualX, actualY, -1, -1);
		return cols;
	}

	struct VisitedFilter: ColFilter {
		std::set<int> visited;
		ColFilter *filter;
		const char *Filter(int item, int other) {
			if (visited.find(item) != visited.end())
				return NULL;
			return filter->Filter(item, other);
		}
	};
	std::vector<Collision> check(int item, double goalX, double goalY,
			ColFilter *filter, double &actualX, double &actualY) {
		defaultFilter df;
		if (!filter)
			filter = &df;

		VisitedFilter vf;
		vf.visited.insert(item);
		vf.filter = filter;

		Rect r = rects[item];

		std::vector<Collision> cols;

		std::vector<Collision> projected_cols = project(item, r.x, r.y, r.w,
				r.h, goalX, goalY, &vf);

		while (projected_cols.size() > 0) {
			Collision col = projected_cols[0];
			cols.push_back(col);
			vf.visited.insert(col.other);
			Response *response = getResponseByName(col.type);

			projected_cols = response->ComputeResponse(this, col, r.x, r.y, r.w, r.h,
					goalX, goalY, &vf, goalX, goalY);
		}

		actualX = goalX;
		actualY = goalY;
		return cols;
	}
};

struct TouchResponse: Response {
	std::vector<Collision> ComputeResponse(World *world, Collision col, double x,
			double y, double w, double h, double goalX, double goalY,
			ColFilter *filter, double &actualX, double &actualY) {
		actualX = col.touch.x;
		actualY = col.touch.y;
		std::vector<Collision> cols;
		return cols;
	}
};

struct CrossResponse: Response {
	std::vector<Collision> ComputeResponse(World *world, Collision col, double x,
			double y, double w, double h, double goalX, double goalY,
			ColFilter *filter, double &actualX, double &actualY) {
		std::vector<Collision> cols = world->project(col.item, x, y, w, h, goalX,
				goalY, filter);
		actualX = goalX;
		actualY = goalY;
		return cols;
	}
};

struct SlideResponse: Response {
	std::vector<Collision> ComputeResponse(World *world, Collision col, double x,
			double y, double w, double h, double goalX, double goalY,
			ColFilter *filter, double &actualX, double &actualY) {
		//goalX = goalX or x TODO
		//goalY = goalY or y TODO

		double sx = col.touch.x;
		double sy = col.touch.y;

		if ((col.move.x != 0) || (col.move.y != 0)) {
			if (col.normal.x == 0)
				sx = goalX;
			else
				sy = goalY;
		}

		col.response.x = sx;
		col.response.y = sy;

		x = col.touch.x;
		y = col.touch.y;
		goalX = sx;
		goalY = sy;
		std::vector<Collision> cols = world->project(col.item, x, y, w, h, goalX,
				goalY, filter);
		actualX = goalX;
		actualY = goalY;
		return cols;
	}
};

struct BounceResponse: Response {
	std::vector<Collision> ComputeResponse(World *world, Collision col, double x,
			double y, double w, double h, double goalX, double goalY,
			ColFilter *filter, double &actualX, double &actualY) {
		double tx = col.touch.x;
		double ty = col.touch.y;
		double bx = tx;
		double by = ty;

		if ((col.move.x != 0) || (col.move.y != 0)) {
			double bnx = goalX - tx, bny = goalY - ty;
			if (col.normal.x == 0)
				bny = -bny;
			else
				bnx = -bnx;
			bx = tx + bnx;
			by = ty + bny;
		}

		col.response.x = bx;
		col.response.y = by;
		x = tx;
		y = ty;
		goalX = bx;
		goalY = by;
		std::vector<Collision> cols = world->project(col.item, x, y, w, h, goalX,
				goalY, filter);
		actualX = goalX;
		actualY = goalY;
		return cols;
	}
};

static CrossResponse responseCross;
static TouchResponse responseTouch;
static SlideResponse responseSlide;
static BounceResponse responseBounce;

int worldCreate(lua_State *L) {
	World *w = new World();
	g_pushInstance(L, "BumpWorld", w);
	lua_newtable(L);
	lua_setfield(L, -2, "__items");
	lua_newtable(L);
	lua_setfield(L, -2, "__itemsr");

	w->addResponse("touch", &responseTouch);
	w->addResponse("cross", &responseCross);
	w->addResponse("slide", &responseSlide);
	w->addResponse("bounce", &responseBounce);

	return 1;
}

int worldDestruct(lua_State *L) {
	World *w = (World *) g_getInstance(L, "BumpWorld", -1);
	w->responses.erase("touch");
	w->responses.erase("cross");
	w->responses.erase("bounce");
	w->responses.erase("slide");
	delete w;
	return 0;
}

struct LuaColFilter: ColFilter {
	lua_State *L;
	int itemsr;
	int func;
	const char *Filter(int item, int other) {
		lua_pushvalue(L, func);
		lua_rawgeti(L, itemsr - 1, item);
		lua_rawgeti(L, itemsr - 2, other);
		lua_call(L, 2, 1);
		const char *ret = lua_tostring(L, -1);
		lua_pop(L, 2);
		return ret;
	}
};

struct LuaItemFilter: ItemFilter {
	lua_State *L;
	int itemsr;
	int func;
	bool Filter(int item) {
		lua_pushvalue(L, func);
		lua_rawgeti(L, itemsr - 1, item);
		lua_call(L, 1, 1);
		bool ret = lua_isnoneornil(L, -1);
		lua_pop(L, 2);
		return ret;
	}
};

int worldProject(lua_State *L) {
	World *wr = (World *) g_getInstance(L, "BumpWorld", 1);
	lua_newtable(L);
	lua_getfield(L, 1, "__items");
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	if (lua_isnil(L, -1)) {
		lua_pushfstring(L,
				"Item %s must be added to the world before getting its rect. Use world:add(item, x,y,w,h) to add it first.",
				lua_tostring(L, 2));
		lua_error(L);
	}
	int item = lua_tonumber(L, -1);
	lua_pop(L, 2);
	double x = luaL_checknumber(L, 3);
	double y = luaL_checknumber(L, 4);
	double w = luaL_checknumber(L, 5);
	double h = luaL_checknumber(L, 6);
	double gx = luaL_checknumber(L, 7);
	double gy = luaL_checknumber(L, 8);

	ColFilter *f = NULL;
	LuaColFilter lf;
	lf.L = L;
	lf.itemsr = -1;
	if (!lua_isnoneornil(L, 9)) {
		luaL_checktype(L, 9, LUA_TFUNCTION);
		lf.func = 9;
		f = &lf;
	}
	double ax, ay;
	std::vector<Collision> items = wr->project(item, y, x, w,h, gx,gy,f);
	int n = 0;
	for (std::vector<Collision>::iterator it = items.begin(); it != items.end();
			it++) {
		lua_newtable(L);
		lua_rawgeti(L, -2, (*it).item);
		lua_setfield(L, -2, "item");
		lua_rawgeti(L, -2, (*it).other);
		lua_setfield(L, -2, "other");
		lua_pushstring(L, (*it).type);
		lua_setfield(L, -2, "type");
		lua_pushboolean(L, (*it).overlaps);
		lua_setfield(L, -2, "overlaps");
		lua_pushnumber(L, (*it).ti);
		lua_setfield(L, -2, "ti");

		lua_newtable(L);
		lua_pushnumber(L, (*it).move.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, (*it).move.y);
		lua_setfield(L, -2, "y");
		lua_setfield(L, -2, "move");
		lua_newtable(L);
		lua_pushnumber(L, (*it).normal.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, (*it).normal.y);
		lua_setfield(L, -2, "y");
		lua_setfield(L, -2, "normal");
		lua_newtable(L);
		lua_pushnumber(L, (*it).touch.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, (*it).touch.y);
		lua_setfield(L, -2, "y");
		lua_setfield(L, -2, "touch");

		lua_newtable(L);
		lua_pushnumber(L, (*it).itemRect.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, (*it).itemRect.y);
		lua_setfield(L, -2, "y");
		lua_pushnumber(L, (*it).itemRect.w);
		lua_setfield(L, -2, "w");
		lua_pushnumber(L, (*it).itemRect.h);
		lua_setfield(L, -2, "h");
		lua_setfield(L, -2, "itemRect");
		lua_newtable(L);
		lua_pushnumber(L, (*it).otherRect.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, (*it).otherRect.y);
		lua_setfield(L, -2, "y");
		lua_pushnumber(L, (*it).otherRect.w);
		lua_setfield(L, -2, "w");
		lua_pushnumber(L, (*it).otherRect.h);
		lua_setfield(L, -2, "h");
		lua_setfield(L, -2, "otherRect");

		lua_rawseti(L, -3, ++n);
	}
	lua_pop(L, 1);
	lua_pushinteger(L, n);

	return 2;
}

int worldCountCells(lua_State *L) {
	World *w = (World *) g_getInstance(L, "BumpWorld", 1);
	lua_pushnumber(L, w->countCells());
	return 1;
}

int worldCountItems(lua_State *L) {
	World *w = (World *) g_getInstance(L, "BumpWorld", 1);
	lua_pushnumber(L,w->countItems());
	return 1;
}

int worldHasItem(lua_State *L) {
	World *w = (World *) g_getInstance(L, "BumpWorld", 1);
	lua_getfield(L, 1, "__items");
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	bool isnil = lua_isnil(L, -1);
	lua_pop(L, 2);
	lua_pushboolean(L, isnil);
	return 1;
}

int worldGetItems(lua_State *L) {
	World *w = (World *) g_getInstance(L, "BumpWorld", 1);
	lua_newtable(L);
	lua_getfield(L, 1, "__itemsr");
	std::set<int> items = w->getItems();
	int n = 0;
	for (std::set<int>::iterator it = items.begin(); it != items.end(); it++) {
		lua_rawgeti(L, -1, *it);
		lua_rawseti(L, -3, ++n);
	}
	lua_pop(L, 1);
	return 1;
}

int worldGetRect(lua_State *L) {
	World *wr = (World *) g_getInstance(L, "BumpWorld", 1);
	lua_getfield(L, 1, "__items");
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	if (lua_isnil(L, -1)) {
		lua_pushfstring(L,
				"Item %s must be added to the world before getting its rect. Use world:add(item, x,y,w,h) to add it first.",
				lua_tostring(L, 2));
		lua_error(L);
	}
	int idx = lua_tonumber(L, -1);
	lua_pop(L, 2);
	double x, y, w, h;
	wr->getRect(idx, x, y, w, h);
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);
	return 4;
}

int worldToWorld(lua_State *L) {
	World *w = (World *) g_getInstance(L, "BumpWorld", 1);
	int cx = luaL_checknumber(L, 2);
	int cy = luaL_checknumber(L, 3);
	double x, y;
	w->toWorld(cx, cy, x, y);
	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	return 2;
}

int worldToCell(lua_State *L) {
	World *w = (World *) g_getInstance(L, "BumpWorld", 1);
	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	int cx, cy;
	w->toCell(x, y, cx, cy);
	lua_pushnumber(L, cx);
	lua_pushnumber(L, cy);
	return 2;
}

int worldQueryRect(lua_State *L) {
	World *wr = (World *) g_getInstance(L, "BumpWorld", 1);
	assertIsRect(L, 2, 3, 4, 5);
	lua_newtable(L);
	lua_getfield(L, 1, "__itemsr");

	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	double w = luaL_checknumber(L, 4);
	double h = luaL_checknumber(L, 5);
	ItemFilter *f = NULL;
	LuaItemFilter lf;
	lf.L = L;
	lf.itemsr = -1;
	if (!lua_isnoneornil(L, 6)) {
		luaL_checktype(L, 6, LUA_TFUNCTION);
		lf.func = 6;
		f = &lf;
	}
	std::set<int> items = wr->queryRect(x, y, w, h, f);
	int n = 0;
	for (std::set<int>::iterator it = items.begin(); it != items.end(); it++) {
		lua_rawgeti(L, -1, *it);
		lua_rawseti(L, -3, ++n);
	}
	lua_pop(L, 1);
	return 1;
}

int worldQueryPoint(lua_State *L) {
	World *wr = (World *) g_getInstance(L, "BumpWorld", 1);
	lua_newtable(L);
	lua_getfield(L, 1, "__itemsr");

	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	ItemFilter *f = NULL;
	LuaItemFilter lf;
	lf.L = L;
	lf.itemsr = -1;
	if (!lua_isnoneornil(L, 4)) {
		luaL_checktype(L, 4, LUA_TFUNCTION);
		lf.func = 4;
		f = &lf;
	}
	std::set<int> items = wr->queryPoint(x, y, f);
	int n = 0;
	for (std::set<int>::iterator it = items.begin(); it != items.end(); it++) {
		lua_rawgeti(L, -1, *it);
		lua_rawseti(L, -3, ++n);
	}
	lua_pop(L, 1);
	return 1;
}

int worldQuerySegment(lua_State *L) {
	World *wr = (World *) g_getInstance(L, "BumpWorld", 1);
	assertIsRect(L, 2, 3, 4, 5);
	lua_newtable(L);
	lua_getfield(L, 1, "__itemsr");

	double x1 = luaL_checknumber(L, 2);
	double y1 = luaL_checknumber(L, 3);
	double x2 = luaL_checknumber(L, 4);
	double y2 = luaL_checknumber(L, 5);
	ItemFilter *f = NULL;
	LuaItemFilter lf;
	lf.L = L;
	lf.itemsr = -1;
	if (!lua_isnoneornil(L, 6)) {
		luaL_checktype(L, 6, LUA_TFUNCTION);
		lf.func = 6;
		f = &lf;
	}
	std::set<int> items = wr->querySegment(x1, y1, x2, y2, f);
	int n = 0;
	for (std::set<int>::iterator it = items.begin(); it != items.end(); it++) {
		lua_rawgeti(L, -1, *it);
		lua_rawseti(L, -3, ++n);
	}
	lua_pop(L, 1);
	return 1;
}

int worldQuerySegmentWithCoords(lua_State *L) {
	World *wr = (World *) g_getInstance(L, "BumpWorld", 1);
	assertIsRect(L, 2, 3, 4, 5);
	lua_newtable(L);
	lua_getfield(L, 1, "__itemsr");

	double x1 = luaL_checknumber(L, 2);
	double y1 = luaL_checknumber(L, 3);
	double x2 = luaL_checknumber(L, 4);
	double y2 = luaL_checknumber(L, 5);
	ItemFilter *f = NULL;
	LuaItemFilter lf;
	lf.L = L;
	lf.itemsr = -1;
	if (!lua_isnoneornil(L, 6)) {
		luaL_checktype(L, 6, LUA_TFUNCTION);
		lf.func = 6;
		f = &lf;
	}
	std::vector<ItemInfo> items = wr->querySegmentWithCoords(x1, y1, x2, y2, f);
	int n = 0;
	for (std::vector<ItemInfo>::iterator it = items.begin(); it != items.end();
			it++) {
		lua_newtable(L);
		lua_rawgeti(L, -2, (*it).item);
		lua_setfield(L, -2, "item");
		lua_pushnumber(L, (*it).ti1);
		lua_setfield(L, -2, "ti1");
		lua_pushnumber(L, (*it).ti2);
		lua_setfield(L, -2, "ti2");
		lua_pushnumber(L, (*it).x1);
		lua_setfield(L, -2, "x1");
		lua_pushnumber(L, (*it).y1);
		lua_setfield(L, -2, "y1");
		lua_pushnumber(L, (*it).x2);
		lua_setfield(L, -2, "x2");
		lua_pushnumber(L, (*it).y2);
		lua_setfield(L, -2, "y2");
		lua_rawseti(L, -3, ++n);
	}
	lua_pop(L, 1);
	return 1;
}

int worldAdd(lua_State *L) {
	assertIsRect(L, 3, 4, 5,6);
	World *wr = (World *) g_getInstance(L, "BumpWorld", 1);
	lua_getfield(L, 1, "__items");
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	if (!lua_isnil(L, -1)) {
		lua_pushfstring(L, "Item %s added to the world twice.",
				lua_tostring(L, 2));
		lua_error(L);
	}
	lua_pop(L, 1);
	int item = wr->rects.size() + 1;
	double x = luaL_checknumber(L, 3);
	double y = luaL_checknumber(L, 4);
	double w = luaL_checknumber(L, 5);
	double h = luaL_checknumber(L, 6);
	wr->add(item, x, y, w, h);
	lua_pushvalue(L, 2);
	lua_pushinteger(L, item);
	lua_settable(L, -3);
	lua_getfield(L, 1, "__itemsr");
	lua_pushvalue(L, 2);
	lua_rawseti(L, -2, item);
	lua_pop(L, 2);
	lua_pushvalue(L, 2);
	return 1;
}

int worldRemove(lua_State *L) {
	World *wr = (World *) g_getInstance(L, "BumpWorld", 1);
	lua_getfield(L, 1, "__items");
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	if (lua_isnil(L, -1)) {
		lua_pushfstring(L,
				"Item %s must be added to the world before getting its rect. Use world:add(item, x,y,w,h) to add it first.",
				lua_tostring(L, 2));
		lua_error(L);
	}
	int item = lua_tonumber(L, -1);
	lua_pop(L, 1);
	lua_pushvalue(L, 2);
	lua_pushnil(L);
	lua_settable(L, -3);
	lua_getfield(L, 1, "__itemsr");
	lua_pushnil(L);
	lua_rawseti(L, -2, item);
	lua_pop(L, 2);
	wr->remove(item);
	return 0;
}

int worldUpdate(lua_State *L) {
	assertIsRect(L, 3, 4, 5, 6);
	World *wr = (World *) g_getInstance(L, "BumpWorld", 1);
	lua_getfield(L, 1, "__items");
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	if (lua_isnil(L, -1)) {
		lua_pushfstring(L,
				"Item %s must be added to the world before getting its rect. Use world:add(item, x,y,w,h) to add it first.",
				lua_tostring(L, 2));
		lua_error(L);
	}
	int item = lua_tonumber(L, -1);
	lua_pop(L, 2);
	double x = luaL_checknumber(L, 3);
	double y = luaL_checknumber(L, 4);
	double w = luaL_checknumber(L, 5);
	double h = luaL_checknumber(L, 6);
	wr->update(item, x, y, w, h);
	return 0;
}

int worldMove(lua_State *L) {
	World *wr = (World *) g_getInstance(L, "BumpWorld", 1);
	lua_newtable(L);
	lua_getfield(L, 1, "__items");
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	if (lua_isnil(L, -1)) {
		lua_pushfstring(L,
				"Item %s must be added to the world before getting its rect. Use world:add(item, x,y,w,h) to add it first.",
				lua_tostring(L, 2));
		lua_error(L);
	}
	int item = lua_tonumber(L, -1);
	lua_pop(L, 2);
	double x = luaL_checknumber(L, 3);
	double y = luaL_checknumber(L, 4);

	ColFilter *f = NULL;
	LuaColFilter lf;
	lf.L = L;
	lf.itemsr = -1;
	if (!lua_isnoneornil(L, 5)) {
		luaL_checktype(L, 5, LUA_TFUNCTION);
		lf.func = 5;
		f = &lf;
	}
	double ax, ay;
	std::vector<Collision> items = wr->move(item, x, y, f, ax, ay);
	int n = 0;
	for (std::vector<Collision>::iterator it = items.begin(); it != items.end();
			it++) {
		lua_newtable(L);
		lua_rawgeti(L, -2, (*it).item);
		lua_setfield(L, -2, "item");
		lua_rawgeti(L, -2, (*it).other);
		lua_setfield(L, -2, "other");
		lua_pushstring(L, (*it).type);
		lua_setfield(L, -2, "type");
		lua_pushboolean(L, (*it).overlaps);
		lua_setfield(L, -2, "overlaps");
		lua_pushnumber(L, (*it).ti);
		lua_setfield(L, -2, "ti");

		lua_newtable(L);
		lua_pushnumber(L, (*it).move.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, (*it).move.y);
		lua_setfield(L, -2, "y");
		lua_setfield(L, -2, "move");
		lua_newtable(L);
		lua_pushnumber(L, (*it).normal.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, (*it).normal.y);
		lua_setfield(L, -2, "y");
		lua_setfield(L, -2, "normal");
		lua_newtable(L);
		lua_pushnumber(L, (*it).touch.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, (*it).touch.y);
		lua_setfield(L, -2, "y");
		lua_setfield(L, -2, "touch");

		if ((!strcmp((*it).type, "bounce")) || (!strcmp((*it).type, "slide"))) {
			lua_newtable(L);
			lua_pushnumber(L, (*it).response.x);
			lua_setfield(L, -2, "x");
			lua_pushnumber(L, (*it).response.y);
			lua_setfield(L, -2, "y");
			lua_setfield(L, -2, (*it).type);
		}

		lua_newtable(L);
		lua_pushnumber(L, (*it).itemRect.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, (*it).itemRect.y);
		lua_setfield(L, -2, "y");
		lua_pushnumber(L, (*it).itemRect.w);
		lua_setfield(L, -2, "w");
		lua_pushnumber(L, (*it).itemRect.h);
		lua_setfield(L, -2, "h");
		lua_setfield(L, -2, "itemRect");
		lua_newtable(L);
		lua_pushnumber(L, (*it).otherRect.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, (*it).otherRect.y);
		lua_setfield(L, -2, "y");
		lua_pushnumber(L, (*it).otherRect.w);
		lua_setfield(L, -2, "w");
		lua_pushnumber(L, (*it).otherRect.h);
		lua_setfield(L, -2, "h");
		lua_setfield(L, -2, "otherRect");

		lua_rawseti(L, -3, ++n);
	}
	lua_pop(L, 1);
	lua_pushnumber(L, ax);
	lua_pushnumber(L, ay);
	lua_pushvalue(L, -3);
	lua_remove(L, -4);
	lua_pushinteger(L, n);

	return 4;
}

int worldCheck(lua_State *L) {
	World *wr = (World *) g_getInstance(L, "BumpWorld", 1);
	lua_newtable(L);
	lua_getfield(L, 1, "__items");
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	if (lua_isnil(L, -1)) {
		lua_pushfstring(L,
				"Item %s must be added to the world before getting its rect. Use world:add(item, x,y,w,h) to add it first.",
				lua_tostring(L, 2));
		lua_error(L);
	}
	int item = lua_tonumber(L, -1);
	lua_pop(L, 2);
	double x = luaL_checknumber(L, 3);
	double y = luaL_checknumber(L, 4);

	ColFilter *f = NULL;
	LuaColFilter lf;
	lf.L = L;
	lf.itemsr = -1;
	if (!lua_isnoneornil(L, 5)) {
		luaL_checktype(L, 5, LUA_TFUNCTION);
		lf.func = 5;
		f = &lf;
	}
	double ax, ay;
	std::vector<Collision> items = wr->check(item, x, y, f, ax, ay);
	int n = 0;
	for (std::vector<Collision>::iterator it = items.begin(); it != items.end();
			it++) {
		lua_newtable(L);
		lua_rawgeti(L, -2, (*it).item);
		lua_setfield(L, -2, "item");
		lua_rawgeti(L, -2, (*it).other);
		lua_setfield(L, -2, "other");
		lua_pushstring(L, (*it).type);
		lua_setfield(L, -2, "type");
		lua_pushboolean(L, (*it).overlaps);
		lua_setfield(L, -2, "overlaps");
		lua_pushnumber(L, (*it).ti);
		lua_setfield(L, -2, "ti");

		lua_newtable(L);
		lua_pushnumber(L, (*it).move.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, (*it).move.y);
		lua_setfield(L, -2, "y");
		lua_setfield(L, -2, "move");
		lua_newtable(L);
		lua_pushnumber(L, (*it).normal.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, (*it).normal.y);
		lua_setfield(L, -2, "y");
		lua_setfield(L, -2, "normal");
		lua_newtable(L);
		lua_pushnumber(L, (*it).touch.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, (*it).touch.y);
		lua_setfield(L, -2, "y");
		lua_setfield(L, -2, "touch");

		if ((!strcmp((*it).type, "bounce")) || (!strcmp((*it).type, "slide"))) {
			lua_newtable(L);
			lua_pushnumber(L, (*it).response.x);
			lua_setfield(L, -2, "x");
			lua_pushnumber(L, (*it).response.y);
			lua_setfield(L, -2, "y");
			lua_setfield(L, -2, (*it).type);
		}

		lua_newtable(L);
		lua_pushnumber(L, (*it).itemRect.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, (*it).itemRect.y);
		lua_setfield(L, -2, "y");
		lua_pushnumber(L, (*it).itemRect.w);
		lua_setfield(L, -2, "w");
		lua_pushnumber(L, (*it).itemRect.h);
		lua_setfield(L, -2, "h");
		lua_setfield(L, -2, "itemRect");
		lua_newtable(L);
		lua_pushnumber(L, (*it).otherRect.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, (*it).otherRect.y);
		lua_setfield(L, -2, "y");
		lua_pushnumber(L, (*it).otherRect.w);
		lua_setfield(L, -2, "w");
		lua_pushnumber(L, (*it).otherRect.h);
		lua_setfield(L, -2, "h");
		lua_setfield(L, -2, "otherRect");

		lua_rawseti(L, -3, ++n);
	}
	lua_pop(L, 1);
	lua_pushnumber(L, ax);
	lua_pushnumber(L, ay);
	lua_pushvalue(L, -3);
	lua_remove(L, -4);
	lua_pushinteger(L, n);

	return 4;
}

int rectGetNearestCorner(lua_State *L) {
	double x1 = luaL_checknumber(L, 1);
	double y1 = luaL_checknumber(L, 2);
	double w1 = luaL_checknumber(L, 3);
	double h1 = luaL_checknumber(L, 4);
	double px = luaL_checknumber(L, 5);
	double py = luaL_checknumber(L, 6);
	double nx, ny;
	rect_getNearestCorner(x1, y1, w1, h1, px, py, nx, ny);
	lua_pushnumber(L, nx);
	lua_pushnumber(L, ny);
	return 2;
}

int rectGetSegmentIntersectionIndices(lua_State *L) {
	double x1 = luaL_checknumber(L, 1);
	double y1 = luaL_checknumber(L, 2);
	double w1 = luaL_checknumber(L, 3);
	double h1 = luaL_checknumber(L, 4);
	double x2 = luaL_checknumber(L, 5);
	double y2 = luaL_checknumber(L, 6);
	double w2 = luaL_checknumber(L, 7);
	double h2 = luaL_checknumber(L, 8);
	double ti1, ti2, nx1, ny1, nx2, ny2;
	rect_getSegmentIntersectionIndices(x1, y1, w1, h1, x2, y2, w2, h2, ti1, ti2,
			nx1, ny1, nx2, ny2);
	lua_pushnumber(L, ti1);
	lua_pushnumber(L, ti2);
	lua_pushnumber(L, nx1);
	lua_pushnumber(L, ny1);
	lua_pushnumber(L, nx2);
	lua_pushnumber(L, ny2);
	return 6;
}

int rectGetDiff(lua_State *L) {
	double x1 = luaL_checknumber(L, 1);
	double y1 = luaL_checknumber(L, 2);
	double w1 = luaL_checknumber(L, 3);
	double h1 = luaL_checknumber(L, 4);
	double x2 = luaL_checknumber(L, 5);
	double y2 = luaL_checknumber(L, 6);
	double w2 = luaL_checknumber(L, 7);
	double h2 = luaL_checknumber(L, 8);

	double x, y, w, h;

	rect_getDiff(x1, y1, w1, h1, x2, y2, w2, h2, x, y, w, h);

	lua_pushnumber(L, x);
	lua_pushnumber(L, y);
	lua_pushnumber(L, w);
	lua_pushnumber(L, h);
	return 4;
}

int rectContainsPoint(lua_State *L) {
	double x1 = luaL_checknumber(L, 1);
	double y1 = luaL_checknumber(L, 2);
	double w1 = luaL_checknumber(L, 3);
	double h1 = luaL_checknumber(L, 4);
	double x2 = luaL_checknumber(L, 5);
	double y2 = luaL_checknumber(L, 6);
	lua_pushboolean(L, rect_containsPoint(x1, y1, w1, h1, x2, y2));
	return 1;
}

int rectIsIntersecting(lua_State *L) {
	double x1 = luaL_checknumber(L, 1);
	double y1 = luaL_checknumber(L, 2);
	double w1 = luaL_checknumber(L, 3);
	double h1 = luaL_checknumber(L, 4);
	double x2 = luaL_checknumber(L, 5);
	double y2 = luaL_checknumber(L, 6);
	double w2 = luaL_checknumber(L, 7);
	double h2 = luaL_checknumber(L, 8);
	lua_pushboolean(L, rect_isIntersecting(x1, y1, w1, h1, x2, y2, w2, h2));
	return 1;
}

int rectGetSquareDistance(lua_State *L) {
	double x1 = luaL_checknumber(L, 1);
	double y1 = luaL_checknumber(L, 2);
	double w1 = luaL_checknumber(L, 3);
	double h1 = luaL_checknumber(L, 4);
	double x2 = luaL_checknumber(L, 5);
	double y2 = luaL_checknumber(L, 6);
	double w2 = luaL_checknumber(L, 7);
	double h2 = luaL_checknumber(L, 8);
	lua_pushnumber(L, rect_getSquareDistance(x1, y1, w1, h1, x2, y2, w2, h2));
	return 1;
}

int rectDetectCollision(lua_State *L) {
	double x1 = luaL_checknumber(L, 1);
	double y1 = luaL_checknumber(L, 2);
	double w1 = luaL_checknumber(L, 3);
	double h1 = luaL_checknumber(L, 4);
	double x2 = luaL_checknumber(L, 5);
	double y2 = luaL_checknumber(L, 6);
	double w2 = luaL_checknumber(L, 7);
	double h2 = luaL_checknumber(L, 8);
	double gx = luaL_checknumber(L, 9);
	double gy = luaL_checknumber(L, 10);
	Collision col;
	if (rect_detectCollision(x1, y1, w1, h1, x2, y2, w2, h2, gx, gy, col)) {
		lua_newtable(L);
		lua_pushboolean(L, col.overlaps);
		lua_setfield(L, -2, "overlaps");
		lua_pushnumber(L, col.ti);
		lua_setfield(L, -2, "ti");

		lua_newtable(L);
		lua_pushnumber(L, col.move.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, col.move.y);
		lua_setfield(L, -2, "y");
		lua_setfield(L, -2, "move");
		lua_newtable(L);
		lua_pushnumber(L, col.normal.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, col.normal.y);
		lua_setfield(L, -2, "y");
		lua_setfield(L, -2, "normal");
		lua_newtable(L);
		lua_pushnumber(L, col.touch.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, col.touch.y);
		lua_setfield(L, -2, "y");
		lua_setfield(L, -2, "touch");

		lua_newtable(L);
		lua_pushnumber(L, col.itemRect.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, col.itemRect.y);
		lua_setfield(L, -2, "y");
		lua_pushnumber(L, col.itemRect.w);
		lua_setfield(L, -2, "w");
		lua_pushnumber(L, col.itemRect.h);
		lua_setfield(L, -2, "h");
		lua_setfield(L, -2, "itemRect");
		lua_newtable(L);
		lua_pushnumber(L, col.otherRect.x);
		lua_setfield(L, -2, "x");
		lua_pushnumber(L, col.otherRect.y);
		lua_setfield(L, -2, "y");
		lua_pushnumber(L, col.otherRect.w);
		lua_setfield(L, -2, "w");
		lua_pushnumber(L, col.otherRect.h);
		lua_setfield(L, -2, "h");
		lua_setfield(L, -2, "otherRect");
		return 1;
	}
	return 0;
}

int bumpNewWorld(lua_State *L) {
	int cs = luaL_optinteger(L, 1, 64);
	assertIsPositiveNumber(L, cs, "cellSize");
	lua_getglobal(L, "BumpWorld");
	lua_getfield(L, -1, "new");
	lua_call(L, 0, 1);
	World *w = (World *) g_getInstance(L, "BumpWorld", -1);
	w->cellSize = cs;
	return 1;
}

int loader(lua_State *L) {
	const luaL_Reg worldFuncs[] =
			{
					//    {"addResponse", worldAddResponse}, Don't implement
					{ "project", worldProject },
					{ "countCells", worldCountCells },
					{ "hasItem", worldHasItem }, { "getItems", worldGetItems },
					{ "countItems", worldCountItems },
					{ "getRect", worldGetRect }, { "toWorld", worldToWorld }, {
							"toCell", worldToCell }, { "queryRect",
							worldQueryRect }, { "queryPoint", worldQueryPoint },
					{ "querySegment", worldQuerySegment }, {
							"querySegmentWithCoords",
							worldQuerySegmentWithCoords }, { "add", worldAdd },
					{ "remove", worldRemove }, { "update", worldUpdate }, {
							"move", worldMove }, { "check", worldCheck }, {
							NULL, NULL }, };

	g_createClass(L, "BumpWorld", NULL, worldCreate, worldDestruct, worldFuncs);

	const luaL_Reg bumpFuncs[] =
			{ { "newWorld", bumpNewWorld }, { NULL, NULL }, };

	lua_newtable(L);
	luaL_register(L, NULL, bumpFuncs);

	const luaL_Reg rectFuncs[] =
			{ { "getNearestCorner", rectGetNearestCorner }, {
					"getSegmentIntersectionIndices",
					rectGetSegmentIntersectionIndices }, { "getDiff",
					rectGetDiff }, { "containsPoint", rectContainsPoint }, {
					"isIntersecting", rectIsIntersecting }, {
					"getSquareDistance", rectGetSquareDistance }, {
					"detectCollision", rectDetectCollision }, { NULL, NULL }, };

	lua_newtable(L);
	luaL_register(L, NULL, rectFuncs);
	lua_setfield(L, -2, "rect");

	return 1;
}

static void g_initializePlugin(lua_State *L) {
	lua_getglobal(L, "package");
	lua_getfield(L, -1, "preload");

	lua_pushcfunction(L, loader);
	lua_setfield(L, -2, "bump");

	lua_pop(L, 2);
}

static void g_deinitializePlugin(lua_State *L) {
}
REGISTER_PLUGIN_NAMED("Bump", "3.1.7", bump)
