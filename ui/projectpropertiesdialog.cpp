#include "projectpropertiesdialog.h"
#include "ui_projectpropertiesdialog.h"
#include <QDebug>
#include <QIntValidator>
#include "projectpropertiesdialog.h"
#include "projectproperties.h"
#include <algorithm>

ProjectPropertiesDialog::ProjectPropertiesDialog(ProjectProperties* properties, QWidget *parent) :
    QDialog(parent),
	ui(new Ui::ProjectPropertiesDialog),
	properties_(properties)
{
    ui->setupUi(this);

	ui->scaleMode->setCurrentIndex(properties_->scaleMode);

	ui->logicalWidth->setText(QString::number(properties_->logicalWidth));
	ui->logicalHeight->setText(QString::number(properties_->logicalHeight));

	ui->logicalWidth->setValidator(new QIntValidator());
	ui->logicalHeight->setValidator(new QIntValidator());

	for (size_t i = 0; i < properties_->imageScales.size(); ++i)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem;
		item->setFlags(item->flags() | Qt::ItemIsEditable);
		item->setText(0, properties_->imageScales[i].first);
		item->setText(1, QString::number(properties_->imageScales[i].second));
		ui->imageScales->addTopLevelItem(item);
	}

	ui->orientation->setCurrentIndex(properties_->orientation);

	switch (properties_->fps)
	{
		case 60:
			ui->fps->setCurrentIndex(0);
			break;
		case 30:
			ui->fps->setCurrentIndex(1);
			break;
	}

    ui->retinaDisplay->setCurrentIndex(properties_->retinaDisplay);
	ui->autorotation->setCurrentIndex(properties_->autorotation);

    ui->mouseToTouch->setChecked(properties_->mouseToTouch);
    ui->touchToMouse->setChecked(properties_->touchToMouse);
    ui->mouseTouchOrder->setCurrentIndex(properties_->mouseTouchOrder);

    connect(ui->add, SIGNAL(clicked()), this, SLOT(add()));
	connect(ui->remove, SIGNAL(clicked()), this, SLOT(remove()));

	connect(this, SIGNAL(accepted()), this, SLOT(onAccepted()));
}

ProjectPropertiesDialog::~ProjectPropertiesDialog()
{
    delete ui;
}


struct ItemScalesComp
{
	bool operator()(const std::pair<QString, double>& i1,
					const std::pair<QString, double>& i2) const
	{
		return i1.second > i2.second;
	}
};


void ProjectPropertiesDialog::onAccepted()
{
	properties_->scaleMode = ui->scaleMode->currentIndex();

	properties_->logicalWidth = ui->logicalWidth->text().toInt();
	properties_->logicalHeight = ui->logicalHeight->text().toInt();

	properties_->imageScales.clear();

	for (int i = 0; i < ui->imageScales->topLevelItemCount(); ++i)
	{
		QTreeWidgetItem* item = ui->imageScales->topLevelItem(i);
		properties_->imageScales.push_back(std::make_pair(item->text(0), item->text(1).toDouble()));
	}

	std::sort(properties_->imageScales.begin(), properties_->imageScales.end(), ItemScalesComp());

	properties_->orientation = ui->orientation->currentIndex();

	switch (ui->fps->currentIndex())
	{
	case 0:
		properties_->fps = 60;
		break;
	case 1:
		properties_->fps = 30;
		break;
	}

    properties_->retinaDisplay = ui->retinaDisplay->currentIndex();
	properties_->autorotation = ui->autorotation->currentIndex();

    properties_->mouseToTouch = ui->mouseToTouch->isChecked();
    properties_->touchToMouse = ui->touchToMouse->isChecked();
    properties_->mouseTouchOrder = ui->mouseTouchOrder->currentIndex();
}

void ProjectPropertiesDialog::add()
{
	QTreeWidgetItem* item = new QTreeWidgetItem;
	item->setFlags(item->flags() | Qt::ItemIsEditable);
	item->setText(0, "suffix");
	ui->imageScales->addTopLevelItem(item);
	ui->imageScales->editItem(item, 0);
}

void ProjectPropertiesDialog::remove()
{
	QList<QTreeWidgetItem*> items = ui->imageScales->selectedItems();

	for (int i = 0; i < items.size(); ++i)
		delete items[i];
}
