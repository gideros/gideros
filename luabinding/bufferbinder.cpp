#include "bufferbinder.h"
#include "stackchecker.h"
#include <gpath.h>
#include "refptr.h"
#include <vector>
#include <string>
#include <errno.h>
#include "glog.h"

class Buffer : public GReferenced
{
public:
	Buffer(std::string sid,bool _stream) {
		id=sid;
		opened=false;
		stream=_stream;
		pos=0;
	}
	virtual ~Buffer() {
	}
	size_t append(const char *data,size_t size) {
		size_t c=d.size();
		d.resize(c+size);
		memcpy(&d[c],data,size);
		return c+size;
	}
	size_t prepend(const char *data,size_t size) {
		size_t c=d.size();
		d.resize(c+size);
		memmove(&d[size],&d[0],c);
		memcpy(&d[0],data,size);
		pos+=size;
		return c+size;
	}
	size_t trim(int a) {
		size_t c=d.size();
		if (a>0) {
			if (a>c) a=c;
			memmove(&d[0],&d[a],c-a);
			d.resize(c-a);
			if (a>pos) pos=0; else pos-=a;
			return c-a;
		}
		else {
			d.resize(c+a);
			return c+a;
		}
	}
	const char *get(size_t o,size_t &size) {
		size_t c=d.size();
		if (o>c) { size=0; return NULL; }
		c-=o;
		if (size>c) size=c;
		return &d[o];
	}
	size_t set(const char *data,size_t o,size_t size) {
		size_t c=d.size();
		size_t m=o+size;
		if (m>c)
			d.resize(m);
		else
			m=c;
		memcpy(&d[o],data,size);
		return m;
	}
	size_t size() { return d.size(); }
	std::string id;
	bool opened;
	bool stream;
	size_t pos;
private:
	std::vector<char> d;
};

static std::map<std::string,Buffer *> bufferMap;
static std::vector<Buffer *> fdMap;

static int s_open(const char *pathname, int flags) {
	std::string sid(pathname+3);
	Buffer *b=bufferMap[sid];
	if (b==NULL) /* sanity check */
	{
		errno = ENOENT;
		return -1;
	}
	if (b->opened) {
		errno = EACCES;
		return -1;
	}
	b->opened=true;
	b->pos=0;
	fdMap.push_back(b);
	int fd=fdMap.size()|0x4000;
	return fd;
}

static int s_close(int fd) {
	fd&=0x0FFF;	Buffer *b=(fd>0)&&(fd<=fdMap.size())?fdMap[fd-1]:NULL;
	if (b==NULL) /* sanity check */
	{
		errno = EBADF;
		return -1;
	}
	b->opened=false;
	fdMap[fd-1]=NULL;
	while ((!fdMap.empty())&&(!fdMap.back()))
		fdMap.erase(fdMap.end()-1);
	return 0;
}

static size_t s_write(int fd, const void* buf, size_t count) {
	fd&=0x0FFF;	Buffer *b=(fd>0)&&(fd<=fdMap.size())?fdMap[fd-1]:NULL;
	if (b==NULL) /* sanity check */
	{
		errno = EBADF;
		return -1;
	}
	b->set((const char *)buf,b->pos,count);
	b->pos+=count;
	return count;
}

static size_t s_read(int fd, void* buf, size_t count) {
	fd&=0x0FFF;	Buffer *b=(fd>0)&&(fd<=fdMap.size())?fdMap[fd-1]:NULL;
	if (b==NULL) /* sanity check */
	{
		errno = EBADF;
		return -1;
	}
	const char *data=b->get(b->pos,count);
	b->pos+=count;
	memcpy(buf,data,count);
	return count;
}

static off_t s_lseek(int fd, off_t offset, int whence) {
	fd&=0x0FFF;	Buffer *b=(fd>0)&&(fd<=fdMap.size())?fdMap[fd-1]:NULL;
	if (b==NULL) /* sanity check */
	{
		errno = EBADF;
		return -1;
	}
	if (b->stream) {
		errno = ESPIPE;
		return -1;
	}
	switch (whence) {
		case SEEK_SET: break;
		case SEEK_CUR: offset+=b->pos; break;
		case SEEK_END: offset+=b->size(); break;
		default: errno = EINVAL; return -1;
	}
	if (offset<0) offset=0;
	if (offset>b->size()) offset=b->size();
	b->pos=offset;
	return b->pos;
}

