#include "ttfont.h"
#include <gfile.h>
#include <gstdio.h>
#include "giderosexception.h"
#include "gstatus.h"
#include "application.h"
#include "ftlibrarysingleton.h"
#include "gtexture.h"

#include <ft2build.h>
#include FT_OUTLINE_H
#include FT_STROKER_H

#include <utf8.h>
#include <algorithm>
#include "path.h"
#include <glog.h>

static unsigned long read(FT_Stream stream, unsigned long offset,
		unsigned char* buffer, unsigned long count) {
	G_FILE* fis = (G_FILE*) stream->descriptor.pointer;
	g_fseek(fis, offset, SEEK_SET);
	if (count == 0)
		return 0;
	return g_fread(buffer, 1, count, fis);
}

static void close(FT_Stream stream) {
	G_FILE* fis = (G_FILE*) stream->descriptor.pointer;
	g_fclose(fis);
}

FT_Face TTFont::getFace(int chr, FT_UInt &glyphIndex) {
	FT_Face face;
	glyphIndex = 0;
	for (std::vector<FontFace>::iterator it = fontFaces_.begin();
			it != fontFaces_.end(); it++) {
		face = (*it).face;
		glyphIndex = FT_Get_Char_Index(face, chr);
		if (glyphIndex != 0)	// 0 means `undefined character code'
			break;
	}
	if (glyphIndex == 0)
		return NULL; // 0 means `undefined character code'
	return face;
}

TTFont::TTFont(Application *application, std::vector<FontSpec> filenames,
		float size, float smoothing, float outline, GStatus *status) :
		FontBase(application) {
	try {
		constructor(filenames, size, smoothing, outline);
	} catch (GiderosException &e) {
		if (status)
			*status = e.status();
	}
}

