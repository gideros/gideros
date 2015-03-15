module("ui", package.seeall)

if not native then
	require "native"
end

local bridge = native.getClass("com.giderosmobile.android.plugins.bridge.GBridge")
local activity = bridge.getActivity()
local layout = bridge.getLayout()
local style = native.getClass("android.R$attr")

local LayoutParams = native.getClass("android.widget.FrameLayout$LayoutParams")

--some classes we will need on the way
local String = native.getClass("java.lang.String")
local Boolean = native.getClass("java.lang.Boolean")


--[[
        TOAST
]]--
local toast

Toast = Core.class()

Toast.LENGTH_SHORT = 0
Toast.LENGTH_LONG = 1

function Toast:init(text, duration)     
        duration = duration or 0
        if toast == nil then
            toast = native.getClass("android.widget.Toast")
        end
        local t = toast.makeText(activity, text, duration)
        t:show()
end

Widget = Core.class(EventDispatcher)

function Widget:init(text)
    self._first = true
    self._iw = LayoutParams.WRAP_CONTENT
    self._ih = LayoutParams.WRAP_CONTENT
    self._ix = 0
    self._iy = 0
    self._w = 0
    self._h = 0
    self._x = 0
    self._y = 0
    self._isAdded = false
	self._lparams = LayoutParams.new(self._iw, self._ih)
	self._lparams.gravity = 51
end

function Widget:show()
    if not self._isAdded and self._core ~= nil then
        self._isAdded = true
        if self._first then
            self._first = false
            self._core:setLayoutParams(self._lparams)
        end
        layout:addView(self._core)
    end
end

function Widget:hide()
    if self._isAdded and self._core ~= nil then
        self._isAdded = false
        layout:removeView(self._core)
    end
end

function Widget:setWidth(width)
    self._w = width
    self._iw = math.floor(width*application:getLogicalScaleX())
    if self._core ~= nil then
		self._lparams.width = self._iw
        self._core:setLayoutParams(self._lparams)
    end
end

function Widget:setHeight(height)
    self._h = height
    self._ih = math.floor(height*application:getLogicalScaleY())
    if self._core ~= nil then
		self._lparams.height = self._ih
        self._core:setLayoutParams(self._lparams)
    end
end

function Widget:getWidth()
    return self._w
end

function Widget:getWidth()
    return self._h
end

function Widget:setX(x)
    self:setPosition(x, self._y)
end

function Widget:setY(y)
    self:setPosition(self._x, y)
end

function Widget:setPosition(x, y)
    self._x = x
    self._y = y
    self._ix = math.floor(x*application:getLogicalScaleX())+application:getLogicalTranslateX()
    self._iy = math.floor(y*application:getLogicalScaleY())+application:getLogicalTranslateY()
    if self._core ~= nil then
        self._lparams:setMargins(self._ix, self._iy, 0, 0)
        self._core:setLayoutParams(self._lparams)
    end
end

function Widget:getX()
    return self._x
end

function Widget:getY()
    return self._y
end

function Widget:getPosition()
    return self:getX(), self:getY()
end

--[[
    BUTTON WIDGET
    Need To Do
		-styling
        -icons
        -try getting real width after init
]]--

local androidButton = native.getClass("android.widget.Button")

Button = Core.class(Widget)

function Button:init(text)
    text = text or ""
    --core element of derived widget
    self._core = androidButton.new(activity)
    self:setText(text)
        
    --set up event
    local eventProxy = native.createProxy("android.view.View$OnClickListener", {onClick = function()
        self:dispatchEvent(Event.new("onClick"))
    end})
    self._core:setOnClickListener(eventProxy)
end

function Button:setText(text)
    self._text = text
    self._core:setText(text)
end

function Button:getText()
    return self._text
end

--[[
    GENERIC SWITCH WIDGET CLASS
    Need To Do
        -Implement RadioButtons and RadioGroup
]]--

Switch = Core.class(Widget)

function Switch:init()
    --set up event
    self.eventProxy = native.createProxy("android.widget.CompoundButton$OnCheckedChangeListener", {onCheckedChanged = function(view, state)
        local event = Event.new("onStateChange")
        event.state = state
        self:dispatchEvent(event)
    end})
end

function Switch:isChecked()
    return self._core:isChecked()
end

function Switch:setState(state)
    self._core:setChecked(state)
end


--[[
    CHECKBOX SWITCH WIDGET
]]--

CheckBox = Core.class(Switch)

local androidCheckbox = native.getClass("android.widget.CheckBox")
function CheckBox:init()
    --core element of derived widget
    self._core = androidCheckbox.new(activity)     
    self._core:setOnCheckedChangeListener(self.eventProxy)
