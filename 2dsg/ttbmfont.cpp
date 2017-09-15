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

TTBMFont::TTBMFont(Application *application, const char *filename, float size,
		const char *chars, float filtering, GStatus *status) :
		BMFontBase(application) {
	try {
		constructor(filename, size, chars, filtering);
	} catch (GiderosException &e) {
		if (status)
			*status = e.status();
	}
}

bool TTBMFont::addGlyph(const wchar32_t chr)
{
	FT_UInt glyphIndex = FT_Get_Char_Index(fontFace_, chr);
	if (glyphIndex == 0)	// 0 means `undefined character code'
		return false;

	FT_Error error;
	error = FT_Load_Glyph(fontFace_, glyphIndex, FT_LOAD_DEFAULT);
	if (error)
		return false;

	int top, left, width, height;
	if (fontFace_->glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
		FT_BBox bbox;
		FT_Outline_Get_CBox(&fontFace_->glyph->outline, &bbox);

		bbox.xMin &= ~63;
		bbox.yMin &= ~63;
		bbox.xMax = (bbox.xMax + 63) & ~63;
		bbox.yMax = (bbox.yMax + 63) & ~63;

		width = (bbox.xMax - bbox.xMin) >> 6;
		height = (bbox.yMax - bbox.yMin) >> 6;
		top = bbox.yMax >> 6;
		left = bbox.xMin >> 6;
	} else if (fontFace_->glyph->format == FT_GLYPH_FORMAT_BITMAP) {
		width = fontFace_->glyph->bitmap.width;
		height = fontFace_->glyph->bitmap.rows;
		top = fontFace_->glyph->bitmap_top;
		left = fontFace_->glyph->bitmap_left;
	} else
		return false;

	TextureGlyph textureGlyph;
	textureGlyph.chr = chr;
	textureGlyph.glyphIndex = glyphIndex;
	textureGlyph.top = top;
	textureGlyph.left = left;
	textureGlyph.width = width;
	textureGlyph.height = height;
	textureGlyph.advancex = fontFace_->glyph->advance.x;
	textureGlyph.advancey = fontFace_->glyph->advance.y;

	fontInfo_.textureGlyphs[chr] = textureGlyph;
	return true;
}

