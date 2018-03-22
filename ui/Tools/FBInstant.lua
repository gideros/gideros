local json=require "json"
local contexts={}
local contextid=0
local player={}
function player.getID() return JS.eval([[FBInstant.player.getID()]]) end
function player.getSignedPlayerInfoAsync(metadata)
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval("FBInstant.player.getSignedPlayerInfoAsync('"..metadata..[[').then(function (stats) {
    var s={ id: stats.getPlayerID(), signature: stats.getSignature() };
    Module.GiderosJSEvent("FBInstantGSPIA",]]..contextid..[[,1,JSON.stringify(s));
  },function (err) {
    Module.GiderosJSEvent("FBInstantGSPIA",]]..contextid..[[,0,err.code);
  })]])
end
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

function player.getStatsAsync(props,cb) 
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval("FBInstant.player.getStatsAsync("..json.encode(props)..[[).then(function (stats) {
    Module.GiderosJSEvent("FBInstantGPA",]]..contextid..[[,1,JSON.stringify(stats));
  },function (err) {
    Module.GiderosJSEvent("FBInstantGPA",]]..contextid..[[,0,err.code);
  })]])
end
function player.setStatsAsync(props,cb) 
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval("FBInstant.player.setStatsAsync("..json.encode(props)..[[).then(function () {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,1,"");
  },function (err) {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,0,err.code);
  })]])
end
function player.incrementStatsAsync(props,cb) 
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval("FBInstant.player.incrementStatsAsync("..json.encode(props)..[[).then(function () {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,1,"");
  },function (err) {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,0,err.code);
  })]])
end

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

local payments={}
function payments.getCatalogAsync(cb)
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval([[FBInstant.payment.getCatalogAsync().then(function (catalog) {
    Module.GiderosJSEvent("FBInstantGPA",]]..contextid..[[,1,JSON.stringify(catalog));
  },function (err) {
    Module.GiderosJSEvent("FBInstantGPA",]]..contextid..[[,0,err.code);
  })]])
end
function payments.purchaseAsync(purchase,cb)
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval([[FBInstant.payment.purchaseAsync(]]..json.encode(purchase or { })..[[).then(function (purchase) {
    Module.GiderosJSEvent("FBInstantGPA",]]..contextid..[[,1,JSON.stringify(purchase));
  },function (err) {
    Module.GiderosJSEvent("FBInstantGPA",]]..contextid..[[,0,err.code);
  })]])
end
function payments.getPurchasesAsync(cb)
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval([[FBInstant.payment.getPurchasesAsync().then(function (purchases) {
    Module.GiderosJSEvent("FBInstantGPA",]]..contextid..[[,1,JSON.stringify(purchases));
  },function (err) {
    Module.GiderosJSEvent("FBInstantGPA",]]..contextid..[[,0,err.code);
  })]])
end
function payments.consumePurchaseAsync(purchaseId,cb) 
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval("FBInstant.payment.consumePurchaseAsync('"..purchaseId..[[').then(function () {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,1,"");
  },function (err) {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,0,err.code);
  })]])
end
local onReady_=nil
function payments.onReady(f) onReady_=f end
JS.eval("FBInstant.payments.onReady(function () { Module.GiderosJSEvent('FBInstantPaymentsReady',0,0,''); })")
JS:addEventListener("FBInstantPaymentsReady",function (e) if onReady_ then onReady_() end end)

local FBInstant={
  player=player,
  context=context,
  payments=payments
}

local onPause_=nil
function FBInstant.getLocale() return JS.eval([[FBInstant.getLocale()]]) end
function FBInstant.getPlatform() return JS.eval([[FBInstant.getPlatform()]]) end
function FBInstant.getSDKVersion() return JS.eval([[FBInstant.getSDKVersion()]]) end
-- initializeAsync (done by init code)
function FBInstant.setLoadingProgress(pct)
  JS.eval("FBInstant.setLoadingProgress("..pct..")")
end
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
function FBInstant.startGameAsync(cb) 
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval([[FBInstant.startGameAsync().then(function () {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,1,"");
  },function (err) {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,0,err.code);
  })]])
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
function FBInstant.switchGameAsync(gameid,cb) 
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval("FBInstant.switchGameAsync('"+gameid+[[').catch(function (e) {
    Module.GiderosJSEvent("FBInstantGEA",]]..contextid..[[,0,e.code);
  })]])
