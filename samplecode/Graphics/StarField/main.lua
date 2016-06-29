w=application:getContentWidth()
h=application:getContentHeight()
application:setBackgroundColor(0)
 
stars=Particles.new()
stars:setPosition(w/2,h/2)
stage:addChild(stars)
 
function gameLoop(e)
	stars:addParticles({{x=0,y=0,size=5,ttl=200,speedX=math.random()-0.5,speedY=math.random()-0.5,decay=1.04}})
end
 
stage:addEventListener(Event.ENTER_FRAME,gameLoop)