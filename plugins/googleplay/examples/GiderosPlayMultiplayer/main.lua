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

require "googleplay"

local isPlaying = false

googleplay:addEventListener(Event.GAME_STARTED, function(e)
	--start your game
	--go to game scene
	isPlaying = true
	print("game started")
end)

googleplay:addEventListener(Event.INVITATION_RECEIVED, function(e)
	--received an invitation to play, let's play
	if not isPlaying then
		googleplay:joinRoom(e.invitationId)
		googleplay:showWaitingRoom()
		--or we can display invitations
		--googleplay:showInvitations()
	end
end)

googleplay:addEventListener(Event.ROOM_CREATED, function(e)
	print("new room created for automatch")
	print_r(googleplay:getAllPlayers())
	googleplay:showWaitingRoom(2)
end)


googleplay:addEventListener(Event.LOGIN_COMPLETE, function(e)
	print("Logged in")
	print(googleplay:getCurrentPlayer())
	stage:addEventListener(Event.MOUSE_DOWN, function(e)
		if not isPlaying then
			print("do automatch")
			--automatch for min 2 max 2 players
			googleplay:autoMatch(2, 2)
			--googleplay:showWaitingRoom(2)
			--googleplay:invitePlayers(2, 2)
			--googleplay:showInvitations()
		end
	end)
end)

googleplay:addEventListener(Event.LOGIN_ERROR, function(e)
	print("LOGIN_ERROR")
end)

googleplay:login()