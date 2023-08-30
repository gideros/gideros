#include "lua.hpp"
#include <vector>
#include <cmath>
#include <cfloat>
#include <algorithm>
#include <string.h>

#include "dj_fft.h"

struct VEC {
	double x;
	double y;
	double z;
};

enum VECTYPE {
	VT_ARGS,
	VT_ARGS3,
	VT_ARRAY,
	VT_ARRAY3,
	VT_TABLE,
	VT_TABLE3,
	VT_VECTOR,
	VT_VECTOR3,
};

namespace GidMath {

class Shape {
public:
	struct Hit {
		double distance;
		VEC point;
		VEC normal;
		VEC reflect;
	};
protected:
	void addHit(VEC o,VEC d,std::vector<Hit> &hits,double l);
public:
	virtual ~Shape() {};
	virtual double inside(VEC p)=0;
	virtual VEC nearest(VEC p)=0;
	virtual void raycast(VEC o,VEC d,std::vector<Hit> &hits)=0;
	virtual void computeNormal(Hit &h)=0;
};

void Shape::addHit(VEC o,VEC d,std::vector<Hit> &hits,double l)
{
    if (l<0) return;
	Hit h;
	h.distance=l;
	h.point.x=o.x+d.x*l;
	h.point.y=o.y+d.y*l;
	h.point.z=o.z+d.z*l;
	computeNormal(h);
	hits.push_back(h);
}

class ShapeGroup : public Shape {
	std::vector<Shape *> shapes;
	VEC origin;
public:
	ShapeGroup(VEC o) {
		origin=o;
	}
	~ShapeGroup() {
		for (std::vector<Shape *>::iterator it=shapes.begin();it!=shapes.end();it++)
			delete (*it);
	}
	void add(Shape *s) { shapes.push_back(s); }
    virtual void computeNormal(Hit &) { };
	double inside(VEC p) {
		p.x-=origin.x;
		p.y-=origin.y;
		p.z-=origin.z;
		double dd=DBL_MAX;
		for (std::vector<Shape *>::iterator it=shapes.begin();it!=shapes.end();it++) {
            double dist=(*it)->inside(p);
			if (dist<dd) dd=dist;
		}
		return dd;
	}
	VEC nearest(VEC p) {
		VEC v;
		p.x-=origin.x;
		p.y-=origin.y;
		p.z-=origin.z;
		double dd=DBL_MAX;
		for (std::vector<Shape *>::iterator it=shapes.begin();it!=shapes.end();it++) {
            VEC vn=(*it)->nearest(p);
			double dist=(vn.x-p.x)*(vn.x-p.x)+(vn.y-p.y)*(vn.y-p.y)+(vn.z-p.z)*(vn.z-p.z);
			if (dist<dd) { v=vn; dd=dist; }
		}
		v.x+=origin.x;
		v.y+=origin.y;
		v.z+=origin.z;
		return v;
	}
    void raycast(VEC o,VEC d,std::vector<Hit> &hits) {
    	std::vector<Shape::Hit> lhits;
        o.x-=origin.x;
        o.y-=origin.y;
        o.z-=origin.z;
		for (std::vector<Shape *>::iterator it=shapes.begin();it!=shapes.end();it++)
            (*it)->raycast(o,d,lhits);
   		for (std::vector<Shape::Hit>::iterator it=lhits.begin();it!=lhits.end();it++) {
   			it->point.x+=origin.x;
   			it->point.y+=origin.y;
   			it->point.z+=origin.z;
   			hits.push_back(*it);
   		}
	}
};

class ShapeSphere : public Shape {
	VEC center;
	double radius;
public:
	ShapeSphere(VEC c,double r) {
		center=c;
		radius=r;
	}
	double inside(VEC p) {
		VEC v;
		v.x=p.x-center.x;
		v.y=p.y-center.y;
		v.z=p.z-center.z;
		double d=sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
		return d-radius;
	}
	VEC nearest(VEC p) {
		VEC v;
		v.x=p.x-center.x;
		v.y=p.y-center.y;
		v.z=p.z-center.z;
		double d=radius/sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
		v.x=center.x+v.x*d;
		v.y=center.y+v.y*d;
		v.z=center.z+v.z*d;
		return v;
	}
    void raycast(VEC o,VEC d,std::vector<Hit> &hits) {
		double a=d.x*d.x+d.y*d.y+d.z*d.z;
        if (a<0) return;
        double b=2*((o.x-center.x)*d.x+(o.y-center.y)*d.y+(o.z-center.z)*d.z);
        double c=o.x*(o.x-2*center.x)+o.y*(o.y-2*center.y)+o.z*(o.z-2*center.z)-radius*radius
                +center.x*center.x+center.y*center.y+center.z*center.z;
		double delta=b*b-4*a*c;
		if (delta>0)
		{
			double sd=sqrt(delta);
            addHit(o,d,hits,(-b-sd)/(2*a));
            addHit(o,d,hits,(-b+sd)/(2*a));
		}
		else {
			if (delta==0)
                addHit(o,d,hits,-b/(2*a));
		}
	}

