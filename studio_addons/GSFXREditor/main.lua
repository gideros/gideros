function ReloadFile()
	data=nil
	pcall(function ()
		local f=io.open(Studio.DATA.editFile,"rb")
		data=f:read("*a")
		f:close()
	end)
	if not data or #data~=204 then 
		data=string.char(0,1).."XF"..(string.char(0):rep(200)) 
		print(#data)
		FileReset()
	end
end

ReloadFile()

function Reload()
	ReloadFile()
	ReloadUI()
end

function Save()
	if not Studio.DATA.editFile then
		local textInputDialog = TextInputDialog.new("Save file", "Enter a file name for this new sound", "sound.sfx", "Cancel", "OK")
		local function onComplete(event)
			print(event.text, event.buttonIndex, event.buttonText)
			if event.buttonIndex then
				local ext=event.text:sub(-4)
				if ext~=".sfx" then event.text=event.text+".sfx" end
				local fl=Studio.addFile(nil,event.text)
				if fl then Studio.DATA.editFile=fl.source end
				Save()
			end
		end
		textInputDialog:addEventListener(Event.COMPLETE, onComplete)
		textInputDialog:show()
	end
	FilePrepare()
	pcall(function ()
		local f=io.open(Studio.DATA.editFile,"wb")
		if f then
			f:write(data)
			f:close()
		end
	end)
end


SND_CHANNEL=nil
function Play()
	FilePrepare()
	if SND_CHANNEL then SND_CHANNEL:stop() end
	local f=io.open("|T|test.sfx","wb")
	f:write(data)
	f:close()
	local s=Sound.new("|T|test.sfx")
	SND_CHANNEL=s:play()
end
function frnd(v) return Core.random(0)*v end
function rnd(i) return Core.random(0,i+1)-1 end
categories={
 { name="PICKUP/COIN", cb=function ()
 	FileReset()
	FileSetField("BBaseFreq",0.4+frnd(0.5))
	FileSetField("BEnvAttack",0.0)
	FileSetField("BEnvSustain",frnd(0.1))
	FileSetField("BEnvDecay",0.1+frnd(0.4))
	FileSetField("BEnvPunch",0.3+frnd(0.3))
	if(rnd(1)>0) then
		FileSetField("BArmSpeed",0.5+frnd(0.2))
		FileSetField("BArmMod",0.2+frnd(0.4))
	end
	ReloadUI()
	end},
	{ name="LASER/SHOOT", cb=function ()
 	FileReset()
	FileSetField("WaveformType",rnd(2))
	if(FileGetField("WaveformType")==2 and (rnd(1)>0)) then
		FileSetField("WaveformType",rnd(1))
	end
	FileSetField("BBaseFreq",0.5+frnd(0.5))
	FileSetField("BFreqLimit",math.min(0.2,FileGetField("BBaseFreq")-0.2-frnd(0.6)))
	FileSetField("BFreqRamp",-0.15-frnd(0.2))
	if(rnd(2)==0) then
		FileSetField("BBaseFreq",0.3+frnd(0.6))
		FileSetField("BFreqLimit",frnd(0.1))
		FileSetField("BFreqRamp",-0.35-frnd(0.3))
	end
	if(rnd(2)==1) then
		FileSetField("BDuty",frnd(0.5))
		FileSetField("BDutyRamp",frnd(0.2))
	else
		FileSetField("BDuty",0.4+frnd(0.5))
		FileSetField("BDutyRamp",-frnd(0.7))
	end
	FileSetField("BEnvAttack",0.0)
	FileSetField("BEnvSustain",0.1+frnd(0.2))
	FileSetField("BEnvDecay",frnd(0.4))
	if (rnd(1)>0) then
		FileSetField("BEnvPunch",frnd(0.3))
	end
	if (rnd(2)==0) then
		FileSetField("BPHAOffset",frnd(0.2))
		FileSetField("BPHARamp",frnd(0.2))
	end
	if (rnd(1)>0) then
		FileSetField("BHPFFreq",frnd(0.3))
	end
	ReloadUI()
	end},
	{ name="EXPLOSION", cb=function ()
 	FileReset()
	FileSetField("WaveformType",3)
	if (rnd(1)>0) then
		FileSetField("BBaseFreq",0.1+frnd(0.4))
		FileSetField("BFreqRamp",-0.1+frnd(0.4))
	else
		FileSetField("BBaseFreq",0.2+frnd(0.7))
		FileSetField("BFreqRamp",-0.2-frnd(0.2))
	end
	FileSetField("BBaseFreq",FileGetField("BBaseFreq")^2)
	if (rnd(4)==0) then
		FileSetField("BFreqRamp",0)
	end
	if (rnd(2)==0) then
		FileSetField("BRepeatSpeed",0.3+frnd(0.5))
	end
	FileSetField("BEnvAttack",0.0)
	FileSetField("BEnvSustain",0.1+frnd(0.3))
	FileSetField("BEnvDecay",frnd(0.5))
	if (rnd(1)==0) then
		FileSetField("BPHAOffset",-0.3+frnd(0.9))
		FileSetField("BPHARamp",-frnd(0.3))
	end
	FileSetField("BEnvPunch",0.2+frnd(0.6))
	if (rnd(1)>0) then
		FileSetField("BVibStrength",frnd(0.7))
		FileSetField("BVibSpeed",frnd(0.6))
	end
	if (rnd(2)==0) then
		FileSetField("BArmSpeed",0.6+frnd(0.3))
		FileSetField("BArmMod",0.8-frnd(1.6))
	end
	ReloadUI()
	end},
	{ name="POWERUP", cb=function ()
 	FileReset()
	if (rnd(1)>0) then
		FileSetField("WaveformType",1)
	else
		FileSetField("BDuty",frnd(0.6))
	end
	if (rnd(1)>0) then
		FileSetField("BBaseFreq",0.2+frnd(0.3))
		FileSetField("BFreqRamp",0.1+frnd(0.4))
		FileSetField("BRepeatSpeed",0.4+frnd(0.4))
	else
		FileSetField("BBaseFreq",0.2+frnd(0.3))
		FileSetField("BFreqRamp",0.05+frnd(0.2))
		if (rnd(1)>0) then
			FileSetField("BVibStrength",frnd(0.7))
			FileSetField("BVibSpeed",frnd(0.6))
		end
	end
	FileSetField("BEnvAttack",0.0)
	FileSetField("BEnvSustain",frnd(0.4))
	FileSetField("BEnvDecay",0.1+frnd(0.4))
	ReloadUI()
	end},
	{ name="HIT/HURT", cb=function ()
 	FileReset()
	FileSetField("WaveformType",rnd(2))
	if(FileGetField("WaveformType")==2) then
		FileSetField("WaveformType",3)
	end
	if(FileGetField("WaveformType")==0) then
		FileSetField("BDuty",frnd(0.6))
	end
	FileSetField("BBaseFreq",0.2+frnd(0.6))
	FileSetField("BFreqRamp",-0.3-frnd(0.4))
	FileSetField("BEnvAttack",0.0)
	FileSetField("BEnvSustain",0.1)
	FileSetField("BEnvDecay",0.1+frnd(0.2))
	if (rnd(1)>0) then
		FileSetField("BHPFFreq",frnd(0.3))
	end
	ReloadUI()
	end},
	{ name="JUMP", cb=function ()
 	FileReset()
	FileSetField("WaveformType",0)
	FileSetField("BDuty",frnd(0.6))
	FileSetField("BBaseFreq",0.3+frnd(0.3))
	FileSetField("BFreqRamp",0.1+frnd(0.2))
	FileSetField("BEnvAttack",0.0)
	FileSetField("BEnvSustain",0.1+frnd(0.3))
	FileSetField("BEnvDecay",0.1+frnd(0.2))
	if (rnd(1)>0) then
		FileSetField("BHPFFreq",frnd(0.3))
	end
	if (rnd(1)>0) then
		FileSetField("BLPFFreq",1.0-frnd(0.6))
	end
	ReloadUI()
	end},
	{ name="BLIP/SELECT", cb=function ()
 	FileReset()
	FileSetField("WaveformType",rnd(1))
	if(FileGetField("WaveformType")==0) then
		FileSetField("BDuty",frnd(0.6))
	end
	FileSetField("BBaseFreq",0.2+frnd(0.4))
	FileSetField("BEnvAttack",0.0)
	FileSetField("BEnvSustain",0.1+frnd(0.1))
	FileSetField("BEnvDecay",frnd(0.2))
	FileSetField("BHPFFreq",0.1)
	ReloadUI()
	end},
	{ name="ROBOTRON", cb=function ()
 	DoRandomize()
	FileSetField("MorphRate",math.max(FileGetField("MorphRate"),0.25))
	FileSetField("BEnvSustain",math.max(FileGetField("BEnvSustain"),0.5))
	FileSetField("BEnvDecay",math.max(FileGetField("BEnvDecay"),0.5))
	FileSetField("BRepeatSpeed",math.max(FileGetField("BRepeatSpeed"),0.5))
	ReloadUI()
	end},
}

local function prnd(n) return frnd(1.0)^n end

function DoRandomize()
	FileSetField("WaveformType",rnd(3))
	FileSetField("BBaseFreq",prnd(2.0))
	if (rnd(1)>0) then
		FileSetField("BBaseFreq",math.min(1,prnd(3.0)+0.5))
	end
	FileSetField("BFreqLimit",0)
	FileSetField("BFreqRamp",prnd(5))
	if FileGetField("BBaseFreq")>0.7 and FileGetField("BFreqRamp")>0.2 then
		FileSetField("BFreqRamp",-FileGetField("BFreqRamp"))
	end
	if FileGetField("BBaseFreq")<0.2 and FileGetField("BFreqRamp")<-0.05 then
		FileSetField("BFreqRamp",-FileGetField("BFreqRamp"))
	end
	FileSetField("BFreqDRamp",prnd(3))
	FileSetField("BDuty",frnd(1))
	FileSetField("BDutyRamp",prnd(3))
	FileSetField("BVibStrength",prnd(3))
	FileSetField("BVibSpeed",frnd(1))
	FileSetField("BVibDelay",frnd(1))
	FileSetField("BEnvAttack",prnd(3))
	FileSetField("BEnvSustain",prnd(2))
	FileSetField("BEnvDecay",0.5+frnd(0.5))
	FileSetField("BEnvPunch",frnd(0.8)^2)
	if(FileGetField("BEnvAttack")+FileGetField("BEnvSustain")+FileGetField("BEnvDecay"))<0.2 then
		FileSetField("BEnvSustain",FileGetField("BEnvSustain")+0.2+frnd(0.3))
		FileSetField("BEnvDecay",FileGetField("BEnvDecay")+0.2+frnd(0.3))
	end
	FileSetField("BLPFResonance",frnd(1))
	FileSetField("BLPFFreq",1-prnd(3))
	FileSetField("BLPFRamp",prnd(3))
	if FileGetField("BLPFFreq")<0.1 and FileGetField("BLPFRamp")<-0.05 then
		FileSetField("BLPFRamp",-FileGetField("BLPFRamp"))
	end
	FileSetField("BHPFFreq",prnd(5))
	FileSetField("BHPFRamp",prnd(5))
	FileSetField("BPHAOffset",prnd(3))
	FileSetField("BPHARamp",prnd(3))
	FileSetField("BRepeatSpeed",frnd(1))
	FileSetField("BArmSpeed",frnd(1))
	FileSetField("BArmMod",frnd(1))

	-- Morph

	if (FileGetField("BRepeatSpeed")==0) then
		FileSetField("MorphRate",0)
	else
		FileSetField("MorphRate",math.max(0.0,frnd(2.0)-1))
	end

	FileSetField("MBaseFreq",prnd(2.0))
	if (rnd(1)>0) then
		FileSetField("MBaseFreq",math.min(prnd(3.0)+0.5))
	end
	FileSetField("MFreqLimit",0)
	FileSetField("MFreqRamp",prnd(5))
	if FileGetField("MBaseFreq")>0.7 and FileGetField("MFreqRamp")>0.2 then
		FileSetField("MFreqRamp",-FileGetField("MFreqRamp"))
	end
	if FileGetField("MBaseFreq")<0.2 and FileGetField("MFreqRamp")<-0.05 then
		FileSetField("MFreqRamp",-FileGetField("MFreqRamp"))
	end
	FileSetField("MFreqDRamp",prnd(3))
	FileSetField("MDuty",frnd(1))
	FileSetField("MDutyRamp",prnd(3))
	FileSetField("MVibStrength",prnd(3))
	FileSetField("MVibSpeed",frnd(1))
	FileSetField("MVibDelay",frnd(1))
	FileSetField("MEnvAttack",prnd(3))
	FileSetField("MEnvSustain",prnd(2))
	FileSetField("MEnvDecay",frnd(0.1))
	FileSetField("MEnvPunch",frnd(0.8)^2)
	if(FileGetField("MEnvAttack")+FileGetField("MEnvSustain")+FileGetField("MEnvDecay"))<0.2 then
		FileSetField("MEnvSustain",FileGetField("MEnvSustain")+0.2+frnd(0.3))
		FileSetField("MEnvDecay",FileGetField("MEnvDecay")+0.2+frnd(0.3))
	end
	FileSetField("MLPFResonance",frnd(1))
	FileSetField("MLPFFreq",1-prnd(3))
	FileSetField("MLPFRamp",prnd(3))
	if FileGetField("MLPFFreq")<0.1 and FileGetField("MLPFRamp")<-0.05 then
		FileSetField("MLPFRamp",-FileGetField("MLPFRamp"))
	end
	FileSetField("MHPFFreq",prnd(5))
	FileSetField("MHPFRamp",prnd(5))
	FileSetField("MPHAOffset",prnd(3))
	FileSetField("MPHARamp",prnd(3))
	FileSetField("MRepeatSpeed",frnd(1))
	FileSetField("MArmSpeed",frnd(1))
	FileSetField("MArmMod",frnd(1))
end

function Randomize()
 	DoRandomize()
	ReloadUI()
end

function Mutate()
	local MFields= {
		"BBaseFreq",		"BFreqRamp",		"BFreqDRamp",		"BDuty",
		"BDutyRamp",		"BVibStrength",		"BVibSpeed",		"BVibDelay",
		"BEnvAttack",		"BEnvSustain",		"BEnvDecay",		"BEnvPunch",
		"BLPFResonance",		"BLPFFreq",		"BLPFRamp",		"BHPFFreq",
		"BHPFRamp",		"BPHAOffset",		"BPHARamp",		"BRepeatSpeed",
		"BArmSpeed",		"BArmMod",
		"MBaseFreq",		"MFreqRamp",		"MFreqDRamp",		"MDuty",
		"MDutyRamp",		"MVibStrength",		"MVibSpeed",		"MVibDelay",
		"MEnvAttack",		"MEnvSustain",		"MEnvDecay",		"MEnvPunch",
		"MLPFResonance",		"MLPFFreq",		"MLPFRamp",		"MHPFFreq",
		"MHPFRamp",		"MPHAOffset",		"MPHARamp",		"MRepeatSpeed",
		"MArmSpeed",		"MArmMod"}
	local bipolars={FreqRamp=true,FreqDRamp=true,ArmMod=true,DutyRamp=true,
				PHAOffset=true,PHARamp=true,LPFRamp=true,HPFRamp=true}
	for _,f in ipairs(MFields) do
		if (rnd(1)>0) then
			local lbnd=0
			if bipolars[f:sub(2)] then lbnd=-1 end
			FileSetField(f,math.max(math.min(1.0,FileGetField(f)+frnd(0.1)-0.05),lbnd))
		end
	end
	ReloadUI()
end

--UI
application:setBackgroundColor(0x404040)
MakeUI()