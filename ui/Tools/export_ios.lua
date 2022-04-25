--
--    iOS project file tooling
--


iOSProject={
  id=0
}

iOSProject.insertPoints={
  Refs={ tag="/* End PBXFileReference section */", type="prepend", data={}},
  -- Groups and tree structure
  GroupPlugins_ios={ tag="EB42A05D1D66ED5000766D7E /* plugins.cpp */,", data={}},
  GroupPlugins_atv={ tag="EB42A0601D674BCB00766D7E /* plugins.cpp */,", data={}},
  GroupPlugins_mac={ tag="033BD45C256FA72E0059695F /* plugins.cpp */,", data={}},
  GroupFrameworks={ tag="5FD896EB15CED77F00D34824 /* UIKit.framework */,", data={}},
  -- Build refs
  Frameworks_ios={ tag="5FD896EC15CED77F00D34824 /* UIKit.framework in Frameworks */,", data={}},
  Frameworks_atv={ tag="B2E3EFD61BAC117B005599BD /* UIKit.framework in Frameworks */,", data={}},
  Frameworks_mac={ tag="033BD4652570FF8A0059695F /* Cocoa.framework in Frameworks */,", data={}},
  SourceBuild_ios={ tag="EB42A05E1D66ED5000766D7E /* plugins.cpp in Sources */,", data={}},
  SourceBuild_atv={ tag="EB42A0611D674BCB00766D7E /* plugins.cpp in Sources */,", data={}},
  SourceBuild_mac={ tag="033BD45D256FA72E0059695F /* plugins.cpp in Sources */,", data={}},  
  ResourceBuild_ios={ tag="5F58D85315D9268900137D76 /* assets in Resources */,", data={}},
  ResourceBuild_atv={ tag="B2E3EFE31BAC1249005599BD /* assets in Resources */,", data={}},
  ResourceBuild_mac={ tag="033BD41A256FA4D90059695F /* Assets.xcassets in Resources */,", data={}},
  --Paths
  FrameworksPaths={ tag="FRAMEWORK_SEARCH_PATHS = (", data={}},
}
iOSProject.InfoPlist={}
iOSProject.Entitlements={ ios={}, atv={}, mac={} }
iOSProject._pods={ ios={}, atv={}, mac={} }
iOSProject._plugins={ ios={}, atv={}, mac={} }

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

iOSProject.addReference=function(filename,filetype,filepath,filetree)
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
  ("%s /* %s */ = {isa = PBXFileReference; fileEncoding = 4; lastKnownFileType = %s; path = %s; sourceTree = \"%s\"; };\n")
  :format(refid,filename,filetype,filepath or filename, filetree or "<group>")
  iOSProject.insertData("Refs",refline)
  return refid
end

iOSProject.addSource=function(filename,filetype,filepath,filetree,extra)
  print("addSource",filename)
  local refid=iOSProject.addReference(filename,filetype,filepath,filetree)
  local fileid=iOSProject.newId()
  local refline=
  ("%s /* %s */ = {isa = PBXBuildFile; fileRef = %s /* %s */; %s};\n")
  :format(fileid,filename,refid,filename,extra or "")
  iOSProject.insertData("Refs",refline)
  return refid,fileid
end

iOSProject.addFramework=function(filename,flavor,filepath,filetree,extra)
  flavor=flavor or "ios"
  print("addFramework",filename,flavor)
  local refid,fileid=iOSProject.addSource(filename,nil,filepath,filetree,extra)
  local refline=
  ("%s /* %s */,\n")
  :format(fileid,filename)
  iOSProject.insertData("Frameworks_"..flavor,refline)
  return refid,fileid
end

iOSProject.addWeakFramework=function(filename,flavor,filepath,filetree)
  return iOSProject.addFramework(filename,flavor,filepath,filetree," settings = {ATTRIBUTES = (Weak, );  };")
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
    elseif s:ends(".plist") or s:ends(".bundle") then
      local sref,bref=iOSProject.addSource(s)
      iOSProject.addToGroup(dest,sref,flavor)
      iOSProject.addToGroup("ResourceBuild",bref,flavor)
    elseif s:ends(".framework") or s:ends(".a") then
      local sref=iOSProject.addFramework(s,flavor)
      iOSProject.addToGroup(dest,sref,flavor)
    else
      local sref=iOSProject.addReference(s)
      iOSProject.addToGroup(dest,sref,flavor)
    end
  end
end