	void computeNormal(Hit &h) {
		VEC n;
		n.x=h.point.x-center.x;
		n.y=h.point.y-center.y;
		n.z=h.point.z-center.z;
		double nl=sqrt(n.x*n.x+n.y*n.y+n.z*n.z);
		h.normal.x=n.x/nl;
		h.normal.y=n.y/nl;
		h.normal.z=n.z/nl;
	}
};

class ShapePlane : public Shape {
	VEC center;
	VEC normal;
	double extent;
public:
	ShapePlane(VEC c,VEC n,double e) {
		center=c;
		normal=n;
		extent=e;
	}
	double inside(VEC p) {
		double d;
		d=(p.x-center.x)*normal.x;
		d+=(p.y-center.y)*normal.y;
		d+=(p.z-center.z)*normal.z;
		return d;
	}
	VEC nearest(VEC p) {
		VEC v;
		v.x=p.x-center.x;
		v.y=p.y-center.y;
		v.z=p.z-center.z;
		double d;
		d=v.x*normal.x;
		d+=v.y*normal.y;
		d+=v.z*normal.z;
		v.x=p.x-normal.x*d;
        v.y=p.y-normal.y*d;
        v.z=p.z-normal.z*d;
		if (extent) {
			VEC r;
			r.x=v.x-center.x;
			r.y=v.y-center.y;
			r.z=v.z-center.z;
			double d=sqrt(r.x*r.x+r.y*r.y+r.z*r.z);
			if (d>extent) {
				double s=extent/d;
				v.x=center.x+r.x*s;
				v.y=center.y+r.y*s;
				v.z=center.z+r.z*s;
			}
		}
		return v;
	}
	void raycast(VEC o,VEC d,std::vector<Hit> &hits) {
		double dn;
		dn=d.x*normal.x;
		dn+=d.y*normal.y;
		dn+=d.z*normal.z;
		if (dn) {
			double on;
			on=(center.x-o.x)*normal.x;
			on+=(center.y-o.y)*normal.y;
			on+=(center.z-o.z)*normal.z;
            on/=dn;
			if (extent) {
				VEC r;
				r.x=o.x+d.x*on-center.x;
				r.y=o.y+d.y*on-center.y;
				r.z=o.z+d.z*on-center.z;
				double rd=sqrt(r.x*r.x+r.y*r.y+r.z*r.z);
				if (rd<=extent)
                    addHit(o,d,hits,on);
			}
			else
				addHit(o,d,hits,on/dn);
		}
	}

	void computeNormal(Hit &h) {
		h.normal=normal;
	}
};

#define abs_index(L, i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : \
                                        lua_gettop(L) + (i) + 1)

static VECTYPE probeVec(lua_State *L,int idx,int d3) {
    idx=abs_index(L,idx);
	const float *vf=lua_tovector(L,idx);
	if (vf)
		return std::isnan(vf[2])?VT_VECTOR:VT_VECTOR3;
	if (lua_type(L,idx)==LUA_TTABLE) {
	  	  lua_getfield(L,idx,"x");
	  	  bool arr=lua_isnil(L,-1);
	  	  lua_getfield(L,idx,"z");
	  	  bool arr3=lua_isnil(L,-1);
	  	  lua_pop(L,2);
	  	  if (!arr3) return VT_TABLE3;
	  	  if (!arr) return VT_TABLE;
	  	  return (lua_objlen(L,idx)>2)?VT_ARRAY3:VT_ARRAY;
  	}
  	return lua_isnoneornil(L,d3)?VT_ARGS:VT_ARGS3;
}

