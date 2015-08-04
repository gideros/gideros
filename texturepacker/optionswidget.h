#ifndef OPTIONSWIDGET_H
#define OPTIONSWIDGET_H

#include <QWidget>
#include "ui_optionswidget.h"

struct ProjectProperties;

class OptionsWidget : public QWidget
{
	Q_OBJECT

public:
	OptionsWidget(QWidget *parent = 0);
	~OptionsWidget();

	int padding() const;
    int extrude() const;
	bool removeAlphaBorder() const;
	double alphaThreshold() const;
	bool forceSquare() const;
	bool showUnusedAreas() const;

	void setText(const QString& text);

	void updateUI(const ProjectProperties& properties);

signals:
	void updateTexture();

private:
	Ui::OptionsWidgetClass ui;
};

#endif // OPTIONSWIDGET_H
