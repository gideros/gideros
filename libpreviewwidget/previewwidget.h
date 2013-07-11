#ifndef PREVIEWWIDGET_H
#define PREVIEWWIDGET_H

#include <QWidget>

class PreviewWidget : public QWidget
{
	Q_OBJECT

public:
	PreviewWidget(QWidget *parent = 0);
	~PreviewWidget();

	void setFileName(const QString& fileName, const QString& title);

private:
	QWidget* widget_;
};

#endif // PREVIEWWIDGET_H
