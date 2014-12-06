#include "mainwindow.h"
#include "libnetwork.h"
#include "applicationwrapper.h"
#include "glcanvas.h"
#include "platform.h"

#include <QActionGroup>
#include <QSettings>
#include <QTimer>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QFileDialog>

#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setupUi();
    setupGroups();
    loadSettings();
}

MainWindow::~MainWindow()
{
}

// connect the handlers into menus and shortcuts
void MainWindow::setupUi()
{
    ui.setupUi(this);

    // file menu
    connect(ui.actionOpen,                 SIGNAL(triggered()),                        this, SLOT(actionOpen()));
    connect(ui.actionExit,                 SIGNAL(triggered()),                        this, SLOT(close()));

    // view menu
    connect(ui.actionFull_Screen,          SIGNAL(triggered(bool)),                    this, SLOT(actionFull_Screen(bool)));
    connect(ui.actionHide_Menu,            SIGNAL(triggered(bool)),                    this, SLOT(actionHide_Menu(bool)));
    connect(ui.actionAlways_on_Top,        SIGNAL(triggered(bool)),                    this, SLOT(alwaysOnTop(bool)));
    connect(ui.actionQuarter,              SIGNAL(triggered()),                        this, SLOT(actionScale()));
    connect(ui.actionHalf,                 SIGNAL(triggered()),                        this, SLOT(actionScale()));
    connect(ui.actionOriginal,             SIGNAL(triggered()),                        this, SLOT(actionScale()));

    // hardware menu
    connect(ui.actionAuto_Scale,           SIGNAL(triggered(bool)),                    this, SLOT(actionAuto_Scale(bool)));
    connect(ui.actionRotate_Left,          SIGNAL(triggered()),                        this, SLOT(rotateLeft()));
    connect(ui.actionRotate_Right,         SIGNAL(triggered()),                        this, SLOT(rotateRight()));
    connect(ui.actionPortrait,             SIGNAL(triggered()),                        this, SLOT(portrait()));
    connect(ui.actionPortrait_Upside_Down, SIGNAL(triggered()),                        this, SLOT(portraitUpsideDown()));
    connect(ui.actionLandscape_Left,       SIGNAL(triggered()),                        this, SLOT(landscapeLeft()));
    connect(ui.actionLandscape_Right,      SIGNAL(triggered()),                        this, SLOT(landscapeRight()));

    // resolution (iDevices)
    connect(ui.action320x480,              SIGNAL(triggered()),                        this, SLOT(actionResolution()));
    connect(ui.action768x1024,             SIGNAL(triggered()),                        this, SLOT(actionResolution()));
    connect(ui.action640x960,              SIGNAL(triggered()),                        this, SLOT(actionResolution()));
    connect(ui.action1536x2048,            SIGNAL(triggered()),                        this, SLOT(actionResolution()));
    connect(ui.action320x568,              SIGNAL(triggered()),                        this, SLOT(actionResolution()));
    connect(ui.action640x1136,             SIGNAL(triggered()),                        this, SLOT(actionResolution()));
	connect(ui.action750x1334,             SIGNAL(triggered()),                        this, SLOT(actionResolution()));
	connect(ui.action1242x2208,            SIGNAL(triggered()),                        this, SLOT(actionResolution()));

    // resolution (other)
    connect(ui.action480x800,              SIGNAL(triggered()),                        this, SLOT(actionResolution()));
    connect(ui.action240x320,              SIGNAL(triggered()),                        this, SLOT(actionResolution()));
    connect(ui.action540x960,              SIGNAL(triggered()),                        this, SLOT(actionResolution()));
    connect(ui.action480x854,              SIGNAL(triggered()),                        this, SLOT(actionResolution()));
    connect(ui.action240x400,              SIGNAL(triggered()),                        this, SLOT(actionResolution()));
    connect(ui.action360x640,              SIGNAL(triggered()),                        this, SLOT(actionResolution()));
    connect(ui.action800x1280,             SIGNAL(triggered()),                        this, SLOT(actionResolution()));
    connect(ui.action600x1024,             SIGNAL(triggered()),                        this, SLOT(actionResolution()));
    connect(ui.action600x800,              SIGNAL(triggered()),                        this, SLOT(actionResolution()));
    connect(ui.action768x1366,             SIGNAL(triggered()),                        this, SLOT(actionResolution()));
    connect(ui.action720x1280,             SIGNAL(triggered()),                        this, SLOT(actionResolution()));
    connect(ui.action900x1200,             SIGNAL(triggered()),                        this, SLOT(actionResolution()));
    connect(ui.action1080x1920,            SIGNAL(triggered()),                        this, SLOT(actionResolution()));

    connect(ui.action15_fps,               SIGNAL(triggered()),                        this, SLOT(action15_fps()));
    connect(ui.action30_fps,               SIGNAL(triggered()),                        this, SLOT(action30_fps()));
    connect(ui.action60_fps,               SIGNAL(triggered()),                        this, SLOT(action60_fps()));
    connect(ui.actionUnlimited,            SIGNAL(triggered()),                        this, SLOT(actionUnlimited()));

    // shortcuts
    connect(ui.actionRun,                  SIGNAL(triggered()),                        this, SLOT(sendRun()));

    // triggers
    connect(ui.glCanvas,                   SIGNAL(projectNameChanged(const QString&)), this, SLOT(projectNameChanged(const QString&)));
}

