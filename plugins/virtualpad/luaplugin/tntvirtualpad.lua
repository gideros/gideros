-- ######################################## --
-- TNT Virtual Pad v1.41                    --
-- Copyright (C) 2012 By Gianluca D'Angelo  --
-- All right Reserved.                      --
-- YOU CAN USE IN FREE OR COMMERCIAL GAMES  --
-- PLEASE DONATE TO KEEP THIS PROJECT ALIVE --
-- ---------------------------------------- --
-- for bug, tips, suggestion contact me at  --
-- gregbug@gmail.com or                     --
-- support@tntparticlesengine.com           --
-- ---------------------------------------- --
-- coded for Gideros Mobile SDK             --
-- ######################################## --

-- GLOBALS "CONSTANTS" (NOT REALLY CONSTANTS BUT...) --

-- v1.41 Add vPad:reset()
--		 reset virtual pads to default "null" position
-- v1.40 fix setMaxRadius half screen Bug...
--       added 2 players example
-- v1.36 fix for replacing a texture resulted in the pressed button image being displayed
-- v1.35 if not layerIndex defined or layerIndex < 1  then layerIndex = 1
-- v1.34 fix for bug with moveable sticks
-- v1.33 fix for portrait 4 buttons
-- v1.32 added screen restrict for setting screen height from base, default to height
-- v1.31 optional vertical border space also activated flipped mode if negative, else same as horizontal border space
-- v1.3  5 and 6 button combo's added by SinisterSoft
-- v1.2  vPad event bug fixed (reported by Tom2012) - examples updated to work fine with gideros 2012.9
--       pad layer gfx fix thx to tom2012
--		 added Function setMaxRadius() thx to Tom2012
-- v1.1  Optimized calcAngle function 20/07/2012
--       function CTNTPadBase:setTextures(spriteA, spriteB) !! BUG FIX - >> reported by Gleen Bacon
-- 
PAD = {
    -- ====================================================================================== --
    STICK_NONE = 0, -- no joystick
    STICK_SINGLE = 1, -- one joystick (default left)
    STICK_DOUBLE = 2, -- two joystick (no button)
    -- ====================================================================================== --
    BUTTONS_NONE = 3, -- no buttons
    BUTTONS_ONE = 4, -- 1 button
    BUTTONS_TWO = 5, -- 2 buttons
    BUTTONS_THREE = 6, -- 3 buttons
    BUTTONS_FOUR = 7, -- 4 buttons
    BUTTONS_FIVE = 8, -- 5 buttons
    BUTTONS_SIX = 9, -- 6 buttons
    -- ====================================================================================== --
    STYLE_CLASSIC = 10, -- classic virtual pad (fixed)
    STYLE_MOVABLE = 11, -- joystick movable
    STYLE_FOLLOW = 12, -- joystick follow user finger
    -- ====================================================================================== --
    MODE_NOHIDE = 13, -- never hide pad
    MODE_GHOST = 14, -- pad is semi-transparent after some time
    MODE_HIDDEN = 15, -- pad is hidden by default and show only when user touch the screen
    -- ====================================================================================== --
    STATE_NONE = 16, -- nothing happen on button
    STATE_BEGIN = 17, -- button is touched
    STATE_DOWN = 18, -- button is touched
    STATE_END = 19, -- button is released
    -- ====================================================================================== --
    COMPO_BUTTON1 = 20, -- define button 1 pad component
    COMPO_BUTTON2 = 21, -- define button 2 pad component
    COMPO_BUTTON3 = 22, -- define button 3 pad component
    COMPO_BUTTON4 = 23, -- define button 4 pad component
    COMPO_BUTTON5 = 24, -- define button 4 pad component
    COMPO_BUTTON6 = 25, -- define button 4 pad component
    -- ====================================================================================== --
    COMPO_LEFTPAD = 26, -- define left stick pad component
    COMPO_RIGHTPAD = 27, -- define right stick pad component

    -- ====================================================================================== --
    BUTTON1_EVENT = "B1E", -- default TNT Virtual button1 pad event name
    BUTTON2_EVENT = "B2E", -- default TNT Virtual button2 pad event name
    BUTTON3_EVENT = "B3E", -- default TNT Virtual button3 pad event name
    BUTTON4_EVENT = "B4E", -- default TNT Virtual button4 pad event name
    BUTTON5_EVENT = "B5E", -- default TNT Virtual button4 pad event name
    BUTTON6_EVENT = "B6E", -- default TNT Virtual button4 pad event name
    LEFTPAD_EVENT = "LE", -- default TNT Virtual left pad event name
    RIGHTPAD_EVENT = "RE", -- default TNT Virtual right pad event name
    ERRORHEADER = "TNT VirtualPAD ERROR", -- default error header message
    -- ====================================================================================== --
    SPRITE_BUTTONUP = "tntvpad_buttonup.png", -- default button up sprite name
    SPRITE_BUTTONDOWN = "tntvpad_buttondown.png", -- default button sprite down name
    SPRITE_JOYUP = "tntvpad_analogpad.png",
    SPRITE_JOYDOWN = "tntvpad_base.png"
}


-- ################################################################################################################### --
-- ################################################################################################################### --
-- ################################################################################################################### --

-- ************************************************ --
-- ************************************************ --
-- simple locals math/misc/gfx helper functions *** --
-- ************************************************ --
-- ************************************************ --

-- ################################################################################################################### --
-- ################################################################################################################### --
-- ################################################################################################################### --

local qSin = math.sin
local qCos = math.cos
local qFloor = math.floor
local qAtan2 = math.atan2
local qSqrt = math.sqrt

-------------------------------------
-- set default button sprites name --
-------------------------------------
function setDefaultButtonSprites(spriteA, spriteB)
    PAD.SPRITE_BUTTONUP = spriteA
    PAD.SPRITE_BUTTONDOWN = spriteB
end

function setDefaultJoySprites(spriteA, spriteB)
    PAD.SPRITE_JOYUP = spriteA
    PAD.SPRITE_JOYDOWN = spriteB
end

---------------------------------------
-- calculates angle between 2 points --
---------------------------------------
local function calcAngle(xA, yA, xB, yB)
    return qAtan2((yB - yA), (xB - xA))
end

------------------------------------------
-- calculates distance between 2 points --
------------------------------------------
local function distance(xA, yA, xB, yB)
    local dx = (xB - xA)
    local dy = (yB - yA)
    return qSqrt(dx * dx + dy * dy)
end

--------------------------
-- check if file exists --
--------------------------
local function fileExists(fileName)
    local file = io.open(fileName)
    if file ~= nil then
        io.close(file)
    end
    return file == nil
end

-- ################################################################################################################### --
-- ################################################################################################################### --
-- ################################################################################################################### --

-- ********************************** --
-- ********************************** --
-- *** TNT Virtual Pad Base class *** --
-- ********************************** --
-- ********************************** --

-- ################################################################################################################### --
-- ################################################################################################################### --
-- ################################################################################################################### --

local CTNTPadBase = Core.class(Sprite)

------------------------
-- init Pad component --
------------------------
function CTNTPadBase:init()
    self.parent = nil -- parent sprite
    self.skin = nil -- texture

    self.spriteA = nil -- sprite 1
    self.spriteB = nil -- sprite 2
    self.hidden = false -- component is hidden ?
    self.alphaOn = .7 -- default max alpha on
    self.alphaOff = .2 -- default min alpha off

    self.startX = 0 -- x start position
    self.startY = 0 -- y start position

    self.xPos = 0 -- current x position
    self.yPos = 0 -- current y position

    self.r = 1 -- red color value
    self.g = 1 -- green color value
    self.b = 1 -- blue color value
    self.a = 1 -- alpha color value

    self.widthA = 0 -- width of pad component sprite A
    self.heightA = 0 -- height of pad component sprite A
    self.hWidthA = 0 -- width/2 of pad component sprite A
    self.hHeightA = 0 -- height/2 of pad component sprite A

    self.widthB = 0 -- width of pad component sprite B
    self.heightB = 0 -- height of pad component sprite B
    self.hWidthB = 0 -- width/2 of pad component sprite B
    self.hHeightB = 0 -- height/2 of pad component sprite B

    self.scale = 1 -- gfx scale
end

---------------------------------
-- update component properties --
---------------------------------
function CTNTPadBase:updateProperties()
    if self.spriteA ~= nil then
        self.widthA = self.spriteA:getWidth()
        self.heightA = self.spriteA:getHeight()
        self.hWidthA = self.widthA / 2 -- width/2 of pad component sprite A
        self.hHeightA = self.heightA / 2 -- height/2 of pad component sprite A
    else
        self.widthA = 0
        self.heightA = 0
        self.hWidthA = 0
        self.hHeightA = 0
    end
    if self.spriteB ~= nil then
        self.widthB = self.spriteB:getWidth()
        self.heightB = self.spriteB:getHeight()
        self.hWidthB = self.widthB / 2 -- width/2 of pad component sprite B
        self.hHeightB = self.heightB / 2 -- height/2 of pad component sprite B
    else
        self.widthB = 0
        self.heightB = 0
        self.hWidthB = 0
        self.hHeightB = 0
    end
end

