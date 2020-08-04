#include "textfieldbase.h"
#include "application.h"

bool TextFieldBase::scaleChanged() {
	float scalex = application_->getLogicalScaleX();
	float scaley = application_->getLogicalScaleY();
	FontBase *font=getFont();
	int fontver = (font==NULL)?-1:font->getCacheVersion();
	bool changed=(scalex!=lscalex_)||(scaley!=lscaley_)||(fontver!=lfontCacheVersion_);
	lscalex_=scalex;
	lscaley_=scaley;
	lfontCacheVersion_=fontver;
	return changed;
}

bool TextFieldBase::setDimensions(float w,float h,bool forLayout)
{
    bool changed=Sprite::setDimensions(w,h);
    if (changed) {
		layout_.w=w;
		layout_.h=h;
		setLayout(&layout_);
    }
    return changed;
}

void TextFieldBase::getDimensions(float &w,float &h)
{
    w=textlayout_.w;
    h=textlayout_.bh;
}

void TextFieldBase::getMinimumSize(float &w,float &h,bool preferred)
{
	w=preferred?textlayout_.w:textlayout_.mw;
	h=textlayout_.bh;
}

static size_t utf8_offset(const char *text,int cp) {
	size_t o=0;
	while ((*text)&&cp) {
		o++;
		text++;
		while (((*text)&0xC0)==0x80) { text++; o++; }
		cp--;
	}
	return o;
}

void TextFieldBase::getPointFromTextPos(size_t ri,float &cx,float &cy)
{
	if (ri>text_.size()) ri=text_.size();
	size_t lc;
	size_t parts=textlayout_.parts.size();
	for (size_t i=0;i<parts;i++) {
		FontBase::ChunkLayout &c=textlayout_.parts[i];
		size_t cl=c.text.size();
		if (ri>cl) {
			ri-=cl;
			ri--;
			if (c.sep>=0x80) ri--;
			if (c.sep>=0x800) ri--;
			if (c.sep>=0x10000) ri--;
			lc=i;
		}
		else {
			float advX=0;
			size_t gln=c.shaped.size();
			const char *t=c.text.c_str();
            bool rtl=c.styleFlags&TEXTSTYLEFLAG_RTL;
			for (size_t g=0;g<gln;g++) {
                FontBase::GlyphLayout &v=c.shaped[g];
                size_t ui=utf8_offset(t,v.srcIndex);
                if (((!rtl)&&(ri>ui))||(rtl&&(ui>ri)))
					advX+=v.advX;
			}
			advX*=c.shapeScaleX;
			/* gln==0:
				local advT=c.text:sub(1,ri)
				advX=self.font:getAdvanceX(advT,lp.letterSpacing)
			 */
			cx=advX+c.dx;
			cy=c.dy;
			return;
		}
	}
	if (parts) {
		FontBase::ChunkLayout &c=textlayout_.parts[lc];
		cx=c.dx+c.advX;
		cy=c.dy;
		return;
	}
	cx=0;
	cy=getFont()->getAscender();
}

size_t TextFieldBase::getTextPosFromPoint(float &cx,float &cy)
{
	size_t ti=0;
	size_t parts=textlayout_.parts.size();
	float rcx=0,rcy=0;
	for (size_t i=0;i<parts;i++) {
		FontBase::ChunkLayout &c=textlayout_.parts[i];
		if (c.y>cy) { rcx=c.dx; rcy=c.dy; break; }
		if ((c.y+c.h)>cy) {
			if (c.dx>cx) { rcx=c.dx; rcy=c.dy; break; }
			if ((c.dx+c.advX)>cx) {
				size_t n=0;
				size_t gln=c.shaped.size();
				float xbase=c.dx;
				for (size_t g=0;g<gln;g++) {
                    FontBase::GlyphLayout &v=c.shaped[g];
					float ax=v.advX*c.shapeScaleX;
					if (cx<(xbase+ax))
					{
						n=v.srcIndex;
						rcx=xbase;
						rcy=c.dy;
						break;
					}
					else
						xbase+=ax;
				}
				    /* gln==0:
					while n<=#c.text do
						local advX=self.font:getAdvanceX(c.text,lp.letterSpacing,n)
						if (c.x+advX)>cx then break	end
						n+=1
					end
					*/
				ti+=utf8_offset(c.text.c_str(),n);
				break;
			}
			else {
				ti+=c.text.size();
				if (c.sep>=0x80) ti++;
				if (c.sep>=0x800) ti++;
				if (c.sep>=0x10000) ti++;
				rcx=c.dx+c.advX;
				rcy=c.dy;
			}
		} else {
			ti+=c.text.size();
			if (c.sep>=0x80) ti++;
			if (c.sep>=0x800) ti++;
			if (c.sep>=0x10000) ti++;
			rcx=c.dx+c.advX;
			rcy=c.dy;
		}
	}
	cx=rcx;
	cy=rcy;
	return ti;
}
