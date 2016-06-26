Assets={}
Assets.count=1000000
Assets.base="sky_world.png"

--Set up a progress bar
ProgressBar=Pixel.new(0x000000,1,application:getContentWidth(),50)
local innerBar=Pixel.new(0xFF0000,1,application:getContentWidth()-10,40)
innerBar:setScale(0,1)
innerBar:setPosition(5,5)
ProgressBar:addChild(innerBar)
ProgressBar.setProgress=function (self,ratio)
 self:getChildAt(1):setScale(ratio,1)
end
stage:addChild(ProgressBar)
ProgressBar:setY((application:getContentHeight()-50)/2)

function loadAssets()
 local ts=os.timer()
 print ("Start loading assets")
 for i=1,Assets.count do
  Assets[i]=Texture.new(Assets.base)  
  ProgressBar:setProgress(i/Assets.count)
 end
 print ("Finished loading assets in",os.timer()-ts)
end

Core.asyncCall(loadAssets)
--loadAssets()