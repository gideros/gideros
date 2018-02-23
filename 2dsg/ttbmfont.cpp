#include <ttbmfont.h>

#include <ft2build.h>
#include FT_OUTLINE_H

#include <ftlibrarysingleton.h>

#include <gstdio.h>
#include <gstatus.h>
#include <giderosexception.h>
#include <application.h>
#include <texturepacker.h>
#include <graphicsbase.h>
#include <utf8.h>
#include <algorithm>

// Textures shouldn't exceed 1024x1024 on most platforms, so limit possible font size to allow at least a few chars to be rendered
#define FONT_SIZE_LIMIT 300.0
static unsigned long read(FT_Stream stream, unsigned long offset,
		unsigned char *buffer, unsigned long count) {
	G_FILE *fis = (G_FILE*) stream->descriptor.pointer;
	g_fseek(fis, offset, SEEK_SET);
	if (count == 0)
		return 0;
	return g_fread(buffer, 1, count, fis);
}

static void close(FT_Stream stream) {
	G_FILE *fis = (G_FILE*) stream->descriptor.pointer;
	g_fclose(fis);
}

TTBMFont::TTBMFont(Application *application, std::vector<FontSpec> filenames,
		float size, const char *chars, float filtering, GStatus *status) :
		BMFontBase(application) {
	try {
		constructor(filenames, size, chars, filtering);
	} catch (GiderosException &e) {
		if (status)
			*status = e.status();
	}
}

bool TTBMFont::addGlyph(const wchar32_t chr) {
	FT_Face face;
	FT_UInt glyphIndex = 0;
	for (std::vector<FontFace>::iterator it = fontFaces_.begin();
			it != fontFaces_.end(); it++) {
		face = (*it).face;
		glyphIndex = FT_Get_Char_Index(face, chr);
		if (glyphIndex != 0)	// 0 means `undefined character code'
			break;
	}
	if (glyphIndex == 0)	// 0 means `undefined character code'
		return false;

	FT_Error error;
	error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
	if (error)
		return false;

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
		return false;

	TextureGlyph textureGlyph;
	textureGlyph.chr = chr;
	textureGlyph.glyphIndex = glyphIndex;
	textureGlyph.top = top;
	textureGlyph.left = left;
	textureGlyph.width = width;
	textureGlyph.height = height;
	textureGlyph.advancex = face->glyph->advance.x;
	textureGlyph.advancey = face->glyph->advance.y;
	textureGlyph.face = face;

	fontInfo_.textureGlyphs[chr] = textureGlyph;
	return true;
}

