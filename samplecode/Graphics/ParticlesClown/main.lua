
local clown=Bitmap.new(Texture.new("clown.png"))
local w,h=clown:getWidth(),clown:getHeight()

local render=RenderTarget.new(w,h)
render:clear(0,0)
render:draw(clown)

local particlesClown=Particles.new()

local p
for y=0,h-1 do
	for x=0,w-1 do
		local c,a=render:getPixel(x,y)
		if a>0 then
			p=particlesClown:addParticles((-w*5)+x*10,(-h*5)+y*10,9,100)
			particlesClown:setParticleColor(p,c,a)
		end
	end
end

particlesClown:setPosition(400,400)
particlesClown:setScale(2)
stage:addChild(particlesClown)

local count=0
local pos=0
function gameLoop(e)
	particlesClown:setRotation(particlesClown:getRotation()+2)
	count=count+1
	if count>300 then
		pos=pos+1
		if pos<=p then
			particlesClown:setParticleSpeed(pos,math.random()-0.5,math.random()-0.5,0.5,1)
		end
	end
--	for loop=1,#pixels do
--		local c,a=pixels[loop]:getColor()
--		a=a+0.03
--		if a>1 then a=a-1 end
--		pixels[loop]:setColor(c,a)
--	end
end

stage:addEventListener(Event.ENTER_FRAME,gameLoop)