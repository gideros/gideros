Label = Core.class(TextField, function(t) return t.font, t.text end)

function Label:init(t)
	local flags = t.flags or FontBase.TLF_REF_TOP|FontBase.TLF_VCENTER|FontBase.TLF_CENTER
	self:setLayout{
		w = t.w,
		h = t.h,
		flags = flags
	}
	self.w = t.w
	self.h = t.h
	--self:addChild(Pixel.new(0xff0000, .3, t.w, t.h))
end

function Label:getWidth() return self.w end
function Label:getHeight() return self.h end