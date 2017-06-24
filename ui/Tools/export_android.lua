--[[
Tools to simplify android exports
]]

AndroidProject={
	permissions={}
}

local MAX_SDK=1000

function AndroidProject.usePermission(name,maxSdk)
	local msdk=maxSdk or MAX_SDK
	local csdk=AndroidProject.permissions[name]
	if csdk==nil or csdk<msdk then
		AndroidProject.permissions[name]=maxSdk
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
 	Export.callXml(([[<template name="Manifest" path="">
			<replacelist wildcards="AndroidManifest.xml">
				<append>
				<orig>]].."<![CDATA[<!-- TAG:MANIFEST-EXTRA -->]]></orig><by><![CDATA[%s]]></by>"..
				[[</append>
	       	</replacelist>
 	</template>]]):format(perms))
end

Export.registerPreFinish(apply)

return AndroidProject