UIDesc={}

local function StringEditorFor(c,g,s)
	return function (d)
		local te=UI.TextField.new(g(c),nil)
		te.onTextChange=function (w,l,txt)
			s(c,txt)
		end
		return te
	end
end


UIDesc.Editor=function (c,p)
	if p.type=="string" then
		return StringEditorFor(c,p.getter,p.setter)
	elseif p.getter then
		return p.getter(c)
	end
end