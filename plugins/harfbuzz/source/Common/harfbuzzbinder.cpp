#include <gideros.h>
#include "glog.h"
#include "gtexture.h"
#include <fontbase.h>
#include "hb.h"
#include "ucdn.h"
#include "lua.h"
#include "luautil.h"
#include "lauxlib.h"

#define ESC	27

static lua_State *L = NULL;

class HarfBuzzFontShaper : public FontShaper {
    hb_font_t *font;
public:
    HarfBuzzFontShaper(void *fontData,size_t fontDataSize,int size,int xres,int yres);
    virtual ~HarfBuzzFontShaper();
    virtual bool shape(struct FontBase::ChunkLayout &part,std::vector<wchar32_t> &wtext);
};

HarfBuzzFontShaper::HarfBuzzFontShaper(void *fontData,size_t fontDataSize,int size,int xres,int yres)
{
    hb_blob_t *blob;
    hb_face_t *face;

    blob = hb_blob_create ((const char  *)fontData, fontDataSize, HB_MEMORY_MODE_DUPLICATE, NULL, NULL);
    face = hb_face_create (blob, 0);
    hb_blob_destroy (blob);
    hb_face_set_upem (face, size);
    font = hb_font_create (face);
    hb_face_destroy (face);
    hb_font_set_scale (font, xres, yres);
}

HarfBuzzFontShaper::~HarfBuzzFontShaper()
{
    hb_font_destroy(font);
}

bool HarfBuzzFontShaper::shape(struct FontBase::ChunkLayout &part,std::vector<wchar32_t> &wtext)
{
    hb_buffer_t *buffer;
    unsigned int len;
    hb_glyph_info_t *glyphs;
    hb_glyph_position_t *positions;

    buffer =  hb_buffer_create ();
    hb_buffer_add_utf32(buffer,(const uint32_t *) &wtext[0],wtext.size(),0,wtext.size());
    hb_buffer_guess_segment_properties(buffer);
    //hb_buffer_set_language(buffer, hb_language_from_string("ar",2));
    //hb_buffer_set_script(buffer,HB_SCRIPT_ARABIC);
    if (part.styleFlags&(TEXTSTYLEFLAG_RTL|TEXTSTYLEFLAG_LTR))
    	hb_buffer_set_direction (buffer, (part.styleFlags&TEXTSTYLEFLAG_RTL)?HB_DIRECTION_RTL:HB_DIRECTION_LTR);

    hb_shape (font, buffer, NULL, 0);

    len = hb_buffer_get_length (buffer);
    glyphs = hb_buffer_get_glyph_infos (buffer, NULL);
    positions = hb_buffer_get_glyph_positions (buffer, NULL);

    struct FontBase::GlyphLayout gl;
    for (size_t k=0;k<len;k++)
    {
        gl.advX=positions[k].x_advance;
        gl.advY=positions[k].y_advance;
        gl.offX=positions[k].x_offset;
        gl.offY=-positions[k].y_offset;
        gl.glyph=glyphs[k].codepoint;
        gl.srcIndex=glyphs[k].cluster;
        if (gl.glyph)
            part.shaped.push_back(gl);
    }

    hb_buffer_destroy(buffer);

    return true;
}


static FontShaper *shaper_builder(void *fontData,size_t fontDataSize,int size,int xres,int yres)
{
    return new HarfBuzzFontShaper(fontData,fontDataSize,size,xres,yres);
}

wchar32_t strgetchar(const char **str)
{
 int unicode;
 if (!str) return 0;
 const unsigned char *s=(const unsigned char *)*str;
 if (!s) return 0;
 while (((*s)&0xC0)==0x80) s++;
 *str=(const char *) s;
 if (!*s) return 0;

 if ((*s)<0x80)
 {
  unicode=*s++;
  goto end;
 }
 if (((*s)&0xE0)==0xC0)
 {
  unicode=((*(s++))&0x1F)<<6;
  if (((*s)&0xC0)!=0x80) goto bad;
  unicode|=((*s)&0x3F);
  if (unicode&&((unicode)<0x80)) goto bad; //Zero may be encoded like this
  s++;
  goto end;
 }
 if (((*s)&0xF0)==0xE0)
 {
  unicode=((*(s++))&0x0F)<<12;
  if (((*s)&0xC0)!=0x80) goto bad;
  unicode|=((*s)&0x3F)<<6;
  s++;
  if (((*s)&0xC0)!=0x80) goto bad;
  unicode|=((*s)&0x3F);
  if ((unicode)<0x800) goto bad;
  s++;
  goto end;
 }
 if (((*s)&0xF8)==0xF0)
 {
  unicode=((*(s++))&0x07)<<18;
  if (((*s)&0xC0)!=0x80) goto bad;
  unicode|=((*s)&0x3F)<<6;
  s++;
  if (((*s)&0xC0)!=0x80) goto bad;
  unicode|=((*s)&0x3F)<<6;
  s++;
  if (((*s)&0xC0)!=0x80) goto bad;
  unicode|=((*s)&0x3F);
  if ((unicode)<0x10000) goto bad;
  s++;
  goto end;
 }
 s++;
bad:
 unicode=-1;
end:
 *str=(const char *) s;
 return unicode;
}

