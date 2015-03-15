--[[
		FOR DEBUGGING
]]--
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

require "controller"

function findKeyCode(code)
	for i, val in pairs(KeyCode) do
		if val == code then
			return i
		end
	end
	return "unknown"
end

controller:addEventListener(Event.KEY_DOWN, function(e)
	print("Button Down ", e.playerId, e.keyCode, findKeyCode(e.keyCode))
end)

controller:addEventListener(Event.KEY_UP, function(e)
	print("Button Up ", e.playerId, e.keyCode, findKeyCode(e.keyCode))
end)

controller:addEventListener(Event.RIGHT_JOYSTICK, function(e)
	print("Player: ", e.playerId)
	print("RIGHT_JOYSTICK:", "x:"..e.x, "y:"..e.y, "angle:"..e.angle, "strength:"..e.strength)
end)

controller:addEventListener(Event.LEFT_JOYSTICK, function(e)
	print("Player: ", e.playerId)
	print("LEFT_JOYSTICK:", "x:"..e.x, "y:"..e.y, "angle:"..e.angle, "strength:"..e.strength)
end)

controller:addEventListener(Event.RIGHT_TRIGGER, function(e)
	print("Player: ", e.playerId)
	print("RIGHT_TRIGGER:", "strength:"..e.strength)
end)

controller:addEventListener(Event.LEFT_TRIGGER, function(e)
	print("Player: ", e.playerId)
	print("LEFT_TRIGGER:", "strength:"..e.strength)
end)

controller:addEventListener(Event.CONNECTED, function(e)
	print("Player: ", e.playerId, "connected")
end)

controller:addEventListener(Event.DISCONNECTED, function(e)
	print("Player: ", e.playerId, "disconnected")
end)

print("any controllers", controller:isAnyAvailable())
print("controller count", controller:getPlayerCount())
print("controller 1", controller:getControllerName(1))
print("players", controller:getPlayers()[1])
print("controller 1 vibrate", controller:vibrate(1, 1000))