------------------------
-- free Pad component --
------------------------
function CTNTPadBase:free()
    -- remove sprites from stage
    if self.spriteA ~= nil then
        if self.parent:contains(self.spriteA) then
            self.parent:removeChild(self.spriteA)
        end
    end
    if self.spriteB ~= nil then
        if self.parent:contains(self.spriteB) then
            self.parent:removeChild(self.spriteB)
        end
    end
    self.parent = nil
    self.skin = nil
    self.spriteA = nil
    self.spriteB = nil
    self = nil
    return nil
end

----------------------------------
-- set component x, y positions --
----------------------------------
function CTNTPadBase:setPosition(x, y)
    self.xPos = x
    self.yPos = y
    self.startX = x
    self.startY = y
    self.spriteA:setPosition(x, y)
    self.spriteB:setPosition(x, y)
end

---------------------------------
-- get component x, y position --
---------------------------------
function CTNTPadBase:getPosition()
    return self.xPos, self.yPos
end

---------------------------------
-- set component color r,g,b,a --
---------------------------------
function CTNTPadBase:setColor(r, g, b, ...)
	local arg = {...}
    local a = arg[1]
    self.r = r / 255
    self.g = g / 255
    self.b = b / 255
    if a ~= nil then
        self.a = a
        self.spriteA:setColorTransform(self.r, self.g, self.b, self.a)
        self.spriteB:setColorTransform(self.r, self.g, self.b, self.a)
    else
        self.spriteA:setColorTransform(self.r, self.g, self.b)
        self.spriteB:setColorTransform(self.r, self.g, self.b)
    end
end

--------------------
-- set Visible or not a component --
--------------------
function CTNTPadBase:setVisible(show, mode)
    self.hidden = not show
    if mode == PAD.MODE_NOHIDE then
        self.spriteA:setAlpha(self.alphaOn)
        self.spriteB:setAlpha(self.alphaOn)
    elseif mode == PAD.MODE_GHOST then
        if show then
            self.spriteA:setAlpha(self.alphaOn)
            self.spriteB:setAlpha(self.alphaOn)
        else
            self.spriteA:setAlpha(self.alphaOff)
            self.spriteB:setAlpha(self.alphaOff)
        end
    elseif mode == PAD.MODE_HIDDEN then
        self.spriteA:setAlpha(self.alphaOn)
        self.spriteB:setAlpha(self.alphaOn)
        self.spriteA:setVisible(show)
        self.spriteB:setVisible(show)
    end
end

------------------------------------
-- get if component hidden or not --
------------------------------------
function CTNTPadBase:isVisible()
    return not self.hidden
end

-----------------------
-- set alpha max,min --
-----------------------
function CTNTPadBase:setGhostValue(alphaOn, alphaOff)
    self.alphaOn = alphaOn
    self.alphaOff = alphaOff
end

---------------------------------
-- get component color r,g,b,a --
---------------------------------
function CTNTPadBase:getColor()
    return self.r, self.g, self.b, self.a
end

-------------------------
-- get component width --
-------------------------
function CTNTPadBase:getWidthA()
    return self.widthA
end

--------------------------
-- get component height --
--------------------------
function CTNTPadBase:getHeightA()
    return self.heightA
end

-------------------------
-- get component width --
-------------------------
function CTNTPadBase:getWidthB()
    return self.widthB
end

--------------------------
-- get component height --
--------------------------
function CTNTPadBase:getHeightB()
    return self.heightB
end

------------------------------
-- get component half width --
------------------------------
function CTNTPadBase:getHalfWidthA()
    return self.hWidthA
end

-------------------------------
-- get component half height --
-------------------------------
function CTNTPadBase:getHalfHeightA()
    return self.hHeightA
end

------------------------------
-- get component half width --
------------------------------
function CTNTPadBase:getHalfWidthB()
    return self.hWidthB
end

-------------------------------
-- get component half height --
-------------------------------
function CTNTPadBase:getHalfHeightB()
    return self.hHeightB
end


-------------------
-- get gfx scale --
-------------------
function CTNTPadBase:getScale()
    return self.scale
end

-------------------
-- set gfx scale --
-------------------
function CTNTPadBase:setScale(scale)
    self.scale = scale
    self.spriteA:setScale(scale, scale)
    self.spriteB:setScale(scale, scale)
    self:updateProperties()
end

-----------------------------------
-- replace texture for component --
-----------------------------------
function CTNTPadBase:setTextures(spriteA, spriteB)
    if spriteB ~= nil then
        if spriteB ~= "" then
            if self.parent:contains(self.spriteB) then
                self.parent:removeChild(self.spriteB)
            end
            self.spriteB = Bitmap.new(self.skin:getTextureRegion(spriteB))
            self.spriteB:setAnchorPoint(.5, .5)
            self.spriteB:setColorTransform(self.r, self.g, self.b, self.a)
            self.spriteB:setPosition(self.xPos, self.yPos)
            self.spriteB:setScale(self.scale, self.scale)
            self.spriteB:setRotation(self.rotation)
            self.parent:addChild(self.spriteB)
        end
    end
    if spriteA ~= nil then
        if spriteA ~= "" then
            if self.parent:contains(self.spriteA) then
                self.parent:removeChild(self.spriteA)
            end
            self.spriteA = Bitmap.new(self.skin:getTextureRegion(spriteA))
            self.spriteA:setAnchorPoint(.5, .5)
            self.spriteA:setColorTransform(self.r, self.g, self.b, self.a)
            self.spriteA:setPosition(self.xPos, self.yPos)
            self.spriteA:setScale(self.scale, self.scale)
            self.spriteA:setRotation(self.rotation)
            self.parent:addChild(self.spriteA)
        end
    end
	-- SinisterSoft fix for wrong image being displayed
	self:pushButton(false)
	-- end of fix
    self:updateProperties()
end

-- ################################################################################################################### --
-- ################################################################################################################### --
-- ################################################################################################################### --

-- **************************** --
-- **************************** --
-- *** VIRTUAL BUTTON CLASS *** --
-- **************************** --
-- **************************** --

-- ################################################################################################################### --
-- ################################################################################################################### --
-- ################################################################################################################### --

local CTNTVirtualButton = Core.class(CTNTPadBase)

-----------------------
-- init button class --
-----------------------
function CTNTVirtualButton:init(parent, skin, layer, flipped)
    if flipped == -1 then
        self.rotation = 180
    else
        self.rotation = 0
    end
    self.touchId = -1
    self.buttonPressed = false -- button is pressed ? (for button class only (not derived from base class))
    self.parent = parent
    self.skin = skin
    self.layer = layer
    self.spriteA = Bitmap.new(self.skin:getTextureRegion(PAD.SPRITE_BUTTONUP))
    self.spriteA:setAnchorPoint(.5, .5)
    self.spriteB = Bitmap.new(self.skin:getTextureRegion(PAD.SPRITE_BUTTONDOWN))
    self.spriteB:setAnchorPoint(.5, .5)
    self.spriteA:setRotation(self.rotation)
    self.spriteB:setRotation(self.rotation)
    self:updateProperties()
    self.parent:addChildAt(self.spriteA, self.layer)
end

------------------------------------
-- push/pop button sprite changer --
------------------------------------
function CTNTVirtualButton:pushButton(isPressed)
    if isPressed then
        -- show button pressed
        self.parent:removeChild(self.spriteA)
        self.parent:addChildAt(self.spriteB, self.layer)
    else
        -- show button up
        self.parent:removeChild(self.spriteB)
        self.parent:addChildAt(self.spriteA, self.layer)
    end
    -- update button state
    self.buttonPressed = isPressed
end

-- ################################################################################################################### --
-- ################################################################################################################### --
-- ################################################################################################################### --

-- ****************************** --
-- ****************************** --
-- *** VIRTUAL Joysitck CLASS *** --
-- ****************************** --
-- ****************************** --

-- ################################################################################################################### --
-- ################################################################################################################### --
-- ################################################################################################################### --

local CTNTJoystick = Core.class(CTNTPadBase)

function CTNTJoystick:init(parent, skin, layer, flipped)
    if flipped == -1 then
        self.rotation = 3.14159265358979324
    else
        self.rotation = 0
    end
    self.parent = parent
    self.skin = skin
    self.maxRadius = -1 -- max radius for touch...
    self.maxPower = 0 -- pad max power
    self.spriteA = Bitmap.new(self.skin:getTextureRegion(PAD.SPRITE_JOYUP))
    self.spriteA:setAnchorPoint(.5, .5)
    self.spriteB = Bitmap.new(self.skin:getTextureRegion(PAD.SPRITE_JOYDOWN))
    self.spriteB:setAnchorPoint(.5, .5)
    self.spriteA:setRotation(self.rotation)
    self.spriteB:setRotation(self.rotation)
    self.padStyle = PAD.STYLE_CLASSIC -- pad follow style
    self.isAnalogic = true
    self:updateProperties()
    self.parent:addChildAt(self.spriteB, layer) -- joystick back
    self.parent:addChildAt(self.spriteA, layer + 1) -- joystick front
end

function CTNTJoystick:moveUpperStick(x, y)
    self.xPos = x
    self.yPos = y
    self.spriteA:setPosition(x, y)
end

function CTNTJoystick:moveBottomStick(x, y)
    self.xPos = x
    self.yPos = y
    self.spriteB:setPosition(x, y)
end

-- ################################################################################################################### --
-- ################################################################################################################### --
-- ################################################################################################################### --

