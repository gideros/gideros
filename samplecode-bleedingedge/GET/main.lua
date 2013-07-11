--[[
<?php 
print_r($_GET); 
?>
--]]

local var = {method=URLLoader.GET, data={["a"]="atılımçetin"}}
local loader = URLLoader.new("http://localhost/dump.php", var)

local function onComplete()
    print(loader.data)
end

local function onError()
    print("error")
end

local function onProgress(event)
    print("progress: " .. event.bytesLoaded .. " of " .. event.bytesTotal)
end

loader:addEventListener(Event.COMPLETE, onComplete)
loader:addEventListener(Event.IO_ERROR, onError)
loader:addEventListener(Event.PROGRESS, onProgress)