void TTFont::constructor(std::vector<FontSpec> filenames, float size,
		float smoothing, float outline) {
	float scalex = application_->getLogicalScaleX();
	float scaley = application_->getLogicalScaleY();

	float RESOLUTION = 72;
    smoothing_ = smoothing;
    if (smoothing_ > 1) {
		scalex /= smoothing_;
		scaley /= smoothing_;
	}

	ascender_ = 0;
	descender_ = 10000000;
	int descender = 0;
	fontFaces_.resize(filenames.size());
	int nf = 0;
	for (std::vector<FontSpec>::iterator it = filenames.begin();
			it != filenames.end(); it++) {
		const char *filename = (*it).filename.c_str();
		G_FILE* fis = g_fopen(filename, "rb");
		if (fis == NULL) {
			throw GiderosException(GStatus(6000, filename)); // Error #6000: %s: No such file or directory.
			return;
		}

		FontFace &ff = fontFaces_[nf++];
		memset(&ff.stream, 0, sizeof(ff.stream));

		g_fseek(fis, 0, SEEK_END);
		ff.stream.size = g_ftell(fis);
		g_fseek(fis, 0, SEEK_SET);
		ff.stream.descriptor.pointer = fis;
		ff.stream.read = read;
		ff.stream.close = close;
		ff.sizeMult = (*it).sizeMult;

		FT_Open_Args args;
		memset(&args, 0, sizeof(args));
		args.flags = FT_OPEN_STREAM;
		args.stream = &ff.stream;

		if (FT_Open_Face(FT_Library_Singleton::instance(), &args, 0, &ff.face))
			throw GiderosException(GStatus(6012, filename)); // Error #6012: %s: Error while reading font file.

         if (FT_Set_Char_Size(ff.face, 0L,
				(int) floor(size * (*it).sizeMult * 64 + 0.5f),
				(int) floor(RESOLUTION * scalex + 0.5f),
				(int) floor(RESOLUTION * scaley + 0.5f))) {
			FT_Done_Face(ff.face);
			ff.face = NULL;
			throw GiderosException(GStatus(6017, filename)); // Error #6017: Invalid font size.
		}

		ascender_ = std::max(ascender_,
				(int) (ff.face->size->metrics.ascender >> 6));
		descender_ = std::min(descender_,
				(int) (ff.face->size->metrics.descender >> 6));
		descender = std::max(descender,
				(int) ((ff.face->size->metrics.height
						- ff.face->size->metrics.ascender) >> 6));
	}

	height_ = ascender_ + descender;

	currentLogicalScaleX_ = scalex;
	currentLogicalScaleY_ = scaley;
	defaultSize_ = size;
	outlineSize_ = outline;

	stroker=NULL;
	if (outline>0)
	{
		FT_Stroker_New(FT_Library_Singleton::instance(), &stroker);
		FT_Stroker_Set(stroker, (FT_Fixed)(outline * 64 * (scalex+scaley)/2), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
	}
}

void TTFont::checkLogicalScale() {
	float scalex = application_->getLogicalScaleX();
	float scaley = application_->getLogicalScaleY();
	if (smoothing_ > 1) {
		scalex /= smoothing_;
		scaley /= smoothing_;
	}

	if ((scalex != currentLogicalScaleX_)
			|| (scaley != currentLogicalScaleY_)) {
	    if (stroker)
			FT_Stroker_Set(stroker, (FT_Fixed)(outlineSize_ * 64 * (scalex+scaley)/2), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
		for (std::map<wchar32_t, GlyphData>::iterator it = glyphCache_.begin();
				it != glyphCache_.end(); it++) {
			free(it->second.bitmap);
		}
		glyphCache_.clear();
		float RESOLUTION = 72;
		ascender_ = 0;
		descender_ = 10000000;
		int descender = 0;
		for (std::vector<FontFace>::iterator it = fontFaces_.begin();
				it != fontFaces_.end(); it++) {
			FT_Face face = (*it).face;
			if (!FT_Set_Char_Size(face, 0L,
					(int) floor(defaultSize_ * (*it).sizeMult * 64 + 0.5f),
					(int) floor(RESOLUTION * scalex + 0.5f),
					(int) floor(RESOLUTION * scaley + 0.5f))) {
				currentLogicalScaleX_ = scalex;
				currentLogicalScaleY_ = scaley;
				ascender_ = std::max(ascender_,
						(int) (face->size->metrics.ascender >> 6));
				descender_ = std::min(descender_,
						(int) (face->size->metrics.descender >> 6));
				descender = std::max(descender,
						(int) ((face->size->metrics.height
								- face->size->metrics.ascender) >> 6));
			}
		}
		height_ = ascender_ + descender;
		cacheVersion_++;
        if (shaper_)
            delete shaper_;
        shaper_=NULL;
	}
}

TTFont::~TTFont() {
	for (std::vector<FontFace>::iterator it = fontFaces_.begin();
			it != fontFaces_.end(); it++)
		FT_Done_Face((*it).face);
	for (std::map<wchar32_t, GlyphData>::iterator it = glyphCache_.begin();
			it != glyphCache_.end(); it++) {
		free(it->second.bitmap);
	}
}

void TTFont::getBounds(const wchar32_t *text, float letterSpacing, int *pminx,
		int *pminy, int *pmaxx, int *pmaxy) {
	checkLogicalScale();
	float scalex = currentLogicalScaleX_;

	int minx = 0x7fffffff;
	int miny = 0x7fffffff;
	int maxx = -0x7fffffff;
	int maxy = -0x7fffffff;

	int size = 0;
	for (const wchar32_t *t = text; *t; ++t, ++size)
		;

	int x = 0, y = 0;
	FT_Face prevFace = NULL;
	FT_UInt prev = 0;
	for (int i = 0; i < size; ++i) {
		FT_UInt glyphIndex;
		FT_Face face = getFace(text[i], glyphIndex);
		if (glyphIndex == 0)
			continue;

		if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT))
			continue;

		int top, left, width, height;
		if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
			FT_BBox bbox;
			FT_Outline_Get_CBox(&face->glyph->outline, &bbox);

			bbox.xMin &= ~63;
			bbox.yMin &= ~63;
			bbox.xMax = (bbox.xMax + 63) & ~63;
			bbox.yMax = (bbox.yMax + 63) & ~63;

			width = (bbox.xMax - bbox.xMin) >> 6;
			height = (bbox.yMax - bbox.yMin) >> 6;
			top = bbox.yMax >> 6;
			left = bbox.xMin >> 6;
		} else if (face->glyph->format == FT_GLYPH_FORMAT_BITMAP) {
			width = face->glyph->bitmap.width;
			height = face->glyph->bitmap.rows;
			top = face->glyph->bitmap_top;
			left = face->glyph->bitmap_left;
		} else
			continue;

		if (face == prevFace)
			x += kerning(face, prev, glyphIndex) >> 6;
		prev = glyphIndex;
		prevFace = face;

		int xo = x + left;
		int yo = y - top;

		minx = std::min(minx, xo);
		miny = std::min(miny, yo);
		maxx = std::max(maxx, xo + width);
		maxy = std::max(maxy, yo + height);

		x += face->glyph->advance.x >> 6;

		x += (int) (letterSpacing * scalex);
	}

    if (!x) minx=miny=maxx=maxy=0;

	if (pminx)
		*pminx = minx;
	if (pminy)
		*pminy = miny;
	if (pmaxx)
		*pmaxx = maxx;
	if (pmaxy)
		*pmaxy = maxy;
}

