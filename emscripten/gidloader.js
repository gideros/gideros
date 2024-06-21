JZPLoaded={}

function JZPLoadAsync(imageUrl, onprogress) {
	  return new Promise((resolve, reject) => {
	    var xhr = new XMLHttpRequest();
	    var notifiedNotComputable = false;

	    xhr.open('GET', imageUrl, true);
	    xhr.responseType = 'arraybuffer';

	    xhr.onprogress = function(ev) {
          onprogress(xhr,ev.loaded);
	    }

	    xhr.onloadend = function() {
	      if (!xhr.status.toString().match(/^2/)) {
	        reject(xhr);
	      } else {
	        var options = {}
	        var headers = xhr.getAllResponseHeaders();
	        var m = headers.match(/^Content-Type\:\s*(.*?)$/mi);
	        options.type="image/png";
	        /*if (m && m[1]) {
	          options.type = m[1];
	        }*/
	        resolve({ data: this.response, options:options});
	      }
	    }

	    xhr.send();
	  });
	}


JPZConvert=function (fl,cb,fmt)
{
var doc = document, canv = doc.createElement("canvas"), ctx = canv.getContext("2d");

// Create an image from the data
var img = new Image(), style = img.style;
style.position="absolute";
style.left="-17000px";
doc.body.appendChild(img);
img.onload = function()
{
    // Draw the loaded image to the canvas
    canv.width = this.offsetWidth;
    canv.height = this.offsetHeight;
    ctx.globalCompositeOperation='copy';
    ctx.drawImage(this, 0, 0);
    doc.body.removeChild(this);
    var bb;
    {
    	var w= ctx.getImageData(0, 0, canv.width, canv.height).data;
        var u=new Uint8Array(3*(w.length/4));
        for (k = 0, ki = 0; k < w.length; k ++)
        {
        	u[ki++]=w[k++];
        	u[ki++]=w[k++];
        	u[ki++]=w[k++];
        }
        var len=u[0];
        len|=u[1]<<8;
        len|=u[2]<<16;
        len|=u[3]<<24;
        u=u.slice(4,len+4);
        if (fmt=="array")
        {
            canv.width=1;
            canv.height=1;
            cb(u);
            return;
        }
        bb = new Blob([u]);
    }
    canv.width=1;
    canv.height=1;
    
    var f = new FileReader();
    f.onload = function(e) {
    	cb(e.target.result);
    };
    f.readAsText(bb);

    /*
    // Convert the drawn image to a string
    v = String.fromCharCode
    for (k = 0, dataStr = ""; k < w.length; k +=4)
    {
        b = w[k];
        if (b) dataStr += v(b);
        b = w[k+1];
        if (b) dataStr += v(b);
        b = w[k+2];
        if (b) dataStr += v(b);
    }

    // Run the script
    ev(dataStr);
    */
}

JZPLoadAsync(fl, (ctx,ratio) => {
	  var diff=ratio-(ctx.loadPoint||0);
	  ctx.loadPoint=ratio;
	  downloadProgress(diff)
	})
	.then(res => {
	  // Loading successfuly complete; set the image and probably do other stuff.
        var blob = new Blob([res.data], res.options);
		img.src = window.URL.createObjectURL(blob);
	}, xhr => {
	  // An error occured. We have the XHR object to see what happened.
		console.error("Failed to load:"+fl)
	});
}

JPZLoad=function (fl,ev,fmt)
{
	JPZConvert(fl,function (code)
	{
		 setTimeout(function() { 
			 ev(code);
			 if (typeof JZPLoaded[fl] =="function") {
					 JZPLoaded[fl]();
			 }
		},1);
	},fmt);
}

JZPLoadPromise=function (fl,fmt)
{
	return new Promise(function (resolve, reject)
	{
		JPZConvert(fl,function (code)
		{
			setTimeout(function() { 
				 resolve(code);
				 if (typeof JZPLoaded[fl] =="function") {
					 JZPLoaded[fl]();
				 }
			},1);
		},fmt);		
	});
}

JPZMALoad=function (fl,ev)
{
	JZPLoadAsync(fl, (ctx,ratio) => {
		  var diff=ratio-(ctx.loadPoint||0);
		  ctx.loadPoint=ratio;
		  downloadProgress(diff)
		})
		.then(res => {
			  var inStream = new LZMA.iStream(res.data);
			  var outStream = LZMA.decompressFile(inStream);
			  var result = outStream.toUint8Array();
			 setTimeout(function() { 
					 ev(result);
					 if (typeof JZPLoaded[fl] =="function") {
							 JZPLoaded[fl]();
					 }
				},1);
		}, xhr => {
		  // An error occured. We have the XHR object to see what happened.
			console.error("Failed to load:"+fl)
		});
}

