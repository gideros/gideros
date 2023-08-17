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
UI.Style.aui_aslib={
	icBDown=Texture.new("ui/icons/rdown.png",true,{ mipmap=true }),
	icBRight=Texture.new("ui/icons/rnext.png",true,{ mipmap=true }),
	icTrash=Texture.new("aui/icons/delete.png",true,{ mipmap=true }),
	styLabel={
		colWidgetBack=UI.Colors.white,
		brdWidget=UI.Border.NinePatch.new({
			texture=Texture.new("ui/icons/bdr-multi.png",true,{ mipmap=true }),
			corners={"textfield.szMargin","textfield.szMargin","textfield.szMargin","textfield.szMargin",63,63,63,63,},
		}),
		shader={ 
			class="UI.Shader.MultiLayer", 
			params={ colLayer1="colDisabled", colLayer2="colHeader", colLayer3=colNone, colLayer4=colNone } 
		},		
	},
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
	icPlus=Texture.new("ui/icons/plus.png",true,{ mipmap=true }),
	icArchive=Texture.new("aui/icons/file-cabinet.png",true,{ mipmap=true }),
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