end


--[[
    TOGGLE (ON/OFF) SWITCH WIDGET
]]--

ToggleButton = Core.class(Switch)

local androidToggleButton = native.getClass("android.widget.ToggleButton")

function ToggleButton:init()
    --core element of derived widget
    self._core = androidToggleButton.new(activity)  
    self._core:setOnCheckedChangeListener(self.eventProxy)
end

--[[
    GENERIC BAR WIDGET
]]--

Bar = Core.class(Widget)

function Bar:init()
    self._value = 0
    self._ivalue = 0
    self._minValue = 0
    self._maxValue = 100
    self._imaxValue = 100
    self._stepValue = 1
end

function Bar:_recalculate()
    local newMax = math.abs(self._maxValue - self._minValue)/self._stepValue
    if self._imaxValue ~= newMax then
        self._imaxValue = newMax
        self._core:setMax(newMax)
    end
end

function Bar:setValue(value)
    if self._value ~= value then
        self._value = value
        self._ivalue = math.abs(value - self._minValue)/self._stepValue
        self._core:setProgress(self._ivalue)
    end
end

function Bar:addValue(value)
    self._value = self._value + value
    self._ivalue = math.abs(self._value - self._minValue)/self._stepValue
    self._core:setProgress(self._ivalue)
end

function Bar:getValue(value)
    return self._value
end

function Bar:setMaxValue(value)
    self._maxValue = value
    self:_recalculate()
end

function Bar:getMaxValue()
    return self._maxValue
end

function Bar:setMinValue(value)
    self._minValue = value
    self:_recalculate()
end

function Bar:getMinValue()
    return self._minValue
end

function Bar:setStepValue(value)
    self._stepValue = value
    self:_recalculate()
end

function Bar:getStepValue()
    return self._stepValue
end

--[[
    SLIDER WIDGET
    Need to do
		- Provide Drawable Thumb
]]--

Slider = Core.class(Bar)

local androidSlider = native.getClass("android.widget.SeekBar")

function Slider:init()
    --core element of derived widget
    self._core = androidSlider.new(activity)
    --set up event
    local eventProxy = native.createProxy("android.widget.SeekBar$OnSeekBarChangeListener", {onProgressChanged = function(bar, value, user)
        if self._ivalue ~= value and user then
            self._ivalue = value
            self._value = (self._ivalue*self._stepValue) + self._minValue
            local event = Event.new("onChange")
            event.value = self._value
            self:dispatchEvent(event)
         end
    end})
    self._core:setOnSeekBarChangeListener(eventProxy)
end

--[[
        PROGRESSBAR WIDGET
        Need to do
                - Style
]]--

ProgressBar = Core.class(Bar)

local androidProgressBar = native.getClass("android.widget.ProgressBar")

function ProgressBar:init()
    --core element of derived widget
    self._core = androidProgressBar.new(activity, nil, style.progressBarStyleHorizontal)
end

--[[
    ACTIVITY INDICATOR WIDGET
]]--

ActivityIndicator = Core.class(Widget)

function ActivityIndicator:init()
    self._core = androidProgressBar.new(activity, nil, style.progressBarStyleSmall)
end

--[[
    TEXTINPUT WIDGET
    Need to do
        - set correct input type (including combined values)
]]--

TextInput = Core.class(Widget)

local androidEditText = native.getClass("android.widget.EditText")
local androidInputType = native.getClass("android.text.InputType")

function TextInput:init()
    self._core = androidEditText.new(activity)

    self._inputType = androidInputType
    self._isSecure = false
        
    --set up event
    local eventProxy = native.createProxy("android.text.TextWatcher", {afterTextChanged = function(edit)
        local event = Event.new("onTextChange")
        event.text = edit:toString()
        self:dispatchEvent(event)
    end})
    self._core:addTextChangedListener(eventProxy)
		
	local eventFocusProxy = native.createProxy("android.view.View$OnFocusChangeListener", {onFocusChange = function(view, hasFocus)
		if hasFocus then
            local event = Event.new("onFocus")
            self:dispatchEvent(event)
		else
			local event = Event.new("onBlur")
            self:dispatchEvent(event)
		end
    end})
    self._core:setOnFocusChangeListener(eventFocusProxy)
end

function TextInput:setText(text)
    self._core:setText(text)
end

function TextInput:getText()
    return self._core:getText():toString()
end

function TextInput:setInputType(type)
    self._core:setInputType(self._inputType.TYPE_CLASS_NUMBER)
end

