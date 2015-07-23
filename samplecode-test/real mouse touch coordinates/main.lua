



--stage:addEventListener(Event.MOUSE_DOWN, function(event) print("down", event.rx, event.ry) end)
--stage:addEventListener(Event.MOUSE_MOVE, function(event) print("move", event.rx, event.ry) end)
--stage:addEventListener(Event.MOUSE_UP, function(event) print("up", event.rx, event.ry) end)


stage:addEventListener(Event.TOUCHES_BEGIN, function(event) print("begin", event.touch.rx, event.touch.ry) end)
stage:addEventListener(Event.TOUCHES_MOVE, function(event) print("move", event.touch.rx, event.touch.ry) end)
stage:addEventListener(Event.TOUCHES_END, function(event) print("end", event.touch.rx, event.touch.ry) end)
