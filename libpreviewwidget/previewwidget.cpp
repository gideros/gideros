#include "previewwidget.h"
#include <QPainter>
#include <QImage>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QBrush>
#include <PVRTDecompress.h>
#include <PVRTResourceFile.h>
#include <PVRTTexture.h>
#include <QLabel>

class ImageWidget : public QWidget
{
public:
	ImageWidget(const QString& fileName, const QString& title, QWidget* parent = 0) : QWidget(parent), title_(title)
	{
		if (isBrushSet_ == false)
		{
			QImage brushImage(64, 64, QImage::Format_RGB32);

			for (int y = 0; y < 64; ++y)
				for (int x = 0; x < 64; ++x)
				{
					int u = x / 32;
					int v = y / 32;
					int b = (u ^ v) & 1;

					brushImage.setPixel(QPoint(x, y), b ? 0xb0b0b0 : 0xe0e0e0);
				}

			brush_ = QBrush(brushImage);

			isBrushSet_ = true;
		}

		QFileInfo fileInfo(fileName);
#if 0	// we drop support for pvrt
		if (fileInfo.suffix().toLower() == "pvr")
		{
			CPVRTResourceFile file(qPrintable(fileName));

			if (file.IsOpen() == true && file.Size() >= sizeof(PVR_Texture_Header))
			{
				PVR_Texture_Header* header = (PVR_Texture_Header*)file.DataPtr();

				int width = header->dwWidth;
				int height = header->dwHeight;
				PVRTuint32 pixelType = header->dwpfFlags & PVRTEX_PIXELTYPE;

				if (pixelType == OGL_PVRTC2 || pixelType == OGL_PVRTC4)
				{
					void* ptr = (char*)file.DataPtr() + header->dwHeaderSize;

					unsigned char* resultImage = (unsigned char*)malloc(width * height * 4);
					PVRTDecompressPVRTC(ptr, pixelType == OGL_PVRTC2, width, height, resultImage);
					
					image_ = QImage(width, height, QImage::Format_ARGB32);

					for (int y = 0; y < height; ++y)
						for (int x = 0; x < width; ++x)
						{
							int index = (x + y * width) * 4;
							unsigned char r = resultImage[index + 0];
							unsigned char g = resultImage[index + 1];
							unsigned char b = resultImage[index + 2];
							unsigned char a = resultImage[index + 3];

							image_.setPixel(x, y, qRgba(r, g, b, a));
						}

					free(resultImage);
				}
			}
		}
		else
#endif
			image_.load(fileName);
	}

protected:
	virtual void paintEvent(QPaintEvent* event)
	{
		QPainter painter(this);
		painter.setBackground(brush_);
		painter.eraseRect(0, 0, width(), height());

		int x = (width() - image_.width()) / 2;
		int y = (height() - image_.height()) / 2;

		painter.drawImage(QPoint(x, y), image_);

		painter.setFont(QFont("arial", 16, QFont::Bold));

		painter.setPen(QPen(Qt::lightGray));
		painter.drawText(7, 22, title_);

		painter.setPen(QPen(Qt::black));
		painter.drawText(7-2, 22-2, title_);
	}

private:
	QImage image_;
	QString title_;
	static QBrush brush_;
	static bool isBrushSet_;
};

QBrush ImageWidget::brush_;
bool ImageWidget::isBrushSet_ = false;


PreviewWidget::PreviewWidget(QWidget *parent) : QWidget(parent)
{
	setLayout(new QVBoxLayout);

	layout()->setMargin(0);
	layout()->setSpacing(0);

	widget_ = new ImageWidget(QString(), QString(), this);
	widget_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

	QLabel* label = new QLabel("Preview");
	label->setMargin(2);
	label->setStyleSheet(
		"border: 1px solid #AAAAAA;"
		"background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FCFCFC, stop: 1 #E2E2E2);"
	);

	layout()->addWidget(label);
	layout()->addWidget(widget_);
}

PreviewWidget::~PreviewWidget()
{

}

void PreviewWidget::setFileName(const QString& fileName, const QString& title)
{
	layout()->removeWidget(widget_);
	delete widget_;

	QFileInfo fileInfo(fileName);

	QString ext = fileInfo.suffix().toLower();

	if (ext == "png"  ||
		ext == "jpg"  ||
		ext == "jpeg" ||
		ext == "pvr")
		widget_ = new ImageWidget(fileName, title, this);
	else
		widget_ = new ImageWidget(QString(), QString(), this);

	widget_->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	layout()->addWidget(widget_);
}
