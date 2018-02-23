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

--Ads support
FBInstant.Ad=Core.class(Object)
function FBInstant.Ad:init(aid)
  self.aid=aid
end
function FBInstant.Ad:loadAsync(cb)
  contextid=contextid+1
  contexts[contextid]={self,cb}
  JS.eval([[FBInstant.GideroAds["]]..aid..[["].loadAsync().then(function () {
    Module.GiderosJSEvent("FBInstantAdsLA",]]..contextid..[[,1,"");
  },function () {
    Module.GiderosJSEvent("FBInstantAdsLA",]]..contextid..[[,0,"");
  }]])
end
function FBInstant.Ad:showAsync(cb)
  contextid=contextid+1
  contexts[contextid]={self,cb}
  JS.eval([[FBInstant.GideroAds["]]..aid..[["].showAsync().then(function () {
    Module.GiderosJSEvent("FBInstantAdsSA",]]..contextid..[[,1,"");
  },function () {
    Module.GiderosJSEvent("FBInstantAdsSA",]]..contextid..[[,0,"");
  })]])
end

function FBInstant.getInterstitialAdAsync(placement,cb)
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval([[FBInstant.getInterstitialAdAsync("]]..placement..[[").then(function (ad) {
    if (FBInstant.GiderosAds === undefined) { FBInstant.GiderosAds={} };
    var pid="I"+ad.getPlacementID();
    FBInstant.GideroAds[pid]=ad;
    Module.GiderosJSEvent("FBInstantAdsGIA",]]..contextid..[[,1,pid);
  },function (err) {
    Module.GiderosJSEvent("FBInstantAdsGIA",]]..contextid..[[,0,err.code);
  })]])
end
function FBInstant.getRewardedVideoAsync(placement,cb)
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval([[FBInstant.getRewardedVideoAsync("]]..placement..[[",).then(function (ad) {
    if (FBInstant.GiderosAds === undefined) { FBInstant.GiderosAds={} };
    var pid="V"+ad.getPlacementID();
    FBInstant.GideroAds[pid]=ad;
    Module.GiderosJSEvent("FBInstantAdsGIA",]]..contextid..[[,1,pid);
  },function (err) {
    Module.GiderosJSEvent("FBInstantAdsGIA",]]..contextid..[[,0,err.code);
  })]])
end
JS:addEventListener("FBInstantAdsGIA",function (e)
  if contexts[e.context] then 
    if e.value>0 then contexts[e.context](FBInstant.Ad.new(e.data),nil) 
    else contexts[e.context](nil,e.data) end
  end
  contexts[e.context]=nil
end)
JS:addEventListener("FBInstantAdsLA",function (e)
  if contexts[e.context] then contexts[e.context][1](contexts[e.context][0],e.value>0) end
  contexts[e.context]=nil
end)
JS:addEventListener("FBInstantAdsSA",function (e)
  if contexts[e.context] then contexts[e.context][1](contexts[e.context][0],e.value>0) end
  contexts[e.context]=nil
end)


return FBInstant
  