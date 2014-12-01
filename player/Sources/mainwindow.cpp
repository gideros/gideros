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

#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	ui.setupUi(this);

	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));
    //connect(ui.actionExport_Accessed_Files, SIGNAL(triggered()), this, SLOT(exportAccessedFiles()));

	connect(ui.actionRun, SIGNAL(triggered()), this, SLOT(sendRun()));

	connect(ui.actionRotate_Left, SIGNAL(triggered()), this, SLOT(rotateLeft()));
	connect(ui.actionRotate_Right, SIGNAL(triggered()), this, SLOT(rotateRight()));
	connect(ui.actionPortrait, SIGNAL(triggered()), this, SLOT(portrait()));
	connect(ui.actionPortrait_Upside_Down, SIGNAL(triggered()), this, SLOT(portraitUpsideDown()));
	connect(ui.actionLandscape_Left, SIGNAL(triggered()), this, SLOT(landscapeLeft()));
	connect(ui.actionLandscape_Right, SIGNAL(triggered()), this, SLOT(landscapeRight()));

	connect(ui.action15_fps, SIGNAL(triggered()), this, SLOT(action15_fps()));
	connect(ui.action30_fps, SIGNAL(triggered()), this, SLOT(action30_fps()));
	connect(ui.action60_fps, SIGNAL(triggered()), this, SLOT(action60_fps()));
	connect(ui.actionUnlimited, SIGNAL(triggered()), this, SLOT(actionUnlimited()));

	connect(ui.action320x480, SIGNAL(triggered()), this, SLOT(actionResolution()));
	connect(ui.action768x1024, SIGNAL(triggered()), this, SLOT(actionResolution()));
	connect(ui.action640x960, SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action1536x2048, SIGNAL(triggered()), this, SLOT(actionResolution()));
	connect(ui.action320x568, SIGNAL(triggered()), this, SLOT(actionResolution()));
	connect(ui.action640x1136, SIGNAL(triggered()), this, SLOT(actionResolution()));
	connect(ui.action480x800, SIGNAL(triggered()), this, SLOT(actionResolution()));
	connect(ui.action240x320, SIGNAL(triggered()), this, SLOT(actionResolution()));
	connect(ui.action540x960, SIGNAL(triggered()), this, SLOT(actionResolution()));
	connect(ui.action480x854, SIGNAL(triggered()), this, SLOT(actionResolution()));
	connect(ui.action240x400, SIGNAL(triggered()), this, SLOT(actionResolution()));
	connect(ui.action360x640, SIGNAL(triggered()), this, SLOT(actionResolution()));
	connect(ui.action800x1280, SIGNAL(triggered()), this, SLOT(actionResolution()));
	connect(ui.action600x1024, SIGNAL(triggered()), this, SLOT(actionResolution()));
	connect(ui.action600x800, SIGNAL(triggered()), this, SLOT(actionResolution()));
	connect(ui.action768x1366, SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action720x1280, SIGNAL(triggered()), this, SLOT(actionResolution()));
    connect(ui.action1080x1920, SIGNAL(triggered()), this, SLOT(actionResolution()));


    connect(ui.actionQuarter, SIGNAL(triggered()), this, SLOT(actionScale()));
	connect(ui.actionHalf, SIGNAL(triggered()), this, SLOT(actionScale()));
	connect(ui.actionOriginal, SIGNAL(triggered()), this, SLOT(actionScale()));

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

	zoomGroup_ = new QActionGroup(this);
    zoomGroup_->addAction(ui.actionQuarter);
	zoomGroup_->addAction(ui.actionHalf);
	zoomGroup_->addAction(ui.actionOriginal);

	connect(ui.actionAlways_on_Top, SIGNAL(triggered(bool)), this, SLOT(alwaysOnTop(bool)));

	QSettings settings;

	QPoint pos = settings.value("pos", QPoint(50, 50)).toPoint();
	QSize size = settings.value("size", QSize(1, 1)).toSize();
	resize(size);
	move(pos);

	bool alwaysOnTop = settings.value("alwaysOnTop").toBool();
	if (alwaysOnTop)
	{
		setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
		show();
	}

	ui.actionAlways_on_Top->setChecked(alwaysOnTop);

	QTimer::singleShot(0, this, SLOT(afterInitialization()));

	int resolution = settings.value("resolution", 320).toInt();
	switch (resolution)
	{
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
    case 1080:
        ui.action1080x1920->setChecked(true);
        break;
    }

    switch (settings.value("scale", 1).toInt())
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

	ui.glCanvas->setScale(scale());
	ui.glCanvas->setFixedSize(hardwareWidth()/scale(), hardwareHeight()/scale());
	ui.glCanvas->setResolution(hardwareWidth(), hardwareHeight());

	Orientation orientation = static_cast<Orientation>(settings.value("orientation", ePortrait).toInt());
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

	int fps = settings.value("fps2", 60).toInt();
	if (fps == 15)
		action15_fps();
	else if (fps == 30)
		action30_fps();
	else if (fps == 60)
		action60_fps();
	else
		actionUnlimited();

	connect(ui.glCanvas, SIGNAL(projectNameChanged(const QString&)), this, SLOT(projectNameChanged(const QString&)));
}