bool TTFont::shapeChunk(struct ChunkLayout &part,std::vector<wchar32_t> &wtext)
{
	if (part.styleFlags&TEXTSTYLEFLAG_SKIPSHAPING)
		return false;
    FontshaperBuilder_t builder=(FontshaperBuilder_t) g_getGlobalHook(GID_GLOBALHOOK_FONTSHAPER);
    if (!builder)
        return false;
    if (fontFaces_.size()!=1) //Multi font not supported (yet ?)
        return false;
    if (!shaper_) {
        float scalex = application_->getLogicalScaleX();
        float scaley = application_->getLogicalScaleY();
        float RESOLUTION=(int) floor(defaultSize_ * fontFaces_[0].sizeMult + 0.5f);//72; //This a empirical, but seems to work
        void *data=malloc(fontFaces_[0].stream.size);
        fontFaces_[0].stream.read(&fontFaces_[0].stream,0,(unsigned char *)data,fontFaces_[0].stream.size);
        shaper_=builder(data,fontFaces_[0].stream.size,
                    //(int) floor(defaultSize_ * fontFaces_[0].sizeMult * 64 + 0.5f),
                    fontFaces_[0].face->units_per_EM,
                    (int) floor(RESOLUTION * scalex + 0.5f),
                    (int) floor(RESOLUTION * scaley + 0.5f));
        free(data);
    }
    if (!shaper_)
        return false;
    bool shaped=shaper_->shape(part,wtext);
    if (!shaped) return false;
    for (int k=0;k<part.shaped.size();k++)
        part.shaped[k]._private=(void *) fontFaces_[0].face;
    return true;
}

