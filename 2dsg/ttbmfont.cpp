#include <ttbmfont.h>

#include <ft2build.h>
#include FT_OUTLINE_H
#include FT_STROKER_H

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
		float size, const char *chars, float filtering, float outline, GStatus *status) :
		BMFontBase(application) {
	try {
		constructor(filenames, size, chars, filtering, outline);
	} catch (GiderosException &e) {
		if (status)
			*status = e.status();
	}
}

bool TTBMFont::addGlyph(const wchar32_t chr) {
	FT_Face face;
	FT_UInt glyphIndex = 0;
	int facenum=0;
	for (std::vector<FontFace>::iterator it = fontFaces_.begin();
			it != fontFaces_.end(); it++,facenum++) {
		face = (*it).face;
		glyphIndex = FT_Get_Char_Index(face, chr);
		if (glyphIndex != 0)	// 0 means `undefined character code'
			break;
	}
	if (glyphIndex == 0)	// 0 means `undefined character code'
		return false;
	fontInfo_.charGlyphs[chr] = glyphIndex;
	fontInfo_.charFace[chr] = facenum;
	return addFontGlyph(facenum,glyphIndex,chr);
}

bool TTBMFont::addFontGlyph(int fontnum,FT_UInt glyphIndex,wchar32_t chr) {
	FT_Error error;
	FT_Face face=fontFaces_[fontnum].face;
	error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
	if (error)
		return false;

	int top, left, width, height;
	if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
		FT_BBox bbox;
		if (stroker) {
			FT_Glyph glyph;
			error = FT_Get_Glyph(face->glyph, &glyph);
			if (error)
				return false;
			error = FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
			if (error)
				return false;
			FT_OutlineGlyph oGlyph = reinterpret_cast<FT_OutlineGlyph>(glyph);
			FT_Outline_Get_CBox(&oGlyph->outline, &bbox);
			FT_Done_Glyph(glyph);
		}
		else
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
	textureGlyph.top = top;
	textureGlyph.left = left;
	textureGlyph.width = width;
	textureGlyph.height = height;
	textureGlyph.advancex = face->glyph->advance.x;
	textureGlyph.advancey = face->glyph->advance.y;
	textureGlyph.chr = chr;

	fontFaces_[fontnum].textureGlyphs[glyphIndex] = textureGlyph;
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

	for (size_t i = 0; i < len; ++i) {
		wchar32_t chr = wchars[i];
		addGlyph(chr);
	}

	std::map<std::pair<wchar32_t, wchar32_t>, int> &kernings =
			fontInfo_.kernings;

	kernings.clear();

	int glyphCount=0;
    for (size_t facenum=0;facenum<fontFaces_.size();facenum++)
	{
		std::map<FT_UInt, TextureGlyph> &textureGlyphs = fontFaces_[facenum].textureGlyphs;
		FT_Face face=fontFaces_[facenum].face;
		std::map<FT_UInt, TextureGlyph>::iterator iter1, iter2, e =
				textureGlyphs.end();

		for (iter1 = textureGlyphs.begin(); iter1 != e; ++iter1)
		{
			if (FT_HAS_KERNING(face))
			for (iter2 = textureGlyphs.begin(); iter2 != e; ++iter2) {
				const TextureGlyph &g1 = iter1->second;
				const TextureGlyph &g2 = iter2->second;
				if (g1.chr && g2.chr) {
					FT_Vector delta;
                    FT_Get_Kerning(face, iter1->first, iter2->first,
							FT_KERNING_DEFAULT, &delta);
					if (delta.x != 0)
						kernings[std::make_pair(g1.chr, g2.chr)] = delta.x;
				}
			}
			glyphCount++;
		}
	}

	TexturePacker *tp = createTexturePacker();

	tp->setTextureCount(glyphCount);
    for (size_t facenum=0;facenum<fontFaces_.size();facenum++)
	{
		std::map<FT_UInt, TextureGlyph> &textureGlyphs = fontFaces_[facenum].textureGlyphs;
        std::map<FT_UInt, TextureGlyph>::iterator iter, e = textureGlyphs.end();
		for (iter = textureGlyphs.begin(); iter != e; ++iter)
			tp->addTexture(iter->second.width, iter->second.height);
	}
	int width = 0, height = 0;
	tp->packTextures(&width, &height, 2, false);

	width = std::max(width, 1);
	height = std::max(height, 1);

	Dib dib(application_, width, height, true);
	unsigned char rgba[] = { 255, 255, 255, 0 };
	dib.fill(rgba);

	int i = 0;
    for (size_t facenum=0;facenum<fontFaces_.size();facenum++)
	{
		std::map<FT_UInt, TextureGlyph> &textureGlyphs = fontFaces_[facenum].textureGlyphs;
        std::map<FT_UInt, TextureGlyph>::iterator iter, e = textureGlyphs.end();
		FT_Face face=fontFaces_[facenum].face;
		for (iter = textureGlyphs.begin(); iter != e; ++iter, ++i) {
			int xo, yo;
			int width, height;
			tp->getTextureLocation(i, &xo, &yo, &width, &height);

			error = FT_Load_Glyph(face, iter->first, FT_LOAD_DEFAULT);
			if (error)
				continue;

			FT_Bitmap *bitmap=NULL;
			FT_Glyph glyph=nullptr;
			if (stroker) {
				error = FT_Get_Glyph(face->glyph, &glyph);
				if (error)
					continue;
				error = FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
				if (error)
					continue;
				error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
				if (error)
					continue;
				FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
				bitmap=&(bitmapGlyph->bitmap);
			}
			else {
				error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
				if (error)
					continue;
				 bitmap = &(face->glyph->bitmap);
			}

			iter->second.x = xo;
			iter->second.y = yo;
			iter->second.texture = 0;

			width = std::min(width, (int) bitmap->width);
			height = std::min(height, (int) bitmap->rows);

			for (int y = 0; y < height; ++y)
				for (int x = 0; x < width; ++x) {
					int index = x + y * bitmap->pitch;
					int c = bitmap->buffer[index];

					dib.setAlpha(xo + x, yo + y, c);
				}
			if (glyph) FT_Done_Glyph(glyph);
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
		const char *chars, float filtering, float outline) {

    currentDib_ = NULL;
    currentPacker_ = NULL;

	fontInfo_.ascender = 0;
	fontInfo_.descender = 1000000;
	fontInfo_.height = 0;
	defaultSize_ = size;
	outlineSize_ = outline;
	stroker=NULL;
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

	if (outline>0)
	{
		FT_Stroker_New(FT_Library_Singleton::instance(), &stroker);
		FT_Stroker_Set(stroker, (FT_Fixed)(outline * 64 * (scalex+scaley)/2), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
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
		fontInfo_.descender = std::min(fontInfo_.descender,
				(int) (ff.face->size->metrics.descender >> 6));
		fontInfo_.height = std::max(fontInfo_.height,
				(int) ((ff.face->size->metrics.height
						- ff.face->size->metrics.ascender) >> 6));
		ff.textureGlyphs.clear();
	}

	fontInfo_.height = fontInfo_.ascender + fontInfo_.height;

	charset_ = chars;

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
	bool updateTexture = false;
	wchar32_t lchar = 0;
	for (const wchar32_t *t = text; size; size--, t++) {
		wchar32_t chr = *t;
		bool newGlyph = false;
		std::map<wchar32_t, FT_UInt> &charGlyphs = fontInfo_.charGlyphs;
		if (charGlyphs.find(chr)==charGlyphs.end()){
			if (!addGlyph(chr))
				continue;
			newGlyph = true;
		}
		int facenum=fontInfo_.charFace[chr];
		FT_Face face=fontFaces_[facenum].face;
		FT_UInt glyph=charGlyphs[chr];
		const TextureGlyph &g = fontFaces_[facenum].textureGlyphs[glyph];
		std::map<wchar32_t, int>::iterator lit=fontInfo_.charFace.find(lchar);
		if (FT_HAS_KERNING(face)&&lchar&&(lit!=fontInfo_.charFace.end())&&(lit->second==facenum)) {
			FT_UInt lglyph=charGlyphs[lchar];
			FT_Vector delta;
			FT_Get_Kerning(face, lglyph,
					glyph, FT_KERNING_DEFAULT,
					&delta);

			if (delta.x != 0)
				kernings[std::make_pair(lchar, chr)] = delta.x;
		}
		lchar=chr;
		if (newGlyph) {
			FT_UInt glyphIndex=glyph;
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

			FT_Error error;
			error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
			if (error)
				continue;

			FT_Bitmap *bitmap=NULL;
			FT_Glyph glyph=nullptr;
			if (stroker) {
				error = FT_Get_Glyph(face->glyph, &glyph);
				if (error)
					continue;
				error = FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
				if (error)
					continue;
				error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
				if (error)
					continue;
				FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
				bitmap=&(bitmapGlyph->bitmap);
			}
			else {
				error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
				if (error)
					continue;
				 bitmap = &(face->glyph->bitmap);
			}

			fontFaces_[facenum].textureGlyphs[glyphIndex].x=xo;
			fontFaces_[facenum].textureGlyphs[glyphIndex].y=yo;
            fontFaces_[facenum].textureGlyphs[glyphIndex].texture=textureData_.size() - 1;

			width = std::min(width, (int) bitmap->width);
			height = std::min(height, (int) bitmap->rows);

			for (int y = 0; y < height; ++y)
				for (int x = 0; x < width; ++x) {
					int index = x + y * bitmap->pitch;
					int c = bitmap->buffer[index];

					currentDib_->setAlpha(xo + x, yo + y, c);
				}
			if (glyph) FT_Done_Glyph(glyph);
			updateTexture = true;
		}
	}
	if (updateTexture) {
		application_->getTextureManager()->updateTextureFromDib(
				textureData_[textureData_.size() - 1], *currentDib_);
		updateTexture = false;
	}
}

void TTBMFont::ensureGlyphs(int facenum,const wchar32_t *text, int size) {
	checkLogicalScale();
	if (!currentPacker_)
		return;
	bool updateTexture = false;
	FT_Face face=fontFaces_[facenum].face;
	std::map<FT_UInt, TextureGlyph> &textureGlyphs = fontFaces_[facenum].textureGlyphs;
	for (const wchar32_t *t = text; size; size--, t++) {
		FT_UInt glyph = (FT_UInt)*t;
		bool newGlyph = false;
		if (textureGlyphs.find(glyph)==textureGlyphs.end()){
			if (!addFontGlyph(facenum,glyph,0))
				continue;
			newGlyph = true;
		}
		const TextureGlyph &g = fontFaces_[facenum].textureGlyphs[glyph];
		if (newGlyph) {
			FT_UInt glyphIndex=glyph;
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

			FT_Error error;
			error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
			if (error)
				continue;

			FT_Bitmap *bitmap=NULL;
			FT_Glyph glyph=nullptr;
			if (stroker) {
				error = FT_Get_Glyph(face->glyph, &glyph);
				if (error)
					continue;
				error = FT_Glyph_StrokeBorder(&glyph, stroker, false, true);
				if (error)
					continue;
				error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, nullptr, true);
				if (error)
					continue;
				FT_BitmapGlyph bitmapGlyph = reinterpret_cast<FT_BitmapGlyph>(glyph);
				bitmap=&(bitmapGlyph->bitmap);
			}
			else {
				error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
				if (error)
					continue;
				 bitmap = &(face->glyph->bitmap);
			}

			fontFaces_[facenum].textureGlyphs[glyphIndex].x=xo;
			fontFaces_[facenum].textureGlyphs[glyphIndex].y=yo;
            fontFaces_[facenum].textureGlyphs[glyphIndex].texture=textureData_.size() - 1;

			width = std::min(width, (int) bitmap->width);
			height = std::min(height, (int) bitmap->rows);

			for (int y = 0; y < height; ++y)
				for (int x = 0; x < width; ++x) {
					int index = x + y * bitmap->pitch;
					int c = bitmap->buffer[index];

					currentDib_->setAlpha(xo + x, yo + y, c);
				}
			if (glyph) FT_Done_Glyph(glyph);
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
			it != fontFaces_.end(); it++) {
        (*it).textureGlyphs.clear();
		FT_Done_Face((*it).face);
	}
	fontFaces_.clear();
	if (currentDib_)
		delete currentDib_;
	if (currentPacker_)
		releaseTexturePacker(currentPacker_);
	fontInfo_.kernings.clear();
	if (stroker)
		FT_Stroker_Done(stroker);
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
	    if (stroker)
			FT_Stroker_Set(stroker, (FT_Fixed)(outlineSize_ * 64 * (scalex+scaley)/2), FT_STROKER_LINECAP_ROUND, FT_STROKER_LINEJOIN_ROUND, 0);
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
		fontInfo_.descender = 1000000;
		fontInfo_.height = 0;

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
			fontInfo_.descender = std::min(fontInfo_.descender,
					(int) ((*it).face->size->metrics.descender >> 6));
			fontInfo_.height = std::max(fontInfo_.descender,
					(int) (((*it).face->size->metrics.height
							- (*it).face->size->metrics.ascender) >> 6));
			it->textureGlyphs.clear();
		}
		fontInfo_.kernings.clear();

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
		fontInfo_.height = fontInfo_.ascender + fontInfo_.height;
		cacheVersion_++;
        if (shaper_)
            delete shaper_;
        shaper_=NULL;
	}
}

bool TTBMFont::shapeChunk(struct ChunkLayout &part,std::vector<wchar32_t> &wtext)
{
	if (part.styleFlags&TEXTSTYLEFLAG_SKIPSHAPING)
		return false;
    FontshaperBuilder_t builder=(FontshaperBuilder_t) g_getGlobalHook(GID_GLOBALHOOK_FONTSHAPER);
    if (!builder)
        return false;
    if (fontFaces_.size()!=1) //Multi font not supported (yet ?)
        return false;
    if (!shaper_) {
        float scalex = currentLogicalScaleX_;
        float scaley = currentLogicalScaleY_;
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
    for (size_t k=0;k<part.shaped.size();k++)
        part.shaped[k]._private=(void *) 0;
    return true;
}

TTBMFont::TextureGlyph *TTBMFont::getCharGlyph(wchar32_t chr,int &facenum,FT_UInt &glyph) {
	std::map<wchar32_t, FT_UInt> &charGlyphs = fontInfo_.charGlyphs;
	if (charGlyphs.find(chr)==charGlyphs.end()) return NULL;
	facenum=fontInfo_.charFace[chr];
	glyph=charGlyphs[chr];
	return &fontFaces_[facenum].textureGlyphs[glyph];
}

void TTBMFont::chunkMetrics(struct ChunkLayout &part, float letterSpacing)
{
	std::vector<wchar32_t> wtext;
    size_t len = utf8_to_wchar(part.text.c_str(), part.text.size(), NULL, 0, 0);
	if (len != 0) {
		wtext.resize(len);
        utf8_to_wchar(part.text.c_str(), part.text.size(), &wtext[0], len, 0);
	}
	wtext.push_back(0);

	checkLogicalScale();

    float minx = 1e30;
    float miny = 1e30;
    float maxx = -1e30;
    float maxy = -1e30;

    float x = 0, y = 0;
	part.shaped.clear();
    if (shapeChunk(part,wtext)) {
        //Shaping has been done externally, iterate over glyphs instead
        len=part.shaped.size();
    	std::vector<wchar32_t> wtext1;
   		wtext1.resize(len);
        for (size_t i = 0; i < len; ++i)
        	wtext1[i]=part.shaped[i].glyph;
        ensureGlyphs(0,&wtext1[0], len);

        for (size_t i = 0; i < len; ++i) {
            GlyphLayout &gl=part.shaped[i];
            FT_UInt glyphIndex=(FT_UInt) gl.glyph;
            int facenum=(int) gl._private;
            std::map<FT_UInt, TextureGlyph>::const_iterator iter =
                    fontFaces_[facenum].textureGlyphs.find(glyphIndex);

    		if (iter == fontFaces_[facenum].textureGlyphs.end())
    			continue;

    		const TextureGlyph &textureGlyph = iter->second;

    		int width = textureGlyph.width;
    		int height = textureGlyph.height;
    		int left = textureGlyph.left;
    		int top = textureGlyph.top;

            gl.offX+=left;
            gl.offY-=top;

    		float x0 = x + gl.offX;
    		float y0 = y + gl.offY;

    		float x1 = x0 + width;
            float y1 = y0 + height;

    		minx = std::min(minx, sizescalex_ * x0);
    		minx = std::min(minx, sizescalex_ * x1);
    		miny = std::min(miny, sizescaley_ * y0);
    		miny = std::min(miny, sizescaley_ * y1);
    		maxx = std::max(maxx, sizescalex_ * x0);
    		maxx = std::max(maxx, sizescalex_ * x1);
    		maxy = std::max(maxy, sizescaley_ * y0);
    		maxy = std::max(maxy, sizescaley_ * y1);

            x += gl.advX;
        }
    }
    else {
    	ensureChars(&wtext[0], len);

    	wchar32_t prev = 0;
        struct GlyphLayout shape;
    	for (std::size_t i = 0; i < wtext.size(); ++i) {
    		int facenum;
    		FT_UInt glyph;
    		TextureGlyph *textureGlyph = getCharGlyph(wtext[i],facenum,glyph);
    		if (!textureGlyph) continue;

    		int width = textureGlyph->width;
    		int height = textureGlyph->height;
    		int left = textureGlyph->left;
    		int top = textureGlyph->top;

    		int kx=kerning(prev, wtext[i]) >> 6;
    		x += kx;
    		prev = wtext[i];

    		float x0 = x + left;
    		float y0 = y - top;

    		float x1 = x0 + width;
    		float y1 = y0 + height;

    		minx = std::min(minx, sizescalex_ * x0);
    		minx = std::min(minx, sizescalex_ * x1);
    		miny = std::min(miny, sizescaley_ * y0);
    		miny = std::min(miny, sizescaley_ * y1);
    		maxx = std::max(maxx, sizescalex_ * x0);
    		maxx = std::max(maxx, sizescalex_ * x1);
    		maxy = std::max(maxy, sizescaley_ * y0);
    		maxy = std::max(maxy, sizescaley_ * y1);

            x += textureGlyph->advancex >> 6;
    		x += (int) (letterSpacing / sizescalex_);

            shape.srcIndex=i;
            shape.glyph=glyph;
            shape.advX=(textureGlyph->advancex >> 6)+kx+(letterSpacing/sizescalex_);
            shape.advY=0;
            shape.offX=left;
            shape.offY=-top;
            shape._private=(void *) facenum;
            part.shaped.push_back(shape);
    	}
    }

    if (!x) minx=miny=maxx=maxy=0;

	part.x = minx;
    part.y = miny + part.dy;
    part.w = (maxx-minx)+1;
    part.h = (maxy-miny)+1;
	part.advX=x*sizescalex_;
	part.advY=y*sizescaley_;
}

void TTBMFont::drawText(std::vector<GraphicsBase>* vGraphicsBase,
		const char* text, float r, float g, float b, float a,
        TextLayoutParameters *layout, bool /*hasSample*/, float minx, float miny,TextLayout &l) {

	if (strlen(text) == 0) {
		return;
	}

    if (!(l.styleFlags&TEXTSTYLEFLAG_SKIPLAYOUT))
        l = layoutText(text, layout);

	std::map<int, int> layerMap;
	std::map<int, int> gfxMap;
	std::map<int, int> gfxMap2;
    int gfx = vGraphicsBase->size();

	for (size_t pn = 0; pn < l.parts.size(); pn++) {
		ChunkLayout c = l.parts[pn];
        size_t wsize=c.shaped.size();

		unsigned char rgba[4];
		if (l.styleFlags&TEXTSTYLEFLAG_COLOR)
		{
			float ca=(c.styleFlags&TEXTSTYLEFLAG_COLOR)?(1.0/255)*((c.color>>24)&0xFF):a;
			rgba[0]=(unsigned char)(ca*((c.styleFlags&TEXTSTYLEFLAG_COLOR)?(c.color>>16)&0xFF:r*255));
			rgba[1]=(unsigned char)(ca*((c.styleFlags&TEXTSTYLEFLAG_COLOR)?(c.color>>8)&0xFF:g*255));
			rgba[2]=(unsigned char)(ca*((c.styleFlags&TEXTSTYLEFLAG_COLOR)?(c.color>>0)&0xFF:b*255));
			rgba[3]=(unsigned char)(ca*255);
		}

		for (size_t i = 0; i < wsize; ++i) {
            GlyphLayout &gl=c.shaped[i];
            int facenum = (int)gl._private;
        	std::map<FT_UInt, TextureGlyph> &textureGlyphs = fontFaces_[facenum].textureGlyphs;

			std::map<FT_UInt, TextureGlyph>::const_iterator iter =
					textureGlyphs.find(gl.glyph);

			if (iter == textureGlyphs.end())
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
			{
				graphicsBase->colors.clear();
				graphicsBase->setColor(r, g, b, a);
			}
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

		for (size_t i = 0; i < wsize; ++i) {
            GlyphLayout &gl=c.shaped[i];
            int facenum = (int)gl._private;
        	std::map<FT_UInt, TextureGlyph> &textureGlyphs = fontFaces_[facenum].textureGlyphs;

			std::map<FT_UInt, TextureGlyph>::const_iterator iter =
					textureGlyphs.find(gl.glyph);

			if (iter == textureGlyphs.end())
				continue;
			const TextureGlyph &textureGlyph = iter->second;
			int gfx = layerMap[textureGlyph.texture] - 1;
			GraphicsBase *graphicsBase = &((*vGraphicsBase)[gfx]);
			graphicsBase->data = textureData_[textureGlyph.texture];

			int width = textureGlyph.width;
			int height = textureGlyph.height;

    		float x0 = x + gl.offX - 1;
    		float y0 = y + gl.offY - 1;

    		float x1 = x0 + width + 2;
            float y1 = y0 + height + 2;

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

			x += gl.advX;
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
        int facenum;
        FT_UInt glyph;
        TextureGlyph *textureGlyph = getCharGlyph(wtext[i],facenum,glyph);
        if (!textureGlyph) continue;

        int width = textureGlyph->width;
        int height = textureGlyph->height;
        int left = textureGlyph->left;
        int top = textureGlyph->top;

		x += kerning(prev, wtext[i]) >> 6;
		prev = wtext[i];

		float x0 = x + left;
		float y0 = y - top;

        float x1 = x0 + width;
        float y1 = y0 + height;

		minx = std::min(minx, sizescalex_ * x0);
		minx = std::min(minx, sizescalex_ * x1);
		miny = std::min(miny, sizescaley_ * y0);
		miny = std::min(miny, sizescaley_ * y1);
		maxx = std::max(maxx, sizescalex_ * x0);
		maxx = std::max(maxx, sizescalex_ * x1);
		maxy = std::max(maxy, sizescaley_ * y0);
		maxy = std::max(maxy, sizescaley_ * y1);

        x += textureGlyph->advancex >> 6;

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
        int facenum;
        FT_UInt glyph;
        TextureGlyph *textureGlyph = getCharGlyph(wtext[i],facenum,glyph);
        if (!textureGlyph) continue;

		x += kerning(prev, wtext[i]) >> 6;
		prev = wtext[i];

        x += textureGlyph->advancex >> 6;

		x += (letterSpacing / sizescalex_);
	}

	x += kerning(prev, wtext[size]) >> 6;

	return x * sizescalex_;
}

float TTBMFont::getCharIndexAtOffset(const char *text, float offset, float letterSpacing, int size) {
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
	offset*=sizescalex_;

	float x = 0;
	float px = 0;
	wchar32_t prev = 0;
	for (int i = 0; i < size; ++i) {
        int facenum;
        FT_UInt glyph;
        TextureGlyph *textureGlyph = getCharGlyph(wtext[i],facenum,glyph);
        if (!textureGlyph) continue;

		x += kerning(prev, wtext[i]) >> 6;
		prev = wtext[i];

        x += textureGlyph->advancex >> 6;

		x += (int) (letterSpacing / sizescalex_);
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

int TTBMFont::kerning(wchar32_t left, wchar32_t right) const {
	std::map<std::pair<wchar32_t, wchar32_t>, int>::const_iterator iter;
	iter = fontInfo_.kernings.find(std::make_pair(left, right));
	return (iter != fontInfo_.kernings.end()) ? iter->second : 0;
}

float TTBMFont::getAscender() {
	return fontInfo_.ascender * sizescaley_;
}

float TTBMFont::getDescender() {
	return -fontInfo_.descender * sizescaley_;
}

float TTBMFont::getLineHeight() {
	return fontInfo_.height * sizescaley_;
}
