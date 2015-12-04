window.fbAsyncInit = function() {
    FB.init({
    appId      : GiderosFB.appId,
    xfbml      : true,
    version    : 'v2.5',
    cookie     : true
    });
    GiderosFB.NeedInit=false;
    GiderosFB.evtLogin=true;
};

if (XMLHttpRequest.prototype.sendAsBinary === undefined ) {
 XMLHttpRequest.prototype.sendAsBinary = function(string) {
  var bytes = Array.prototype.map.call(string, function(c) {
   return c.charCodeAt(0) & 0xff;
  });
  this.send(new Uint8Array(bytes).buffer);
 };
}

GiderosFB={};
GiderosFB.evtLogin=false;
GiderosFB.NeedInit=true;

GiderosFB.ClickChain=window.onclick;
window.onclick=function()
{
 if (GiderosFB.evtLogin)
 {
    FB.login(function(response) {
       if (response.authResponse) {
          GiderosFB.accessToken=response.authResponse.accessToken;
          GiderosFB.expiresIn=response.authResponse.expiresIn;
          Module.ccall('gfacebook_onLoginComplete','number',[],[]);
       }
       else
          Module.ccall('gfacebook_onLoginError','number',['string'],[ "" ]);
    }, {scope: GiderosFB.loginScope}); 
    GiderosFB.evtLogin=false;
 }
 if (GiderosFB.ClickChain)
  GiderosFB.ClickChain();
}

GiderosFB.Init=function(){};
GiderosFB.Deinit=function(){};
GiderosFB.Login=function(appid,params)
{
 GiderosFB.appId=appid;
 GiderosFB.loginScope=params;
 if (GiderosFB.NeedInit) {
 (function(d, s, id){
      var js, fjs = d.getElementsByTagName(s)[0];
      if (d.getElementById(id)) {return;}
      js = d.createElement(s); js.id = id;
      js.src = "//connect.facebook.net/en_US/sdk.js";
      fjs.parentNode.insertBefore(js, fjs);
 }(document, 'script', 'facebook-jssdk'));
 }
 else
    GiderosFB.evtLogin=true; 
};

GiderosFB.Logout=function()
{
 FB.logout(function(response)
 {
  console.log(response);
       if (response) {
          GiderosFB.accessToken="";
          GiderosFB.expiresIn=0;
          Module.ccall('gfacebook_onLogoutComplete','number',[],[]);
       }
       else
          Module.ccall('gfacebook_onLogoutError','number',['string'],[ "" ]);
 });
};

GiderosFB.GetAccessToken=function()
{
 return GiderosFB.accessToken;
};

GiderosFB.GetExpirationDate=function()
{
 return GiderosFB.expiresIn;
};

GiderosFB.postMultipart=function(path,params,cb)
{
// this is the multipart/form-data boundary we'll use
 var boundary = '----ThisIsTheBoundary1234567890';
        
// let's encode our image file, which is contained in the var
 var formData = "";
 for (var p in params)
 {
  var v=params[p];
  formData+= '--' + boundary + '\r\n'
  if (v.name)
  {
   var mimeType="image/png";
   if ((v.type=="jpeg")||(v.type=="jpg"))
    mimeType="image/jpeg";
   formData += 'Content-Disposition: form-data; name="'+v.name+'"; filename="'+v.name+'.'+v.type+'"\r\n';
   formData += 'Content-Type: ' + mimeType + '\r\n\r\n';
   formData += v.data+'\r\n';
  }
  else
  {
   formData += 'Content-Type: text/plain; charset=UTF-8\r\n';
   formData += 'Content-Disposition: form-data; name="'+p+'"\r\n\r\n';
   formData += unescape(encodeURIComponent(v)) + '\r\n';
  }
 }
 formData += '--' + boundary + '--\r\n';
 var xhr = new XMLHttpRequest();
 xhr.open( 'POST', 'https://graph.facebook.com/'+path+'?access_token=' + GiderosFB.accessToken, true );
 xhr.onload = xhr.onerror = function() {
    cb( xhr.responseText );
 };

 xhr.setRequestHeader( "Content-Type", "multipart/form-data; boundary=" + boundary );
 xhr.sendAsBinary( formData );
};

GiderosFB.Request=function(path,method,params)
{
 var cb=function(response)
 {
  if (!response) {
          Module.ccall('gfacebook_onRequestError',null,['string','string'],
          [ path,"No response from service" ]);  
  }
  else if (response.error)
  {
          Module.ccall('gfacebook_onRequestError',null,['string','string'],
          [ path, response.error.message ]);  
  }
  else
  {
          Module.ccall('gfacebook_onRequestComplete',null,['string','string'],
          [ path, JSON.stringify(response) ]);  
  }  
 }; 
 if ((method=="post")&&params.source) 
  GiderosFB.postMultipart(path,params,cb);
 else
  FB.api(path,method,params,cb);
};

GiderosFB.Dialog=function(action,params)
{
 params.method=action;
 FB.ui(params,function(response)
 {
  if (!response) {
          Module.ccall('gfacebook_onDialogError',null,['string','string'],
          [ action,"No response from service" ]);  
  }
  else if (response.error)
  {
          Module.ccall('gfacebook_onDialogError',null,['string','string'],
          [action, response.error.message ]);  
  }
  else
  {
          Module.ccall('gfacebook_onDialogComplete',null,['string','string'],
          [ action, JSON.stringify(response) ]);  
  }  
 });
};

GiderosFB.Upload=function(path,orig)
{
};
