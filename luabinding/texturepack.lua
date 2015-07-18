function TexturePack:getTextureRegion(index)
	local x, y, width, height, dx1, dy1, dx2, dy2
	x, y, width, height, dx1, dy1, dx2, dy2 = self:getLocation(index)
	
	if x == nil then
		return nil
	end

	return TextureRegion.new(self, x, y, width, height, dx1, dy1, dx2, dy2)
end

--[[
TextureRegion = BitmapData
TexturePack.getTextureRegion = TexturePack.getBitmapData
--]]
