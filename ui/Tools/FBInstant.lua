local json=require "json"
local contexts={}
local contextid=0
local player={}
function player.getID() return JS.eval([[FBInstant.player.getID()]]) end
-- getSignedPlayerInfoAsync
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

--getStatsAsync
--setStatsAsync
--incrementStatsAsync

function player.getConnectedPlayersAsync(cb)
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval([[FBInstant.player.getConnectedPlayersAsync().then(function (players) {
    Module.GiderosJSEvent("FBInstantGPA",]]..contextid..[[,1,JSON.stringify(players));
  },function (err) {
    Module.GiderosJSEvent("FBInstantGPA",]]..contextid..[[,0,err.code);
  })]])
end


local context={}
function context.getID() return JS.eval([[FBInstant.context.getID()]]) end
function context.getType() return JS.eval([[FBInstant.context.getType()]]) end
function context.isSizeBetween(a,b) return json.decode(JS.eval([[FBInstant.context.isSizeBetween(]]..(a or "null")..","..(b or "null")..[[)]])) end
function context.switchAsync(id,cb) 
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval("FBInstant.context.switchAsync('"..id..[[').then(function () {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,1,"");
  },function (err) {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,0,err.code);
  })]])
end
function context.chooseAsync(opts,cb) 
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval("FBInstant.context.chooseAsync("..json.encode(opts or { })..[[).then(function () {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,1,"");
  },function (err) {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,0,err.code);
  })]])
end
function context.createhAsync(id,cb) 
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval("FBInstant.context.createAsync('"..id..[[').then(function () {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,1,"");
  },function (err) {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,0,err.code);
  })]])
end
function context.getPlayersAsync(cb)
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval([[FBInstant.context.getPlayersAsync().then(function (players) {
    Module.GiderosJSEvent("FBInstantGPA",]]..contextid..[[,1,JSON.stringify(players));
  },function (err) {
    Module.GiderosJSEvent("FBInstantGPA",]]..contextid..[[,0,err.code);
  })]])
end

--payments
-- getCatalogAsync
-- purchaseAsync
-- getPurchasesAsync
-- consumePurchaseAsync
-- onReady

local FBInstant={
  player=player,
  context=context
}

local onPause_=nil
function FBInstant.getLocale() return JS.eval([[FBInstant.getLocale()]]) end
function FBInstant.getPlatform() return JS.eval([[FBInstant.getPlatform()]]) end
function FBInstant.getSDKVersion() return JS.eval([[FBInstant.getSDKVersion()]]) end
-- initializeAsync (done by init code)
-- setLoadingProgress (done by init code)
function FBInstant.getSupportedAPIs()
  return json.decode(JS.eval("JSON.stringify(FBInstant.getSupportedAPIs())"))
end
function FBInstant.getEntryPointData() 
  return json.decode(JS.eval("JSON.stringify(FBInstant.getEntryPointData())"))
end
function FBInstant.getEntryPointAsync(cb) 
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval([[FBInstant.getEntryPointAsync().then(function (e) {
    Module.GiderosJSEvent("FBInstantGEA",]]..contextid..[[,1,e);
  })]])
end
function FBInstant.setSessionData(props) 
  JS.eval("FBInstant.setSessionData("..json.encode(props)..[[)]])
end
function FBInstant.shareAsync(props,cb) 
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval("FBInstant.shareAsync("..json.encode(props)..[[).then(function () {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,1,"");
  },function (err) {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,0,err.code);
  })]])
end
function FBInstant.updateAsync(props,cb) 
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval("FBInstant.updateAsync("..json.encode(props)..[[).then(function () {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,1,"");
  },function (err) {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,0,err.code);
  })]])
end
-- switchGameAsync
function FBInstant.quit() return JS.eval([[FBInstant.quit()]]) end
function FBInstant.logEvent(name,val,props) return JS.eval("FBInstant.logEvent('"..name.."',"..val..","..json.encode(props)..");") end
function FBInstant.onPause(f) onPause_=f end
JS.eval("FBInstant.onPause(function () { Module.GiderosJSEvent('FBInstantPause',0,0,''); })")

JS:addEventListener("FBInstantGPA",function (e)
  if contexts[e.context] then 
    if e.value>0 then contexts[e.context](json.decode(e.data),nil) 
    else contexts[e.context](nil,e.data) end
  end
  contexts[e.context]=nil
end)
JS:addEventListener("FBInstantGEA",function (e)
  if contexts[e.context] then contexts[e.context](e.data) end
  contexts[e.context]=nil
end)
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
JS:addEventListener("FBInstantPause",function (e) if onPause_ then onPause_() end end)
JS:addEventListener("FBInstantUpdAsync",function (e)
  if contexts[e.context] then contexts[e.context](e.value>0,e.data) end
  contexts[e.context]=nil
end)

--Ads support
FBInstant.Ad=Core.class(Object)
function FBInstant.Ad:init(aid)
  self.aid=aid
end
function FBInstant.Ad:loadAsync(cb)
  contextid=contextid+1
  contexts[contextid]={self,cb}
  JS.eval([[FBInstant.GiderosAds["]]..self.aid..[["].loadAsync().then(function () {
    Module.GiderosJSEvent("FBInstantAdsLA",]]..contextid..[[,1,"");
  },function () {
    Module.GiderosJSEvent("FBInstantAdsLA",]]..contextid..[[,0,"");
  })]])
end
function FBInstant.Ad:showAsync(cb)
  contextid=contextid+1
  contexts[contextid]={self,cb}
  JS.eval([[FBInstant.GiderosAds["]]..self.aid..[["].showAsync().then(function () {
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
    FBInstant.GiderosAds[pid]=ad;
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
    FBInstant.GiderosAds[pid]=ad;
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
  if contexts[e.context] then contexts[e.context][2](contexts[e.context][1],e.value>0) end
  contexts[e.context]=nil
end)
JS:addEventListener("FBInstantAdsSA",function (e)
  if contexts[e.context] then contexts[e.context][2](contexts[e.context][1],e.value>0) end
  contexts[e.context]=nil
end)


-- matchPlayerAsync
-- checkCanPlayerMatchAsync
-- getLeaderboardAsync

return FBInstant
  