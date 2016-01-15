#include "mainwindow.h"
#include <QActionGroup>
#include <QSettings>
#include <QTimer>
#include <QFileDialog>
#include <QDesktopServices>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>
#include <algorithm>
#include <QPalette>
#include "libnetwork.h"
#include "applicationwrapper.h"
#include "glcanvas.h"
#include "platform.h"
#include "settingsdialog.h"
#include "constants.cpp"

MainWindow* MainWindow::instance;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent){
    MainWindow::instance = this;
    projectName_ = QString ();
    setupUiActions();
    setupUiProperties();

    createUiGroups();

    loadSettings();
}

MainWindow::~MainWindow(){}

void MainWindow::setupUiActions(){
    ui.setupUi(this);

    connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(ui.actionRestart, SIGNAL(triggered()), this, SLOT(actionRestart()));
    connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(actionOpen()));
    connect(ui.actionOpen_Directory, SIGNAL(triggered()), this, SLOT(actionOpen_Directory()));

    connect(ui.actionFull_Screen,   SIGNAL(triggered(bool)), this, SLOT(actionFull_Screen(bool)));
    connect(ui.actionHide_Menu,     SIGNAL(triggered()), this, SLOT(actionHide_Menu()));
    connect(ui.actionAlways_on_Top, SIGNAL(triggered(bool)), this, SLOT(actionAlwaysOnTop(bool)));

    connect(ui.actionQuarter,       SIGNAL(triggered()), this, SLOT(actionScale()));
    connect(ui.actionHalf,          SIGNAL(triggered()), this, SLOT(actionScale()));
    connect(ui.actionOriginal,      SIGNAL(triggered()), this, SLOT(actionScale()));
    connect(ui.actionDouble,        SIGNAL(triggered()), this, SLOT(actionScale()));
    connect(ui.actionTriple,        SIGNAL(triggered()), this, SLOT(actionScale()));
    connect(ui.actionZoom_In,       SIGNAL(triggered()), this, SLOT(actionScale()));
    connect(ui.actionZoom_Out,      SIGNAL(triggered()), this, SLOT(actionScale()));
    connect(ui.actionFit_To_Window, SIGNAL(triggered()), this, SLOT(actionScale()));

    connect(ui.actionFit_To_App, SIGNAL(triggered()), this, SLOT(actionFitWindow()));

    connect(ui.actionDraw_Infos, SIGNAL(triggered(bool)), this, SLOT(actionDraw_Infos(bool)));

    connect(ui.actionAuto_Scale, SIGNAL(triggered(bool)), this, SLOT(actionAuto_Scale(bool)));

    connect(ui.actionRotate_Left,  SIGNAL(triggered()), this, SLOT(actionRotate()));
    connect(ui.actionRotate_Right, SIGNAL(triggered()), this, SLOT(actionRotate()));

    connect(ui.actionPortrait,             SIGNAL(triggered()), this, SLOT(actionOrientation()));
    connect(ui.actionPortrait_Upside_Down, SIGNAL(triggered()), this, SLOT(actionOrientation()));
    connect(ui.actionLandscape_Left,       SIGNAL(triggered()), this, SLOT(actionOrientation()));
    connect(ui.actionLandscape_Right,      SIGNAL(triggered()), this, SLOT(actionOrientation()));

    connect(ui.action320x480,   SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action768x1024,  SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action640x960,   SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action1536x2048, SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action320x568,   SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action640x1136,  SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action480x800,   SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action240x320,   SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action540x960,   SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action480x854,   SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action240x400,   SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action360x640,   SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action800x1280,  SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action600x1024,  SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action600x800,   SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action768x1366,  SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action720x1280,  SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action1080x1920, SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action750x1334,  SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action1242x2208, SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action900x1200,  SIGNAL(triggered()), this, SLOT(actionResolution()));

    connect(ui.action15_fps,    SIGNAL(triggered()), this, SLOT(actionFps()));
    connect(ui.action30_fps,    SIGNAL(triggered()), this, SLOT(actionFps()));
    connect(ui.action60_fps,    SIGNAL(triggered()), this, SLOT(actionFps()));
    connect(ui.actionUnlimited, SIGNAL(triggered()), this, SLOT(actionFps()));

    connect(ui.actionSettings, SIGNAL(triggered()), this, SLOT(actionSettings()));

    connect(ui.glCanvas, SIGNAL(projectNameChanged(const QString&)), this, SLOT(projectNameChanged(const QString&)));
}