static int getVec(lua_State *L,int idx,VECTYPE vt,VEC &v) {
    idx=abs_index(L,idx);
    if ((vt==VT_ARGS)||(vt==VT_ARGS3)) {
		v.x=luaL_checknumber(L,idx);
		v.y=luaL_checknumber(L,idx+1);
		v.z=(vt==VT_ARGS3)?luaL_checknumber(L,idx+2):0;
		return idx+((vt==VT_ARGS3)?3:2);
	}
	else {
		const float *vf=lua_tovector(L,idx);
		if (vf) {
			v.x=vf[0];
			v.y=vf[1];
			v.z=vf[2];
		}
		else {
			luaL_checktype(L,idx,LUA_TTABLE);
			if ((vt==VT_ARRAY)||(vt==VT_ARRAY3)) {
				lua_rawgeti(L,idx,1);
				lua_rawgeti(L,idx,2);
				lua_rawgeti(L,idx,3);
			}
			else {
				lua_getfield(L,idx,"x");
				lua_getfield(L,idx,"y");
				lua_getfield(L,idx,"z");
			}
			v.x=luaL_optnumber(L,-3,0);
			v.y=luaL_optnumber(L,-2,0);
			v.z=luaL_optnumber(L,-1,0);
			lua_pop(L,3);
		}
		return idx+1;
	}
}

static int pushVec(lua_State *L,VECTYPE vt,VEC v) {
	if ((vt==VT_ARGS)||(vt==VT_ARGS3)) {
		lua_pushnumber(L,v.x);
		lua_pushnumber(L,v.y);
		if (vt==VT_ARGS) return 2;
		lua_pushnumber(L,v.z);
		return 3;
	}
	else {
		if (vt==VT_VECTOR)
			lua_pushvector(L,v.x,v.y,nan(""),nan(""));
		else if (vt==VT_VECTOR3)
			lua_pushvector(L,v.x,v.y,v.z,nan(""));
		else if ((vt==VT_ARRAY)||(vt==VT_ARRAY3)) {
			lua_createtable(L,3,0);
			lua_pushnumber(L,v.x);
			lua_rawseti(L,-2,1);
			lua_pushnumber(L,v.y);
			lua_rawseti(L,-2,2);
			if (vt==VT_ARRAY) return 1;
			lua_pushnumber(L,v.z);
			lua_rawseti(L,-2,3);
		}
		else {
			lua_createtable(L,0,3);
			lua_pushnumber(L,v.x);
			lua_setfield(L,-2,"x");
			lua_pushnumber(L,v.y);
			lua_setfield(L,-2,"y");
			if (vt==VT_TABLE) return 1;
			lua_pushnumber(L,v.z);
			lua_setfield(L,-2,"z");
		}
		return 1;
	}
}

static Shape *getShape(lua_State *L,int idx,int ti) {
    idx=abs_index(L,idx);
    if (lua_type(L,idx)!=LUA_TTABLE)
	{
		if (ti)
			lua_pushstring(L,"Shape definition must be a table");
		else
			lua_pushfstring(L,"Shape definition must be a table (at index %d)",ti);
		lua_error(L);
	}
	//Test for sphere
	lua_getfield(L,idx,"radius");
	if (!lua_isnil(L,-1)) {
		double radius=luaL_optnumber(L,-1,1);
		lua_pop(L,1);
		VEC ctr;
		getVec(L,idx,VT_TABLE3,ctr);
		return new ShapeSphere(ctr,radius);
	}
	lua_pop(L,1);
	//Test for plane
	lua_getfield(L,idx,"normal");
	if (!lua_isnil(L,-1)) {
		VEC n;
		getVec(L,-1,VT_TABLE3,n);
		lua_getfield(L,idx,"extent");
		double extent=luaL_optnumber(L,-1,0);
		lua_pop(L,2);
		VEC ctr;
		getVec(L,idx,VT_TABLE3,ctr);
		return new ShapePlane(ctr,n,extent);
	}
	lua_pop(L,1);
    //Check for a 2D line
    lua_getfield(L,idx,"dx");
    if (!lua_isnil(L,-1)) {
        double dx=luaL_optnumber(L,-1,0);
        lua_getfield(L,idx,"dy");
        double dy=luaL_optnumber(L,-1,0);
        lua_pop(L,2);
        VEC ctr,n;
        getVec(L,idx,VT_TABLE3,ctr);
        n.x=dy;
        n.y=-dx;
        n.z=0;
        return new ShapePlane(ctr,n,0);
    }
    lua_pop(L,1);
    //Check for a 2D segment
    lua_getfield(L,idx,"x2");
    if (!lua_isnil(L,-1)) {
        double x2=luaL_optnumber(L,-1,0);
        lua_getfield(L,idx,"y2");
        double y2=luaL_optnumber(L,-1,0);
        lua_pop(L,2);
        VEC ctr,n;
        getVec(L,idx,VT_TABLE3,ctr);
        double dx=x2-ctr.x;
        double dy=y2-ctr.y;
        ctr.x+=dx/2;
        ctr.y+=dy/2;
        double dn=sqrt(dx*dx+dy*dy);
        n.x=dy/dn;
        n.y=-dx/dn;
        n.z=0;
        return new ShapePlane(ctr,n,dn/2);
    }
    lua_pop(L,1);

    //Not a specific shape, assume a group
    VEC origin;
    getVec(L,idx,VT_TABLE3,origin);
    ShapeGroup *group=new ShapeGroup(origin);
	int ln=lua_objlen(L,idx);
	for (int in=1;in<=ln;in++) {
		lua_rawgeti(L,idx,in);
		Shape *s=getShape(L,-1,in);
		lua_pop(L,1);
		group->add(s);
	}
	return group;
}

}

