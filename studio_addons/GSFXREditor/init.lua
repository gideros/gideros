FILEFMT={
 "IVersion",
 "IWaveformType",
 "FSoundVol",
 "FMorphRate",
 "ILengthInSamples",
 "FBBaseFreq","FBFreqLimit","FBFreqRamp","FBFreqDRamp","FBDuty","FBDutyRamp",
 "FBVibStrength","FBVibSpeed","FBVibDelay",
 "FBEnvAttack","FBEnvSustain","FBEnvDecay","FBEnvPunch",
 "FBLPFResonance","FBLPFFreq","FBLPFRamp","FBHPFFreq","FBHPFRamp",
 "FBPHAOffset","FBPHARamp","FBRepeatSpeed","FBArmSpeed","FBArmMod",
 "FMBaseFreq","FMFreqLimit","FMFreqRamp","FMFreqDRamp","FMDuty","FMDutyRamp",
 "FMVibStrength","FMVibSpeed","FMVibDelay",
 "FMEnvAttack","FMEnvSustain","FMEnvDecay","FMEnvPunch",
 "FMLPFResonance","FMLPFFreq","FMLPFRamp","FMHPFFreq","FMHPFRamp",
 "FMPHAOffset","FMPHARamp","FMRepeatSpeed","FMArmSpeed","FMArmMod",
}

local FileFields={}
for n,v in ipairs(FILEFMT) do
	local fn=v:sub(2)
	local ft=v:sub(1,1)
	local fo=(n-1)*4+1
	FileFields[fn]={ type=ft, offset=fo }
end


function PackIEEE754(number)
    if number == 0 then
        return 0x00, 0x00, 0x00, 0x00
    elseif number ~= number then
        return 0xFF, 0xFF, 0xFF, 0xFF
    else
        local sign = 0x00
        if number < 0 then
            sign = 0x80
            number = -number
        end
        local mantissa, exponent = math.frexp(number)
        exponent = exponent + 0x7F
        if exponent <= 0 then
            mantissa = math.ldexp(mantissa, exponent - 1)
            exponent = 0
        elseif exponent > 0 then
            if exponent >= 0xFF then
                return 0x00,0x00,0x80,sign + 0x7F
            elseif exponent == 1 then
                exponent = 0
            else
                mantissa = mantissa * 2 - 1
                exponent = exponent - 1
            end
        end
        mantissa = math.floor(math.ldexp(mantissa, 23) + 0.5)
        return mantissa % 0x100,
                math.floor(mantissa / 0x100) % 0x100,
				(exponent % 2) * 0x80 + math.floor(mantissa / 0x10000),
				sign + math.floor(exponent / 2)
    end
end
function UnpackIEEE754(b4,b3,b2,b1)
    local exponent = (b1 % 0x80) * 0x02 + math.floor(b2 / 0x80)
    local mantissa = math.ldexp(((b2 % 0x80) * 0x100 + b3) * 0x100 + b4, -23)
    if exponent == 0xFF then
        if mantissa > 0 then
            return 0 / 0
        else
            mantissa = math.huge
            exponent = 0x7F
        end
    elseif exponent > 0 then
        mantissa = mantissa + 1
    else
        exponent = exponent + 1
    end
    if b1 >= 0x80 then
        mantissa = -mantissa
    end
    return math.ldexp(mantissa, exponent - 0x7F)
end

function FileGetField(fi)
	local f=FileFields[fi]
	if not f then error("No Field "..fi) end
	local a,b,c,d=data:byte(f.offset,f.offset+3)
	if f.type=="I" then return a+(b*256)+(c*65536)+(d*256*65536)
	elseif f.type=="F" then return UnpackIEEE754(a,b,c,d)
	end
	return nil	
end

function FileSetField(fi,v)
	local f=FileFields[fi]
	if not f then error("No Field "..fi) end
	local a,b,c,d=0
	if f.type=="I" then 
		a=v&0xFF
		b=(v>>8)&0xFF
		c=(v>>16)&0xFF
		d=(v>>24)&0xFF
	elseif f.type=="F" then 
		a,b,c,d=PackIEEE754(v)
	end
	data=data:sub(1,f.offset-1)..string.char(a,b,c,d)..data:sub(f.offset+4)
end

function FileReset()
	for n,f in pairs(FileFields) do
		if f.type=="F" then
			FileSetField(n,0.0)
		end
	end
	FileSetField("WaveformType",0)
	FileSetField("SoundVol",0.5)
	FileSetField("BBaseFreq",0.3)
	FileSetField("BEnvSustain",0.3)
	FileSetField("BEnvDecay",0.4)
	FileSetField("BLPFFreq",1.0)
	FileSetField("BArmSpeed",1.0)
	FileSetField("MBaseFreq",0.3)
	FileSetField("MLPFFreq",1.0)
	FileSetField("MArmSpeed",1.0)
end

function FilePrepare()
	el=FileGetField("BEnvAttack")^2
	el=el+FileGetField("BEnvSustain")^2
	el=el+FileGetField("BEnvDecay")^2
	FileSetField("LengthInSamples",100000*el)
end