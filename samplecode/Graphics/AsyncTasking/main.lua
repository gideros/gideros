local object1, object2 = Pixel.new(0xc0392b,1,50,50), Pixel.new(0x2980b9,1,50,50)
object1:setY(100); object2:setY(200)
stage:addChild(object1); stage:addChild(object2)

local function moveObjects()
	
	print ("1. Object1 started moving")
	for i=1,135 do
		object1:setX(i)
		Core.yield(true)
	end
	print ("2. Object1 finished moving")
	
	
	print ("3. Let's wait 2sec, then move Object2")
	Core.yield(2)
	
	
	print ("4. 2sec passed, Object2 started moving")
	for i=1,160 do
		object2:setX(i)
		Core.yield(true)
	end
	print ("5. Object2 finished moving")
	
	
	print ("6. Let's move both objects to bottom")
	while true do
		if object1:getY()<430 then
			object1:setY(object1:getY()+1)
		end
		
		if object2:getY()<430 then
			object2:setY(object2:getY()+1)
		end
		
		if object1:getY()>=430 and object2:getY()>=430 then
			print("7. Objects reached bottom, let's break")
			break
		end
		
		Core.yield(true)
	end
	
	
	print ("8. All tasks finished")
end

Core.asyncCall(moveObjects)