-- ***************************** --
-- ***************************** --
-- *** TNT Virutal Pad Class *** --
-- ***************************** --
-- ***************************** --

-- ################################################################################################################### --
-- ################################################################################################################### --
-- ################################################################################################################### --

CTNTVirtualPad = Core.class(EventDispatcher)

------------------------
-- update device info --
------------------------
function CTNTVirtualPad:updateDeviceInfo()
    self.screenWidth = application:getDeviceWidth()
    self.screenHeight = application:getDeviceHeight()
    self.screenLogicWidth = application:getLogicalWidth()
    self.screenLogicHeight = application:getLogicalHeight()
    self.screenContentWidth = application:getContentWidth()
    self.screenContentHeight = application:getContentHeight()
    self.halfScreenWidth = self.screenContentWidth / 2
    self.halfScreenHeight = self.screenContentHeight / 2

    -- SinisterSoft fix (pt1/2) for clipped drawing problem
    self.clippedX1 = -application:getLogicalTranslateX() / application:getLogicalScaleX()
    self.clippedY1 = -application:getLogicalTranslateY() / application:getLogicalScaleY()
    local orientation = application:getOrientation()
    if orientation == Application.PORTRAIT or orientation == Application.PORTRAIT_UPSIDE_DOWN then
        self.clippedX2 = self.clippedX1 + (application:getDeviceWidth() / application:getLogicalScaleX())
        self.clippedY2 = self.clippedY1 + (application:getDeviceHeight() / application:getLogicalScaleY())
    else
        self.clippedX2 = self.clippedX1 + (application:getDeviceHeight() / application:getLogicalScaleX())
        self.clippedY2 = self.clippedY1 + (application:getDeviceWidth() / application:getLogicalScaleY())
    end
    if self.flipped == -1 then
        self.clippedY1, self.clippedY2 = self.clippedY2, self.clippedY1
        self.clippedX1, self.clippedX2 = self.clippedX2, self.clippedX1
    end
end

------------------------
-- show or hide stick --
------------------------
function CTNTVirtualPad:showVirtualPad(visible)
    if self.hideTimer ~= nil then
        self.hideTimer:reset()
        if self.leftPad ~= nil then
            self.leftPad:setVisible(visible, self.hideMode)
        end
        if self.rightPad ~= nil then
            self.rightPad:setVisible(visible, self.hideMode)
        end
        if self.buttonsCount > 0 then
            for k = 1, self.buttonsCount do
                if self.buttons[k] ~= nil then
                    self.buttons[k]:setVisible(visible, self.hideMode)
                end
            end
        end
    end
end

