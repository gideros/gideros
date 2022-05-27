if (not ImGui) then 
	require "ImGui_pre_build"
	if (not ImGui) then 
		require "ImGui"
	end
end

require "Shapes"

assert(ImGui, "ImGui module not found!")

BaseScene = Core.class(Sprite)

function BaseScene:init(themeName, drawHeader)
	self.drawHeader = drawHeader
	
	self.filledShapes = true
	self.drawAlpha = 0.4
	self.filledAlpha = self.drawAlpha
	self.showDemo = false
	self.themeIndex = 0
	self.scenesList = {}
	self.scenesIndex = -1
	
	self.ui = ImGui.new()
	self.io = self.ui:getIO()
	self:addChild(self.ui)
	
	local style = self.ui:getStyle()
	style:setItemSpacing(8, 3)
	style:setFramePadding(4, 1)
	
	if (themeName == "Dark") then
		self.ui:setDarkStyle()
		self.themeIndex = 0
	elseif (themeName == "Light") then
		self.ui:setLightStyle()
		self.themeIndex = 1
	else
		self.ui:setClassicStyle()
		self.themeIndex = 2
	end
	
	if (sceneManager and sceneManager.scenes) then 
		self.scenesIndex = 0
		for k,v in pairs(sceneManager.scenes) do 
			self.scenesList[#self.scenesList + 1] = k
		end
	end
	
	self:onResize()
	
	self:addEventListener("enterFrame", self.onEnterFrame, self)
	self:addEventListener("applicationResize", self.onResize, self)
end

function BaseScene:onEnterFrame(e)
	local ui = self.ui
	local dt = e.deltaTime
	local sceneSelected = false
	
	ui:newFrame(dt)
	
	if (ui:beginFullScreenWindow("Main", nil, ImGui.WindowFlags_Modal)) then
		if (self.drawHeader) then
			self.showDemo = ui:checkbox("Demo", self.showDemo)
			ui:sameLine()
			local themeIndexChanged = false
			self.themeIndex, themeIndexChanged = ui:combo("Theme", self.themeIndex, "Dark\0Light\0Classic\0\0")
			
			if (themeIndexChanged) then
				if (self.themeIndex == 0) then
					ui:setDarkStyle()
				elseif (self.themeIndex == 1) then
					ui:setLightStyle()
				elseif (self.themeIndex == 2) then
					ui:setClassicStyle()
				end
			end
			
			self.scenesIndex, sceneSelected = ui:combo("Scene", self.scenesIndex, self.scenesList)
			
			self.filledShapes = ui:checkbox("Filled", self.filledShapes)
			
			self.drawAlpha = 1
			if (self.filledShapes) then 
				ui:sameLine()
				self.filledAlpha = ui:sliderFloat("Alpha", self.filledAlpha, 0, 1)
				self.drawAlpha = self.filledAlpha
			end
			
			ui:text("Drag & drop with LMB, scale with RMB")
			ui:separator()
		end
		
		self:onDrawUI()
	end
	
	ui:endWindow()
	if (self.showDemo) then 
		self.showDemo = ui:showDemoWindow(self.showDemo)
	end
	
	ui:render()
	ui:endFrame()
	
	if (sceneSelected) then 
		self:removeAllListeners()
		ui:shutdown()
		sceneManager:changeScene(self.scenesList[self.scenesIndex + 1], 1.0, SceneManager.crossFade)
		sceneManager.scene2.scenesIndex = self.scenesIndex
	end
end

function BaseScene:createRandomShape(centerX, centerY, polyVerticesCount)
	local shapeType = math.random(CuteC2.TYPE_CIRCLE, CuteC2.TYPE_POLY)
	local minValue = 64
	local maxValue = 64
	
	if (shapeType == CuteC2.TYPE_AABB) then 
		local w = math.random(minValue, maxValue)
		local h = math.random(minValue, maxValue)
		local x = centerX - w / 2
		local y = centerY - h / 2
		return Rect.new(x, y, w, h)
	elseif (shapeType == CuteC2.TYPE_CIRCLE) then 
		local r = math.random(minValue, maxValue)
		return Circle.new(centerX, centerY, r)
	elseif (shapeType == CuteC2.TYPE_CAPSULE) then 
		local h = math.random(minValue, maxValue)
		local r = math.random(minValue, maxValue)
		return Capsule.new(centerX, centerY - h / 2, h, r)
	elseif (shapeType == CuteC2.TYPE_POLY) then 
		polyVerticesCount = polyVerticesCount or 6
		assert(polyVerticesCount > 2, "Polygon must have atleast 3 points, bu got: " .. tostring(polyVerticesCount))
		local step = (math.pi * 2) / polyVerticesCount
		local r = math.random(minValue, maxValue)
		local points = {}
		
		for i = 0, polyVerticesCount - 1 do 
			local idx = (i + 1) * 2 - 1
			points[idx + 0] = math.cos(i * step) * r
			points[idx + 1] = math.sin(i * step) * r
		end
		
		return Poly.new(centerX, centerY, points)
	end
end

function BaseScene:createRandomShapes(objects, shapesCount, x, y)
	assert(type(shapesCount) == 'number' and shapesCount > 0, "Incorrect amount of shapes! Must be atleast 1, but was: " .. tostring(shapesCount))
	assert(type(objects) == 'table', "wrong argument #1, must be 'table', but was: " .. type(objects))
	
	local minX, minY, maxX, maxY = application:getLogicalBounds()
	local w = maxX - minX
	local h = maxY - minY
	local cx = minX + w / 2
	local cy = minY + h / 2
	x = x or cx
	y = y or cy
	
	local step = (math.pi * 2) / shapesCount
	for i = 0, shapesCount - 1 do 
		local sx = x + math.cos(i * step) * ((w / 2) - 100)
		local sy = y + math.sin(i * step) * ((h / 2) - 100)
		objects[i+1] = self:createRandomShape(sx, sy, math.random(3, 6))
	end
end

function BaseScene:onResize()
	local ui = self.ui
	local minX, minY, maxX, maxY = application:getLogicalBounds()
	local W = maxX - minX
	local H = maxY - minY
	local sx, sy = ui:getScale()
	
	ui:setPosition(minX, minY)
	self.io:setDisplaySize(W * 1 / sx, H * 1 / sy)
end

function BaseScene:onDrawUI()
end