void TTBMFont::constructor(const char *filename, float size, const char *chars,
		float filtering) {

	G_FILE *fis = g_fopen(filename, "rb");
	if (!fis)
		throw GiderosException(GStatus(6000, filename));// Error #6000: %s: No such file or directory.

    memset(&stream_, 0, sizeof(stream_));

	g_fseek(fis, 0, SEEK_END);
    stream_.size = g_ftell(fis);
	g_fseek(fis, 0, SEEK_SET);
    stream_.descriptor.pointer = fis;
    stream_.read = read;
    stream_.close = close;

	FT_Open_Args args;
	memset(&args, 0, sizeof(args));
	args.flags = FT_OPEN_STREAM;
    args.stream = &stream_;

	FT_Error error;

	FT_Face face;

	error = FT_Open_Face(FT_Library_Singleton::instance(), &args, 0, &face);
	if (error)
		throw GiderosException(GStatus(6012, filename));// Error #6012: %s: Error while reading font file.

	float scalex = application_->getLogicalScaleX();
	float scaley = application_->getLogicalScaleY();

	float RESOLUTION = 72;
	if (filtering > 1) {
		scalex /= filtering;
		scaley /= filtering;
	}
	error = FT_Set_Char_Size(face, 0L, (int) floor(size * 64 + 0.5f),
			(int) floor(RESOLUTION * scalex + 0.5f),
			(int) floor(RESOLUTION * scaley + 0.5f));

	if (error) {
		FT_Done_Face(face);
		throw GiderosException(GStatus(6017, filename));// Error #6017: Invalid font size.
	}

	std::vector<wchar32_t> wchars;
	size_t len = utf8_to_wchar(chars, strlen(chars), NULL, 0, 0);
	if (len != 0) {
		wchars.resize(len);
		utf8_to_wchar(chars, strlen(chars), &wchars[0], len, 0);
	}

	fontInfo_.ascender = face->size->metrics.ascender >> 6;
	fontInfo_.height = face->size->metrics.height >> 6;

	std::map<wchar32_t, TextureGlyph> &textureGlyphs = fontInfo_.textureGlyphs;

	textureGlyphs.clear();
	fontFace_=face;
    currentDib_=NULL;
    currentPacker_=NULL;

	if (len) {
		for (size_t i = 0; i < len; ++i) {
			wchar32_t chr = wchars[i];
			addGlyph(chr);
		}

		std::map<std::pair<wchar32_t, wchar32_t>, int> &kernings =
				fontInfo_.kernings;

		kernings.clear();

		if (FT_HAS_KERNING(face)) {
			std::map<wchar32_t, TextureGlyph>::iterator iter1, iter2, e =
					textureGlyphs.end();

			for (iter1 = textureGlyphs.begin(); iter1 != e; ++iter1)
				for (iter2 = textureGlyphs.begin(); iter2 != e; ++iter2) {
					const TextureGlyph &g1 = iter1->second;
					const TextureGlyph &g2 = iter2->second;

					FT_Vector delta;
					FT_Get_Kerning(face, g1.glyphIndex, g2.glyphIndex,
							FT_KERNING_DEFAULT, &delta);

					if (delta.x != 0)
						kernings[std::make_pair(g1.chr, g2.chr)] = delta.x;
				}
		}

		TexturePacker *tp = createTexturePacker();

		tp->setTextureCount(textureGlyphs.size());
		std::map<wchar32_t, TextureGlyph>::iterator iter, e =
				textureGlyphs.end();
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

			FT_UInt glyph_index = FT_Get_Char_Index(face, iter->second.chr);
			if (glyph_index == 0)
				continue;

			error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
			if (error)
				continue;

			error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
			if (error)
				continue;

			FT_Bitmap &bitmap = face->glyph->bitmap;

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

		FT_Done_Face(face);
		fontFace_=NULL;

		TextureParameters parameters;
		parameters.filter = (filtering != 0) ? eLinear : eNearest;
		parameters.wrap = eClamp;
		parameters.format = eA8;
		textureData_.push_back(application_->getTextureManager()->createTextureFromDib(dib,
				parameters));
	} else {
		currentDib_ =new Dib(application_, 1024, 1024, true); //use 102x1024 textures for packing
		currentPacker_ = createProgressiveTexturePacker(currentDib_->width(),currentDib_->height());
		TextureParameters parameters;
		parameters.filter = (filtering != 0) ? eLinear : eNearest;
		parameters.wrap = eClamp;
		parameters.format = eA8;
		textureData_.push_back(application_->getTextureManager()->createTextureFromDib(*currentDib_,
				parameters));
	}

	filtering_=filtering;
	sizescalex_ = 1 / scalex;
	sizescaley_ = 1 / scaley;
	uvscalex_ = 1;
	uvscaley_ = 1;
}

