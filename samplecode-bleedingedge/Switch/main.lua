local off = Bitmap.new(Texture.new("off.png"))
local on = Bitmap.new(Texture.new("on.png"))

local switch = Switch.new(off, on)
switch:addEventListener("change", function(event) print(event.state) end)
stage:addChild(switch)
