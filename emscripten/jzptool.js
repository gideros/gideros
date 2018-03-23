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

	        if (m && m[1]) {
	          options.type = m[1];
	        }

	        var blob = new Blob([this.response], options);

	        resolve(window.URL.createObjectURL(blob));
	      }
	    }

	    xhr.send();
	  });
	}


JPZConvert=function (fl,cb)
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
        u=new Uint8Array(3*(w.length/4));
        for (k = 0, ki = 0; k < w.length; k ++)
        {
        	u[ki++]=w[k++];
        	u[ki++]=w[k++];
        	u[ki++]=w[k++];
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
	.then(imgSrc => {
	  // Loading successfuly complete; set the image and probably do other stuff.
		img.src = imgSrc;
	}, xhr => {
	  // An error occured. We have the XHR object to see what happened.
		console.error("Failed to load:"+fl)
	});
}

JPZLoad=function (fl,ev)
{
	JPZConvert(fl,function (code)
	{
		 setTimeout(function() { 
			 ev(code)
		},1)
	})
}