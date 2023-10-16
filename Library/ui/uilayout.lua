--!NEEDS:uiinit.lua

UI.Layout={}
function UI.Layout.Vertical(p,ps)
	local n=0
	ps.columnWeights={1}
	ps.rowWeights={}
	ps.equalizeCells=true
	local function VerticalLayouter(l,li)
		l.gridy=n
		l.fill=li.fill or Sprite.LAYOUT_FILL_BOTH
		n=n+1
		ps.rowWeights[n]=li.weighty or 0
	end
	return VerticalLayouter
end
function UI.Layout.Horizontal(p,ps)
	local n=0
	ps.rowWeights={1}
	ps.columnWeights={}
	ps.equalizeCells=true
	local function HorizontalLayouter(l,li)
		l.gridx=n
		l.fill=li.fill or Sprite.LAYOUT_FILL_BOTH
		n=n+1
		ps.columnWeights[n]=li.weightx or 0
	end
	return HorizontalLayouter
end