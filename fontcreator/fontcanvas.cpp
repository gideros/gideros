#include <ft2build.h>
#include FT_OUTLINE_H

#include "fontcanvas.h"
#include <QPainter>
#include <QImage>
#include <cmath>
#include <QFileInfo>
#include <QDir>
#include "texturepacker.h"
#include <QDebug>

FontCanvas::FontCanvas(QWidget *parent)
	: QWidget(parent)
{
	fontSize_ = 0;
}

FontCanvas::~FontCanvas()
{

}

void FontCanvas::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setBackground(QBrush(Qt::white));
    painter.eraseRect(0, 0, width(), height());

    if (fontFile_.isEmpty())
        return;

    FT_Error error;

    FT_Library library;
    FT_Init_FreeType(&library);

    FT_Face face;
    error = FT_New_Face(library, qPrintable(fontFile_), 0, &face);
    if (error)
    {
        FT_Done_FreeType(library);
        return;
    }

    familyName_ = face->family_name;

    const int RESOLUTION = 72;

    error = FT_Set_Char_Size(face, 0L, fontSize_ * 64, RESOLUTION, RESOLUTION);
    if (error)
    {
        FT_Done_Face(face);
        FT_Done_FreeType(library);
        return;
    }

    ascender_ = face->size->metrics.ascender >> 6;
    height_ = face->size->metrics.height >> 6;

    textureGlyphs_.clear();
    kernings_.clear();

    for (int i = 0; i < chars_.size(); ++i)
    {
        QChar chr = chars_[i];

        FT_UInt glyphIndex = FT_Get_Char_Index(face, chr.unicode());
        if (glyphIndex == 0)	// 0 means `undefined character code'
            continue;

        error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
        if (error)
            continue;

        int top, left, width, height;
        if (face->glyph->format == FT_GLYPH_FORMAT_OUTLINE)
        {
            FT_BBox bbox;
            FT_Outline_Get_CBox(&face->glyph->outline, &bbox);

            bbox.xMin &= ~63;
            bbox.yMin &= ~63;
            bbox.xMax  = (bbox.xMax + 63) & ~63;
            bbox.yMax  = (bbox.yMax + 63) & ~63;

            width  = (bbox.xMax - bbox.xMin) >> 6;
            height = (bbox.yMax - bbox.yMin) >> 6;
            top = bbox.yMax >> 6;
            left = bbox.xMin >> 6;
        }
        else if (face->glyph->format == FT_GLYPH_FORMAT_BITMAP)
        {
            width = face->glyph->bitmap.width;
            height = face->glyph->bitmap.rows;
            top = face->glyph->bitmap_top;
            left = face->glyph->bitmap_left;
        }
        else
            continue;

        TextureGlyph textureGlyph;
        textureGlyph.chr = chr;
        textureGlyph.glyphIndex = glyphIndex;
        textureGlyph.top = top;
        textureGlyph.left = left;
        textureGlyph.width = width;
        textureGlyph.height = height;
        textureGlyph.advancex = face->glyph->advance.x;
        textureGlyph.advancey = face->glyph->advance.y;

        textureGlyphs_[chr] = textureGlyph;
    }

    // kerning
    {
        std::map<QChar, TextureGlyph>::iterator iter1, iter2, e = textureGlyphs_.end();

        for (iter1 = textureGlyphs_.begin(); iter1 != e; ++iter1)
            for (iter2 = textureGlyphs_.begin(); iter2 != e; ++iter2)
            {
                const TextureGlyph &g1 = iter1->second;
                const TextureGlyph &g2 = iter2->second;

                FT_Vector delta;
                FT_Get_Kerning(face,
                               g1.glyphIndex, g2.glyphIndex,
                               FT_KERNING_DEFAULT, &delta);

                if (delta.x != 0)
                    kernings_[std::make_pair(g1.chr, g2.chr)] = delta.x;
            }
    }

    TexturePacker *tp = createTexturePacker();

    tp->setTextureCount(textureGlyphs_.size());
    std::map<QChar, TextureGlyph>::iterator iter, e = textureGlyphs_.end();
    for (iter = textureGlyphs_.begin(); iter != e; ++iter)
        tp->addTexture(iter->second.width, iter->second.height);

    int width = 0, height = 0;
    tp->packTextures(&width, &height, 2, false);

    width = std::max(width, 1);
    height = std::max(height, 1);

    texture_ = QImage(width, height, QImage::Format_RGB32);
    QPainter tpainter(&texture_);
    tpainter.setBackground(QBrush(Qt::white));
    tpainter.eraseRect(0, 0, width, height);

    int i = 0;
    for (iter = textureGlyphs_.begin(); iter != e; ++iter, ++i)
    {
        int xo, yo;
        int width, height;
        tp->getTextureLocation(i, &xo, &yo, &width, &height);

        FT_UInt glyph_index = FT_Get_Char_Index(face, iter->second.chr.unicode());
        if (glyph_index == 0)
            continue;

        error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
        if (error)
            continue;

        error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
        if (error)
            continue;

        FT_Bitmap bitmap = face->glyph->bitmap;

        iter->second.x = xo;
        iter->second.y = yo;

        width = std::min(width, (int)bitmap.width);
        height = std::min(height, (int)bitmap.rows);

        QImage image(width, height, QImage::Format_RGB32);

        for (int y = 0; y < height; ++y)
            for (int x = 0; x < width; ++x)
            {
                int index = x + y * bitmap.pitch;
                int c = 255 - bitmap.buffer[index];

                image.setPixel(x, y, qRgb(c, c, c));
            }

        painter.drawImage(xo, yo, image);
        tpainter.drawImage(xo, yo, image);
    }

    releaseTexturePacker(tp);

    FT_Done_Face(face);
    FT_Done_FreeType(library);
}


