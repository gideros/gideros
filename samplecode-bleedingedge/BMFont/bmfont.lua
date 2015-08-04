-- check if string1 starts with string2
local function startsWith(string1, string2)
   return string1:sub(1, #string2) == string2
end

-- create table from a bmfont line
local function lineToTable(line)
	local result = {}
	for pair in line:gmatch("%a+=[-%d]+") do 	
		local key = pair:match("%a+")
		local value = pair:match("[-%d]+")
		result[key] = tonumber(value)
	end
	return result
end

-- this is our BMFont class
BMFont = {}
BMFont.__index = BMFont

-- and its new function
function BMFont.new(...)
	local self = setmetatable({}, BMFont)
	if self.init ~= nil and type(self.init) == "function" then
		self:init(...)
	end	
	return self
end

function BMFont:init(fontfile, imagefile, filtering)
	-- load font texture
	self.texture = Texture.new(imagefile, filtering)

	-- read glyphs from fontfile and store them in chars table
	self.chars = {}
	file = io.open(fontfile, "rt")
	for line in file:lines() do	
		if startsWith(line, "char ") then
			local char = lineToTable(line)
			self.chars[char.id] = char
		end
	end
	io.close(file)
end

-- this is our BMTextField class
BMTextField = gideros.class(Sprite)

function BMTextField:init(font, str)
	self.font = font
	self.str = str
	self:createCharacters()
end

function BMTextField:setText(str)
	if self.str ~= str then
		self.str = str
		self:createCharacters()
	end
end

function BMTextField:createCharacters()
	-- remove all children
	for i=self:getNumChildren(),1,-1 do
		self:removeChildAt(i)
	end
	
	-- create a TextureRegion from each character and add them as a Bitmap
	local x = 0
	local y = 0
	for i=1,#self.str do
		local char = self.font.chars[self.str:byte(i)]
		if char ~= nil then
			local region = TextureRegion.new(self.font.texture, char.x, char.y, char.width, char.height)
			local bitmap = Bitmap.new(region)
			bitmap:setPosition(x + char.xoffset, y + char.yoffset)
			self:addChild(bitmap)
			x = x + char.xadvance
		end
	end
end