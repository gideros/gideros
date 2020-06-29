
if json then
	Chroma=Core.class(EventDispatcher)
	
	function Chroma:init(devices,title,description,name,contact,category)
		self.uri="http://localhost:54235/razer/chromasdk"
		self.sessionId=0
		self.error=0
		self.ticks=0
		self.changed=false

		self.effects={["none"]="CHROMA_NONE",["static"]="CHROMA_STATIC",["custom"]="CHROMA_CUSTOM",["key"]="CHROMA_CUSTOM_KEY",["new"]="CHROMA_CUSTOM2"}	
		self.all={"keyboard","mouse","headset","mousepad","keypad","chromalink"}
		self.rzkey={["esc"]=0x0001,["f1"]=0x0003,["f2"]=0x0004,["f3"]=0x0005,["f4"]=0x0006,["f5"]=0x0007,["f6"]=0x0008,["f7"]=0x0009,["f8"]=0x000A,["f9"]=0x000B,["f10"]=0x000C,["f11"]=0x000D,["f12"]=0x000E,
					["1"]=0x0102,["2"]=0x0103,["3"]=0x0104,["4"]=0x0105,["5"]=0x0106,["6"]=0x0107,["7"]=0x0108,["8"]=0x0109,["9"]=0x010A,["0"]=0x010B,
					["a"]=0x0302,["b"]=0x0407,["c"]=0x0405,["d"]=0x0304,["e"]=0x0204,["f"]=0x0305,["g"]=0x0306,["h"]=0x0307,["i"]=0x0209,["j"]=0x0308,["k"]=0x0309,["l"]=0x030A,["m"]=0x0409,
					["n"]=0x0408,["o"]=0x020A,["p"]=0x020B,["q"]=0x0202,["r"]=0x0205,["s"]=0x0303,["t"]=0x0206,["u"]=0x0208,["v"]=0x0406,["w"]=0x0203,["x"]=0x0404,["y"]=0x0207,["z"]=0x0403,
					["numlock"]=0x0112,["num0"]=0x0513,["num1"]=0x0412,["num2"]=0x0413,["num3"]=0x0414,["num4"]=0x0312,["num5"]=0x0313,["num6"]=0x0314,["num7"]=0x0212,["num8"]=0x0213,["num9"]=0x0214,
					["num/"]=0x0113,["num*"]=0x0114,["num-"]=0x0115,["num+"]=0x0215,["numenter"]=0x0415,["num."]=0x0514,["printscreen"]=0x000F,["scroll"]=0x0010,["pause"]=0x0011,
					["insert"]=0x010F,["home"]=0x0110,["pageup"]=0x0111,["delete"]=0x020f,["end"]=0x0210,["pagedown"]=0x0211,["up"]=0x0410,["left"]=0x050F,["down"]=0x0510,["right"]=0x0511,["tab"]=0x0201,["capslock"]=0x0301,
					["backspace"]=0x010E,["enter"]=0x030E,["lctrl"]=0x0501,["win"]=0x0502,["lalt"]=0x0503,["space"]=0x0507,["ralt"]=0x050B,["fn"]=0x050C,["menu"]=0x050D,["rctrl"]=0x050E,["lshift"]=0x0401,["rshift"]=0x040E,
					["macro1"]=0x0100,["macro2"]=0x0200,["macro3"]=0x0300,["macro4"]=0x0400,["macro5"]=0x0500,
					["oem1"]=0x0101,["oem2"]=0x010C,["oem3"]=0x010D,["oem4"]=0x020C,["oem5"]=0x020D,["oem6"]=0x020E,["oem7"]=0x030B,["oem8"]=0x030C,["oem9"]=0x040A,["oem10"]=0x040B,["oem11"]=0x040C,["eur1"]=0x030D,["eur2"]=0x0402,
					["jpn1"]=0x0015,["jpn2"]=0x040D,["jpn3"]=0x0504,["jpn4"]=0x0509,["jpn5"]=0x050A,["kor1"]=0x0015,["kor2"]=0x030D,["kor3"]=0x0402,["kor4"]=0x040D,["kor5"]=0x0504,["kor6"]=0x0509,["kor7"]=0x050A,["invalid"]=0xFFFF}
					
		local keys={}
		for loop=1,8 do
			local k={}
			for loop2=1,24 do k[loop2]=0 end
			keys[loop]=k
		end
		local chars={}
		for loop=1,6 do
			local k={}
			for loop2=1,22 do k[loop2]=0 end
			chars[loop]=k
		end
		local pad={}
		for loop=1,20 do pad[loop]=0 end
		local keys2={}
		for loop=1,4 do keys2[loop]={0,0,0,0,0} end
		self.colors={["keyboard"]=keys,["mouse"]={},["headset"]={0,0,0,0,0},["mousepad"]=pad,["keypad"]=keys2,["chromalink"]={0,0,0,0,0},["chars"]=chars}
		
		self.errors={	[-2]="No heartbeat",
						[-1]="Invalid",[0]="Success",[5]="Access denied",[6]="Invalid handle",[50]="Not supported",[87]="Invalid parameter",[1062]="Serive not active",
						[1152]="Single instance app",[1167]="Device not connected",[1168]="Not found",[1235]="Request aborted",[1247]="Already initialized",
						[4309]="Resource disabled",[4319]="Device not available",[5023]="Not valid state",[259]="No more items",[2147500037]="General failure"}

		self.loader=UrlLoader.new(self.uri,UrlLoader.POST,{["Content-Type"]="application/json"},
									json.encode({title=string.sub(title or "GiderosGame",1,64),
									description=string.sub(description or "A game written using GiderosSDK",1,256),
									author={name=string.sub(name or "Coder",1,64),contact=string.sub(contact or "www.giderosmobile.com",1,64)},
									device_supported=devices or self.all,
									category=category or "game"}))
		if self.loader then
			self.loader:addEventListener(Event.COMPLETE,function(e)
				local result=json.decode(e.data or "")
				if result.sessionid then
					self.sessionId=result.sessionid
					if result.uri then self.uri=result.uri end -- update uri if needed
					self.heartbeat=Timer.new(1000)
					self.heartbeat:addEventListener(Event.TIMER,function(e)
						local tick=UrlLoader.new(self.uri.."/heartbeat",UrlLoader.PUT)
						if tick then
							tick:addEventListener(Event.COMPLETE,function(e)
								local result=json.decode(e.data or {})
								if result.tick then
									self.ticks=result.tick
								else
									self.error=-2
									self.sessionId=0	-- signal no further commands to work
									self.heartbeat:stop()
								end
							end)
						end
					end)
					self.heartbeat:start()
				elseif result.result then
					self.error=result.result
				end
			end)
		end
	end
	
	function Chroma:exit()
		if self.sessionId~=0 then
			if self.heartbeat then self.heartbeat:stop() end
			UrlLoader.new(self.uri,UrlLoader.DELETE) -- signal to delete the session
		end
	end
	
	function Chroma:getWidth(device)
		local c=self.colors[device]
		if c then
			if #c>0 and type(c[1])=="table" then
				return #c[1]
			else
				return #c
			end
		end
	end
	
	function Chroma:getHeight(device)
		local c=self.colors[device]
		if c then
			if #c>0 and type(c[1])=="table" then
				return #c
			else
				return 0
			end
		end
	end
	
	function Chroma:color(devices,color)
		local d=devices or self.all
		local effect="none"
		if color~=nil then effect="static" end
		if type(d)=="table" then
			for loop=1,#d do
				self:effect(d[loop],effect,color)
				self:clear(d[loop],color)
			end
		else
			self:effect(d,effect,color)
			self:clear(d,color)
		end
		self.changed=true
	end
	
	function Chroma:clear(device,color,flush)
		local c=self.colors[device]
		if c then
			local col=color or 0
			local f=flush or false
			if #c>0 and type(c[1])=="table" then
				for loop=1,#c do
					local r=c[loop]
					for loop2=1,#r do
						r[loop2]=col
					end
				end				
			else
				for loop=1,#c do
					c[loop]=col
				end
			end
			self.changed=true
			if f then self:flush(device) end
		end
	end
	
	function Chroma:colorScroll(device,dirX,dirY,flush)
		local c=self.colors[device]
		if c then
			local f=flush or false
			if dirX<0 then
				if #c>0 and type(c[1])=="table" then
					for loop=1,#c do
						local r=c[loop]
						local t=r[1]
						table.remove(r,1)
						r[#r+1]=t
					end
				else
					local t=c[1]
					table.remove(c,1)
					c[#c+1]=t
				end
			elseif dirX>0 then
				if #c>0 and type(c[1])=="table" then
					for loop=1,#c do
						local r=c[loop]
						local t=r[#r]
						table.remove(r,#r)
						table.insert(r,1,t)
					end
				else
					local t=c[#c]
					table.remove(c,#c)
					table.insert(c,1,t)
				end
			end
			if dirY<0 then
				if #c>0 and type(c[1])=="table" then
					local t=c[1]
					table.remove(c,1)
					c[#c+1]=t
				end
			elseif dirY>0 then
				if #c>0 and type(c[1])=="table" then
					local t=c[#c]
					table.remove(c,#c)
					table.insert(c,1,t)
				end
			end
			self.changed=true
			if f then self:flush(device) end
		end
	end
	
	function Chroma:setKey(keys,colors,flush)
		if type(keys)=="table" then
			for loop=1,#keys do
				local k=self.rzkey[keys[loop]]
				if k then
					self:setColor("chars",(k&0xff)+1,(k>>8)+1,colors,flush)
				end
			end
		else
			local k=self.rzkey[keys]
			if k then
				self:setColor("chars",(k&0xff)+1,(k>>8)+1,colors,flush)
			end
		end
	end
	
	function Chroma:setColor(device,x,y,colors,flush)
		local c=self.colors[device]
		if c then
			local f=flush or false
			if #c>0 and type(c[1])=="table" then
				if y>0 and y<=#c then
					local r=c[y]
					if x>0 and x<=#r then
						if type(colors)=="table" then
							for loop=1,#colors do
								local col=colors[loop]
								if device=="chars" then
									if col==-1 then col=0 else col=col|0x1000000 end
								end
								r[x]=col
								x+=1
								if x>#r then break end
							end
						else
							if device=="chars" then
								if colors==-1 then colors=0 else colors=colors|0x1000000 end
							end
							r[x]=colors
						end
					end
				end
			else
				if x>0 and x<=#c then
					if type(colors)=="table" then
						for loop=1,#colors do
							c[x]=colors[loop]
							x+=1
							if x>#c then break end
						end
					else
						c[x]=colors
					end
				end
			end
			self.changed=true
			if f then self:flush(device) end
		end
	end
	
	function Chroma:getColor(device,x,y)
		local c=self.colors(device)
		if c then
			if #c>0 and type(c[1])=="table" then
				if y>0 and y<=#c then
					local r=c[y]
					if x>0 and x<=#r then
						return r[x]
					end
				end
			else
				if x>0 and x<=#c then
					return c[x]
				end
			end
		end
	end
	
	function Chroma:flush(device,forced)
		if self.sessionId~=0 and (self.changed or (forced or false)) then
			if device=="keyboard" or "chars" then
				self:effect("keyboard","new",{color=self.colors["keyboard"],key=self.colors["chars"]})
			else
				self:effect(device,"custom",self.colors[device])
			end
			self.changed=false
		end
	end
	
	function Chroma:effect(device,effect,param)
		if self.sessionId~=0 then
			local e=self.effects[effect] or "CHROMA_NONE"
			local params={effect=e}
			if e=="CHROMA_STATIC" then
				params["param"]={color=param or 0xffffff}
			elseif e=="CHROMA_CUSTOM" or e=="CHROMA_CUSTOM_KEY" or e=="CHROMA_CUSTOM2" then
				params["param"]=param
			end
			local send=json.encode(params)
			local loader=UrlLoader.new(self.uri.."/"..(device or "keyboard"),UrlLoader.PUT,{["Content-Type"]="applicationn",["Content-Length"]=#send},send)
			if loader then
				loader:addEventListener(Event.COMPLETE,function(e)
					local result=json.decode(e.data or "")
					if result.result~=0 then self.error=result.result end
				end)
			end
		end
	end
	
	function Chroma:isReady()
		if self.sessionId~=0 and self.ticks>5 then
			return true
		end
	end
	
	function Chroma:getLastError()
		local e=self.error
		self.error=0
		return e
	end
	
	function Chroma:getErrorText(e)
		return self.errors[e] or "Unknown Chroma error"
	end
	
	function Chroma:getTicks()
		return self.ticks
	end
	
	function Chroma:getSession()
		return self.session
	end

else
	print("Chroma plugin requires the json plugin.")
end

