require "googlelicensing"

googlelicensing:setKey("MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAlanL9lpyJbFyQ1TcxxIXxo13SZCxgXEVSedfOOXfI6XZF1r/zUu9gsyIsgytSCzKt8IKwAZhU9hPteLhJsYhXj0hXorC+hgWcmd3LUeDQkxqPUQdVzdEKE0g4m0AshQoiVsJmgejxQCZzhc6wsLss0EgmXXYjndr53OrGK3sil7KSvrLonm7CdGChTFqECpGazQEW+ru5lzbI1MEqU1KsGg31DyvPeh27Xsbk9I6I6Ff/2LvEfMZa//0LVJwJmS0p+Z/LUUnv3Qi+V5KopOMx1pkqXY3EL9SAdaPABAkr2j2Ic8L5YtDaMqhwjZtp7As/9EmMFlwo765/9RTHf8fCwIDAQAB") --set your public key

local text = TextField.new(nil, "Touch to check license")
text:setScale(2)
text:setPosition(10, 100)
stage:addChild(text)

googlelicensing:addEventListener(Event.LICENSE_ALLOWED, function()
	AlertDialog.new("Enjoy the game", "You can playe the game, license verified", "Ok"):show()
end)

googlelicensing:addEventListener(Event.LICENSE_DENIED, function()
	AlertDialog.new("Buy the game", "You need to buy the game to play it", "Cancel", "Buy"):show()
end)

googlelicensing:addEventListener(Event.LICENSE_RETRY, function()
	AlertDialog.new("Please retry", "We can not verify your license right now. Please retry later.", "Ok"):show()
end)

googlelicensing:addEventListener(Event.DOWNLOAD_REQUIRED, function()
	AlertDialog.new("Need download", "Need to download some files", "Ok"):show()
end)

googlelicensing:addEventListener(Event.DOWNLOAD_NOT_REQUIRED, function()
	AlertDialog.new("Enjoy the game", "Everything ok, lets play", "Ok"):show()
end)

googlelicensing:addEventListener(Event.DOWNLOAD_STATE, function(e)
	print(e.state, e.message, "Ok")
end)

googlelicensing:addEventListener(Event.DOWNLOAD_PROGRESS, function(e)
	print("Download progress", e.speed, e.timeLeft, e.progress, e.total)
end)

googlelicensing:addEventListener(Event.ERROR, function(e)
	AlertDialog.new("Error", e.error, "Ok"):show()
end)

text:addEventListener(Event.TOUCHES_BEGIN, function()
	print("started checking")
	--googlelicensing:checkLicense() -- check for license
	googlelicensing:checkExpansion() -- check for license
	--googlelicensing:cellularDownload(true)
end)