bool TTBMFont::staticCharsetInit() {
	std::vector<wchar32_t> wchars;
	size_t len = utf8_to_wchar(charset_.c_str(), charset_.size(), NULL, 0, 0);
	if (len != 0) {
		wchars.resize(len);
		utf8_to_wchar(charset_.c_str(), charset_.size(), &wchars[0], len, 0);
	}

	if (!len)
		return false;

	FT_Error error;
	std::map<wchar32_t, TextureGlyph> &textureGlyphs = fontInfo_.textureGlyphs;

	for (size_t i = 0; i < len; ++i) {
		wchar32_t chr = wchars[i];
		addGlyph(chr);
	}

	std::map<std::pair<wchar32_t, wchar32_t>, int> &kernings =
			fontInfo_.kernings;

	kernings.clear();

	std::map<wchar32_t, TextureGlyph>::iterator iter1, iter2, e =
			textureGlyphs.end();

	for (iter1 = textureGlyphs.begin(); iter1 != e; ++iter1)
		for (iter2 = textureGlyphs.begin(); iter2 != e; ++iter2) {
			const TextureGlyph &g1 = iter1->second;
			const TextureGlyph &g2 = iter2->second;
			if ((g1.face == g2.face) && FT_HAS_KERNING(g1.face)) {

				FT_Vector delta;
				FT_Get_Kerning(g1.face, g1.glyphIndex, g2.glyphIndex,
						FT_KERNING_DEFAULT, &delta);

				if (delta.x != 0)
					kernings[std::make_pair(g1.chr, g2.chr)] = delta.x;
			}
		}

	TexturePacker *tp = createTexturePacker();

	tp->setTextureCount(textureGlyphs.size());
	std::map<wchar32_t, TextureGlyph>::iterator iter;
	for (iter = textureGlyphs.begin(); iter != e; ++iter)
		tp->addTexture(iter->second.width, iter->second.height);

	int width = 0, height = 0;
	tp->packTextures(&width, &height, 2, false);

	width = std::max(width, 1);
	height = std::max(height, 1);

	Dib dib(application_, width, height, true);
	unsigned char rgba[] = { 255, 255, 255, 0 };
	dib.fill(rgba);

	int i = 0;
	for (iter = textureGlyphs.begin(); iter != e; ++iter, ++i) {
		int xo, yo;
		int width, height;
		tp->getTextureLocation(i, &xo, &yo, &width, &height);
		const TextureGlyph &g1 = iter->second;

		FT_UInt glyph_index = FT_Get_Char_Index(g1.face, g1.chr);
		if (glyph_index == 0)
			continue;

		error = FT_Load_Glyph(g1.face, glyph_index, FT_LOAD_DEFAULT);
		if (error)
			continue;

		error = FT_Render_Glyph(g1.face->glyph, FT_RENDER_MODE_NORMAL);
		if (error)
			continue;

		FT_Bitmap &bitmap = g1.face->glyph->bitmap;

		iter->second.x = xo;
		iter->second.y = yo;
		iter->second.texture = 0;

		width = std::min(width, (int) bitmap.width);
		height = std::min(height, (int) bitmap.rows);

		for (int y = 0; y < height; ++y)
			for (int x = 0; x < width; ++x) {
				int index = x + y * bitmap.pitch;
				int c = bitmap.buffer[index];

				dib.setAlpha(xo + x, yo + y, c);
			}
	}

	releaseTexturePacker(tp);

	TextureParameters parameters;
	parameters.filter = (filtering_ != 0) ? eLinear : eNearest;
	parameters.wrap = eClamp;
	parameters.format = eA8;
	textureData_.push_back(
			application_->getTextureManager()->createTextureFromDib(dib,
					parameters));
	return true;
}

