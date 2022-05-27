function getScreenCenter()
	local minX, minY, maxX, maxY = application:getLogicalBounds()
	local w = maxX - minX
	local h = maxY - minY
	return minX + w / 2, minY + h / 2
end