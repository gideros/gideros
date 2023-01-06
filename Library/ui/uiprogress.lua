UI.Progress=Core.class(UI.Panel,function() return nil end)

function UI.Progress:setProgress(p)
	self.progress=p
end

function UI.Progress:getProgress()
	return self.progress
end

UI.CircularProgress=Core.class(UI.Progress,function() return nil end)
function UI.CircularProgress:init()
	self:setBaseStyle("progress.styCircular")
end
function UI.CircularProgress:updateStyle(...)
	UI.Panel.updateStyle(self,...)
	self:setShaderConstant("numAngleEnd",Shader.CFLOAT,1,self.progress or 0)
end

function UI.CircularProgress:setProgress(p)
	UI.Progress.setProgress(self,p)
	self:setShaderConstant("numAngleEnd",Shader.CFLOAT,1,p)
end

UI.Progress.Definition= {
	name="Progress",
	icon="ui/icons/checkbox.png",
	class="UI.Progress",
	constructorArgs={ },
	properties={
		{ name="Progress", type="number" },
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
	self:setShaderConstant("fRatio",Shader.CFLOAT,1,self.progress)
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
		self.label:setText(self.textFormat:format(self.progress*100))
	end
end

function UI.ProgressBar:setTextFormat(p)	
	self.textFormat=p
	if not p then self.label:setText("") end
	self:updateLabel()
end

function UI.ProgressBar:setProgress(p)
	UI.Progress.setProgress(self,p)
	self:setShaderConstant("fRatio",Shader.CFLOAT,1,p)
	self:updateLabel()
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