// set up the groups QActionGroup
void MainWindow::setupGroups()
{
    // view menu
    zoomGroup_ = new QActionGroup(this);
    zoomGroup_->addAction(ui.actionQuarter);
    zoomGroup_->addAction(ui.actionHalf);
    zoomGroup_->addAction(ui.actionOriginal);

    // hardware menu
    orientationGroup_ = new QActionGroup(this);
    orientationGroup_->addAction(ui.actionPortrait);
    orientationGroup_->addAction(ui.actionPortrait_Upside_Down);
    orientationGroup_->addAction(ui.actionLandscape_Left);
    orientationGroup_->addAction(ui.actionLandscape_Right);
    resolutionGroup_ = new QActionGroup(this);

    // resolution (iDevices)
    resolutionGroup_->addAction(ui.action320x480);
    resolutionGroup_->addAction(ui.action768x1024);
    resolutionGroup_->addAction(ui.action640x960);
    resolutionGroup_->addAction(ui.action1536x2048);
    resolutionGroup_->addAction(ui.action320x568);
    resolutionGroup_->addAction(ui.action640x1136);
    resolutionGroup_->addAction(ui.action750x1334);
    resolutionGroup_->addAction(ui.action1242x2208);

 	// resolution (other)
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
    resolutionGroup_->addAction(ui.action900x1200);
    resolutionGroup_->addAction(ui.action1080x1920);
}

// load the QSettings and configure with persistent parameters
void MainWindow::loadSettings()
{
    QSettings settings;

    // the position and size of window
    QPoint pos = settings.value("pos", QPoint(50, 50)).toPoint();
    QSize size = settings.value("size", QSize(1, 1)).toSize();
    resize(size);
    move(pos);

    // the bools menu options
    if (settings.value("alwaysOnTop").toBool())
        ui.actionAlways_on_Top->trigger();

    if (settings.value("autoScale").toBool())
        ui.actionAuto_Scale->trigger();

    // load configured resolution and scale
    loadResolution(settings.value("resolution", 320).toInt());
    loadScale(settings.value("scale", 1).toInt());

    // set canvas settings, scale, size and resolution
    ui.glCanvas->setScale(scale());
    ui.glCanvas->setFixedSize(hardwareWidth()/deviceScale(), hardwareHeight()/deviceScale());
    ui.glCanvas->setResolution(hardwareWidth(), hardwareHeight());

    // load orientation and fps of player
    loadOrientation(static_cast<Orientation>(settings.value("orientation", ePortrait).toInt()));
    loadFps(settings.value("fps2", 60).toInt());
}