<!-- Fix for WebKit/ iOS 11.3 issue -->
window.addEventListener('touchmove', function (event) {
	event.preventDefault();
	}, { passive: false, capture: false });    
var hasFBInstant = (typeof FBInstant !== 'undefined');
if (hasFBInstant) {
	FBInstant.initializeAsync();
}
var progressCur=0;
var progressMax=1000000;
function downloadProgress(diff) {
	progressCur+=diff;
	var pro=100*progressCur/progressMax;
    if (hasFBInstant)
		FBInstant.setLoadingProgress(pro);
    else {
        progressElement.value = pro;
        progressElement.hidden = false;
        canvas.hidden=true;	    	
    }
	//console.log("Progress "+progressCur+"/"+progressMax);
}
function getParameterByName(name, url) {
	if (!url) url = window.location.href;
    	name = name.replace(/[\[\]]/g, "\\$&");
        	var regex = new RegExp("[?&]" + name + "(=([^&#]*)|&|#|$)",
        	"i"),
            results = regex.exec(url);
            if (!results) return null;
            if (!results[2]) return '';
            return decodeURIComponent(results[2].replace(/\+/g," "));
     }      
  var GIDEROS_MEMORY_MB=128;
  var GAPP_URL=null;
  //GAPP_URL="gideros.GApp";
/* GIDEROS-PLAYER-START */
  var uarg=getParameterByName("GIDEROS_GAPP");
  if ((uarg!=null) && (uarg!=""))
    GAPP_URL=uarg;
