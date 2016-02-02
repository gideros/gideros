GiderosAds={};
GiderosAds.evtShow=false;

GiderosAds.ClickChain=window.onclick;
window.onclick=function()
{
 if (GiderosAds.evtShow)
 {
    GiderosAds.evtShow=false;
 }
 if (GiderosAds.ClickChain)
  GiderosAds.ClickChain();
}

GiderosAds.Init=function(){};
GiderosAds.Deinit=function(){};

GiderosAds.frameworks = {};
function capitalizeFirstLetter(string) {
    return string.charAt(0).toUpperCase() + string.slice(1);
}

GiderosAds.Initialize=function(ad) {
/*	 (function(d, s, id, callback){
	      var js, fjs = d.getElementsByTagName(s)[0];
	      if (d.getElementById(id)) {return;}
	      js = d.createElement(s); js.id = id;
	      js.src = id+'.js';
          js.onreadystatechange = callback;
          js.onload = callback;
	      fjs.parentNode.insertBefore(js, fjs);
	 }(document, 'script', 'Ads'+capitalizeFirstLetter(ad), function(){
    if(!GiderosAds.frameworks[ad])        
            GiderosAds.frameworks[ad] = new window['Ads'+capitalizeFirstLetter(ad)]();
         Module.ccall('gads_onAdInitialized',null,['string'],[ ad ]);	
     })); */
     Module.ccall('gads_onAdInitialized',null,['string'],[ ad ]);	     
};

GiderosAds.Destroy=function(ad) 
{ 
/*    delete GiderosAds.frameworks[ad];
	 (function(d, s, id){
	      var e=d.getElementById(id);
	      if (e)
	    	  e.parentNode.removeNode(e);
	 }(document, 'script', 'Ads'+capitalizeFirstLetter(ad)));
	 */
};

GiderosAds.EnableTesting=function(ad) { 
    if(GiderosAds.frameworks[ad])
        GiderosAds.frameworks[ad].enableTesting();
};
GiderosAds.SetKey=function(ad,params) {
    if(GiderosAds.frameworks[ad])
        GiderosAds.frameworks[ad].setKey(params);
};
GiderosAds.LoadAd=function(ad,params) {
    if(GiderosAds.frameworks[ad])
        GiderosAds.frameworks[ad].loadAd(params);
};
GiderosAds.ShowAd=function(ad,params) {
    if(GiderosAds.frameworks[ad])
        GiderosAds.frameworks[ad].showAd(params);
};
GiderosAds.HideAd=function(ad,type) {
    if(GiderosAds.frameworks[ad])
        GiderosAds.frameworks[ad].hideAd(type);
};
GiderosAds.SetAlignment=function(ad,hor,ver) {
    if(GiderosAds.frameworks[ad])
        GiderosAds.frameworks[ad].setAlignment(hor,ver);
};
GiderosAds.SetX=function(ad,v) {
    if(GiderosAds.frameworks[ad])
        GiderosAds.frameworks[ad].setX(v);
};
GiderosAds.SetY=function(ad,v) {
    if(GiderosAds.frameworks[ad])
        GiderosAds.frameworks[ad].setY(v);
};
GiderosAds.GetX=function(ad) {
    if(GiderosAds.frameworks[ad])
        return GiderosAds.frameworks[ad].getX();
    return 0;
};
GiderosAds.GetY=function(ad) {
    if(GiderosAds.frameworks[ad])
        return GiderosAds.frameworks[ad].getY();
    return 0;
};
GiderosAds.GetWidth=function(ad) {
    if(GiderosAds.frameworks[ad])
        return GiderosAds.frameworks[ad].getWidth();
    return 0;
};
GiderosAds.GetHeight=function(ad) {
    if(GiderosAds.frameworks[ad])
        return GiderosAds.frameworks[ad].getHeight();
    return 0;
};

GiderosAds.onAdReceived=function(ad,type) {
    Module.ccall('gads_onAdReceived',null,['string','string'],[ ad,type ]);	
};
GiderosAds.onAdFailed=function(ad,type,error) {
    Module.ccall('gads_onAdFailed',null,['string','string','string'],[ ad,type,error ]);	
};
GiderosAds.onAdActionBegin=function(ad,type) {
    Module.ccall('gads_onAdActionBegin',null,['string','string'],[ ad,type ]);	
};
GiderosAds.onAdActionEnd=function(ad,type) {
    Module.ccall('gads_onAdActionEnd',null,['string','string'],[ ad,type ]);	
};
GiderosAds.onAdDismissed=function(ad,type) {
    Module.ccall('gads_onAdDismissed',null,['string','string'],[ ad,type ]);	
};
GiderosAds.onAdDisplayed=function(ad,type) {
    Module.ccall('gads_onAdDisplayed',null,['string','string'],[ ad,type ]);	
};
GiderosAds.onAdError=function(ad,error) {
    Module.ccall('gads_onAdError',null,['string','string'],[ ad,error ]);	
};

/*
GiderosAds.Login=function(appid,params)
{
};*/



