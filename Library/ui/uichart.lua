--!NEEDS:(GiderosUI)/uicolor.lua
--!NEEDS:(GiderosUI)/uipanel.lua
--!NEEDS:(GiderosUI)/uistyle.lua
--!NEEDS:(GiderosUI)/uishaders.lua

UI.Chart=Core.class(UI.Panel,function() return nil end)

function UI.Chart:init()
	self.series,self.scales={},{}
	self.selected=vector(0,0)
end

function UI.Chart:setSerie(s,n)
	self.series=self.series or {}
	self.series[n or 1]=s
	self:setSeries(self.series)
end

function UI.Chart:setSeries(s)
	self.series=s
	self:updateView()
end

function UI.Chart:setSerieScale(s,n)
	self.scales=self.scales or {}
	self.scales[n or 1]=s
	self:setSeriesScale(self.scales)
end

function UI.Chart:setSeriesScale(s)
	self.scales=s
	self:updateView()
end

function UI.Chart:setSelected(s)
	self.selected=s
	self:updateView()
end

function UI.Chart:updateView()
	local sm=0
	local sn=0
	for k,v in ipairs(self.series) do
		sm=sm<>#v
		sn=k
	end
	
	local d=""
	for y=1,sn do
		local s=self.series[y] or { }
		local ss=self.scales[y] or { }
		local vmin=ss.min or 0
		local vmax=ss.max or 1
		local vspan=vmax-vmin
		
		for x=1,sm do
			local v=((s[x] or 0)-vmin)/vspan
			v=(v*255)//1
			d=d..string.char(v,v,v,255)
		end
	end
	if not self.texture or not TextureBase.update then
		self.texture=Texture.new(d,sm,sn,false,{ format=TextureBase.Y8 })
	else
		self.texture:update(d,sm,sn)
	end
end

UI.Chart.Definition= {
	name="Chart",
	icon="ui/icons/panel.png",
	class="UI.Chart",
	constructorArgs={ },
	properties={
		{ name="Series", type="table", setter=UI.Chart.setSeries, },
		{ name="SeriesScale", type="table", setter=UI.Chart.setSeriesScale, },
		{ name="Selected", type="vector", setter=UI.Chart.setSelected, },
	},
}

-- Axis
UI.Chart.Axis=Core.class(UI.Panel,function() return UI.Colors.white end)
UI.Chart.Axis.Shader=Core.class(UI.Shader)
function UI.Chart.Axis.Shader:init(params)
	self.params=params
	self.shader=self.overrideStandardShader(UI.Chart.Axis.Shader,Shader.SHADER_PROGRAM_BASIC,0,function(spec)
		table.insert(spec.uniforms,{name="vOrientation",type=Shader.CFLOAT,vertex=true})
		table.insert(spec.uniforms,{name="vBounds",type=Shader.CFLOAT4,sys=Shader.SYS_BOUNDS,vertex=true})
		function spec.vertexShader(vVertex,vColor,vTexCoord) : Shader
			local vertex = hF4(vVertex,0.0,1.0)
			if vOrientation==1 then
				fPos=vertex.yx
			elseif vOrientation==2 then
				fPos=vertex.xy
			elseif vOrientation==3 then
				fPos=hF2(vBounds.w-vertex.y,vBounds.z-vertex.x)
			else
				fPos=hF2(vertex.x,vBounds.w-vertex.y)
			end
			return vMatrix*vertex
		end
		table.insert(spec.varying,{name="fPos",type=Shader.CFLOAT2})
		table.insert(spec.uniforms,{name="colLine",type=Shader.CFLOAT4,}) -- Offset,Interval,Count,N/A
		table.insert(spec.uniforms,{name="fAxisScaleWidth",type=Shader.CFLOAT4,}) -- Offset,Interval,Count,N/A
		table.insert(spec.uniforms,{name="fThickness",type=Shader.CFLOAT,}) -- Line thickness
		function spec.fragmentShader() : Shader
			local frag=hF4(0,0,0,0)
			if fPos.y<fThickness then
				frag=colLine
			else
				local bx=(fPos.x-fAxisScaleWidth.x+fThickness/2)/fAxisScaleWidth.y
				local dn=floor(bx)
				if dn<=fAxisScaleWidth.z or fAxisScaleWidth.z<0 then
					if (bx-dn)<(fThickness/fAxisScaleWidth.y) then
						frag=colLine
					end
				end
			end
			frag=fColor*frag
			if (frag.a==0.0) then discard() end
			return lF4(frag)
		end
	end)