/* GIDEROS-PLAYER-END */
  uarg=getParameterByName("GIDEROS_MEMORY");
  if ((uarg!=null) && (uarg!=""))
  	GIDEROS_MEMORY_MB=Number(uarg);
  var statusElement = document.getElementById('status');
  var progressElement = document.getElementById('progress');
  var spinnerElement = document.getElementById('spinner');
  var infopaneElement = document.getElementById('infopane');
  var errDetailsElement = document.getElementById('errDetails');
  var oopsElement = document.getElementById('oops');
  var webglLostElement = document.getElementById('webglLost');
  if (hasFBInstant) {
  	spinnerElement.hidden = true;
	infopaneElement.display="none";
  }

  var Module = {
    preRun: [],
    postRun: [],
    preInit: [],
    INITIAL_MEMORY: GIDEROS_MEMORY_MB*1024*1024,
    GiderosPlugins: [ /*GIDEROS_DYNLIB_PLUGIN*/ null ],
    JSPlugins: [ /*GIDEROS_JS_PLUGIN*/ null ],
    requestFile: function (file) { return 0; },
    print: (function() {
      var element = document.getElementById('output');
      if (element) element.value = ''; // clear browser cache
      return function(text) {
        if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
        // These replacements are necessary if you render to raw HTML
        //text = text.replace(/&/g, "&amp;");
        //text = text.replace(/</g, "&lt;");
        //text = text.replace(/>/g, "&gt;");
        //text = text.replace('\n', '<br>', 'g');
        console.log(text);
        if (element) {
          element.value += text + "\n";
          element.scrollTop = element.scrollHeight; // focus on bottom
        }
      };
    })(),
    printErr: function(text) {
      if (arguments.length > 1) text = Array.prototype.slice.call(arguments).join(' ');
      if (0) { // XXX disabled for safety typeof dump == 'function') {
        dump(text + '\n'); // fast, straight to the real console
      } else {
          console.error(text);
    	  Module.onAbort(text);
      }
    },
    luaPrint: function (text) { Module.print(text); },
    luaError: function (text) { Module.giderosHadLuaError=text; Module.print(text); },
    canvas: (function() {
      var canvas = document.getElementById('canvas');

      // As a default initial behavior, pop up an alert when webgl context is lost. To make your
      // application robust, you may want to override this behavior before shipping!
      // See http://www.khronos.org/registry/webgl/specs/latest/1.0/#5.15.2
      // canvas.addEventListener("webglcontextlost", function(e) { alert('WebGL context lost. You will need to reload the page.'); e.preventDefault(); }, false);

      return canvas;
    })(),
    uploadCrash: function (type,detail) {
  		var CRASH_URL='';
  		if (CRASH_URL.startsWith('http')) { //Send error report
  			var crash={
  				app: 'Gideros',
  				type: type,
  				detail: detail
      		};
  			var http = new XMLHttpRequest();
  			http.open("POST", CRASH_URL, true);
  			http.setRequestHeader("Content-Type","application/json");
  			http.send(JSON.stringify(crash));      			
  		}        	
    },
    showError: function(type,detail) {
	    if (hasFBInstant) //start game to show canvas
        	FBInstant.startGameAsync();
    	if ((type=="genErr")&&(Module.giderosHadLuaError)) {
    		type="luaErr";
    		detail=Module.giderosHadLuaError;
    	}
    	spinnerElement.hidden = true;
  		infopaneElement.style.display="none";
  		canvas.hidden = true;  
  		oopsElement.style.display="block";
  		errDetailsElement.innerHTML=detail;
  		document.getElementById(type).style.display="block";
  		Module.uploadCrash(type,detail);
    },
    showGLContextLost: function(lost) {
    	spinnerElement.hidden = true;
  		infopaneElement.style.display="none";
  		if (lost>0) {
	  		canvas.hidden = true;  
	  		webglLostElement.style.display="flex";	
		}
		else {
	  		canvas.hidden = false;  
	  		webglLostElement.style.display="none";				
		} 
    },
    onAbort: function (what) {
    	if (what.includes('ALLOW_MEMORY_GROWTH=1'))
    		Module.showError("memoryErr",what);
    	else if (what.includes('-s WASM=0'))
    		Module.showError("featErr","Your browser doesn't support WebAssembly technology");
    	else if (what.includes('failed to asynchronously prepare wasm'))
    		Module.showError("featErr","There was an issue with your browser's WebAssembly support:\n"+what);
    	else if (what.includes('_glGetString'))
    		Module.showError("featErr","Your browser's WebGL doesn't support this app");
    	else if (what.includes('--no-heap-copy')) {} //Ignore (mem growth related)
    	else if (what.includes('-s DEMANGLE_SUPPORT=1')) {} //Ignore (atexit related)
    	else if (what.includes('EXIT_RUNTIME to 1')) {} //Ignore (gvfs atexit)       
    	else if (what.includes('dependency:')||what.includes('dependencies:')||what.includes('(end of list)')) {} //Ignore (loading)       
    	else if (what.includes('Real sample count')) {} //Ignore (MP3)
 	  	else if (what.includes('Premature end of JPEG file')) {} //Ignore (JPEG end)
     	else if (what.includes('iCCP: known incorrect sRGB profile')) {} //Ignore (libpng)
    	else if (what.includes('Interlace handling should be turned on when using png_read_image')) {} //Ignore (Interlace handling)
    	else if (what.trim().length < 2) {} //Ignore
    	else if (what.includes('stub instead of')) {} //Stubs warnings
    	else if (what.includes('DISABLE_DEPRECATED_FIND_EVENT_TARGET_BEHAVIOR=1')) {} //Event changing warnings   
    	else if (what.includes('wasm streaming compile failed')||
    			what.includes('falling back')) {} //Ignore (server not supplying right WASM MIME) 
		else        		
    		Module.showError("genErr",what);
    },
    setStatus: function(text) {
      if (!Module.setStatus.last) Module.setStatus.last = { time: Date.now(), text: '' };
      if (text === Module.setStatus.text) return;
        progressElement.value = null;
        progressElement.hidden = true;
        if (text=="Running") {
        	spinnerElement.hidden = true;
        	infopaneElement.display="none";
           	canvas.hidden = false;
      	}
      statusElement.innerHTML = text;
    },
    totalDependencies: 0,
    monitorRunDependencies: function(left) {
      this.totalDependencies = Math.max(this.totalDependencies, left);
      //Module.setStatus(left ? 'Preparing... (' + (this.totalDependencies-left) + '/' + this.totalDependencies + ')' : 'All downloads complete.');
    }
  };
  Module.GiderosPlugins.pop();
  Module.JSPlugins.pop();
  Module.XMLHttpRequest= function(flags) {
      var req = new XMLHttpRequest(flags);
      req._curLoaded=0;
      req.onprogress=function (event) {
          req.onprogress=function (event) {
 		        var diff=event.loaded-req._curLoaded;
 		        req._curLoaded=event.loaded;
				downloadProgress(diff);
        }
      }
      return req;
	  };
 Module.setStatus('Preparing...');
  window.addEventListener('click',function(){
      window.focus();
  });    
  window.onerror = function() {
    Module.setStatus('Exception thrown, see JavaScript console');
    spinnerElement.style.display = 'none';
    Module.setStatus = function(text) {
      if (text) Module.printErr('[post-exception status] ' + text);
    };
    //Module.showError("genErr",Module.giderosErrorLog);
  };

