require("lp")

local function trim(s)
  return (s:gsub("^%s*(.-)%s*$", "%1"))
end

--[[
local function trimLines(lines)
	while lines[1] == "" do
		table.remove(lines, 1)
	end

	while lines[#lines] == "" do
		table.remove(lines, #lines)
	end
end
--]]

function checkObsolete(str)
	local keyword = "(obsolete)"
	local len = #keyword
	if str:sub(-len) == keyword then
		return true, trim(str:sub(0, -(len + 1)))
	end
	return false, str
end


local function doInfo(first, line)
	info[#info + 1] = line
end

local function doClass(first, line)
	if first then
		className = trim(line)

		local i = className:find(">")

		if i then
			baseClass = trim(className:sub(i+1))
			className = trim(className:sub(1, i-1))
		end
	else
		classDescription[#classDescription + 1] = line
	end
end

local function doCategory(first, line)
	if first then
		category = trim(line)
	end
end

local function doFunction(first, line)
	if first then
		isObsolete, functionName = checkObsolete(trim(line))
	else
		functionDescription[#functionDescription + 1] = line
	end
end

local function doParameters(first, line)
	local i, j = line:find("%-")
	if i then
		parameters[#parameters + 1] = {name=trim(line:sub(1, i-1)), description=trim(line:sub(i+1))}
	end
end

local function doReturns(first, line)
	returns[#returns + 1] = line
end
local function doSeeAlso(first, line)
	seeAlso[#seeAlso + 1] = line
end

local function doExample(first, line)
	example[#example + 1] = line
end


local function doEnd(first)
	if first then
		if functionName then
			local paramnames = {}
			for i=1,#parameters do
				paramnames[i] = parameters[i].name
			end

			local functionFullName = functionName .. "(" .. table.concat(paramnames, ", ") .. ")"

			doc[#doc+1] =  {
				type = "function",
				functionName = functionName,
				functionFullName = functionFullName,
				functionDescription = table.concat(functionDescription, "\n"),
				parameters = parameters,
				returns = table.concat(returns, "\n"),
				seeAlso = table.concat(seeAlso, "\n"),
				example = table.concat(example, "\n"),
				isObsolete = isObsolete,
			}
		end

		if className then
			doc[#doc+1] =  {
				type = "class",
				className = className,
				baseClass = baseClass,
				classDescription = table.concat(classDescription, "\n"),
				example = table.concat(example, "\n"),
			}
		end

		if #info > 0 then
			doc[#doc+1] =  {
				type = "info",
				info = table.concat(info, "\n"),
			}
		end

		className = nil
		baseClass = nil
		classDescription = {}
		functionName = nil
		functionDescription = {}
		parameters = {}
		returns = {}
		seeAlso = {}
		example = {}
		info = {}
	end
end

local keywords = {
	["Info:"] = doInfo,
	["Class:"] = doClass,
	["Category:"] = doCategory,
	["Function:"] = doFunction,
	["Parameters:"] = doParameters,
	["Returns:"] = doReturns,
	["See Also:"] = doSeeAlso,
	["Example:"] = doExample,
	["%-%-%-%-%-%-%-%-"]  = doEnd,
}

function convertReferences(line)
	return line:gsub("%[%[([%w_%:%.]+)%]%]", "%[%1%]%(#%1%)")
end


io.input("template.lp", "r")
template = io.read("*all");

function clear()
	className = nil
	baseClass = nil
	category = nil
	classDescription = {}
	functionName = nil
	functionDescription = {}
	parameters = {}
	returns = {}
	seeAlso = {}
	example = {}
	info = {}
	isObsolete = false

	doc = {}
end


function parse(inputfile)
	local currf = nil
	for line in io.lines(inputfile) do
		line = convertReferences(line)
		local minj = 1e30
		local first = false
		for k,f in pairs(keywords) do
			local i,j = line:find("^%s*"..k)
			if j and j < minj then
				currf = f
				minj = j
				first = true
			end
		end

		if first then
			line = line:sub(minj + 1)
		end

		if currf then
			currf(first, line)
		end
	end
	doEnd(true)
end

function postpass()
	local lastclass = nil
	toc = {}
	for i=1,#doc do
		if doc[i].type == "class" then
			toc[#toc + 1] = {
				className = doc[i].className,
				baseClass = doc[i].baseClass,
			}
			lastclass = toc[#toc]
		end

		if doc[i].type == "function" then
			lastclass[#lastclass + 1] = doc[i].functionName
		end
	end
end


function output(outputfile)
	postpass()
	io.output(outputfile, "w")
	loadstring(lp.translate(template))()
end

clear()
parse("events_ref.txt")
doc.toc = true
output("events_ref.html")
clear()
parse("graphics.txt")
doc.toc = true
output("graphics.html")
clear()
parse("audio.txt")
doc.toc = true
output("audio.html")
clear()
parse("utils.txt")
doc.toc = true
output("utils.html")
clear()
parse("math.txt")
doc.toc = true
output("math.html")
clear()
parse("physics.txt")
doc.toc = true
output("physics.html")
clear()
parse("analytics.txt")
doc.toc = true
output("analytics.html")
clear()
parse("geolocation.txt")
doc.toc = true
output("geolocation.html")
clear()
parse("gyroscope.txt")
doc.toc = true
output("gyroscope.html")
clear()
parse("accelerometer.txt")
doc.toc = true
output("accelerometer.html")


clear()
parse("events.txt")
output("events.html")

clear()
parse("file_system.txt")
output("file_system.html")

clear()
parse("getting_started.txt")
output("getting_started.html")

clear()
parse("deployment.txt")
output("deployment.html")

clear()
parse("index.txt")
output("index.html")

clear()
parse("classes_in_gideros.txt")
output("classes_in_gideros.html")

clear()
parse("automatic_screen_scaling.txt")
output("automatic_screen_scaling.html")

clear()
parse("automatic_image_resolution.txt")
output("automatic_image_resolution.html")

clear()
parse("migrating_from_2011.9_to_2012.2.txt")
output("migrating_from_2011.9_to_2012.2.html")

clear()
parse("iad.txt")
output("iad.html")

clear()
parse("facebook.txt")
output("facebook.html")

clear()
parse("events_ref.txt")
parse("graphics.txt")
parse("application.txt")
parse("audio.txt")
parse("urlloader.txt")
parse("utils.txt")
parse("math.txt")
parse("physics.txt")
parse("geolocation.txt")
parse("gyroscope.txt")
parse("accelerometer.txt")
parse("keycode.txt")
parse("nativeui.txt")
parse("analytics.txt")
parse("sqlite3.txt")
parse("storekit.txt")
parse("iad.txt")
parse("facebook.txt")
parse("googlebilling.txt")
doc.toc = true
output("reference_manual.html")