void TTFont::chunkMetrics(struct ChunkLayout &part, float letterSpacing)
{
	std::vector<wchar32_t> wtext;
    size_t len = utf8_to_wchar(part.text.c_str(), part.text.size(), NULL, 0, 0);
	if (len != 0) {
		wtext.resize(len);
        utf8_to_wchar(part.text.c_str(), part.text.size(), &wtext[0], len, 0);
	}
	wtext.push_back(0);

	checkLogicalScale();
    float scalex = currentLogicalScaleX_;
    float scaley = currentLogicalScaleY_;

	int minx = 0x7fffffff;
	int miny = 0x7fffffff;
	int maxx = -0x7fffffff;
	int maxy = -0x7fffffff;

	int x = 0, y = 0;
	part.shaped.clear();
    if (shapeChunk(part,wtext)) {
        //Shaping has been done externally, iterate over glyphs instead
        len=part.shaped.size();
        for (size_t i = 0; i < len; ++i) {
            GlyphLayout &gl=part.shaped[i];
            FT_UInt glyphIndex=(FT_UInt) gl.glyph;
            FT_Face face = (FT_Face) gl._private;
            if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT))
                continue;

            int top, left, width, height;
            if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
                FT_BBox bbox;
                FT_Outline_Get_CBox(&face->glyph->outline, &bbox);

                bbox.xMin &= ~63;
                bbox.yMin &= ~63;
                bbox.xMax = (bbox.xMax + 63) & ~63;
                bbox.yMax = (bbox.yMax + 63) & ~63;

                width = (bbox.xMax - bbox.xMin) >> 6;
                height = (bbox.yMax - bbox.yMin) >> 6;
                top = bbox.yMax >> 6;
                left = bbox.xMin >> 6;
            } else if (face->glyph->format == FT_GLYPH_FORMAT_BITMAP) {
                width = face->glyph->bitmap.width;
                height = face->glyph->bitmap.rows;
                top = face->glyph->bitmap_top;
                left = face->glyph->bitmap_left;
            } else
                continue;

            gl.offX+=left;
            gl.offY-=top;

            int xo = x + gl.offX;
            int yo = y + gl.offY;

            minx = std::min(minx, xo);
            miny = std::min(miny, yo);
            maxx = std::max(maxx, xo + width);
            maxy = std::max(maxy, yo + height);

            x += gl.advX;
        }
    }
    else {
        FT_Face prevFace = NULL;
        FT_UInt prev = 0;
        struct GlyphLayout shape;
        for (size_t i = 0; i < len; ++i) {
            FT_UInt glyphIndex;
            FT_Face face = getFace(wtext[i], glyphIndex);
            if (glyphIndex == 0)
                continue;

            if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT))
                continue;

            int top, left, width, height;
            if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
                FT_BBox bbox;
                FT_Outline_Get_CBox(&face->glyph->outline, &bbox);

                bbox.xMin &= ~63;
                bbox.yMin &= ~63;
                bbox.xMax = (bbox.xMax + 63) & ~63;
                bbox.yMax = (bbox.yMax + 63) & ~63;

                width = (bbox.xMax - bbox.xMin) >> 6;
                height = (bbox.yMax - bbox.yMin) >> 6;
                top = bbox.yMax >> 6;
                left = bbox.xMin >> 6;
            } else if (face->glyph->format == FT_GLYPH_FORMAT_BITMAP) {
                width = face->glyph->bitmap.width;
                height = face->glyph->bitmap.rows;
                top = face->glyph->bitmap_top;
                left = face->glyph->bitmap_left;
            } else
                continue;

            int kx=0;
            if (face == prevFace)
                kx= kerning(face, prev, glyphIndex) >> 6;
            prev = glyphIndex;
            prevFace = face;

            int xo = x + left;
            int yo = y - top;

            minx = std::min(minx, xo);
            miny = std::min(miny, yo);
            maxx = std::max(maxx, xo + width);
            maxy = std::max(maxy, yo + height);

            x += kx+(face->glyph->advance.x >> 6);

            x += (int) (letterSpacing * scalex);

            shape.srcIndex=i;
            shape.glyph=glyphIndex;
            shape.advX=(face->glyph->advance.x >> 6)+kx+(letterSpacing * scalex);
            shape.advY=face->glyph->advance.y >> 6;
            shape.offX=left;
            shape.offY=-top;
            shape._private=face;
            part.shaped.push_back(shape);
        }
        if (prevFace)
            x += kerning(prevFace, prev, FT_Get_Char_Index(prevFace, wtext[len])) >> 6;
    }

    if (!x) minx=miny=maxx=maxy=0;

	part.x = minx / scalex;
    part.y = (miny / scaley) + part.dy;
    part.w = ((maxx-minx) / scalex)+1;
    part.h = ((maxy-miny) / scaley)+1;
	part.advX=x/scalex;
	part.advY=y/scaley;
}