end

function UI.Chart.Axis:init()
	self.shader=UI.Chart.Axis.Shader.new({
		fThickness="chart.szAxisThickness",
		colLine="chart.colAxis",
	})
	self.shader:apply(self)
	self:setOrientation(0)
	self:addEventListener(Event.LAYOUT_RESIZED,self.onResize,self)
end

function UI.Chart.Axis:setOrientation(orientation)
	self.orientation=orientation
	self:setShaderConstant("vOrientation",Shader.CFLOAT,1,orientation)
end

function UI.Chart.Axis:updateStyle(fromParent)
	UI.Chart.updateStyle(self)
	self.shader:apply(self)
	self:setAxis(self.axisOffset,self.axisInterval,self.axisCount)
	self:setLabels(self.labelGenerator,self.labelSkip)
end

function UI.Chart.Axis:onResize()
	self:setAxis(self.axisOffset,self.axisInterval,self.axisCount)
	self:setLabels(self.labelGenerator,self.labelSkip)
end

function UI.Chart.Axis:setAxis(offset,interval,count)
	self.axisOffset=offset
	self.axisInterval=interval
	self.axisCount=count
	local iv=interval
	if iv==0 then
		iv=self:getWidth()/count
	else
		if type(iv)=="string" then
			iv=self:resolveStyle(iv)
		end
		iv=tonumber(iv) or 0
	end
	self.axisInter=iv
	self:setShaderConstant("fAxisScaleWidth",Shader.CFLOAT4,1,offset,iv,count,0)
end

function UI.Chart.Axis:setLabels(generator,skip)
	self.labelGenerator=generator
	self.labelSkip=skip or 0
	if self.axisInter and generator then
		local lbs=""
		local iv=1
		while iv<=self.axisCount do
			local lb=generator(iv)
			iv+=(self.labelSkip+1)
			lbs=lbs..lb.."\t"
		end
		if #lbs>0 then
			if not self.label then
				self.label=TextField.new()
				self:addChild(self.label)
				self.label:setFont("label.font")
				self.label:setTextColor("label.color")
			end
			
			FontBase.TLF_TABLE = FontBase.TLF_TABLE or 0 --TEMPORAIRE : Gideros 24.9
			
			self.label:setLayout({ tabSpace=-self.axisInter, flags=FontBase.TLF_REF_LINETOP|FontBase.TLF_TABLE|FontBase.TLF_CENTER})
			self.label:setText(lbs)
		elseif self.label then
			self.label:removeFromParent()
			self.label=nil		
		end
	end
end

UI.Chart.Axis.Definition= {
	name="ChartAxis",
	icon="ui/icons/panel.png",
	class="UI.Chart.Axis",
	constructorArgs={ },
	properties={
		{ name="Orientation", type="number", setter=UI.Chart.setOrientation, },
	},
}

--Bar chart
UI.BarChart=Core.class(UI.Chart,function() return nil end)

