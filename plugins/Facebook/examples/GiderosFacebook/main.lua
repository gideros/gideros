require "facebook"

facebook:addEventListener(Event.LOGIN_COMPLETE, function()
	print("login successful")
end)
facebook:addEventListener(Event.DIALOG_ERROR, function(e)
	print("dialog error", e.type, e.error)
end)
facebook:addEventListener(Event.DIALOG_COMPLETE, function(e)
	print("dialog complete", e.type)
end)
facebook:addEventListener(Event.LOGOUT_COMPLETE, function()
	print("logged out")
end)
facebook:addEventListener(Event.REQUEST_COMPLETE, function(e)
	print("request complete:", e.type, e.response)
end)

facebook:addEventListener(Event.REQUEST_ERROR, function(e)
	print("request error:", e.type, e.error)
end)

facebook:login("352852401492555", {"public_profile", "user_friends", "publish_actions"})

local text = TextField.new(nil, "post to wall")
text:setScale(3)
text:setPosition(10, 50)
stage:addChild(text)
text:addEventListener(Event.TOUCHES_BEGIN, function(e)
	if text:hitTestPoint(e.touch.x, e.touch.y) then
		facebook:dialog("feed", {
			link = "http://giderosmobile.com", 
			picture = "http://www.giderosmobile.com/wp-content/uploads/2012/06/gideros-mobile-small.png", 
			name = "GiderosMobile.com", 
			caption = "Awesome tool", 
			description = "Check out this awesome product"
		})
		--facebook:dialog("apprequests");
	end
end)


local text2 = TextField.new(nil, "Make api request")
text2:setScale(3)
text2:setPosition(10, 150)
stage:addChild(text2)
text2:addEventListener(Event.TOUCHES_BEGIN, function(e)
	if text2:hitTestPoint(e.touch.x, e.touch.y) then
		print("try")
		--facebook:graphRequest("100001081690027/picture", {width = 144, height=144})
		--facebook:get("me")
		--print("trying to log out")
		--facebook:logout()
		facebook:postPhoto("ball.png", {message = "Test"})
	end
end)