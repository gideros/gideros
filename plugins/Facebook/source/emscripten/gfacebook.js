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

GiderosFB={};
GiderosFB.evtLogin=false;
GiderosFB.NeedInit=true;

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

GiderosFB.Request=function(path,method,params)
{
 FB.api(path,method,params,function(response)
 {
  console.log(response);
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
 });
};

GiderosFB.Dialog=function(action,params)
{
 params.method=action;
 FB.ui(params,function(response)
 {
  console.log(response);
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
