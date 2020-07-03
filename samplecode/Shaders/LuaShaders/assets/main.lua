

local tex=Texture.new("texture.jpg",false)
local function buildEffect(effect,w,h)
	local s=Pixel.new(w,h,tex)
	s:setShader(effect)
	return s
end

local grid=Pixel.new(0x808080,1,320,480) stage:addChild(grid)
grid:setLayoutParameters{ 
	equalizeCells=true, cellSpacingX=5,cellSpacingY=5,
	insets=5
}
 
local function makeCell(effect,x,y,w,h)
	local cell=buildEffect(effect,1,1)
	grid:addChild(cell)
	cell:setLayoutConstraints{ 
		fill=1, weightx=1, weighty=1,
		gridx=x, gridy=y, gridwidth=w,gridheight=h,
	}
	cell:setLayoutParameters{ }
	local label=Pixel.new(0x808080,1)
	cell:addChild(label)
	label:setLayoutConstraints{ weightx=1,weighty=1,anchorx=1,anchory=1 }
	label:setLayoutParameters{ insets=5 }
	
	local text=TextField.new(nil,(effect and effect.name) or "",{ flags=FontBase.TLF_REF_LINETOP|FontBase.TLF_RIGHT|FontBase.TLF_BOTTOM,w=1000 })
	label:addChild(text)
	text:setLayoutConstraints{ fill=1,weightx=1, weighty=1 }
	text:setTextColor(0xFFFFFF)
end

makeCell(Effect.none,0,0,1,1)
makeCell(Effect.grayscale,1,0,1,1)
makeCell(Effect.blur,2,0,1,2)
makeCell(Effect.saturate,0,1,2,1)
makeCell(Effect.emphasize,0,2,3,1)
makeCell(Effect.waves,0,3,1,1)
makeCell(Effect.bloom,1,3,2,1)

stage:addChild(grid)
