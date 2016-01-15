#include "settingsdialog.h"
#include <QFont>
#include <QFontMetrics>
#include <QSettings>
#include <QColor>
#include "constants.cpp"
#include "mainwindow.h"

SettingsDialog::SettingsDialog(MainWindow *mainWindow, QWidget *parent) : QDialog(parent), mainWindow_(mainWindow){
    ui.setupUi(this);

    setWindowFlags((windowFlags() & ~Qt::WindowStaysOnBottomHint) | Qt::WindowStaysOnTopHint);

    connect(ui.dialogButtonBox, SIGNAL(accepted()), this, SLOT(ok()));
    connect(ui.dialogButtonBox, SIGNAL(rejected()), this, SLOT(cancel()));

    setupUiItems();

    loadValues();
}

SettingsDialog::~SettingsDialog(){}

void SettingsDialog::setupUiItems(){
    ui.comboBoxOrientation->addItem(Constants::PORTRAIT, ePortrait);
    ui.comboBoxOrientation->addItem(Constants::LANDSCAPE_LEFT, eLandscapeLeft);
    ui.comboBoxOrientation->addItem(Constants::PORTRAIT_UPSIDE_DOWN, ePortraitUpsideDown);
    ui.comboBoxOrientation->addItem(Constants::LANDSCAPE_RIGHT, eLandscapeRight);

    ui.comboBoxFps->addItem("15", 15);
    ui.comboBoxFps->addItem("30", 30);
    ui.comboBoxFps->addItem("60", 60);
    ui.comboBoxFps->addItem("Unlimited", 10000);
}

void SettingsDialog::loadValues(){
    int width = mainWindow_->width();
    int height = mainWindow_->height();
    int orientation = mainWindow_->orientation();
    int fps = mainWindow_->fps();
    int fpsIndex;
    if(fps == 15)
        fpsIndex = 0;

    else if(fps == 30)
        fpsIndex = 1;

    else if(fps == 60)
        fpsIndex = 2;

    else
        fpsIndex = 3;
    int scale = mainWindow_->scale();
    QColor backgroundColor = mainWindow_->backgroundColor();
    QColor canvasColor = mainWindow_->canvasColor();
    QColor infoColor = mainWindow_->infoColor();

    ui.lineEditWidth->setText(QString::number(width));
    ui.lineEditHeight->setText(QString::number(height));
    ui.comboBoxOrientation->setCurrentIndex(orientation);
    ui.comboBoxFps->setCurrentIndex(fpsIndex);
    ui.lineEditScale->setText(QString::number(scale));
    ui.lineEditWindowRed->setText(QString::number(backgroundColor.red()));
    ui.lineEditWindowGreen->setText(QString::number(backgroundColor.green()));
    ui.lineEditWindowBlue->setText(QString::number(backgroundColor.blue()));
    ui.lineEditCanvasRed->setText(QString::number(canvasColor.red()));
    ui.lineEditCanvasGreen->setText(QString::number(canvasColor.green()));
    ui.lineEditCanvasBlue->setText(QString::number(canvasColor.blue()));
    ui.lineEditInfoRed->setText(QString::number(infoColor.red()));
    ui.lineEditInfoGreen->setText(QString::number(infoColor.green()));
    ui.lineEditInfoBlue->setText(QString::number(infoColor.blue()));
}

void SettingsDialog::cancel(){
    reject();
}

void SettingsDialog::ok(){
    saveValues();

    updateMainStatus();

    accept();
}

void SettingsDialog::saveValues(){
    int width = ui.lineEditWidth->text().toInt();
    int height = ui.lineEditHeight->text().toInt();
    Orientation orientation = static_cast<Orientation>(ui.comboBoxOrientation->currentData().toInt());
    int fps = ui.comboBoxFps->currentData().toInt();
    int scale = ui.lineEditScale->text().toInt();
    int red = ui.lineEditWindowRed->text().toInt();
    int green = ui.lineEditWindowGreen->text().toInt();
    int blue = ui.lineEditWindowBlue->text().toInt();
    QColor backgroundColor = QColor(red, green, blue);
    red = ui.lineEditCanvasRed->text().toInt();
    green = ui.lineEditCanvasGreen->text().toInt();
    blue = ui.lineEditCanvasBlue->text().toInt();
    QColor canvasColor = QColor(red, green, blue);
    red = ui.lineEditInfoRed->text().toInt();
    green = ui.lineEditInfoGreen->text().toInt();
    blue = ui.lineEditInfoBlue->text().toInt();
    QColor infoColor = QColor(red, green, blue);

    mainWindow_->setWidth(width);
    mainWindow_->setHeight(height);
    mainWindow_->setOrientation(orientation);
    mainWindow_->setFps(fps);
    mainWindow_->setScale(scale);
    mainWindow_->setBackgroundColor(backgroundColor);
    mainWindow_->setCanvasColor(canvasColor);
    mainWindow_->setInfoColor(infoColor);

    mainWindow_->saveSettings();
}

void SettingsDialog::updateMainStatus(){
    mainWindow_->updateFps();

    mainWindow_->updateOrientation();
    mainWindow_->updateResolution(true);
    mainWindow_->updateBackgroundColor();
    mainWindow_->updateCanvasColor();
    mainWindow_->updateInfoColor();

    mainWindow_->checkLoadedSettings();
}
