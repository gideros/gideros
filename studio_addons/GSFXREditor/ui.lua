local font=TTFont.new("Oswald-Medium.ttf",10)

function setWaveformType(t)
	t=t&3
	FileSetField("WaveformType",t)
	for i=0,3 do BtnWaveFormTypes[i]:highlight(i==t) end
	Play()
end

function MakeUI()	
	DrawText(10, 10, 0xffffff, "GENERATOR")
	for i=0,7 do
		Button(5, 35+i*30, false, categories[i+1].name, 300+i,categories[i+1].cb)
	end
	DrawBar(110, 0, 2, 600, 0x000000);
	DrawText(120, 10, 0xffffff, "MANUAL SETTINGS");

	BtnWaveFormTypes={}
	BtnWaveFormTypes[0]=Button(130, 30, false, "SQUAREWAVE", 10,function () setWaveformType(0) end)
	BtnWaveFormTypes[1]=Button(250, 30, false, "SAWTOOTH", 11,function () setWaveformType(1) end)
	BtnWaveFormTypes[2]=Button(370, 30, false, "SINEWAVE", 12,function () setWaveformType(2) end)
	BtnWaveFormTypes[3]=Button(490, 30, false, "NOISE", 13,function () setWaveformType(3) end)


	DrawBar(5-1-1, 352-1-1, 102+2, 19+2, 0x000000)
	Button(5, 352, false, "RANDOMIZE", 40,Randomize)
	Button(5, 322, false, "MUTATE", 30,Mutate)

	Button(5, 412, false, "PLAY", 20,Play)
	Button(5, 472, false, "RELOAD", 14,Reload)
	Button(5, 502, false, "SAVE", 15,Save)

	local ypos=4
	local xpos=350

	-- Left side

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000)

	Slider(xpos, (ypos)*18, "BEnvAttack", false, "ATTACK TIME") ypos=ypos+1
	Slider(xpos, (ypos)*18, "BEnvSustain", false, "SUSTAIN TIME")  ypos=ypos+1
	Slider(xpos, (ypos)*18, "BEnvPunch", false, "SUSTAIN PUNCH") ypos=ypos+1
	Slider(xpos, (ypos)*18, "BEnvDecay", false, "DECAY TIME") ypos=ypos+1

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos)*18, "BBaseFreq", false, "START FREQUENCY"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "BFreqLimit", false, "MIN FREQUENCY"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "BFreqRamp", true, "SLIDE"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "BFreqDRamp", true, "DELTA SLIDE"); ypos=ypos+1

	Slider(xpos, (ypos)*18, "BVibStrength", false, "VIBRATO DEPTH"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "BVibSpeed", false, "VIBRATO SPEED"); ypos=ypos+1

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos)*18, "BArmMod", true, "CHANGE AMOUNT"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "BArmSpeed", false, "CHANGE SPEED"); ypos=ypos+1

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos)*18, "BDuty", false, "SQUARE DUTY"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "BDutyRamp", true, "DUTY SWEEP"); ypos=ypos+1

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos)*18, "BRepeatSpeed", false, "REPEAT SPEED"); ypos=ypos+1

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos)*18, "BPHAOffset", true, "PHASER OFFSET"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "BPHARamp", true, "PHASER SWEEP"); ypos=ypos+1

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos)*18, "BLPFFreq", false, "LP FILTER CUTOFF"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "BLPFRamp", true, "LP FILTER CUTOFF SWEEP"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "BLPFResonance", false, "LP FILTER RESONANCE"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "BHPFFreq", false, "HP FILTER CUTOFF"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "BHPFRamp", true, "HP FILTER CUTOFF SWEEP"); ypos=ypos+1

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	DrawBar(xpos-190, 4*18-5, 1, (ypos-4)*18, 0x0000000);
	DrawBar(xpos-190+299, 4*18-5, 1, (ypos-4)*18, 0x0000000);

	Slider(xpos+100, (ypos+2)*18, "MorphRate", false, "MORPH RATE");

	-- Right side

	ypos = 4;
	xpos += 300;

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos)*18, "MEnvAttack", false, "ATTACK TIME"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "MEnvSustain", false, "SUSTAIN TIME"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "MEnvPunch", false, "SUSTAIN PUNCH"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "MEnvDecay", false, "DECAY TIME"); ypos=ypos+1

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos)*18, "MBaseFreq", false, "END FREQUENCY","BBaseFreq"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "MFreqLimit", false, "MIN FREQUENCY",	"BFreqLimit"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "MFreqRamp", true, "SLIDE",				"BFreqRamp"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "MFreqDRamp", true, "DELTA SLIDE",		"BFreqDRamp"); ypos=ypos+1

	Slider(xpos, (ypos)*18, "MVibStrength", false, "VIBRATO DEPTH",	"BVibStrength"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "MVibSpeed", false, "VIBRATO SPEED",	"BVibSpeed"); ypos=ypos+1

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos)*18, "MArmMod", true, "CHANGE AMOUNT",		"BArmMod"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "MArmSpeed", false, "CHANGE SPEED",		"BArmSpeed"); ypos=ypos+1

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos)*18, "MDuty", false, "SQUARE DUTY",			"BDuty"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "MDutyRamp", true, "DUTY SWEEP",		"BDutyRamp"); ypos=ypos+1

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos)*18, "MRepeatSpeed", false, "REPEAT SPEED",	"BRepeatSpeed"); ypos=ypos+1

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos)*18, "MPHAOffset", true, "PHASER OFFSET",	"BPHAOffset"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "MPHARamp", true, "PHASER SWEEP",		"BPHARamp"); ypos=ypos+1
	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	Slider(xpos, (ypos)*18, "MLPFFreq", false, "LP FILTER CUTOFF",			"BLPFFreq"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "MLPFRamp", true, "LP FILTER CUTOFF SWEEP",		"BLPFRamp"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "MLPFResonance", false, "LP FILTER RESONANCE",	"BLPFResonance"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "MHPFFreq", false, "HP FILTER CUTOFF",			"BHPFFreq"); ypos=ypos+1
	Slider(xpos, (ypos)*18, "MHPFRamp", true, "HP FILTER CUTOFF SWEEP",		"BHPFRamp"); ypos=ypos+1

	DrawBar(xpos-190, ypos*18-5, 300, 2, 0x0000000);

	DrawBar(xpos-190, 4*18-5, 1, (ypos-4)*18, 0x0000000);
	DrawBar(xpos-190+299, 4*18-5, 1, (ypos-4)*18, 0x0000000);
	
	setWaveformType(FileGetField("WaveformType"))
