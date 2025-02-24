Module.preRun
		.push(function() {
			__ATPRERUN__.push(function() {
				Module.JSPlugins.forEach(function(p) {
					var xhr = new Module.XMLHttpRequest;
					var tag = document.createElement("script");
					xhr.open("GET", p, true);
					xhr.onload = function(e) {
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
				});

				var loader=new Promise(function (resolve,reject) { resolve(); });
				Module['addRunDependency']("gidPlugins");
				var directLoader={
					'cache': {},
				}
				directLoader.findObject= function (name) {
					return name in directLoader.cache;
				}
				directLoader.readFile= function (name,enc) {
					return directLoader.cache[name];
				}
				Module.GiderosPlugins.forEach(function(p) {
					if (p.endsWith(".gidz")) {
							loader=loader.then(function () { console.log("Loading plugin:"+p); return JZPLoadPromise(p,"array");})
								.then(function (c) { 
									console.log("Instanciating plugin:"+p);
									directLoader.cache[p]=c;
									var so=loadDynamicLibrary(p,{global: true, nodelete: true, loadAsync:true, fs:directLoader});
									directLoader.cache[p]=undefined;
									return so;
								});
					} else {
						loader=loader.then(function () {
							return new Promise(function (resolve,reject) {
								console.log("Loading plugin:"+p); 
								var xhr = new Module.XMLHttpRequest;
								xhr.open("GET", p, true);
								xhr.onload = function(e) {
									if (xhr.readyState === 4) {
										if (xhr.status === 200) {
											resolve(xhr.response);
										} else {
											reject(xhr.response);
										}
									}
								};								
								xhr.responseType = 'arraybuffer';
								xhr.send();
							});
						});
						loader=loader.then(function (c) { 
							console.log("Instanciating plugin:"+p); 
							directLoader.cache[p]=new Uint8Array(c);
							var so=loadDynamicLibrary(p,{global: true, nodelete: true, loadAsync:true, fs:directLoader});
							directLoader.cache[p]=undefined;
							return so;
							});
					}
				});
				loader=loader.then(function () { console.log("Plugins loaded"); Module['removeRunDependency']("gidPlugins");});							
			});

			Module.setStatus("Loading application...");
			// Load GAPP if supplied
			Module.hasGApp = ((typeof (GAPP_URL) != 'undefined') && (GAPP_URL != null));
			if (Module.hasGApp) {
				if (GAPP_URL.endsWith(".gidz")) {
					Module['addRunDependency']("gidLoadGApp");
					var loader=JZPLoadPromise(GAPP_URL,"array")
					.then(function (c) { 
						console.log("Copying application");
						FS.createPreloadedFile("/", "main.gapp", c, true, false);
						console.log("Application ready");
						Module['removeRunDependency']("gidLoadGApp");
					});
				}
				else 
					FS.createPreloadedFile("/", "main.gapp", GAPP_URL, true, false);
			}
			// Initial syncfs to get existing saved files.
			Module['addRunDependency']('syncfs');

			FS.gidSyncing=false;
			FS.mkdir('/documents');
			FS.mount(IDBFS, {}, '/documents');
			FS.documentsOk = true;
			FS
					.syncfs(
							true,
							function(err) {
								if (err) {
									FS.unmount('/documents');
									FS.rmdir('/documents');
									console
											.warn("IndexedDB not available, persistant storage disabled");
									FS.documentsOk = false;
								}
								Module['removeRunDependency']('syncfs');
							});

			GiderosNetplayerWS = null;
		})
Module.registerPlugins = function() {
	Module.GiderosPlugins
			.forEach(function(p) {
				var pname = p.split(".")[0].split("/").pop();
				var pentry = "g_pluginMain_" + pname;
				// var pp=getCFunc(pentry);
				Module.ccall('main_registerPlugin', 'number', [ 'string', 'string' ],
						[ p, pentry ]);
				// g_registerPlugin(g_pluginMain_##symbol);
				console.log(pname);
			});
}

Module.gplatformLanguage = function() {
	var lang;
	if (navigator
			&& navigator.userAgent
			&& (lang = navigator.userAgent.match(/android.*\W(\w\w)-(\w\w)\W/i))) {
		lang = lang[1];
	}

	if (!lang && navigator) {
		if (navigator.language) {
			lang = navigator.language;
		} else if (navigator.browserLanguage) {
			lang = navigator.browserLanguage;
		} else if (navigator.systemLanguage) {
			lang = navigator.systemLanguage;
		} else if (navigator.userLanguage) {
			lang = navigator.userLanguage;
		}
		lang = lang.substr(0, 2);
	}
	return lang;
}

Module.gnetplayerSend = function(data) {
	if ((GiderosNetplayerWS != null) && (GiderosNetplayerWS.readyState == 1))
		GiderosNetplayerWS.send(data);
}

var gid_wget = {
		 wgetRequests: {},
		 nextWgetRequestHandle: 0,
		 getNextWgetRequestHandle: function() {
		  var handle = gid_wget.nextWgetRequestHandle;
		  gid_wget.nextWgetRequestHandle++;
		  return handle;
		 }
		};
Module.ghttpjs_urlload = function(url, request, rhdr, param, arg, free, onload,
		onerror, onprogress) {
	var _url = url;
	var _request = request;
	var _param = param;

	var http = new XMLHttpRequest();
	http.open(_request, _url, true);
	http.responseType = 'arraybuffer';
	
    while (rhdr) {    	
    	var rk=Module.getValue(rhdr,'*');
    	if (!rk) break;
    	rhdr+=4; //Assuming 32bit
    	var rv=Module.getValue(rhdr,'*');
    	rhdr+=4; //Assuming 32bit
		http.setRequestHeader(Module.UTF8ToString(rk), Module.UTF8ToString(rv));
    }
	var handle = gid_wget.getNextWgetRequestHandle();

	// LOAD
	http.onload = function http_onload(e) {
		// if (http.status == 200 || _url.substr(0,4).toLowerCase() != "http") {
		// console.log("rhdr:"+http.getAllResponseHeaders());
		var hdrs = allocate(intArrayFromString(http.getAllResponseHeaders()),ALLOC_STACK);
		var byteArray = new Uint8Array(http.response);
		var buffer = _malloc(byteArray.length);
		HEAPU8.set(byteArray, buffer);
		if (onload)
			dynCall('viiiiiii', onload, [ handle, arg, buffer,
					byteArray.length, http.status, hdrs,0 ]);
		if (free)
			_free(buffer);
		/*
		 * } else { console.log(url+" ERROR"); if (onerror)
		 * dynCall('viiii', onerror, [handle, arg, http.status,
		 * http.statusText]); }
		 */
		delete gid_wget.wgetRequests[handle];
	};

	// ERROR
	http.onerror = function http_onerror(e) {
		if (onerror) {
			dynCall('viiii', onerror, [ handle, arg, http.status,
					http.statusText ]);
		}
		delete gid_wget.wgetRequests[handle];
	};

	// PROGRESS
	http.onprogress = function http_onprogress(e) {
		if (onprogress)
					dynCall(
							'viiiiii',
							onprogress,
							[
									handle,
									arg,
									e.loaded,
									e.lengthComputable
											|| e.lengthComputable === undefined ? e.total
											: 0,0,0 ]);
	};

	// ABORT
	http.onabort = function http_onabort(e) {
		delete gid_wget.wgetRequests[handle];
	};

	// Useful because the browser can limit the number of redirection
	try {
		if (http.channel instanceof Ci.nsIHttpChannel)
			http.channel.redirectionLimit = 0;
	} catch (ex) { /* whatever */
	}

	if ((_request == "POST")||(_request == "PUT")) {
		http.send(_param);
	} else {
		http.send(null);
	}

	gid_wget.wgetRequests[handle] = http;

	return handle;
}

Module.ghttpjs_urlstream = function(url, request, rhdr, param, arg, free, onload,
		onerror, onprogress) {
	var _url = url;
	var _request = request;
	var _param = param;
	var handle=0;

	var gHeaders = new Headers();
	gHeaders.append('Content-Type', 'image/jpeg');

    while (rhdr) {    	
    	var rk=Module.getValue(rhdr,'*');
    	if (!rk) break;
    	rhdr+=4; //Assuming 32bit
    	var rv=Module.getValue(rhdr,'*');
    	rhdr+=4; //Assuming 32bit
		gHeaders.append(Module.UTF8ToString(rk), Module.UTF8ToString(rv));
    }

	var gInit = { method: _request,
            headers: gHeaders,
            mode: 'cors',
            cache: 'no-cache',
            body: _param};

	var http = new Request(_url, gInit);

	fetch(http).then(function(res) {
		  if (res) {
			  	var ahdr="";
			  	res.headers.forEach(function(value,key) {
			  	  ahdr=ahdr+key+": "+value+"\r\n";
			  	});
				var hdrs = allocate(intArrayFromString(ahdr), ALLOC_STACK);
				if (onload)
					dynCall('viiiiiii', onload, [ handle, arg, 0,
							0, res.status, hdrs,1 ]);
			  var reader = res.body.getReader();
			  let charsReceived = 0;
			  
			  reader.read().then(function processText({ done, value }) {
				    // Result objects contain two properties:
				    // done  - true if the stream has already given you all its data.
				    // value - some data. Always undefined when done is true.
				    if (done) {
						if (onload)
							dynCall('viiiiiii', onload, [ handle, arg, 0,
									0, res.status, hdrs,0 ]);
				      return;
				    }


					// value for fetch streams is a Uint8Array
				    charsReceived += value.length;
					var buffer = _malloc(value.length);
					HEAPU8.set(value, buffer);
					if (onprogress)
						dynCall(
								'viiiiii',
								onprogress,
								[
										handle,
										arg,
										charsReceived,
										0,buffer,value.length ]);
					_free(buffer);

				    // Read some more, and call this function again
				    return reader.read().then(processText);
				  });
		  }
		  else {
				if (onerror) {
					dynCall('viiii', onerror, [ handle, arg, res.status,
							res.statusText ]);
				}			  
		  }
	});

	return handle;
}

Module.checkALMuted = function() {
	var actx= _WebAudio_ALSoft
	if ((actx==undefined) && window.AL && window.AL.currentCtx)
		actx=window.AL.currentCtx.audioCtx;
	if (actx && (!Module.GidAudioUnlocked)) {		
		actx.resume();
		Module.GidAudioUnlocked = true;		
	}
}

Module.GiderosJSEvent = function(type, context, value, data, meta) {
	var etype = 'number';
	var len = data.length;
	var dataPtr;
	if (typeof meta != 'string') meta='';
	if (typeof data == 'string') {
		etype = 'string';
		len = -1;
	} else {
		if (len==undefined) len=data.byteLength;
		var dataPtr = Module._malloc(len);
		var dataHeap = new Uint8Array(Module.HEAPU8.buffer, dataPtr, len);
		dataHeap.set(data);
		data = dataPtr;
	}
	Module.ccall('JSNative_enqueueEvent', 'number', [ 'string', 'number',
			'number', etype, 'number','string' ], [ type, context, value, data, len, meta ]);
	if (etype == 'number')
		Module._free(dataPtr);
}

Module.GiderosPlayer_Play = function(project) {
	Module.ccall('JSPlayer_play', 'number', [ 'string' ], [ project ]);
}

Module.GiderosPlayer_Stop = function() {
	Module.ccall('JSPlayer_stop', 'number', [], []);
}

Module.GiderosPlayer_WriteFile = function(project, path, data) {
	var etype = 'number';
	var len = data.length;
	if (typeof data == 'string')
		etype = 'string';
	else {
		var dataPtr = Module._malloc(len);
		var dataHeap = new Uint8Array(Module.HEAPU8.buffer, dataPtr, len);
		dataHeap.set(data);
		data = dataPtr;
	}
	Module.ccall('JSPlayer_writeFile', 'number', [ 'string', 'string', etype,
			'number' ], [ project, path, data, len ]);
	if (etype == 'number')
		Module._free(dataPtr);
}

Module.JSCallJS = function(mtd,ja) {
	return JSON.stringify(eval(mtd).apply(null,JSON.parse(ja)));
}
