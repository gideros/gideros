if false then
	local letterSpacing = 10

	--local font = Font.new("arial_50_old.txt", "arial_50_old.png")
	--local font = Font.new("arial_50_new.txt", "arial_50_new.png")
	--local font = TTFont.new("arial.ttf", 50)
	--local font = TTFont.new("arial.ttf", 50, "Walter")
	local text = TextField.new(font, "Walter")
	text:setLetterSpacing(letterSpacing)
	text:setTextColor(0x306090)
	text:setPosition(10, 150)
	stage:addChild(text)
	
	local str = "Walter"
	
	for i=1,#str do
		local text = TextField.new(font, str:sub(i, i))
		text:setTextColor(0x202020 * i)
		local advanceX = font:getAdvanceX(str, letterSpacing, i-1)
		text:setPosition(10 + advanceX, 180)
		print(advanceX)
		stage:addChild(text)
	end

	return
end



--helper function to draw bounds
local function bounds(font, textField)
	print(textField:getBounds(textField))
	print(font:getBounds(textField:getText(), textField:getLetterSpacing()))
	local x, y, w, h = textField:getBounds(stage)
	local shape = Shape.new()
	shape:setLineStyle(1, 0x000000, 0.25)
	shape:beginPath()
	shape:moveTo(x, y)
	shape:lineTo(x + w, y)
	shape:lineTo(x + w, y + h)
	shape:lineTo(x, y + h)
	shape:closePath()
	shape:endPath()
	stage:addChild(shape)
end

-- old format (50pt)
do
	local font = Font.new("arial_50_old.txt", "arial_50_old.png")
	local text = TextField.new(font, "Abg")
	text:setLetterSpacing(10)
	text:setTextColor(0x306090)
	text:setPosition(10, 50)
	stage:addChild(text)
	bounds(font, text)
end

-- new format (50pt)
do
	local font = Font.new("arial_50_new.txt", "arial_50_new.png")
	local text = TextField.new(font, "Abg")
	text:setLetterSpacing(10)
	text:setTextColor(0x306090)
	text:setPosition(10, 100)
	stage:addChild(text)
	bounds(font, text)
end


-- ttf not cached (50pt)
do
	local font = TTFont.new("arial.ttf", 50)
	local text = TextField.new(font, "Abg")
	text:setLetterSpacing(10)
	text:setTextColor(0x306090)
	text:setPosition(10, 150)
	stage:addChild(text)
	bounds(font, text)
end

-- ttf cached (50pt)
do
	local font = TTFont.new("arial.ttf", 50, " Abg")
	local text = TextField.new(font, "Abg")
	text:setLetterSpacing(10)
	text:setTextColor(0x306090)
	text:setPosition(10, 200)
	stage:addChild(text)
	bounds(font, text)
end

-- old format (10pt)
do
	local font = Font.new("arial_10_old.txt", "arial_10_old.png")
	local text = TextField.new(font, "ABCDEFGabcdefg")
	text:setLetterSpacing(0)
	text:setTextColor(0x306090)
	text:setPosition(140, 50)
	stage:addChild(text)
	bounds(font, text)
end

-- new format (10pt)
do
	local font = Font.new("arial_10_new.txt", "arial_10_new.png")
	local text = TextField.new(font, "ABCDEFGabcdefg")
	text:setLetterSpacing(0)
	text:setTextColor(0x306090)
	text:setPosition(140, 100)
	stage:addChild(text)
	bounds(font, text)
end


-- ttf not cached (10pt)
do
	local font = TTFont.new("arial.ttf", 10)
	local text = TextField.new(font, "ABCDEFGabcdefg")
	text:setLetterSpacing(0)
	text:setTextColor(0x306090)
	text:setPosition(140, 150)
	stage:addChild(text)
	bounds(font, text)
end

-- ttf cached (10pt)
do
	local font = TTFont.new("arial.ttf", 10, "ABCDEFGabcdefg")
	local text = TextField.new(font, "ABCDEFGabcdefg")
	text:setLetterSpacing(0)
	text:setTextColor(0x306090)
	text:setPosition(140, 200)
	stage:addChild(text)
	bounds(font, text)
end

-- old format (intr, 20pt)
do
	local font = Font.new("arial_20_intr_old.txt", "arial_20_intr_old.png")
	local text = TextField.new(font, "Îñţérñåţîöñåļîžåţîöñ (UTF-8)")
	text:setLetterSpacing(0)
	text:setTextColor(0x306090)
	text:setPosition(10, 240)
	stage:addChild(text)
	bounds(font, text)
end

-- new format (intr, 20pt)
do
	local font = Font.new("arial_20_intr_new.txt", "arial_20_intr_new.png")
	local text = TextField.new(font, "Îñţérñåţîöñåļîžåţîöñ (UTF-8)")
	text:setLetterSpacing(0)
	text:setTextColor(0x306090)
	text:setPosition(10, 270)
	stage:addChild(text)
	bounds(font, text)
end

-- ttf not cached (intr, 20pt)
do
	local font = TTFont.new("arial.ttf", 20)
	local text = TextField.new(font, "Îñţérñåţîöñåļîžåţîöñ (UTF-8)")
	text:setLetterSpacing(0)
	text:setTextColor(0x306090)
	text:setPosition(10, 300)
	stage:addChild(text)
	bounds(font, text)
end

-- ttf cached (intr, 20pt)
do
	local font = TTFont.new("arial.ttf", 20, "Îñţérñåţîöñåļîžåţîöñ (UTF-8)")
	local text = TextField.new(font, "Îñţérñåţîöñåļîžåţîöñ (UTF-8)")
	text:setLetterSpacing(0)
	text:setTextColor(0x306090)
	text:setPosition(10, 330)
	stage:addChild(text)
	bounds(font, text)
end

