--[[
Tools to simplify html5 exports
]]

Html5Project={
}

function Html5Project.export(m,flavour)
  local pack=Export.isSet("project.html5_pack")
  flavour=flavour or ""
  local ext="wasm"
  local zext=ext
  if pack then zext="gidz" end
  Export.callXml(([=[
   <template name="binaries" path="[[[sys.pluginDir]]]/bin/Html5" include="%s%s.%s"/>
  ]=]):format(m,flavour,ext))
  if #flavour>0 then
    Export.remove(m.."."..ext)
    Export.move(m..flavour.."."..ext,m.."."..ext)
  end
  Export.callXml(([=[
    <template name="Activity" path="">
        <replacelist wildcards="gidloader.js">
            <append orig="/*GIDEROS_DYNLIB_PLUGIN*/" by="&quot;%s.%s&quot;,"/>
        </replacelist>
    </template>
  ]=]):format(m,zext))
  if pack then
  Export.callXml(([=[
        <exec cmd="[[[sys.toolsDir]]]/crunchme[[[sys.exeExtension]]]">
           <arg>-nostrip</arg>
           <arg>-i</arg>
           <arg>%s.%s</arg>
           <arg>%s.gidz</arg>
        </exec>
  ]=]):format(m,ext,m))
  Export.remove(m.."."..ext)
  end
end

function Html5Project.exportJS(m)
  Export.callXml(([=[
   <template name="binaries" path="[[[sys.pluginDir]]]/bin/Html5" include="%s.js"/>
  ]=]):format(m))
  Export.callXml(([=[
    <template name="Activity" path="">
        <replacelist wildcards="gidloader.js">
            <append orig="/*GIDEROS_JS_PLUGIN*/" by="&quot;%s.js&quot;,"/>
        </replacelist>
    </template>
  ]=]):format(m))
end

return Html5Project
