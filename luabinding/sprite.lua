--[[
Sprite.numChildren = Sprite.getNumChildren
Sprite.childAt = Sprite.getChildAt
Sprite.getParent = Sprite.parent
Sprite.childIndex = Sprite.getChildIndex
Sprite.x = Sprite.getX
Sprite.y = Sprite.getY
Sprite.rotation = Sprite.getRotation
Sprite.scaleX = Sprite.getScaleX
Sprite.scaleY = Sprite.getScaleY
Sprite.visible = Sprite.getVisible
Sprite.colorTransform = Sprite.getColorTransform
Sprite.width = Sprite.getWidth
Sprite.height = Sprite.getHeight
Sprite.matrix = Sprite.getMatrix
Sprite.alpha = Sprite.getAlpha
--]]


local setBlendFunc = Sprite.setBlendFunc
local clearBlendFunc = Sprite.clearBlendFunc

Sprite.setBlendFunc = nil
Sprite.clearBlendFunc = nil

local SRC_ALPHA = BlendFactor.SRC_ALPHA
local ONE_MINUS_SRC_ALPHA = BlendFactor.ONE_MINUS_SRC_ALPHA
local ONE = BlendFactor.ONE
local ZERO = BlendFactor.ZERO
local DST_COLOR = BlendFactor.DST_COLOR
local ONE_MINUS_SRC_COLOR = BlendFactor.ONE_MINUS_SRC_COLOR

BlendFactor = nil

Sprite.ALPHA = "alpha"
Sprite.NO_ALPHA = "noAlpha"
Sprite.ADD = "add"
Sprite.MULTIPLY = "multiply"
Sprite.SCREEN = "screen"
 
function Sprite:setBlendMode(mode)
	if type(mode) ~= "string" then
		error("bad argument #2 to 'setBlendMode' (string expected, got "..type(mode)..")")
	end
 
	if mode == Sprite.ALPHA then
		setBlendFunc(self, ONE, ONE_MINUS_SRC_ALPHA)
	elseif mode == Sprite.NO_ALPHA then
		setBlendFunc(self, ONE, ZERO)
	elseif mode == Sprite.ADD then
		setBlendFunc(self, ONE, ONE)
	elseif mode == Sprite.MULTIPLY then
		setBlendFunc(self, DST_COLOR, ONE_MINUS_SRC_ALPHA)
	elseif mode == Sprite.SCREEN then
		setBlendFunc(self, ONE, ONE_MINUS_SRC_COLOR)
	else
		error("Parameter 'blendMode' must be one of the accepted values.")
	end
end
 
Sprite.clearBlendMode = clearBlendFunc


