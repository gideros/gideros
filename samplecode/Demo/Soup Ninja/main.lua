local bg = Bitmap.new(Texture.new("background.png"))
stage:addChild(bg)
local sensei = Bitmap.new(Texture.new("sensei.png"))
sensei:setPosition(10, 40)
stage:addChild(sensei)


local font = BMFont.new("made_in_china.fnt", "made_in_china.png")
local number = BMTextField.new(font, "3")
number:setY(43)
stage:addChild(number)

local soup1 = Bitmap.new(Texture.new("soup1.png"))
local soup2 = Bitmap.new(Texture.new("soup2.png"))
soup1:setAnchorPoint(0.5, 1)
soup2:setAnchorPoint(0.5, 1)
soup1:setPosition(240, 400)
soup2:setPosition(240, 400)

local vegetableLayer = Sprite.new()

stage:addChild(soup1)
stage:addChild(vegetableLayer)
stage:addChild(soup2)

local vegetables = {}

local function split(x1, y1, x2, y2)
	for i=1,#vegetables do
		vegetables[i]:split(x1, y1, x2, y2)
	end
end


local shape = Shape.new()
stage:addChild(shape)

local x, y

local xs = {}
local ys = {}
local ss = {}

function onMouseDown(event)
	x,y = event.x,event.y
end

function onMouseMove(event)
	x,y = event.x,event.y
end

function onMouseUp()
	x,y = nil,nil
end

function onEnterFrame()
	if x and y then
		xs[#xs + 1] = x
		ys[#ys + 1] = y
		ss[#ss + 1] = 10
	end
		
	while #ss > 0 and ss[1] <= 1 do
		table.remove(xs, 1)
		table.remove(ys, 1)
		table.remove(ss, 1)
	end

	shape:clear()
	for i=1,#xs-1 do
		shape:setLineStyle(ss[i], 0x8080ff)
		shape:beginPath()
		shape:moveTo(xs[i], ys[i])
		shape:lineTo(xs[i+1], ys[i+1])
		shape:endPath()
	end

	for i=1,#ss do
		ss[i] = ss[i] - 1
	end

	if #ss > 0 then
		local x2 = xs[#ss]
		local y2 = ys[#ss]
		
		for i=#ss-1, 1, -1 do
			local x1 = xs[i]
			local y1 = ys[i]
			
			local d = (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2)
			if d > 10 then
				split(x1, y1, x2, y2)
			end			
		end
	end
end

stage:addEventListener(Event.MOUSE_DOWN, onMouseDown)
stage:addEventListener(Event.MOUSE_MOVE, onMouseMove)
stage:addEventListener(Event.MOUSE_UP, onMouseUp)
stage:addEventListener(Event.ENTER_FRAME, onEnterFrame)


local function fire()
	number:setText(tostring(math.random(1, 4)))
	number:setX(96-number:getWidth()/2)

	local texture = Texture.new("vegetable1.png", true)
	local vertices = {{x=7.5,  y=87},
					  {x=0,    y=63},
					  {x=2.5,  y=31},
					  {x=18,   y=0},
					  {x=26.5, y=0},
					  {x=33.5, y=39},
					  {x=36.5, y=118}}

	local vegetable = Vegetable.new(texture, vertices)
	
	if math.random(0, 1) == 0 then
		vegetable:setPosition(480, 250)
		vegetable.xspeed = -math.random(13, 26) / 10
	else
		vegetable:setPosition(-50, 250)
		vegetable.xspeed = math.random(13, 26) / 10
	end
	vegetable.yspeed = -7
	
	vegetables[#vegetables + 1] = vegetable

	vegetableLayer:addChild(vegetable)
end


local function onEnterFrame2()
	for i=#vegetables,1,-1 do
		local x,y = vegetables[i]:getPosition()
		x = x + vegetables[i].xspeed
		y = y + vegetables [i].yspeed
		vegetables[i].yspeed = vegetables[i].yspeed + 0.1
		vegetables[i]:setPosition(x, y)
		
		if y > 320 then
			vegetableLayer:removeChild(vegetables[i])
			table.remove(vegetables, i)
		end
	end
end


stage:addEventListener(Event.ENTER_FRAME, onEnterFrame2)

timer = Timer.new(3000, 1000)
timer:addEventListener(Event.TIMER, fire)
timer:start()
fire()

