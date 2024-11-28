Tile = Core.class(Sprite)

function Tile:init(name, dorpShadow)
	self.g = Bitmap.new(BLOCKS:getTextureRegion(name..".png"))
	self:addChild(self.g)
	
	self.name = name
	self.isSleeping = false
	self.tx = 0
	self.ty = 0
	
	self:setAnchorPoint(ANCHOR_X,ANCHOR_Y)
	self.path = {}
	self.curIndex = 1
end
--
function Tile:setSleep(flag)
	self.isSleeping = flag
	self.g:setTextureRegion(BLOCKS:getTextureRegion(self.name..".png"))
end
--
function Tile:addWayPoint(x,y)
	self.path[#self.path+1] = {x=x,y=y}
end
--
function Tile:runFadeOUT()
	if not self.fadeOut then 
		self.fadeOut = MovieClip.new{
			{1,5,self,{alpha={0,1}}}
		}
	end
	self.fadeOut:gotoAndPlay(1)
end
--
function Tile:runEndAnimation()
	local frames = 30
	local hs = frames // 2
	if not self.sqush then 
		local y = self:getY()
		self.sqush = MovieClip.new{
			{1,hs,self,
				{alpha = {1,1.5,"inQuartic"}},
			},
			{hs+1,frames,self,
				{alpha = {1.5,1,"outQuartic"}},
			}
		}
	end
	self.sqush:gotoAndPlay(1)
end
--
function Tile:move(delay)
	delay = delay or 0
	if self.curIndex <= #self.path then 
		local point = self.path[self.curIndex]
		
		self:animate{
			duration = ANIM_SPEED,
			values = {
				x = point.x,
				y = point.y,
			}
		}
	end
end
--
function Tile:onGtEnd()
	self.curIndex += 1
	if self.curIndex <= #self.path then 
		self:move()
	else
		self.curIndex = 1
		self.path = {}
		self:runEndAnimation()
	end
end
--
function Tile:animate(t)
	t.duration = t.duration or 0
	t.ease = t.ease or easing.linear
 
	if (self.gt) then 
		self.gt.duration = t.duration
		self.gt.ease = t.ease
		self.gt:setValues(t.values)
	else
		self.gt = GTween.new(self, t.duration, t.values, {ease = t.ease})
		self.gt:addEventListener("complete", self.onGtEnd, self)
	end
	self.gt:setPosition(0)
	self.gt:setPaused(false)
end