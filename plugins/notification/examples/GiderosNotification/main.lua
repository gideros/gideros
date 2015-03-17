require "notification"

application:setKeepAwake(true)
local text = TextField.new(nil, "Test")
text:setPosition(100,100)
text:setScale(4)
stage:addChild(text)

--recursive print of tables
function print_r (t, indent, done)
  done = done or {}
  indent = indent or ''
  local nextIndent -- Storage for next indentation value
  for key, value in pairs (t) do
    if type (value) == "table" and not done [value] then
      nextIndent = nextIndent or
          (indent .. string.rep(' ',string.len(tostring (key))+2))
          -- Shortcut conditional allocation
      done [value] = true
      print (indent .. "[" .. tostring (key) .. "] => Table {");
      print  (nextIndent .. "{");
      print_r (value, nextIndent .. string.rep(' ',2), done)
      print  (nextIndent .. "}");
    else
      print  (indent .. "[" .. tostring (key) .. "] => " .. tostring (value).."")
    end
  end
end
local mngr = NotificationManager.getSharedInstance()
--mngr:cancelAllNotifications()
--mngr:cancelNotification(1)

stage:addEventListener(Event.MOUSE_DOWN, function()
local note = Notification.new(1)
note:setTitle("Test from Lua")
print(note:getTitle())
note:setTitle("Test from Lua2")
print(note:getTitle())
note:setMessage("Do you see me?")
note:setSound("./point.wav")
note:setCustom('{"x":157,"y":100}')
note:dispatchAfter({sec = 20})
end)

--[[local note2 = Notification.new(2)
print(note2:getTitle())
note2:setTitle("Test from Lua 10")
print(note2:getTitle())
note2:setMessage("Do you see me second time?")
note2:setSound("point.wav")
note2:dispatchAfter({sec=10})]]

--[[local mngr = NotificationManager.getSharedInstance()
mngr:cancelAllNotifications()
print(mngr)
mngr = nil
collectgarbage()
collectgarbage()
collectgarbage()
collectgarbage()
collectgarbage()
collectgarbage()
collectgarbage()
collectgarbage()
collectgarbage()
print(NotificationManager.getSharedInstance())]]

local mngr = NotificationManager.getSharedInstance()
local t = mngr:getLocalNotifications()
print(t)
--print_r(t)

mngr:addEventListener(Event.LOCAL_NOTIFICATION, function(e)
	mngr:cancelNotification(e.id)
	print("Local notification: "..e.id)
	print("Title: "..e.title)
	print("Text: "..e.message)
	print("Number: "..e.number)
	print("Sound: "..e.sound)
	print("Custom: "..e.custom)
	print("Did Open app", e.didOpen)
	text:setText("Id: "..e.id)
end)

mngr:addEventListener(Event.PUSH_REGISTRATION, function(e)
	print("Push notification registered: "..e.deviceId)
end)

mngr:addEventListener(Event.PUSH_REGISTRATION_ERROR, function(e)
	print("Could not register: "..e.error)
end)

mngr:addEventListener(Event.PUSH_NOTIFICATION, function(e)
	print("Push notification: "..e.id)
	print("Title: "..e.title)
	print("Text: "..e.message)
	print("Number: "..e.number)
	print("Custom: "..e.custom)
	print("Did Open app", e.didOpen)
	text:setText("Id: "..e.id)
	local t = mngr:getPushNotifications()
	print_r(t)
end)

mngr:registerForPushNotifications("953841987672")

--print("Testing clear local")
Timer.delayedCall(30000, function()
	print("Local")
	print_r(mngr:getLocalNotifications())
	--print("Scheduled")
	--print_r(mngr:getScheduledNotifications())
end)
mngr:clearLocalNotifications()
print_r(mngr:getLocalNotifications())
--[[print("Testing clear push")
print_r(mngr:getPushNotifications())
--mngr:clearPushNotifications()
--print_r(mngr:getPushNotifications())
print("Testing scheduled notifications")
print_r(mngr:getScheduledNotifications())]]

stage:addEventListener(Event.KEY_DOWN, function(e)
	if e.keyCode == KeyCode.BACK then
		application:exit()
	end
end)