float MainWindow::deviceScale()
{
    return (float)((float)scale() * (float)devicePixelRatio());
}

void MainWindow::setupUiProperties(){
    ui.action320x480->setProperty("width",  320);
    ui.action320x480->setProperty("height", 480);

    ui.action768x1024->setProperty("width",  768);
    ui.action768x1024->setProperty("height", 1024);

    ui.action640x960->setProperty("width",  640);
    ui.action640x960->setProperty("height", 960);

    ui.action1536x2048->setProperty("width",  1536);
    ui.action1536x2048->setProperty("height", 2048);

    ui.action320x568->setProperty("width",  320);
    ui.action320x568->setProperty("height", 568);

    ui.action640x1136->setProperty("width",  640);
    ui.action640x1136->setProperty("height", 1136);

    ui.action480x800->setProperty("width",  480);
    ui.action480x800->setProperty("height", 800);

    ui.action240x320->setProperty("width",  240);
    ui.action240x320->setProperty("height", 320);

    ui.action540x960->setProperty("width",  540);
    ui.action540x960->setProperty("height", 960);

    ui.action480x854->setProperty("width",  480);
    ui.action480x854->setProperty("height", 854);

    ui.action240x400->setProperty("width",  240);
    ui.action240x400->setProperty("height", 400);

    ui.action360x640->setProperty("width",  360);
    ui.action360x640->setProperty("height", 640);

    ui.action800x1280->setProperty("width",  800);
    ui.action800x1280->setProperty("height", 1280);

    ui.action600x1024->setProperty("width",  600);
    ui.action600x1024->setProperty("height", 1024);

    ui.action600x800->setProperty("width",  600);
    ui.action600x800->setProperty("height", 800);

    ui.action768x1366->setProperty("width",  768);
    ui.action768x1366->setProperty("height", 1366);

    ui.action720x1280->setProperty("width",  720);
    ui.action720x1280->setProperty("height", 1280);

    ui.action1080x1920->setProperty("width",  1080);
    ui.action1080x1920->setProperty("height", 1920);

    ui.action750x1334->setProperty("width",  750);
    ui.action750x1334->setProperty("height", 1334);

    ui.action1242x2208->setProperty("width",  1242);
    ui.action1242x2208->setProperty("height", 2208);

    ui.action900x1200->setProperty("width",  900);
    ui.action900x1200->setProperty("height", 1200);

    ui.action15_fps->setProperty("fps",    15);
    ui.action30_fps->setProperty("fps",    30);
    ui.action60_fps->setProperty("fps",    60);
    ui.actionUnlimited->setProperty("fps", 10000);

    ui.actionPortrait->setProperty("orientation",             ePortrait);
    ui.actionPortrait_Upside_Down->setProperty("orientation", ePortraitUpsideDown);
    ui.actionLandscape_Left->setProperty("orientation",       eLandscapeLeft);
    ui.actionLandscape_Right->setProperty("orientation",      eLandscapeRight);

    ui.actionQuarter->setProperty("scale",       25);
    ui.actionHalf->setProperty("scale",          50);
    ui.actionOriginal->setProperty("scale",      100);
    ui.actionDouble->setProperty("scale",        200);
    ui.actionTriple->setProperty("scale",        300);
    ui.actionZoom_In->setProperty("scale",       eZoomIn);
    ui.actionZoom_Out->setProperty("scale",      eZoomOut);
    ui.actionFit_To_Window->setProperty("scale", eFitToWindow);

    ui.actionRotate_Left->setProperty("rotate",  eLeft);
    ui.actionRotate_Right->setProperty("rotate", eRight);
}

