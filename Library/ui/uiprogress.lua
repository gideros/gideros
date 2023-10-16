UI.Progress=Core.class(UI.Panel,function() return nil end)

function UI.Progress:setProgress(p)
	self.progress=p
end

function UI.Progress:getProgress()
	return self.progress
end

function UI.Progress:setIndeterminate(ind)
	self.indeterminate=ind
	if ind then
		UI.Control.onEnterFrame[self]=self
	else
		UI.Control.onEnterFrame[self]=nil
		self:setProgress(self.progress)
	end
end

function UI.Progress:getIndeterminate(ind)
	return self.indeterminate
end

function UI.Progress:onEnterFrame()
end

UI.CircularProgress=Core.class(UI.Progress,function() return nil end)
function UI.CircularProgress:init()
	self:setBaseStyle("progress.styCircular")
end
function UI.CircularProgress:updateStyle(...)
	UI.Panel.updateStyle(self,...)
	self:updateShader()
end

function UI.CircularProgress:setProgress(p)
	UI.Progress.setProgress(self,p)
	self:updateShader()
end

function UI.CircularProgress:onEnterFrame()
	if self.indeterminate then
		self:updateShader()
	end
end

function UI.CircularProgress:updateShader()
	local s,e=0,self.progress or 0
	if self.indeterminate then
		local t=os:timer()*2
		s=(t*.4)%1--(math.sin(t)+1)/2
		e=s+.1+(math.sin(t*1.5)+1)*0.2
		if e>1 then e-=1 end
	end
	self:setShaderConstant("numAngleStart",Shader.CFLOAT,1,s)
	self:setShaderConstant("numAngleEnd",Shader.CFLOAT,1,e)
end

UI.Progress.Definition= {
	name="Progress",
	icon="ui/icons/checkbox.png",
	class="UI.Progress",
	constructorArgs={ },
	properties={
		{ name="Progress", type="number" },
		{ name="Indeterminate", type="any" },
	},
}

UI.ProgressBar=Core.class(UI.Progress,function() return nil end)
UI.ProgressBar.Template={
	class="UI.ProgressBar",
	layoutModel={ columnWeights={1}, rowWeights={1} },
	BaseStyle="progress.styBar",
	StateStyle="progressbar.styNormal",
	children={
		{ class="UI.Label", Text="", name="label", BaseStyle="progress.styBarText", layout={},},
	}}
	
function UI.ProgressBar:init(textFormat)
	self.textFormat=textFormat
	UI.BuilderSelf(UI.ProgressBar.Template,self)
end
function UI.ProgressBar:updateStyle(...)
	UI.Panel.updateStyle(self,...)
	self:updateShader()
end
function UI.ProgressBar:setFlags(changes)
	UI.Panel.setFlags(self,changes)
	local s=if self._flags.disabled then "progress.styDisabled" else "progress.styNormal" 
	self:setStateStyle(s)
end
function UI.ProgressBar:updateLabel()
	local ft=type(self.textFormat)
	if ft=="function" then
		self.label:setText(self.textFormat(self.progress))
	elseif ft=="string" then
		self.label:setText(self.textFormat:format((self.progress or 0)*100))
	end
end

function UI.ProgressBar:setTextFormat(p)	
	self.textFormat=p
	if not p then self.label:setText("") end
	self:updateLabel()
end

function UI.ProgressBar:setProgress(p)
	UI.Progress.setProgress(self,p)
	self:updateShader()
	self:updateLabel()
end

function UI.ProgressBar:updateShader()
	local s,e=0,self.progress or 0
	if self.indeterminate then
		local t=os:timer()*2
		s=(math.sin(t)+1)/2
		e=s+.1+(math.sin(t*1.5)+1)*0.2
		e=e><1
	end
	self:setShaderConstant("fRatio",Shader.CFLOAT2,1,s,e)
end
	
function UI.ProgressBar:onEnterFrame()
	if self.indeterminate then
		self:updateShader()
	end
end

UI.ProgressBar.Definition= {
	name="Progress",
	icon="ui/icons/checkbox.png",
	class="UI.ProgressBar",
	super=UI.Progress,
	constructorArgs={ "TextFormat" },
	properties={
		{ name="TextFormat", type="variable" },
	},
}

UI.ProgressBeads=Core.class(UI.Progress,function() return nil end)
function UI.ProgressBeads:init()
	self:setBaseStyle("progress.styBeads")
	self.particles=Particles.new()
	self:addChild(self.particles)
	self.pcount=0
	self:addEventListener(Event.LAYOUT_RESIZED,self.onResized,self)
	self:onResized()
end

function UI.ProgressBeads:onResized()
	local sw,sh=self:getDimensions()
	local pw=self:resolveStyle("progress.szBead")
	self.pspan=sw-pw
	self.psize=pw
	self.pmargin=self:resolveStyle("progress.szBeadMargin")
	self.particles:setPosition(pw/2,self:getHeight()-pw/2)
end
	
function UI.ProgressBeads:updateStyle(...)
	UI.Panel.updateStyle(self,...)
	self:onResized()
	UI.Utils.colorTransform(self.particles,"progress.colBead")
	local tx=self.particles:resolveStyle("progress.icBead")
	if tx then
		self.particles:setTexture(tx)
	else
		self.particles:clearTexture()
	end
	for n=1,self.pcount do
		self.particles:setParticleSize(n,self.psize)
	end
end

function UI.ProgressBeads:setProgress(p)
	UI.Progress.setProgress(self,p)
	self.particles:removeParticles()
	self.pcount=0
	self.nextTick=nil
end

function UI.ProgressBeads:setIndeterminate(ind)
	if ind then -- Leave anymation running, we will handle reset in onEnterFrame
		UI.Progress.setIndeterminate(self,ind)
	else
		self.indeterminate=nil
	end
end


local timings={ slide=.3, decay=.3, hold=.2, restart=1}
function UI.ProgressBeads:onEnterFrame()
	local nbeads=tonumber(self.indeterminate) or 0
	if nbeads>0 or self.pcount>0 then
		local nstep=self.nextTick or 0
		local ctick=os:timer()
		if ctick>nstep then
			if self.pcount<nbeads then
				local pw=self:resolveStyle("progress.szBead")
				self.particles:addParticles({{
					x=0,y=0,size=pw,
				}})
				self.pcount+=1
				if self.pcount==nbeads then
					self.nextTick=ctick+timings.slide+timings.decay+timings.hold
				else
					self.nextTick=ctick+timings.slide
				end
			else
				self.particles:removeParticles()
				self.pcount=0
				self.nextTick=ctick+timings.restart
				if not self.indeterminate then
					UI.Progress.setIndeterminate(self,nil)
				end
			end
		else		
			local nt=nstep-ctick
			local nal
			if self.pcount<nbeads then
				nal=1
				nt=((5*(timings.slide-nt))<>0)><1
			else
				nal=(nt><timings.decay)/timings.decay
				nt=((5*((timings.slide+timings.decay+timings.hold)-nt))<>0)><1
			end
			for n=1,self.pcount do
				self.particles:setParticleColor(n,0xFFFFFF,nal)
				local mt=if n==self.pcount then nt else 1
				self.particles:setParticlePosition(n,mt*(self.pspan-(self.psize+self.pmargin)*(n-1)),0)
			end
		end
	end
end


UI.ProgressBeads.Definition= {
	name="Progress",
	icon="ui/icons/checkbox.png",
	class="UI.ProgressBeads",
	super=UI.Progress,
	constructorArgs={ "TextFormat" },
	properties={
		{ name="TextFormat", type="variable" },
	},
}
