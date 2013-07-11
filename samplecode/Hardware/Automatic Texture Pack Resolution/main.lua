--[[
Automatic selection of different resolution texture packs according to scale

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 
]]

local pack = TexturePack.new("pack.txt", "pack.png")

local hammer = pack:getTextureRegion("hammer.png")
local monkeywrench = pack:getTextureRegion("monkeywrench.png")
local pliers = pack:getTextureRegion("pliers.png")
local screwdriver = pack:getTextureRegion("screwdriver.png")

local hammerb = Bitmap.new(hammer)
local monkeywrenchb = Bitmap.new(monkeywrench)
local pliersb = Bitmap.new(pliers)
local screwdriverb = Bitmap.new(screwdriver)

hammerb:setPosition(30, 30)
monkeywrenchb:setPosition(150, 30)
pliersb:setPosition(30, 150)
screwdriverb:setPosition(150, 150)

stage:addChild(hammerb)
stage:addChild(monkeywrenchb)
stage:addChild(pliersb)
stage:addChild(screwdriverb)