static bool classifier(std::vector<FontBase::ChunkClass> &chunks,std::string text)
{
	FontBase::ChunkClass cc;
    cc.script=0;
	const char *bt=text.c_str();
	const char *rt=bt;
	bool rtl=false;
	while (*rt)
	{
		const char *pt=rt;
        wchar32_t c=strgetchar(&rt);
        int uc=ucdn_get_bidi_class((uint32_t)c);
        bool brk=true;
        if (uc==UCDN_BIDI_CLASS_CS) {
        	//Look ahead for next directional char
        	const char *at=rt;
        	while (*at) {
                wchar32_t c=strgetchar(&at);
                int uca=ucdn_get_bidi_class((uint32_t)c);
                if ((uca==UCDN_BIDI_CLASS_L)||
                		(uca==UCDN_BIDI_CLASS_R)||
						(uca==UCDN_BIDI_CLASS_AL)) {
                	bool nrtl=(uca!=UCDN_BIDI_CLASS_L);
                	if (nrtl!=rtl) {
                		uc=UCDN_BIDI_CLASS_L;
                		brk=false;
                	}
                	break;
                }
        	}
        }
        if ((rtl&&(uc==UCDN_BIDI_CLASS_L))||
        		((!rtl)&&((uc==UCDN_BIDI_CLASS_R)||(uc==UCDN_BIDI_CLASS_AL)))||
				(uc==UCDN_BIDI_CLASS_B)||
				(uc==UCDN_BIDI_CLASS_S)||
				(uc==UCDN_BIDI_CLASS_WS)||
				(c==ESC))
        {
        	cc.sep=c;
        	bool nrtl=rtl;
        	switch (uc) {
        	case UCDN_BIDI_CLASS_B:
        		cc.sepFlags=CHUNKCLASS_FLAG_BREAK;
        		break;
        	case UCDN_BIDI_CLASS_S:
        	case UCDN_BIDI_CLASS_WS:
        		cc.sepFlags=CHUNKCLASS_FLAG_BREAKABLE;
        		break;
        	case UCDN_BIDI_CLASS_BN: //Catch ESC
        		cc.sepFlags=0;
        		break;
        	default: //Letter
        		cc.sep=0;
        		cc.sepFlags=brk?CHUNKCLASS_FLAG_BREAKABLE:0;
        		nrtl=!rtl;
        	}
        	if ((pt!=bt)||(cc.sep))
        	{
                cc.text=std::string(bt,pt-bt);
                cc.textFlags=CHUNKCLASS_FLAG_BREAKABLE|(rtl?CHUNKCLASS_FLAG_RTL:CHUNKCLASS_FLAG_LTR);
                if (cc.sep==ESC)
                {
                	//Extract escape sequence
                	if (*rt=='[') {
        				const char *ss=rt+1;
        				const char *se=ss;
        				while ((*se)&&((*se)!=']')) se++;
        				if (*se==']') {
        		            chunks.push_back(cc);
        		            rt=se+1;
        		            bt=rt;
        		            cc.text=std::string(ss,se-ss);
        		            cc.textFlags=CHUNKCLASS_FLAG_STYLE;
        		            cc.sep=0;
        		            cc.sepFlags=0;
        				}
                	}
                }
                else if (cc.sep)
                    bt=rt;
                else
                	bt=pt;
                chunks.push_back(cc);
        	}
        	rtl=nrtl;
        }
	}
	if (rt!=bt)
	{
        cc.text=std::string(bt,rt-bt);
        cc.textFlags=CHUNKCLASS_FLAG_BREAKABLE|(rtl?CHUNKCLASS_FLAG_RTL:CHUNKCLASS_FLAG_LTR);
        cc.sep=0;
        cc.sepFlags=0;
        chunks.push_back(cc);
	}
	return true;
}

static void g_initializePlugin(lua_State *L) {
	::L = L;

    g_setGlobalHook(GID_GLOBALHOOK_FONTSHAPER,(void *)shaper_builder);
    g_setGlobalHook(GID_GLOBALHOOK_TEXTCLASSIFIER,(void *)classifier);
}

static void g_deinitializePlugin(lua_State *) {
    g_setGlobalHook(GID_GLOBALHOOK_FONTSHAPER,NULL);
    g_setGlobalHook(GID_GLOBALHOOK_TEXTCLASSIFIER,NULL);
}

#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR || defined(_MSC_VER) || (TARGET_OS_OSX && !defined(QT_CORE_LIB))
REGISTER_PLUGIN_STATICNAMED_CPP("Harfbuzz", "1.0",harfbuzz)
#else
REGISTER_PLUGIN_NAMED("Harfbuzz", "1.0", harfbuzz)
#endif