using namespace GidMath;
static int math_length (lua_State *L) {
	VEC v;
	VECTYPE vt=probeVec(L,1,3);
	getVec(L,1,vt,v);
	lua_pushnumber(L, sqrt(v.x*v.x+v.y*v.y+v.z*v.z));
	return 1;
}

static int math_distance (lua_State *L) {
    VEC v1,v;
    VECTYPE vt=probeVec(L,1,5);
    int idx=getVec(L,1,vt,v1);
    getVec(L,idx,vt,v);
    v.x-=v1.x; v.y-=v1.y; v.z-=v1.z;
    lua_pushnumber(L, sqrt(v.x*v.x+v.y*v.y+v.z*v.z));
    return 1;
}

static int math_cross (lua_State *L) {
    VEC u,v,c;
    VECTYPE vt=probeVec(L,1,5);
    int idx=getVec(L,1,vt,u);
    getVec(L,idx,vt,v);
    c.x=u.y*v.z-u.z*v.y;
    c.y=u.z*v.x-u.x*v.z;
    c.z=u.x*v.y-u.y*v.x;
    if (vt==VT_ARGS) vt=VT_ARGS3;
    if (vt==VT_ARRAY) vt=VT_ARRAY3;
    if (vt==VT_TABLE) vt=VT_TABLE3;
    if (vt==VT_VECTOR) vt=VT_VECTOR3;
    return pushVec(L,vt,c);
}

static int math_dot (lua_State *L) {
    VEC v1,v;
    VECTYPE vt=probeVec(L,1,5);
    int idx=getVec(L,1,vt,v1);
    getVec(L,idx,vt,v);
    lua_pushnumber(L, v.x*v1.x+v.y*v1.y+v.z*v1.z);
    return 1;
}

static bool dist_sorta(std::pair<int,double> a,std::pair<int,double> b) { return (a.second<b.second); }
static bool dist_sortb(std::pair<int,double> a,std::pair<int,double> b) { return (b.second<a.second); }
static bool hit_sort(Shape::Hit a,Shape::Hit b) { return (a.distance<b.distance); }

static int math_distances (lua_State *L) {
	VEC v1,v;
	luaL_checktype(L,1,LUA_TTABLE);
	luaL_checktype(L,2,LUA_TTABLE);
	int sort=luaL_optinteger(L,3,0);
	VECTYPE vt=probeVec(L,1,1);
	getVec(L,1,vt,v1);

	int ln=lua_objlen(L,2);
	lua_createtable(L,ln,0);
	std::vector<std::pair<int,double>> dists;
	for (int in=1;in<=ln;in++) {
		lua_rawgeti(L,2,in);
		getVec(L,-1,vt,v);
		lua_pop(L,1);
		v.x-=v1.x; v.y-=v1.y; v.z-=v1.z;
		double dist=sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
		if (sort)
			dists.push_back(std::pair<int,double>(in,dist));
		else {
			lua_pushnumber(L, dist);
			lua_rawseti(L,-2,in);
		}
	}
	if (sort) {
		std::sort(dists.begin(),dists.end(),(sort>0)?dist_sorta:dist_sortb);
		for (int in=1;in<=ln;in++) {
			lua_createtable(L,2,0);
            lua_rawgeti(L,2,dists[in-1].first);
			lua_rawseti(L,-2,1);
            lua_pushnumber(L, dists[in-1].second);
			lua_rawseti(L,-2,2);
			lua_rawseti(L,-2,in);
		}
	}
	return 1;
}

