#ifndef CANVAS_H
#define CANVAS_H

#include <QWidget>
#include <QImage>
#include <QBrush>

class Canvas : public QWidget
{
	Q_OBJECT

public:
	Canvas(QWidget *parent = 0);
	~Canvas();

	void clear();

    QString packTextures(const QStringList& textureFileNames, const QStringList& names, int transparentPadding, int opaquePadding, bool removeAlphaBorder, float alphaThreshold, bool forceSquare, bool showUnusedAreas);

	const QImage& texture() const
	{
		return texture_;
	}

	const QString& textureLocationList() const
	{
		return textureLocationList_;
	}

protected:	
	void paintEvent(QPaintEvent* event);

private:
	QImage texture_;
	QBrush brush_;
	QString textureLocationList_;
};

#endif // CANVAS_H
