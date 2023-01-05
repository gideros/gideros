--!NEEDS:(GiderosUI)/uistyle.lua
UI.Style.aui_asshelf={
	szSpacing=".1s",
	szColumn="4s",
	szAll="21s", --5*szCol+4*szSpacing+scrollbar
}
UI.Style.aui_asitem={
	szSpacing=".1s",
	font="font.small",
}
UI.Style.aui_proplist={
	styCategory={
		colWidgetBack="colDisabled",
	}
}
UI.Style.aui_tree={
	stySelected={
		colWidgetBack="colSelect",
	}
}
UI.Style.aui_main={
	szSpacing=".1s",
	styLabel={
		colWidgetBack=UI.Colors.white,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/bdr-multi.png",true,{ mipmap=true }),
			corners={"textfield.szMargin","textfield.szMargin","textfield.szMargin","textfield.szMargin",63,63,63,63,},
		}),
		shader={ 
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="textfield.colBackground", colLayer2="textfield.colBorder", colLayer3="textfield.colBorderWide", colLayer4=colNone } 
		}		
	}
}


