require "json"

local f=io.open("vr-room.json")
local src=json.decode(f:read("*all"))
print(#src.meshes)
print(#src.materials)

local mesh=src.meshes[1]
local idxCol={}
local colMap,matMap={},{}
for _,n in ipairs(src.materials) do
	matMap[n.id]=n
end
for _,n in ipairs(src.nodes[1].parts) do
	local r,g,b,a=matMap[n.materialid].diffuse[1],matMap[n.materialid].diffuse[2],matMap[n.materialid].diffuse[3],matMap[n.materialid].diffuse[4] or 1
	r=(r*255)//1
	g=(g*255)//1
	b=(b*255)//1
	a=(a*255)//1
	colMap[n.meshpartid]={
		r,g,b,a
	}
end

local imap={}
local ii=1
for _,p in ipairs(mesh.parts) do
	for _,i in ipairs(p.indices) do
		idxCol[i]=colMap[p.id]
		imap[ii]=i ii+=1
	end
end

local vmap,cmap,nmap={},{},{}
local vi,ci,ni=1,1,1
local ii=0
for i=1,#mesh.vertices,8 do
	vmap[vi]=mesh.vertices[i] vi+=1
	vmap[vi]=mesh.vertices[i+1] vi+=1
	vmap[vi]=mesh.vertices[i+2] vi+=1
	nmap[ni]=mesh.vertices[i+3] ni+=1
	nmap[ni]=mesh.vertices[i+4] ni+=1
	nmap[ni]=mesh.vertices[i+5] ni+=1
	cmap[ci]=idxCol[ii][1] ci+=1
	cmap[ci]=idxCol[ii][2] ci+=1
	cmap[ci]=idxCol[ii][3] ci+=1
	cmap[ci]=idxCol[ii][4] ci+=1
	ii+=1
end

local f=io.open("|T|vr-room.h","w")
local jss=json.encode(vmap)
f:write("static float oculusRoomV[]={\n"..jss:sub(2,#jss-1).."\n};\n")
local jss=json.encode(nmap)
f:write("static float oculusRoomN[]={\n"..jss:sub(2,#jss-1).."\n};\n")
local jss=json.encode(cmap)
f:write("static unsigned char oculusRoomC[]={\n"..jss:sub(2,#jss-1).."\n};\n")
local jss=json.encode(imap)
f:write("static unsigned short oculusRoomI[]={\n"..jss:sub(2,#jss-1).."\n};\n")
f:close()
