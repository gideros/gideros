GidController_getGamepads=function () {
	var gp=[];
	navigator.getGamepads().forEach(function (val){
		var b=[];
		val.buttons.forEach(function (bt) {
			b.push({ pressed: bt.pressed, touched: bt.touched, value: bt.value});
		});
		var g={ axes: val.axes, id: val.id, index: val.index, mapping: val.mapping, timestamp: val.timestamp, buttons: b};
		gp.push(g);
	});
	return gp;
}