static int math_nearest (lua_State *L) {
	VEC v1,v;
	luaL_checktype(L,1,LUA_TTABLE);
	luaL_checktype(L,2,LUA_TTABLE);
	VECTYPE vt=probeVec(L,1,1);
	getVec(L,1,vt,v1);

	int ln=lua_objlen(L,2);
	int nr=0;
	double dd=DBL_MAX;
	for (int in=1;in<=ln;in++) {
		lua_rawgeti(L,2,in);
		getVec(L,-1,vt,v);
		lua_pop(L,1);
		v.x-=v1.x; v.y-=v1.y; v.z-=v1.z;
		double dist=sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
		if (dist<dd) {
			nr=in;
			dd=dist;
		}
	}
	if (nr) {
		lua_rawgeti(L,2,nr);
		lua_pushnumber(L,dd);
		return 2;
	}
	return 0;
}

static int math_normalize (lua_State *L) {
	VEC v;
	VECTYPE vt=probeVec(L,1,3);
	getVec(L,1,vt,v);
	double l=sqrt(v.x*v.x+v.y*v.y+v.z*v.z);
	if (l > 0)
	{
		v.x/=l;
		v.y/=l;
		v.z/=l;
	}
	return pushVec(L,vt,v);
}

static int math_raycast (lua_State *L) {
	VEC vo,vn;
	VECTYPE vt=probeVec(L,1,5);
    int optpos=4;
    if (vt==VT_ARGS) optpos=6;
    if (vt==VT_ARGS3) optpos=8;
    int limit=luaL_optnumber(L,optpos,0);
    int idx=getVec(L,1,vt,vo);
	idx=getVec(L,idx,vt,vn);
	std::vector<Shape::Hit> hits;
	Shape *s=getShape(L,idx,0);
    s->raycast(vo, vn, hits);
	delete s;
    std::sort(hits.begin(),hits.end(),hit_sort);
    lua_createtable(L,hits.size(),0);
	if (hits.size()) {
		int hn=0;
		if (vt==VT_ARGS) vt=VT_TABLE;
		if (vt==VT_ARGS3) vt=VT_TABLE3;
		for (std::vector<Shape::Hit>::iterator it=hits.begin();it!=hits.end();it++) {
			lua_createtable(L,0,4);
			lua_pushnumber(L,it->distance);
			lua_setfield(L,-2,"distance");
			pushVec(L,vt,it->point);
			lua_setfield(L,-2,"point");
			pushVec(L,vt,it->normal);
			lua_setfield(L,-2,"normal");
            double dn=2*(vn.x*it->normal.x+vn.y*it->normal.y+vn.z*it->normal.z);
            it->reflect.x=vn.x-dn*it->normal.x;
            it->reflect.y=vn.y-dn*it->normal.y;
            it->reflect.z=vn.z-dn*it->normal.z;
			pushVec(L,vt,it->reflect);
			lua_setfield(L,-2,"reflect");
			lua_rawseti(L,-2,++hn);
            if (hn==limit) break;
		}
	}
	return 1;
}

static int math_inside (lua_State *L) {
	VEC v;
	VECTYPE vt=probeVec(L,1,3);
	int idx=getVec(L,1,vt,v);
	Shape *s=getShape(L,idx,0);
	lua_pushnumber(L,s->inside(v));
	delete s;
	return 1;
}

static int math_edge (lua_State *L) {
	VEC v;
	VECTYPE vt=probeVec(L,1,3);
	int idx=getVec(L,1,vt,v);
	Shape *s=getShape(L,idx,0);
	int rs=pushVec(L,vt,s->nearest(v));
	delete s;
	return rs;
}

