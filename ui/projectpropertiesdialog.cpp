#include "projectpropertiesdialog.h"
#include "ui_projectpropertiesdialog.h"
#include <QDebug>
#include <QIntValidator>
#include <QFileDialog>
#include <QPixmap>
#include <QColorDialog>
#include "projectpropertiesdialog.h"
#include "projectproperties.h"
#include <algorithm>

ProjectPropertiesDialog::ProjectPropertiesDialog(QString projectFileName,ProjectProperties* properties, QWidget *parent) :
    QDialog(parent),
	ui(new Ui::ProjectPropertiesDialog),
	properties_(properties)
{
	projectFileName_=projectFileName;

    ui->setupUi(this);

	ui->scaleMode->setCurrentIndex(properties_->scaleMode);

	ui->logicalWidth->setText(QString::number(properties_->logicalWidth));
	ui->logicalHeight->setText(QString::number(properties_->logicalHeight));

	ui->logicalWidth->setValidator(new QIntValidator());
	ui->logicalHeight->setValidator(new QIntValidator());

    ui->windowWidth->setText(QString::number(properties_->windowWidth));
    ui->windowHeight->setText(QString::number(properties_->windowHeight));

    ui->windowWidth->setValidator(new QIntValidator());
    ui->windowHeight->setValidator(new QIntValidator());

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

    ui->version->setText(properties_->version);
    ui->version_code->setText(QString::number(properties_->version_code));
    ui->disableSplash->setChecked(properties_->disableSplash);
    this->backgroundColor = properties_->backgroundColor;
    QPalette p;
    p.setColor(QPalette::Button, QColor(properties_->backgroundColor));
    ui->backgroundColor->setPalette(p);
    ui->backgroundColor->setAutoFillBackground(true);
    ui->backgroundColor->setFlat(true);

    connect(ui->add, SIGNAL(clicked()), this, SLOT(add()));
	connect(ui->remove, SIGNAL(clicked()), this, SLOT(remove()));

    connect(ui->appIcon, SIGNAL(clicked()), this, SLOT(addAppIcon()));
    connect(ui->tvIcon, SIGNAL(clicked()), this, SLOT(addTvIcon()));
    connect(ui->splashHImage, SIGNAL(clicked()), this, SLOT(addSplashHImage()));
    connect(ui->splashVImage, SIGNAL(clicked()), this, SLOT(addSplashVImage()));

    connect(ui->backgroundColor, SIGNAL(clicked()), this, SLOT(chooseColor()));

	connect(this, SIGNAL(accepted()), this, SLOT(onAccepted()));
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(loadImages()));

    loadImages();
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

    properties_->windowWidth = ui->windowWidth->text().toInt();
    properties_->windowHeight = ui->windowHeight->text().toInt();

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

    properties_->version = ui->version->text();
    properties_->version_code = ui->version_code->text().toInt();

	QDir path(QFileInfo(projectFileName_).path());
    if(!this->app_icon.isNull())
        properties_->app_icon = path.relativeFilePath(this->app_icon);
    if(!this->tv_icon.isNull())
        properties_->tv_icon = path.relativeFilePath(this->tv_icon);
    if(!this->splash_h_image.isNull())
        properties_->splash_h_image = path.relativeFilePath(this->splash_h_image);
    if(!this->splash_v_image.isNull())
        properties_->splash_v_image = path.relativeFilePath(this->splash_v_image);

    properties_->disableSplash = ui->disableSplash->isChecked();
    properties_->backgroundColor = this->backgroundColor.name();

}

void ProjectPropertiesDialog::loadImages()
{
    this->showImage(properties_->app_icon, ui->appIconLabel);
    this->showImage(properties_->tv_icon, ui->tvIconLabel);
    this->showImage(properties_->splash_h_image, ui->splashHImageLabel);
    this->showImage(properties_->splash_v_image, ui->splashVImageLabel);

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

void ProjectPropertiesDialog::showImage(QString fileName, QLabel* label)
{
    if(!fileName.isNull()){
		QDir path(QFileInfo(projectFileName_).path());
		QString src = path.absoluteFilePath(fileName);
		int w = label->width();
        int h = label->height();
        QPixmap p = QPixmap(src);
        label->setPixmap(p.scaled(w,h,Qt::KeepAspectRatio));
        label->update();
    }
}

void ProjectPropertiesDialog::addAppIcon()
{
    this->app_icon = QFileDialog::getOpenFileName(0, QObject::tr("Select app icon"),"",QObject::tr("Images (*.png *.jpeg *.jpg)"));
    this->showImage(this->app_icon, ui->appIconLabel);
}

void ProjectPropertiesDialog::addTvIcon()
{
    this->tv_icon = QFileDialog::getOpenFileName(0, QObject::tr("Select TV icon"),"",QObject::tr("Images (*.png *.jpeg *.jpg)"));
    this->showImage(this->tv_icon, ui->tvIconLabel);
}

void ProjectPropertiesDialog::addSplashHImage()
{
    this->splash_h_image = QFileDialog::getOpenFileName(0, QObject::tr("Select horizontal splash image"),"",QObject::tr("Images (*.png *.jpeg *.jpg)"));
    this->showImage(this->splash_h_image, ui->splashHImageLabel);
}

void ProjectPropertiesDialog::addSplashVImage()
{
    this->splash_v_image = QFileDialog::getOpenFileName(0, QObject::tr("Select vertical splash image"),"",QObject::tr("Images (*.png *.jpeg *.jpg)"));
    this->showImage(this->splash_v_image, ui->splashVImageLabel);
}

void ProjectPropertiesDialog::chooseColor()
{
    QColor color = QColorDialog::getColor(this->backgroundColor, 0, QObject::tr("Choose Splash Background Color"));
    if(color.isValid()){
        this->backgroundColor = color;
        QPalette p;
        p.setColor(QPalette::Button, this->backgroundColor);
        ui->backgroundColor->setPalette(p);
    }
}