end
function FBInstant.quit() return JS.eval([[FBInstant.quit()]]) end
function FBInstant.logEvent(name,val,props) return JS.eval("FBInstant.logEvent('"..name.."',"..val..","..json.encode(props)..");") end
function FBInstant.onPause(f) onPause_=f end
JS.eval("FBInstant.onPause(function () { Module.GiderosJSEvent('FBInstantPause',0,0,''); })")
JS:addEventListener("FBInstantPause",function (e) if onPause_ then onPause_() end end)

JS:addEventListener("FBInstantGPA",function (e)
  if contexts[e.context] then 
    if e.value>0 then contexts[e.context](json.decode(e.data),nil) 
    else contexts[e.context](nil,e.data) end
  end
  contexts[e.context]=nil
end)
JS:addEventListener("FBInstantGSPIA",function (e)
  if contexts[e.context] then 
    if e.value>0 then
      local sa=json.decode(e.data)
      sa.getPlayerID=function() return sa.id end
      sa.getSignature=function() return sa.signature end
      contexts[e.context](sa,nil) 
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
  },function (err) {
    Module.GiderosJSEvent("FBInstantAdsLA",]]..contextid..[[,0,err.code);
  })]])
end
function FBInstant.Ad:showAsync(cb)
  contextid=contextid+1
  contexts[contextid]={self,cb}
  JS.eval([[FBInstant.GiderosAds["]]..self.aid..[["].showAsync().then(function () {
    Module.GiderosJSEvent("FBInstantAdsSA",]]..contextid..[[,1,"");
  },function (err) {
    Module.GiderosJSEvent("FBInstantAdsSA",]]..contextid..[[,0,err.code);
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
  if contexts[e.context] then contexts[e.context][2](contexts[e.context][1],e.value>0,e.data) end
  contexts[e.context]=nil
end)
JS:addEventListener("FBInstantAdsSA",function (e)
  if contexts[e.context] then contexts[e.context][2](contexts[e.context][1],e.value>0,e.data) end
  contexts[e.context]=nil
end)

local function escapeString(s)
 return s:gsub("\"","\\\"")
end

function FBInstant.matchPlayerAsync(room,cb) 
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval("FBInstant.matchPlayerAsync('"..room..[[').then(function () {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,1,"");
  },function (err) {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,0,err.code);
  })]])
end
function FBInstant.checkCanPlayerMatchAsync(cb) 
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval([[FBInstant.checkCanPlayerMatchAsync().then(function () {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,1,"");
  },function (err) {
    Module.GiderosJSEvent("FBInstantUpdAsync",]]..contextid..[[,0,err.code);
  })]])
end

FBInstant.LeaderboardEntry=Core.class(Object)
function FBInstant.LeaderboardEntry:init(entry)
    self.entry=entry
end
function FBInstant.LeaderboardEntry:getScore() return self.entry.score end
function FBInstant.LeaderboardEntry:getFormattedScore() return self.entry.formattedScore end
function FBInstant.LeaderboardEntry:getTimestamp() return self.entry.timestamp end
function FBInstant.LeaderboardEntry:getRank() return self.entry.rank end
function FBInstant.LeaderboardEntry:getExtraData() return self.entry.extraData end
function FBInstant.LeaderboardEntry:getPlayer()
  player=self.player
  return { 
    getName=function() return player.name end,
    getPhoto=function() return player.photo end,
    getId=function() return player.id end,
    }
end

FBInstant.Leaderboard=Core.class(Object)
function FBInstant.Leaderboard:init(aid)
  self.aid=aid
end
function FBInstant.Leaderboard:getName()
  return self.aid
end
function FBInstant.Leaderboard:getContextID()
  return JS.eval([[FBInstant.GiderosLdb["]]..self.aid..[["].getContextID()]])
end
function FBInstant.Leaderboard:getEntryCountAsync(cb)
  contextid=contextid+1
  contexts[contextid]={self,cb}
  JS.eval([[FBInstant.GiderosLdb["]]..self.aid..[["].getEntryCountAsync().then(function (count) {
    Module.GiderosJSEvent("FBInstantLdbGECA",]]..contextid..[[,1,""+count);
  },function (err) {
    Module.GiderosJSEvent("FBInstantLdbGECA",]]..contextid..[[,0,err.code);
  })]])