void TTBMFont::constructor(std::vector<FontSpec> filenames, float size,
		const char *chars, float filtering) {

    currentDib_ = NULL;
    currentPacker_ = NULL;

	fontInfo_.ascender = 0;
	fontInfo_.descender = 0;
	defaultSize_ = size;
	FT_Error error;

	float scalex = application_->getLogicalScaleX();
	float scaley = application_->getLogicalScaleY();

	float RESOLUTION = 72;
	if (filtering > 1) {
		scalex /= filtering;
		scaley /= filtering;
	}
    //Limit font size
    float scaleRatio=(FONT_SIZE_LIMIT/defaultSize_)/scaley;
    if (scaleRatio<1)
    {
        scaley*=scaleRatio;
        scalex*=scaleRatio;
    }


	fontFaces_.resize(filenames.size());
	int nf = 0;
	for (std::vector<FontSpec>::iterator it = filenames.begin();
			it != filenames.end(); it++) {
		G_FILE *fis = g_fopen((*it).filename.c_str(), "rb");
		if (!fis)
			throw GiderosException(GStatus(6000, (*it).filename.c_str()));// Error #6000: %s: No such file or directory.

		FontFace &ff = fontFaces_[nf++];
		memset(&ff.stream, 0, sizeof(ff.stream));

		g_fseek(fis, 0, SEEK_END);
		ff.stream.size = g_ftell(fis);
		g_fseek(fis, 0, SEEK_SET);
		ff.stream.descriptor.pointer = fis;
		ff.stream.read = read;
		ff.stream.close = close;

		FT_Open_Args args;
		memset(&args, 0, sizeof(args));
		args.flags = FT_OPEN_STREAM;
		args.stream = &ff.stream;

		error = FT_Open_Face(FT_Library_Singleton::instance(), &args, 0,
				&ff.face);
		if (error)
			throw GiderosException(GStatus(6012, (*it).filename.c_str()));// Error #6012: %s: Error while reading font file.

		error = FT_Set_Char_Size(ff.face, 0L,
				(int) floor(size * 64 * (*it).sizeMult + 0.5f),
				(int) floor(RESOLUTION * scalex + 0.5f),
				(int) floor(RESOLUTION * scaley + 0.5f));
		ff.sizeMult = (*it).sizeMult;

		if (error) {
			FT_Done_Face(ff.face);
			throw GiderosException(GStatus(6017, (*it).filename.c_str()));// Error #6017: Invalid font size.
		}

		fontInfo_.ascender = std::max(fontInfo_.ascender,
				(int) (ff.face->size->metrics.ascender >> 6));
		fontInfo_.descender = std::max(fontInfo_.descender,
				(int) ((ff.face->size->metrics.height
						- ff.face->size->metrics.ascender) >> 6));
	}

	fontInfo_.height = fontInfo_.ascender + fontInfo_.descender;

	charset_ = chars;

	std::map<wchar32_t, TextureGlyph> &textureGlyphs = fontInfo_.textureGlyphs;

	textureGlyphs.clear();

	std::vector<wchar32_t> wchars;
	size_t len = utf8_to_wchar(charset_.c_str(), charset_.size(), NULL, 0, 0);
	if (len != 0) {
		wchars.resize(len);
		utf8_to_wchar(charset_.c_str(), charset_.size(), &wchars[0], len, 0);
	}

	if (!staticCharsetInit()) {
		currentDib_ = new Dib(application_, 1024, 1024, true); //use 102x1024 textures for packing
		currentPacker_ = createProgressiveTexturePacker(currentDib_->width(),
				currentDib_->height());
		TextureParameters parameters;
		parameters.filter = (filtering != 0) ? eLinear : eNearest;
		parameters.wrap = eClamp;
		parameters.format = eA8;
		textureData_.push_back(
				application_->getTextureManager()->createTextureFromDib(
						*currentDib_, parameters));
	}

	filtering_ = filtering;
	sizescalex_ = 1 / scalex;
	sizescaley_ = 1 / scaley;
	uvscalex_ = 1;
	uvscaley_ = 1;
	currentLogicalScaleX_ = scalex;
	currentLogicalScaleY_ = scaley;
}

void TTBMFont::ensureChars(const wchar32_t *text, int size) {
	checkLogicalScale();
	if (!currentPacker_)
		return;
	std::map<std::pair<wchar32_t, wchar32_t>, int> &kernings =
			fontInfo_.kernings;
	std::map<wchar32_t, TextureGlyph> &textureGlyphs = fontInfo_.textureGlyphs;
	bool updateTexture = false;
	wchar32_t lchar = 0;
	for (const wchar32_t *t = text; size; size--, t++) {
		wchar32_t chr = *t;
		bool newGlyph = false;
		if (textureGlyphs.find(chr) == textureGlyphs.end()) {
			if (!addGlyph(chr))
				continue;
			newGlyph = true;
		}
		const TextureGlyph &g = textureGlyphs[chr];
		if (lchar) {
			const TextureGlyph &gl = textureGlyphs[lchar];
			if ((g.face == gl.face) && FT_HAS_KERNING(gl.face)) {
				FT_Vector delta;
				FT_Get_Kerning(gl.face, textureGlyphs[lchar].glyphIndex,
						textureGlyphs[chr].glyphIndex, FT_KERNING_DEFAULT,
						&delta);

				if (delta.x != 0)
					kernings[std::make_pair(lchar, chr)] = delta.x;
			}
		}
		lchar=chr;
		if (newGlyph) {
			if (!currentPacker_->addTexture(g.width, g.height)) {
				//Build a new layer
				if (updateTexture) {
					application_->getTextureManager()->updateTextureFromDib(
							textureData_[textureData_.size() - 1],
							*currentDib_);
					updateTexture = false;
				}
				releaseTexturePacker(currentPacker_);
				currentPacker_ = createProgressiveTexturePacker(
						currentDib_->width(), currentDib_->height());
				unsigned char rgba[] = { 255, 255, 255, 0 };
				currentDib_->fill(rgba);
				TextureParameters parameters;
				parameters.filter = (filtering_ != 0) ? eLinear : eNearest;
				parameters.wrap = eClamp;
				parameters.format = eA8;
				textureData_.push_back(
						application_->getTextureManager()->createTextureFromDib(
								*currentDib_, parameters));
				currentPacker_->addTexture(g.width, g.height);
			}
			int xo, yo;
			int width, height;
			currentPacker_->getTextureLocation(-1, &xo, &yo, &width, &height);

			FT_UInt glyph_index = FT_Get_Char_Index(g.face, chr);
			if (glyph_index == 0)
				continue;
			FT_Error error;

			error = FT_Load_Glyph(g.face, glyph_index, FT_LOAD_DEFAULT);
			if (error)
				continue;

			error = FT_Render_Glyph(g.face->glyph, FT_RENDER_MODE_NORMAL);
			if (error)
				continue;

			FT_Bitmap &bitmap = g.face->glyph->bitmap;

			textureGlyphs[chr].x = xo;
			textureGlyphs[chr].y = yo;
			textureGlyphs[chr].texture = textureData_.size() - 1;

			width = std::min(width, (int) bitmap.width);
			height = std::min(height, (int) bitmap.rows);

			for (int y = 0; y < height; ++y)
				for (int x = 0; x < width; ++x) {
					int index = x + y * bitmap.pitch;
					int c = bitmap.buffer[index];

					currentDib_->setAlpha(xo + x, yo + y, c);
				}
			updateTexture = true;
		}
	}
	if (updateTexture) {
		application_->getTextureManager()->updateTextureFromDib(
				textureData_[textureData_.size() - 1], *currentDib_);
		updateTexture = false;
	}
}

