Module.preRun.push(function() {
 __ATPRERUN__.push(function() {
  Module.JSPlugins.forEach(function (p) {
   var xhr = new XMLHttpRequest;
   var tag = document.createElement("script");
   xhr.open("GET", p, true);
   xhr.onload = function (e) {
   if (xhr.readyState === 4) {
      if (xhr.status === 200) {
        tag.text = xhr.response;
        document.head.appendChild(tag);
      } else {
        console.error(xhr.response);
      }
      Module['removeRunDependency'](p);
    }
   };
   Module['addRunDependency'](p);
   xhr.send();
 }); });

	Module.setStatus("Loading application...");
	//Load GAPP if supplied
	Module.hasGApp=((typeof(GAPP_URL) != 'undefined')&&(GAPP_URL!=null));
	if (Module.hasGApp)
	 FS.createPreloadedFile("/", "main.gapp", GAPP_URL, true,false);
	  // Initial syncfs to get existing saved files.
	Module['addRunDependency']('syncfs');
     
	FS.mkdir('/documents');	
	FS.mount(IDBFS, {}, '/documents');
	FS.documentsOk=true;
	FS.syncfs(true, function(err) {
	    if (err) {
	        FS.unmount('/documents');
	        FS.rmdir('/documents');
	        console.warn("IndexedDB not available, persistant storage disabled");
	        FS.documentsOk=false;
	    }	    
	    Module['removeRunDependency']('syncfs');
	});
})
Module.registerPlugins=function()
{
	Module.dynamicLibraries.forEach(function (p) {
	    var pname=p.split(".")[0].split("/").pop();
	    var pentry="g_pluginMain_"+pname;
//	    var pp=getCFunc(pentry);
	    Module.ccall('main_registerPlugin','number', ['string'], [pentry]);
	    //g_registerPlugin(g_pluginMain_##symbol);
	    console.log(pname);
	});
}

Module.ghttpjs_urlload=function(url, request, rhdr, param, arg, free, onload, onerror, onprogress) {
    var _url = url;
    var _request = request;
    var _param = param;
            
    var http = new XMLHttpRequest();
    http.open(_request, _url, true);
    http.responseType = 'arraybuffer';
    for(var index in rhdr) { 
     if (rhdr.hasOwnProperty(index)) {
      var attr = rhdr[index];
      http.setRequestHeader(index,attr);
     }
    }                        
    var handle = Browser.getNextWgetRequestHandle();
                            
    // LOAD
    http.onload = function http_onload(e) {
    // if (http.status == 200 || _url.substr(0,4).toLowerCase() != "http") {
      //console.log("rhdr:"+http.getAllResponseHeaders());
      var hdrs=allocate(intArrayFromString(http.getAllResponseHeaders()), 'i8', ALLOC_STACK);
      var byteArray = new Uint8Array(http.response);
      var buffer = _malloc(byteArray.length);
      HEAPU8.set(byteArray, buffer);
      if (onload) Runtime.dynCall('viiiiii', onload, [handle, arg, buffer, byteArray.length, http.status,hdrs]);
      if (free) _free(buffer);
   /*  } else {
      console.log(url+" ERROR");
      if (onerror) Runtime.dynCall('viiii', onerror, [handle, arg, http.status, http.statusText]);
     }*/
     delete Browser.wgetRequests[handle];
    };
                                                                                                                
    // ERROR
    http.onerror = function http_onerror(e) {
     if (onerror) {
      Runtime.dynCall('viiii', onerror, [handle, arg, http.status, http.statusText]);
     }
     delete Browser.wgetRequests[handle];
    };
                                                                                                                                                     
    // PROGRESS
    http.onprogress = function http_onprogress(e) {
     if (onprogress) Runtime.dynCall('viiii', onprogress, [handle, arg, e.loaded, e.lengthComputable || e.lengthComputable === undefined ? e.total : 0]);
    };
                                                                                                                                                                        
    // ABORT
    http.onabort = function http_onabort(e) {
     delete Browser.wgetRequests[handle];
    };
                                                                                                                                                                                          
    // Useful because the browser can limit the number of redirection
    try {
     if (http.channel instanceof Ci.nsIHttpChannel)
     http.channel.redirectionLimit = 0;
    } catch (ex) { /* whatever */ }
                                                                                                                                                                                                                  
    if (_request == "POST") {
     http.send(_param);
    } else {
     http.send(null);
    }
                                                                                                                                                                                                                                                                  
    Browser.wgetRequests[handle] = http;
                                                                                                                                                                                                                                                                      
    return handle;
}

Module.checkALMuted=function()
{
 if (window.AL&&window.AL.currentContext&&(!window.AL.currentContext.unmuted))
 {
  // create empty buffer and play it
  var buffer = window.AL.currentContext.ctx.createBuffer(1, 1, 22050);
  var source = window.AL.currentContext.ctx.createBufferSource();
  source.buffer = buffer;
  source.connect(window.AL.currentContext.ctx.destination);
  source.noteOn(0);
       
        // by checking the play state after some time, we know if we're really unlocked
         setTimeout(function() {
           if((source.playbackState === source.PLAYING_STATE || source.playbackState === source.FINISHED_STATE)) {
              window.AL.currentContext.unmuted=true;
                }
           }, 0);
                 
 }
}