#if 0
void FontCanvas::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	painter.setBackground(QBrush(Qt::white));
	painter.eraseRect(0, 0, width(), height());
	
	if (fontFile_.isEmpty())
		return;

	if (chars_.isEmpty())
		return;

	FT_Error error;

	FT_Library library;
	error = FT_Init_FreeType(&library);
	if (error)
	{
		printf("Error: FT_Init_FreeType(...)\n");
		return;
	}

	FT_Face face;
	error = FT_New_Face(library, qPrintable(fontFile_), 0, &face); 
	if (error)
	{
		printf("Error: FT_New_Face(...)\n");
		return;
	}

    familyName_ = face->family_name;

	const int RESOLUTION = 72;

	error = FT_Set_Char_Size(face, 0L, fontSize_ * 64, RESOLUTION, RESOLUTION);
	if (error)
	{
		printf("Error: FT_Set_Char_Size(...)\n");
		return;
	}

	ascender_ = face->size->metrics.ascender >> 6;
	height_ = face->size->metrics.height >> 6;

	textureGlyphs_.clear();

	int maxwidth = 0, maxheight = 0;

	int dx = 1;
	int dy = 1;

	// fill textureGlyphs_ array
	for (int i = 0; i < chars_.size(); ++i)
	{
		QChar chr = chars_[i];
		FT_UInt glyph_index = FT_Get_Char_Index(face, chr.unicode());
		if (glyph_index == 0)	// 0 means `undefined character code'
			continue;
		error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT); 
		error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		FT_Bitmap bitmap = face->glyph->bitmap;

		TextureGlyph textureGlyph;
		textureGlyph.chr = chr;
		//textureGlyph.x = ;
		//textureGlyph.y = ;
		textureGlyph.left = face->glyph->bitmap_left;
		textureGlyph.top = face->glyph->bitmap_top;
		textureGlyph.width = bitmap.width;
		textureGlyph.height = bitmap.rows;
		textureGlyph.advancex = face->glyph->advance.x;
		textureGlyph.advancey = face->glyph->advance.y;

		textureGlyphs_[chr] = textureGlyph;

		maxwidth = std::max(maxwidth, textureGlyph.width + 2 * dx);
		maxheight = std::max(maxheight, textureGlyph.height + 2 * dy);
	}

	if (textureGlyphs_.empty())
		return;

	int ncolumns = std::sqrt((double)textureGlyphs_.size());

	// find glyph positions
	int pos = 0;
	std::map<QChar, TextureGlyph>::iterator iter, e = textureGlyphs_.end();
	for (iter = textureGlyphs_.begin(); iter != e; ++iter, ++pos)
	{
		int gridx = pos % ncolumns;
		int gridy = pos / ncolumns;

		iter->second.x = gridx * maxwidth + dx;
		iter->second.y = gridy * maxheight + dy;
	}

	// find texture size
	int texwidth = 0, texheight = 0;
	for (iter = textureGlyphs_.begin(); iter != e; ++iter, ++pos)
	{
		int x = iter->second.x;
		int y = iter->second.y;
		int width = iter->second.width;
		int height = iter->second.height;

		texwidth = std::max(texwidth, x + width + dx);
		texheight = std::max(texheight, y + height + dy);
	}

	texture_ = QImage(texwidth, texheight, QImage::Format_RGB32);
	QPainter tpainter(&texture_);
	tpainter.setBackground(QBrush(Qt::white));
	tpainter.eraseRect(0, 0, texwidth, texheight);

	for (iter = textureGlyphs_.begin(); iter != e; ++iter, ++pos)
	{
		FT_UInt glyph_index = FT_Get_Char_Index(face, iter->first.unicode());

		error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT); 
		if (error)
		{
			printf("Error: FT_Load_Glyph(...)\n");
			return;
		}

		error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
		if (error)
		{
			printf("Error: FT_Render_Glyph(...)\n");
			return;	
		}

		FT_Bitmap bitmap = face->glyph->bitmap;

		QImage image(bitmap.width, bitmap.rows, QImage::Format_RGB32);

		for (int y = 0; y < bitmap.rows; ++y)
			for (int x = 0; x < bitmap.width; ++x)
			{
				int index = x + y * bitmap.pitch;
				int c = 255 - bitmap.buffer[index];
				image.setPixel(x, y, c | (c << 8) | (c << 16));
			}

		int x = iter->second.x;
		int y = iter->second.y;
		painter.drawImage(x, y, image);
		tpainter.drawImage(x, y, image);
	}

	painter.setPen(QPen(Qt::DashLine));
	painter.drawRect(0, 0, texwidth, texheight);

	FT_Done_Face(face);
	FT_Done_FreeType(library);

	//texture.save(fontFile_ + ".png");
	//writeTextureGlyphs(fontFile_ + ".txt");
}
#endif