----------------------
-- set use of iCade --
----------------------
--[[
function CTNTVirtualPad:setICade(onOff)
	self.useIcade = onOff
end

-------------------
-- get ICade use --
-------------------
function CTNTVirtualPad:getICade()
	return self.useIcade
end
--]]
----------------------
-- init virtual pad --
----------------------
function CTNTVirtualPad:init(parent, texturePack, padSticks, padButtons, borderSpace, layerIndex, vBorderSpace, restrict)
    self.buttons = {} -- array of buttons defined
    self.parent = parent -- parent sprite
    self.skin = nil -- pad (texture) skin pointer
    self.skin=texturePack
    self.borderSpace = borderSpace -- pad border space
    self.flipped = 1
    if vBorderSpace then
        self.vBorderSpace = vBorderSpace
        if self.vBorderSpace < 0 then
            self.flipped = -1
        end
    else
        self.vBorderSpace = borderSpace
    end
    self.buttonsCount = 0 -- how may button have our pad?
    self.padsCount = 0 -- how may pads have out pad?
    self.hideTimer = nil -- hide timer function pointer
    self.hideDelay = 600 -- hide timer delay
    self.hideMode = PAD.MODE_GHOST -- pad hiding mode
    self.useIcade = false
    self.scale = 1
    if layerIndex == nil or layerIndex < 1 then
        self.layer = 1
    else
        self.layer = layerIndex
    end
    self.leftPad = nil -- (default left) pad is there ?
    self.rightPad = nil -- (default right) pad is there ?
    -- ================ --
    if self.parent == nil then
        error(PAD.ERRORHEADER .. ": bad argument #1 (parent) to 'CTNTVirtualPad new()' parent is NIL.", 3)
    end
    -- ================ --

    if (type(padSticks) ~= "number") or (padSticks < PAD.STICK_NONE) or (padSticks > PAD.STICK_DOUBLE) then
        error(PAD.ERRORHEADER .. ": bad argument #3 (padSticks) to 'CTNTVirtualPad New()' \nValid arguments are:  \n'PAD.STICK_NONE'\n'PAD.STICK_SINGLE'\n'PAD.STICK_DOUBLE'", 3)
    end
    if (type(padButtons) ~= "number") or (padButtons < PAD.BUTTONS_NONE) or (padButtons > PAD.BUTTONS_SIX) then
        error(PAD.ERRORHEADER .. ": bad argument #4 (padButtons) to 'CTNTVirtualPad New()' \nValid arguments are:  \n'PAD.BUTTONS_NONE'\n'PAD.BUTTONS_ONE'\n'PAD.BUTTONS_TWO'\n'PAD.BUTTONS_THREE'\n'PAD.BUTTONS_FOUR'\n'PAD.BUTTONS_FIVE'\n'PAD.BUTTONS_SIX'", 3)
    end
    -- resources are ok. so continue --
    self:updateDeviceInfo()

    if restrict == nil then
        restrict = self.screenContentHeight
    end
    if restrict <= 1 then
        self.screenRestrict = self.halfScreenHeight
    else
        if self.flipped == 1 then
            self.screenRestrict = self.screenContentHeight - restrict
        else
            self.screenRestrict = restrict
        end
    end

    if (self.hideMode ~= PAD.MODE_NOHIDE) then
        self.hideTimer = Timer.new(self.hideDelay, 1)
    end
    -- ======= --
    -- JOY PAD --
    -- ======= --
    -- no joystick !

    if padSticks == PAD.STICK_NONE then
        self.padsCount = 0
        -- one joystick !
    elseif (padSticks == PAD.STICK_SINGLE) or (padSticks == PAD.STICK_DOUBLE) then
        self.padsCount = 1
        self.leftPad = CTNTJoystick.new(self.parent, self.skin, self.layer, self.flipped)

        self.leftPad:setPosition(self.clippedX1 + self.borderSpace * self.flipped + self.leftPad.hWidthB * self.flipped, self.clippedY2 - self.vBorderSpace - (self.leftPad.hWidthB * self.flipped))
        self.leftPad.maxPower = self.leftPad.hWidthB
    end
    -- two joystick !
    if padSticks == PAD.STICK_DOUBLE then
        self.padsCount = 2
        self.rightPad = CTNTJoystick.new(self.parent, self.skin, self.layer, self.flipped)
        self.rightPad:setPosition(self.clippedX2 - self.borderSpace * self.flipped - self.rightPad.hWidthB * self.flipped, self.clippedY2 - self.vBorderSpace - (self.rightPad.hWidthB * self.flipped))
        self.rightPad.maxPower = self.rightPad.hWidthB
        --	padButtons = PAD.BUTTONS_NONE
    end
    -- ======= --
    -- BUTTONS --
    -- ======= --

    -- SinisterSoft fix (pt2/2) for clipped drawing problem
    if padButtons == PAD.BUTTONS_NONE then -- NO BUTTONS
        self.buttonsCount = 0
    elseif padButtons == PAD.BUTTONS_ONE then -- ONE BUTTON
        self.buttons[1] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped)
        self.buttons[1]:setPosition(self.clippedX2 - self.buttons[1].hWidthB * self.flipped - self.borderSpace * self.flipped, self.clippedY2 - self.buttons[1].hHeightB * self.flipped - self.vBorderSpace)
        self.buttons[1]:setColor(255, 70, 60)
        self.buttonsCount = 1
    elseif padButtons == PAD.BUTTONS_TWO then -- TWO BUTTON
        self.buttons[1] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped)
        self.buttons[1]:setPosition(self.clippedX2 - self.buttons[1].hWidthB * 3 * self.flipped - self.borderSpace * self.flipped, self.clippedY2 - self.buttons[1].hHeightB * self.flipped - self.vBorderSpace)
        self.buttons[1]:setColor(255, 70, 60)
        self.buttons[2] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped)
        self.buttons[2]:setPosition(self.clippedX2 - self.buttons[2].hWidthB * self.flipped - self.borderSpace * self.flipped, self.clippedY2 - self.buttons[2].hHeightB * 2 * self.flipped - self.vBorderSpace)
        self.buttons[2]:setColor(70, 255, 120)
        self.buttonsCount = 2
    elseif padButtons == PAD.BUTTONS_THREE then -- THREE BUTTONS
        self.buttons[1] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped)
        self.buttons[1]:setPosition(self.clippedX2 - self.buttons[1].hWidthB * 3 * self.flipped - self.borderSpace * self.flipped, self.clippedY2 - self.buttons[1].hHeightB * self.flipped - self.vBorderSpace)
        self.buttons[1]:setColor(255, 70, 60)
        self.buttons[2] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped)
        self.buttons[2]:setPosition(self.clippedX2 - self.buttons[2].hWidthB * self.flipped - self.borderSpace * self.flipped, self.clippedY2 - self.buttons[2].hHeightB * 2 * self.flipped - self.vBorderSpace)
        self.buttons[2]:setColor(70, 255, 120)
        self.buttons[3] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped)
        self.buttons[3]:setPosition(self.clippedX2 - self.buttons[3].hWidthB * 2.8 * self.flipped - self.borderSpace * self.flipped, self.clippedY2 - self.buttons[3].hHeightB * 3.2 * self.flipped - self.vBorderSpace)
        self.buttons[3]:setColor(90, 120, 255)
        self.buttonsCount = 3
    elseif padButtons == PAD.BUTTONS_FOUR then -- FOUR BUTTONS
        self.buttons[1] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped)
        self.buttons[1]:setPosition(self.clippedX2 - self.buttons[1].hWidthB * 3 * self.flipped - self.borderSpace * self.flipped, self.clippedY2 - self.buttons[1].hHeightB * self.flipped - self.vBorderSpace)
        self.buttons[1]:setColor(255, 70, 60)
        self.buttons[2] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped)
        self.buttons[2]:setPosition(self.clippedX2 - self.buttons[2].hWidthB * self.flipped - self.borderSpace * self.flipped, self.clippedY2 - self.buttons[2].hHeightB * 2 * self.flipped - self.vBorderSpace)
        self.buttons[2]:setColor(70, 255, 120)
        self.buttons[3] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped)
        self.buttons[3]:setPosition(self.clippedX2 - self.buttons[3].hWidthB * 3 * self.flipped - self.borderSpace * self.flipped, self.clippedY2 - self.buttons[3].hHeightB * 3.2 * self.flipped - self.vBorderSpace)
        self.buttons[3]:setColor(90, 120, 255)
        self.buttons[4] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped)
        if application:getOrientation() == "landscape" then
            self.buttons[4]:setPosition(self.clippedX2 - self.buttons[4].hWidthB * 5 * self.flipped - self.borderSpace * self.flipped, self.clippedY2 - self.buttons[4].hHeightB * 2 * self.flipped - self.vBorderSpace)
        else
            self.buttons[4]:setPosition(self.clippedX2 - self.buttons[4].hWidthB * self.flipped - self.borderSpace * self.flipped, self.clippedY2 - self.buttons[4].hHeightB * 4.2 * self.flipped - self.vBorderSpace)
        end
        self.buttons[4]:setColor(210, 210, 60)
        self.buttonsCount = 4
    elseif padButtons == PAD.BUTTONS_FIVE then -- FIVE BUTTONS
        self.buttons[1] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped) -- RED
        self.buttons[1]:setPosition(self.clippedX2 - self.buttons[1].hWidthB * 3 * self.flipped - self.borderSpace * self.flipped, self.clippedY2 - self.buttons[1].hHeightB * self.flipped - self.vBorderSpace)
        self.buttons[1]:setColor(255, 70, 60)
        self.buttons[2] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped) -- GREEN
        self.buttons[2]:setPosition(self.clippedX2 - self.buttons[2].hWidthB * self.flipped - self.borderSpace * self.flipped, self.clippedY2 - self.buttons[2].hHeightB * 2 * self.flipped - self.vBorderSpace)
        self.buttons[2]:setColor(70, 255, 120)
        self.buttons[3] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped) -- BLUE
        self.buttons[3]:setPosition(self.clippedX2 - self.buttons[3].hWidthB * 3 * self.flipped - self.borderSpace * self.flipped, self.clippedY2 - self.buttons[3].hHeightB * 3.2 * self.flipped - self.vBorderSpace)
        self.buttons[3]:setColor(90, 120, 255)

        self.buttons[4] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped) -- GREEN
        self.buttons[4]:setPosition(self.clippedX1 + self.buttons[4].hWidthB * self.flipped + self.borderSpace * self.flipped, self.clippedY2 - self.buttons[4].hHeightB * 2 * self.flipped - self.vBorderSpace)
        self.buttons[4]:setColor(70, 255, 120)
        self.buttons[5] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped) -- RED
        self.buttons[5]:setPosition(self.clippedX1 + self.buttons[5].hWidthB * 3 * self.flipped + self.borderSpace * self.flipped, self.clippedY2 - self.buttons[5].hHeightB * self.flipped - self.vBorderSpace)
        self.buttons[5]:setColor(255, 70, 60)

        self.buttonsCount = 5
    elseif padButtons == PAD.BUTTONS_SIX then -- SIX BUTTONS
        self.buttons[1] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped) -- RED
        self.buttons[1]:setPosition(self.clippedX2 - self.buttons[1].hWidthB * 3 * self.flipped - self.borderSpace * self.flipped, self.clippedY2 - self.buttons[1].hHeightB * self.flipped - self.vBorderSpace)
        self.buttons[1]:setColor(255, 70, 60)
        self.buttons[2] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped) -- GREEN
        self.buttons[2]:setPosition(self.clippedX2 - self.buttons[2].hWidthB * self.flipped - self.borderSpace * self.flipped, self.clippedY2 - self.buttons[2].hHeightB * 2 * self.flipped - self.vBorderSpace)
        self.buttons[2]:setColor(70, 255, 120)
        self.buttons[3] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped) -- BLUE
        self.buttons[3]:setPosition(self.clippedX2 - self.buttons[3].hWidthB * 3 * self.flipped - self.borderSpace * self.flipped, self.clippedY2 - self.buttons[3].hHeightB * 3.2 * self.flipped - self.vBorderSpace)
        self.buttons[3]:setColor(90, 120, 255)

        self.buttons[4] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped) -- GREEN
        self.buttons[4]:setPosition(self.clippedX1 + self.buttons[4].hWidthB * self.flipped + self.borderSpace * self.flipped, self.clippedY2 - self.buttons[4].hHeightB * 2 * self.flipped - self.vBorderSpace)
        self.buttons[4]:setColor(70, 255, 120)
        self.buttons[5] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped) -- RED
        self.buttons[5]:setPosition(self.clippedX1 + self.buttons[5].hWidthB * 3 * self.flipped + self.borderSpace * self.flipped, self.clippedY2 - self.buttons[5].hHeightB * self.flipped - self.vBorderSpace)
        self.buttons[5]:setColor(255, 70, 60)
        self.buttons[6] = CTNTVirtualButton.new(self.parent, self.skin, self.layer, self.flipped) -- BLUE
        self.buttons[6]:setPosition(self.clippedX1 + self.buttons[6].hWidthB * 3 * self.flipped + self.borderSpace * self.flipped, self.clippedY2 - self.buttons[6].hHeightB * 3.2 * self.flipped - self.vBorderSpace)
        self.buttons[6]:setColor(90, 120, 255)
        self.buttonsCount = 6
    end

    -- ================ --
    self.button1_Event = Event.new(PAD.BUTTON1_EVENT)
    self.button2_Event = Event.new(PAD.BUTTON2_EVENT)
    self.button3_Event = Event.new(PAD.BUTTON3_EVENT)
    self.button4_Event = Event.new(PAD.BUTTON4_EVENT)
    self.button5_Event = Event.new(PAD.BUTTON5_EVENT)
    self.button6_Event = Event.new(PAD.BUTTON6_EVENT)
    self.leftPad_Event = Event.new(PAD.LEFTPAD_EVENT)
    self.rightPad_Event = Event.new(PAD.RIGHTPAD_EVENT)

    self.button1_Event.data = { selected = false, state = PAD.STATE_NONE }
    self.button2_Event.data = { selected = false, state = PAD.STATE_NONE }
    self.button3_Event.data = { selected = false, state = PAD.STATE_NONE }
    self.button4_Event.data = { selected = false, state = PAD.STATE_NONE }
    self.button5_Event.data = { selected = false, state = PAD.STATE_NONE }
    self.button6_Event.data = { selected = false, state = PAD.STATE_NONE }
    self.leftPad_Event.data = { selected = false, power = 0, angle = 0, state = PAD.STATE_NONE }
    self.rightPad_Event.data = { selected = false, power = 0, angle = 0, state = PAD.STATE_NONE }
    self:showVirtualPad(false)
end

--------------------------------
-- set analog joy mode on/off --
--------------------------------
function CTNTVirtualPad:setJoyAsAnalog(padComponent, isAnalogic)
    if padComponent == PAD.COMPO_LEFTPAD then
        if self.leftPad ~= nil then
            self.leftPad.isAnalogic = isAnalogic
        end
    elseif padComponent == PAD.COMPO_RIGHTPAD then
        if self.rightPad ~= nil then
            self.rightPad.isAnalogic = isAnalogic
        end
    else
        error(PAD.ERRORHEADER .. ": bad argument #1 (padComponent) to 'setAnalogMode(padComponent, isAnalogic)'  \nValid arguments are:  \nPAD.COMPO_LEFTPAD\nPAD.COMPO_RIGHTPAD.", 2)
    end
end

----------------------------------------
-- clear all pad buttons status flags --
----------------------------------------
function CTNTVirtualPad:clearPadButtonStatus()
    self.button1_Event.data = { selected = false, state = PAD.STATE_NONE }
    self.button2_Event.data = { selected = false, state = PAD.STATE_NONE }
    self.button3_Event.data = { selected = false, state = PAD.STATE_NONE }
    self.button4_Event.data = { selected = false, state = PAD.STATE_NONE }
    self.button5_Event.data = { selected = false, state = PAD.STATE_NONE }
    self.button6_Event.data = { selected = false, state = PAD.STATE_NONE }
