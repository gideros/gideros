#include "ttfont.h"
#include <gfile.h>
#include <gstdio.h>
#include "giderosexception.h"
#include "gstatus.h"
#include "application.h"
#include "ftlibrarysingleton.h"

#include <ft2build.h>
#include FT_OUTLINE_H

#include <utf8.h>
#include <algorithm>
#include "path.h"

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
		float size, float smoothing, GStatus *status) :
		FontBase(application) {
	try {
		constructor(filenames, size, smoothing);
	} catch (GiderosException &e) {
		if (status)
			*status = e.status();
	}
}

void TTFont::constructor(std::vector<FontSpec> filenames, float size,
		float smoothing) {
	float scalex = application_->getLogicalScaleX();
	float scaley = application_->getLogicalScaleY();

	float RESOLUTION = 72;
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

		smoothing_ = smoothing;

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

Dib TTFont::renderFont(const char *text, TextLayoutParameters *layout,
        int *pminx, int *pminy, int *pmaxx, int *pmaxy, unsigned int color, bool &isRGB, TextLayout &l) {
	checkLogicalScale();
    float scalex = currentLogicalScaleX_;
    float scaley = currentLogicalScaleY_;

    l = layoutText(text, layout);

    Dib dib(application_, l.w*scalex + 2, l.h*scaley + 2, true);
	unsigned char rgba[] = { 255, 255, 255, 0 };
	dib.fill(rgba);

    for (size_t pn = 0; pn < l.parts.size(); pn++) {
		ChunkLayout c = l.parts[pn];
		std::basic_string<wchar32_t> wtext;
		size_t wsize = utf8_to_wchar(c.text.c_str(), c.text.size(), NULL, 0, 0);
		wtext.resize(wsize);
		utf8_to_wchar(c.text.c_str(), c.text.size(), &wtext[0], wsize, 0);

        int x = 1 + c.dx*scalex, y = 1 + c.dy*scaley;
		FT_UInt prev = 0;
		FT_Face prevFace = NULL;
        for (size_t i = 0; i < wsize; ++i) {
			GlyphData g = glyphCache_[wtext[i]];
			if (g.bitmap == NULL) {
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

				if (FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL))
					continue;

				FT_Bitmap &bitmap = face->glyph->bitmap;
				width = std::min(width, (int) bitmap.width);
				height = std::min(height, (int) bitmap.rows);

				g.face = face;
				g.pitch = bitmap.pitch;
				g.height = height;
				g.width = width;
				g.top = top;
				g.left = left;
				g.glyph = glyphIndex;
				g.advX = face->glyph->advance.x >> 6;
				g.bitmap = (unsigned char *) malloc(g.height * g.pitch);
				memcpy(g.bitmap, bitmap.buffer, g.height * g.pitch);
                glyphCache_[wtext[i]] = g;
			}

			if (prevFace == g.face)
				x += kerning(g.face, prev, g.glyph) >> 6;
			prev = g.glyph;
			prevFace = g.face;

            int xo = x + g.left - l.x*scalex;
            int yo = y - g.top - l.y*scaley;
			int index = 0;

			if (l.styleFlags&TEXTSTYLEFLAG_COLOR) {
				unsigned char rgba[4];
                unsigned int col=(c.styleFlags&TEXTSTYLEFLAG_COLOR)?c.color:color|0xFF000000;
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

			x += g.advX;

			x += (int) (layout->letterSpacing * scalex);
		}

	}

    isRGB=(l.styleFlags&TEXTSTYLEFLAG_COLOR);
    if (isRGB)
		dib.premultiplyAlpha();

	if (pminx)
        *pminx = l.x*scalex;
	if (pminy)
        *pminy = l.y*scaley;
	if (pmaxx)
        *pmaxx = (l.x + l.w)*scalex;
	if (pmaxy)
        *pmaxy = (l.y + l.h)*scaley;

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

	int x = 0;
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

		x += (int) (letterSpacing * scalex);
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