TTBMFont::~TTBMFont() {
	for (std::vector<TextureData *>::iterator it = textureData_.begin();
			it != textureData_.end(); it++)
		if (*it)
			application_->getTextureManager()->destroyTexture(*it);
	for (std::vector<FontFace>::iterator it = fontFaces_.begin();
			it != fontFaces_.end(); it++)
		FT_Done_Face((*it).face);
	fontFaces_.clear();
	if (currentDib_)
		delete currentDib_;
	if (currentPacker_)
		releaseTexturePacker(currentPacker_);
	fontInfo_.kernings.clear();
	fontInfo_.textureGlyphs.clear();
}

void TTBMFont::checkLogicalScale() {
	float scalex = application_->getLogicalScaleX();
	float scaley = application_->getLogicalScaleY();
	if (filtering_ > 1) {
		scalex /= filtering_;
		scaley /= filtering_;
	}
    float scaleRatio=(FONT_SIZE_LIMIT/defaultSize_)/scaley;
    if (scaleRatio<1)
    {
        scaley*=scaleRatio;
        scalex*=scaleRatio;
    }

	float RESOLUTION = 72;
	if ((fabs(scalex-currentLogicalScaleX_)>0.01)
			|| (fabs(scaley-currentLogicalScaleY_)>0.01)) {
		sizescalex_ = 1 / scalex;
		sizescaley_ = 1 / scaley;

		currentLogicalScaleX_ = scalex;
		currentLogicalScaleY_ = scaley;
		for (std::vector<TextureData *>::iterator it = textureData_.begin();
				it != textureData_.end(); it++)
			if (*it)
				application_->getTextureManager()->destroyTexture(*it);
		textureData_.clear();

		fontInfo_.ascender = 0;
		fontInfo_.descender = 0;

		for (std::vector<FontFace>::iterator it = fontFaces_.begin();
				it != fontFaces_.end(); it++) {
			FT_Face face = (*it).face;
			if (!FT_Set_Char_Size(face, 0L,
					(int) floor(defaultSize_ * (*it).sizeMult * 64 + 0.5f),
					(int) floor(RESOLUTION * scalex + 0.5f),
					(int) floor(RESOLUTION * scaley + 0.5f))) {
			}
			fontInfo_.ascender = std::max(fontInfo_.ascender,
					(int) ((*it).face->size->metrics.ascender >> 6));
			fontInfo_.descender = std::max(fontInfo_.descender,
					(int) (((*it).face->size->metrics.height
							- (*it).face->size->metrics.ascender) >> 6));
		}
		fontInfo_.kernings.clear();
		fontInfo_.textureGlyphs.clear();

		if (!staticCharsetInit()) {
			if (currentDib_)
				delete currentDib_;
			if (currentPacker_)
				releaseTexturePacker(currentPacker_);
			currentDib_ = new Dib(application_, 1024, 1024, true); //use 102x1024 textures for packing
			currentPacker_ = createProgressiveTexturePacker(
					currentDib_->width(), currentDib_->height());
			TextureParameters parameters;
			parameters.filter = (filtering_ != 0) ? eLinear : eNearest;
			parameters.wrap = eClamp;
			parameters.format = eA8;
			textureData_.push_back(
					application_->getTextureManager()->createTextureFromDib(
							*currentDib_, parameters));
		}
		fontInfo_.height = fontInfo_.ascender + fontInfo_.descender;
	}
}

