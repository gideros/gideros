--[[
Tools to simplify android exports
]]

AndroidProject={
  permissions={},
  _minSdk=14
}

local MAX_SDK=1000

function AndroidProject.usePermission(name,maxSdk)
  local msdk=maxSdk or MAX_SDK
  local csdk=AndroidProject.permissions[name]
  if csdk==nil or csdk<msdk then
    AndroidProject.permissions[name]=msdk
  end
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
    if s<MAX_SDK then
      e=([[android:maxSdkVersion="%d" ]]):format(s)
    end
    perms=perms..(([[
    <uses-permission android:name="%s" %s/>]]):format(n,e))
  end
  Export.callXml(([[<template name="AndroidPermissionsManifest" path="">
      <replacelist wildcards="AndroidManifest.xml">
        <append>
        <orig>]].."<![CDATA[<!-- TAG:MANIFEST-EXTRA -->]]></orig><by><![CDATA[%s]]></by>"..
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
