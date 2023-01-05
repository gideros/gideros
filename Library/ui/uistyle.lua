--!NEEDS:uiinit.lua
--!NEEDS:uiborder.lua
--!NEEDS:uicolor.lua

if _PRINTER then print("uistyle.lua") end

local debug = _PRINTER
if debug then print("UI.Style","debug !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!") end

UI.Default=UI.Default or {}

local targetTiny		=4.5
local targetTab			=8
local targetMonitor		=12

local dpi=application:getScreenDensity() or 120--326 --96
local diag=(application:getDeviceHeight()^2+application:getDeviceWidth()^2)^0.5/dpi
local ls=application:getLogicalScaleX()
local tgtdpi=120 --96
local detectedMode="monitor"
local platform=application:getDeviceInfo()
local DesktopPlatforms={
	WinRT=true,
	MacOS=true,
}
if diag>=targetMonitor or DesktopPlatforms[platform] 	then tgtdpi=120 detectedMode="monitor"
elseif diag>=targetTab	then tgtdpi=180 detectedMode="tablet" --240
elseif diag>=targetTiny	then tgtdpi=240 detectedMode="phone" --360
else tgtdpi=360 detectedMode="tiny" --360
end

tgtdpi=UI.Default.TargetDpi or tgtdpi

local zoom=(dpi/tgtdpi)/ls
local fzoom=20
if debug then print("UI.Style","application:getScreenDensity()",application:getScreenDensity(),"diag",diag,"ls",ls,"dpi",dpi,"tgtdpi",tgtdpi,"zoom",zoom,fzoom,"platform",platform,"mode",detectedMode) end

UI.Style={}
setmetatable(UI.Style,{
	--[[
	__index=function(t,n)
		if n=="__Reference" then return end
		assert(type(n)=="string","Style index must be a string, not "..type(n))
		assert(not n:find("%."),"Styles can't contain dots")
		if n:lower()==n then 
			t[n]={}
			return t[n] 
		end
	end,]]
	__newindex=function(t,n,v) --Unroll value if a table and if key is all lowercase
		assert(type(n)=="string","Style index must be a string, not "..type(n))
		if type(v)=="table" and not v.__classname and n:lower()==n and not n:find("%.") and not (n:sub(1,1)=="_") then 
			for k,v in pairs(v) do
				t[n.."."..k]=v
			end
		else
			rawset(t,n,v)
		end
	end,
})
UI.Style.__index=UI.Style
UI.Style._style={}
UI.Style._style.__index=UI.Style._style
setmetatable(UI.Style._style,UI.Style)

function UI.Style.getScreenProperties() return diag,targetTiny,targetTab,targetMonitor,dpi,tgtdpi,ls,zoom,detectedMode end

function UI.Style:setDefault(style)
	--Unlink current style from root style
	local m=self._style
	while true do
		local mm=getmetatable(m)
		if not mm then break end
		if mm==UI.Style then
			setmetatable(m,nil)
			break
		end
		m=mm
	end
	
	style=style or {}
	table.clear(self._style)
	table.clone(style,self._style)
	self._style.__index=self._style
	setmetatable(self._style,getmetatable(style))
	
	--Relink styles
	m=self._style
	while true do
		local mm=getmetatable(m)
		if not mm then 
			setmetatable(m,UI.Style)
			break
		end
		m=mm
	end

	if UI.Default.styleCustomSizes then
		UI.Default.styleCustomSizes(self._style)
	end
	if UI.Control then
		UI.Control.DRAG_THRESHOLD=stage:resolveStyle("szDragThreshold",self._style)
		UI.Control.WHEEL_DISTANCE=stage:resolveStyle("szWheelDistance",self._style)
	end
end

local isNumTbl={ 
	["0"]=true,
	["1"]=true,
	["2"]=true,
	["3"]=true,
	["4"]=true,
	["5"]=true,
	["6"]=true,
	["7"]=true,
	["8"]=true,
	["9"]=true,
	["-"]=true,
	["."]=true,
}
local fileSuffix={
	[".png"]=true,
	[".jpg"]=true,
	}

if Sprite.resolveStyle then
	--[[function UI.Style:resolve(n)
		return stage:resolveStyle(n,self)
	end	]]
