#ifndef FONTCANVAS_H
#define FONTCANVAS_H

#include <QWidget>
#include <map>

#include <ft2build.h>
#include FT_FREETYPE_H

class QPaintEvent;

class FontCanvas : public QWidget
{
	Q_OBJECT

public:
	FontCanvas(QWidget *parent);
	~FontCanvas();

	void setFontFile(const QString& fontFile);

	void exportFont(const QString& fileName);

public slots:
	void setFontSize(int size);
	void setChars(const QString& chars);

protected:
	void paintEvent(QPaintEvent *);

private:
    QString fontFile_;
	int fontSize_;
	QString chars_;

	int ascender_;
	int height_;
    QString familyName_;

	struct TextureGlyph
	{
		TextureGlyph() : chr(0) {}

		QChar chr;
        FT_UInt glyphIndex;
		int x, y;
		int width, height;
		int left, top;
		int advancex, advancey;
	};

	std::map<QChar, TextureGlyph> textureGlyphs_;    
    std::map<std::pair<QChar, QChar>, int> kernings_;

    QImage texture_;

    void writeTextureGlyphs(const QString& filename);
	void writeBMFont(const QString& filename, const QString& pngfilename);
};

#endif // FONTCANVAS_H