function TextInput:setSecure(secure)
    if self._isSecure ~= secure then
        self._isSecure = secure
        local type = self._core:getInputType()
        if secure then
            self._core:setInputType(self._inputType.TYPE_TEXT_VARIATION_PASSWORD)
        else
            self._core:setInputType(not self._inputType.TYPE_TEXT_VARIATION_PASSWORD)
        end
    end
end

function TextInput:isSecure()
    return self._isSecure
end


--[[
    WEBVIEW WIDGET
    Need To Do
        - implement additional setting based on what is available on both Android and IOS
]]--

WebView = Core.class(Widget)

local androidWebView = native.getClass("android.webkit.WebView")
local androidWebViewClient = native.getClass("android.webkit.WebViewClient")
local androidWebChromeClient = native.getClass("android.webkit.WebChromeClient")

function WebView:init()
    self._core = androidWebView.new(activity)
    local sets = self._core:getSettings()
    sets:setJavaScriptEnabled(true)
    sets:setBuiltInZoomControls(true)
    self._core:requestFocusFromTouch()
    self._core:setWebViewClient(androidWebViewClient.new())
    self._core:setWebChromeClient(androidWebChromeClient.new())
end

function WebView:loadUrl(url)
    self._core:loadUrl(url);
end

function WebView:loadData(data, type, encoding)
    type = type or "text/html"
    encoding = encoding or "utf8"
    self._core:loadData(data, type, encoding)
end

function WebView:evalJS(js)
    self._core:loadUrl("javascript:(function(){"..js.."})()")
end

function WebView:goBack()
    self._core:goBack()
end

function WebView:goForward()
    self._core:goForward()
end


VideoView = Core.class(Widget)

local androidVideoView = native.getClass("android.widget.VideoView")
local androidMediaController = native.getClass("android.widget.MediaController")
local androidPixelFormat = native.getClass("android.graphics.PixelFormat")

function VideoView:init()
    activity:getWindow():setFormat(androidPixelFormat.TRANSLUCENT)
    self._core = androidVideoView.new(activity)
    self._core:setZOrderMediaOverlay(true)
    self._mc = androidMediaController.new(activity)
    self._mc:setAnchorView(self._core)
    self._core:setMediaController(self._mc)
end

function VideoView:setFile(file)
    local path = native.getPath(file)
    print("path", path)
    self._core:setVideoPath(path);      
    self._core:requestFocus();
end

function VideoView:play()
    self._core:start()
end

function VideoView:stop()
    self._core:stopPlayback()
end

function VideoView:pause()
    self._core:pause()
end

function VideoView:isPlaying()
    return self._core:isPlaying()
end

function VideoView:isPlaying()
    return self._core:isPlaying()
end


--[[
        GENERIC DIALOG WIDGET
]]--

Dialog = Core.class(EventDispatcher)

local androidDialogInterface = native.getClass("android.content.DialogInterface")

function Dialog:init()
    self._title = ""
    self._message = ""
    self._visible = false
    self._negative = "Cancel"
    self._neutral = ""
    self._positive = ""
    self._c = androidDialogInterface
    --set up click event
    self.eventClick = native.createProxy("android.content.DialogInterface$OnClickListener", {onClick= function(dialog, button)
        self._visible = false
        local event = Event.new("onButtonClick")
        if button == self._c.BUTTON_NEGATIVE then
            event.buttonIndex = 0
            event.buttonText = self._negative
        elseif button == self._c.BUTTON_NEUTRAL then
            event.buttonIndex = 1
            event.buttonText = self._neutral
        elseif button == self._c.BUTTON_POSITIVE then
            event.buttonIndex = 2
            event.buttonText = self._positive
        end
        self:dispatchEvent(event)
    end})
    
    --set up cancel event
    self.eventCancel = native.createProxy("android.content.DialogInterface$OnCancelListener", {onCancel= function(dialog, button)
        self._visible = false
        local event = Event.new("onButtonClick")
        event.buttonIndex = 0
        event.buttonText = self._negative
        self:dispatchEvent(event)
    end})
end

function Dialog:setTitle(title)
    self._title = title
    self._core:setTitle(self._title)
end

function Dialog:setMessage(text)
    self._message = text
    self._core:setMessage(self._message)
end

function Dialog:show()
    self._visible = true
    self._core:show()
end

function Dialog:hide()
    self._visible = false
    self._core:hide()
end

function Dialog:setRightButton(text)
    self._negative = text
    local button = self._core:getButton(self._c.BUTTON_NEGATIVE)
    if button then
        button:setText(self._negative)
        button:invalidate()
    else
        self._core:setButton(self._c.BUTTON_NEGATIVE, self._negative, self.eventClick)
    end
