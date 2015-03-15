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

googleplay:addEventListener(Event.LOGIN_ERROR, function()
	print("LOGIN_ERROR")
end)

googleplay:addEventListener(Event.LOGIN_COMPLETE, function()
	print("LOGIN_COMPLETE")
	--googleplay:showSettings()
	--googleplay:reportScore("CgkIq_W6x6sQEAIQBA", 100)
	--googleplay:showLeaderboard("CgkIq_W6x6sQEAIQBA")
	--googleplay:reportAchievement("CgkIq_W6x6sQEAIQAQ")
	stage:addEventListener(Event.MOUSE_DOWN, function()
		--print(googleplay:getCurrentPlayer())
		--print(googleplay:getCurrentPlayerId())
		--googleplay:reportScore("CgkIq_W6x6sQEAIQBA", 550, true)
		--googleplay:reportAchievement("CgkIq_W6x6sQEAIQAw", 2, true)
		--googleplay:showAchievements()
		--googleplay:showLeaderboard("CgkIq_W6x6sQEAIQBA")
		--googleplay:loadAchievements()
		--googleplay:loadScores("CgkIq_W6x6sQEAIQBA")
		--googleplay:loadPlayerScores("CgkIq_W6x6sQEAIQBA")
		googleplay:loadState(0)
		--googleplay:updateState(0, "my supers syncccccc", true)
		--googleplay:deleteState(0)
	end)
	
end)

googleplay:addEventListener(Event.LOAD_SCORES_COMPLETE, function(e)
	print("LOAD_SCORES_COMPLETE")
	print_r(e)
end)

googleplay:addEventListener(Event.REPORT_SCORE_COMPLETE, function()
	print("REPORT_SCORE_COMPLETE")
end)

googleplay:addEventListener(Event.LOAD_ACHIEVEMENTS_COMPLETE, function(e)
	print("LOAD_ACHIEVEMENTS_COMPLETE")
	print_r(e.achievements)
end)

googleplay:addEventListener(Event.REPORT_ACHIEVEMENT_COMPLETE, function()
	print("REPORT_ACHIEVEMENT_COMPLETE")
end)

googleplay:addEventListener(Event.STATE_LOADED, function(e)
	print("STATE_LOADED")
	print(e.key, e.isFresh, e.data)
end)

googleplay:addEventListener(Event.STATE_ERROR, function(e)
	print("STATE_ERROR")
	print(e.key, e.error)
end)

googleplay:addEventListener(Event.STATE_CONFLICT, function(e)
	print("STATE_CONFLICT")
	print(e.key, e.version, e.localData, e.serverData)
	googleplay:resolveState(e.key, e.version, "data resolved in conflict")
end)

googleplay:addEventListener(Event.STATE_DELETED, function(e)
	print("STATE_DELETED")
	print(e.key)
end)

local text = TextField.new(nil, "Test")
text:setScale(4)
text:setPosition(100, 100)
stage:addChild(text)

print(googleplay:isAvailable())
googleplay:login()
--googleplay:showSettings()