GidController_getGamepads=function () {
	var gp=[];
	var gpi;
	var gpl=navigator.getGamepads();
	for (gpi=0;gpi<=gpl.length;gpi++) {
		var val=gpl[gpi];
		if (val) {
			var b=[];
			var bi;
			var bl=val.buttons;
			for (bi=1;bi<=bl.length;bi++) {
				var bt=bl[bi];
				if (bt)
					b.push({ pressed: bt.pressed, touched: bt.touched, value: bt.value});
			}
			var g={ axes: val.axes, id: val.id, index: val.index, mapping: val.mapping, timestamp: val.timestamp, buttons: b};
			gp.push(g);			
		}
	}
	return gp;
}