// save the current player settings
void MainWindow::saveSettings()
{
    QSettings settings;

    // save all flags and int values
    settings.setValue("pos", pos());
    settings.setValue("size", size());
    settings.setValue("orientation", orientation());
    settings.setValue("alwaysOnTop", ui.actionAlways_on_Top->isChecked());
    settings.setValue("autoScale", ui.actionAuto_Scale->isChecked());

    // save fps setting
    if (ui.action15_fps->isChecked())
        settings.setValue("fps2", 15);
    else if (ui.action30_fps->isChecked())
        settings.setValue("fps2", 30);
    else if (ui.action60_fps->isChecked())
        settings.setValue("fps2", 60);
    else
        settings.setValue("fps2", 10000);

    // save resolution setting
    // iDevices
    if (resolutionGroup_->checkedAction() == ui.action320x480)
        settings.setValue("resolution", 320);
    else if (resolutionGroup_->checkedAction() == ui.action768x1024)
        settings.setValue("resolution", 768);
    else if (resolutionGroup_->checkedAction() == ui.action640x960)
        settings.setValue("resolution", 640);
    else if (resolutionGroup_->checkedAction() == ui.action1536x2048)
        settings.setValue("resolution", 1536);
    else if (resolutionGroup_->checkedAction() == ui.action320x568)
        settings.setValue("resolution", 320+1);
    else if (resolutionGroup_->checkedAction() == ui.action640x1136)
        settings.setValue("resolution", 640+1);
    else if (resolutionGroup_->checkedAction() == ui.action750x1334)
        settings.setValue("resolution", 750);
    else if (resolutionGroup_->checkedAction() == ui.action1242x2208)
        settings.setValue("resolution", 1242);
    // other
    else if (resolutionGroup_->checkedAction() == ui.action480x800)
        settings.setValue("resolution", 480);
    else if (resolutionGroup_->checkedAction() == ui.action240x320)
        settings.setValue("resolution", 240);
    else if (resolutionGroup_->checkedAction() == ui.action540x960)
        settings.setValue("resolution", 540);
    else if (resolutionGroup_->checkedAction() == ui.action480x854)
        settings.setValue("resolution", 480+1);
    else if (resolutionGroup_->checkedAction() == ui.action240x400)
        settings.setValue("resolution", 240+1);
    else if (resolutionGroup_->checkedAction() == ui.action360x640)
        settings.setValue("resolution", 360);
    else if (resolutionGroup_->checkedAction() == ui.action800x1280)
        settings.setValue("resolution", 800);
    else if (resolutionGroup_->checkedAction() == ui.action600x1024)
        settings.setValue("resolution", 600);
    else if (resolutionGroup_->checkedAction() == ui.action600x800)
        settings.setValue("resolution", 600+1);
    else if (resolutionGroup_->checkedAction() == ui.action768x1366)
        settings.setValue("resolution", 768+1);
    else if (resolutionGroup_->checkedAction() == ui.action720x1280)
        settings.setValue("resolution", 720);
    else if (resolutionGroup_->checkedAction() == ui.action900x1200)
        settings.setValue("resolution", 900);
    else if (resolutionGroup_->checkedAction() == ui.action1080x1920)
        settings.setValue("resolution", 1080);
    else
        settings.setValue("resolution", 320);

    // save zoom factor setting
    if (ui.actionQuarter->isChecked())
        settings.setValue("scale", 4);
    else if (ui.actionHalf->isChecked())
        settings.setValue("scale", 2);
    else
        settings.setValue("scale", 1);
}

// load a resolution of loaded player by int res
void MainWindow::loadResolution(int resolution)
{
    switch (resolution)
    {
    // iDevices
    case 320:
        ui.action320x480->setChecked(true);
        break;
    case 768:
        ui.action768x1024->setChecked(true);
        break;
    case 640:
        ui.action640x960->setChecked(true);
        break;
    case 1536:
        ui.action1536x2048->setChecked(true);
        break;
    case 320+1:
        ui.action320x568->setChecked(true);
        break;
    case 640+1:
        ui.action640x1136->setChecked(true);
        break;
    case 750:
        ui.action750x1334->setChecked(true);
        break;
    case 1242:
        ui.action1242x2208->setChecked(true);
        break;
    // other
    case 480:
        ui.action480x800->setChecked(true);
        break;
    case 240:
        ui.action240x320->setChecked(true);
        break;
    case 540:
        ui.action540x960->setChecked(true);
        break;
    case 480+1:
        ui.action480x854->setChecked(true);
        break;
    case 240+1:
        ui.action240x400->setChecked(true);
        break;
    case 360:
        ui.action360x640->setChecked(true);
        break;
    case 800:
        ui.action800x1280->setChecked(true);
        break;
    case 600:
        ui.action600x1024->setChecked(true);
        break;
    case 600+1:
        ui.action600x800->setChecked(true);
        break;
    case 768+1:
        ui.action768x1366->setChecked(true);
        break;
    case 720:
        ui.action720x1280->setChecked(true);
        break;
    case 900:
        ui.action900x1200->setChecked(true);
        break;
    case 1080:
        ui.action1080x1920->setChecked(true);
        break;
    }
}