end

-----------------------------
-- set pad component color --
-----------------------------
function CTNTVirtualPad:setColor(padComponent, r, g, b)
    if padComponent == PAD.COMPO_LEFTPAD then
        if self.leftPad ~= nil then
            self.leftPad:setColor(r, g, b)
        end
    elseif padComponent == PAD.COMPO_RIGHTPAD then
        if self.rightPad ~= nil then
            self.rightPad:setColor(r, g, b)
        end
    elseif padComponent == PAD.COMPO_BUTTON1 then
        if self.buttons[1] ~= nil then
            self.buttons[1]:setColor(r, g, b)
        end
    elseif padComponent == PAD.COMPO_BUTTON2 then
        if self.buttons[2] ~= nil then
            self.buttons[2]:setColor(r, g, b)
        end
    elseif padComponent == PAD.COMPO_BUTTON3 then
        if self.buttons[3] ~= nil then
            self.buttons[3]:setColor(r, g, b)
        end
    elseif padComponent == PAD.COMPO_BUTTON4 then
        if self.buttons[4] ~= nil then
            self.buttons[4]:setColor(r, g, b)
        end
    elseif padComponent == PAD.COMPO_BUTTON5 then
        if self.buttons[5] ~= nil then
            self.buttons[5]:setColor(r, g, b)
        end
    elseif padComponent == PAD.COMPO_BUTTON6 then
        if self.buttons[6] ~= nil then
            self.buttons[6]:setColor(r, g, b)
        end
    else
        error(PAD.ERRORHEADER .. ": bad argument #1 (padComponent) to 'setColor(padComponent, r, g, b)'  \nValid arguments are:  \nPAD.COMPO_LEFTPAD\nPAD.COMPO_RIGHTPAD\nPAD.COMPO_BUTTON1\nPAD.COMPO_BUTTON2\nPAD.COMPO_BUTTON3\nPAD.COMPO_BUTTON4\nPAD.COMPO_BUTTON5\nPAD.COMPO_BUTTON6.", 2)
    end
end

------------------------------
-- set Pad components Scale --
------------------------------
function CTNTVirtualPad:setScale(padComponent, scale)
    if padComponent == PAD.COMPO_LEFTPAD then
        if self.leftPad ~= nil then
            self.leftPad:setScale(scale)
            if self.leftPad.maxRadius > 1 then
                self.leftPad.maxRadius = self.leftPad.maxRadius * scale
            end
        end
    elseif padComponent == PAD.COMPO_RIGHTPAD then
        if self.rightPad ~= nil then
            self.rightPad:setScale(scale)
            if self.rightPad.maxRadius > 1 then
                self.rightPad.maxRadius = self.rightPad.maxRadius * scale
            end
        end
    elseif padComponent == PAD.COMPO_BUTTON1 then
        if self.buttons[1] ~= nil then
            self.buttons[1]:setScale(scale)
        end
    elseif padComponent == PAD.COMPO_BUTTON2 then
        if self.buttons[2] ~= nil then
            self.buttons[2]:setScale(scale)
        end
    elseif padComponent == PAD.COMPO_BUTTON3 then
        if self.buttons[3] ~= nil then
            self.buttons[3]:setScale(scale)
        end
    elseif padComponent == PAD.COMPO_BUTTON4 then
        if self.buttons[4] ~= nil then
            self.buttons[4]:setScale(scale)
        end
    elseif padComponent == PAD.COMPO_BUTTON5 then
        if self.buttons[5] ~= nil then
            self.buttons[5]:setScale(scale)
        end
    elseif padComponent == PAD.COMPO_BUTTON6 then
        if self.buttons[6] ~= nil then
            self.buttons[6]:setScale(scale)
        end
    else
        error(PAD.ERRORHEADER .. ": bad argument #1 (padComponent) to 'setScale(padComponent, scale)'  \nValid arguments are:  \nPAD.COMPO_LEFTPAD\nPAD.COMPO_RIGHTPAD\nPAD.COMPO_BUTTON1\nPAD.COMPO_BUTTON2\nPAD.COMPO_BUTTON3\nPAD.COMPO_BUTTON4\nPAD.COMPO_BUTTON5\nPAD.COMPO_BUTTON6.", 2)
    end
end

---------------------------------
-- set pad components position --
---------------------------------
function CTNTVirtualPad:setPosition(padComponent, x, y)
    if padComponent == PAD.COMPO_LEFTPAD then
        if self.leftPad ~= nil then
            self.leftPad:setPosition(x, y)
        end
    elseif padComponent == PAD.COMPO_RIGHTPAD then
        if self.rightPad ~= nil then
            self.rightPad:setPosition(x, y)
        end
    elseif padComponent == PAD.COMPO_BUTTON1 then
        if self.buttons[1] ~= nil then
            self.buttons[1]:setPosition(x, y)
        end
    elseif padComponent == PAD.COMPO_BUTTON2 then
        if self.buttons[2] ~= nil then
            self.buttons[2]:setPosition(x, y)
        end
    elseif padComponent == PAD.COMPO_BUTTON3 then
        if self.buttons[3] ~= nil then
            self.buttons[3]:setPosition(x, y)
        end
    elseif padComponent == PAD.COMPO_BUTTON4 then
        if self.buttons[4] ~= nil then
            self.buttons[4]:setPosition(x, y)
        end
    elseif padComponent == PAD.COMPO_BUTTON5 then
        if self.buttons[5] ~= nil then
            self.buttons[5]:setPosition(x, y)
        end
    elseif padComponent == PAD.COMPO_BUTTON6 then
        if self.buttons[6] ~= nil then
            self.buttons[6]:setPosition(x, y)
        end
    else
        error(PAD.ERRORHEADER .. ": bad argument #1 (padComponent) to 'setPosition(padComponent, x, y)'  \nValid arguments are:  \nPAD.COMPO_LEFTPAD\nPAD.COMPO_RIGHTPAD\nPAD.COMPO_BUTTON1\nPAD.COMPO_BUTTON2\nPAD.COMPO_BUTTON3\nPAD.COMPO_BUTTON4\nPAD.COMPO_BUTTON5\nPAD.COMPO_BUTTON6.", 2)
    end
end

---------------------------------------
-- set delay for hide (in millisecs) --
---------------------------------------
function CTNTVirtualPad:setHideDelay(msDelay)
    self.hideDelay = msDelay
    self.hideTimer:setDelay(self.hideDelay)
end

---------------------------------------------------------------------
-- set max touch radius < 1 disable (use half screen touch system) --
---------------------------------------------------------------------
function CTNTVirtualPad:setMaxRadius(padComponent, maxRadius)
    if padComponent == PAD.COMPO_LEFTPAD then
        if self.leftPad ~= nil then
            self.leftPad.maxRadius = maxRadius * self.leftPad:getScale()
        end
    elseif padComponent == PAD.COMPO_RIGHTPAD then
        if self.rightPad ~= nil then
            self.rightPad.maxRadius = maxRadius * self.rightPad:getScale()
        end
    else
        error(PAD.ERRORHEADER .. ": bad argument #1 (padComponent) to 'setMaxRadius(padComponent, maxRadius)'  \nValid arguments are:  \nPAD.COMPO_LEFTPAD\nPAD.COMPO_RIGHTPAD", 2)
    end
end

-------------------------------------
-- set pad components user texture --
-------------------------------------
function CTNTVirtualPad:setTextures(padComponent, textureA, textureB)
    if padComponent == PAD.COMPO_LEFTPAD then
        if self.leftPad ~= nil then
            self.leftPad:setTextures(textureA, textureB)
        end
    elseif padComponent == PAD.COMPO_RIGHTPAD then
        if self.rightPad ~= nil then
            self.rightPad:setTextures(textureA, textureB)
        end
    elseif padComponent == PAD.COMPO_BUTTON1 then
        if self.buttons[1] ~= nil then
            self.buttons[1]:setTextures(textureA, textureB)
        end
    elseif padComponent == PAD.COMPO_BUTTON2 then
        if self.buttons[2] ~= nil then
            self.buttons[2]:setTextures(textureA, textureB)
        end
    elseif padComponent == PAD.COMPO_BUTTON3 then
        if self.buttons[3] ~= nil then
            self.buttons[3]:setTextures(textureA, textureB)
        end
    elseif padComponent == PAD.COMPO_BUTTON4 then
        if self.buttons[4] ~= nil then
            self.buttons[4]:setTextures(textureA, textureB)
        end
    elseif padComponent == PAD.COMPO_BUTTON5 then
        if self.buttons[5] ~= nil then
            self.buttons[5]:setTextures(textureA, textureB)
        end
    elseif padComponent == PAD.COMPO_BUTTON6 then
        if self.buttons[6] ~= nil then
            self.buttons[6]:setTextures(textureA, textureB)
        end
    else
        error(PAD.ERRORHEADER .. ": bad argument #1 (padComponent) to 'setTextures(padComponent, [textureA], [textureB])'  \nValid arguments are:  \nPAD.COMPO_LEFTPAD\nPAD.COMPO_RIGHTPAD\nPAD.COMPO_BUTTON1\nPAD.COMPO_BUTTON2\nPAD.COMPO_BUTTON3\nPAD.COMPO_BUTTON4\nPAD.COMPO_BUTTON5\nPAD.COMPO_BUTTON6.", 2)
    end
