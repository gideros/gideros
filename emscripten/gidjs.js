Module.preRun
		.push(function() {
			__ATPRERUN__.push(function() {
				Module.JSPlugins.forEach(function(p) {
					var xhr = new XMLHttpRequest;
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
				var r=Module['read'];
				Module['read']=function (l) {
					if (l.startsWith("local:"))
						return l.substring(6);
					else
						return r(l);
				}
				Module.GiderosPlugins.forEach(function(p) {
					if (p.endsWith(".gidz")) {
						if (Module['wasmBinary'])
							loader=loader.then(function () { console.log("Loading plugin:"+p); return JZPLoadPromise(p,"array");})
								.then(function (c) { console.log("Instanciating plugin:"+p); return loadDynamicLibrary(c,{global: true, nodelete: true, loadAsync:true}); });
						else
							loader=loader.then(function () {console.log("Loading plugin:"+p); return JZPLoadPromise(p);})
							.then(function (c) { console.log("Instanciating plugin:"+p); return loadDynamicLibrary("local:"+c); });
					} else {
						loader=loader.then(function () {
							return new Promise(function (resolve,reject) {
								console.log("Loading plugin:"+p); 
								var xhr = new XMLHttpRequest;
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
								if (Module['wasmBinary'])
									xhr.responseType = 'arraybuffer';
								xhr.send();
							});
						});
						if (Module['wasmBinary'])
							loader=loader.then(function (c) { console.log("Instanciating plugin:"+p); return loadDynamicLibrary(c,{global: true, nodelete: true, loadAsync:true}); });
						else
							loader=loader.then(function (c) { console.log("Instanciating plugin:"+p); return loadDynamicLibrary("local:"+c); });
					}
				});
				loader=loader.then(function () { console.log("Plugins loaded"); Module['removeRunDependency']("gidPlugins");});							
			});

			Module.setStatus("Loading application...");
			// Load GAPP if supplied
			Module.hasGApp = ((typeof (GAPP_URL) != 'undefined') && (GAPP_URL != null));
			if (Module.hasGApp)
				FS.createPreloadedFile("/", "main.gapp", GAPP_URL, true, false);
			// Initial syncfs to get existing saved files.
			Module['addRunDependency']('syncfs');

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
				Module.ccall('main_registerPlugin', 'number', [ 'string' ],
						[ pentry ]);
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

Module.ghttpjs_urlload = function(url, request, rhdr, param, arg, free, onload,
		onerror, onprogress) {
	var _url = url;
	var _request = request;
	var _param = param;

	var http = new XMLHttpRequest();
	http.open(_request, _url, true);
	http.responseType = 'arraybuffer';
	for ( var index in rhdr) {
		if (rhdr.hasOwnProperty(index)) {
			var attr = rhdr[index];
			http.setRequestHeader(index, attr);
		}
	}
	var handle = Browser.getNextWgetRequestHandle();

	// LOAD
	http.onload = function http_onload(e) {
		// if (http.status == 200 || _url.substr(0,4).toLowerCase() != "http") {
		// console.log("rhdr:"+http.getAllResponseHeaders());
		var hdrs = allocate(intArrayFromString(http.getAllResponseHeaders()),
				'i8', ALLOC_STACK);
		var byteArray = new Uint8Array(http.response);
		var buffer = _malloc(byteArray.length);
		HEAPU8.set(byteArray, buffer);
		if (onload)
			Runtime.dynCall('viiiiii', onload, [ handle, arg, buffer,
					byteArray.length, http.status, hdrs ]);
		if (free)
			_free(buffer);
		/*
		 * } else { console.log(url+" ERROR"); if (onerror)
		 * Runtime.dynCall('viiii', onerror, [handle, arg, http.status,
		 * http.statusText]); }
		 */
		delete Browser.wgetRequests[handle];
	};

	// ERROR
	http.onerror = function http_onerror(e) {
		if (onerror) {
			Runtime.dynCall('viiii', onerror, [ handle, arg, http.status,
					http.statusText ]);
		}
		delete Browser.wgetRequests[handle];
	};

	// PROGRESS
	http.onprogress = function http_onprogress(e) {
		if (onprogress)
			Runtime
					.dynCall(
							'viiii',
							onprogress,
							[
									handle,
									arg,
									e.loaded,
									e.lengthComputable
											|| e.lengthComputable === undefined ? e.total
											: 0 ]);
	};

	// ABORT
	http.onabort = function http_onabort(e) {
		delete Browser.wgetRequests[handle];
	};

	// Useful because the browser can limit the number of redirection
	try {
		if (http.channel instanceof Ci.nsIHttpChannel)
			http.channel.redirectionLimit = 0;
	} catch (ex) { /* whatever */
	}

	if (_request == "POST") {
		http.send(_param);
	} else {
		http.send(null);
	}

	Browser.wgetRequests[handle] = http;

	return handle;
}

Module.checkALMuted = function() {
	if (window.AL && window.AL.currentCtx && (!Module.GidAudioUnlocked)) {
		window.AL.currentCtx.audioCtx.resume();
		Module.GidAudioUnlocked = true;
	}
}

Module.GiderosJSEvent = function(type, context, value, data) {
	var etype = 'number';
	var len = data.length;
	if (typeof data == 'string') {
		etype = 'string';
		len = -1;
	} else {
		var dataPtr = Module._malloc(len);
		var dataHeap = new Uint8Array(Module.HEAPU8.buffer, dataPtr, len);
		dataHeap.set(data);
		data = dataPtr;
	}
	Module.ccall('JSNative_enqueueEvent', 'number', [ 'string', 'number',
			'number', etype, 'number' ], [ type, context, value, data, len ]);
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