//FFT
static int lua_fftop(lua_State *L,bool inv)
{
    luaL_checktype(L,1,LUA_TTABLE); //REAL PART
    int hasI=(lua_type(L,2)==LUA_TTABLE); //IMAGINARY PART
    int hasW=(lua_type(L,3)==LUA_TTABLE); //WINDOW
    int nlen=lua_objlen(L,1);
	if (nlen&(nlen-1)) {
		lua_pushfstring(L,"Input array size must be a power of two, got %d",nlen);
		lua_error(L);
	}
    if (hasI&&(lua_objlen(L,2)!=nlen)) {
        lua_pushfstring(L,"Input arrays must have the same size (imaginary mismatch)");
        lua_error(L);
    }
    if (hasW&&(lua_objlen(L,3)!=nlen)) {
        lua_pushfstring(L,"Input arrays must have the same size (window mismatch)");
        lua_error(L);
    }
    std::vector<std::complex<double>> dd;
    int npop=(hasI?2:1)+(hasW?1:0);
    for (int k=1;k<=nlen;k++) {
		lua_rawgeti(L,1,k);
		double r=lua_tonumber(L,-1);
		double i=0;
        if (hasI) {
            lua_rawgeti(L,2,k);
            i=lua_tonumber(L,-1);
        }
        if (hasW) {
            lua_rawgeti(L,3,k);
            r*=lua_tonumber(L,-1);
        }
        lua_pop(L,npop);
		dd.push_back(std::complex<double>(r,i));
	}
    std::vector<std::complex<double>> dout=dj::fft1d(dd,inv?dj::fft_dir::DIR_BWD:dj::fft_dir::DIR_FWD);
	lua_createtable(L,nlen,0);
	lua_createtable(L,nlen,0);
    for (int k=1;k<=nlen;k++) {
        lua_pushnumber(L,dout[k-1].real());
        lua_rawseti(L,-3,k);
        lua_pushnumber(L,dout[k-1].imag());
        lua_rawseti(L,-2,k);
	}

    return 2;
}

static int lua_fft(lua_State* L)
{
	return lua_fftop(L,false);
}

static int lua_ifft(lua_State* L)
{
	return lua_fftop(L,true);
}

//LUAU vector
static int lua_vector(lua_State* L)
{
    double x = luaL_checknumber(L, 1);
    double y = luaL_checknumber(L, 2);
    double z = luaL_optnumber(L, 3, nan(""));

#if LUA_VECTOR_SIZE == 4
    double w = luaL_optnumber(L, 4, nan(""));
    lua_pushvector(L, float(x), float(y), float(z), float(w));
#else
    lua_pushvector(L, float(x), float(y), float(z));
#endif
    return 1;
}

static int lua_vector_dot(lua_State* L)
{
    const float* a = luaL_checkvector(L, 1);
    const float* b = luaL_checkvector(L, 2);

    lua_pushnumber(L, a[0] * b[0] + a[1] * b[1] + a[2] * b[2]);
    return 1;
}

static int lua_vector_index(lua_State* L)
{
    const float* v = luaL_checkvector(L, 1);
    const char* name = luaL_checkstring(L, 2);

    if (strcmp(name, "Magnitude") == 0)
    {
        lua_pushnumber(L, sqrtf(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]));
        return 1;
    }

    if (strcmp(name, "Dot") == 0)
    {
        lua_pushcfunction(L, lua_vector_dot, "Dot");
        return 1;
    }

    luaL_error(L, "%s is not a valid member of vector", name);
}

static int lua_vector_namecall(lua_State* L)
{
    if (const char* str = lua_namecallatom(L, nullptr))
    {
        if (strcmp(str, "Dot") == 0)
            return lua_vector_dot(L);
    }

    luaL_error(L, "%s is not a valid method of vector", luaL_checkstring(L, 1));
}


void register_gideros_math(lua_State *L) {
	// Register math helpers
	static const luaL_Reg mathlib[] = {
      {"length",   math_length},
      {"cross",   math_cross},
      {"dot",   math_dot},
      {"distance",  math_distance},
	  {"distances",  math_distances},
	  {"nearest",  math_nearest},
	  {"normalize",  math_normalize},
	  {"raycast", math_raycast},
	  {"inside", math_inside},
	  {"edge", math_edge},
	  {"vector", lua_vector},
	  {"fft", lua_fft},
	  {"ifft", lua_ifft},
	  {NULL, NULL}
	};
	luaL_register(L, LUA_MATHLIBNAME, mathlib);
	lua_pop(L,1);

    lua_pushcfunction(L, lua_vector, "vector");
    lua_setglobal(L, "vector");

#if LUA_VECTOR_SIZE == 4
    lua_pushvector(L, 0.0f, 0.0f, 0.0f, 0.0f);
#else
    lua_pushvector(L, 0.0f, 0.0f, 0.0f);
#endif
    luaL_newmetatable(L, "vector");

    lua_pushstring(L, "__index");
    lua_pushcfunction(L, lua_vector_index, "vector__index");
    lua_settable(L, -3);

    lua_pushstring(L, "__namecall");
    lua_pushcfunction(L, lua_vector_namecall, "vector__namecall");
    lua_settable(L, -3);

    lua_setmetatable(L, -2);
    lua_pop(L, 1);
}
