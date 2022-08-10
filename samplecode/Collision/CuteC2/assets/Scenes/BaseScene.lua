if (not ImGui) then 
	require "ImGui"
end

require "Shapes"

assert(ImGui, "ImGui module not found!")

BaseScene = Core.class(Sprite)
BaseScene.currentThemeIndex = 0 -- Dark ImGui theme by default

local helpText = [[
Drag & drop with LMB, scale with RMB

POLYGONS:
	Rotate - MMB (click INSIDE)
	Drag & drop vertex - LMB
	Add vertex - MMB (click OUTSIDE near the edge)
	Remove vertex - MMB (click DIRECTLY on vertex)]]

function BaseScene:init(drawHeader)
	self.drawHeader = drawHeader
	
	self.allShown = true
	self.showDemo = false
	self.scenesList = {}
	self.scenesIndex = -1
	
	self.ui = ImGui.new()
	self.io = self.ui:getIO()
	self:addChild(self.ui)
	
	local style = self.ui:getStyle()
	style:setItemSpacing(8, 3)
	style:setFramePadding(4, 1)
	
	self:updateTheme()
	
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

function BaseScene:updateTheme()
	if (BaseScene.currentThemeIndex == 0) then
		self.ui:setDarkStyle()
	elseif (BaseScene.currentThemeIndex == 1) then
		self.ui:setLightStyle()
	elseif (BaseScene.currentThemeIndex == 2) then
		self.ui:setClassicStyle()
	end
end

function BaseScene:onEnterFrame(e)
	local ui = self.ui
	local dt = e.deltaTime
	local sceneSelected = false
	
	ui:newFrame(dt)
	
	ui:setNextWindowPos(self.screenR - 500, self.screenT, ImGui.Cond_Always)
	ui:setNextWindowSize(500, self.screenH, ImGui.Cond_Always)
	if (ui:beginWindow("Scene properties", nil, ImGui.WindowFlags_NoResize)) then
		if (self.drawHeader) then
			self.showDemo = ui:checkbox("Demo", self.showDemo)
			
			local themeIndexChanged = false
			BaseScene.currentThemeIndex, themeIndexChanged = ui:combo("Theme", BaseScene.currentThemeIndex, "Dark\0Light\0Classic\0\0")
			
			if (themeIndexChanged) then
				self:updateTheme()
			end
			
			self.scenesIndex, sceneSelected = ui:combo("Scene", self.scenesIndex, self.scenesList)
			
			SHAPE_FILL = ui:checkbox("Fill color", SHAPE_FILL)
			
			SHAPE_DRAW_ALPHA = 1
			
			if (SHAPE_FILL) then
				SHAPE_FILL_ALPHA = ui:sliderFloat("Alpha", SHAPE_FILL_ALPHA, 0, 1)
				SHAPE_DRAW_ALPHA = SHAPE_FILL_ALPHA
			end
			
			DRAG_POINT_VISIBLE = ui:checkbox("Draw vertices", DRAG_POINT_VISIBLE)
			
			if (DRAG_POINT_VISIBLE) then
				DRAG_POINT_RADIUS  = ui:sliderFloat("Vertices size", DRAG_POINT_RADIUS, 4, 30)
			end
			
			POLY_LINES_VISIBLE = ui:checkbox("Draw polygon lines", POLY_LINES_VISIBLE)
			
			ui:text(helpText)
			ui:separator()
		end
		
		self:onDrawUI()
	end
	ui:endWindow()
	
	self:onDraw()
	
	if (self.showDemo) then 
		self.showDemo = ui:showDemoWindow(self.showDemo)
	end
	
	ui:render()
	ui:endFrame()
	
	if (sceneSelected) then 
		self:removeAllListeners()
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
	
	local offset = 100
	local spawnW = self.screenW - (offset * 2) - 500
	local spawnH = self.screenH - (offset * 2)
	local hw = spawnW / 2
	local hh = spawnH / 2
	
	x = x or (self.screenL + hw)
	y = y or (self.screenT + hh)
	
	local step = (math.pi * 2) / shapesCount
	for i = 0, shapesCount - 1 do 
		local sx = x + math.cos(i * step) * hw + offset
		local sy = y + math.sin(i * step) * hh + offset
		
		objects[i+1] = self:createRandomShape(sx, sy, math.random(3, 6))
	end
end

function BaseScene:onResize()
	local ui = self.ui
	local minX, minY, maxX, maxY = application:getLogicalBounds()
	local W = maxX - minX
	local H = maxY - minY
	local sx, sy = ui:getScale()
	
	self.screenL = minX
	self.screenT = minY
	self.screenR = maxX
	self.screenB = maxY
	self.screenW = W
	self.screenH = H
	self.screenCX = minX + W / 2
	self.screenCY = minY + H / 2
	
	ui:setPosition(minX, minY)
	self.io:setDisplaySize(W * 1 / sx, H * 1 / sy)
end

function BaseScene:onDrawUI()
	if (self.objects) then 
		if (self.ui:button("Hide/Show all")) then 
			self.allShown = not self.allShown
			for i,obj in ipairs(self.objects) do 
				obj.show = self.allShown
			end
		end
		
		for i,obj in ipairs(self.objects) do 
			obj:drawProperties(self.ui)
		end
	end
end

function BaseScene:onDraw()
end