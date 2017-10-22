#include "fontbase.h"
#include <math.h>
#include <algorithm>

static void layoutHorizontal(FontBase::TextLayout *tl,int start, float w, float cw, float sw, float tabSpace, int flags, bool wrapped=false)
{
	size_t cur=tl->parts.size();
	size_t cnt=cur-start;
	float ox=0;
	if ((flags&FontBase::TLF_JUSTIFIED)&&wrapped)
        sw+=(cnt>1)?((w-cw)/(cnt-1)):0;
	else if (flags&FontBase::TLF_RIGHT)
		ox=w-cw;
	else if (flags&FontBase::TLF_CENTER)
		ox=(w-cw)/2;
	for (size_t i=start;i<cur;i++)
	{
		tl->parts[i].x+=ox;
		tl->parts[i].dx=ox;
		float ns=(tl->parts[i].sep=='\t')?(tabSpace*(1+floor(cw/tabSpace))-cw):sw;
		ox+=tl->parts[i].w+ns;
	}
}

FontBase::TextLayout FontBase::layoutText(const char *text, FontBase::TextLayoutParameters *params)
{
	TextLayout tl;
	float lh=getLineHeight()+params->lineSpacing;
	float sw=getAdvanceX(" ",params->letterSpacing,-1);
    float as=getAscender();
    float ds=getLineHeight()-as;
	bool wrap=!(params->flags&TLF_NOWRAP);
	const char *bt=text;
	const char *rt=bt;
	//Cut text around spaces and control chars (ascii<' ')
	float y=0;
	float cw=0;
	int st=0;
	int lines=0;
	float tabSpace;
	if (params->tabSpace<0)
		tabSpace=-params->tabSpace;
	else
		tabSpace=params->tabSpace*sw;
	while (*rt)
	{
        while (((*rt)&0xFF)>' ') rt++;
		ChunkLayout cl;
		cl.text=std::string(bt,rt-bt);
        cl.x=cl.y=cl.w=cl.h=0;
        if (cl.text.size())
        {
            getBounds(cl.text.c_str(),params->letterSpacing,&cl.x,&cl.y,&cl.w,&cl.h);
            cl.w=cl.w-cl.x+1;
            cl.h=cl.h-cl.y+1;
        }
		cl.y+=y;
		cl.dy=y;
		cl.sep=*rt;
		cl.line=lines+1;
		float ns=(cl.sep=='\t')?(tabSpace*(1+floor(cw/tabSpace))-cw):sw;
		if (wrap&&cw&&((cw+cl.w+ns)>params->w))
		{
			//The current line will exceed max width (and is not empty): wrap
			layoutHorizontal(&tl,st, params->w, cw, sw, tabSpace, params->flags,true);
			st=tl.parts.size();
			y+=lh;
			cl.y+=lh;
			cl.dy=y;
			cw=0;
			lines++;
			cl.line=lines+1;
		}
		tl.parts.push_back(cl);
		if (cw) cw+=sw;
		cw+=cl.w;
		if ((*rt)=='\n')
		{
			//Line break
			layoutHorizontal(&tl,st, params->w, cw, sw, tabSpace, params->flags);
			st=tl.parts.size();
			y+=lh;
			cw=0;
			lines++;
		}
		if (*rt) rt++;
		bt=rt;
	}
	//Layout final line
	if (cw)
	{
		layoutHorizontal(&tl,st, params->w, cw, sw, tabSpace, params->flags);
		st=tl.parts.size();
		y+=lh;
		cw=0;
		lines++;
	}

	//Layout lines of text
	float yo=0;
	if (params->flags&TLF_BOTTOM)
		yo=params->h-y;
	else if (params->flags&TLF_VCENTER)
		yo=(params->h-y)/2;
    if (params->flags&TLF_REF_TOP)
        yo+=as;
    else if (params->flags&TLF_REF_BOTTOM)
        yo-=ds;
    else if (params->flags&TLF_REF_MIDDLE)
        yo+=(as-ds)/2;

	tl.x = 1e30;
	tl.y = 1e30;
	float mx=-1e30,my=-1e30;
	for (size_t k=0;k<tl.parts.size();k++)
	{
		tl.parts[k].y+=yo;
		tl.parts[k].dy+=yo;
		tl.x=std::min(tl.x,tl.parts[k].x);
		tl.y=std::min(tl.y,tl.parts[k].y);
		mx=std::max(mx,tl.parts[k].x+tl.parts[k].w-1);
		my=std::max(my,tl.parts[k].y+tl.parts[k].h-1);
	}
	tl.w=mx-tl.x+1;
	tl.h=my-tl.y+1;
    if (tl.parts.size()==0)
    {
        tl.x=tl.y=0;
        tl.w=tl.h=0;
    }
	tl.lines=lines;

	return tl;
}
