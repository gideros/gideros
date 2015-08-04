--[[

This example demonstrates
	1. saving and loading the state of an application
	2. pausing the application when it goes to the background

There are 4 type of broadcast events that are dispatched to notify the state of the application:
	- Event.APPLICATION_START is dispatched right after the application is launched and all Lua codes are executed.
	- Event.APPLICATION_EXIT is dispatched when the application is about to exit. If an application is forced
		to be terminated (e.g. by double tapping the home button and kill the application), this event may 
		not be dispatched.
	- Event.APPLICATION_SUSPEND is dispatched when the application is about to move from the active to
		inactive state. When an application is inactive, Event.ENTER_FRAME and Event.TIMER events are not dispatched until the
		application is resumed.
	- Event.APPLICATION_RESUME is dispatched when the application is moved from the inactive to active state.
	- Event.APPLICATION_BACKGROUND is dispatched when the application is now in the background.
	- Event.APPLICATION_FOREGROUND is dispatched when the application is about to enter the foreground.

This code is MIT licensed, see http://www.opensource.org/licenses/mit-license.php
(C) 2010 - 2011 Gideros Mobile 

--]]

-- this two variables holds the state of the application
local frame = 0
local paused = false

-- create pause and resume buttons
local pause = Button.new(Bitmap.new(Texture.new("pause-up.png")),
						 Bitmap.new(Texture.new("pause-down.png")))

local resume = Button.new(Bitmap.new(Texture.new("resume-up.png")),
						  Bitmap.new(Texture.new("resume-down.png")))

pause:setPosition(80, 200)
resume:setPosition(70, 200)

-- create label that shows the current frame
local framelabel = TextField.new()
framelabel:setPosition(130, 190)
stage:addChild(framelabel)

-- if pause button is clicked, pause the application
pause:addEventListener("click", 
	function() 
		stage:removeChild(pause)
		stage:addChild(resume)
		paused = true
	end)

-- if resume button is clicked, resume the application
resume:addEventListener("click", 
	function()
		stage:removeChild(resume)
		stage:addChild(pause)
		paused = false
	end)

-- save the state of the applitation to "save.txt" in documents folder
local function loadState()
	local file = io.open("|D|save.txt", "rt")
	if file then
		frame = file:read("*number")
		paused = file:read("*number") ~= 0
		file:close()
	end
end

-- load the state of the applitation from "save.txt" in documents folder
local function saveState()
	local file = io.open("|D|save.txt", "wt")
	if file then
		file:write(frame, " ", paused and 1 or 0)
		file:close()
	end
end

-- this function will be triggered right after the application is launched
local function onStart(event)
	print(event:getType())
	
	-- we load the state from file
	loadState()

	-- if state is paused add resume button else add pause button to the stage
	if paused then
		stage:addChild(resume)
	else
		stage:addChild(pause)
	end

	framelabel:setText("frames: "..frame)	
end

-- when the application is about to go to background, save our current state and pause the application
local function onSuspend(event)
	print(event:getType())
	saveState()
	if paused == false then
		paused = true
		stage:removeChild(pause)
		stage:addChild(resume)
	end
end

-- when the application is resumed, do nothing (because we already pause the application)
local function onResume(event)
	print(event:getType())
end

-- when the application is about to exit, save our current state
local function onExit(event)
	print(event:getType())
	saveState()
end

local function onEnterFrame()
	if paused == false then
		frame = frame + 1
		framelabel:setText("frames: "..frame)	
	end
end

-- register events
stage:addEventListener(Event.APPLICATION_START, onStart)
stage:addEventListener(Event.APPLICATION_SUSPEND, onSuspend)
stage:addEventListener(Event.APPLICATION_RESUME, onResume)
stage:addEventListener(Event.APPLICATION_EXIT, onExit)

stage:addEventListener(Event.ENTER_FRAME, onEnterFrame)