MainWindow::~MainWindow()
{
}


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

void MainWindow::portrait()
{
	ui.glCanvas->setFixedSize(hardwareWidth()/scale(), hardwareHeight()/scale());
	ui.glCanvas->setHardwareOrientation(ePortrait);
}

void MainWindow::portraitUpsideDown()
{
	ui.glCanvas->setFixedSize(hardwareWidth()/scale(), hardwareHeight()/scale());
	ui.glCanvas->setHardwareOrientation(ePortraitUpsideDown);
}

void MainWindow::landscapeLeft()
{
	ui.glCanvas->setFixedSize(hardwareHeight()/scale(), hardwareWidth()/scale());
	ui.glCanvas->setHardwareOrientation(eLandscapeLeft);
}

void MainWindow::landscapeRight()
{
	ui.glCanvas->setFixedSize(hardwareHeight()/scale(), hardwareWidth()/scale());
	ui.glCanvas->setHardwareOrientation(eLandscapeRight);
}

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

void MainWindow::alwaysOnTop(bool checked)
{
	// TODO: burada mac ve X11 icin extra seyler yapmak lazim
	// keywords: Qt::Tool, Qt::WA_MacAlwaysShowToolWindow
	if (checked)
	{
		setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
	}
	else 
	{
		setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
	}

	show();
}


void MainWindow::closeEvent(QCloseEvent* event)
{
	QSettings settings;
	settings.setValue("pos", pos());
	settings.setValue("size", size());
	settings.setValue("orientation", orientation());
	settings.setValue("alwaysOnTop", ui.actionAlways_on_Top->isChecked());

	if (ui.action15_fps->isChecked())
		settings.setValue("fps2", 15);
	else if (ui.action30_fps->isChecked())
		settings.setValue("fps2", 30);
	else if (ui.action60_fps->isChecked())
		settings.setValue("fps2", 60);
	else
		settings.setValue("fps2", 10000);

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
    else if (resolutionGroup_->checkedAction() == ui.action1080x1920)
        settings.setValue("resolution", 1080);
    else
		settings.setValue("resolution", 320);

    if (ui.actionQuarter->isChecked())
        settings.setValue("scale", 4);
    else if (ui.actionHalf->isChecked())
        settings.setValue("scale", 2);
	else
		settings.setValue("scale", 1);
}


void MainWindow::afterInitialization()
{

}

void MainWindow::action15_fps()
{
	ui.action15_fps->setChecked(true);
	ui.action30_fps->setChecked(false);
	ui.action60_fps->setChecked(false);
	ui.actionUnlimited->setChecked(false);

	ui.glCanvas->setFps(15);
}

void MainWindow::action30_fps()
{
	ui.action15_fps->setChecked(false);
	ui.action30_fps->setChecked(true);
	ui.action60_fps->setChecked(false);
	ui.actionUnlimited->setChecked(false);

	ui.glCanvas->setFps(30);
}

void MainWindow::action60_fps()
{
	ui.action15_fps->setChecked(false);
	ui.action30_fps->setChecked(false);
	ui.action60_fps->setChecked(true);
	ui.actionUnlimited->setChecked(false);

	ui.glCanvas->setFps(60);
}

void MainWindow::actionUnlimited()
{
	ui.action15_fps->setChecked(false);
	ui.action30_fps->setChecked(false);
	ui.action60_fps->setChecked(false);
	ui.actionUnlimited->setChecked(true);

	ui.glCanvas->setFps(10000);
}

void MainWindow::actionResolution()
{
	switch (orientation())
	{
	case ePortrait:
	case ePortraitUpsideDown:
		ui.glCanvas->setScale(scale());
		ui.glCanvas->setFixedSize(hardwareWidth()/scale(), hardwareHeight()/scale());
		ui.glCanvas->setResolution(hardwareWidth(), hardwareHeight());
		break;

	case eLandscapeLeft:
	case eLandscapeRight:
		ui.glCanvas->setScale(scale());
		ui.glCanvas->setFixedSize(hardwareHeight()/scale(), hardwareWidth()/scale());
		ui.glCanvas->setResolution(hardwareWidth(), hardwareHeight());
		break;
	}
}

int MainWindow::hardwareWidth()
{
	if (resolutionGroup_->checkedAction() == ui.action320x480)
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
    else if (resolutionGroup_->checkedAction() == ui.action1080x1920)
        return 1080;


	return 320;
}

int MainWindow::hardwareHeight()
{
	if (resolutionGroup_->checkedAction() == ui.action320x480)
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
    else if (resolutionGroup_->checkedAction() == ui.action1080x1920)
        return 1920;

    return 480;
}

int MainWindow::scale()
{
    if (ui.actionQuarter->isChecked())
        return 4;

    if (ui.actionHalf->isChecked())
		return 2;

	return 1;
}

void MainWindow::projectNameChanged(const QString& projectName)
{
	if (projectName.isEmpty() == true)
		setWindowTitle("Gideros Player");
	else
		setWindowTitle(projectName + " - Gideros Player");
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

void MainWindow::sendRun()
{
	ui.glCanvas->sendRun();
}

void MainWindow::actionScale()
{
	ui.glCanvas->setScale(scale());
	actionResolution();
}