iOSProject.addFrameworkPath=function(path)
  local refline=("\"\\\"$(PROJECT_DIR)/%s\\\"\",\n"):format(path)
  iOSProject.insertData("FrameworksPaths",refline)
end

iOSProject.exportPluginFiles=function(pname,srcdir,srcfiles,existing,foriOS,forATV,forMAC)
  if foriOS then
    local tgtDir=Export.getProperty("project.name").."/Plugins/"..pname
    if not existing then
	    Export.mkdir(tgtDir)
    end
    Export.recursiveCopy(pname,srcdir,tgtDir,"*.m;*.mm;*.c;*.h;*.cpp","emscripten;win32;jni;iOS;Android")
    if not existing then
	    iOSProject.addGroup(pname,"Plugins/"..pname,"Group"..pname.."_ios","GroupPlugins_ios")
    end
    iOSProject.addSources(srcfiles, "Group"..pname, "ios")
  end
  if forATV then
    local tgtDir="AppleTV/Plugins/"..pname
    if not existing then
    	Export.mkdir(tgtDir)
    end
    Export.recursiveCopy(pname,srcdir,tgtDir,"*.m;*.mm;*.c;*.h;*.cpp","emscripten;win32;jni;iOS;Android")
    if not existing then
    	iOSProject.addGroup(pname,pname,"Group"..pname.."_atv","GroupPlugins_atv")
    end
    iOSProject.addSources(srcfiles, "Group"..pname, "atv")
  end
  if forMAC then
    local tgtDir="Mac/Plugins/"..pname
    if not existing then
      Export.mkdir(tgtDir)
    end
    Export.recursiveCopy(pname,srcdir,tgtDir,"*.m;*.mm;*.c;*.h;*.cpp","emscripten;win32;jni;iOS;Android")
    if not existing then
      iOSProject.addGroup(pname,pname,"Group"..pname.."_mac","GroupPlugins_mac")
    end
    iOSProject.addSources(srcfiles, "Group"..pname, "mac")
  end
  iOSProject.commit()
end

function iOSProject.exportBinaryPlugin(name,sym,for_ios,for_atv,for_mac)
local srcdir="[[[sys.pluginDir]]]/bin/iOS"
  if for_ios then
    local tgtDir=Export.getProperty("project.name").."/Plugins"
    if not existing then Export.mkdir(tgtDir) end
    Export.recursiveCopy(name,srcdir,tgtDir,"*.ios.a")
    iOSProject.addSources({ "Plugins/lib"..name..".ios.a"  }, "GroupPlugins", "ios")
    if sym then table.insert(iOSProject._plugins.ios,sym) end
  end
  
  if for_atv then
    local tgtDir="AppleTV/Plugins"
    if not existing then Export.mkdir(tgtDir) end
    Export.recursiveCopy(name,srcdir,tgtDir,"*.atv.a")
    iOSProject.addSources({ "lib"..name..".atv.a"  }, "GroupPlugins", "atv")
    if sym then table.insert(iOSProject._plugins.atv,sym) end
  end
  
  if for_mac then
    local tgtDir="Mac/Plugins"
    if not existing then Export.mkdir(tgtDir) end
    Export.recursiveCopy(name,srcdir,tgtDir,"*.mac.a")
    iOSProject.addSources({ "lib"..name..".mac.a"  }, "GroupPlugins", "mac")
    if sym then table.insert(iOSProject._plugins.mac,sym) end
  end
  iOSProject.commit()
end

iOSProject.needObjCLinking=function () iOSProject.needObjCLinkingFlag=true end
iOSProject.pod=function(dep,ios,atv,mac)
    if ios then table.insert(iOSProject._pods.ios,dep) end
    if atv then table.insert(iOSProject._pods.atv,dep) end
    if mac then table.insert(iOSProject._pods.mac,dep) end
end