// load a scale to start player by scale factor
void MainWindow::loadScale(int scale)
{
    switch (scale)
    {
    case 1:
        ui.actionOriginal->setChecked(true);
        break;
    case 2:
        ui.actionHalf->setChecked(true);
        break;
    case 4:
        ui.actionQuarter->setChecked(true);
        break;
    }
}

// load an orientation for player by Orientation
void MainWindow::loadOrientation(Orientation orientation)
{
    switch (orientation)
    {
    case ePortrait:
        portrait();
        ui.actionPortrait->setChecked(true);
        break;
    case eLandscapeLeft:
        landscapeLeft();
        ui.actionLandscape_Left->setChecked(true);
        break;
    case ePortraitUpsideDown:
        portraitUpsideDown();
        ui.actionPortrait_Upside_Down->setChecked(true);
        break;
    case eLandscapeRight:
        landscapeRight();
        ui.actionLandscape_Right->setChecked(true);
        break;
    }
}

// load fpt of player by int
void MainWindow::loadFps(int fps)
{
    if (fps == 15)
        action15_fps();
    else if (fps == 30)
        action30_fps();
    else if (fps == 60)
        action60_fps();
    else
        actionUnlimited();
}

// get the scale factor of screen
int MainWindow::scale()
{
    if (ui.actionQuarter->isChecked())
        return 4;

    if (ui.actionHalf->isChecked())
        return 2;

    return 1;
}

int MainWindow::deviceScale()
{
    return scale() * devicePixelRatio();
}

// get hardware width based on resolution or in screen if auto scale is on
int MainWindow::hardwareWidth()
{
    if(ui.actionAuto_Scale->isChecked())
        switch (orientation())
        {
        case ePortrait:
        case ePortraitUpsideDown:
            return width();

        case eLandscapeLeft:
        case eLandscapeRight:
            return height();
        }
    // iDevices
    else if (resolutionGroup_->checkedAction() == ui.action320x480)
        return 320;
    else if (resolutionGroup_->checkedAction() == ui.action768x1024)
        return 768;
    else if (resolutionGroup_->checkedAction() == ui.action640x960)
        return 640;
    else if (resolutionGroup_->checkedAction() == ui.action1536x2048)
        return 1536;
    else if (resolutionGroup_->checkedAction() == ui.action320x568)
        return 320;
    else if (resolutionGroup_->checkedAction() == ui.action640x1136)
        return 640;
    else if (resolutionGroup_->checkedAction() == ui.action750x1334)
        return 750;
    else if (resolutionGroup_->checkedAction() == ui.action1242x2208)
        return 1242;
    // other
    else if (resolutionGroup_->checkedAction() == ui.action480x800)
        return 480;
    else if (resolutionGroup_->checkedAction() == ui.action240x320)
        return 240;
    else if (resolutionGroup_->checkedAction() == ui.action540x960)
        return 540;
    else if (resolutionGroup_->checkedAction() == ui.action480x854)
        return 480;
    else if (resolutionGroup_->checkedAction() == ui.action240x400)
        return 240;
    else if (resolutionGroup_->checkedAction() == ui.action360x640)
        return 360;
    else if (resolutionGroup_->checkedAction() == ui.action800x1280)
        return 800;
    else if (resolutionGroup_->checkedAction() == ui.action600x1024)
        return 600;
    else if (resolutionGroup_->checkedAction() == ui.action600x800)
        return 600;
    else if (resolutionGroup_->checkedAction() == ui.action768x1366)
        return 768;
    else if (resolutionGroup_->checkedAction() == ui.action720x1280)
        return 720;
    else if (resolutionGroup_->checkedAction() == ui.action900x1200)
        return 900;
    else if (resolutionGroup_->checkedAction() == ui.action1080x1920)
        return 1080;


    return 320;
}