void TTBMFont::ensureChars(const wchar32_t *text)
{
	if (!currentPacker_) return;
	std::map<std::pair<wchar32_t, wchar32_t>, int> &kernings =
			fontInfo_.kernings;
	std::map<wchar32_t, TextureGlyph> &textureGlyphs = fontInfo_.textureGlyphs;
	bool updateTexture=false;
	wchar32_t lchar=0;
	for (const wchar32_t *t = text; *t; ++t)
	{
		wchar32_t chr=*t;
		bool newGlyph=false;
		if (textureGlyphs.find(chr)==textureGlyphs.end())
		{
			if (!addGlyph(chr)) continue;
			newGlyph=true;
		}
		if (FT_HAS_KERNING(fontFace_)&&lchar) {
				FT_Vector delta;
				FT_Get_Kerning(fontFace_, textureGlyphs[lchar].glyphIndex, textureGlyphs[chr].glyphIndex,
						FT_KERNING_DEFAULT, &delta);

				if (delta.x != 0)
					kernings[std::make_pair(lchar, chr)] = delta.x;
		}
		if (newGlyph)
		{
			if (!currentPacker_->addTexture(textureGlyphs[chr].width, textureGlyphs[chr].height))
			{
				//Build a new layer
				if (updateTexture)
				{
					application_->getTextureManager()->updateTextureFromDib(textureData_[textureData_.size()-1],*currentDib_);
					updateTexture=false;
				}
				releaseTexturePacker(currentPacker_);
				currentPacker_ = createProgressiveTexturePacker(currentDib_->width(),currentDib_->height());
				unsigned char rgba[] = { 255, 255, 255, 0 };
				currentDib_->fill(rgba);
				TextureParameters parameters;
				parameters.filter = (filtering_ != 0) ? eLinear : eNearest;
				parameters.wrap = eClamp;
				parameters.format = eA8;
				textureData_.push_back(application_->getTextureManager()->createTextureFromDib(*currentDib_,
						parameters));
				currentPacker_->addTexture(textureGlyphs[chr].width, textureGlyphs[chr].height);
			}
			int xo, yo;
			int width, height;
			currentPacker_->getTextureLocation(-1, &xo, &yo, &width, &height);

			FT_UInt glyph_index = FT_Get_Char_Index(fontFace_, chr);
			if (glyph_index == 0)
				continue;
			FT_Error error;

			error = FT_Load_Glyph(fontFace_, glyph_index, FT_LOAD_DEFAULT);
			if (error)
				continue;

			error = FT_Render_Glyph(fontFace_->glyph, FT_RENDER_MODE_NORMAL);
			if (error)
				continue;

			FT_Bitmap &bitmap = fontFace_->glyph->bitmap;

			textureGlyphs[chr].x = xo;
			textureGlyphs[chr].y = yo;
			textureGlyphs[chr].texture = textureData_.size()-1;

			width = std::min(width, (int) bitmap.width);
			height = std::min(height, (int) bitmap.rows);

			for (int y = 0; y < height; ++y)
				for (int x = 0; x < width; ++x) {
					int index = x + y * bitmap.pitch;
					int c = bitmap.buffer[index];

					currentDib_->setAlpha(xo + x, yo + y, c);
				}
			updateTexture=true;
		}
	}
	if (updateTexture)
	{
		application_->getTextureManager()->updateTextureFromDib(textureData_[textureData_.size()-1],*currentDib_);
		updateTexture=false;
	}
}

TTBMFont::~TTBMFont() {
	for (std::vector<TextureData *>::iterator it=textureData_.begin();it!=textureData_.end();it++)
		if (*it)
			application_->getTextureManager()->destroyTexture(*it);
	if (fontFace_)
		FT_Done_Face(fontFace_);
	if (currentDib_)
		delete currentDib_;
	if (currentPacker_)
		releaseTexturePacker(currentPacker_);
}

