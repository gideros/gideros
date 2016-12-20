--[[
iOS project file tooling
]]

iOSProject={
id=0
}

iOSProject.insertPoints={
Refs={ tag="/* End PBXFileReference section */", type="prepend", data={}},
-- Groups and tree structure
GroupPlugins_ios={ tag="EB42A05D1D66ED5000766D7E /* plugins.cpp */,", data={}},
GroupPlugins_atv={ tag="EB42A0601D674BCB00766D7E /* plugins.cpp */,", data={}},
GroupFrameworks={ tag="5FD896EB15CED77F00D34824 /* UIKit.framework */,", data={}},
-- Build refs
Frameworks_ios={ tag="5FD896EC15CED77F00D34824 /* UIKit.framework in Frameworks */,", data={}},
Frameworks_atv={ tag="B2E3EFD61BAC117B005599BD /* UIKit.framework in Frameworks */,", data={}},
SourceBuild_ios={ tag="EB42A05E1D66ED5000766D7E /* plugins.cpp in Sources */,", data={}},
SourceBuild_atv={ tag="EB42A0611D674BCB00766D7E /* plugins.cpp in Sources */,", data={}},
--Paths
FrameworksPaths={ tag="FRAMEWORK_SEARCH_PATHS = (", data={}},
}
iOSProject.XML_TEMPLATE=
[[<template name="iOS Project file" path=""><replacelist wildcards="project.pbxproj">%s</replacelist></template>]]
iOSProject.XML_PREPEND="<prepend><orig>%s</orig><by><![CDATA[%s]]></by></prepend>"
iOSProject.XML_APPEND="<append><orig>%s</orig><by><![CDATA[%s]]></by></append>"

iOSProject.newId=function()
local id=string.format("%08x1111000000000000",iOSProject.id)
iOSProject.id=iOSProject.id+1
return id
end

iOSProject.insertData=function(section,line)
--print("insertData",section,line)
table.insert(iOSProject.insertPoints[section].data,line)
end

iOSProject.commit=function()
local rep=""
local korder={}
for k,v in pairs(iOSProject.insertPoints) do table.insert(korder,{ key=k, order=v.order or 0}) end
table.sort(korder,function (a,b) return a.order<b.order end)
for _,k in ipairs(korder) do
local p=iOSProject.insertPoints[k.key]
if #p.data>0 then
local tmpXml=iOSProject.XML_APPEND
if p.type=="prepend" then tmpXml=iOSProject.XML_PREPEND end
rep=rep..tmpXml:format(p.tag,table.concat(p.data,""))
p.data={}
end
end
if #rep>0 then
rep=iOSProject.XML_TEMPLATE:format(rep)
--print("XML",rep)
Export.callXml(rep)
end
end

iOSProject.addReference=function(filename,filetype)
if filetype==nil then
if filename:ends(".m") then
filetype="sourcecode.c.objc"
elseif filename:ends(".h") then
filetype="sourcecode.c.h"
elseif filename:ends(".c") then
filetype="sourcecode.c.c"
elseif filename:ends(".framework") then
filetype="wrapper.framework"
elseif filename:ends(".cpp") then
filetype="sourcecode.cpp.cpp"
elseif filename:ends(".mm") then
filetype="sourcecode.cpp.objcpp"
elseif filename:ends(".dylib") then
filetype="compiled.mach-o.dylib"
elseif filename:ends(".a") then
filetype="archive.ar"
elseif filename:ends(".plist") then
filetype="text.plist.xml"
elseif filename:ends(".strings") then
filetype="text.plist.strings"
elseif filename:ends(".png") then
filetype"image.png"
else
filetype="text"
end
end
local refid=iOSProject.newId()
local refline=
("%s /* %s */ = {isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = %s; path = %s; sourceTree = \"<group>\"; };\n")
:format(refid,filename,filetype,filename)
iOSProject.insertData("Refs",refline)
return refid
end

iOSProject.addSource=function(filename,filetype)
print("addSource",filename)
local refid=iOSProject.addReference(filename,filetype)
local fileid=iOSProject.newId()
local refline=
("%s /* %s */ = {isa = PBXBuildFile; fileRef = %s /* %s */; };\n")
:format(fileid,filename,refid,filename)
iOSProject.insertData("Refs",refline)
return refid,fileid
end

iOSProject.addFramework=function(filename,flavor)
flavor=flavor or "ios"
print("addFramework",filename,flavor)
local refid,fileid=iOSProject.addSource(filename)
local refline=
("%s /* %s */,\n")
:format(fileid,filename)
iOSProject.insertData("Frameworks_"..flavor,refline)
return refid,fileid
end

iOSProject.addGroup=function(foldername,path,publicname,dest)
print("addGroup",foldername,publicname,dest)
local refid=iOSProject.newId()
local refline=
("%s /* %s */ = {isa = PBXGroup; children = (\n/* GIDEXP-GRP-%s */\n); name = %s; path = %s; sourceTree = \"<group>\"; };\n")
:format(refid,foldername,refid,foldername, path)
iOSProject.insertData("Refs",refline)
publicname=publicname or refid
iOSProject.insertPoints[publicname]={ order=iOSProject.id, tag=("/* GIDEXP-GRP-%s */"):format(refid), data={}}
if dest then
iOSProject.addToGroup(dest,refid)
end
return refid
end

iOSProject.addToGroup=function(groupname,refid,flavor)
flavor=flavor or "ios"
local refline=("%s,\n"):format(refid)
if iOSProject.insertPoints[groupname.."_"..flavor] then
groupname=groupname.."_"..flavor
end
assert(iOSProject.insertPoints[groupname],"Error: group "..groupname.." is not defined (yet)")
iOSProject.insertData(groupname,refline)
end

iOSProject.addSources=function(srcs,dest,flavor)
flavor=flavor or "ios"
for _,s in ipairs(srcs) do
if s:ends(".m") or s:ends(".mm") or s:ends(".c") or s:ends(".cpp") then --Source
local sref,bref=iOSProject.addSource(s)
iOSProject.addToGroup(dest,sref,flavor)
iOSProject.addToGroup("SourceBuild",bref,flavor)
elseif s:ends(".framework") then
local sref=iOSProject.addFramework(s,flavor)
iOSProject.addToGroup(dest,sref,flavor)
else
local sref=iOSProject.addReference(s)
iOSProject.addToGroup(dest,sref,flavor)
end
end
end

iOSProject.addFrameworkPath=function(path)
local refline=("\"$(PROJECT_DIR)/%s\",\n"):format(path)
iOSProject.insertData("FrameworksPaths",refline)
end

return iOSProject
