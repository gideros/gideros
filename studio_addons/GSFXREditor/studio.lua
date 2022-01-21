require "socket.core"
require "json"
local GIDEROS_STUDIO_DATA=loadstring("return "..os.getenv("GIDEROS_STUDIO_DATA"))()
--GIDEROS_STUDIO_DATA={serverPort=49441}
local STUDIO_SRV=socket.tcp()
local STUDIO_SRV_ID=0

local function doRequest(mtd,...)
	local arg={...}
	local params={}
	for k=1,#arg do
		local a=arg[k] or json.null
		table.insert(params,a)
	end
	local j=json.encode({ method=mtd, params=params })
	local id=STUDIO_SRV_ID+1 or 1
	STUDIO_SRV_ID=id
	local size=#j+12
	local header=string.char(size&0xFF,(size>>8)&0xFF,(size>>16)&0xFF,(size>>24)&0xFF)
	header=header..string.char(id&0xFF,(id>>8)&0xFF,(id>>16)&0xFF,(id>>24)&0xFF)
	header=header..string.char(0,0,0,0)
	STUDIO_SRV:send(header..j)
	while true do
		local rhdr=STUDIO_SRV:receive(12)
		local a,b,c,d=rhdr:byte(1,4)
		size=a+b<<8+c<<16+d<<24
		local a,b,c,d=rhdr:byte(5,8)
		local rid=a+b<<8+c<<16+d<<24
		local a,b,c,d=rhdr:byte(9,12)
		local rtype=a+b<<8+c<<16+d<<24
		local ret=STUDIO_SRV:receive(size-12)
		--print(size,rtype,ret)
		if rid==id then
			if rtype==0 then
				ret=json.decode(ret)
				if ret.error then
					assert(false,ret.error.message)
				elseif ret.result~=json.null then
					return ret.result
				else
					return nil
				end					
			else
				assert(false,ret)
			end
		end
	end
end

Studio=setmetatable({ __prefix="Studio." }, { __index=function (t,i)
	return function(...) return doRequest(t.__prefix..i,...) end
end})

local cnx,err=STUDIO_SRV:connect("127.0.0.1",GIDEROS_STUDIO_DATA.serverPort)
Studio.DATA=GIDEROS_STUDIO_DATA
if (cnx) then
	Studio.CONNECTED=true
else
	Studio.CONNECTED=false
end