Dib TTFont::renderFont(const char *text, TextLayoutParameters *layout,
        int *pminx, int *pminy, int *pmaxx, int *pmaxy, unsigned int color, bool &isRGB, TextLayout &l) {
	checkLogicalScale();
    float scalex = currentLogicalScaleX_;
    float scaley = currentLogicalScaleY_;

    l = layoutText(text, layout);

    Dib dib(application_, (l.w+2*outlineSize_)*scalex + 2, (l.h+2*outlineSize_)*scaley + 2, true);
	unsigned char rgba[] = { 255, 255, 255, 0 };
	dib.fill(rgba);

    for (size_t pn = 0; pn < l.parts.size(); pn++) {
		ChunkLayout c = l.parts[pn];
        size_t wsize=c.shaped.size();

        int x = 1 + (c.dx+outlineSize_)*scalex, y = 1 + (c.dy+outlineSize_)*scaley;
        for (size_t i = 0; i < wsize; ++i) {
            GlyphLayout &gl=c.shaped[i];
            GlyphData g = glyphCache_[gl.glyph];
			if (g.bitmap == NULL) {
                FT_Face face = (FT_Face)gl._private;
                if (FT_Load_Glyph(face, gl.glyph, FT_LOAD_DEFAULT))
					continue;

				int top, left, width, height;
				if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
					FT_BBox bbox;
					FT_Outline_Get_CBox(&face->glyph->outline, &bbox);

					bbox.xMin &= ~63;
					bbox.yMin &= ~63;
					bbox.xMax = (bbox.xMax + 63) & ~63;
					bbox.yMax = (bbox.yMax + 63) & ~63;

					width = (bbox.xMax - bbox.xMin) >> 6;
					height = (bbox.yMax - bbox.yMin) >> 6;
					top = bbox.yMax >> 6;
					left = bbox.xMin >> 6;
				} else if (face->glyph->format == FT_GLYPH_FORMAT_BITMAP) {
					width = face->glyph->bitmap.width;
					height = face->glyph->bitmap.rows;
					top = face->glyph->bitmap_top;
					left = face->glyph->bitmap_left;
				} else
					continue;

				FT_Bitmap *bitmap=NULL;
				FT_Glyph glyph=nullptr;
				if (stroker) {
                    if (FT_Get_Glyph(face->glyph, &glyph))
						continue;
					if (FT_Glyph_StrokeBorder(&glyph, stroker, false, true))
					{
						FT_Done_Glyph(glyph); continue;
					}

					FT_BBox bbox;
					FT_OutlineGlyph oGlyph = reinterpret_cast<FT_OutlineGlyph>(glyph);
					FT_Outline_Get_CBox(&oGlyph->outline, &bbox);

					bbox.xMin &= ~63;
					bbox.yMin &= ~63;
					bbox.xMax = (bbox.xMax + 63) & ~63;
					bbox.yMax = (bbox.yMax + 63) & ~63;

					width = (bbox.xMax - bbox.xMin) >> 6;
					height = (bbox.yMax - bbox.yMin) >> 6;
					top = bbox.yMax >> 6;
					left = bbox.xMin >> 6;

					if (FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true))
					{
						FT_Done_Glyph(glyph); continue;
					}

					FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
					bitmap=&(bitmapGlyph->bitmap);
				}
				else {
                    if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
						continue;
                     bitmap = &(face->glyph->bitmap);
				}

				width = std::min(width, (int) bitmap->width);
				height = std::min(height, (int) bitmap->rows);

				g.face = face;
				g.pitch = bitmap->pitch;
				g.height = height;
				g.width = width;
				g.top = top;
				g.left = left;
                g.glyph = gl.glyph;
				g.advX = face->glyph->advance.x >> 6;
				g.bitmap = (unsigned char *) malloc(g.height * g.pitch);
				memcpy(g.bitmap, bitmap->buffer, g.height * g.pitch);
                glyphCache_[gl.glyph] = g;
                if (glyph) FT_Done_Glyph(glyph);
			}

            int xo = x + gl.offX - l.x*scalex;
            int yo = y + gl.offY - l.y*scaley;
			int index = 0;

			if (l.styleFlags&TEXTSTYLEFLAG_COLOR) {
				unsigned char rgba[4];
                unsigned int col=(c.styleFlags&TEXTSTYLEFLAG_COLOR)?c.color:color;
				rgba[0]=(col>>16)&0xFF;
				rgba[1]=(col>>8)&0xFF;
				rgba[2]=(col>>0)&0xFF;
				for (unsigned int y = 0; y < g.height; ++y) {
					for (unsigned int x = 0; x < g.width; ++x)
					{
                        rgba[3]=(((col>>24)&0xFF)*g.bitmap[index++])>>8;
						dib.setPixel(xo + x, yo + y, rgba);
					}
					index = index + g.pitch - g.width;
				}
			}
			else
			for (unsigned int y = 0; y < g.height; ++y) {
				for (unsigned int x = 0; x < g.width; ++x)
					dib.satAlpha(xo + x, yo + y, g.bitmap[index++]);
				index = index + g.pitch - g.width;
			}

            x += gl.advX;
		}

	}

    isRGB=(l.styleFlags&TEXTSTYLEFLAG_COLOR);
    if (isRGB)
		dib.premultiplyAlpha();

	if (pminx)
        *pminx = (l.x-outlineSize_)*scalex;
	if (pminy)
        *pminy = (l.y-outlineSize_)*scaley;
	if (pmaxx)
        *pmaxx = (l.x + l.w + 2*outlineSize_)*scalex;
	if (pmaxy)
        *pmaxy = (l.y + l.h + 2*outlineSize_)*scaley;

	return dib;
}

