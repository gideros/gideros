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

GiderosAds.Initialize=function(ad) {
	 (function(d, s, id){
	      var js, fjs = d.getElementsByTagName(s)[0];
	      if (d.getElementById(id)) {return;}
	      js = d.createElement(s); js.id = id;
	      js.src = 'Ads'+ad+'.js';
	      fjs.parentNode.insertBefore(js, fjs);
	 }(document, 'script', 'Ads'+ad));
};

GiderosAds.Destroy=function(ad) 
{ 
	 (function(d, s, id){
	      var e=d.getElementById(id);
	      if (e)
	    	  e.parentNode.removeNode(e);
	 }(document, 'script', 'Ads'+ad));
};

GiderosAds.EnableTesting=function(ad) { };
GiderosAds.SetKey=function(ad,params) { };
GiderosAds.LoadAd=function(ad,params) { };
GiderosAds.ShowAd=function(ad,params) { };
GiderosAds.HideAd=function(ad,type) { };
GiderosAds.SetAlignment=function(ad,hor,ver) { };
GiderosAds.SetX=function(ad,v) { };
GiderosAds.SetY=function(ad,v) { };
GiderosAds.GetX=function(ad) { };
GiderosAds.GetY=function(ad) { };
GiderosAds.GetWidth=function(ad) { };
GiderosAds.GetHeight=function(ad) { };

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