end

-----------------------------------------
-- set ghost mode alpha channel values --
-----------------------------------------
function CTNTVirtualPad:setAlpha(alphaOn, alphaOff)
    if self.leftPad ~= nil then
        self.leftPad:setGhostValue(alphaOn, alphaOff)
    end
    if self.rightPad ~= nil then
        self.rightPad:setGhostValue(alphaOn, alphaOff)
    end
    for k = 1, 6 do
        if self.buttons[k] ~= nil then
            self.buttons[k]:setGhostValue(alphaOn, alphaOff)
        end
    end
end

------------------
-- set pad mode --
------------------
function CTNTVirtualPad:setHideMode(hideMode)
    if (hideMode ~= PAD.MODE_NOHIDE) and (hideMode ~= PAD.MODE_GHOST) and (hideMode ~= PAD.MODE_HIDDEN) then
        error(PAD.ERRORHEADER .. " parameter #2 'hideMode' must be one of the accepted values. \n'PAD.MODE_NOHIDE'\n'PAD.MODE_GHOST'\n'PAD.MODE_HIDDEN'", 2)
    end
    self.hideMode = hideMode
end

-------------------
-- set pad style --
-------------------
function CTNTVirtualPad:setJoyStyle(padComponent, style)
    if (style ~= PAD.STYLE_CLASSIC) and (style ~= PAD.STYLE_MOVABLE) and (style ~= PAD.STYLE_FOLLOW) then
        error(PAD.ERRORHEADER .. " parameter #2 'style' must be one of the accepted values. \n'PAD.STYLE_CLASSIC'\n'PAD.STYLE_MOVABLE'\n'PAD.STYLE_FOLLOW'", 2)
    end
    if padComponent == PAD.COMPO_LEFTPAD then
        if self.leftPad ~= nil then
            self.leftPad.padStyle = style
        end
    elseif padComponent == PAD.COMPO_RIGHTPAD then
        if self.rightPad ~= nil then
            self.rightPad.padStyle = style
        end
    else
        error(PAD.ERRORHEADER .. ": bad argument #1 (padComponent) to 'setPadStyle(padComponent, style)'  \nValid arguments are:  \nPAD.COMPO_LEFTPAD\nPAD.COMPO_RIGHTPAD.", 2)
    end
end

-----------------
-- move joypad --
-----------------
function CTNTVirtualPad:moveJoy(joyPad, Event, isOnMoveEvent)
    -- calc distance and angle from pad centre to user finger...
    local dist = distance(Event.touch.x, Event.touch.y, joyPad.spriteB:getX(), joyPad.spriteB:getY())
    local angle = calcAngle(Event.touch.x, Event.touch.y, joyPad.spriteB:getX(), joyPad.spriteB:getY())
    -- get half width and height of joystick
    local pii = 3.14159265358979324
    local power = dist / (joyPad.maxPower * joyPad.scale)
    if (dist > joyPad.hWidthB) then -- move only if user finger is outside joy border
        if (joyPad.padStyle == PAD.STYLE_MOVABLE) or (joyPad.padStyle == PAD.STYLE_FOLLOW) then
            joyPad.xPos = Event.touch.x
            joyPad.yPos = Event.touch.y
            if (isOnMoveEvent) then
                if (joyPad.padStyle == PAD.STYLE_FOLLOW) then
                    joyPad:moveUpperStick(joyPad.xPos, joyPad.yPos)
                    joyPad.xPos = joyPad.xPos + qCos(angle) * joyPad.hWidthB
                    joyPad.yPos = joyPad.yPos + qSin(angle) * joyPad.hWidthB
                else
                    -- Touch BEGIN
                    dist = joyPad.hWidthB
                    joyPad:moveUpperStick(joyPad.startX - qCos(angle) * joyPad.hWidthB, joyPad.startY - qSin(angle) * joyPad.hWidthB)
                end
            end
            -- check top border
            if joyPad.yPos < joyPad.hWidthB then
                joyPad.yPos = joyPad.hWidthB
            end
            -- check bottom border
            if joyPad.yPos > (self.screenContentHeight - joyPad.hWidthB) then
                joyPad.yPos = (self.screenContentHeight - joyPad.hWidthB)
            end
            if self.flipped == 1 then
                if joyPad == self.leftPad then
                    -- check left border
                    if joyPad.xPos < joyPad.hWidthB then
                        joyPad.xPos = joyPad.hWidthB
                    end
                    -- check right border
                    if joyPad.xPos > (self.halfScreenWidth - joyPad.hWidthB) then
                        joyPad.xPos = (self.halfScreenWidth - joyPad.hWidthB)
                    end
                else -- rightpad
                    -- check right border
                    if joyPad.xPos > (self.screenContentWidth - joyPad.hWidthB) then
                        joyPad.xPos = (self.screenContentWidth - joyPad.hWidthB)
                    end
                    -- check left border
                    if joyPad.xPos < (self.halfScreenWidth + joyPad.hWidthB) then
                        joyPad.xPos = (self.halfScreenWidth + joyPad.hWidthB)
                    end
                end
            else
                if joyPad == self.rightPad then
                    -- check left border
                    if joyPad.xPos < joyPad.hWidthB then
                        joyPad.xPos = joyPad.hWidthB
                    end
                    -- check right border
                    if joyPad.xPos > (self.halfScreenWidth - joyPad.hWidthB) then
                        joyPad.xPos = (self.halfScreenWidth - joyPad.hWidthB)
                    end
                else -- leftpad if upside down
                    -- check right border
                    if joyPad.xPos > (self.screenContentWidth - joyPad.hWidthB) then
                        joyPad.xPos = (self.screenContentWidth - joyPad.hWidthB)
                    end
                    -- check left border
                    if joyPad.xPos < (self.halfScreenWidth + joyPad.hWidthB) then
                        joyPad.xPos = (self.halfScreenWidth + joyPad.hWidthB)
                    end
                end
            end
            if isOnMoveEvent then
                if joyPad.padStyle ~= PAD.STYLE_MOVABLE then
                    joyPad:moveBottomStick(joyPad.xPos, joyPad.yPos)
                end
            else
                -- touch begin
                joyPad:setPosition(joyPad.xPos, joyPad.yPos)
            end
            if not (isOnMoveEvent) then
                power = 0
            end
        else
            -- joystick is fixed "STYLE_CLASSIC...
            local x = qCos(angle) * joyPad.hWidthB
            local y = qSin(angle) * joyPad.hWidthB
            joyPad:moveUpperStick(joyPad.startX - x, joyPad.startY - y)
        end
    else
        joyPad:moveUpperStick(Event.touch.x, Event.touch.y)
    end
    Event:stopPropagation()

    local jPadData, jPad
    if joyPad == self.rightPad then
        jPadData = self.rightPad_Event.data
        jPad = self.rightPad
    else
        jPadData = self.leftPad_Event.data
        jPad = self.leftPad
    end

    jPadData.selected = true

    if power > 1 then
        power = 1
    end

    if not (isOnMoveEvent) then
        jPadData.state = PAD.STATE_BEGIN
    else
        jPadData.state = PAD.STATE_DOWN
    end

    jPadData.power = power

    if jPad.isAnalogic then
        jPadData.angle = (angle) + pii
    else
        jPadData.angle = qFloor((angle + pii + .37) / .79)
        if jPadData.angle > 7 then
            jPadData.angle = 0
        end
    end
end


