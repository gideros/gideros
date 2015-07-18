#include "optionswidget.h"
#include "iconlibrary.h"
#include "projectproperties.h"


OptionsWidget::OptionsWidget(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	ui.dot->setPixmap(IconLibrary::instance().icon("red dot").pixmap(16));
	setText("");

	connect(ui.removeAlphaBorder, SIGNAL(clicked(bool)), ui.alphaThreshold, SLOT(setEnabled(bool)));

    connect(ui.padding, SIGNAL(valueChanged(int)), this, SIGNAL(updateTexture()));
    connect(ui.extrude, SIGNAL(valueChanged(int)), this, SIGNAL(updateTexture()));
    connect(ui.alphaThreshold, SIGNAL(valueChanged(double)), this, SIGNAL(updateTexture()));
	connect(ui.removeAlphaBorder, SIGNAL(toggled(bool)), this, SIGNAL(updateTexture()));
	connect(ui.forceSquare, SIGNAL(toggled(bool)), this, SIGNAL(updateTexture()));
	connect(ui.showUnusedAreas, SIGNAL(toggled(bool)), this, SIGNAL(updateTexture()));
}

OptionsWidget::~OptionsWidget()
{

}

int OptionsWidget::padding() const
{
    return ui.padding->value();
}

int OptionsWidget::extrude() const
{
    return ui.extrude->value();
}

bool OptionsWidget::removeAlphaBorder() const
{
	return ui.removeAlphaBorder->isChecked();
}

double OptionsWidget::alphaThreshold() const
{
	return ui.alphaThreshold->value();
}

bool OptionsWidget::forceSquare() const
{
	return ui.forceSquare->isChecked();
}

bool OptionsWidget::showUnusedAreas() const
{
	return ui.showUnusedAreas->isChecked();
}

void OptionsWidget::setText(const QString& text)
{
	ui.dot->setVisible(!text.isEmpty());
	ui.text->setText(text);
}

void OptionsWidget::updateUI(const ProjectProperties& properties)
{
	bool isBlocked = blockSignals(true);

    ui.padding->setValue(properties.padding);
    ui.extrude->setValue(properties.extrude);
    ui.forceSquare->setChecked(properties.forceSquare);
	ui.removeAlphaBorder->setChecked(properties.removeAlphaBorder);
	ui.alphaThreshold->setValue(properties.alphaThreshold);
	ui.showUnusedAreas->setChecked(properties.showUnusedAreas);
	ui.alphaThreshold->setEnabled(ui.removeAlphaBorder->isChecked());

	blockSignals(isBlocked);
}


