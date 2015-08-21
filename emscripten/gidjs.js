Module.TOTAL_MEMORY=256*1024*1024
Module.preRun.push(function() {
	FS.createPreloadedFile("/", "main.gapp", "gideros.GApp", true,false)
})
