Module.TOTAL_MEMORY=256*1024*1024
Module.preRun.push(function() {
	Module.setStatus("Loading application...");
	FS.createPreloadedFile("/", "main.gapp", GAPP_URL, true,false);
	  // Initial syncfs to get existing saved files.
	Module['addRunDependency']('syncfs');
	FS.mkdir('/documents');
	FS.mount(IDBFS, {}, '/documents');
	FS.syncfs(true, function(err) {
	    if (err) {
	        throw err;
	    }
	    Module['removeRunDependency']('syncfs');
	});
})
Module.registerPlugins=function()
{
	Module.dynamicLibraries.forEach(function (p) {
	    var pname=p.split(".")[0];
	    var pentry="g_pluginMain_"+pname;
//	    var pp=getCFunc(pentry);
	    Module.ccall('main_registerPlugin','number', ['string'], [pentry]);
	    //g_registerPlugin(g_pluginMain_##symbol);
	    console.log(pname);
	});
}
