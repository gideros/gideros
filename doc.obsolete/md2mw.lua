-- markdown to mediawiki

local function emphasis(line, pattern, rep1, rep2)
	local i, j = line:find(pattern.."[^%s]")
	if i and j then
		local k, l = line:find("[^%s]"..pattern, i)

		if k and l then
			return line:sub(1, i-1)..rep1..line:sub(j, k)..rep2..line:sub(l+1)
		end
	end
end

local function fullemphasis(line, pattern, rep1, rep2)
	while true do
		local newline = emphasis(line, pattern, rep1, rep2)
		if newline then
			line = newline
		else
			break
		end
	end

	return line
end

local function doimage(line)
	local a, b = line:find("![", 1, true)
	if a and b then
		local c, d = line:find("]", b, true)

		if c and d then
			local e, f = line:find("(", d, true)

			if e and f then
				local g, h = line:find(")", f, true)

				line = line:sub(e+1, g-1)

				line = "[[File:"..line:sub(-line:reverse():find("/", 1, true)+1).."]]"

			end
		end
	end

	return line
end


-- [This link](http://example.net/)   -> [http://example.net/ This link]
local function dolink(line)
	local a = line:find("[", 1, true)
	if a then
		local b = line:find("]", a, true)

		if b then
			local c = line:find("(", b, true)

			if c then
				local d = line:find(")", c, true)

				line = line:sub(1, a-1).."["..line:sub(c+1, d-1).." "..line:sub(a+1, b-1).."]"..line:sub(d+1)
			end
		end
	end

	return line
end



local function convert(infile, outfile)
	io.output(outfile)

	local code = false

	for line in io.lines(infile) do
		if line:find("Info:", 1, true) then
			line = ""
		end



		local currcode = line:find("^\t") ~= nil

		if code == false and currcode == true then
			io.write('<source lang="lua">\n')
			code = true
		elseif code == true and currcode == false then
			io.write('</source>\n')
			code = false
		end

		if code then
			line = line:gsub("^\t", "")	-- delete first tab
		end


		line = line:gsub("^%#([^%#]+)", "=%1 =")				-- header 1
		line = line:gsub("^%#%#([^%#]+)", "==%1 ==")			-- header 2
		line = line:gsub("^%#%#%#([^%#]+)", "===%1 ===")		-- header 3
		line = line:gsub("^%#%#%#%#([^%#]+)", "====%1 ====")	-- header 4

		line = fullemphasis(line, "%*%*", "'''", "'''")
		line = fullemphasis(line, "%*", "''", "''")
		line = fullemphasis(line, "%`", "<code>", "</code>")

		line = doimage(line)
		line = dolink(line)

		io.write(line .. "\n")
	end
end


convert("events.txt", "events.mw")
convert("getting_started.txt", "getting_started.mw")
convert("classes_in_gideros.txt", "classes_in_gideros.mw")
convert("deployment.txt", "deployment.mw")
convert("file_system.txt", "file_system.mw")
convert("automatic_screen_scaling.txt", "automatic_screen_scaling.mw")
convert("automatic_image_resolution.txt", "automatic_image_resolution.mw")




