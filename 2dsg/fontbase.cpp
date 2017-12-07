#include "fontbase.h"
#include <math.h>
#include <algorithm>
#include <stdlib.h>

void FontBase::layoutHorizontal(FontBase::TextLayout *tl,int start, float w, float cw, float sw, float tabSpace, int flags,float letterSpacing, bool wrapped)
{
	size_t cur=tl->parts.size();
	size_t cnt=cur-start;
	float ox=0;
	bool justified=false;
	if ((flags&FontBase::TLF_JUSTIFIED)&&wrapped)
	{
        sw+=(cnt>1)?((w-cw)/(cnt-1)):0;
        justified=true;
	}
	else if (flags&FontBase::TLF_RIGHT)
		ox=w-cw;
	else if (flags&FontBase::TLF_CENTER)
		ox=(w-cw)/2;
	if (!justified) //Not justified, try to merge space separated chunks together
	{
		bool merged=false;
		for (size_t i=start;i<(cur-1);i++)
		{
			if (tl->parts[i].sep==' ')
			{
				tl->parts[i].text=tl->parts[i].text+" "+tl->parts[i+1].text;
				tl->parts[i].sep=tl->parts[i+1].sep;
				tl->parts.erase(tl->parts.begin()+i+1);
				cur--;
				merged=true;
                i--;
                continue;
			}
            if (merged)
			{
	            getBounds(tl->parts[i].text.c_str(),letterSpacing,&tl->parts[i].x,&tl->parts[i].y,&tl->parts[i].w,&tl->parts[i].h);
	            tl->parts[i].w=tl->parts[i].w-tl->parts[i].x+1;
	            tl->parts[i].h=tl->parts[i].h-tl->parts[i].y+1;
				tl->parts[i].y+=tl->parts[i].dy;
				merged=false;
			}
		}
		if (merged)
		{
			size_t i=cur-1;
            getBounds(tl->parts[i].text.c_str(),letterSpacing,&tl->parts[i].x,&tl->parts[i].y,&tl->parts[i].w,&tl->parts[i].h);
            tl->parts[i].w=tl->parts[i].w-tl->parts[i].x+1;
            tl->parts[i].h=tl->parts[i].h-tl->parts[i].y+1;
			tl->parts[i].y+=tl->parts[i].dy;
		}
	}
	for (size_t i=start;i<cur;i++)
	{
		tl->parts[i].x+=ox;
		tl->parts[i].dx=ox;
        char sep=tl->parts[i].sep;
        float ns=(sep=='\t')?(tabSpace*(1+floor(cw/tabSpace))-cw):sw;
        if (sep=='\e') ns=0;
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
	ChunkLayout styles; //To hold styling info
	styles.styleFlags=0;
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
		cl.styleFlags=styles.styleFlags;
		cl.color=styles.color;
		float ns=(cl.sep=='\t')?(tabSpace*(1+floor(cw/tabSpace))-cw):sw;
		if (wrap&&cw&&((*rt)!='\e')&&((cw+cl.w+ns)>params->w))
		{
			//The current line will exceed max width (and is not empty): wrap
			layoutHorizontal(&tl,st, params->w, cw, sw, tabSpace, params->flags,params->letterSpacing,true);
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
			layoutHorizontal(&tl,st, params->w, cw, sw, tabSpace, params->flags,params->letterSpacing);
			st=tl.parts.size();
			y+=lh;
			cw=0;
			lines++;
		}
		if ((*rt)=='\e') {
			//Parse styles
			if ((*(++rt))=='[')
			{
				const char *ss=rt+1;
				const char *se=ss;
				while ((*se)&&((*se)!=']')) se++;
				if (*se==']')
				{
					rt=se+1;
					while (true) {
						const char *sp=ss;
						const char *sa=NULL;
						while ((sp<se)&&((*sp)!=','))
						{
							if ((*sp)=='=') sa=sp;
							sp++;
						}
						std::string key,val;
						if (sa)
						{
							key=std::string(ss,sa-ss);
							val=std::string(sa+1,sp-sa-1);
						}
						else
							key=std::string(ss,sp-ss);
						if (!key.compare("color"))
						{
							if ((val.size()==0)||(val.at(0)!='#'))
								styles.styleFlags&=~TEXTSTYLEFLAG_COLOR;
							else
							{
								styles.styleFlags|=TEXTSTYLEFLAG_COLOR;
								int param=strtol(val.c_str()+1,NULL,16);
								switch (val.size())
								{
									case 4:
										styles.color=((param&0x0F)<<4)|((param&0x0F)<<0)|
													 ((param&0xF0)<<8)|((param&0xF0)<<4)|
													 ((param&0xF00)<<12)|((param&0xF00)<<8)|
													 0xFF000000;
										break;
									case 5:
										styles.color=((param&0x0F)<<28)|((param&0x0F)<<24)|
													 ((param&0xF0)<<0)|((param&0xF0)>>4)|
													 ((param&0xF00)<<4)|((param&0xF00)>>0)|
													 ((param&0xF000)<<8)|((param&0xF000)<<4);
										break;
									case 7:
										styles.color=param|0xFF000000;
										break;
									case 9:
										styles.color=(param>>8)|(param&0xFF)<<24;
										break;
									default:
										styles.styleFlags&=~TEXTSTYLEFLAG_COLOR;
								}
							}
						}
						if (sp==se) break;
						ss=sp+1;
					}
				}
			}
		}
		else if (*rt) rt++;
		bt=rt;
	}
	//Layout final line
	if (cw)
	{
		layoutHorizontal(&tl,st, params->w, cw, sw, tabSpace, params->flags,params->letterSpacing);
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
	tl.styleFlags=0;
	for (size_t k=0;k<tl.parts.size();k++)
	{
		tl.parts[k].y+=yo;
		tl.parts[k].dy+=yo;
		tl.x=std::min(tl.x,tl.parts[k].x);
		tl.y=std::min(tl.y,tl.parts[k].y);
		mx=std::max(mx,tl.parts[k].x+tl.parts[k].w-1);
		my=std::max(my,tl.parts[k].y+tl.parts[k].h-1);
		tl.styleFlags|=tl.parts[k].styleFlags;
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