void TTBMFont::drawText(std::vector<GraphicsBase>* vGraphicsBase,
		const wchar32_t* text, float r, float g, float b, float letterSpacing,
		bool hasSample, float minx, float miny) {
	int size = 0;
	for (const wchar32_t *t = text; *t; ++t, ++size);
	ensureChars(text);

	if (size == 0) {
		vGraphicsBase->clear();
		return;
	}

	std::map<int,int> layerMap;
	std::map<int,int> gfxMap;
	int gfx=0;
	for (int i = 0; i < size; ++i) {
			std::map<wchar32_t, TextureGlyph>::const_iterator iter =
					fontInfo_.textureGlyphs.find(text[i]);

			if (iter == fontInfo_.textureGlyphs.end())
				continue;
			const TextureGlyph &textureGlyph = iter->second;
			int l=layerMap[textureGlyph.texture];
			if (!l) {
				gfx++;
				layerMap[textureGlyph.texture]=gfx;
				l=gfx;
			}
			l--;
			gfxMap[l]=gfxMap[l]+1;
	}

	vGraphicsBase->resize(gfx);

	for (std::map<int,int>::iterator it=gfxMap.begin();it!=gfxMap.end();it++)
	{
	GraphicsBase *graphicsBase = &((*vGraphicsBase)[it->first]);
	int size=it->second;
	graphicsBase->setColor(r, g, b, 1);
	graphicsBase->vertices.resize(size * 4);
	graphicsBase->texcoords.resize(size * 4);
	graphicsBase->indices.resize(size * 6);
	graphicsBase->vertices.Update();
	graphicsBase->texcoords.Update();
	graphicsBase->indices.Update();
	it->second=0;
	}

	float x = -minx / sizescalex_, y = -miny / sizescaley_;

	if (hasSample) {
		std::map<wchar32_t, TextureGlyph>::const_iterator iter =
				fontInfo_.textureGlyphs.find(text[0]);
		const TextureGlyph &textureGlyph = iter->second;
		x = -textureGlyph.left;
		//y *= application_->getLogicalScaleY();
	}

	wchar32_t prev = 0;

	for (int i = 0; i < size; ++i) {
		std::map<wchar32_t, TextureGlyph>::const_iterator iter =
				fontInfo_.textureGlyphs.find(text[i]);

		if (iter == fontInfo_.textureGlyphs.end())
			continue;

		const TextureGlyph &textureGlyph = iter->second;
		int gfx=layerMap[textureGlyph.texture]-1;
		GraphicsBase *graphicsBase = &((*vGraphicsBase)[gfx]);
		graphicsBase->data = textureData_[textureGlyph.texture];

		int width = textureGlyph.width;
		int height = textureGlyph.height;
		int left = textureGlyph.left;
		int top = textureGlyph.top;

		x += kerning(prev, text[i]) >> 6;
		prev = text[i];

		float x0 = x + left;
		float y0 = y - top;

		float x1 = x0 + width;
		float y1 = y0 + height;
		int vi=gfxMap[gfx];
		gfxMap[gfx]=vi+1;

		graphicsBase->vertices[vi * 4 + 0] = Point2f(sizescalex_ * x0,
				sizescaley_ * y0);
		graphicsBase->vertices[vi * 4 + 1] = Point2f(sizescalex_ * x1,
				sizescaley_ * y0);
		graphicsBase->vertices[vi * 4 + 2] = Point2f(sizescalex_ * x1,
				sizescaley_ * y1);
		graphicsBase->vertices[vi * 4 + 3] = Point2f(sizescalex_ * x0,
				sizescaley_ * y1);

		float u0 = (float) textureGlyph.x / (float) textureData_[textureGlyph.texture]->exwidth;
		float v0 = (float) textureGlyph.y / (float) textureData_[textureGlyph.texture]->exheight;
		float u1 = (float) (textureGlyph.x + width) / (float) textureData_[textureGlyph.texture]->exwidth;
		float v1 = (float) (textureGlyph.y + height) / (float) textureData_[textureGlyph.texture]->exheight;

		u0 *= uvscalex_;
		v0 *= uvscaley_;
		u1 *= uvscalex_;
		v1 *= uvscaley_;

		graphicsBase->texcoords[vi * 4 + 0] = Point2f(u0, v0);
		graphicsBase->texcoords[vi * 4 + 1] = Point2f(u1, v0);
		graphicsBase->texcoords[vi * 4 + 2] = Point2f(u1, v1);
		graphicsBase->texcoords[vi * 4 + 3] = Point2f(u0, v1);

		graphicsBase->indices[vi * 6 + 0] = vi * 4 + 0;
		graphicsBase->indices[vi * 6 + 1] = vi * 4 + 1;
		graphicsBase->indices[vi * 6 + 2] = vi * 4 + 2;
		graphicsBase->indices[vi * 6 + 3] = vi * 4 + 0;
		graphicsBase->indices[vi * 6 + 4] = vi * 4 + 2;
		graphicsBase->indices[vi * 6 + 5] = vi * 4 + 3;

		x += (textureGlyph.advancex) >> 6;

		x += letterSpacing / sizescalex_;
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

	if (size < 0 || size > wtext.size())
		size = wtext.size();

	wtext.push_back(0);

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
