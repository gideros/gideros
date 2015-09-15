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
