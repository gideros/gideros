// Create a 2D canvas
JPZLoad=function (fl,ev)
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
    
    w = ctx.getImageData(0, 0, canv.width, canv.height).data;
    canv= null;
    u=new Uint8Array(3*(w.length/4));
    for (k = 0, ki = 0; k < w.length; k ++)
    {
    	u[ki++]=w[k++];
    	u[ki++]=w[k++];
    	u[ki++]=w[k++];
    }
    
    var bb = new Blob([u]);
    var f = new FileReader();
    f.onload = function(e) {
    	ev(e.target.result);
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
img.src = fl;
}