else
	function Sprite.resolveStyle(sp,n,self,noRefs)
		if not self then self=sp._style end
		local iself=self
		local pself=self.__Parent
		local limit=100
		while limit>0 do
			local nt=type(n)
			if nt~="string" then return n,nt end
			local fc=n:sub(1,1)
			if isNumTbl[fc] then --number
				local num,unit=n:match("([-.0-9]+)(.*)")
				if unit=="s" then
					n=tonumber(num)*UI.Style.fontSize
				elseif unit=="is" then
					n=tonumber(num)*UI.Style.fontSize*UI.Style.iconScale
				elseif unit=="em" then
					n=tonumber(num)*sp:resolveStyle("font",iself):getLineHeight()
				else
					assert(false,"Unit not recognized:"..unit)
				end
				return n,"number"
			elseif fc=="|" or (#n>3 and fileSuffix[n:lower():sub(#n-3)]) then --file
				return n,nt
			end
			if not noRefs then
				local rname=rawget(self,"__Reference")
				if rname then
					local ref=sp:resolveStyle(rname,iself,true)
					assert(ref,"No such reference:"..rname)
					assert(not ref.__Reference,"Referenced style shouldn't contain references")
					self=ref
				end
			end
			local nn=self[n]
			if nn==nil then
				self=pself
				if not self then return end
				pself=self.__Parent
			else 
				n=nn
				self=iself
				pself=self.__Parent
			end
			limit-=1
		end
		assert(limit==0,"Recursion while resolving:"..n)
	end
end

UI.Style.zoomFactor=zoom
UI.Style.fontSize=(UI.Default.fontSize or fzoom*zoom)*(UI.Default.fontScale or 1)

local function loadFont(ttf,size,outline)
	local f=TTFont.new(ttf,size,"",true,outline)
	f._size=size
	return f
end
if UI.Default.Fonts then
	for k,fs in pairs(UI.Default.Fonts) do
		UI.Style[k]=loadFont(fs.ttf,UI.Style.fontSize*(fs.size or 1))
	end
elseif UI.Default.TTF then
	UI.Style.font=loadFont(UI.Default.TTF,UI.Style.fontSize)
	UI.Style["font.small"]=loadFont(UI.Default.TTF,UI.Style.fontSize*.7)
	UI.Style["font.bold"]=loadFont(UI.Default.TTF,UI.Style.fontSize,1.1)
else --SystemFont
	UI.Style.font=Font.getDefault()
	UI.Style.font._size=0
	print("UI.Style.font is Font Default ! set UI.Default.TTF=File.ttf or UI.Default.TTF={File1.ttf,File2.ttf} or dependency GiderosLib/GiderosInit/base with _UI = true in init.lua")
end

if debug then 
	print("UI.Style","UI.Style.fontSize",UI.Style.fontSize)
	print("UI.Style","UI.Default.fontSize?",UI.Default.fontSize,"UI.Default.fontScale?",UI.Default.fontScale)
	if _inspect then print("UI.Style","UI.Default.TTF?",_inspect(UI.Default.TTF)) end
end

UI.Style.icon=Texture.new("ui/icons/panel.png",true)
UI.Style.iconScale=1 --Button/icons have the same size as a line of text
if UI.Default.iconScale then UI.Style.iconScale = UI.Default.iconScale end

if UI.Default.styleCustomSizes then
	UI.Default.styleCustomSizes(UI.Style)
end

UI.Style["unit.s"]=UI.Style.fontSize
UI.Style["unit.is"]=UI.Style.fontSize*UI.Style.iconScale

UI.Style.colUI				=UI.Colors.black
UI.Style.colText			=UI.Colors.black
UI.Style.colDisabled		=UI.Color(.5,.5,.5,1) 		--Dark Grey
UI.Style.colBackground		=UI.Colors.white 			--White
UI.Style.colDarkBackground	=UI.Color(.3,.3,.3,1)		--Very dark grey
UI.Style.colShadow			=UI.Color(0,0,0,.5) 		--Light Black
UI.Style.colTile			=UI.Color(.5,.5,.5,1) 		--Dark Grey
UI.Style.colHeader			=UI.Color(.75,.75,.75,1) 	--Grey
UI.Style.colSelect			=UI.Color(0,.75,1,.9) 		--Light blue
UI.Style.colHighlight		=UI.Color(0,.25,1,.9) 		--Dark blue
UI.Style.colError			=UI.Color(0xFF0000) 		--red
UI.Style.colTesting			=UI.Color(1,0.7,0.92,.5) 	--Light pink
UI.Style.colWidgetBack		=UI.Colors.transparent
UI.Style.brdWidget			=nil
UI.Style.szDragThreshold	="1s"
UI.Style.szWheelDistance	="1em"

local colNone=UI.Colors.transparent		--TOSEE: maybe create a UI.Color table for standard things ?
local colFull=UI.Colors.white

if UI.Default.styleCustom then
	UI.Default.styleCustom(UI.Style)
end
UI.Style.bar={
	colForeground="colSelect",
	colBackground="colBackground",
}
UI.Style.breadcrumbs={
	styItem={
	},
	styRoot={ 
	},
	styLast={
		colText="colHighlight",
	},
	styElipsis="breadcrumbs.styRoot",
	stySeparator={
		colText="colHeader",
	},
	szSpacing=".1s"
}
UI.Style.button={
	styBack={
		colWidgetBack=colFull,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/radio-multi.png",true,{ mipmap=true }),
			corners={".5s",".5s",".5s",".5s",63,63,63,63,},
			insets={ left=".5s",right=".5s",top=".5s",bottom=".5s"},
		}),
		shader={ 
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="button.colBackground", colLayer2="button.colFocus", colLayer3="button.colSelect", colLayer4=colNone } 
		},
	},
	styError={
		["button.colBackground"]="colError",
	},
	stySelected={
		["button.colSelect"]="colSelect"
	},
	stySelectedFocused={
		["button.colSelect"]="colSelect",
		["button.colFocus"]="colHighlight"
	},
	styFocused={
		["button.colFocus"]="colHighlight"
	},
	colBackground="colHeader",
	colFocus="colUI",
	colSelect=colNone,
}
UI.Style.calendar={	
	fontDayNames="font.bold",
	colBackground="colBackground",
	colBorder="colHeader",
	colSpinnerBackground="calendar.colBorder",
	colSpinnerBorder="calendar.colBorder",
	colDays="colText",
	colDaysOther="colDisabled",
	colDaySelected="colHighlight",
	colDayHeader="colText",
	szDay="1.7em",
	szCellSpacing=".2s",
	szCorner=".5s",
	szInset="0s",
	szMargin=".2s",
	szSpinnerInset=".3s",
	styDayHeader={
		["label.color"]="calendar.colDayHeader",
		["label.font"]="calendar.fontDayNames",
	},
	styDays={
		["label.color"]="calendar.colDays",
	},
	styDaysOther={
		["label.color"]="calendar.colDaysOther",
	},
	styDaySelected={
		["label.color"]="calendar.colDays",
		colWidgetBack=colFull,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/radio-multi.png",true,{ mipmap=true }),
			corners={},
		}),
		shader={ 
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="calendar.colDaySelected", colLayer2=colNone, colLayer3=colNone, colLayer4=colNone } 
		}
	},
	
	styBase={
		colWidgetBack=colFull,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/radio-multi.png",true,{ mipmap=true }),
			corners={"calendar.szCorner","calendar.szCorner","calendar.szCorner","calendar.szCorner",63,63,63,63,},
			insets={ left="calendar.szInset", right="calendar.szInset", top="calendar.szInset", bottom="calendar.szInset" },
		}),
		shader={ 
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="calendar.colBackground", colLayer2="calendar.colBorder", colLayer3=colNone, colLayer4=colNone } 
		}
	},
	stySpinners={
	},
	stySpinnersLocal={
		colWidgetBack=colFull,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/radio-multi.png",true,{ mipmap=true }),
			corners={"calendar.szCorner","calendar.szCorner","calendar.szCorner","calendar.szCorner",63,63,63,63,},
			insets={ left="calendar.szSpinnerInset", right="calendar.szSpinnerInset", top="calendar.szSpinnerInset", bottom="calendar.szSpinnerInset" },
		}),
		shader={ 
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="calendar.colSpinnerBackground", colLayer2="calendar.colSpinnerBorder", colLayer3=colNone, colLayer4=colNone } 
		}
	},
}
UI.Style.checkbox={
	styTickbox={
		colWidgetBack=colFull,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/checkbox-multi.png",true,{ rawalpha=true, mipmap=true }),
			corners={0,0,0,0,0,0,0,0},
		}),
		shader={ 
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="checkbox.colTickboxBack", colLayer2="checkbox.colTickboxFore", colLayer3="checkbox.colTickboxTick", colLayer4="checkbox.colTickboxThird" } 
		}
	},
	colTickboxBack=colNone,
	colTickboxFore="colUI",
	colTickboxTick="colUI",
	colTickboxThird=colNone,
	styUnticked={
		["checkbox.colTickboxTick"]=colNone, 
	},
	styTicked={
	},
	styThird={
		["checkbox.colTickboxTick"]=colNone, 
		["checkbox.colTickboxThird"]="colUI", 
	},
	styDisabled={
		["checkbox.colTickboxTick"]=colNone, 
		["checkbox.colTickboxFore"]="colDisabled", 
	},
	styDisabledTicked={
		["checkbox.colTickboxFore"]="colDisabled", 
		["checkbox.colTickboxTick"]="colDisabled", 
	},
	styDisabledThird={
		["checkbox.colTickboxFore"]="colDisabled", 
		["checkbox.colTickboxTick"]=colNone, 
		["checkbox.colTickboxThird"]="colDisabled", 
	},
	szIcon="1is",
}
UI.Style.combobox={
	styBase={
		["button.styBack"]={
			colWidgetBack=colFull,
			brdWidget=UI.Border.NinePatch.new({
				texture=Texture.new("ui/icons/radio-multi.png",true,{ mipmap=true }),
				corners={".5s",".5s",".5s",".5s",63,63,63,63,},
				insets={ left=".5s",right=".5s",top=".5s",bottom=".5s"},
			}),
			shader={ 
				class="UI.Shader.MultiLayer", 
				params={ colLayer1="button.colBackground", colLayer2="button.colFocus", colLayer3="button.colSelect", colLayer4=colNone } 
			},
		},
		["button.styError"]={
			["button.colBackground"]="colError",
		},
		["button.stySelected"]={
			["button.colSelect"]="colSelect"
		},
		["button.stySelectedFocused"]={
			["button.colSelect"]="colSelect",
			["button.colFocus"]="colHighlight"
		},
		["button.styFocused"]={
			["button.colFocus"]="colHighlight"
		},
		["button.colBackground"]="colHeader",
		["button.colFocus"]="colUI",
		["button.colSelect"]=colNone,
		["textfield.styReadonly"]={
		},
	},
	styListContainer={
		colWidgetBack=colFull,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/radio-multi.png",true,{ mipmap=true }),
			corners={".5s",".5s",".5s",".5s",63,63,63,63,},
			insets={ left=".5s",right=".5s",top=".5s",bottom=".5s"},
		}),
		shader={ 
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="combobox.colBackground", colLayer2="combobox.colBorder", colLayer3=colNone, colLayer4=colNone } 
		},		
	},
	colBackground="colBackground",
	colBorder="colUI",
}
UI.Style.comboboxbutton={
	styBase="combobox.styBase",
	styListContainer="combobox.styListContainer",
}
UI.Style.datepicker={
	szWidth="7em",
	styNormal={},
	styError={
		["label.color"]="colError",
		["textfield.colForeground"]="colError",
	},
	icPicker=Texture.new("ui/icons/ic_cal.png",true,{ mipmap=true }),
}
UI.Style.dialog={
	bborder=UI.Border.NinePatch.new({
		texture=Texture.new("ui/icons/grey-panel.png",true,{ mipmap=true }),
		corners={15,15,15,15,15,15,15,15},
		insets={ left=20, right=10, top=10, bottom=20 },
	}),
	colBackground="colBackground",
}
UI.Style.dnd={
	colSrcHighlight="colHighlight",
	colDstHighlight="colHighlight",
	szInsertPoint=".3s",
	szMarkerMargin=".4s",
	szMarkerInset=".3s",
	styMarker={
		colWidgetBack=colFull,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/textfield-multi.png",true,{ mipmap=true }),
			corners={"dnd.szMarkerMargin","dnd.szMarkerMargin","dnd.szMarkerMargin","dnd.szMarkerMargin",39,39,39,39},
			insets={ left="dnd.szMarkerInset",right="dnd.szMarkerInset",top="dnd.szMarkerInset",bottom="dnd.szMarkerInset"},
		}),
		shader={
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="dnd.colMarkerBack", colLayer2=colNone, colLayer3="dnd.colMarkerBorder", colLayer4=colNone }
		},
	},
	styMarkerDenied={
		["dnd.colMarkerBorder"]="colDisabled",
	},
	colMarkerBack=colNone,
	colMarkerBorder="colHighlight",
	colMarkerTint=UI.Color(1,1,1,.85),
}
UI.Style.editableclock={	
	colBackground="calendar.colBackground",
	colBorder="calendar.colBorder",
	
	colRing="colDisabled",
	colCenter="editableclock.colBackground",
	colDot="editableclock.colBorder",
		
	fontLabels="font",	
	colLabels="colText",
	
	szClock="10is",
	szRing="9.5is",
	szText="9is",
	szCenter="7is",
	szDot=".6s",
	
	szCorner=".5s",
	szInset=".2s",
	szMargin=".2s",

	styLabel={
		["label.color"]="editableclock.colLabels",
	},
	
	styBase={
		colWidgetBack=colFull,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/radio-multi.png",true,{ mipmap=true }),
			corners={"editableclock.szCorner","editableclock.szCorner","editableclock.szCorner","editableclock.szCorner",63,63,63,63,},
			insets={ left="editableclock.szInset", right="editableclock.szInset", top="editableclock.szInset", bottom="editableclock.szInset" },
		}),
		shader={ 
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="editableclock.colBackground", colLayer2="editableclock.colBorder", colLayer3=colNone, colLayer4=colNone } 
		}
	},
	styHandBase={		
		["handle.styBase"]={
			colWidgetBack=colFull,
			brdWidget="handle.marker",
			shader={ 
				class="UI.Shader.MultiLayer", 
				params={ colLayer1="hand.colHand", colLayer2="handle.colText", colLayer3=colNone, colLayer4=colNone } 
			}
		},
		["handle.styArrows"]={
			colWidgetBack="hand.colArrows",
			brdWidget=UI.Border.NinePatch.new({
				texture=Texture.new("ui/icons/clock_arrows.png",true,{ mipmap=true }),
				corners={0,0,0,0,0,0,0,0},
			}),
		},
		["handle.colText"]="colText",
		["handle.szHandle"]="2is",
		["hand.colHand"]="colHighlight",
		["hand.colArrows"]="colUI",
		["hand.szWidth"]=".1s",
		
		colWidgetBack="hand.colHand",
	},
	styHandH={		
		["handle.marker"]=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/clock_hand_h.png",true,{ mipmap=true }),
			corners={0,0,0,0,0,0,0,0},
		}),
		["hand.szHand"]="3is",
		["hand.szOffset"]=".5s",
		["handle.szOffset"]="2is",
	},
	styHandM={		
		["handle.marker"]=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/clock_hand_m.png",true,{ mipmap=true }),
			corners={0,0,0,0,0,0,0,0},
		}),
		["hand.szHand"]="4.5is",
		["hand.szOffset"]=".5s",
		["handle.szOffset"]="3.5is",
	},
	styHandSel={
		["hand.colHand"]="colSelect",
	}
}

