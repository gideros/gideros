Layer = Core.class(Sprite)

function Layer:init(name)
	self.name = name
end
-- remove all childs
function Layer:clear()
	for i = self:getNumChildren(), 1, -1 do 
		self:removeChildAt(i)
	end
end
--

Layers = Core.class(Sprite)

function Layers:init(...)
	self.layers = {}
	self:load(...)
end
-- add new layers on top 
function Layers:load(...)
	for i,name in ipairs({...}) do 
		self:addLayer(name)
	end
end
-- add layer on top of given layer
-- for example, if we have layers: "L1", "L2", "L3"
-- we can add layer after "L2", using this method:  layers:insert("L2", "newLayerName")
-- now we have this structure: "L1", "L2", "newLayerName", "L3"
function Layers:insert(name_or_id, name)
	local lr = self:getLayer(name_or_id)
	local i = self:getChildIndex(lr)
	self:addLayer(name, i)
end
-- create new layer, add it on top
function Layers:addLayer(name, ind)
	local i = ind or self:getNumChildren()+1
	
	assert(self.layers[name] == nil, "Layer \""..name.."\" already exists")
	
	-- simple layer object
	local lr = Layer.new(name)	
	self.layers[name] = lr
	self:addChildAt(lr, i)
end
-- delete layer
function Layers:removeLayer(name_or_id)
	local lr = self:getLayer(name_or_id)
	self.layers[lr.name] = nil
	self:removeChild(lr)
end
-- delete all layers
function Layers:removeAllLayers()
	for i = self:getNumChildren(), 1, -1 do 
		self:removeLayer(i)
	end
end
-- get layer by name
function Layers:getByName(name)
	assert(self.layers[name], "Layer with name \"".. name .."\" does not exist")
	return self.layers[name]
end
-- get layer by index
function Layers:getByID(id)
	return self:getChildAt(id)
end
-- get layer by name OR index
function Layers:getLayer(name_or_id)
	if (type(name_or_id) == "string") then 
		return self:getByName(name_or_id)
	elseif (type(name_or_id) == "number") then 
		return self:getByID(name_or_id)
	end
	error("First paramter must be string or number, but was \""..type(name_or_id).."\".")
end
-- add sprite to layer
function Layers:add(name_or_id, sprite)
	local lr = self:getLayer(name_or_id)	
	lr:addChild(sprite)
end
-- add sprite to top layer
function Layers:addTop(sprite)
	local lr = self:getLayer(self:getNumChildren())
	lr:addChild(sprite)
end
-- remove sprite from layer
function Layers:remove(sprite)
	sprite:removeFromParent()
end