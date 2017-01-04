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