end

function ReloadUI()
	setWaveformType(FileGetField("WaveformType"))
	for _,v in ipairs(Sliders) do v:setbar() end
end

function DrawText(x,y,c,t,ra)
	local t=TextField.new(font,t,{ flags=FontBase.TLF_REF_TOP|FontBase.TLF_NOWRAP|(ra or 0) })
	t:setTextColor(c)
	t:setPosition(x,y)
	stage:addChild(t)
	return t
end

function DrawBar(x,y,w,h,c)
 local p=Pixel.new(c,1,w,h)
 p:setPosition(x,y)
 stage:addChild(p)
 return p
end

function Button(x,y,e,t,id,cb)
	local b1=DrawBar(x-1, y-1, 102, 19, 0x000000);
	local b2=DrawBar(x, y, 100, 17, 0x808080);
	local t=DrawText(x+5, y+2, 0x000000, t);
	t.b1=b1
	t.b2=b2
	t.highlight=function (self,e)
		local color1=0x000000;
		local color2=0x808080;
		local color3=0x000000;
		if(e) then
			color1=0x808080;
			color2=0x988070;
			color3=0xffffff;
		end
		self.b1:setColor(color1)
		self.b2:setColor(color2)
		self:setTextColor(color3)		
	end
	t.cb=cb
	b1:addEventListener(Event.MOUSE_DOWN,function (self,e)
		if self:hitTestPoint(e.x,e.y) then cb() end
	end,b1)
	return t
end

local function sliderSetBar(self)
	local value=FileGetField(self.field)
	local ival=(value*99)//1;
	if(self.bipolar) then
		ival=(value*49.5+49.5)//1;
	end
	self.i1:setWidth(ival)
	self.i2:setX(self.x+ival) self.i2:setWidth(100-ival)		
	self.i3:setX(self.x+ival)
end

local function sliderUpdate(self,e)
	if self:hitTestPoint(e.x,e.y) then
		local m=self:globalToLocal(e.x,e.y)-1
		if self.bipolar then
			m=math.max(math.min(1.0,(m-49.5)/49.5),-1.0)
		else
			m=math.max(math.min(1.0,m/99),0.0)
		end
		FileSetField(self.field,m)
		self:setbar()
		Play()
	end
end

Sliders={}
function Slider(x,y,v,bipolar,text,clone)
	local tgt=DrawBar(x-1, y, 102, 10, 0x000000);
	if(bipolar) then
		DrawBar(x+50, y-1, 1, 3, 0x000000);
		DrawBar(x+50, y+8, 1, 3, 0x000000);
	end
	tgt.x=x
	tgt.i1=DrawBar(x, y+1, 0, 8, 0xF0C090);
	tgt.i2=DrawBar(x, y+1, 100, 8, 0x807060);
	tgt.i3=DrawBar(x, y+1, 1, 8, 0xFFFFFF);
	local tcol=0xffffff;
--[[	if(g_RetroSFXVoice.m_Voice.nWaveformType!=0 && (&value==&g_RetroSFXVoice.m_Voice.FXBaseParams.fDuty || &value==&g_RetroSFXVoice.m_Voice.FXBaseParams.fDutyRamp))
		tcol=0x808080; ]] 
	DrawText(x-4, y-1, tcol, text, FontBase.TLF_RIGHT);
	tgt.bipolar=bipolar
	tgt.field=v
	tgt.setbar=sliderSetBar
	tgt.update=sliderUpdate
	tgt:addEventListener(Event.MOUSE_DOWN,tgt.update,tgt)
	tgt:addEventListener(Event.MOUSE_MOVE,tgt.update,tgt)
	tgt:setbar()
	table.insert(Sliders,tgt)
end