--------------------------------
-- user start touching screen --
--------------------------------
function CTNTVirtualPad:onTouchesBegin(Event)

    if (self.hideMode ~= PAD.MODE_NOHIDE) then
        self:showVirtualPad(true)
    end
    local buttonHit = false
    -- ============ --
    -- CHECK BUTTON --
    -- ============ --
    if self.buttonsCount > 0 then --and ((not self.leftPad_Event.data.selected) or (not self.rightPad_Event.data.selected)) then -- there are button defined in pad ?
        for k = 1, self.buttonsCount do -- check buttons
            if (not self.buttons[k].buttonPressed) then -- button touched ?
                -- current button is not pressed ?
                if (self.buttons[k].spriteA:hitTestPoint(Event.touch.x, Event.touch.y)) then -- button touched ?
                    self.buttons[k]:pushButton(true) -- change button gfx with pushed one!
                    self.buttons[k].touchId = Event.touch.id -- assign id to button
                    Event:stopPropagation() -- stop event propagation
                    -- set button event ---
                    if k == 1 then
                        self.button1_Event.data.selected = true
                        self.button1_Event.data.state = PAD.STATE_BEGIN
                        self:dispatchEvent(self.button1_Event)
                        self:clearPadButtonStatus()
                    elseif k == 2 then
                        self.button2_Event.data.selected = true
                        self.button2_Event.data.state = PAD.STATE_BEGIN
                        self:dispatchEvent(self.button2_Event)
                        self:clearPadButtonStatus()
                    elseif k == 3 then
                        self.button3_Event.data.selected = true
                        self.button3_Event.data.state = PAD.STATE_BEGIN
                        self:dispatchEvent(self.button3_Event)
                        self:clearPadButtonStatus()
                    elseif k == 4 then
                        self.button4_Event.data.selected = true
                        self.button4_Event.data.state = PAD.STATE_BEGIN
                        self:dispatchEvent(self.button4_Event)
                        self:clearPadButtonStatus()
                    elseif k == 5 then
                        self.button5_Event.data.selected = true
                        self.button5_Event.data.state = PAD.STATE_BEGIN
                        self:dispatchEvent(self.button5_Event)
                        self:clearPadButtonStatus()
                    elseif k == 6 then
                        self.button6_Event.data.selected = true
                        self.button6_Event.data.state = PAD.STATE_BEGIN
                        self:dispatchEvent(self.button6_Event)
                        self:clearPadButtonStatus()
                    end
                    buttonHit = true
                    break -- exit form for loop ...
                end
            end
        end
    end
    -- ============= --
    -- check joypads --
    -- ============= --
    if self.flipped == 1 then
        if (self.leftPad ~= nil) and (self.leftPad.scale > 0) and (Event.touch.x < self.halfScreenWidth) and (Event.touch.y > self.screenRestrict) and (not buttonHit) then -- left pad present ?
            if self.leftPad.touchId == nil then
                if self.leftPad.maxRadius < 1 then
                    self.leftPad.touchId = Event.touch.id
                    self:moveJoy(self.leftPad, Event, false)
                elseif distance(Event.touch.x, Event.touch.y, self.leftPad.spriteB:getX(), self.leftPad.spriteB:getY()) < self.leftPad.maxRadius then
                    self.leftPad.touchId = Event.touch.id
                    self:moveJoy(self.leftPad, Event, false)
                end
            end
        end
        if (self.rightPad ~= nil) and (self.rightPad.scale > 0) and (Event.touch.x > self.halfScreenWidth) and (Event.touch.y > self.screenRestrict) and (not buttonHit) then -- left pad present ?
            if self.rightPad.touchId == nil then
                if self.rightPad.maxRadius < 1 then
                    self.rightPad.touchId = Event.touch.id
                    self:moveJoy(self.rightPad, Event, false)
                elseif distance(Event.touch.x, Event.touch.y, self.rightPad.spriteB:getX(), self.rightPad.spriteB:getY()) < self.rightPad.maxRadius then
                    self.rightPad.touchId = Event.touch.id
                    self:moveJoy(self.rightPad, Event, false)
                end
            end
        end
    else
        if (self.leftPad ~= nil) and (self.leftPad.scale > 0) and (Event.touch.x > self.halfScreenWidth) and (Event.touch.y < self.screenRestrict) and (not buttonHit) then -- left pad present ?
            if self.leftPad.touchId == nil then
                if self.leftPad.maxRadius < 1 then
                    self.leftPad.touchId = Event.touch.id
                    self:moveJoy(self.leftPad, Event, false)
                elseif distance(Event.touch.x, Event.touch.y, self.leftPad.spriteB:getX(), self.leftPad.spriteB:getY()) < self.leftPad.maxRadius then
                    self.leftPad.touchId = Event.touch.id
                    self:moveJoy(self.leftPad, Event, false)
                end
            end
        end
        if (self.rightPad ~= nil) and (self.rightPad.scale > 0) and (Event.touch.x < self.halfScreenWidth) and (Event.touch.y < self.screenRestrict) and (not buttonHit) then -- left pad present ?
            if self.rightPad.touchId == nil then
                if self.rightPad.maxRadius < 1 then
                    self.rightPad.touchId = Event.touch.id
                    self:moveJoy(self.rightPad, Event, false)
                elseif distance(Event.touch.x, Event.touch.y, self.rightPad.spriteB:getX(), self.rightPad.spriteB:getY()) < self.rightPad.maxRadius then
                    self.rightPad.touchId = Event.touch.id
                    self:moveJoy(self.rightPad, Event, false)
                end
            end
        end
    end
end

--------------------------------
-- user move finger on screen --
--------------------------------
function CTNTVirtualPad:onTouchesMove(Event)
    -- ============ --
    -- CHECK BUTTON --
    -- ============ --
    if self.buttonsCount > 0 then --and ((not self.leftPad_Event.data.selected) or (not self.rightPad_Event.data.selected)) then -- there are button defined in pad ?
        for k = 1, self.buttonsCount do -- check buttons
            if self.buttons[k].buttonPressed then -- current button is pressed ?
                -- user slide finger outside the button...
                if not (self.buttons[k].spriteB:hitTestPoint(Event.touch.x, Event.touch.y)) and (self.buttons[k].touchId == Event.touch.id) then
                    self.buttons[k]:pushButton(false)
                    self.buttons[k].touchId = -1
                    Event:stopPropagation()
                    if k == 1 then
                        self.button1_Event.data.selected = true
                        self.button1_Event.data.state = PAD.STATE_END
                        self:dispatchEvent(self.button1_Event)
                        self:clearPadButtonStatus()
                    elseif k == 2 then
                        self.button2_Event.data.selected = true
                        self.button2_Event.data.state = PAD.STATE_END
                        self:dispatchEvent(self.button2_Event)
                        self:clearPadButtonStatus()
                    elseif k == 3 then
                        self.button3_Event.data.selected = true
                        self.button3_Event.data.state = PAD.STATE_END
                        self:dispatchEvent(self.button3_Event)
                        self:clearPadButtonStatus()
                    elseif k == 4 then
                        self.button4_Event.data.selected = true
                        self.button4_Event.data.state = PAD.STATE_END
                        self:dispatchEvent(self.button4_Event)
                        self:clearPadButtonStatus()
                    elseif k == 5 then
                        self.button5_Event.data.selected = true
                        self.button5_Event.data.state = PAD.STATE_END
                        self:dispatchEvent(self.button5_Event)
                        self:clearPadButtonStatus()
                    elseif k == 6 then
                        self.button6_Event.data.selected = true
                        self.button6_Event.data.state = PAD.STATE_END
                        self:dispatchEvent(self.button6_Event)
                        self:clearPadButtonStatus()
                    end
                    break
                end
            elseif (self.buttons[k].spriteA:hitTestPoint(Event.touch.x, Event.touch.y)) and (not self.leftPad_Event.data.selected) and (not self.rightPad_Event.data.selected) then
                -- if button is not pressed and user slide finger inside button then press button
                self.buttons[k]:pushButton(true)
                self.buttons[k].touchId = Event.touch.id
                Event:stopPropagation()
                if k == 1 then
                    self.button1_Event.data.selected = true
                    self.button1_Event.data.state = PAD.STATE_BEGIN
                    self:dispatchEvent(self.button1_Event)
                    self:clearPadButtonStatus()
                elseif k == 2 then
                    self.button2_Event.data.selected = true
                    self.button2_Event.data.state = PAD.STATE_BEGIN
                    self:dispatchEvent(self.button2_Event)
                    self:clearPadButtonStatus()
                elseif k == 3 then
                    self.button3_Event.data.selected = true
                    self.button3_Event.data.state = PAD.STATE_BEGIN
                    self:dispatchEvent(self.button3_Event)
                    self:clearPadButtonStatus()
                elseif k == 4 then
                    self.button4_Event.data.selected = true
                    self.button4_Event.data.state = PAD.STATE_BEGIN
                    self:dispatchEvent(self.button4_Event)
                    self:clearPadButtonStatus()
                elseif k == 5 then
                    self.button5_Event.data.selected = true
                    self.button5_Event.data.state = PAD.STATE_BEGIN
                    self:dispatchEvent(self.button5_Event)
                    self:clearPadButtonStatus()
                elseif k == 6 then
                    self.button6_Event.data.selected = true
                    self.button6_Event.data.state = PAD.STATE_BEGIN
                    self:dispatchEvent(self.button6_Event)
                    self:clearPadButtonStatus()
                end
                break
            end
        end
    end
    -- ============= --
    -- check joypads --
    -- ============= --
    local lWidth = self.halfScreenWidth
	local rWidth = self.halfScreenWidth
	if (self.leftPad ~= nil) then
		if self.leftPad.maxRadius > 1 then 
			lWidth = self.halfScreenWidth*2
		end
	end	
    if (self.rightPad ~= nil) then
		if self.rightPad.maxRadius > 1 then 
			lWidth = self.halfScreenWidth*2
		end
	end	
	
    if self.flipped == 1 then
        if (self.leftPad ~= nil) and (self.leftPad.scale > 0) and (Event.touch.x < lWidth) and (Event.touch.y > self.screenRestrict) then -- left pad present ?
            if self.leftPad.touchId == Event.touch.id then
                self:moveJoy(self.leftPad, Event, true)
            end
        end
        if (self.rightPad ~= nil) and (self.rightPad.scale > 0) and (Event.touch.x > rWidth) and (Event.touch.y > self.screenRestrict) then -- left pad present ?
            if self.rightPad.touchId == Event.touch.id then
                self:moveJoy(self.rightPad, Event, true)
            end
        end
    else
        if (self.leftPad ~= nil) and (self.leftPad.scale > 0) and (Event.touch.x > lWidth) and (Event.touch.y < self.screenRestrict) then -- left pad present ?
            if self.leftPad.touchId == Event.touch.id then
                self:moveJoy(self.leftPad, Event, true)
            end
        end
        if (self.rightPad ~= nil) and (self.rightPad.scale > 0) and (Event.touch.x < rWidth) and (Event.touch.y < self.screenRestrict) then -- left pad present ?
            if self.rightPad.touchId == Event.touch.id then
                self:moveJoy(self.rightPad, Event, true)
            end
        end
    end