static g_Vfs buffer_vfs={ s_open, s_close, s_read, s_write, s_lseek, };

BufferBinder::BufferBinder(lua_State* L)
{
	Binder binder(L);

	static const luaL_Reg functionList[] = {
        {"append", append},
        {"prepend", prepend},
        {"trim", trim},
        {"set", set},
        {"get", get},
        {"size", size},
        {"seek", seek},
        {"tell", tell},
        {NULL, NULL},
    };

	binder.createClass("Buffer", NULL, create, destruct, functionList);
	bufferMap.clear();

	gpath_addDrivePrefix(10,"|B|");
	gpath_addDrivePrefix(10,"|b|");
	gpath_setDriveFlags(10,GPATH_RW);
	gpath_setDriveVfs(10,&buffer_vfs);
}

int BufferBinder::create(lua_State* L)
{
	StackChecker checker(L, "BufferBinder::create", 1);
	Binder binder(L);
	const char *id=luaL_checkstring(L,1);
	std::string sid(id);
	Buffer *b=bufferMap[sid];
	if (b==NULL) {
		b=new Buffer(sid,lua_toboolean(L,2));
		bufferMap[sid]=b;
	}
	binder.pushInstance("Buffer", b);
	return 1;
}

int BufferBinder::destruct(lua_State* L)
{
	void* ptr = *(void**)lua_touserdata(L, 1);
	Buffer* buffer = static_cast<Buffer*>(ptr);
	if (buffer->refCount()==1)
		bufferMap[buffer->id]=NULL;
	buffer->unref();
	return 0;
}

int BufferBinder::append(lua_State *L)
{
    Binder binder(L);

    Buffer* buffer = static_cast<Buffer*>(binder.getInstance("Buffer", 1));
    lua_pushinteger(L,buffer->append(luaL_checkstring(L,2),lua_objlen(L,2)));

    return 1;
}

int BufferBinder::prepend(lua_State *L)
{
    Binder binder(L);

    Buffer* buffer = static_cast<Buffer*>(binder.getInstance("Buffer", 1));
    lua_pushinteger(L,buffer->prepend(luaL_checkstring(L,2),lua_objlen(L,2)));

    return 1;
}

int BufferBinder::trim(lua_State *L)
{
    Binder binder(L);

    Buffer* buffer = static_cast<Buffer*>(binder.getInstance("Buffer", 1));
    lua_pushinteger(L,buffer->trim(luaL_checkinteger(L,2)));

    return 1;
}

int BufferBinder::get(lua_State *L)
{
    Binder binder(L);

    Buffer* buffer = static_cast<Buffer*>(binder.getInstance("Buffer", 1));
    size_t l=luaL_optinteger(L,3,buffer->size());
    const char *data=buffer->get(luaL_optinteger(L,2,0),l);
    lua_pushlstring(L,data,l);

    return 1;
}

int BufferBinder::set(lua_State *L)
{
    Binder binder(L);

    Buffer* buffer = static_cast<Buffer*>(binder.getInstance("Buffer", 1));
    lua_pushinteger(L,buffer->set(luaL_checkstring(L,2),luaL_optinteger(L,3,0),lua_objlen(L,2)));

    return 1;
}

int BufferBinder::size(lua_State *L)
{
    Binder binder(L);

    Buffer* buffer = static_cast<Buffer*>(binder.getInstance("Buffer", 1));
    lua_pushinteger(L,buffer->size());

    return 1;
}

int BufferBinder::seek(lua_State *L)
{
    Binder binder(L);

    Buffer* buffer = static_cast<Buffer*>(binder.getInstance("Buffer", 1));
    int seek=luaL_optinteger(L,2,0);
    size_t sz=buffer->size();
    if (seek<0) seek=0;
    if (seek>sz) seek=sz;
    buffer->pos=seek;
    lua_pushinteger(L,seek);

    return 1;
}

int BufferBinder::tell(lua_State *L)
{
    Binder binder(L);

    Buffer* buffer = static_cast<Buffer*>(binder.getInstance("Buffer", 1));
    lua_pushinteger(L,buffer->pos);

    return 1;
}
