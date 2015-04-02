
t=0

function update()
  t=t+1
  application:setBackgroundColor(0xff00+t%256)

  local x,y=rect:getPosition()
  x=x+math.sin(0.01*t)
  y=y+math.cos(0.01*t)
  rect:setPosition(x,y)
  rect:setRotation(t)

  ego:setScale(2*math.sin(0.01*t))
end

application:setBackgroundColor(0)

rect=Shape.new()

rect:setFillStyle(Shape.SOLID,0xff0000)
rect:setLineStyle(20,0x0000ff)

rect:beginPath()
rect:moveTo(-50,-50)
rect:lineTo(50,-50)
rect:lineTo(50,50)
rect:lineTo(-50,50)
rect:closePath()
rect:endPath()

--stage:addChild(rect)

rect:setPosition(450,300)

ego=Bitmap.new(Texture.new("ball.png"))
stage:addChild(ego)
ego:setPosition(600,200)
--ego:setAnchorPoint(0.5,0.5)

--text=TextField.new(nil,"Gideros on WinRT!")
--stage:addChild(text)
--text:setPosition(20,20)
--text:setScale(2)
--text:setRotation(45)
--text:setTextColor(0xff00ff)

local rt1 = RenderTarget.new(500, 500, true)
rt1:clear(0xff0000,1)
brt1 = Bitmap.new(rt1)
stage:addChild(brt1)
brt1:setPosition(50,50)

local bitmap = Bitmap.new(Texture.new("ball.png"))
bitmap:setPosition(20,20)
rt1:draw(bitmap)

stage:addEventListener(Event.ENTER_FRAME,update)