void TTBMFont::drawText(std::vector<GraphicsBase>* vGraphicsBase,
		const char* text, float r, float g, float b,
        TextLayoutParameters *layout, bool hasSample, float minx, float miny,TextLayout &l) {

	if (strlen(text) == 0) {
		vGraphicsBase->clear();
		return;
	}

    l = layoutText(text, layout);

	std::map<int, int> layerMap;
	std::map<int, int> gfxMap;
	std::map<int, int> gfxMap2;
	int gfx = 0;

	for (size_t pn = 0; pn < l.parts.size(); pn++) {
		ChunkLayout c = l.parts[pn];

		unsigned char rgba[4];
		if (l.styleFlags&TEXTSTYLEFLAG_COLOR)
		{
			float ca=(c.styleFlags&TEXTSTYLEFLAG_COLOR)?(1.0/255)*((c.color>>24)&0xFF):1.0f;
			rgba[0]=(unsigned char)(ca*((c.styleFlags&TEXTSTYLEFLAG_COLOR)?(c.color>>16)&0xFF:r*255));
			rgba[1]=(unsigned char)(ca*((c.styleFlags&TEXTSTYLEFLAG_COLOR)?(c.color>>8)&0xFF:g*255));
			rgba[2]=(unsigned char)(ca*((c.styleFlags&TEXTSTYLEFLAG_COLOR)?(c.color>>0)&0xFF:b*255));
			rgba[3]=(unsigned char)(ca*255);
		}

		std::basic_string<wchar32_t> wtext;
		size_t wsize = utf8_to_wchar(c.text.c_str(), c.text.size(), NULL, 0, 0);
		wtext.resize(wsize);
		utf8_to_wchar(c.text.c_str(), c.text.size(), &wtext[0], wsize, 0);
		ensureChars(&wtext[0], wsize);

		for (size_t i = 0; i < wsize; ++i) {
			std::map<wchar32_t, TextureGlyph>::const_iterator iter =
					fontInfo_.textureGlyphs.find(wtext[i]);

			if (iter == fontInfo_.textureGlyphs.end())
				continue;
			const TextureGlyph &textureGlyph = iter->second;
			int l = layerMap[textureGlyph.texture];
			if (!l) {
				gfx++;
				layerMap[textureGlyph.texture] = gfx;
				l = gfx;
			}
			l--;
			gfxMap[l] = gfxMap[l] + 1;
		}

		vGraphicsBase->resize(gfx);

		for (std::map<int, int>::iterator it = gfxMap.begin();
				it != gfxMap.end(); it++) {
			GraphicsBase *graphicsBase = &((*vGraphicsBase)[it->first]);
			int size = it->second;
			if (l.styleFlags&TEXTSTYLEFLAG_COLOR)
			{
				graphicsBase->colors.resize(size * 16);
				graphicsBase->colors.Update();
			}
			else
				graphicsBase->setColor(r, g, b, 1);
			graphicsBase->vertices.resize(size * 4);
			graphicsBase->texcoords.resize(size * 4);
			graphicsBase->indices.resize(size * 6);
			graphicsBase->vertices.Update();
			graphicsBase->texcoords.Update();
			graphicsBase->indices.Update();
		}

        float x = (c.dx-minx) / sizescalex_, y = (c.dy-miny) / sizescaley_;

        /* if (hasSample) {
			std::map<wchar32_t, TextureGlyph>::const_iterator iter =
					fontInfo_.textureGlyphs.find(text[0]);
			const TextureGlyph &textureGlyph = iter->second;
            x = c.dx / sizescalex_ - textureGlyph.left; //FIXME is this needed ?
        }*/

		wchar32_t prev = 0;

		for (size_t i = 0; i < wsize; ++i) {
			std::map<wchar32_t, TextureGlyph>::const_iterator iter =
					fontInfo_.textureGlyphs.find(wtext[i]);

			if (iter == fontInfo_.textureGlyphs.end())
				continue;

			const TextureGlyph &textureGlyph = iter->second;
			int gfx = layerMap[textureGlyph.texture] - 1;
			GraphicsBase *graphicsBase = &((*vGraphicsBase)[gfx]);
			graphicsBase->data = textureData_[textureGlyph.texture];

			int width = textureGlyph.width;
			int height = textureGlyph.height;
			int left = textureGlyph.left;
			int top = textureGlyph.top;

			x += kerning(prev, wtext[i]) >> 6;
			prev = wtext[i];

			float x0 = x + left-1;
			float y0 = y - top-1;

			float x1 = x0 + width+2;
			float y1 = y0 + height+2;
			int vi = gfxMap2[gfx];
			gfxMap2[gfx] = vi + 1;

			graphicsBase->vertices[vi * 4 + 0] = Point2f(sizescalex_ * x0,
					sizescaley_ * y0);
			graphicsBase->vertices[vi * 4 + 1] = Point2f(sizescalex_ * x1,
					sizescaley_ * y0);
			graphicsBase->vertices[vi * 4 + 2] = Point2f(sizescalex_ * x1,
					sizescaley_ * y1);
			graphicsBase->vertices[vi * 4 + 3] = Point2f(sizescalex_ * x0,
					sizescaley_ * y1);

			float u0 = ((float) textureGlyph.x-1.0)
					/ (float) textureData_[textureGlyph.texture]->exwidth;
			float v0 = ((float) textureGlyph.y-1.0)
					/ (float) textureData_[textureGlyph.texture]->exheight;
			float u1 = (float) (textureGlyph.x + width + 1)
					/ (float) textureData_[textureGlyph.texture]->exwidth;
			float v1 = (float) (textureGlyph.y + height + 1)
					/ (float) textureData_[textureGlyph.texture]->exheight;

			u0 *= uvscalex_;
			v0 *= uvscaley_;
			u1 *= uvscalex_;
			v1 *= uvscaley_;

			graphicsBase->texcoords[vi * 4 + 0] = Point2f(u0, v0);
			graphicsBase->texcoords[vi * 4 + 1] = Point2f(u1, v0);
			graphicsBase->texcoords[vi * 4 + 2] = Point2f(u1, v1);
			graphicsBase->texcoords[vi * 4 + 3] = Point2f(u0, v1);

			if (l.styleFlags&TEXTSTYLEFLAG_COLOR)
			{
				for (int v=0;v<16;v+=4) {
					graphicsBase->colors[vi * 16 + 0 + v] = rgba[0];
					graphicsBase->colors[vi * 16 + 1 + v] = rgba[1];
					graphicsBase->colors[vi * 16 + 2 + v] = rgba[2];
					graphicsBase->colors[vi * 16 + 3 + v] = rgba[3];
				}
			}


			graphicsBase->indices[vi * 6 + 0] = vi * 4 + 0;
			graphicsBase->indices[vi * 6 + 1] = vi * 4 + 1;
			graphicsBase->indices[vi * 6 + 2] = vi * 4 + 2;
			graphicsBase->indices[vi * 6 + 3] = vi * 4 + 0;
			graphicsBase->indices[vi * 6 + 4] = vi * 4 + 2;
			graphicsBase->indices[vi * 6 + 5] = vi * 4 + 3;

			x += (textureGlyph.advancex) >> 6;

			x += layout->letterSpacing / sizescalex_;
		}
	}
}