local function apply()
  if iOSProject.needObjCLinkingFlag then
    Export.callXml([[<template name="Project" path=""><replacelist wildcards="project.pbxproj">
    <append>
     <orig>COMPRESS_PNG_FILES = NO;</orig>
     <by>
OTHER_LDFLAGS = "-ObjC";</by>
    </append>
    </replacelist></template>]])
  end

  local function xmlEscape(s)
	  s = string.gsub(s, "&", "&amp;")
	  s = string.gsub(s, "<", "&lt;")
	  s = string.gsub(s, ">", "&gt;")
	  s = string.gsub(s, "'", "&apos;")
	  s = string.gsub(s, '"', "&quot;")
	  return s
  end

  local function plistValue(v)
    if type(v)=="table" then
      if #v>0 then
       local d="<array>\n"
       for n,v2 in ipairs(v) do d=d..plistValue(v2).."\n" end
       d=d.."</array>"
       return d
      elseif next(v,nil) then
       local d="<dict>\n"
       for k,v2 in pairs(v) do d=d.."<key>"..k.."</key>\n"..plistValue(v2).."\n" end
       d=d.."</dict>"
       return d
      else
        return "<array></array>"
      end
    elseif type(v)=="boolean" then
      if v then return "<true/>" else return "<false/>" end
    else
      return "<string>"..xmlEscape(v).."</string>"
    end
  end

  local dic=""
  for k,v in pairs(iOSProject.InfoPlist) do
   dic=dic.."<key>"..k.."</key>\n"..plistValue(v).."\n"
  end

  Export.callXml([[<template name="Project" path="">
    <replacelist wildcards="]]..Export.getProperty("project.name")..[[-Info.plist;Info.plist">
      <prepend>
        <orig>]].."<![CDATA[<key>CFBundleDisplayName</key>]]></orig><by><![CDATA["..dic.."]]></by>"..[[
      </prepend>
    </replacelist>
  </template>]])

  local dic=""
  for k,v in pairs(iOSProject.Entitlements.mac) do
   dic=dic.."<key>"..k.."</key>\n"..plistValue(v).."\n"
  end
  Export.callXml([[<template name="Project" path="">
    <replacelist wildcards="]]..Export.getProperty("project.name")..[[ Mac.entitlements">
      <prepend>
        <orig>]].."<![CDATA[<key>com.apple.security.app-sandbox</key>]]></orig><by><![CDATA["..dic.."]]></by>"..[[
      </prepend>
    </replacelist>
  </template>]])
  local dic=""
  for k,v in pairs(iOSProject.Entitlements.ios) do
   dic=dic.."<key>"..k.."</key>\n"..plistValue(v).."\n"
  end
  Export.callXml([[<template name="Project" path="">
    <replacelist wildcards="]]..Export.getProperty("project.name")..[[ iOS.entitlements">
      <prepend>
        <orig>]].."<![CDATA[</dict>]]></orig><by><![CDATA["..dic.."]]></by>"..[[
      </prepend>
    </replacelist>
  </template>]])

    if #iOSProject._pods.ios>0 or #iOSProject._pods.atv>0 or #iOSProject._pods.mac>0 then
        local function podlist(t)
            local pl=""
            for _,p in ipairs(t) do
                pl=pl.."pod '"..p.."'\n"
            end
            return pl
        end
        Export.callXml([[<template name="Project" path="">
        <replacelist wildcards="Podfile">
        <append>
        <orig>#TAG-GIDEROS-POD-IOS</orig><by>]]..podlist(iOSProject._pods.ios)..[[</by>
        </append>
        <append>
        <orig>#TAG-GIDEROS-POD-ATV</orig><by>]]..podlist(iOSProject._pods.atv)..[[</by>
        </append>
        <append>
        <orig>#TAG-GIDEROS-POD-MAC</orig><by>]]..podlist(iOSProject._pods.mac)..[[</by>
        </append>
        </replacelist>
        </template>]])
        Export.callXml([[<exec cmd="sh"><arg>-c</arg><arg>source /etc/profile;pod install</arg></exec>]])
    end

    if #iOSProject._plugins.ios>0 or #iOSProject._plugins.atv>0 or #iOSProject._plugins.mac>0 then
        local function plist(t)
            local pl=""
            for _,p in ipairs(iOSProject._plugins[t]) do
                pl=pl.."   IMPORT_PLUGIN("..p..")\n"
            end
            return pl
        end
        Export.callXml([[<template name="Project" path="">
        <replacelist wildcards="plugins.cpp">
           <append orig="//GIDEROS-TAG-IOS:IMPORTPLUGIN//" by="]]..plist("ios")..[[" force="true" />
           <append orig="//GIDEROS-TAG-ATV:IMPORTPLUGIN//" by="]]..plist("atv")..[[" force="true" />
           <append orig="//GIDEROS-TAG-MAC:IMPORTPLUGIN//" by="]]..plist("mac")..[[" force="true" />
        </replacelist>
        </template>]])
    end        
end

Export.registerPreFinish(apply)

iOSProject.InfoPlist.NSLocationUsageDescription="Do you accept to share your position?"
iOSProject.InfoPlist.NSLocationWhenInUseUsageDescription="Do you accept to share your position in background?"

return iOSProject
