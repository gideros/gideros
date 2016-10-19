--[[

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

]]

local font = TTFont.new("Roboto-Regular.ttf", 18)

local b=Pixel.new(0xC0C0C0,1,200,30)
local bt=TextField.new(font,"Toggle Phone Keyboard")
bt:setPosition(0,20)
b:addChild(bt)
local kbdVisible=false
b:addEventListener(Event.MOUSE_DOWN,function ()
  kbdVisible=not kbdVisible
  application:setKeyboardVisibility(kbdVisible)
end)
stage:addChild(b)

local text1 = TextField.new(font, "press keys!")
text1:setPosition(10, 100)
stage:addChild(text1)

local textc = TextField.new(font, "")
textc:setPosition(10, 130)
stage:addChild(textc)

local text2 = TextField.new(font, "press back button 3 time(s) to exit")
text2:setPosition(10, 160)
stage:addChild(text2)

-- key codes are integer. we map to strings so that we can display key name easily
local keyNames = {
  [KeyCode.BACK] = "back",
  [KeyCode.SEARCH] = "search",
  [KeyCode.MENU] = "menu",
  [KeyCode.CENTER] = "center",
  [KeyCode.SELECT] = "select",
  [KeyCode.START] = "start",
  [KeyCode.L1] = "L1",
  [KeyCode.R1] = "R1",
  [KeyCode.LEFT] = "left",
  [KeyCode.UP] = "up",
  [KeyCode.RIGHT] = "right",
  [KeyCode.DOWN] = "down",
  [KeyCode.X] = "x",
  [KeyCode.Y] = "y",
  [KeyCode.SPACE] = "space",
  [KeyCode.BACKSPACE] = "backspace",
  [KeyCode.SHIFT] = "shift",
}

local function onKeyDown(event)
  text1:setText("key down: "..(keyNames[event.keyCode] or "unknown"))
end

local function onKeyChar(event)
  local extra=""
  if event.text:len() then
    extra=" (Byte:"..event.text:byte(1)..")"
  end
  textc:setText("key chars: "..event.text..extra)
end

local backCount = 0

local function onKeyUp(event)
  text1:setText("key up: "..(keyNames[event.keyCode] or "unknown"))
  
  if event.keyCode == KeyCode.BACK then
    backCount = backCount + 1   
    text2:setText("press back button "..(3 - backCount).." time(s) to exit")
    if backCount == 3 then
      application:exit()
    end
  end
end

-- key events are dispatched to all Sprite instances on the scene tree (similar to mouse and touch events)
stage:addEventListener(Event.KEY_DOWN, onKeyDown)
stage:addEventListener(Event.KEY_UP, onKeyUp)
stage:addEventListener(Event.KEY_CHAR, onKeyChar)