// get hardware height based on resolution or in screen if auto scale is on
int MainWindow::hardwareHeight()
{
    if(ui.actionAuto_Scale->isChecked())
        switch (orientation())
        {
        case ePortrait:
        case ePortraitUpsideDown:
            if(ui.menuBar->isHidden())
                return height();
            else
                return height() - 21; // TODO: aqui, 21 é o height do menu superior, quando está à mostra, ele deve diminuir na screen, então devo
                                      // alterar aqui pegando o height do menu, pois pode ser diferente do mac para o windows

        case eLandscapeLeft:
        case eLandscapeRight:
            return width();
        }
    // iDevices
    else if (resolutionGroup_->checkedAction() == ui.action320x480)
        return 480;
    else if (resolutionGroup_->checkedAction() == ui.action768x1024)
        return 1024;
    else if (resolutionGroup_->checkedAction() == ui.action640x960)
        return 960;
    else if (resolutionGroup_->checkedAction() == ui.action1536x2048)
        return 2048;
    else if (resolutionGroup_->checkedAction() == ui.action320x568)
        return 568;
    else if (resolutionGroup_->checkedAction() == ui.action640x1136)
        return 1136;
    else if (resolutionGroup_->checkedAction() == ui.action750x1334)
        return 1334;
    else if (resolutionGroup_->checkedAction() == ui.action1242x2208)
        return 2208;
    // other
    else if (resolutionGroup_->checkedAction() == ui.action480x800)
        return 800;
    else if (resolutionGroup_->checkedAction() == ui.action240x320)
        return 320;
    else if (resolutionGroup_->checkedAction() == ui.action540x960)
        return 960;
    else if (resolutionGroup_->checkedAction() == ui.action480x854)
        return 854;
    else if (resolutionGroup_->checkedAction() == ui.action240x400)
        return 400;
    else if (resolutionGroup_->checkedAction() == ui.action360x640)
        return 640;
    else if (resolutionGroup_->checkedAction() == ui.action800x1280)
        return 1280;
    else if (resolutionGroup_->checkedAction() == ui.action600x1024)
        return 1024;
    else if (resolutionGroup_->checkedAction() == ui.action600x800)
        return 800;
    else if (resolutionGroup_->checkedAction() == ui.action768x1366)
        return 1366;
    else if (resolutionGroup_->checkedAction() == ui.action720x1280)
        return 1280;
    else if (resolutionGroup_->checkedAction() == ui.action900x1200)
        return 1200;
    else if (resolutionGroup_->checkedAction() == ui.action1080x1920)
        return 1920;

    return 480;
}

// player in portrait mode
void MainWindow::portrait()
{
    ui.glCanvas->setFixedSize(hardwareWidth()/deviceScale(), hardwareHeight()/deviceScale());
    ui.glCanvas->setHardwareOrientation(ePortrait);
}

// player in portraitUpsideDown mode
void MainWindow::portraitUpsideDown()
{
    ui.glCanvas->setFixedSize(hardwareWidth()/deviceScale(), hardwareHeight()/deviceScale());
    ui.glCanvas->setHardwareOrientation(ePortraitUpsideDown);
}

// player in landscapeLeft mode
void MainWindow::landscapeLeft()
{
    ui.glCanvas->setFixedSize(hardwareHeight()/deviceScale(), hardwareWidth()/deviceScale());
    ui.glCanvas->setHardwareOrientation(eLandscapeLeft);
}

// player in landscapeRight mode
void MainWindow::landscapeRight()
{
    ui.glCanvas->setFixedSize(hardwareHeight()/deviceScale(), hardwareWidth()/deviceScale());
    ui.glCanvas->setHardwareOrientation(eLandscapeRight);
}

// get the orientation of player by checked action
Orientation MainWindow::orientation() const
{
    if (orientationGroup_->checkedAction() == ui.actionPortrait)
        return ePortrait;
    else if (orientationGroup_->checkedAction() == ui.actionPortrait_Upside_Down)
        return ePortraitUpsideDown;
    else if (orientationGroup_->checkedAction() == ui.actionLandscape_Left)
        return eLandscapeLeft;
    else if (orientationGroup_->checkedAction() == ui.actionLandscape_Right)
        return eLandscapeRight;

    return ePortrait;
}

// function to handle auto scale option
void MainWindow::actionAuto_Scale(bool checked)
{
    if (checked)
    {
        if(ui.centralWidget->minimumSize() == QSize(0, 0))
            ui.centralWidget->setMinimumSize(1, 1);
    }
    else
    {
        ui.centralWidget->setMinimumSize(0, 0);
    }

    actionResolution();
}