void MainWindow::createUiGroups(){
    orientationGroup_ = new QActionGroup(this);
    orientationGroup_->addAction(ui.actionPortrait);
    orientationGroup_->addAction(ui.actionPortrait_Upside_Down);
    orientationGroup_->addAction(ui.actionLandscape_Left);
    orientationGroup_->addAction(ui.actionLandscape_Right);

    resolutionGroup_ = new QActionGroup(this);
    resolutionGroup_->addAction(ui.action320x480);
    resolutionGroup_->addAction(ui.action768x1024);
    resolutionGroup_->addAction(ui.action640x960);
    resolutionGroup_->addAction(ui.action1536x2048);
    resolutionGroup_->addAction(ui.action320x568);
    resolutionGroup_->addAction(ui.action640x1136);
    resolutionGroup_->addAction(ui.action480x800);
    resolutionGroup_->addAction(ui.action240x320);
    resolutionGroup_->addAction(ui.action540x960);
    resolutionGroup_->addAction(ui.action480x854);
    resolutionGroup_->addAction(ui.action240x400);
    resolutionGroup_->addAction(ui.action360x640);
    resolutionGroup_->addAction(ui.action800x1280);
    resolutionGroup_->addAction(ui.action600x1024);
    resolutionGroup_->addAction(ui.action600x800);
    resolutionGroup_->addAction(ui.action768x1366);
    resolutionGroup_->addAction(ui.action720x1280);
    resolutionGroup_->addAction(ui.action1080x1920);
    resolutionGroup_->addAction(ui.action750x1334);
    resolutionGroup_->addAction(ui.action1242x2208);
    resolutionGroup_->addAction(ui.action900x1200);

    fpsGroup_ = new QActionGroup(this);
    fpsGroup_->addAction(ui.action15_fps);
    fpsGroup_->addAction(ui.action30_fps);
    fpsGroup_->addAction(ui.action60_fps);
    fpsGroup_->addAction(ui.actionUnlimited);
}

void MainWindow::loadSettings(){
    setFullScreen(false);
    setHideMenu(false);

    QSettings settingsNative;

    resize(settingsNative.value("size", QSize(320, 480 + ui.menuBar->height())).toSize());
    move(settingsNative.value("pos",    QPoint(0, 0)).toPoint());

    int red = settingsNative.value("backgroundRed",     240).toInt();
    int green = settingsNative.value("backgroundGreen", 240).toInt();
    int blue = settingsNative.value("backgroundBlue",   240).toInt();
    QColor backgroundColor = QColor(red, green, blue);

    red = settingsNative.value("canvasRed",     255).toInt();
    green = settingsNative.value("canvasGreen", 255).toInt();
    blue = settingsNative.value("canvasBlue",   255).toInt();
    QColor canvasColor = QColor(red, green, blue);

    red = settingsNative.value("infoRed",     0).toInt();
    green = settingsNative.value("infoGreen", 0).toInt();
    blue = settingsNative.value("infoBlue",   0).toInt();
    QColor infoColor = QColor(red, green, blue);

    setBackgroundColor(backgroundColor);
    setCanvasColor(canvasColor);
    setInfoColor(infoColor);

    updateBackgroundColor();
    updateCanvasColor();
    updateInfoColor();

    QSettings settings(Constants::SETTINGS_FOLDER + "/" + Constants::PLAYER_SETTINGS_FILE, QSettings::IniFormat);

    setWidth(settings.value("width",             320).toInt());
    setHeight(settings.value("height",           480).toInt());
    setScale(settings.value("scale",             100).toInt());
    setFps(settings.value("fps",                 60).toInt());
    setOrientation(static_cast<Orientation>(settings.value("orientation", ePortrait).toInt()));
    setDrawInfos(settings.value("drawInfos",     false).toBool());
    setAutoScale(settings.value("autoScale",     false).toBool());
    setAlwaysOnTop(settings.value("alwaysOnTop", false).toBool());

    checkLoadedSettings();

    updateFps();
    updateDrawInfos();
    updateAlwaysOnTop();
    updateAutoScale();
    updateOrientation();
    updateResolution(true);
}