void TTBMFont::getBounds(const char *text, float letterSpacing, float *pminx,
		float *pminy, float *pmaxx, float *pmaxy) {
	float minx = 1e30;
	float miny = 1e30;
	float maxx = -1e30;
	float maxy = -1e30;

	std::vector<wchar32_t> wtext;
	size_t len = utf8_to_wchar(text, strlen(text), NULL, 0, 0);
	if (len != 0) {
		wtext.resize(len);
		utf8_to_wchar(text, strlen(text), &wtext[0], len, 0);
	}
	ensureChars(&wtext[0], len);

	float x = 0, y = 0;
	wchar32_t prev = 0;
	for (std::size_t i = 0; i < wtext.size(); ++i) {
		std::map<wchar32_t, TextureGlyph>::const_iterator iter =
				fontInfo_.textureGlyphs.find(wtext[i]);

		if (iter == fontInfo_.textureGlyphs.end())
			continue;

		const TextureGlyph &textureGlyph = iter->second;

		int width = textureGlyph.width;
		int height = textureGlyph.height;
		int left = textureGlyph.left;
		int top = textureGlyph.top;

		x += kerning(prev, wtext[i]) >> 6;
		prev = wtext[i];

		float x0 = x + left;
		float y0 = y - top;

		float x1 = x + left + width;
		float y1 = y - top + height;

		minx = std::min(minx, sizescalex_ * x0);
		minx = std::min(minx, sizescalex_ * x1);
		miny = std::min(miny, sizescaley_ * y0);
		miny = std::min(miny, sizescaley_ * y1);
		maxx = std::max(maxx, sizescalex_ * x0);
		maxx = std::max(maxx, sizescalex_ * x1);
		maxy = std::max(maxy, sizescaley_ * y0);
		maxy = std::max(maxy, sizescaley_ * y1);

		x += textureGlyph.advancex >> 6;

		x += (int) (letterSpacing / sizescalex_);
	}

	if (!x)
		minx = miny = maxx = maxy = 0;

	if (pminx)
		*pminx = minx;
	if (pminy)
		*pminy = miny;
	if (pmaxx)
		*pmaxx = maxx;
	if (pmaxy)
		*pmaxy = maxy;
}

