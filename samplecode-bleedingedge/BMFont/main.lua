-- load font
local font = BMFont.new("font.txt", "font.png")

-- create text
local text = BMTextField.new(font, "Hello Gideros!")

-- add to stage
stage:addChild(text)

-- you can also change the text
text:setText("Hello Giderans!")
