function AdsAdsense(){
    var isLoaded = false;
    var loadRequest = null;
    var ad = "adsense";
    
    //load adsense scripts
    (function(d, s, id, callback){
	      var js, fjs = d.getElementsByTagName(s)[0];
	      if (d.getElementById(id)) {return;}
	      js = d.createElement(s); js.id = id;
	      js.src = 'http://pagead2.googlesyndication.com/pagead/js/adsbygoogle.js';
          js.onreadystatechange = callback;
          js.onload = callback;
	      fjs.parentNode.insertBefore(js, fjs);
	 }(document, 'script', 'adsbygooglejs', function(){
         isLoaded = true;
         if(loadRequest){
            GiderosAds.onAdReceived(ad,loadRequest);
            loadRequest = null;
         }
     }));
    this.view = document.createElement("ins");
    this.view.className = "adsbygoogle";
    this.view.style.display = "inline-block";
    this.view.style.position = "absolute";
    this.view.style.zIndex = "1000";
    this.view.style.width = "300px";
    this.view.style.height = "250px";
    this.view.style.top = "0px";
    this.view.style.left = "0px";
    
    this.enableTesting = function(ad) {};
    this.setKey = function(params) {
        this.view.setAttribute("data-ad-client", params[0]);
        this.view.setAttribute("data-ad-slot", params[1]);
    };
    this.loadAd = function(params) {
        if(isLoaded && params && params[0])
            GiderosAds.onAdReceived(ad,params[0]);
        else if(params && params[0])
            loadRequest = params[0];
    };
    this.showAd = function(params) {
        this.loadAd(params);
        var size = [300, 250];
        if(params && params[0])
            size = params[0].split("x");
        size[0] = size[0] || 300;
        size[1] = size[1] || 250;
        this.view.style.width = size[0]+"px";
        this.view.style.height = size[1]+"px";
        
        if(!this.view.parentNode){
            document.body.appendChild(this.view);
            (adsbygoogle = window.adsbygoogle || []).push({});
            if(params && params[0])
                GiderosAds.onAdDisplayed(ad, params[0]);
        }
    };
    this.hideAd = function(params) {
        if(this.view.parentNode){
            this.view.parentNode.removeChild(this.view);
            if(params && params[0])
                GiderosAds.onAdDismissed(ad, params[0]);
        }
    };
    this.setAlignment = function(hor,ver) {
        if(hor == "left")
            this.view.style.left = "0px";
        else if(hor == "right")
            this.view.style.left = (window.innerWidth - parseInt(this.view.style.width))+"px";
        else
            this.view.style.left = ((window.innerWidth - parseInt(this.view.style.width))/2)+"px";
        
        if(ver == "top")
            this.view.style.top = "0px";
        else if(hor == "bottom")
            this.view.style.top = (window.innerHeight - parseInt(this.view.style.height))+"px";
        else
            this.view.style.top = ((window.innerHeight - parseInt(this.view.style.height))/2)+"px";
    };
    
    this.setX=function(v) {
        this.view.style.left = v + "px";
    };
    this.setY=function(v) {
        this.view.style.top = v + "px";
    };
    this.getX=function(ad) {
        return parseInt(this.view.style.left);
    };
    this.getY=function(ad) {
        return parseInt(this.view.style.top);
    };
    this.getWidth=function(ad) {
        return parseInt(this.view.style.width);
    };
    this.setHeight=function(ad) {
        return parseInt(this.view.style.height);
    };

    GiderosAds.frameworks[ad]=this;    
}

GiderosAds.frameworks["adsense"]=new AdsAdsense();    

