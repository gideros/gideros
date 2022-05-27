#include "textfieldbase.h"
#include "application.h"

void TextFieldBase::cloneFrom(TextFieldBase *s)
{
    Sprite::cloneFrom(s);
    text_=s->text_;
    sample_=s->sample_;
    layout_=s->layout_;
    lscalex_=s->lscalex_;
    lscaley_=s->lscaley_;
    lfontCacheVersion_=s->lfontCacheVersion_;
    textlayout_=s->textlayout_;
    prefWidth_=s->prefWidth_;
    prefHeight_=s->prefHeight_;
}

bool TextFieldBase::scaleChanged() {
	float scalex = application_->getLogicalScaleX();
	float scaley = application_->getLogicalScaleY();
	FontBase *font=getFont();
	int fontver = (font==NULL)?-1:font->getCacheVersion();
	bool changed=(scalex!=lscalex_)||(scaley!=lscaley_)||(fontver!=lfontCacheVersion_);
	lscalex_=scalex;
	lscaley_=scaley;
	lfontCacheVersion_=fontver;
    prefWidth_=prefHeight_=-1;
    textlayout_.letterSpacingCache=-1;
	return changed;
}

bool TextFieldBase::setDimensions(float w,float h,bool forLayout)
{
    G_UNUSED(forLayout);
    bool changed=Sprite::setDimensions(w,h);
    if (changed) {
		layout_.w=w;
		layout_.h=h;
        float pwidth=prefWidth_;
        float pheight=prefHeight_;
        setLayout(&layout_);
        if (forLayout) {
            prefWidth_=pwidth;
            prefHeight_=pheight;
        }
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
    /* For wrappable texts we are in trouble here, since height will depend on width
     * Minimum case is easy: we give th minimum on both axis
     * But what should be an ideal/preferred size ?
     * Use aspect ratio hint and try to reach it */

    float tgtar=layout_.aspect;
    float ar=(((prefWidth_+0.001)/tgtar)/(prefHeight_+0.001));
    if (ar<1) ar=1/ar;
    float ar2=(((textlayout_.w+0.001)/tgtar)/(textlayout_.bh+0.001));
    if (ar2<1) ar2=1/ar2;
    if ((prefHeight_==-1)||(ar2<ar)) {
        prefHeight_=textlayout_.bh;
        prefWidth_=textlayout_.w;
    }
    w=(preferred&&(prefHeight_>0))?prefWidth_:textlayout_.mw;
    h=(preferred&&(prefHeight_>0))?prefHeight_:textlayout_.bh;
}

bool TextFieldBase::optimizeSize(float &w,float &h)
{
	if ((w!=layout_.w)||(h!=layout_.h)) {
		layout_.w=w;
		layout_.h=h;
		setLayout(&layout_);
		if (textlayout_.w<w)
			w=textlayout_.w;
		if (textlayout_.h<h)
			h=textlayout_.h;
		return true;
	}
	return false;
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

#define ESC	27
void TextFieldBase::getPointFromTextPos(size_t ri,float &cx,float &cy)
{
	if (ri>text_.size()) ri=text_.size();
	size_t lc;
	size_t parts=textlayout_.parts.size();
	for (size_t i=0;i<parts;i++) {
		FontBase::ChunkLayout &c=textlayout_.parts[i];
        size_t cl=c.text.size()-c.extrasize;
		if (ri>cl) {
			ri-=cl;
            if (c.sep) ri--;
			if (c.sep>=0x80) ri--;
			if (c.sep>=0x800) ri--;
			if (c.sep>=0x10000) ri--;
			lc=i;
		}
		else {
			float advX=0;
			size_t gln=c.shaped.size();
			const char *t=c.text.c_str();
            bool rtl=c.style.styleFlags&TEXTSTYLEFLAG_RTL;
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
    size_t rti=0;
	size_t parts=textlayout_.parts.size();
    float rcx=0,rcy=0;
    if (parts>0) {
        rcx=textlayout_.parts[0].dx;
        rcy=textlayout_.parts[0].dy;
    }
    else
        rcy=getFont()->getAscender();
    for (size_t i=0;i<parts;i++) {
		FontBase::ChunkLayout &c=textlayout_.parts[i];
        if (c.y>cy) break;
		if ((c.y+c.h)>cy) {
            rti=ti;
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
                size_t toff=utf8_offset(c.text.c_str(),n);
                size_t tsize=c.text.size()-((c.extrasize>0)?c.extrasize:0);
                if (toff>tsize) toff=tsize;
                ti+=toff;
                rti=ti;
				break;
			}
			else {
                ti+=c.text.size()-((c.extrasize>0)?c.extrasize:0);
                rti=ti;
                if (c.sep) ti++;
                if (c.sep>=0x80) ti++;
				if (c.sep>=0x800) ti++;
				if (c.sep>=0x10000) ti++;
                if (c.extrasize<0) ti-=c.extrasize;
				rcx=c.dx+c.advX;
				rcy=c.dy;
			}
		} else {
            ti+=c.text.size()-c.extrasize;
            if (c.sep) ti++;
			if (c.sep>=0x80) ti++;
			if (c.sep>=0x800) ti++;
			if (c.sep>=0x10000) ti++;
			rcx=c.dx+c.advX;
			rcy=c.dy;
            rti=ti;
        }
	}
	cx=rcx;
	cy=rcy;
    return rti;
}