void MainWindow::checkLoadedSettings(){
    QAction *action;

    action = resolutionGroup_->checkedAction();

    if(action)
        action->setChecked(false);

    QList<QAction*> resolutionListActions = resolutionGroup_->actions();
    foreach(QAction *action, resolutionListActions){
        if(action->property("width").toInt() == width() && action->property("height").toInt() == height()){
            action->setChecked(true);
            break;
        }
    }

    action = fpsGroup_->checkedAction();
    if(action)
        action->setChecked(false);

    QList<QAction*> fpsListActions = fpsGroup_->actions();
    foreach(QAction *action, fpsListActions){
        if(action->property("fps").toInt() == fps()){
            action->setChecked(true);
            break;
        }
    }

    action = orientationGroup_->checkedAction();
    if(action)
        action->setChecked(false);

    QList<QAction*> orientationListActions = orientationGroup_->actions();
    foreach(QAction *action, orientationListActions){
        if(static_cast<Orientation>(action->property("orientation").toInt()) == orientation()){
            action->setChecked(true);
            break;
        }
    }

    ui.actionDraw_Infos->setChecked(drawInfos());
    ui.actionAlways_on_Top->setChecked(alwaysOnTop());
    ui.actionAuto_Scale->setChecked(autoScale());
}

void MainWindow::saveSettings(){
    if(fullScreen())
        this->showNormal();

    QSettings settingsNative;

    settingsNative.setValue("pos",  pos());
    settingsNative.setValue("size", size());

    settingsNative.setValue("backgroundRed", backgroundColor().red());
    settingsNative.setValue("backgroundGreen", backgroundColor().green());
    settingsNative.setValue("backgroundBlue", backgroundColor().blue());

    settingsNative.setValue("canvasRed", canvasColor().red());
    settingsNative.setValue("canvasGreen", canvasColor().green());
    settingsNative.setValue("canvasBlue", canvasColor().blue());

    settingsNative.setValue("infoRed", infoColor().red());
    settingsNative.setValue("infoGreen", infoColor().green());
    settingsNative.setValue("infoBlue", infoColor().blue());

    QSettings settings(Constants::SETTINGS_FOLDER + "/" + Constants::PLAYER_SETTINGS_FILE, QSettings::IniFormat);

    settings.setValue("orientation", orientation());
    settings.setValue("alwaysOnTop", alwaysOnTop());
    settings.setValue("autoScale",   autoScale());
    settings.setValue("width",       width());
    settings.setValue("height",      height());
    settings.setValue("fps",         fps());
    settings.setValue("scale",       scale());
    settings.setValue("drawInfos",   drawInfos());

    if(fullScreen())
        this->showFullScreen();
}

