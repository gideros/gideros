
local lpeg = require "lpeg"
local re = require "re"

module(..., package.seeall)

function parse_selector(selector, env)
  env = env or "env"
  selector = string.sub(selector, 2, #selector)
  local parts = {}
  for w in string.gmatch(selector, "[^|]+") do
    local n = tonumber(w)
    if n then
      table.insert(parts, "[" .. n .. "]")
    else
      table.insert(parts, "['" .. w .. "']")
    end
  end
  return env .. table.concat(parts)
end

local start = "[" * lpeg.P"="^0 * "["

local longstring = lpeg.P(function (s, i)
  local l = lpeg.match(start, s, i)
  if not l then return nil end
  local p = lpeg.P("]" .. string.rep("=", l - i - 2) .. "]")
  p = (1 - p)^0 * p
  return lpeg.match(p, s, l)
end)

longstring = #("[" * lpeg.S"[=") * longstring

local alpha =  lpeg.R('__','az','AZ','\127\255') 

local n = lpeg.R'09'

local alphanum = alpha + n

local name = alpha * (alphanum)^0

local number = (lpeg.P'.' + n)^1 * (lpeg.S'eE' * lpeg.S'+-'^-1)^-1 * (alphanum)^0
number = #(n + (lpeg.P'.' * n)) * number

local shortstring = (lpeg.P'"' * ( (lpeg.P'\\' * 1) + (1 - (lpeg.S'"\n\r\f')) )^0 * lpeg.P'"') +
  (lpeg.P"'" * ( (lpeg.P'\\' * 1) + (1 - (lpeg.S"'\n\r\f")) )^0 * lpeg.P"'")

local space = (lpeg.S'\n \t\r\f')^0
 
local syntax = [[
  template <- (%state <item>* -> {} !.) -> compiletemplate
  item <- <text> / <templateappl> / (. => error)
  text <- (%state {~ (!<selector> ('$$' -> '$' / .))+ ~}) -> compiletext
  selector <- '$' %alphanum+ ('|' %alphanum+)*
  templateappl <- (%state {<selector>} {~ <args>? ~} !'{' 
		   {%longstring?} !%start (%s ',' %s {%longstring})* -> {} !(',' %s %start)) 
		     -> compileapplication
  args <- '{' %s '}' / '{' %s <arg> %s (',' %s <arg> %s)* ','? %s '}'
  arg <- <attr> / <exp>
  attr <- <symbol> %s '=' !'=' %s <exp> / '[' !'[' !'=' %s <exp> %s ']' %s '=' %s <exp>
  symbol <- %alpha %alphanum*
  explist <- <exp> (%s ',' %s <exp>)* (%s ',')?
  exp <- <simpleexp> (%s <binop> %s <simpleexp>)*
  simpleexp <- <args> / %string / %longstring / %number / 'true' / 'false' / 
     'nil' / <prefixexp> / <unop> %s <exp> / (. => error)
  unop <- '-' / 'not' / '#' 
  binop <- '+' / '-' / '*' / '/' / '^' / '%' / '..' / '<=' / '<' / '>=' / '>' / '==' / '~=' /
     'and' / 'or'
  prefixexp <- ( {<selector>} -> parseselector / {%name} -> addenv / '(' %s <exp> %s ')' ) 
    ( %s <args> / '.' %name / ':' %name %s ('(' %s ')' / '(' %s <explist> %s ')') / 
    '[' %s <exp> %s ']' / '(' %s ')' / '(' %s <explist> %s ')' / 
    %string / %longstring %s )*
]]

local function pos_to_line(str, pos)
  local s = str:sub(1, pos)
  local line, start = 1, 0
  local newline = string.find(s, "\n")
  while newline do
    line = line + 1
    start = newline
    newline = string.find(s, "\n", newline + 1)
  end
  return line, pos - start
end

local syntax_defs = {
  start = start,
  alpha = alpha,
  alphanum = alphanum,
  name = name,
  number = number,
  string = shortstring,
  longstring = longstring,
  s = space,
  parseselector = parse_selector,
  addenv = function (s) return "env['" .. s .. "']" end,
  state = lpeg.Carg(1),
  error = function (tmpl, pos)
    	        local line, pos = pos_to_line(tmpl, pos)
		error("syntax error in template at line " .. line .. " position " .. pos)
	      end
}

function cosmo_compiler(compiler_funcs)
   syntax_defs.compiletemplate = compiler_funcs.template
   syntax_defs.compiletext = compiler_funcs.text
   syntax_defs.compileapplication = compiler_funcs.template_application
   return re.compile(syntax, syntax_defs)
end
