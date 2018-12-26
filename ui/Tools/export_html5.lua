--[[
Tools to simplify html5 exports
]]

Html5Project={
}

function Html5Project.export(m,flavour)
  local wasm=tonumber(Export.getProperty("project.html5_wasm"))>0
  local pack=tonumber(Export.getProperty("project.html5_pack"))>0
  flavour=flavour or ""
  local ext="js"
  if wasm then
    ext="wasm"
  end
  local zext=ext
  if pack then zext="gidz" end
  Export.callXml(([=[
   <template name="binaries" path="[[[sys.pluginDir]]]/bin/Html5" include="%s%s.%s"/>
  ]=]):format(m,flavour,ext))
  if #flavour>0 then
    Export.move(m..flavour.."."..ext,m.."."..ext)
  end
  Export.callXml(([=[
    <template name="Activity" path="">
        <replacelist wildcards="index.html">
            <append orig="/*GIDEROS_DYNLIB_PLUGIN*/" by="&quot;%s.%s&quot;,"/>
        </replacelist>
    </template>
  ]=]):format(m,zext))
  if pack then
  Export.callXml(([=[
        <exec>
          "[[[sys.giderosDir]]]/Tools/crunchme[[[sys.exeExtension]]]" -nostrip -i %s.%s %s.gidz
        </exec>
  ]=]):format(m,ext,m))
  Export.callXml(([=[
        <rm>%s.%s</rm>
  ]=]):format(m,ext))
  end
end

function Html5Project.exportJS(m)
  Export.callXml(([=[
   <template name="binaries" path="[[[sys.pluginDir]]]/bin/Html5" include="%s.js"/>
    <template name="Activity" path="">
        <replacelist wildcards="index.html">
            <append orig="/*GIDEROS_JS_PLUGIN*/" by="&quot;%s.js&quot;,"/>
        </replacelist>
    </template>
  ]=]):format(m,m))
end

return Html5Project
