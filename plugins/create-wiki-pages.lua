--
-- tool to create files of wiki formatted text for easier creation of wiki docs
-- for plugins and/or new additions to the API
--

local docs =
[[
	create-wiki-pages.lua
	---------------------

	Usage:

		lua create-wiki-pages.lua thing.wiki.lua

		where thing.wiki.lua is a file that returns a table in the same
		format as the example provided (threads.wiki.lua)


	Output:

		A directory called wikipages-Thing containing one main file for
		your plugin or API addition and a file for each method described
		in the .wiki.lua file.

		The content of the files should be copy/pastable to pages in the
		Gideros wiki...

			wiki.giderosmobile.com


	Unsolicited advice:

		Please feel free to provide examples in the relevant sections to
		demonstrate how your interface should be used, so that younglings
		and/or padowans - as well as heroic jedis like yourself - can grok
		it.

]]

--
local function title(num, str)
  local s = string.rep("=", num)
  return s .. " " .. str .. " " .. s
end

--
local function bold(str)
  local s = string.rep("'", 3)
  return s .. str .. s
end

--
local function italics(str)
  local s = string.rep("'", 2)
  return s .. str .. s
end

--
local function translate(str)
--  return "<translate>" .. str .. "</translate>"
  return str -- 2020-03-14: removed translate tag
end

--
local function boldTranslateColon(str)
  return bold(translate(str) .. ":") .. " "
end

--
local function boldTitleOneLine(title, content)
  return boldTranslateColon(title) .. content .. "<br/>\n"
end

--
local function writeExamples(file, object)
  if not object.examples or #object.examples == 0 then return end

  local str = #object.examples == 1 and "Example" or "Examples"
  file:write(title(3, translate(str)) .. "\n\n")
  for _, v in ipairs(object.examples) do
	file:write(bold(v.title) .. "<br/>\n")
	file:write([[<source lang="lua">]] .. v.code .. "</source>\n\n")
  end
end

