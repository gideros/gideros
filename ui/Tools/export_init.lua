--[[
Called before the first LUA tag of a .gplugin or .gexport file
Includes generic routines for export system
]]

function string.starts(String,Start)
return string.sub(String,1,string.len(Start))==Start
end

function string.ends(String,End)
return End=='' or string.sub(String,-string.len(End))==End
end

-- Allows plugins or scripts to defer processing until export is about to finish
Export._finishList={}
function Export.registerPreFinish(f)
  table.insert(Export._finishList,1,f)
end

function Export.registerPostFinish(f)
  table.insert(Export._finishList,f)
end

-- This function is called by gideros export 
function Export._finish()
  for _,f in ipairs(Export._finishList) do f() end
end