--[[--------------------------------------------------------------------

  equiv.lua: Source and binary equivalency comparisons
  This file is part of LuaSrcDiet.

  Copyright (c) 2011 Kein-Hong Man <keinhong@gmail.com>
  The COPYRIGHT file describes the conditions
  under which this software may be distributed.

----------------------------------------------------------------------]]

--[[--------------------------------------------------------------------
-- NOTES:
-- * intended as an extra safety check for mission-critical code,
--   should give affirmative results if everything works
-- * heavy on loadstring() and string.dump(), which may be slowish,
--   and may cause problems for cross-compiled applications
-- * optional detailed information dump is mainly for debugging,
--   reason being, if the two are not equivalent when they should be,
--   then some form of optimization has failed
-- * source: IMPORTANT: TK_NAME not compared if opt-locals enabled
-- * binary: IMPORTANT: some shortcuts are taken with int and size_t
--   value reading -- if the functions break, then the binary chunk
--   is very large indeed
-- * binary: there is a lack of diagnostic information when a compare
--   fails; you can use ChunkSpy and compare using visual diff
----------------------------------------------------------------------]]

local base = _G

module "equiv"

local string = base.require "string"
local loadstring = base.loadstring
local sub = string.sub
local match = string.match
local dump = string.dump
local byte = string.byte

--[[--------------------------------------------------------------------
-- variable and data initialization
----------------------------------------------------------------------]]

local is_realtoken = {          -- significant (grammar) tokens
  TK_KEYWORD = true,
  TK_NAME = true,
  TK_NUMBER = true,
  TK_STRING = true,
  TK_LSTRING = true,
  TK_OP = true,
  TK_EOS = true,
}

local option, llex, warn

--[[--------------------------------------------------------------------
-- functions
----------------------------------------------------------------------]]

------------------------------------------------------------------------
-- initialization function
------------------------------------------------------------------------

function init(_option, _llex, _warn)
  option = _option
  llex = _llex
  warn = _warn
end

------------------------------------------------------------------------
-- function to build lists containing a 'normal' lexer stream
------------------------------------------------------------------------

