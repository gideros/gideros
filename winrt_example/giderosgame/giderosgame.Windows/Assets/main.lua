--[[

An example of loading and displaying custom fonts

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2013 Gideros Mobile 

]]

local font1 = Font.new("billo.fnt", "billo.png")		-- this is bitmap font
local font2 = TTFont.new("billo.ttf", 50, " !abcdefgh")	-- this is TTFont with caching
local font3 = TTFont.new("arial.ttf", 20)				-- this is TTFont without caching

local text1 = TextField.new(font1, "!!abcdefgh!!")
local text2 = TextField.new(font2, "!!abcdefgh!!")
local text3 = TextField.new(font3, "Îñţérñåţîöñåļîžåţîöñ (UTF-8)")
local text4 = TextField.new(nil, "and this bottom line shows embedded system font")

text2:setTextColor(0xff6090)

text1:setPosition(10, 100)
text2:setPosition(10, 200)
text3:setPosition(10, 300)
text4:setPosition(10, 380)

stage:addChild(text1)
stage:addChild(text2)
stage:addChild(text3)
stage:addChild(text4)
