local json=require "json"
local contexts={}
local contextid=0
local player={}
function player.getID() return JS.eval([[FBInstant.player.getID()]]) end
function player.getName() return JS.eval([[FBInstant.player.getName()]]) end
function player.getPhoto() return JS.eval([[FBInstant.player.getPhoto()]]) end
function player.getDataAsync(keys,cb)
  contextid=contextid+1
  contexts[contextid]=cb
  local ks=""
  for _,k in ipairs(keys) do ks=ks..",'"..k.."'" end  
  JS.eval([[FBInstant.player.getDataAsync([]]..ks:sub(2,-1)..[[]).then(function (data) {
    Module.GiderosJSEvent("FBInstantGDA",]]..contextid..[[,0,JSON.stringify(data));
  })]])
end

function player.setDataAsync(values,cb)
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval([[FBInstant.player.setDataAsync(]]..json.encode(values)..[[).then(function () {
    Module.GiderosJSEvent("FBInstantSDA",]]..contextid..[[,0,"");
  })]])
end

function player.flushDataAsync(cb)
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval([[FBInstant.player.flushDataAsync().then(function () {
    Module.GiderosJSEvent("FBInstantFDA",]]..contextid..[[,0,"");
  })]])
end

local context={}
function context.getID() return JS.eval([[FBInstant.context.getID()]]) end
function context.getType() return JS.eval([[FBInstant.context.getType()]]) end
function context.isSizeBetween(a,b) return json.decode(JS.eval([[FBInstant.context.isSizeBetween(]]..(a or "null")..","..(b or "null")..[[)]])) end

local FBInstant={
  player=player,
  context=context
}

function FBInstant.getLocale() return JS.eval([[FBInstant.getLocale()]]) end
function FBInstant.getPlatform() return JS.eval([[FBInstant.getPlatform()]]) end
function FBInstant.getSDKVersion() return JS.eval([[FBInstant.getSDKVersion()]]) end

JS:addEventListener("FBInstantGDA",function (e)
  if contexts[e.context] then contexts[e.context](json.decode(e.data)) end
  contexts[e.context]=nil
end)
JS:addEventListener("FBInstantSDA",function (e)
  if contexts[e.context] then contexts[e.context]() end
end)
JS:addEventListener("FBInstantFDA",function (e)
  if contexts[e.context] then contexts[e.context]() end
end)

return FBInstant
  