float TTBMFont::getAdvanceX(const char *text, float letterSpacing, int size) {
	std::vector<wchar32_t> wtext;
	size_t len = utf8_to_wchar(text, strlen(text), NULL, 0, 0);
	if (len != 0) {
		wtext.resize(len);
		utf8_to_wchar(text, strlen(text), &wtext[0], len, 0);
	}

	if (size < 0 || size > ((int) wtext.size()))
		size = wtext.size();

	wtext.push_back(0);
	ensureChars(&wtext[0], len);

	float x = 0;
	wchar32_t prev = 0;
	for (int i = 0; i < size; ++i) {
		std::map<wchar32_t, TextureGlyph>::const_iterator iter =
				fontInfo_.textureGlyphs.find(wtext[i]);

		if (iter == fontInfo_.textureGlyphs.end())
			continue;

		const TextureGlyph &textureGlyph = iter->second;

		x += kerning(prev, wtext[i]) >> 6;
		prev = wtext[i];

		x += textureGlyph.advancex >> 6;

		x += (int) (letterSpacing / sizescalex_);
	}

	x += kerning(prev, wtext[size]) >> 6;

	return x * sizescalex_;
}

int TTBMFont::kerning(wchar32_t left, wchar32_t right) const {
	std::map<std::pair<wchar32_t, wchar32_t>, int>::const_iterator iter;
	iter = fontInfo_.kernings.find(std::make_pair(left, right));
	return (iter != fontInfo_.kernings.end()) ? iter->second : 0;
}

float TTBMFont::getAscender() {
	return fontInfo_.ascender * sizescaley_;
}

float TTBMFont::getLineHeight() {
	return fontInfo_.height * sizescaley_;
}
