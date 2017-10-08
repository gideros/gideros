#include "fontbase.h"
#include <math.h>

static void layoutHorizontal(FontBase::TextLayout *tl,int start, float w, float cw, float sw, float tabSpace, int flags)
{
	size_t cur=tl->parts.size();
	size_t cnt=cur-start;
	float ox=0;
	if (flags&FontBase::TLF_JUSTIFIED)
		sw=(w-cw)/(cnt-1);
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

FontBase::TextLayout FontBase::layoutText(const char *text, float w, float h,int flags,float letterSpacing,float lineSpacing, float tabSpace)
{
	TextLayout tl;
	float lh=getLineHeight()+lineSpacing;
	float sw=getAdvanceX(" ",letterSpacing,-1);
	bool wrap=!(flags&TLF_NOWRAP);
	const char *bt=text;
	const char *rt=bt;
	//Cut text around spaces and control chars (ascii<' ')
	float y=0;
	float cw=0;
	int st=0;
	int lines=0;
	if (tabSpace<0)
		tabSpace=-tabSpace;
	else
		tabSpace*=sw;
	while (*rt)
	{
		while (*(rt)>' ') rt++;
		ChunkLayout cl;
		cl.text=std::string(bt,rt-bt);
		getBounds(cl.text.c_str(),letterSpacing,&cl.x,&cl.y,&cl.w,&cl.h);
		cl.w=cl.w-cl.x;
		cl.h=cl.h-cl.y;
		cl.y+=y;
		cl.dy=y;
		cl.sep=*rt;
		cl.line=lines+1;
		float ns=(cl.sep=='\t')?(tabSpace*(1+floor(cw/tabSpace))-cw):sw;
		if (wrap&&cw&&((cw+cl.w+ns)>w))
		{
			//The current line will exceed max width (and is not empty): wrap
			layoutHorizontal(&tl,st, w, cw, sw, tabSpace, flags);
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
			layoutHorizontal(&tl,st, w, cw, sw, tabSpace, flags);
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
		//The current line will exceed max width (and is not empty): wrap
		layoutHorizontal(&tl,st, w, cw, sw, tabSpace, flags);
		st=tl.parts.size();
		y+=lh;
		cw=0;
		lines++;
	}

	//Layout lines of text
	float yo=0;
	if (flags&TLF_BOTTOM)
		yo=h-y;
	else if (flags&TLF_VCENTER)
		yo=(h-y)/2;
	tl.x = 1e30;
	tl.y = 1e30;
	float mx=-1e30,my=-1e30;
	for (size_t k=0;k<tl.parts.size();k++)
	{
		tl.parts[k].y+=yo;
		tl.parts[k].dy+=yo;
		tl.x=std::min(tl.x,tl.parts[k].x);
		tl.y=std::min(tl.y,tl.parts[k].y);
		mx=std::max(mx,tl.parts[k].x+tl.parts[k].w);
		my=std::max(my,tl.parts[k].y+tl.parts[k].h);
	}
	tl.w=mx-tl.x;
	tl.h=my-tl.y;
	tl.lines=lines;

	return tl;
}