void TTFont::getBounds(const char *text, float letterSpacing, float *pminx,
		float *pminy, float *pmaxx, float *pmaxy) {
	std::vector<wchar32_t> wtext;
	size_t len = utf8_to_wchar(text, strlen(text), NULL, 0, 0);
	if (len != 0) {
		wtext.resize(len);
		utf8_to_wchar(text, strlen(text), &wtext[0], len, 0);
	}
	wtext.push_back(0);

	int minx, miny, maxx, maxy;
	getBounds(&wtext[0], letterSpacing, &minx, &miny, &maxx, &maxy);

	float scalex = currentLogicalScaleX_;
	float scaley = currentLogicalScaleY_;

	if (pminx)
		*pminx = minx / scalex;
	if (pminy)
		*pminy = miny / scaley;
	if (pmaxx)
		*pmaxx = maxx / scalex;
	if (pmaxy)
		*pmaxy = maxy / scaley;
}

float TTFont::getAdvanceX(const char *text, float letterSpacing, int size) {
	checkLogicalScale();
	float scalex = currentLogicalScaleX_;

	std::vector<wchar32_t> wtext;
	size_t len = utf8_to_wchar(text, strlen(text), NULL, 0, 0);
	if (len != 0) {
		wtext.resize(len);
		utf8_to_wchar(text, strlen(text), &wtext[0], len, 0);
	}

	if (size < 0 || size > (int) (wtext.size()))
		size = wtext.size();

	wtext.push_back(0);

	float x = 0;
	FT_Face prevFace = NULL;
	FT_UInt prev = 0;
	for (int i = 0; i < size; ++i) {
		FT_UInt glyphIndex;
		FT_Face face = getFace(wtext[i], glyphIndex);
		if (glyphIndex == 0)
			continue;

		if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT))
			continue;

		if (prevFace == face)
			x += kerning(face, prev, glyphIndex) >> 6;
		prev = glyphIndex;
		prevFace = face;

		x += face->glyph->advance.x >> 6;

		x += (letterSpacing * scalex);
	}

    if (prevFace)
        x += kerning(prevFace, prev, FT_Get_Char_Index(prevFace, wtext[size])) >> 6;

	return x / scalex;
}

float TTFont::getCharIndexAtOffset(const char *text, float offset, float letterSpacing, int size)
{
	checkLogicalScale();
	float scalex = currentLogicalScaleX_;
	offset*=scalex;

	std::vector<wchar32_t> wtext;
	size_t len = utf8_to_wchar(text, strlen(text), NULL, 0, 0);
	if (len != 0) {
		wtext.resize(len);
		utf8_to_wchar(text, strlen(text), &wtext[0], len, 0);
	}

	if (size < 0 || size > (int) (wtext.size()))
		size = wtext.size();

	wtext.push_back(0);

	int x = 0;
	FT_Face prevFace = NULL;
	FT_UInt prev = 0;
	int px=0;
	for (int i = 0; i < size; ++i) {
		FT_UInt glyphIndex;
		FT_Face face = getFace(wtext[i], glyphIndex);
		if (glyphIndex == 0)
			continue;

		if (FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT))
			continue;

		if (prevFace == face)
			x += kerning(face, prev, glyphIndex) >> 6;
		prev = glyphIndex;
		prevFace = face;

		x += face->glyph->advance.x >> 6;

		x += (int) (letterSpacing * scalex);
		if ((x>px)&&(offset>=px)&&(offset<x))
		{
			const char *tp=text;
			while (i--)
			{
				char fc=*(tp++);
				if ((fc&0xE0)==0xC0) tp++;
				if ((fc&0xF0)==0xE0) tp+=2;
				if ((fc&0xF8)==0xF0) tp+=3;
			}
			float rv=tp-text;
			return rv+(offset-px)/(x-px);
		}
	}

	return strlen(text);
}


int TTFont::kerning(FT_Face face, FT_UInt left, FT_UInt right) const {
	if (FT_HAS_KERNING(face)) {
		FT_Vector delta;
		FT_Get_Kerning(face, left, right, FT_KERNING_DEFAULT, &delta);
		return delta.x;
	}

	return 0;
}

float TTFont::getAscender() {
	checkLogicalScale();
	float scaley = currentLogicalScaleY_;
	return ascender_ / scaley;
}

float TTFont::getDescender() {
	checkLogicalScale();
	float scaley = currentLogicalScaleY_;
	return -descender_ / scaley;
}

float TTFont::getLineHeight() {
	checkLogicalScale();
	float scaley = currentLogicalScaleY_;
	return height_ / scaley;
}
