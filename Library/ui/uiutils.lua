--!NEEDS:uiinit.lua
UI.Utils={}

-- TextMapper(MapperContext,Data,Flags)    			-> text  			[,textColor]
-- IconTextFlagsMapper(MapperContext,Data,Flags)	-> icons,icon,text  [,iconColor,textColor] --if d.iconFlags (max 4)
-- IconTextMapper(MapperContext,Data,Flags)   		-> icon,text    	[,iconColor,textColor]
-- IconWidgetMapper(MapperContext,Data,Flags)   	-> icon,widget    	[,iconColor,textColor] --if mapper return a string then widget will be a TextField from makeWidget so can have txc
-- IconsWidgetMapper(MapperContext,Data,Flags)  	-> icon       		[,iconColor,backgroundColor]
-- WidgetsMapper(MapperContext,Data,Flags)    		-> widget     		[,textColor,backgroundColor]

local function updateStateFlags(w,f)
  if w then
    f=f or {}
    if w.setColor then
      if f.selected then
        w:setColor(w._style.colSelect)
      else
        w:setColor(#0)
      end
    end
    if w.setTicked then
		w:setTicked(f.ticked) --set true or false
    end
  end
end

function UI.Utils.colorTransform(p,c,style)
  while type(c)=="string" do
	assert(style,"Style needed for color refs:"..c)
	local rc=stage:resolveStyle(c,style)
	if not rc then	
		--print("NOCOL:",c) 
	end
	c=rc
  end
  if not p or not c then return end
	if type(c)=="vector" then
		p:setColorTransform(c.r,c.g,c.b,c.a)
	else
	  local a=255
	  if type(c)~="number" then
		a=(c>>24)()
		c=(c&0xFFFFFF)()
	  end
	  p:setColorTransform(((c>>16)&0xFF)/255,((c>>8)&0xFF)/255,((c>>0)&0xFF)/255,a/255)
	end
end

function UI.Utils.colorVector(c,style)
  local ct=type(c)
  --local ic=c
  if ct=="string" then
	assert(style,"Style needed for color refs:"..c)
	c,ct=stage:resolveStyle(c,style)
  end
  if not c then return vector(0,0,0,0) end
  if ct=="vector" then
	  return c
  else
	--assert(false,"Un-normalized color:"..tostring(ic))
	return UI.Color(c)
  end
end

function UI.Utils.colorSplit(c,style)
  local v=UI.Utils.colorVector(c,style)
  local col=(((v.r*0xFF)&0xFF)<<16)|(((v.g*0xFF)&0xFF)<<8)|(((v.b*0xFF)&0xFF)<<0)
  return col,v.a
end

function UI.Utils.makeWidget(d,p,style)
  --print("makeWidget",d and d.name or "",d,type(d),"Sprite?",(d and d.isInstanceOf) and d:isInstanceOf("Sprite"),"UI.Factory?",(d and d.isInstanceOf) and getmetatable(d)==UI.Factory)
  if d==nil then return UI.Panel.new() end
  local tt=type(d)
  if tt=="function" then
    d=d(p)
	tt=type(d)
  end
  if tt~="table" then
		if Sprite.clone then
			UI.Utils._TPL_Label=UI.Utils._TPL_Label or UI.Label.new()
			local ld=UI.Utils._TPL_Label:clone()
			ld:setText(tostring(d))
			d=ld
		else
			d=UI.Label.new(tostring(d))
		end
  elseif d.isInstanceOf and d:isInstanceOf("Sprite") then
    --Leave untouched
	if style then d:setStyle(style) end
	return d
  elseif d.isInstanceOf and getmetatable(d)==UI.Factory then
    d=d:build(p)
  else
    d=UI.Builder(d)
  end
  assert(d.isInstanceOf and d:isInstanceOf("Sprite"),"resolved widget isn't a sprite")
  if style then d:setStyle(style) end
  return d
end

function UI.Utils.getScriptPath()
   local str = debug.getinfo(2, "S").source
   return str:match("@?(.*[/\\])") or ""
end