
--reference point testing position
local shape = Shape.new()
shape:setFillStyle(Shape.SOLID, 0xff0000)
shape:beginPath()
shape:moveTo(0, 0)
shape:lineTo(0, 120)
shape:lineTo(120, 120)
shape:lineTo(120, 0)
shape:closePath()
shape:endPath()
shape:setPosition(90, 90)
stage:addChild(shape)

--Some simple stuff

local sys = native.getClass("java.lang.System")
print(sys.currentTimeMillis())

local Integer = native.getClass("java.lang.Integer")
local i = Integer.new(10)
print(i:intValue())

local StringBuilder = native.getClass("java.lang.StringBuilder")
local str = StringBuilder.new("test")
print(str:toString())


ui.Toast.new("test", ui.Toast.LENGTH_LONG)

local button = ui.Button.new("Text")
button:show()

button:setWidth(100)
button:setHeight(100)
button:setPosition(100, 100)

button:addEventListener("onClick", function(self)
	print("showing toast")
	ui.Toast.new("Test")
end, button)


--checkbox widget

local chk = ui.CheckBox.new()
chk:show()
chk:setPosition(300, 100)

chk:addEventListener("onStateChange", function(e)
	print(e.state)
end)

--toggle widget

local toggle = ui.ToggleButton.new()
toggle:show()
toggle:setPosition(400, 100)

toggle:addEventListener("onStateChange", function(e)
	print(e.state)
end)

--slider widget

local slider = ui.Slider.new()
slider:show()

slider:setWidth(200)
slider:setMaxValue(100)
slider:setMinValue(-100)
slider:setStepValue(5)
slider:setPosition(200, 200)

slider:addEventListener("onChange", function(self, e)
	print(e.value)
end, slider)

slider:setValue(0)

--progress bar widget

local progress = ui.ProgressBar.new()
progress:show()

progress:setWidth(200)
progress:setStepValue(5)
progress:setPosition(200, 250)

progress:setValue(10)
progress:addValue(10)

--activity indicator widget
local activity = ui.ActivityIndicator.new()
activity:setPosition(10, 10)
activity:show()


--text input widget
local textinput = ui.TextInput.new()
--textinput:setWidth(200)
--textinput:setHeight(80)
textinput:setPosition(10, 710)
textinput:show()
textinput:setSecure(true)
textinput:setText("aaa")
print("Getting text: "..textinput:getText())

textinput:addEventListener("onTextChange", function(e)
	print(e.text)
end)

--[[
--webview widget
local web = ui.WebView.new()
web:setWidth(300)
web:setHeight(300)
web:setPosition(200, 300)
web:show()
web:loadUrl("http://google.com/")
--web:loadData("<html><script type='text/javascript'>function show_alert(){alert('JavaScript Working');} show_alert();</script><body><h1 id='test'>Test</h1>!</body></html>")
--web:evalJS("alert('me');")

--videoview widget
local video = ui.VideoView.new()
video:setWidth(300)
video:setHeight(300)
video:setPosition(0, 350)
video:show()
video:setFile("documentariesandyou.mp4")
video:play()
]]
--[[
--alert dialog
local alert = ui.AlertDialog.new()
alert:addEventListener("onButtonClick", function(e)
	print(e.buttonIndex, e.buttonText)
end)
alert:setTitle("Title")
alert:setMessage("Message")
alert:setMiddleButton("Middle")
alert:setRightButton("Right")
alert:setLeftButton("Test")
alert:show()
alert:setLeftButton("Left")
]]

--[[
local opt = ui.OptionsDialog.new({"Option1", "Option2", "Option3"}, false)
opt:setTitle("Select: ")
opt:addEventListener("onOptionClick", function(e)
	print(e.optionIndex, e.optionText, e.optionSelected)
end)
opt:addEventListener("onButtonClick", function(e)
	print(e.buttonIndex, e.buttonText)
end)
opt:setRightButton("Right")
opt:show()
]]

--[[
--time picker dialog
local tp = ui.TimePicker.new()
tp:setTitle("Select: ")
tp:addEventListener("onTimeSet", function(e)
	print(e.hour, e.minute)
end)
tp:addEventListener("onButtonClick", function(e)
	print(e.buttonIndex, e.buttonText)
end)
tp:setRightButton("Cancel")
tp:show()
tp:setTime(0,0)
]]

local dp = ui.DatePicker.new()
dp:setTitle("Select: ")
dp:addEventListener("onDateSet", function(e)
	print(e.year, e.month, e.day)
end)
dp:addEventListener("onButtonClick", function(e)
	print(e.buttonIndex, e.buttonText)
end)
dp:setRightButton("Cancel")
dp:show()

