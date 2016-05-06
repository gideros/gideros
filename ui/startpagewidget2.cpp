#include "startpagewidget2.h"
#include "ui_startpagewidget2.h"
#include <QCloseEvent>
#include <QMdiArea>
#include <QUrl>
#include <QDesktopServices>
#include <QDir>

StartPageWidget2::StartPageWidget2(QWidget *parent) :
	MdiSubWindow(parent),
    ui(new Ui::StartPageWidget2)
{
    ui->setupUi(this);
	setAttribute(Qt::WA_DeleteOnClose);

	connect(ui->exampleProjects, SIGNAL(selected(int)), ui->recentProjects, SLOT(clearSelection()));
	connect(ui->recentProjects, SIGNAL(selected(int)), ui->exampleProjects, SLOT(clearSelection()));

	connect(ui->recentProjects, SIGNAL(openProject(const QString&)), this, SIGNAL(openProject(const QString&)));
	connect(ui->exampleProjects, SIGNAL(openProject(const QString&)), this, SIGNAL(openProject(const QString&)));

	connect(ui->createNewProject, SIGNAL(clicked()), this, SIGNAL(newProject()));
	connect(ui->gettingStarted, SIGNAL(clicked()), this, SLOT(gettingStarted()));
	connect(ui->referenceManual, SIGNAL(clicked()), this, SLOT(referenceManual()));
}

StartPageWidget2::~StartPageWidget2()
{
    delete ui;
}

void StartPageWidget2::closeEvent(QCloseEvent * event)
{
/*	event->ignore();

	if (mdiArea())
	{
		hide();
		mdiArea()->removeSubWindow(this);
		hide();
	} */
}

void StartPageWidget2::gettingStarted()
{
#if defined(Q_OS_MAC)
    QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::current().filePath("../../Documentation/index.html")));
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::current().filePath("Documentation/index.html")));
#endif
}

void StartPageWidget2::referenceManual()
{
#if defined(Q_OS_MAC)
    QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::current().filePath("../../Documentation/reference.html")));
#else
    QDesktopServices::openUrl(QUrl::fromLocalFile(QDir::current().filePath("Documentation/reference.html")));
#endif
}