// TODO: desativada por enquanto, mas esta basicamente funcionando (muitos bugs)
// action to open a project directly in the player
void MainWindow::actionOpen()
{
    QDir directory = QFileDialog::getExistingDirectory(this, "Open Directory", "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui.glCanvas->play(directory);
}

// handler of rotations to left
void MainWindow::rotateLeft()
{
	switch (orientation())
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
}

// handler of rotations to right
void MainWindow::rotateRight()
{
	switch (orientation())
	{
	case ePortrait:
		ui.actionLandscape_Right->trigger();
		break;
	case eLandscapeLeft:
		ui.actionPortrait->trigger();
		break;
	case ePortraitUpsideDown:
		ui.actionLandscape_Left->trigger();
		break;
	case eLandscapeRight:
		ui.actionPortrait_Upside_Down->trigger();
		break;
	}
}

// always on top function
void MainWindow::alwaysOnTop(bool checked)
{
    // TODO: burada mac ve X11 icin extra seyler yapmak lazim
    // keywords: Qt::Tool, Qt::WA_MacAlwaysShowToolWindow
    if (checked)
        setWindowFlags((windowFlags() & ~Qt::WindowStaysOnBottomHint) | Qt::WindowStaysOnTopHint);

    else
        setWindowFlags((windowFlags() & ~Qt::WindowStaysOnTopHint) | Qt::WindowStaysOnBottomHint);

    show();
}

// handler to set zoom factor, scale
void MainWindow::actionScale()
{
    ui.glCanvas->setScale(scale());
    actionResolution();
}

// action for 15 fps
void MainWindow::action15_fps()
{
	ui.action15_fps->setChecked(true);
	ui.action30_fps->setChecked(false);
	ui.action60_fps->setChecked(false);
	ui.actionUnlimited->setChecked(false);

	ui.glCanvas->setFps(15);
}

// action for 30 fps
void MainWindow::action30_fps()
{
	ui.action15_fps->setChecked(false);
	ui.action30_fps->setChecked(true);
	ui.action60_fps->setChecked(false);
	ui.actionUnlimited->setChecked(false);

	ui.glCanvas->setFps(30);
}

// action for 60 fps
void MainWindow::action60_fps()
{
	ui.action15_fps->setChecked(false);
	ui.action30_fps->setChecked(false);
	ui.action60_fps->setChecked(true);
	ui.actionUnlimited->setChecked(false);

	ui.glCanvas->setFps(60);
}

// action for 10000 (unlimited) fps
void MainWindow::actionUnlimited()
{
	ui.action15_fps->setChecked(false);
	ui.action30_fps->setChecked(false);
	ui.action60_fps->setChecked(false);
	ui.actionUnlimited->setChecked(true);

	ui.glCanvas->setFps(10000);
}

// key action to configure resolution
void MainWindow::actionResolution()
{
	switch (orientation())
	{
	case ePortrait:
	case ePortraitUpsideDown:
		ui.glCanvas->setScale(scale());
		ui.glCanvas->setFixedSize(hardwareWidth()/deviceScale(), hardwareHeight()/deviceScale());
		ui.glCanvas->setResolution(hardwareWidth(), hardwareHeight());
		break;

	case eLandscapeLeft:
	case eLandscapeRight:
		ui.glCanvas->setScale(scale());
		ui.glCanvas->setFixedSize(hardwareHeight()/deviceScale(), hardwareWidth()/deviceScale());
		ui.glCanvas->setResolution(hardwareWidth(), hardwareHeight());
		break;
	}
}

// handler to emmit project name changed event
void MainWindow::projectNameChanged(const QString& projectName)
{
	if (projectName.isEmpty() == true)
		setWindowTitle("Gideros Player");
	else
		setWindowTitle(projectName + " - Gideros Player");
}

// action to make full screen support
void MainWindow::actionFull_Screen(bool checked)
{
    if(checked)
        this->showFullScreen();
    else
        this->showNormal();
}

// action to hide top menu
void MainWindow::actionHide_Menu(bool checked)
{
    if (checked)
        ui.menuBar->hide();

    else
        ui.menuBar->show();

    actionResolution();
}

// send run shortcut (ctrl+r)
void MainWindow::sendRun()
{
	ui.glCanvas->sendRun();
}

// on resize handler
void MainWindow::resizeEvent(QResizeEvent*)
{
    actionAuto_Scale(ui.actionAuto_Scale->isChecked());
}

//on close of player event, save some settings
void MainWindow::closeEvent(QCloseEvent*)
{
    saveSettings();
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
} */
