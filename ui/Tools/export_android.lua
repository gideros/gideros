--[[
Tools to simplify android exports
]]

AndroidProject={
  permissions={},
  features={},
  _minSdk=14
}

local MAX_SDK=1000

function AndroidProject.useFeature(name,req)
    AndroidProject.features[name]=req or false
end

function AndroidProject.usePermission(name,maxSdk,flags)
  local msdk=maxSdk or MAX_SDK
  local cperm=AndroidProject.permissions[name] or { maxSdk=msdk, flags={} }
  cperm.maxSdk=cperm.maxSdk><msdk
  if flags then
	if type(flags)=="table" then
		for _,f in ipairs(flags) do
			cperm.flags[f]=true
		end
	elseif type(flags)=="string" then
		cperm.flags[flags]=true
	end
  end
  AndroidProject.permissions[name]=cperm
end

function AndroidProject.minSdk(minsdk)
  if minsdk>AndroidProject._minSdk then
    AndroidProject._minSdk=minsdk
  end
end

local function apply()
  local perms=""
  for n,s in pairs(AndroidProject.permissions) do
    local e=""
    if s.maxSdk<MAX_SDK then
      e=([[android:maxSdkVersion="%d" ]]):format(s.maxSdk)
    end
    if next(s.flags) and (tonumber(Export.getProperty("export.androidTarget")) or 0)>=32 then
	local p=""
	for pf,_ in pairs(s.flags) do
		p=p.."|"..pf
	end
      e=e..([[android:usesPermissionFlags="%s" ]]):format(p:sub(2))
    end
    perms=perms..(([[<uses-permission android:name="%s" %s/>
      ]]):format(n,e))
  end
  for n,r in pairs(AndroidProject.features) do
    perms=perms..(([[<uses-feature android:name="%s" android:required="%s" />
      ]]):format(n,tostring(not not r)))
  end
  Export.callXml(([[<template name="AndroidPermissionsManifest" path="">
      <replacelist wildcards="AndroidManifest.xml">
        <append>
        <orig>]].."<![CDATA[<!-- TAG:MANIFEST-EXTRA -->]]></orig><by><![CDATA[\n%s]]></by>"..
        [[</append>
          </replacelist>
  </template>]]):format(perms))
  Export.callXml(([[<template name="AndroidPermissionsManifest" path="">
      <replacelist wildcards="AndroidManifest.xml;build.gradle">
        <replace><orig>minSdkVersion 9</orig><by>minSdkVersion %d</by></replace>
       </replacelist>
  </template>]]):format(AndroidProject._minSdk,AndroidProject._minSdk))
  if AndroidProject._minSdk<21 then  
    Export.callXml([[<template name="Android Template Changes" path="">
      <replacelist wildcards="build.gradle">
        <append>
          <orig>//TAG-DEPENDENCIES//</orig>               
          <by>
           implementation 'com.android.support:multidex:1.0.3'
          </by>
        </append>
          </replacelist>
  </template>]])  
  end
end

Export.registerPreFinish(apply)

return AndroidProject
