require "flurry"
-- Put your Flurry API key here
local api_key = "" 
if flurry ~= nil and flurry.isAvailable() then
	print("Flurry is available")
	flurry.startSession(api_key)
	flurry.logEvent("test_event",{name="test"})
else
	print("Flurry is not available")
end