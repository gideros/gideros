JZPLoaded={}

function JZPLoadAsync(imageUrl, onprogress) {
	  return new Promise((resolve, reject) => {
	    var xhr = new Module.XMLHttpRequest();
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