end

function Dialog:setMiddleButton(text)
    self._neutral = text
    local button = self._core:getButton(self._c.BUTTON_NEUTRAL)
    if button then
        button:setText(self._neutral)
        button:invalidate()
    else
        self._core:setButton(self._c.BUTTON_NEUTRAL, self._neutral, self.eventClick)
    end
end

function Dialog:setLeftButton(text)
    self._positive = text
    local button = self._core:getButton(self._c.BUTTON_POSITIVE)
    if button then
        button:setText(self._positive)
        button:invalidate()
    else
        self._core:setButton(self._c.BUTTON_POSITIVE, self._positive, self.eventClick)
    end
end


--[[
        ALERTDIALOG WIDGET
]]--

local androidAlertDialog = native.getClass("android.app.AlertDialog$Builder")

AlertDialog = Core.class(Dialog)

function AlertDialog:init()
    self._core = androidAlertDialog.new(activity):create()
    self._core:setCancelable(true)
    self._core:setButton(self._c.BUTTON_NEGATIVE, self._negative, self.eventClick)
    self._core:setOnCancelListener(self.eventCancel)
end

--[[
        OPTIONDIALOG WIDGET
]]--

OptionsDialog = Core.class(Dialog)

function OptionsDialog:init(options, multi)
    self._options = options
    local arr = String.newArray(options)
     
    local build = androidAlertDialog.new(activity)
    if not multi then
        --set up click event
        local eventClick = native.createProxy("android.content.DialogInterface$OnClickListener", {onClick= function(dialog, choice)
			self._visible = false
			local event = Event.new("onOptionClick")
			event.optionIndex = choice+1
			event.optionText = self._options[choice+1]
			event.optionSelected = true
			self:dispatchEvent(event)
        end})
        build:setItems(arr, eventClick)
    else
        local eventClick = native.createProxy("android.content.DialogInterface$OnMultiChoiceClickListener", {onClick= function(dialog, choice, isChecked)
            self._visible = false
            local event = Event.new("onOptionClick")
            event.optionIndex = choice+1
            event.optionText = self._options[choice+1]
            event.optionSelected = isChecked
            self:dispatchEvent(event)
        end})
        build:setMultiChoiceItems(arr, nil, eventClick)
    end
    self._core = build:create()
    self._core:setCancelable(true)
    self._core:setButton(self._c.BUTTON_NEGATIVE, self._negative, self.eventClick)
    self._core:setOnCancelListener(self.eventCancel)
end

--[[
        TIMEPICKER WIDGET
]]--

TimePicker = Core.class(Dialog)

local androidTimePicker = native.getClass("android.app.TimePickerDialog")

function TimePicker:init(houre, minute, format24)
    hour = hour or os.date("%H")
    minute = minute or os.date("%M")
    format24 = format24 or false
    local eventClick = native.createProxy("android.app.TimePickerDialog$OnTimeSetListener", {onTimeSet = function(dialog, hour, minute)
        self._visible = false
        local event = Event.new("onTimeSet")
        event.hour = hour
        event.minute = minute
        self:dispatchEvent(event)
    end})
    self._core = androidTimePicker.new(activity, eventClick, tonumber(hour), tonumber(minute), format24)
    self._core:setCancelable(true)
    self._core:setButton(self._c.BUTTON_NEGATIVE, self._negative, self.eventClick)
    self._core:setOnCancelListener(self.eventCancel)
end

function TimePicker:setTime(hour, minute)
    self._core:updateTime(tonumber(hour), tonumber(minute))
end


--[[
    DATEPICKER WIDGET
]]--

DatePicker = Core.class(Dialog)

local androidDatePicker = native.getClass("android.app.DatePickerDialog")

function DatePicker:init(year, month, day)
    year = year or os.date("%Y")
    month = month or os.date("%m")
    day = day or os.date("%d")
    local eventClick = native.createProxy("android.app.DatePickerDialog$OnDateSetListener", {onDateSet = function(dialog, year, month, day)
        self._visible = false
        local event = Event.new("onDateSet")
        event.year = year
        event.month = month
        event.day = day
        self:dispatchEvent(event)
    end})
    self._core = androidDatePicker.new(activity, eventClick, tonumber(year), tonumber(month), tonumber(day))
    self._core:setCancelable(true)
    self._core:setButton(self._c.BUTTON_NEGATIVE, self._negative, self.eventClick)
    self._core:setOnCancelListener(self.eventCancel)
end

function DatePicker:setDate(year, month, day)
    self._core:updateDate(tonumber(year), tonumber(month), tonumber(day))
end