UI.Style.image={
	colTint=colFull,
	szIcon="1is",
}
UI.Style.keyboard={
	colSpecKeys="colHighlight",
	szSpacing=".5s",
	szKey="2s",
	szLineHeight="2is",
	szMargin=".5s",
	szSpecKey="6s",
	szEnterKey="13s",
	icBS=Texture.new("ui/icons/kbd_bs.png",true,{ mipmap=true }),
	icOK=Texture.new("ui/icons/kbd_ocheck.png",true,{ mipmap=true }),
	icShift=Texture.new("ui/icons/kbd_shift.png",true,{ mipmap=true }),
	icUnShift=Texture.new("ui/icons/kbd_noshift.png",true,{ mipmap=true }),
	icHide=Texture.new("ui/icons/kbd_hide.png",true,{ mipmap=true }),
	icCaps=Texture.new("ui/icons/kbd_capslock.png",true,{ mipmap=true }),
	styKeys={
	},
	stySpecKeys={
		["button.colBackground"]="keyboard.colSpecKeys",
	},
	styBase={
		colWidgetBack="colBackground",
	}
}
UI.Style.label={
	color="colText",
	szInset=".2s",
	font="font",
}
UI.Style.buttontextfield={
	icButton=Texture.new("ui/icons/panel.png",true,{ mipmap=true }),
	styButton={
		["button.styBack"]={
			colWidgetBack=UI.Colors.transparent,
			brdWidget={},
			shader={},
		},
		["button.colBackground"]=colNone,
		["button.colBorder"]=colNone,
		["button.colFocus"]=colNone,
		["button.colSelect"]=colNone,
		["button.styInside"]={
			["image.colTint"]="colUI",
		},
		["button.styError"]={
			["button.colBackground"]="colError",
		},
		["button.stySelected"]={
			["button.styInside"]={
				["image.colTint"]="colSelect",
			}
		},
		["button.stySelectedFocused"]={
			["button.styInside"]={
				["image.colTint"]="colSelect",
			}
		},
		["button.styFocused"]={
			["button.styInside"]={
				["image.colTint"]="colUI",
			}
		},
	},
	styButtonDisabled={
		["button.styBack"]={
			colWidgetBack=UI.Colors.transparent,
			brdWidget={},
			shader={},
		},
		["button.colBackground"]=colNone,
		["button.colBorder"]=colNone,
		["button.colFocus"]=colNone,
		["button.colSelect"]=colNone,
		["button.styInside"]={
			["image.colTint"]="colDisabled",
		},
		["button.styError"]={},
		["button.stySelected"]={},
		["button.stySelectedFocused"]={},
		["button.styFocused"]={},
	},
}
UI.Style.buttontextfieldcombo={
	icButton=Texture.new("ui/icons/rdown.png",true,{ mipmap=true }),
	styBase="buttontextfield",
	styButton="buttontextfield.styButton",
	styButtonDisabled="buttontextfield.styButtonDisabled",
}
UI.Style.passwordfield={
	icButton=Texture.new("ui/icons/eye.png",true,{ mipmap=true }),
	styButton="buttontextfield.styButton",
	styButtonDisabled="buttontextfield.styButtonDisabled",
}
UI.Style.progress={
	szCircular="2is",
	brdCircular=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/spot.png",true,{ mipmap=true }),
			corners={0,0,0,0,0,0,0,0},
			insets={ left="progress.szCircular", right="progress.szCircular", top="progress.szCircular", bottom="progress.szCircular" },
	}),
	styCircular={
		colWidgetBack="colHighlight",
		
		brdWidget="progress.brdCircular",
		shader={ 
			class="UI.Shader.SectorPainter", 
			params={
				numAngleStart=0,
				--numAngleEnd=0,
				numRadiusStart=0,
				numRadiusEnd=1,
				colIn=0xFFFFFF, --White
				colOut=#0
			}
		},
	},
	colBorder="colUI",
	colBackground="colBackground",
	colDone="colHighlight",
	colRemain=colNone,
	styBar={
		colWidgetBack=colFull,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/radio-multi.png",true,{ mipmap=true }),
			corners={".5s",".5s",".5s",".5s",63,63,63,63,},
		}),
		shader={ 
			class="UI.Shader.ProgressMultiLayer", 
			params={ colLayer1="progress.colBackground", colLayer2="progress.colBorder", colLayer3="progress.colDone", colLayer3a="progress.colRemain", colLayer4=colNone } 
		},		
	},
	styBarText={
	},
	styNormal={
	},
	styDisabled={
		["progress.colBackground"]="colDisabled",
		["progress.colDone"]="colDisabled",
	},
}
UI.Style.radio={
	styTickbox={
		colWidgetBack=colFull,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/radio-multi.png",true,{ mipmap=true }),
			corners={0,0,0,0,0,0,0,0},
		}),
		shader={ 
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="radio.colTickboxBack", colLayer2="radio.colTickboxFore", colLayer3="radio.colTickboxTick", colLayer4=colNone } 
		}
	},
	colTickboxBack=colNone,
	colTickboxFore="colUI",
	colTickboxTick="colUI",
	styUnticked={
		["radio.colTickboxTick"]=colNone, 
	},
	styTicked={
	},
	styDisabled={
		["radio.colTickboxTick"]=colNone, 
		["radio.colTickboxFore"]="colDisabled", 
	},
	styDisabledTicked={
		["radio.colTickboxFore"]="colDisabled", 
		["radio.colTickboxTick"]="colDisabled", 
	},
	szIcon="1is",
}
UI.Style.scrollbar={
	styBar={
		colWidgetBack=colFull,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/radio-multi.png",true,{ mipmap=true }),
			corners={"scrollbar.szKnob","scrollbar.szKnob","scrollbar.szKnob","scrollbar.szKnob",63,63,63,63,},
			insets={ left=0, right=0, top=0, bottom=0 },
		}),
		shader={ 
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="scrollbar.colBar", colLayer2=colNone, colLayer3=colNone, colLayer4=colNone } 
		}
	},
	szKnob=".3s",
	styKnob={
		colWidgetBack=colFull,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/radio-multi.png",true),
			corners={"scrollbar.szKnob","scrollbar.szKnob","scrollbar.szKnob","scrollbar.szKnob",63,63,63,63,},
			insets={ left="scrollbar.szKnob",right="scrollbar.szKnob",top="scrollbar.szKnob",bottom="scrollbar.szKnob"},
		}),
		shader={ 
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="scrollbar.colKnob", colLayer2=colNone, colLayer3=colNone, colLayer4=colNone } 
		}
	},
	colBar="colHeader",
	colKnob="colHighlight",
}
UI.Style.slider={
	colKnob="colHighlight",
	colKnobCenter="colText",
	colRailBorder="colHeader",
	colRailBackground="colBackground",
	colRailActive="colSelect",
	colRailInactive=colNone,
	szRail=".5s",
	szKnob="2s",
	styKnob={
		colWidgetBack=colFull,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/radio-multi.png",true,{ mipmap=true }),
			corners={0,0,0,0,0,0,0,0},
		}),
		shader={ 
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="slider.colKnobCenter", colLayer2="slider.colKnob", colLayer3=colNone, colLayer4=colNone } 
		}
	},
	styRail={
		["slider.railTexture"]=Texture.new("ui/icons/radio-multi.png",true,{ mipmap=true }),
		["slider.railCorners"]={".5s",".5s",".5s",".5s",63,63,63,63,},
		shader={ 
			class="UI.Shader.MeshLineMultiLayer", 
			params={ colLayer1="slider.colRailBackground", colLayer2="slider.colRailBorder", colLayer3="slider.colRailActive", colLayer3a="slider.colRailInactive", colLayer4=colNone } 
		},		
	},
	styNormal={
	},
	styDisabled={
		["slider.colRailActive"]=colNone,
	},
	styKnobNormal={
	},
	styKnobDisabled={
		["slider.colKnob"]="colDisabled",
		["slider.colKnobCenter"]="colDisabled",
	},
}
UI.Style.spinner={
	icNumPrev=Texture.new("ui/icons/minus.png",true,{ mipmap=true }),
	icNumNext=Texture.new("ui/icons/plus.png",true,{ mipmap=true }),
	icLstPrev=Texture.new("ui/icons/rprev.png",true,{ mipmap=true }),
	icLstNext=Texture.new("ui/icons/rnext.png",true,{ mipmap=true }),
	szIcon="1is",
	colIcon="colHighlight",
	colIconDisabled="colDisabled",
	styNormal={
	},
	styDisabled={
	},
	styButtonNormal={
		["image.colTint"]="spinner.colIcon",
	},
	styButtonDisabled={
		["image.colTint"]="spinner.colIconDisabled",
	},
	styTextNormal={
	},
	styTextDisabled={
	},
}
UI.Style.splitpane={
	szKnob="1is",
	brdKnobH=UI.Border.NinePatch.new({
		texture=Texture.new("ui/icons/splitpane-bar-h.png",true,{ mipmap=true }),
		corners={0,0,"1s","1s",0,0,16,16},
	}),
	brdKnobV=UI.Border.NinePatch.new({
		texture=Texture.new("ui/icons/splitpane-bar-v.png",true,{ mipmap=true }),
		corners={"1s","1s",0,0,16,16,0,0},
	}),
	tblKnobSizes={".9is",".2is",".9is"},
	colKnobBackground=colNone,
	colKnob="colHeader",
	colKnobHandle="colHighlight",
	colKnobSymbol="colUI",
	styKnobH={
		brdWidget="splitpane.brdKnobH",
		colWidgetBack="splitpane.colKnob" 
	},
	styKnobV={
		brdWidget="splitpane.brdKnobV",
		colWidgetBack="splitpane.colKnob" 
	},
	styKnobHandleH={
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/splitpane-h.png",true,{ mipmap=true }),
			corners={0,0,0,0,0,0,0,0},
			insets={ left="splitpane.szKnob", right="splitpane.szKnob", top="splitpane.szKnob", bottom="splitpane.szKnob" },
		}),
		colWidgetBack=0xFFFFFF,
		shader={ class="UI.Shader.MultiLayer", params={ colLayer1="splitpane.colKnobHandle", colLayer2="splitpane.colKnobSymbol", colLayer3="splitpane.colKnobHandle" }},
	},
	styKnobHandleV={
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/splitpane-v.png",true,{ mipmap=true }),
			corners={0,0,0,0,0,0,0,0},
			insets={ left="splitpane.szKnob", right="splitpane.szKnob", top="splitpane.szKnob", bottom="splitpane.szKnob" },
		}),
		colWidgetBack=0xFFFFFF,
		shader={ class="UI.Shader.MultiLayer", params={ colLayer1="splitpane.colKnobHandle", colLayer2="splitpane.colKnobSymbol", colLayer3="splitpane.colKnobHandle" }},
	},
}
UI.Style.table={
	colHeader="colHeader",
	colTextHeader="colHighlight",
	styDndMarker={
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/radio-multi.png",true,{ mipmap=true }),
			corners={".5s",".5s",".5s",".5s",63,63,63,63,},
			insets={ left=".5s",right=".5s",top=".5s",bottom=".5s"},
		}),
		shader={
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="table.colHeader", colLayer2=colNone, colLayer3=colNone, colLayer4=colNone }
		},
		colWidgetBack="table.colHeader",
		colText="table.colTextHeader",
	},
	szResizeHandle="1s", --Size of column resize handle
	styRowHeader={ colText="table.colTextHeader" },
	styRowHeaderLocal={ colWidgetBack="table.colHeader" },
	styRowSelected={ colWidgetBack="colSelect" },
	styRowOdd={ },
	styRowEven={ },
	styCell={ },
	styCellSelected={ },
}
UI.Style.textfield={
	styBase={
		colWidgetBack=colFull,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/textfield-multi.png",true,{ mipmap=true }),
			corners={"textfield.szMargin","textfield.szMargin","textfield.szMargin","textfield.szMargin",63,63,63,63,},
		}),
		shader={ 
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="textfield.colBackground", colLayer2="textfield.colBorder", colLayer3="textfield.colBorderWide", colLayer4=colNone } 
		}
	},
	colForeground="colText",
	colTipText="colDisabled",
	colBackground="colBackground",
	colBorder="colUI",
	colBorderWide=colNone,
	colSelection="colSelect",
	styNormal={
	},
	styDisabled={
		["textfield.colBackground"]=colNone, 
		["textfield.colBorder"]="colDisabled", 
		["textfield.colBorderWide"]=colNone, 
		["textfield.colForeground"]="colDisabled", 
	},
	styFocused={
		["textfield.colBorder"]="colHighlight", 
	},
	styFocusedReadonly={
		["textfield.colBorder"]="colHighlight", 
	},
	styReadonly={
	},
	szMargin=".3s",
}
UI.Style.timepicker={
	szWidth="5em",
	styNormal={},
	styError={
		["label.color"]="colError",
		["textfield.colForeground"]="colError",
	},
	icPicker=Texture.new("ui/icons/ic_clock.png",true,{ mipmap=true }),
}
UI.Style.toolbox={
	styToolbox={
		["table.styRowHeader"]="toolbox.styHeader",
		["table.styRowHeaderLocal"]="toolbox.styHeaderLocal",
		["table.styCell"]="toolbox.styItem",
	},
	styContainer={
		colWidgetBack=colFull,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/radio-multi.png",true,{ mipmap=true }),
			corners={"toolbox.szBorder","toolbox.szBorder","toolbox.szBorder","toolbox.szBorder",63,63,63,63,},
			insets={ left=0, right=0, top=0, bottom=0 },
		}),
		shader={ 
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="toolbox.colBack", colLayer2=colNone, colLayer3=colNone, colLayer4=colNone } 
		}
	},
	styItem={},
	styHeader={
	},
	styHeaderLocal={
		colWidgetBack=colFull,
		shader={ 
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="toolbox.colHeader", colLayer2="toolbox.colBorder", colLayer3=colNone, colLayer4=colNone } 
		}
	},
	styHeaderHorizontal={
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/radio-multi.png",true,{ mipmap=true }),
			corners={"toolbox.szBorder",0,"toolbox.szBorder","toolbox.szBorder",63,63,63,63,},
			insets={ left=0, right=0, top=0, bottom=0 },
		}),
	},
	styHeaderVertical={
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/radio-multi.png",true,{ mipmap=true }),
			corners={"toolbox.szBorder","toolbox.szBorder","toolbox.szBorder",0,63,63,63,63,},
			insets={ left=0, right=0, top=0, bottom=0 },
		}),
	},
	colBack="colBackground",
	colHeader="colHeader",
	colBorder="colHighlight",
	szBorder=".3s",
}
UI.Style.tooltip={
	szOffsetMax="4s",
	styMarker={
		colWidgetBack=colFull,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/radio-multi.png",true,{ mipmap=true }),
			corners={".5s",".5s",".5s",".5s",63,63,63,63,},
			insets={ left=".1s",right=".1s",top=".1s",bottom=".1s"},
		}),
		shader={
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="tooltip.colMarkerBack", colLayer2="tooltip.colMarkerBorder", colLayer3=colNone, colLayer4=colNone }
		},
	},
	colMarkerBack="colBackground",
	colMarkerBorder="colHighlight",
}
UI.Style.tree={
	icExpand=Texture.new("ui/icons/tree-expand.png",true,{ mipmap=true }),
	icCollapse=Texture.new("ui/icons/tree-collapse.png",true,{ mipmap=true }),
	icVert=Texture.new("ui/icons/tree-vert.png",true,{ mipmap=false }),
	icSub=Texture.new("ui/icons/tree-sub.png",true,{ mipmap=true }),
	icEnd=Texture.new("ui/icons/tree-end.png",true,{ mipmap=true }),
	colLine="colUI",
	szBox="1s",
	stySubCell={
	},
	stySub={
		colWidgetBack="tree.colLine",
		brdWidget=UI.Border.NinePatch.new({
			texture="tree.icSub",
			corners={0,0,0,0,0,0,0,0},
		}),
	},
	styEnd={
		colWidgetBack="tree.colLine",
		brdWidget=UI.Border.NinePatch.new({
			texture="tree.icEnd",
			corners={0,0,0,0,0,0,0,0},
		}),
	},
	styCollapse={
		colWidgetBack="tree.colLine",
		brdWidget=UI.Border.NinePatch.new({
			texture="tree.icCollapse",
			corners={0,0,0,0,0,0,0,0},
		}),
	},
	styExpand={
		colWidgetBack="tree.colLine",
		brdWidget=UI.Border.NinePatch.new({
			texture="tree.icExpand",
			corners={0},
		}),
	},
	styVert={
		colWidgetBack="tree.colLine",
		brdWidget=UI.Border.NinePatch.new({
			texture="tree.icVert",
			corners={0,0,1,0,0,0,1,0},
		}),
	},
}
UI.Style.weekschedule={	
	colGrid="colHeader",
	colGridFirst="colText",
	fontLabel="font.small",
	colLabelBack="colHeader",
	colBars="colHighlight",
	colCell="colBackground",
	colHeader="colHeader",
	szCell="1.5em",
	szCellSpacing=".2s",
	styHeader={
		brdWidget=nil,
		colWidgetBack="weekschedule.colHeader",
	},
	styBars={
		colWidgetBack=colFull,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/radio-multi.png",true,{ mipmap=true }),
			corners={"scrollbar.szKnob","scrollbar.szKnob","scrollbar.szKnob","scrollbar.szKnob",63,63,63,63,},
			insets={ left=0, right=0, top=0, bottom=0 },
		}),
		shader={ 
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="weekschedule.colBars", colLayer2=colNone, colLayer3=colNone, colLayer4=colNone } 
		}
	},
	styLabel={
		colWidgetBack=colFull,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/radio-multi.png",true,{ mipmap=true }),
			corners={"scrollbar.szKnob","scrollbar.szKnob","scrollbar.szKnob","scrollbar.szKnob",63,63,63,63,},
			insets={ left=0, right=0, top=0, bottom=0 },
		}),
		shader={ 
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="weekschedule.colLabelBack", colLayer2=colNone, colLayer3=colNone, colLayer4=colNone } 
		}
	},
	styNormal={
	},
	styDisabled={
		["weekschedule.colBars"]="colDisabled",
		["weekschedule.colLabelBack"]="colDisabled",
		["weekschedule.colHeader"]="colDisabled",
	},
}

if UI.Default.styleCustomWidgets then
	UI.Default.styleCustomWidgets(UI.Style)
end
