--[[--------------------------------------------------------------------

  html.lua: Turns Lua 5.1 source code into HTML files
  This file is part of LuaSrcDiet.

  Copyright (c) 2008,2011 Kein-Hong Man <keinhong@gmail.com>
  The COPYRIGHT file describes the conditions
  under which this software may be distributed.

----------------------------------------------------------------------]]

--[[--------------------------------------------------------------------
-- NOTES:
-- WARNING: highly experimental! interface liable to change
-- * This HTML highlighter marks globals brightly so that their usage
--   can be manually optimized.
-- * Either uses a .html extension for output files or it follows the
--   -o <filespec> option.
-- * The HTML style tries to follow that of the Lua wiki.
----------------------------------------------------------------------]]

local base = _G

module "plugin/html"

local string = base.require "string"
local table = base.require "table"
local io = base.require "io"

------------------------------------------------------------------------
-- constants and configuration
------------------------------------------------------------------------

local HTML_EXT = ".html"
local ENTITIES = {
  ["&"] = "&amp;", ["<"] = "&lt;", [">"] = "&gt;",
  ["'"] = "&apos;", ["\""] = "&quot;",
}

-- simple headers and footers
local HEADER = [[
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
<title>%s</title>
<meta name="Generator" content="LuaSrcDiet">
<style type="text/css">
%s</style>
</head>
<body>
<pre class="code">
]]
local FOOTER = [[
</pre>
</body>
</html>
]]
-- for more, please see wikimain.css from the Lua wiki site
local STYLESHEET = [[
BODY {
    background: white;
    color: navy;
}
pre.code { color: black; }
span.comment { color: #00a000; }
span.string  { color: #009090; }
span.keyword { color: black; font-weight: bold; }
span.number { color: #993399; }
span.operator { }
span.name { }
span.global { color: #ff0000; font-weight: bold; }
span.local { color: #0000ff; font-weight: bold; }
]]

------------------------------------------------------------------------
-- option handling, plays nice with --quiet option
------------------------------------------------------------------------

local option                    -- local reference to list of options
local srcfl, destfl             -- filenames
local toklist, seminfolist, toklnlist  -- token data

local function print(...)               -- handle quiet option
  if option.QUIET then return end
  base.print(...)
end

------------------------------------------------------------------------
-- initialization
------------------------------------------------------------------------

function init(_option, _srcfl, _destfl)
  option = _option
  srcfl = _srcfl
  local extb, exte = string.find(srcfl, "%.[^%.%\\%/]*$")
  local basename, extension = srcfl, ""
  if extb and extb > 1 then
    basename = string.sub(srcfl, 1, extb - 1)
    extension = string.sub(srcfl, extb, exte)
  end
  destfl = basename..HTML_EXT
  if option.OUTPUT_FILE then
    destfl = option.OUTPUT_FILE
  end
  if srcfl == destfl then
    base.error("output filename identical to input filename")
  end
end

------------------------------------------------------------------------
-- message display, post-load processing
------------------------------------------------------------------------

function post_load(z)
  print([[
HTML plugin module for LuaSrcDiet
]])
  print("Exporting: "..srcfl.." -> "..destfl.."\n")
end

------------------------------------------------------------------------
-- post-lexing processing, can work on lexer table output
------------------------------------------------------------------------

function post_lex(_toklist, _seminfolist, _toklnlist)
  toklist, seminfolist, toklnlist
    = _toklist, _seminfolist, _toklnlist
end

------------------------------------------------------------------------
-- escape the usual suspects for HTML/XML
------------------------------------------------------------------------

local function do_entities(z)
  local i = 1
  while i <= #z do
    local c = string.sub(z, i, i)
    local d = ENTITIES[c]
    if d then
      c = d
      z = string.sub(z, 1, i - 1)..c..string.sub(z, i + 1)
    end
    i = i + #c
  end--while
  return z
end

------------------------------------------------------------------------
-- save source code to file
------------------------------------------------------------------------

local function save_file(fname, dat)
  local OUTF = io.open(fname, "wb")
  if not OUTF then base.error("cannot open \""..fname.."\" for writing") end
  local status = OUTF:write(dat)
  if not status then base.error("cannot write to \""..fname.."\"") end
  OUTF:close()
end

------------------------------------------------------------------------
-- post-parsing processing, gives globalinfo, localinfo
------------------------------------------------------------------------

function post_parse(globalinfo, localinfo)
  local html = {}
  local function add(s)         -- html helpers
    html[#html + 1] = s
  end
  local function span(class, s)
    add('<span class="'..class..'">'..s..'</span>')
  end
  ----------------------------------------------------------------------
  for i = 1, #globalinfo do     -- mark global identifiers as TK_GLOBAL
    local obj = globalinfo[i]
    local xref = obj.xref
    for j = 1, #xref do
      local p = xref[j]
      toklist[p] = "TK_GLOBAL"
    end
  end--for
  ----------------------------------------------------------------------
  for i = 1, #localinfo do      -- mark local identifiers as TK_LOCAL
    local obj = localinfo[i]
    local xref = obj.xref
    for j = 1, #xref do
      local p = xref[j]
      toklist[p] = "TK_LOCAL"
    end
  end--for
  ----------------------------------------------------------------------
  add(string.format(HEADER,     -- header and leading stuff
    do_entities(srcfl),
    STYLESHEET))
  for i = 1, #toklist do        -- enumerate token list
    local tok, info = toklist[i], seminfolist[i]
    if tok == "TK_KEYWORD" then
      span("keyword", info)
    elseif tok == "TK_STRING" or tok == "TK_LSTRING" then
      span("string", do_entities(info))
    elseif tok == "TK_COMMENT" or tok == "TK_LCOMMENT" then
      span("comment", do_entities(info))
    elseif tok == "TK_GLOBAL" then
      span("global", info)
    elseif tok == "TK_LOCAL" then
      span("local", info)
    elseif tok == "TK_NAME" then
      span("name", info)
    elseif tok == "TK_NUMBER" then
      span("number", info)
    elseif tok == "TK_OP" then
      span("operator", do_entities(info))
    elseif tok ~= "TK_EOS" then  -- TK_EOL, TK_SPACE
      add(info)
    end
  end--for
  add(FOOTER)
  save_file(destfl, table.concat(html))
  option.EXIT = true
end
