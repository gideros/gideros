--[[--------------------------------------------------------------------

  example.lua: Example of a plugin for LuaSrcDiet
  This file is part of LuaSrcDiet.

  Kein-Hong Man <keinhong@gmail.com> 2008,2011 PUBLIC DOMAIN

----------------------------------------------------------------------]]

--[[--------------------------------------------------------------------
-- NOTES:
-- WARNING: highly experimental! interface liable to change
-- * Any function can be omitted and LuaSrcDiet won't call it.
-- * The functions are:
--   (1) init(_option, _srcfl, _destfl)
--   (2) post_load(z) can return z
--   (3) post_lex(toklist, seminfolist, toklnlist)
--   (4) post_parse(globalinfo, localinfo)
--   (5) post_optparse()
--   (6) post_optlex(toklist, seminfolist, toklnlist)
-- * Older tables can be copied and kept in the plugin and used later.
-- * If you modify 'option', remember that LuaSrcDiet might be
--   processing more than one file.
-- * Arrangement of the functions is not final!
-- * TODO: not sure about return base.getfenv() if module name different
-- * TODO: can't process additional options from command line yet
----------------------------------------------------------------------]]

-- if anything else is added before 'module', then onefile.lua must be
-- changed in order to correctly embed the plugin
local base = _G

module "plugin/example"

local string = base.require "string"
local table = base.require "table"

------------------------------------------------------------------------
-- option handling, plays nice with --quiet option
------------------------------------------------------------------------

local option                    -- local reference to list of options
local srcfl, destfl             -- filenames
local old_quiet

local function print(...)               -- handle quiet option
  if option.QUIET then return end
  base.print(...)
end

------------------------------------------------------------------------
-- initialization
------------------------------------------------------------------------

function init(_option, _srcfl, _destfl)
  option = _option
  srcfl, destfl = _srcfl, _destfl
  -- plugin can impose its own option starting from here
end

------------------------------------------------------------------------
-- message display, post-load processing, can return z
------------------------------------------------------------------------

function post_load(z)
  -- this message will print after the LuaSrcDiet title message
  print([[
Example plugin module for LuaSrcDiet
]])
  print("Example: source file name is '"..srcfl.."'")
  print("Example: destination file name is '"..destfl.."'")
  print("Example: the size of the source file is "..#z.." bytes")
  -- returning z is optional; this allows optional replacement of
  -- the source data prior to lexing
  return z
end

------------------------------------------------------------------------
-- post-lexing processing, can work on lexer table output
------------------------------------------------------------------------

function post_lex(toklist, seminfolist, toklnlist)
  print("Example: the number of lexed elements is "..#toklist)
end

------------------------------------------------------------------------
-- post-parsing processing, gives globalinfo, localinfo
------------------------------------------------------------------------

function post_parse(globalinfo, localinfo)
  print("Example: size of globalinfo is "..#globalinfo)
  print("Example: size of localinfo is "..#localinfo)
  old_quiet = option.QUIET
  option.QUIET = true
end

------------------------------------------------------------------------
-- post-parser optimization processing, can get tables from elsewhere
------------------------------------------------------------------------

function post_optparse()
  option.QUIET = old_quiet
  print("Example: pretend to do post-optparse")
end

------------------------------------------------------------------------
-- post-lexer optimization processing, can get tables from elsewhere
------------------------------------------------------------------------

function post_optlex(toklist, seminfolist, toklnlist)
  print("Example: pretend to do post-optlex")
  -- restore old settings, other file might need original settings
  option.QUIET = old_quiet
  -- option.EXIT can be set at the end of any post_* function to stop
  -- further processing and exit for the current file being worked on
  -- in this case, final stats printout is disabled and the output will
  -- not be written to the destination file
  option.EXIT = true
end
