local require = require

local grammar = require "cosmo.grammar"
local interpreter = require "cosmo.fill"
local loadstring = loadstring

module(..., package.seeall)

yield = coroutine.yield

local compiled_template = [[
      local concat = table.concat
      local insert = table.insert
      local compile = compile
      local getmetatable = getmetatable
      local setmetatable = setmetatable
      local function prepare_env(env, parent)
        local meta = getmetatable(env)
        if meta and meta.__index then
          local index = meta.__index
          meta.__index = function (t, k)
			   local v
			   if type(index) == "table" then 
			     v = index[k] 
			   else
			     v = index(t, k)
			   end
                           if not v then v = parent[k] end
                           return v
                         end
        else
          setmetatable(env, { __index = parent })
        end
      end
      local function id() return "" end
      return function (env)
		local out = {}
		if type(env) == "string" then env = { it = env } end
		$parts[=[
		      insert(out, $quoted_text)
		]=],
		[=[
		      local selector = $parsed_selector
		      local selector_name = '$selector'
		      if not selector then selector = '$selector' end
		      $if_subtemplate[==[
			    local subtemplates = {}
			    $subtemplates[===[
				  subtemplates[$i] = compile($subtemplate)
			    ]===]
			    $if_args[===[
				  for e in coroutine.wrap(selector), $args, true do
				     if type(e) ~= "table" then
					e = { it = tostring(e) }
				     end
				     prepare_env(e, env) 
				     insert(out, (subtemplates[rawget(e, '_template') or 1] or id)(e))
				  end
			    ]===],
			    [===[
				  if type(selector) == 'table' then
				     for _, e in ipairs(selector) do
					if type(e) ~= "table" then
					   e = { it = tostring(e) }
					end
					prepare_env(e, env) 
					insert(out, (subtemplates[rawget(e, '_template') or 1] or id)(e))
				     end
				  else
				     for e in coroutine.wrap(selector), nil, true do
					if type(e) ~= "table" then
					   e = { it = tostring(e) }
					end
					prepare_env(e, env) 
					insert(out, (subtemplates[rawget(e, '_template') or 1] or id)(e))
				     end
				  end
			    ]===]
		      ]==],
		      [==[
			    $if_args[===[
				  if type(selector) == 'function' then
				     selector = selector($args, false)
				  end
				  insert(out, tostring(selector))
			    ]===],
			    [===[
				  if type(selector) == 'function' then
				     insert(out, tostring(selector()))
				  else
				     insert(out, tostring(selector))
				  end
			    ]===]
		      ]==]
		]=]
	        return concat(out)
             end
]]

local function compile_text(chunkname, text)
   return { _template = 1, quoted_text = string.format("%q", text) }
end

local function compile_template_application(chunkname, selector, args, first_subtemplate, 
					    subtemplates)
   subtemplates = subtemplates or {}
   if first_subtemplate ~= "" then table.insert(subtemplates, 1, first_subtemplate) end
   local ta = { _template = 2, selector = selector, 
      parsed_selector = grammar.parse_selector(selector) }
   local do_subtemplates = function ()
			      for i, subtemplate in ipairs(subtemplates) do
				 yield{ i = i, subtemplate = subtemplate }
			      end
			   end
   if #subtemplates == 0 then
      if args and args ~= "" then
	 ta.if_subtemplate = { { _template = 2, if_args = { { _template = 1, args = args } } } }
      else
	 ta.if_subtemplate = { { _template = 2, if_args = { { _template = 2 } } } }
      end
   else
      if args and args ~= "" then
	 ta.if_subtemplate = { { _template = 1, subtemplates = do_subtemplates,
	       if_args = { { _template = 1, args = args } } } }
      else
	 ta.if_subtemplate = { { _template = 1, subtemplates = do_subtemplates,
	       if_args = { { _template = 2 } } } }
      end
   end
   return ta
end

local function compile_template(chunkname, compiled_parts)
   local template_code = interpreter.fill(compiled_template, { parts = compiled_parts })
   local template_func, err = loadstring(template_code, chunkname)
   if not template_func then
      error("syntax error when compiling template: " .. err)
   else
      setfenv(template_func, _M)
      return template_func()
   end
end

local compiler = grammar.cosmo_compiler{ text = compile_text,
   template_application = compile_template_application, 
   template = compile_template }

local cache = {}
setmetatable(cache, { __index = function (tab, key)
				   local new = {}
				   tab[key] = new
				   return new
				end,
		      __mode = "v" })

function compile(template, chunkname)
  template = template or ""
  chunkname = chunkname or template
  local compiled_template = cache[template][chunkname]
  if not compiled_template then
    compiled_template = compiler:match(template, 1, chunkname)
    cache[template][chunkname] = compiled_template
  end
  return compiled_template
end

local filled_templates = {}

function fill(template, env)
   template = template or ""
   local start = template:match("^(%[=*%[)")
   if start then template = template:sub(#start + 1, #template - #start) end
   if filled_templates[template] then 
      return compile(template)(env)
   else
      filled_templates[template] = true
      return interpreter.fill(template, env, fill)
   end
end

local nop = function () end

function cond(bool, table)
   if bool then
      return function () yield(table) end
   else
      return nop
   end
end

f = compile

function c(bool)
   if bool then 
      return function (table)
		return function () yield(table) end
	     end
   else
      return function (table) return nop end
   end
end

function map(arg, has_block)
   if has_block then
      for _, item in ipairs(arg) do
	 cosmo.yield(item)
      end
   else
      return table.concat(arg)
   end
end

function inject(arg)
   cosmo.yield(arg)
end

function cif(arg, has_block)
  if not has_block then error("this selector needs a block") end
  if arg[1] then
    arg._template = 1
  else
    arg._template = 2
  end
  cosmo.yield(arg)
end
