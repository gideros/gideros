--
--    gmake definintion file, execute with gmake.lua
--

plugin = {}

plugin.name = {
  full = "MegaCool",
  camel = "megaCool"
}

plugin.desc = "MegaCool promotion services plugin."

  -- available: "desktop", "android", "win32", "winrt", "html5", "ios"
plugin.targets = { "android", "ios" }

-- paths are relative to myplugin/source/common
plugin.sources =
{
  -- for vpath in Makefiles
  dirs = {
  },
  -- source files can have '_'s or '-'s but no other non-alphanumeric characters
  cxx =
  {
    "gmegacool_stub",
  },
  c = {}, -- not yet implemented
  java = {} -- not implemented
}

plugin.headers =
{
  "gmegacool.h"
}


plugin.libs = {
  "lua",
  "gideros"
}

plugin.includes =
{
  -- relative to myplugin/source/common
  "../../../../Sdk/include",
  "../../../../Sdk/include/gideros",
}


return plugin