local function build_stream(s)
  llex.init(s)
  llex.llex()
  local stok, sseminfo -- source list (with whitespace elements)
    = llex.tok, llex.seminfo
  local tok, seminfo   -- processed list (real elements only)
    = {}, {}
  for i = 1, #stok do
    local t = stok[i]
    if is_realtoken[t] then
      tok[#tok + 1] = t
      seminfo[#seminfo + 1] = sseminfo[i]
    end
  end--for
  return tok, seminfo
end

------------------------------------------------------------------------
-- test source (lexer stream) equivalence
------------------------------------------------------------------------

function source(z, dat)
  --------------------------------------------------------------------
  -- function to return a dumped string for seminfo compares
  --------------------------------------------------------------------
  local function dumpsem(s)
    local sf = loadstring("return "..s, "z")
    if sf then
      return dump(sf)
    end
  end
  --------------------------------------------------------------------
  -- mark and optionally report non-equivalence
  --------------------------------------------------------------------
  local function bork(msg)
    if option.DETAILS then base.print("SRCEQUIV: "..msg) end
    warn.SRC_EQUIV = true
  end
  --------------------------------------------------------------------
  -- get lexer streams for both source strings, compare
  --------------------------------------------------------------------
  local tok1, seminfo1 = build_stream(z)        -- original
  local tok2, seminfo2 = build_stream(dat)      -- compressed
  --------------------------------------------------------------------
  -- compare shbang lines ignoring EOL
  --------------------------------------------------------------------
  local sh1 = match(z, "^(#[^\r\n]*)")
  local sh2 = match(dat, "^(#[^\r\n]*)")
  if sh1 or sh2 then
    if not sh1 or not sh2 or sh1 ~= sh2 then
      bork("shbang lines different")
    end
  end
  --------------------------------------------------------------------
  -- compare by simple count
  --------------------------------------------------------------------
  if #tok1 ~= #tok2 then
    bork("count "..#tok1.." "..#tok2)
    return
  end
  --------------------------------------------------------------------
  -- compare each element the best we can
  --------------------------------------------------------------------
  for i = 1, #tok1 do
    local t1, t2 = tok1[i], tok2[i]
    local s1, s2 = seminfo1[i], seminfo2[i]
    if t1 ~= t2 then  -- by type
      bork("type ["..i.."] "..t1.." "..t2)
      break
    end
    if t1 == "TK_KEYWORD" or t1 == "TK_NAME" or t1 == "TK_OP" then
      if t1 == "TK_NAME" and option["opt-locals"] then
        -- can't compare identifiers of locals that are optimized
      elseif s1 ~= s2 then  -- by semantic info (simple)
        bork("seminfo ["..i.."] "..t1.." "..s1.." "..s2)
        break
      end
    elseif t1 == "TK_EOS" then
      -- no seminfo to compare
    else-- "TK_NUMBER" or "TK_STRING" or "TK_LSTRING"
      -- compare 'binary' form, so dump a function
      local s1b,s2b = dumpsem(s1), dumpsem(s2)
      if not s1b or not s2b or s1b ~= s2b then
        bork("seminfo ["..i.."] "..t1.." "..s1.." "..s2)
        break
      end
    end
  end--for
  --------------------------------------------------------------------
  -- successful comparison if end is reached with no borks
  --------------------------------------------------------------------
end

------------------------------------------------------------------------
-- test binary chunk equivalence
------------------------------------------------------------------------

function binary(z, dat)
  local TNIL     = 0
  local TBOOLEAN = 1
  local TNUMBER  = 3
  local TSTRING  = 4
  --------------------------------------------------------------------
  -- mark and optionally report non-equivalence
  --------------------------------------------------------------------
  local function bork(msg)
    if option.DETAILS then base.print("BINEQUIV: "..msg) end
    warn.BIN_EQUIV = true
  end
  --------------------------------------------------------------------
  -- function to remove shbang line so that loadstring runs
  --------------------------------------------------------------------
  local function zap_shbang(s)
    local shbang = match(s, "^(#[^\r\n]*\r?\n?)")
    if shbang then                      -- cut out shbang
      s = sub(s, #shbang + 1)
    end
    return s
  end
  --------------------------------------------------------------------
  -- attempt to compile, then dump to get binary chunk string
  --------------------------------------------------------------------
  local cz = loadstring(zap_shbang(z), "z")
  if not cz then
    bork("failed to compile original sources for binary chunk comparison")
    return
  end
  local cdat = loadstring(zap_shbang(dat), "z")
  if not cdat then
    bork("failed to compile compressed result for binary chunk comparison")
  end
  -- if loadstring() works, dump assuming string.dump() is error-free
  local c1 = { i = 1, dat = dump(cz) }
  c1.len = #c1.dat
  local c2 = { i = 1, dat = dump(cdat) }
  c2.len = #c2.dat
  --------------------------------------------------------------------
  -- support functions to handle binary chunk reading
  --------------------------------------------------------------------
  local endian,
        sz_int, sz_sizet,               -- sizes of data types
        sz_inst, sz_number,
        getint, getsizet
  --------------------------------------------------------------------
  local function ensure(c, sz)          -- check if bytes exist
    if c.i + sz - 1 > c.len then return end
    return true
  end
  --------------------------------------------------------------------
  local function skip(c, sz)            -- skip some bytes
    if not sz then sz = 1 end
    c.i = c.i + sz
  end
  --------------------------------------------------------------------
  local function getbyte(c)             -- return a byte value
    local i = c.i
    if i > c.len then return end
    local d = sub(c.dat, i, i)
    c.i = i + 1
    return byte(d)
  end
  --------------------------------------------------------------------
  local function getint_l(c)            -- return an int value (little-endian)
    local n, scale = 0, 1
    if not ensure(c, sz_int) then return end
    for j = 1, sz_int do
      n = n + scale * getbyte(c)
      scale = scale * 256
    end
    return n
  end
  --------------------------------------------------------------------
  local function getint_b(c)            -- return an int value (big-endian)
    local n = 0
    if not ensure(c, sz_int) then return end
    for j = 1, sz_int do
      n = n * 256 + getbyte(c)
    end
    return n
  end
  --------------------------------------------------------------------
  local function getsizet_l(c)          -- return a size_t value (little-endian)
    local n, scale = 0, 1
    if not ensure(c, sz_sizet) then return end
    for j = 1, sz_sizet do
      n = n + scale * getbyte(c)
      scale = scale * 256
    end
    return n
  end
  --------------------------------------------------------------------
  local function getsizet_b(c)          -- return a size_t value (big-endian)
    local n = 0
    if not ensure(c, sz_sizet) then return end
    for j = 1, sz_sizet do
      n = n * 256 + getbyte(c)
    end
    return n
  end
  --------------------------------------------------------------------
  local function getblock(c, sz)        -- return a block (as a string)
    local i = c.i
    local j = i + sz - 1
    if j > c.len then return end
    local d = sub(c.dat, i, j)
    c.i = i + sz
    return d
  end
  --------------------------------------------------------------------
  local function getstring(c)           -- return a string
    local n = getsizet(c)
    if not n then return end
    if n == 0 then return "" end
    return getblock(c, n)
  end
  --------------------------------------------------------------------
  local function goodbyte(c1, c2)       -- compare byte value
    local b1, b2 = getbyte(c1), getbyte(c2)
    if not b1 or not b2 or b1 ~= b2 then
      return
    end
    return b1
  end
  --------------------------------------------------------------------
  local function badbyte(c1, c2)        -- compare byte value
    local b = goodbyte(c1, c2)
    if not b then return true end
  end
  --------------------------------------------------------------------
  local function goodint(c1, c2)        -- compare int value
    local i1, i2 = getint(c1), getint(c2)
    if not i1 or not i2 or i1 ~= i2 then
      return
    end
    return i1
  end
  --------------------------------------------------------------------
  -- recursively-called function to compare function prototypes
  --------------------------------------------------------------------
  local function getfunc(c1, c2)
    -- source name (ignored)
    if not getstring(c1) or not getstring(c2) then
      bork("bad source name"); return
    end
    -- linedefined (ignored)
    if not getint(c1) or not getint(c2) then
      bork("bad linedefined"); return
    end
    -- lastlinedefined (ignored)
    if not getint(c1) or not getint(c2) then
      bork("bad lastlinedefined"); return
    end
    if not (ensure(c1, 4) and ensure(c2, 4)) then
      bork("prototype header broken")
    end
    -- nups (compared)
    if badbyte(c1, c2) then
      bork("bad nups"); return
    end
    -- numparams (compared)
    if badbyte(c1, c2) then
      bork("bad numparams"); return
    end
    -- is_vararg (compared)
    if badbyte(c1, c2) then
      bork("bad is_vararg"); return
    end
    -- maxstacksize (compared)
    if badbyte(c1, c2) then
      bork("bad maxstacksize"); return
    end
    -- code (compared)
    local ncode = goodint(c1, c2)
    if not ncode then
      bork("bad ncode"); return
    end
    local code1 = getblock(c1, ncode * sz_inst)
    local code2 = getblock(c2, ncode * sz_inst)
    if not code1 or not code2 or code1 ~= code2 then
      bork("bad code block"); return
    end
    -- constants (compared)
    local nconst = goodint(c1, c2)
    if not nconst then
      bork("bad nconst"); return
    end
    for i = 1, nconst do
      local ctype = goodbyte(c1, c2)
      if not ctype then
        bork("bad const type"); return
      end
      if ctype == TBOOLEAN then
        if badbyte(c1, c2) then
          bork("bad boolean value"); return
        end
      elseif ctype == TNUMBER then
        local num1 = getblock(c1, sz_number)
        local num2 = getblock(c2, sz_number)
        if not num1 or not num2 or num1 ~= num2 then
          bork("bad number value"); return
        end
      elseif ctype == TSTRING then
        local str1 = getstring(c1)
        local str2 = getstring(c2)
        if not str1 or not str2 or str1 ~= str2 then
          bork("bad string value"); return
        end
      end
    end
    -- prototypes (compared recursively)
    local nproto = goodint(c1, c2)
    if not nproto then
      bork("bad nproto"); return
    end
    for i = 1, nproto do
      if not getfunc(c1, c2) then
        bork("bad function prototype"); return
      end
    end
    -- debug information (ignored)
    -- lineinfo (ignored)
    local sizelineinfo1 = getint(c1)
    if not sizelineinfo1 then
      bork("bad sizelineinfo1"); return
    end
    local sizelineinfo2 = getint(c2)
    if not sizelineinfo2 then
      bork("bad sizelineinfo2"); return
    end
    if not getblock(c1, sizelineinfo1 * sz_int) then
      bork("bad lineinfo1"); return
    end
    if not getblock(c2, sizelineinfo2 * sz_int) then
      bork("bad lineinfo2"); return
    end
    -- locvars (ignored)
    local sizelocvars1 = getint(c1)
    if not sizelocvars1 then
      bork("bad sizelocvars1"); return
    end
    local sizelocvars2 = getint(c2)
    if not sizelocvars2 then
      bork("bad sizelocvars2"); return
    end
    for i = 1, sizelocvars1 do
      if not getstring(c1) or not getint(c1) or not getint(c1) then
        bork("bad locvars1"); return
      end
    end
    for i = 1, sizelocvars2 do
      if not getstring(c2) or not getint(c2) or not getint(c2) then
        bork("bad locvars2"); return
      end
    end
    -- upvalues (ignored)
    local sizeupvalues1 = getint(c1)
    if not sizeupvalues1 then
      bork("bad sizeupvalues1"); return
    end
    local sizeupvalues2 = getint(c2)
    if not sizeupvalues2 then
      bork("bad sizeupvalues2"); return
    end
    for i = 1, sizeupvalues1 do
      if not getstring(c1) then bork("bad upvalues1"); return end
    end
    for i = 1, sizeupvalues2 do
      if not getstring(c2) then bork("bad upvalues2"); return end
    end
    return true
  end
  --------------------------------------------------------------------
  -- parse binary chunks to verify equivalence
  -- * for headers, handle sizes to allow a degree of flexibility
  -- * assume a valid binary chunk is generated, since it was not
  --   generated via external means
  --------------------------------------------------------------------
  if not (ensure(c1, 12) and ensure(c2, 12)) then
    bork("header broken")
  end
  skip(c1, 6)                   -- skip signature(4), version, format
  endian    = getbyte(c1)       -- 1 = little endian
  sz_int    = getbyte(c1)       -- get data type sizes
  sz_sizet  = getbyte(c1)
  sz_inst   = getbyte(c1)
  sz_number = getbyte(c1)
  skip(c1)                      -- skip integral flag
  skip(c2, 12)                  -- skip other header (assume similar)
  if endian == 1 then           -- set for endian sensitive data we need
    getint   = getint_l
    getsizet = getsizet_l
  else
    getint   = getint_b
    getsizet = getsizet_b
  end
  getfunc(c1, c2)               -- get prototype at root
  if c1.i ~= c1.len + 1 then
    bork("inconsistent binary chunk1"); return
  elseif c2.i ~= c2.len + 1 then
    bork("inconsistent binary chunk2"); return
  end
  --------------------------------------------------------------------
  -- successful comparison if end is reached with no borks
  --------------------------------------------------------------------
end
