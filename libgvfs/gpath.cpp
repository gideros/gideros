#include <gpath.h>
#include <gpath_p.h>
#include <string.h>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <stdio.h>
#include <assert.h>

struct Drive
{
    Drive() : flags(0) {}

    std::set<std::string> prefixes;
    int flags;
    std::string path;
};

static std::map<int, Drive> s_drives;
static int s_defaultDrive = 0;
static int s_absolutePathFlags = 0;
static char s_path[1024];

static bool isAbsolute(const char *pathName)
{
    size_t len = strlen(pathName);

    // check if it is absolute path
    bool b;

    bool b1 = (len >= 1 && (pathName[0] == '/'));		// "/linux" or "/windows"
#ifdef _WIN32
    bool b2 = (len >= 1 && (pathName[0] == '\\'));		// "\windows"
    bool b3 = (len >= 2 && (pathName[1] == ':'));		// "c:\windows"
    b = b1 || b2 || b3;
#else
    b = b1;
#endif

    return b;
}

extern "C" {

void gpath_init()
{
    s_drives.clear();
    s_defaultDrive = 0;
    s_absolutePathFlags = 0;
    s_path[0] = 0;
}

void gpath_cleanup()
{
    s_drives.clear();
}

void gpath_setDrivePath(int id, const char *path)
{
    s_drives[id].path = path;
}

void gpath_setDriveFlags(int id, int flags)
{
    s_drives[id].flags = flags;
}

void gpath_addDrivePrefix(int id, const char *prefix)
{
    s_drives[id].prefixes.insert(prefix);
}

void gpath_setDefaultDrive(int id)
{
    s_defaultDrive = id;
}

int gpath_getDefaultDrive()
{
    return s_defaultDrive;
}

const char *gpath_getDrivePath(int id)
{
    std::map<int, Drive>::iterator iter = s_drives.find(id);

    if (iter == s_drives.end())
        return NULL;

    return iter->second.path.c_str();
}

int gpath_getDriveFlags(int id)
{
    if (id == GPATH_ABSOLUTE)
        return s_absolutePathFlags;

    std::map<int, Drive>::iterator iter = s_drives.find(id);

    if (iter == s_drives.end())
        return 0;

    return iter->second.flags;
}

void gpath_setAbsolutePathFlags(int flags)
{
    s_absolutePathFlags = flags;
}

int gpath_getPathDrive(const char *pathName)
{
    if (isAbsolute(pathName))
        return GPATH_ABSOLUTE;

    std::map<int, Drive>::iterator iter, e = s_drives.end();
    for (iter = s_drives.begin(); iter != e; ++iter)
    {
        std::set<std::string>& prefixes = iter->second.prefixes;
        std::set<std::string>::iterator iter2, e2 = prefixes.end();
        for (iter2 = prefixes.begin(); iter2 != e2; ++iter2)
            if (strncmp(pathName, iter2->c_str(), iter2->size()) == 0)
                return iter->first;
    }

    return s_defaultDrive;
}

const char* gpath_join(const char *path1, const char *path2)
{
    if (path1[0] == 0)
        return strcpy(s_path, path2);

    char last = path1[strlen(path1) - 1];

    if (last == '/' || last == '\\')
        sprintf(s_path, "%s%s", path1, path2);
    else
        sprintf(s_path, "%s/%s", path1, path2);

    return s_path;
}

const char *gpath_transform(const char *pathName)
{
    if (isAbsolute(pathName))
        return strcpy(s_path, pathName);

    std::map<int, Drive>::iterator iter, e = s_drives.end();
    for (iter = s_drives.begin(); iter != e; ++iter)
    {
        std::set<std::string>& prefixes = iter->second.prefixes;
        std::set<std::string>::iterator iter2, e2 = prefixes.end();
        for (iter2 = prefixes.begin(); iter2 != e2; ++iter2)
            if (strncmp(pathName, iter2->c_str(), iter2->size()) == 0)
                return gpath_join(iter->second.path.c_str(), pathName + iter2->size());
    }

    iter = s_drives.find(s_defaultDrive);
    assert(iter != s_drives.end());
    return gpath_join(iter->second.path.c_str(), pathName);
}

// Transform a path to a platform independant form suitable for archive lookup
const char *gpath_normalizeArchivePath(const char *pathname)
{
	pathname=gpath_transform(pathname);
	//We reuse s_path as our dest buffer, but in most case it will be the same as input buffer, so take care
    char *xform=s_path;
    bool skipSep=true;
    while (*pathname)
    {
        if (!strncmp("./",pathname,2))
        {
         pathname+=2;
         continue;
        }
    	switch (*pathname)
    	{
    	case '/':
    	case '\\':
    	case ':': //For WIN32, but will break names containing a colon, which are never almost never seen in practice
    		if (!skipSep)
    		{
    			*(xform++)='/'; //output a separator
    			skipSep=true;
    		}
    		break;
    	default:
    		*(xform++)=*pathname;
    		skipSep=false;
    	}
    	pathname++;
    }
    *xform=0;

	return s_path;
}


}
