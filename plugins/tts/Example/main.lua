require "tts"
 
local platform =application:getDeviceInfo()

print(platform)
 
--local tts=TTS.new()
local tts=TTS.new("fr",1,1)
local count = 1
local text = "This is a fairly long sentence that will test the new TTS engine for Android devices." 
 
tts:addEventListener(Event.TTS_ERROR,function (e)
  print("TTS Error:"..e.error)
end)

local function stopSpeech()
    tts:stop()

    local function resumeSpeech()
      tts:speak("It was stopped and has been resumed")
    end
    local timerB=Timer.new(500,1)
    timerB:addEventListener(Event.TIMER,resumeSpeech)
    timerB:start()
end
local timer=Timer.new(9000,1)
timer:addEventListener(Event.TIMER,stopSpeech)


  tts:addEventListener(Event.TTS_INIT_COMPLETE,function ()
    print("TTS Initialised")
    local sentence = "Test "..count..text 
  tts:setVolume(1)
    tts:speak(sentence,tostring(count))
    timer:start()
    --count = count + 1
  end)
  tts:setVolume(1)
 
tts:addEventListener(Event.TTS_UTTERANCE_COMPLETE,function (e)
    
  print("Utterance:"..e.utteranceId.." state:"..e.state.. " Count: "..count)
 
  if e.state == "start" then
    print("started")
  elseif e.state == "done" then
      count = count + 1
    local sentence = "test "..count .. text 
 
      if count == 2 then 
       tts:setLanguage("en-US")
        tts:speak(sentence.. " in British English",tostring(count))
    elseif count == 3 then 
      tts:setSpeed(0.4)
      tts:setPitch(2)
      tts:speak("The pitch and speed have been changed",tostring(count))
    elseif count == 4 and platform == "iOS" then 
      tts:setSpeed(1)
      tts:setPitch(1)
      tts:setVolume(0.2)
      print("volume: "..tts:getVolume())
      tts:speak("This is quiet","3")
      tts:setVolume(1)
    elseif count ==4 then  
      count=count+1
      tts:shutdown()    
    elseif count == 5 and platform == "iOS" then
      tts:setVoice("com.apple.ttsbundle.siri_female_en-GB_premium")
      tts:speak("A new voice has been selected by the identifier, but it needs to be installed on the device.",tostring(count))   
      print("pitch: "..tts:getPitch()..", speed: "..tts:getSpeed())
      local voices = tts:getVoicesInstalled()   
      for i=1,#voices do 
        print(voices[i]["language"]..", "..voices[i]["identifier"]..", "..voices[i]["quality"]..", "..voices[i]["name"])
      end   
      elseif count > 5 then 
        tts:shutdown()  
    end 
         
  end
 
end)