void FontCanvas::setFontFile(const QString& fontFile)
{
	fontFile_ = fontFile;
	update();
}

void FontCanvas::setFontSize(int size)
{
	fontSize_ = size;
	update();
}

void FontCanvas::setChars(const QString& chars)
{
	chars_ = chars;
	update();
}

void FontCanvas::writeTextureGlyphs(const QString& filename)
{
	FILE* fos = fopen(qPrintable(filename), "wt");

	if (fos)
	{
		fprintf(fos, "0\n");		// don't create mipmaps

		std::map<QChar, TextureGlyph>::iterator iter, e = textureGlyphs_.end();
		for (iter = textureGlyphs_.begin(); iter != e; ++iter)
		{
			const TextureGlyph& glyph = iter->second;

			if (glyph.chr != 0)
			{
				fprintf(fos, "%d\n", glyph.chr.unicode());
				fprintf(fos, "%d %d\n", glyph.x, glyph.y);
				fprintf(fos, "%d %d\n", glyph.width, glyph.height);
				fprintf(fos, "%d %d\n", glyph.left, glyph.top);
				fprintf(fos, "%d %d\n", glyph.advancex, glyph.advancey);
				fprintf(fos, "\n");
			}
		}

		fclose(fos);
	}
}

void FontCanvas::writeBMFont(const QString& filename, const QString& pngfilename)
{
	FILE* fos = fopen(qPrintable(filename), "wt");

	if (fos)
	{
        fprintf(fos, "info face=\"%s\" size=%d bold=0 italic=0 charset=\"\" unicode=1 stretchH=100 smooth=1 aa=1 padding=0,0,0,0 spacing=2,2\n", qPrintable(familyName_), fontSize_);
        fprintf(fos, "common lineHeight=%d base=%d scaleW=%d scaleH=%d pages=1 packed=0 alphaChnl=0 redChnl=4 greenChnl=4 blueChnl=4\n", height_, ascender_, texture_.width(), texture_.height());
		fprintf(fos, "page id=0 file=\"%s\"\n", qPrintable(pngfilename));

        fprintf(fos, "chars count=%d\n", textureGlyphs_.size());
		std::map<QChar, TextureGlyph>::iterator iter, e = textureGlyphs_.end();
		for (iter = textureGlyphs_.begin(); iter != e; ++iter)
		{
			const TextureGlyph& glyph = iter->second;

            fprintf(fos, "char id=%-4d x=%-5d y=%-5d width=%-5d height=%-5d xoffset=%-5d yoffset=%-5d xadvance=%-5d page=0  chnl=0\n",
				glyph.chr.unicode(),
				glyph.x, glyph.y,
				glyph.width, glyph.height,
                glyph.left, ascender_ - glyph.top,
                glyph.advancex >> 6);
		}

        {
            fprintf(fos, "kernings count=%d\n", kernings_.size());
            std::map<std::pair<QChar, QChar>, int>::iterator iter, e = kernings_.end();
            for (iter = kernings_.begin(); iter != kernings_.end(); ++iter)
                fprintf(fos, "kerning first=%-3d second=%-3d amount=%d\n", iter->first.first.unicode(), iter->first.second.unicode(), iter->second >> 6);
        }

		fclose(fos);
	}
}

static QImage convertImage(const QImage &texture)
{
    QImage result(texture.width(), texture.height(), QImage::Format_ARGB32);

    for (int y = 0; y < texture.height(); ++y)
        for (int x = 0; x < texture.width(); ++x)
        {
            QRgb c = texture.pixel(x, y);
            result.setPixel(x, y, qRgba(255, 255, 255, 255 - qRed(c)));
        }

    return result;
}

void FontCanvas::exportFont(const QString& fileName)
{
    convertImage(texture_).save(fileName);

	QFileInfo fileInfo(fileName);
	QString txtFileName = fileInfo.dir().absoluteFilePath(fileInfo.completeBaseName() + ".txt");

    //writeTextureGlyphs(txtFileName);
    writeBMFont(txtFileName, fileInfo.fileName());
}


