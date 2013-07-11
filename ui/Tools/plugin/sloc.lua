--[[--------------------------------------------------------------------

  sloc.lua: Calculates SLOC for Lua 5.1 scripts
  This file is part of LuaSrcDiet.

  Copyright (c) 2008,2011 Kein-Hong Man <keinhong@gmail.com>
  The COPYRIGHT file describes the conditions
  under which this software may be distributed.

----------------------------------------------------------------------]]

--[[--------------------------------------------------------------------
-- NOTES:
-- WARNING: highly experimental! interface liable to change
-- * SLOC's behaviour is based on David Wheeler's SLOCCount.
-- * Empty lines and comment don't count as significant.
-- * Empty lines in long strings are also insignificant. This is
--   debatable. In SLOCCount, this allows counting of invalid multi-
--   line strings for C. But an empty line is still an empty line.
-- * Ignores the --quiet option, print own result line.
----------------------------------------------------------------------]]

local base = _G

module "plugin/sloc"

local string = base.require "string"
local table = base.require "table"

------------------------------------------------------------------------
-- initialization
------------------------------------------------------------------------

local option                    -- local reference to list of options
local srcfl                     -- source file name

function init(_option, _srcfl, _destfl)
  option = _option
  option.QUIET = true
  srcfl = _srcfl
end

------------------------------------------------------------------------
-- splits a block into a table of lines (minus EOLs)
------------------------------------------------------------------------

local function split(blk)
  local lines = {}
  local i, nblk = 1, #blk
  while i <= nblk do
    local p, q, r, s = string.find(blk, "([\r\n])([\r\n]?)", i)
    if not p then
      p = nblk + 1
    end
    lines[#lines + 1] = string.sub(blk, i, p - 1)
    i = p + 1
    if p < nblk and q > p and r ~= s then  -- handle Lua-style CRLF, LFCR
      i = i + 1
    end
  end
  return lines
end

------------------------------------------------------------------------
-- post-lexing processing, can work on lexer table output
------------------------------------------------------------------------

function post_lex(toklist, seminfolist, toklnlist)
  local lnow, sloc = 0, 0
  local function chk(ln)        -- if a new line, count it as an SLOC
    if ln > lnow then           -- new line # must be > old line #
      sloc = sloc + 1; lnow = ln
    end
  end
  for i = 1, #toklist do        -- enumerate over all tokens
    local tok, info, ln
      = toklist[i], seminfolist[i], toklnlist[i]
    --------------------------------------------------------------------
    if tok == "TK_KEYWORD" or tok == "TK_NAME" or       -- significant
       tok == "TK_NUMBER" or tok == "TK_OP" then
      chk(ln)
    --------------------------------------------------------------------
    -- Both TK_STRING and TK_LSTRING may be multi-line, hence, a loop
    -- is needed in order to mark off lines one-by-one. Since llex.lua
    -- currently returns the line number of the last part of the string,
    -- we must subtract in order to get the starting line number.
    --------------------------------------------------------------------
    elseif tok == "TK_STRING" then      -- possible multi-line
      local t = split(info)
      ln = ln - #t + 1
      for j = 1, #t do
        chk(ln); ln = ln + 1
      end
    --------------------------------------------------------------------
    elseif tok == "TK_LSTRING" then     -- possible multi-line
      local t = split(info)
      ln = ln - #t + 1
      for j = 1, #t do
        if t[j] ~= "" then chk(ln) end
        ln = ln + 1
      end
    --------------------------------------------------------------------
    -- other tokens are comments or whitespace and are ignored
    --------------------------------------------------------------------
    end
  end--for
  base.print(srcfl..": "..sloc) -- display result
  option.EXIT = true
end