end

--------------------
-- end user touch --
--------------------
function CTNTVirtualPad:onTouchesEnd(Event)
    -----------------------------------------------
    -- check for end pad press (if pads defined) --
    -----------------------------------------------
    if self.padsCount > 0 then
        if self.leftPad.touchId == Event.touch.id then
            self.leftPad.spriteA:setPosition(self.leftPad.spriteB:getX(), self.leftPad.spriteB:getY())
            self.leftPad:setPosition(self.leftPad.spriteA:getPosition())
            self.leftPad.touchId = nil
            self.leftPad_Event.data.state = PAD.STATE_END
            self.leftPad_Event.data.selected = false
            self.leftPad_Event.data.power = 0
            --	self.leftPad_Event.data.angle = 0
            self:dispatchEvent(self.leftPad_Event)
            Event:stopPropagation()
        end
        if self.rightPad ~= nil then
            if self.rightPad.touchId == Event.touch.id then
                self.rightPad.spriteA:setPosition(self.rightPad.spriteB:getX(), self.rightPad.spriteB:getY())
                self.rightPad.touchId = nil
                self.rightPad_Event.data.state = PAD.STATE_END
                self.rightPad_Event.data.selected = false
                self.rightPad_Event.data.power = 0
                --		self.rightPad_Event.data.angle = 0
                self:dispatchEvent(self.rightPad_Event)
                Event:stopPropagation()
            end
        end
    end
    -- ============ --
    -- CHECK BUTTON --
    -- ============ --
    if self.buttonsCount > 0 then -- there are button defined in pad ?
        for k = 1, self.buttonsCount do -- check buttons
            if self.buttons[k].touchId == Event.touch.id then -- current button was pressed ?
                self.buttons[k]:pushButton(false) -- change button gfx with up button
                self.buttons[k].touchId = -1 -- reset touch id associated
                Event:stopPropagation() -- stop propagation
                -- set button event ---
                if k == 1 then
                    self.button1_Event.data.selected = true
                    self.button1_Event.data.state = PAD.STATE_END
                    self:dispatchEvent(self.button1_Event)
                    self:clearPadButtonStatus()
                elseif k == 2 then
                    self.button2_Event.data.selected = true
                    self.button2_Event.data.state = PAD.STATE_END
                    self:dispatchEvent(self.button2_Event)
                    self:clearPadButtonStatus()
                elseif k == 3 then
                    self.button3_Event.data.selected = true
                    self.button3_Event.data.state = PAD.STATE_END
                    self:dispatchEvent(self.button3_Event)
                    self:clearPadButtonStatus()
                elseif k == 4 then
                    self.button4_Event.data.selected = true
                    self.button4_Event.data.state = PAD.STATE_END
                    self:dispatchEvent(self.button4_Event)
                    self:clearPadButtonStatus()
                elseif k == 5 then
                    self.button5_Event.data.selected = true
                    self.button5_Event.data.state = PAD.STATE_END
                    self:dispatchEvent(self.button5_Event)
                    self:clearPadButtonStatus()
                elseif k == 6 then
                    self.button6_Event.data.selected = true
                    self.button6_Event.data.state = PAD.STATE_END
                    self:dispatchEvent(self.button6_Event)
                    self:clearPadButtonStatus()
                end
                break -- exit form for loop ...
            end
        end
    end
    if (self.hideTimer ~= nil) and (#Event.allTouches < 2) then
        self.hideTimer:start()
    end
end

-----------------
-- on hide pad --
-----------------
function CTNTVirtualPad:onHidePad()
    self:showVirtualPad(false)
    self.hideTimer:reset()
end

---------------------------------------
-- dispatch pad Event on EnterFrames --
---------------------------------------
function CTNTVirtualPad:onEnterFrame(Event)
    if self.leftPad ~= nil then
        if self.leftPad_Event.data.selected then
            self.leftPad_Event.data.deltaTime = Event.deltaTime
            self:dispatchEvent(self.leftPad_Event)
            self.leftPad_Event.data.buttonJoyState = PAD.STATE_NONE
        end
    end
    if self.rightPad ~= nil then
        if self.rightPad_Event.data.selected then
            self.rightPad_Event.data.deltaTime = Event.deltaTime
            self:dispatchEvent(self.rightPad_Event)
            self.rightPad_Event.data.buttonJoyState = PAD.STATE_NONE
        end
    end
    if self.buttonsCount > 0 then -- there are button defined in pad ?
        for k = 1, self.buttonsCount do -- check buttons
            if self.buttons[k].buttonPressed then -- current button is not pressed ?
                -- set button event ---
                if k == 1 then
                    self.button1_Event.data.selected = true
                    self.button1_Event.data.state = PAD.STATE_DOWN
                    self:dispatchEvent(self.button1_Event)
                    self:clearPadButtonStatus()
                elseif k == 2 then
                    self.button2_Event.data.selected = true
                    self.button2_Event.data.state = PAD.STATE_DOWN
                    self:dispatchEvent(self.button2_Event)
                    self:clearPadButtonStatus()
                elseif k == 3 then
                    self.button3_Event.data.selected = true
                    self.button3_Event.data.state = PAD.STATE_DOWN
                    self:dispatchEvent(self.button3_Event)
                    self:clearPadButtonStatus()
                elseif k == 4 then
                    self.button4_Event.data.selected = true
                    self.button4_Event.data.state = PAD.STATE_DOWN
                    self:dispatchEvent(self.button4_Event)
                    self:clearPadButtonStatus()
                elseif k == 5 then
                    self.button5_Event.data.selected = true
                    self.button5_Event.data.state = PAD.STATE_DOWN
                    self:dispatchEvent(self.button5_Event)
                    self:clearPadButtonStatus()
                elseif k == 6 then
                    self.button6_Event.data.selected = true
                    self.button6_Event.data.state = PAD.STATE_DOWN
                    self:dispatchEvent(self.button6_Event)
                    self:clearPadButtonStatus()
                end
            end
        end
    end
end

----------------------
-- stop Virutal Pad --
----------------------
function CTNTVirtualPad:stop()
    if not self.useIcade then
        stage:removeEventListener(Event.TOUCHES_BEGIN, self.onTouchesBegin, self)
        stage:removeEventListener(Event.TOUCHES_MOVE, self.onTouchesMove, self)
        stage:removeEventListener(Event.TOUCHES_END, self.onTouchesEnd, self)
        stage:removeEventListener(Event.TOUCHES_CANCEL, self.onTouchesEnd, self)
        stage:removeEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)
        if (self.hideMode ~= PAD.MODE_NOHIDE) then
            self.hideTimer:removeEventListener(Event.TIMER, self.onHidePad, self)
        end
    end
end

------------------------------------
-- reset vPad to default position --
------------------------------------
function CTNTVirtualPad:reset()
        if self.leftPad then
            self.leftPad.spriteA:setPosition(self.leftPad.spriteB:getX(), self.leftPad.spriteB:getY())
            self.leftPad:setPosition(self.leftPad.spriteA:getPosition())
            self.leftPad.touchId = nil
            self.leftPad_Event.data.state = PAD.STATE_END
            self.leftPad_Event.data.selected = false
            self.leftPad_Event.data.power = 0
        end
        if self.rightPad  then
            self.rightPad.spriteA:setPosition(self.rightPad.spriteB:getX(), self.rightPad.spriteB:getY())
            self.rightPad.touchId = nil
            self.rightPad_Event.data.state = PAD.STATE_END
            self.rightPad_Event.data.selected = false
            self.rightPad_Event.data.power = 0
        end
end

-----------------------
-- start Virutal Pad --
-----------------------
function CTNTVirtualPad:start()
    if not self.useIcade then
        self:showVirtualPad(true)
        stage:addEventListener(Event.TOUCHES_BEGIN, self.onTouchesBegin, self)
        stage:addEventListener(Event.TOUCHES_MOVE, self.onTouchesMove, self)
        stage:addEventListener(Event.TOUCHES_END, self.onTouchesEnd, self)
        stage:addEventListener(Event.TOUCHES_CANCEL, self.onTouchesEnd, self)
        stage:addEventListener(Event.ENTER_FRAME, self.onEnterFrame, self)
        if (self.hideMode ~= PAD.MODE_NOHIDE) then
            self.hideTimer:addEventListener(Event.TIMER, self.onHidePad, self)
            self.hideTimer:start()
        end
    else
        self.hideMode = PAD.MODE_HIDDEN
        self:showVirtualPad(false)
    end
end

----------------------
-- free virtual pad --
----------------------
function CTNTVirtualPad:free()
    self:stop()
    if self.leftPad ~= nil then
        self.leftPad = self.leftPad:free()
    end
    if self.rightPad ~= nil then
        self.rightPad = self.rightPad:free()
    end
    for j = 1, self.buttonsCount do
        self.buttons[j] = self.buttons[j]:free()
    end
    self.button1_Event = nil
    self.button2_Event = nil
    self.button3_Event = nil
    self.button4_Event = nil
    self.button5_Event = nil
    self.button6_Event = nil
    self.leftPad_Event = nil
    self.rightPad_Event = nil
    self.parent = nil
    self.skin = nil
    self.buttons = nil
    self.padEvent = nil
end