void MainWindow::actionOpen(){
    QDir dir = QDir::temp();
    dir.mkdir("gideros");
    dir.cd("gideros");

    QString directoryName = QFileDialog::getExistingDirectory(this, Constants::PLAYER_OPEN_DIALOG_NAME, dir.absolutePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if(!directoryName.isEmpty()){
        QDir directory(directoryName);
        ui.glCanvas->projectDir_ = directory.absolutePath();
    }
}

void MainWindow::actionRestart(){
    if(!projectName_.isEmpty()){
        QDir dir = QDir::temp();
        dir.mkdir("gideros");
        dir.cd("gideros");
        dir.cd(projectName_);
        ui.glCanvas->projectDir_ = dir.absolutePath();
		
        MainWindow::getInstance()->setCursor(Qt::ArrowCursor);
        MainWindow::getInstance()->setWindowFlags(Qt::Window);
        MainWindow::updateFullScreen();
		
    }
}

void MainWindow::actionOpen_Directory(){
    QDesktopServices::openUrl(QUrl("file:///"+getWorkingDirectory(), QUrl::TolerantMode));
}

QString MainWindow::getWorkingDirectory(){
    QDir dir = QDir::temp();
    dir.mkdir("gideros");
    dir.cd("gideros");
    if(!projectName_.isEmpty())
        dir.cd(projectName_);
    return dir.absolutePath();
}

void MainWindow::actionDraw_Infos(bool checked){
    setDrawInfos(checked);

    updateDrawInfos();
}

void MainWindow::actionFull_Screen(bool checked){
    setFullScreen(checked);

    updateFullScreen();
}

void MainWindow::actionHide_Menu(){
    setHideMenu(!ui.menuBar->isHidden());

    updateHideMenu();
}

void MainWindow::actionAlwaysOnTop(bool checked){
    setAlwaysOnTop(checked);

    updateAlwaysOnTop();
}

void MainWindow::actionAuto_Scale(bool checked){
    setAutoScale(checked);

    updateAutoScale();
}

void MainWindow::actionResolution(){
    QAction *action = (QAction*)sender();

    setWidth( action->property("width").toInt());
    setHeight(action->property("height").toInt());

    updateResolution(true);
}

void MainWindow::actionFps(){
    QAction *action = (QAction*)sender();

    setFps(action->property("fps").toInt());

    updateFps();
}

void MainWindow::actionOrientation(){
    QAction *action = (QAction*)sender();

    setOrientation(static_cast<Orientation>(action->property("orientation").toInt()));

    updateOrientation();
}

void MainWindow::actionScale(){
    QAction *action = (QAction*)sender();

    int scaleProperty = action->property("scale").toInt();
    int scaleCurrent = scale();

    switch(scaleProperty){
        case(eZoomIn):
            scaleProperty = scaleCurrent;
            ++scaleProperty;
            break;

        case(eZoomOut):
            scaleProperty = scaleCurrent;
            if(scaleProperty > 1)
                --scaleProperty;
            break;

        case(eFitToWindow):
            int width = scale() * ui.centralWidget->width() / ui.glCanvas->width();
            int height = scale() * ui.centralWidget->height() / ui.glCanvas->height();

            scaleProperty = std::min(width, height);
            break;
    }

    setScale(scaleProperty);

    updateResolution(true);
}

void MainWindow::actionFitWindow(){
    resize(sizeHint());
}

void MainWindow::actionRotate(){
    QAction *action = (QAction*)sender();

    if(action->property("rotate").toInt() == eLeft){
        switch(orientation())
        {
        case ePortrait:
            ui.actionLandscape_Left->trigger();
            break;
        case eLandscapeLeft:
            ui.actionPortrait_Upside_Down->trigger();
            break;
        case ePortraitUpsideDown:
            ui.actionLandscape_Right->trigger();
            break;
        case eLandscapeRight:
            ui.actionPortrait->trigger();
            break;
        }

    }else{
        switch (orientation())
        {
        case ePortrait:
            ui.actionLandscape_Right->trigger();
            break;
        case eLandscapeRight:
            ui.actionPortrait_Upside_Down->trigger();
            break;
        case ePortraitUpsideDown:
            ui.actionLandscape_Left->trigger();
            break;
        case eLandscapeLeft:
            ui.actionPortrait->trigger();
            break;
        }
    }

    updateOrientation();
}

void MainWindow::actionSettings(){
    SettingsDialog settingsDialog(this);
    settingsDialog.exec();
}

void MainWindow::updateHideMenu(){
    if(hideMenu())
        ui.menuBar->hide();

    else
        ui.menuBar->show();

    updateResolution(true);
}

void MainWindow::updateFullScreen(){
    if(fullScreen())
        this->showFullScreen();
    else
        this->showNormal();
}

void MainWindow::updateResolution(bool event){
    if(autoScale()){
        setWidth(ui.centralWidget->width());
        setHeight(ui.centralWidget->height());

        if(orientation() == eLandscapeLeft || orientation() == eLandscapeRight){
            setWidth(ui.centralWidget->height());
            setHeight(ui.centralWidget->width());

            if(hideMenu())
                setWidth(width() + ui.menuBar->height());

        }else if(hideMenu())
            setHeight(height() + ui.menuBar->height());
    }

    float canvasScaleFactor = 1;
    float widgetScaleFactor = 1;

    if (deviceScale() != 0) {
        const float hundredPercentScale = 100;
        canvasScaleFactor = hundredPercentScale / deviceScale();
        widgetScaleFactor = hundredPercentScale / scale();
    }

    ui.glCanvas->setScale(canvasScaleFactor);

    switch (orientation()){
        case ePortrait:
        case ePortraitUpsideDown:
            ui.glCanvas->setFixedSize(width() / widgetScaleFactor, height() / widgetScaleFactor);
            break;

        case eLandscapeLeft:
        case eLandscapeRight:
            ui.glCanvas->setFixedSize(height() / widgetScaleFactor, width() / widgetScaleFactor);
            break;
    }

    ui.glCanvas->setResolution(width(), height(),event);
}

void MainWindow::updateAutoScale(){
    if(autoScale()){
        ui.centralWidget->setMinimumSize(1, 1);

    }else{
        ui.centralWidget->setMinimumSize(0, 0);

        QAction *action = resolutionGroup_->checkedAction();
        if(action){
            setWidth(action->property("width").toInt());
            setHeight(action->property("height").toInt());
        }
    }

    resolutionGroup_->setEnabled(!autoScale());

    updateResolution(true);
}

void MainWindow::updateAlwaysOnTop(){
    // TODO: burada mac ve X11 icin extra seyler yapmak lazim
    // keywords: Qt::Tool, Qt::WA_MacAlwaysShowToolWindow
    if(alwaysOnTop())
        setWindowFlags((windowFlags() & ~Qt::WindowStaysOnBottomHint) | Qt::WindowStaysOnTopHint);

    else
        setWindowFlags((windowFlags() & ~Qt::WindowStaysOnTopHint) | Qt::WindowStaysOnBottomHint);

    show();
}

void MainWindow::updateFps(){
    ui.glCanvas->setFps(fps());
}

void MainWindow::updateOrientation(){
    ui.glCanvas->setHardwareOrientation(orientation());

    updateResolution(true);
}

void MainWindow::updateDrawInfos(){
    ui.glCanvas->setDrawInfos(drawInfos());
}

void MainWindow::updateBackgroundColor(){
    QPalette palette;
    palette.setColor(QPalette::Window, backgroundColor());
    ui.centralWidget->window()->setPalette(palette);
}

void MainWindow::updateCanvasColor(){
    float glCanvasColor[3];
    glCanvasColor[0] = (float)canvasColor().red() / (float)255;
    glCanvasColor[1] = (float)canvasColor().green() / (float)255;
    glCanvasColor[2] = (float)canvasColor().blue() / (float)255;

    ui.glCanvas->setCanvasColor(glCanvasColor);
}

void MainWindow::updateInfoColor(){
    float glInfoColor[3];
    glInfoColor[0] = (float)infoColor().red() / (float)255;
    glInfoColor[1] = (float)infoColor().green() / (float)255;
    glInfoColor[2] = (float)infoColor().blue() / (float)255;

    ui.glCanvas->setInfoColor(glInfoColor);
}

int MainWindow::fps(){
    return fps_;
}

void MainWindow::setFps(int fps){
    fps_ = fps;
}

int MainWindow::height(){
    return height_;
}

void MainWindow::setHeight(int height){
    height_ = height;
}

int MainWindow::width(){
    return width_;
}

void MainWindow::setWidth(int width){
    width_ = width;
}

bool MainWindow::autoScale(){
    return autoScale_;
}

void MainWindow::setAutoScale(bool autoScale){
    autoScale_ = autoScale;
}

bool MainWindow::drawInfos(){
    return drawInfos_;
}

void MainWindow::setDrawInfos(bool drawInfos){
    drawInfos_ = drawInfos;
}

bool MainWindow::alwaysOnTop(){
    return alwaysOnTop_;
}

void MainWindow::setAlwaysOnTop(bool alwaysOnTop){
    alwaysOnTop_ = alwaysOnTop;
}

Orientation MainWindow::orientation(){
    return orientation_;
}

void MainWindow::setOrientation(Orientation orientation){
    orientation_ = orientation;
}

bool MainWindow::hideMenu(){
    return hideMenu_;
}

void MainWindow::setHideMenu(bool hideMenu){
    hideMenu_ = hideMenu;
}

bool MainWindow::fullScreen(){
    return fullScreen_;
}

void MainWindow::setFullScreen(bool fullScreen){
    fullScreen_ = fullScreen;
    ui.actionFull_Screen->setChecked(fullScreen);
}

float MainWindow::scale(){
    return scale_;
}

void MainWindow::setScale(float scale){
    scale_ = scale;
}

void MainWindow::setBackgroundColor(QColor backgroundColor){
    backgroundColor_ = backgroundColor;
}

QColor MainWindow::backgroundColor(){
    return backgroundColor_;
}

void MainWindow::setCanvasColor(QColor canvasColor){
    canvasColor_ = canvasColor;
}

QColor MainWindow::canvasColor(){
    return canvasColor_;
}

void MainWindow::setInfoColor(QColor infoColor){
    infoColor_ = infoColor;
}

QColor MainWindow::infoColor(){
    return infoColor_;
}

void MainWindow::resizeWindow(int width, int height){
    setWidth(width);
    setHeight(height);
    if(orientation() == eLandscapeLeft || orientation() == eLandscapeRight){
        int temp = width;
        width = height;
        height = temp;
    }
    if(!hideMenu())
        height = height + ui.menuBar->height();

    resize(width, height);
    updateResolution(false);
}

void MainWindow::fullScreenWindow(bool _fullScreen){
    setAutoScale(_fullScreen);
    setFullScreen(_fullScreen);
    actionFull_Screen(fullScreen());
    updateAutoScale();
}

void MainWindow::resizeEvent(QResizeEvent*){    
    if(autoScale())
        updateResolution(true);
}

void MainWindow::closeEvent(QCloseEvent*){
    saveSettings();
}

void MainWindow::projectNameChanged(const QString& projectName){
    show();
    raise();
    activateWindow();
    projectName_ = projectName;
    if (projectName.isEmpty() == true)
        setWindowTitle(Constants::PLAYER_WINDOW_TITLE);
    else
        setWindowTitle(projectName + " - " + Constants::PLAYER_WINDOW_TITLE);
}

void MainWindow::setFixedSize(bool fixedSize){

}

QSize MainWindow::windowSize(){
    int width,height;
    width = size().width();
    height = size().height();

    if(!hideMenu())
        height = height - ui.menuBar->height();

    return QSize(width,height);
}


void MainWindow::printToOutput(const char* text){
    ui.glCanvas->printToOutput(text);
}

/*
void MainWindow::exportAccessedFiles()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
        QString(),
        tr("Text File (*.txt)"));


    if (fileName.isEmpty() == false)
    {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            // TODO: show message box
            return;
        }

        QTextStream out(&file);

        const std::set<std::string>& accessedFiles = ui.glCanvas->accessedResourceFiles;

        out << "------- Accessed Files ---------------\n";
        for (std::set<std::string>::const_iterator iter = accessedFiles.begin(); iter != accessedFiles.end(); ++iter)
            out << iter->c_str() << "\n";

        const std::set<std::string>& allFiles = ui.glCanvas->allResourceFiles;

        std::vector<std::string> v;
        std::set_difference(allFiles.begin(), allFiles.end(), accessedFiles.begin(), accessedFiles.end(), std::back_inserter(v));

        out << "------ Unaccessed Files --------------\n";
        for (std::size_t i = 0; i < v.size(); ++i)
            out << v[i].c_str() << "\n";
    }
}

#include <QCursor>
#include <QBitmap>
#include <QPixmap>

QPixmap pixmap("Images/cursor.png");
QCursor cursor(pixmap, 222, 62);

QApplication::setOverrideCursor(cursor);

void MainWindow::mousePressEvent(QMouseEvent*)
{
    QPixmap pixmap("Images/cursor2.png");
    QCursor cursor(pixmap, 222, 62);

    QApplication::setOverrideCursor(cursor);
}

void MainWindow::mouseReleaseEvent(QMouseEvent*)
{
    QPixmap pixmap("Images/cursor.png");
    QCursor cursor(pixmap, 222, 62);

    QApplication::setOverrideCursor(cursor);
}
*/

/*
// send run shortcut (ctrl+r)
void MainWindow::sendRun()
{
    ui.glCanvas->sendRun();
}
*/