--
local function getReturnValuesString(returns_table)
  local res = {}
  res[#res + 1] = "local "
  for _, v in ipairs(returns_table) do
	if #res > 1 then res[#res + 1] = " " end
	res[#res + 1] = v.name
	res[#res + 1] = ","
  end
  if #res > 1 then
	res[#res] = " = " -- replace last comma
	return table.concat(res)
  else
	return ""
  end
end

--
local function getParenthesisedParamString(param_table)
  local res = {}
  res[#res + 1] = "("
  for _, v in ipairs(param_table) do
	if #res > 1 then res[#res + 1] = " " end
	res[#res + 1] = v.name
	res[#res + 1] = ","
  end
  if #res > 1 then res[#res] = nil end -- remove last comma
  res[#res + 1] = ")"
  return table.concat(res)
end

--
local function makeLink(str)
--  return "[[Special:MyLanguage/".. str .. "|" .. str .. "]]"
  return "[[".. str .. "|" .. str .. "]]" -- 2020-03-14: removed special language tag
end

--
local function writeHeader(file, t)
--  file:write("__NOTOC__\n<languages />\n")
  file:write("__NOTOC__\n\n") -- 2020-03-14: removed language tag
  file:write(boldTitleOneLine("Available since", t.available_since))
end

--
local function writeEventConstantPage(t, class, dir, object)
  local full_name = class.name .. "." .. object.name
  local file = io.open(dir .. "/" .. full_name .. ".wiki", "w")
  writeHeader(file, t)
  if object.value then
	file:write(boldTitleOneLine("Value", object.value))
  end
  file:write(boldTitleOneLine("Defined by", makeLink(class.name)))
  file:write(title(3, translate("Description")) .. "\n")
  file:write(translate(object.desc) .. "<br/>\n")
  writeExamples(file, object)
  file:close()
end

--
local function writeMethodPage(t, class, dir, method)
  local full_name = class.name .. method.delimiter .. method.name
  print("Creating page for " .. full_name)
  local file = io.open(dir .. "/" .. method.name .. ".wiki", "w")
  writeHeader(file, t)
  file:write(boldTitleOneLine("Class", makeLink(class.name)) .. "\n")
  file:write(title(3, translate("Description")) .. "\n")
  if #method.long > 2 then -- CRLF betweem [==[]==]
	file:write(translate(method.long) .. "<br/>\n")
  else
	file:write(translate(method.short) .. "<br/>\n")
  end

  -- create one line syntax usage
  do
	if method.delimiter == ":" then
	  -- make class lowercase, since the code would be operating on an object
	  full_name = full_name:gsub("^%u", string.lower)
	end
	local str = getReturnValuesString(method.returns) .. full_name .. getParenthesisedParamString(method.parameters)
	file:write([[<source lang="lua">]] .. str .. "</source>\n\n")
  end

  writeExamples(file, method)

  file:write(title(3, translate("Parameters")) .. "\n")
  if #method.parameters == 0 then
	file:write(italics(translate("none")) .. "<br/>\n")
  else
	for _, v in ipairs(method.parameters) do
	  local desc = " "
	  if v.desc then desc = desc .. translate(v.desc) end -- make description optional
	  file:write(bold(v.name .. ": ") .. " " .. italics("(" .. v._type .. ")") .. " " ..
				  desc .. "<br/>\n")
	end
  end

  file:write(title(3, translate("Return values")) .. "\n")
  if #method.returns == 0 then
	file:write(italics(translate("none")) .. "<br/>\n")
  else
	for _, v in ipairs(method.returns) do
	  local desc = " "
	  if v.desc then desc = desc .. translate(v.desc) end -- make description optional
	  file:write(bold(v.name .. ": ") .. " " .. italics("(" .. v._type .. ")") .. " " ..
				  desc .. "<br/>\n")
	end
  end

  file:close()
end

-- lookup table
local targets = {
  android = "[[File:Platform android.png]]",
  ios = "[[File:Platform ios.png]]",
  mac = "[[File:Platform mac.png]]",
  pc = "[[File:Platform pc.png]]",
  html5 = "[[File:Platform html5.png]]",
  winrt = "[[File:Platform winrt.png]]",
  win32 = "[[File:Platform win32.png]]"
}

--
local function getTargetString(sp)
  local t = {}
  for _, v in ipairs(sp) do
	t[#t + 1] = targets[v]
  end
  return table.concat(t)
end

--
local function writeClassPages(dir, class, t)
  local filename = class.name .. ".wiki"
  local file = io.open(dir .. "/" .. filename, "w")
--  file:write("__NOTOC__\n<languages />\n")
  file:write("__NOTOC__\n\n") -- 2020-03-14: removed language tag
  file:write("<!-- GIDEROSOBJ:".. class.name .." -->\n")

  print("Creating main page... " .. dir .. "/" .. filename)
  file:write(boldTitleOneLine("Supported platforms",
								getTargetString(t.supported_platforms)))
  file:write(boldTitleOneLine("Available since", t.available_since))
  if #class.inherits_from > 0 then
	file:write(boldTitleOneLine("Inherits from", makeLink(class.inherits_from[1])))
	file:write("\n")
  end
  file:write(title(3, translate("Description")) .. "\n")
  file:write(translate(class.description) .. "<br/>\n")

  writeExamples(file, class)

  -- start column section
  file:write([[{|-]] .. "\n" .. [[| style="width: 50%; vertical-align:top;"|]] .."\n")

  file:write("\n"..title(3, translate("Methods")) .. "\n")
  if #class.methods > 0 then
	for _, v in ipairs(class.methods) do
	  local full_name = class.name .. v.delimiter .. v.name
	  file:write(
	  makeLink(full_name) .. "  " .. italics(translate(v.short))
			  .."<br/><!-- GIDEROSMTD:" .. full_name
			  .. getParenthesisedParamString(v.parameters) .. " "
			  .. v.short .. " -->")
	  file:write("\n")
	  writeMethodPage(t, class, dir, v)
	end
  else
	file:write("\n" .. italics("none") .. "<br/>\n\n")
  end

  -- events and constants in second column
  file:write([[| style="width: 50%; vertical-align:top;"|]] .."\n")
  file:write(title(3, translate("Events")) .. "\n")
  if #class.events > 0 then
	for _, v in ipairs(class.events) do
	  file:write(makeLink(v.name) .. "<br/>\n")
	  writeEventConstantPage(t, class, dir, v)
	end
  else
	file:write("\n" .. italics("none") .. "<br/>\n\n")
  end

  file:write(title(3, translate("Constants")) .. "\n")
  if #class.constants > 0 then
	for _, v in ipairs(class.constants) do
	  file:write(makeLink(v.name) .. "<br/>\n")
	  writeEventConstantPage(t, class, dir, v)
	end
  else
	file:write("\n" .. italics("none") .. "<br/>\n\n")
  end

  -- finish column section
  file:write("|}")
  file:close()
end

--
local function outputAPI(t)
  print("Starting....\n")

  local dir = "wikipages-" .. t.title
  print("Creating directory..." .. dir)
  os.execute("mkdir " .. dir)

  if #t.classes > 1 or (t.title ~= t.classes[1].name)
					and t.title ~= "" and t.title ~= nil then
	-- create landing page for feature with links to class/es
	local filename = t.title .. ".wiki"
	local file = io.open(dir .. "/" .. filename, "w")
--	file:write("__NOTOC__\n<languages />\n")
	file:write("__NOTOC__\n\n") -- 2020-03-14: removed language tag
	file:write("<!-- GIDEROSOBJ:".. t.title .." -->\n")

	print("Creating feature landing page... " .. dir .. "/" .. filename)
	file:write(boldTitleOneLine("Supported platforms",
								  getTargetString(t.supported_platforms)))
	file:write(boldTitleOneLine("Available since", t.available_since))

	file:write(title(3, translate("Description")) .. "\n")
	file:write(translate(t.description) .. "<br/>\n")

	writeExamples(file, t)

	local plural = #t.classes == 1 and "" or "es"
	file:write(title(3, translate("Class" .. plural)) .. "\n")
	for _, v in ipairs(t.classes) do
	  file:write(makeLink(v.name) .. "<br/>\n")
	end
	file:close()
  end

  for _, v in ipairs(t.classes) do
	writeClassPages(dir, v, t)
  end

  return dir
end

--
local function main(args)
  if args[1] == nil then print(docs) return end
  local file = io.open(args[1], "r")
  if not file then
	print("Error opening file: \"" .. args[1] .. "\"")
	return
  end
  local contents = file:read("*a")
  if not contents then
	print("Error: file, \"" .. args[1] .. "\" is empty.")
	return
  end
  local t = assert(loadstring(contents))()

  local dir = outputAPI(t)
  file:close()
  print("\n\nFinished. Please check directory... \n\n    " .. dir .. "\n\n...for files.\n")
end

local args = {...}
main(args)