UI.BarChart.Shader=Core.class(UI.Shader)
function UI.BarChart.Shader:init(params)
	self.params=params
	self.shader=self.overrideStandardShader(UI.BarChart.Shader,Shader.SHADER_PROGRAM_TEXTURE,0,function(spec)
		function spec.vertexShader(vVertex,vColor,vTexCoord) : Shader
			local vertex = hF4(vVertex,0.0,1.0)
			fTexCoord=vTexCoord
			fPos=vertex.xy
			return vMatrix*vertex
		end
		table.insert(spec.uniforms,{name="fBounds",type=Shader.CFLOAT4,sys=Shader.SYS_BOUNDS})
		table.insert(spec.varying,{name="fPos",type=Shader.CFLOAT2})
		table.insert(spec.uniforms,{name="fTextureInfo",type=Shader.CFLOAT4,sys=Shader.SYS_TEXTUREINFO})
		table.insert(spec.uniforms,{name="fBars",type=Shader.CFLOAT2,})
		table.insert(spec.uniforms,{name="fTex",type=Shader.CFLOAT2,})
		table.insert(spec.uniforms,{name="fBarTex",type=Shader.CTEXTURE,})
		table.insert(spec.uniforms,{name="fSelected",type=Shader.CFLOAT2,})
		table.insert(spec.uniforms,{name="barWidth",type=Shader.CFLOAT,})
		table.insert(spec.uniforms,{name="barMargin",type=Shader.CFLOAT,})
		table.insert(spec.uniforms,{name="colBack",type=Shader.CFLOAT4,})
		table.insert(spec.uniforms,{name="colBar",type=Shader.CFLOAT4,})
		table.insert(spec.uniforms,{name="colSel",type=Shader.CFLOAT4,})
		function spec.fragmentShader() : Shader
			local bw=barWidth
			if bw==0 then
				bw=(fBounds.z-fBounds.x)/fBars.x
			end
			local barn=floor(fPos.x/bw)
			local bard=fPos.x-bw*barn
			local frag=colBack
			if barn<fBars.x and bard>barMargin and bard<(bw-barMargin) then
				local bn=(barn+0.5)*fTextureInfo.z
				local t=texture2D(fTexture, hF2(bn,0.5)).r
				local sr=hF2(fTexCoord.x/fTextureInfo.x,1-fTexCoord.y/fTextureInfo.y)
				
				if sr.y<t then 
					frag=colBar
					if fSelected.x==barn then
						frag=colSel
					end
					local bc=(bard-barMargin)
					local bm=bw-2*barMargin
					local tx,ty
					if bc<fTex.x then
						tx=mix(0,fTex.y,bc/fTex.x)
					elseif bc>(bm-fTex.x) then
						tx=mix(1-fTex.y,1,(bc-bm+fTex.x)/fTex.x)
					else
						tx=mix(fTex.y,1-fTex.y,(bc-fTex.x)/(bm-2*fTex.x))
					end
					local pr=(t-sr.y)*(fBounds.w-fBounds.y)
					if pr<fTex.x then
						ty=mix(0,fTex.y,pr/fTex.x)
					else
						local bh=t*(fBounds.w-fBounds.y)
						ty=mix(fTex.y,0.5,(pr-fTex.x)/(bh-fTex.x))
					end
					local t2=texture2D(fBarTex, hF2(tx,ty))
					frag=frag*t2.r
				end
			end
			frag=fColor*frag
			if (frag.a==0.0) then discard() end
			return lF4(frag)
		end
	end)
end

function UI.BarChart:init()
	self:setNinePatch(0)
	self.shader=UI.BarChart.Shader.new({
		colBack="chart.colBackground",
		colBar="chart.colItem",
		colSel="chart.colItemSelected",
		barWidth="0s",
		barMargin=".1s",
	})
	self.shader:apply(self)
end

function UI.BarChart:updateStyle(fromParent)
	UI.Chart.updateStyle(self)
	self.shader:apply(self)
	local bTex=self:resolveStyle("chart.icBar")
	self:setTexture(bTex,1)
	self:setShaderConstant("fTex",Shader.CFLOAT2,1,
		(self:resolveStyle("chart.szBarCorner")),
		(self:resolveStyle("chart.szBarCornerTextureRatio")))
end

function UI.BarChart:updateView()
	UI.Chart.updateView(self)
	self:setColor(0xFFFFFF,1)
	local tw,th=self.texture:getSize()
	self:setTexture(self.texture)
	self:setShaderConstant("fBars",Shader.CFLOAT2,1,tw,th)
	self:setShaderConstant("fSelected",Shader.CFLOAT2,1,self.selected.x,self.selected.y)
end