end
function FBInstant.Leaderboard:setScoreAsync(score,data,cb)
  contextid=contextid+1
  contexts[contextid]={self,cb}
  JS.eval([[FBInstant.GiderosLdb["]]..self.aid..[["].setScoreAsync(]]..score..[[,']]..escapeString(data)..[[').then(function (entry) {
    var e={ score: entry.getScore(), formattedScore: entry.getFormattedScore(), timestamp: entry.getTimestamp(), rank: entry.getRank(), extraData: entry.getExtraData(),
            player: { name: entry.getPlayer().getName(), photo: entry.getPlayer().getPhoto(), id: entry.getPlayer().getID() }};
    Module.GiderosJSEvent("FBInstantLdbSSA",]]..contextid..[[,1,JSON.stringify(e));
  },function (err) {
    Module.GiderosJSEvent("FBInstantLdbSSA",]]..contextid..[[,0,err.code);
  })]])
end
function FBInstant.Leaderboard:getPlayerEntryAsync(cb)
  contextid=contextid+1
  contexts[contextid]={self,cb}
  JS.eval([[FBInstant.GiderosLdb["]]..self.aid..[["].getPlayerEntryAsync().then(function (entry) {
    var e={ score: entry.getScore(), formattedScore: entry.getFormattedScore(), timestamp: entry.getTimestamp(), rank: entry.getRank(), extraData: entry.getExtraData(),
            player: { name: entry.getPlayer().getName(), photo: entry.getPlayer().getPhoto(), id: entry.getPlayer().getID() }};
    Module.GiderosJSEvent("FBInstantLdbSSA",]]..contextid..[[,1,JSON.stringify(e));
  },function (err) {
    Module.GiderosJSEvent("FBInstantLdbSSA",]]..contextid..[[,0,err.code);
  })]])
end
function FBInstant.Leaderboard:getEntriesAsync(count,offset,cb)
  count=count or 10
  offset=offset or 0
  contextid=contextid+1
  contexts[contextid]={self,cb}
  JS.eval([[FBInstant.GiderosLdb["]]..self.aid..[["].getEntriesAsync(]]..count..","..offset..[[).then(function (entries) {
    var ea=[];
    for (k=0;k<entries.length;k++) {
      var entry=entries[k];
      var e={ score: entry.getScore(), formattedScore: entry.getFormattedScore(), timestamp: entry.getTimestamp(), rank: entry.getRank(), extraData: entry.getExtraData(),
            player: { name: entry.getPlayer().getName(), photo: entry.getPlayer().getPhoto(), id: entry.getPlayer().getID() }};
      ea.push(e);
    }
    Module.GiderosJSEvent("FBInstantLdbGEA",]]..contextid..[[,1,JSON.stringify(ea));
  },function (err) {
    Module.GiderosJSEvent("FBInstantLdbGEA",]]..contextid..[[,0,err.code);
  })]])
end

function FBInstant.getLeaderboardAsync(name,cb)
  contextid=contextid+1
  contexts[contextid]=cb
  JS.eval([[FBInstant.getLeaderboardAsync("]]..name..[[").then(function (ldb) {
    if (FBInstant.GiderosLdb === undefined) { FBInstant.GiderosLdb={} };
    var pid=ldb.getName();
    FBInstant.GiderosLdb[pid]=ldb;
    Module.GiderosJSEvent("FBInstantLdbGLA",]]..contextid..[[,1,pid);
  },function (err) {
    Module.GiderosJSEvent("FBInstantLdbGLA",]]..contextid..[[,0,err.code);
  })]])
end
JS:addEventListener("FBInstantLdbSSA",function (e)
  if contexts[e.context] then
    local d=e.data
    if e.value>0 then d=FBInstant.LeaderboardEntry.new(json.decode(d)) end 
    contexts[e.context][2](contexts[e.context][1],e.value>0,d) 
  end
  contexts[e.context]=nil
end)
JS:addEventListener("FBInstantLdbGEA",function (e)
  if contexts[e.context] then
    local d=e.data
    if e.value>0 then
      d={}
      for k,v in ipairs(json.decode(e.data)) do table.insert(d,FBInstant.LeaderboardEntry.new(v)) end 
    end 
    contexts[e.context][2](contexts[e.context][1],e.value>0,d) 
  end
  contexts[e.context]=nil
end)
JS:addEventListener("FBInstantLdbGECA",function (e)
  if contexts[e.context] then
    local d=e.data
    if e.value>0 then d=tonumber(d) end 
    contexts[e.context][2](contexts[e.context][1],e.value>0,d) 
  end
  contexts[e.context]=nil
end)
JS:addEventListener("FBInstantLdbGLA",function (e)
  if contexts[e.context] then 
    if e.value>0 then contexts[e.context](FBInstant.Leaderboard.new(e.data),nil) 
    else contexts[e.context](nil,e.data) end
  end
  contexts[e.context]=nil
end)

return FBInstant
  