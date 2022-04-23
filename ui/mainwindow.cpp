#include "mainwindow.h"
#include <QDockWidget>
#include "librarywidget.h"
#include "outlinewidget.h"
#include <QAction>
#include <QIcon>
#include <QPixmap>
#include <QSet>
#include <QString>
#include <QTextStream>
#include "iconlibrary.h"
#include <stack>
//#include "libnetwork.h"
#include <QProgressDialog>
#include <QFile>
#include <string>
#include <QTextEdit>
#include "newprojectdialog.h"
#include <QDir>
#include <QDirIterator>
#include <QMessageBox>
#include <QTextStream>
#include <QFileDialog>
#include <QSettings>
#include <QProcess>
#include <QMdiArea>
#include <QDateTime>
#include "fileassociationsdialog.h"
#include "textedit.h"
#include <QTextBlock>
#include <QCloseEvent>
#include "savechangesdialog.h"
#include "playersettingsdialog.h"
#include "finddialog.h"
#include "replacedialog.h"
#include "gotolinedialog.h"
#include "giderosnetworkclient2.h"
#include <previewwidget.h>
#include <QProgressDialog>
#include <QThread>
#include <QTextStream>
#include <bytebuffer.h>
#include <platformutil.h>
#include "findinfilesdialog.h"
#include <QDesktopServices>
#include <QUrl>
#include "exportprojectdialog.h"
#include "exportprogress.h"
#include "startpagewidget2.h"
#include "aboutdialog.h"
#include <QSplitter>
#include <QImage>
#include "mdiarea.h"
#include <QToolBar>
#include <QKeySequence>
#include "addons.h"
#include <QToolTip>
#include <QRegularExpression>
#include "preferencesdialog.h"
#include "profilerreport.h"
#include <QStandardPaths>
#include <QTimer>

#include <QInputDialog>

MainWindow *MainWindow::lua_instance=NULL;
QTemporaryDir *MainWindow::tempDir=NULL;

static int ltw_notifyClient(lua_State *L) {
    if (!MainWindow::lua_instance) return 0;
    const char *cid=luaL_optstring(L,1,NULL);
    const char *data=luaL_checkstring(L,2);
    QString clientId;
    if (cid) clientId=QString(cid);
    MainWindow::lua_instance->notifyAddon(clientId,data);
    return 0;
}

static int ltw_saveAll(lua_State *L) {
    Q_UNUSED(L);
    if (!MainWindow::lua_instance) return 0;
    MainWindow::lua_instance->saveAll();
    return 0;
}

void MainWindow::notifyAddon(QString clientId,const char *data) {
    if (addonsServer_)
        addonsServer_->notify(clientId,data);
}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    inChooseTab_ = false;
    changeTabKeyPressState_ = 0;
    tabListWidget_ = NULL;

	ui.setupUi(this);

    tempDir=new QTemporaryDir();

	mdiArea_ = new MdiArea(this);
	
	setTabPosition(Qt::AllDockWidgetAreas, QTabWidget::North);
	
    // Load the theme at startup
    QSettings settings;

    QString themePath = QDir::currentPath()+"/Resources/Themes/";
    QDir dir(themePath);


    if (!dir.exists())
    {
        dir.mkdir(themePath);
    }
    else
    {
        QString themeFile = settings.value("uiTheme").toString();
        QFile file(themeFile);
        QString theme;
        if (file.open(QIODevice::ReadOnly|QIODevice::Text))
        {
            QTextStream in(&file);
            while (!in.atEnd())
            {
            theme = in.readAll();
            }

            qApp->setStyleSheet(theme);
        }
    }

/*	mdiArea_->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mdiArea_->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	mdiArea_->setViewMode(QMdiArea::TabbedView);
	mdiArea_->setDocumentMode(true); */

//	mdiArea_->setTabShape(QTabWidget::Triangular);

	Q_FOREACH (QTabBar* tab, mdiArea_->findChildren<QTabBar *>())
	{
		tab->setTabsClosable(true);
		tab->setExpanding(false);
		tab->setDocumentMode(true);
		tab->setMovable(true);
		connect(tab, SIGNAL(tabCloseRequested(int)),
				this, SLOT(closeMdiTab(int)));
	}
	
	// set toolbar position by settings value
	//qDebug() << "toggleToolBar value:" << settings.value("toggleToolBar").toBool();
	toggleToolBar(settings.value("toggleToolBar").toBool());
	
	ui.mainToolBar->setIconSize(QSize(16, 16));

	ui.actionNew->setIcon(IconLibrary::instance().icon(0, "new"));
	ui.actionOpen->setIcon(IconLibrary::instance().icon(0, "open"));

	ui.actionSave->setIcon(IconLibrary::instance().icon(0, "save"));
	connect(ui.actionSave, SIGNAL(triggered()), this, SLOT(save()));

	ui.actionSave_All->setIcon(IconLibrary::instance().icon(0, "save all"));
	connect(ui.actionSave_All, SIGNAL(triggered()), this, SLOT(saveAll()));

    ui.actionExport_Project->setIcon(IconLibrary::instance().icon(0, "export"));

    connect(ui.actionClone_Project, SIGNAL(triggered()), this, SLOT(cloneProject()));
    connect(ui.actionConsolidate_Project, SIGNAL(triggered()), this, SLOT(consolidateProject()));

	ui.actionUndo->setIcon(IconLibrary::instance().icon(0, "undo"));
	ui.actionRedo->setIcon(IconLibrary::instance().icon(0, "redo"));

	ui.actionCut->setIcon(IconLibrary::instance().icon(0, "cut"));
	ui.actionCopy->setIcon(IconLibrary::instance().icon(0, "copy"));
	ui.actionPaste->setIcon(IconLibrary::instance().icon(0, "paste"));

	ui.actionToggle_Bookmark->setIcon(IconLibrary::instance().icon(0, "toggle bookmark"));
	ui.actionNext_Bookmark->setIcon(IconLibrary::instance().icon(0, "next bookmark"));
	ui.actionPrevious_Bookmark->setIcon(IconLibrary::instance().icon(0, "previous bookmark"));
	ui.actionClear_Bookmarks->setIcon(IconLibrary::instance().icon(0, "clear bookmarks"));

	ui.actionStart_Player->setIcon(IconLibrary::instance().icon(0, "gamepad"));
	connect(ui.actionStart_Player, SIGNAL(triggered()), this, SLOT(startPlayer()));
	
	connect(ui.actionTexture_Packer, SIGNAL(triggered()), this, SLOT(startTexturePacker()));
	connect(ui.actionFont_Creator, SIGNAL(triggered()), this, SLOT(startFontCreator()));
	
	ui.actionWrap->setIcon(IconLibrary::instance().icon(0, "wrap"));
	connect(ui.actionWrap, SIGNAL(triggered()), this, SLOT(setWrap()));
	
	ui.actionColorPicker->setIcon(IconLibrary::instance().icon(0, "color picker"));
	connect(ui.actionColorPicker, SIGNAL(triggered()), this, SLOT(insertColorInCode()));

	ui.actionStart->setIcon(IconLibrary::instance().icon(0, "start"));
	ui.actionStart->setEnabled(false);
	ui.actionStart->setShortcuts( QList<QKeySequence>() << tr("Ctrl+R") << tr("F5") );
	connect(ui.actionStart, SIGNAL(triggered()), this, SLOT(start()));

	ui.actionDebug->setIcon(IconLibrary::instance().icon(0, "debug"));
	ui.actionDebug->setEnabled(false);
	connect(ui.actionDebug, SIGNAL(triggered()), this, SLOT(startDebug()));

    ui.actionResume->setIcon(IconLibrary::instance().icon(0, "resume"));
    ui.actionResume->setEnabled(false);
    connect(ui.actionResume, SIGNAL(triggered()), this, SLOT(resume()));

    ui.actionStepOver->setIcon(IconLibrary::instance().icon(0, "step over"));
    ui.actionStepOver->setEnabled(false);
    connect(ui.actionStepOver, SIGNAL(triggered()), this, SLOT(stepOver()));

    ui.actionStepInto->setIcon(IconLibrary::instance().icon(0, "step into"));
    ui.actionStepInto->setEnabled(false);
    connect(ui.actionStepInto, SIGNAL(triggered()), this, SLOT(stepInto()));

    ui.actionStepReturn->setIcon(IconLibrary::instance().icon(0, "step return"));
    ui.actionStepReturn->setEnabled(false);
    connect(ui.actionStepReturn, SIGNAL(triggered()), this, SLOT(stepReturn()));

	ui.actionStartAll->setIcon(IconLibrary::instance().icon(0, "start all"));
	ui.actionStartAll->setEnabled(true);
	connect(ui.actionStartAll, SIGNAL(triggered()), this, SLOT(startAllPlayers()));

	ui.actionStop->setIcon(IconLibrary::instance().icon(0, "stop"));
	ui.actionStop->setEnabled(false);
	connect(ui.actionStop, SIGNAL(triggered()), this, SLOT(stop()));

    ui.actionProfile->setIcon(IconLibrary::instance().icon("profiler",QStringList()));
    ui.actionProfile->setEnabled(false);
    connect(ui.actionProfile, SIGNAL(triggered()), this, SLOT(startProfile()));

    connect(ui.actionCheck_Syntax, SIGNAL(triggered()), this, SLOT(compile()));
    connect(ui.actionCheck_Syntax_All, SIGNAL(triggered()), this, SLOT(compileAll()));
    connect(ui.actionClear_Output, SIGNAL(triggered()), this, SLOT(clearOutput()));
	connect(ui.actionCancel, SIGNAL(triggered()), this, SLOT(cancel()));

#if 0
	setCentralWidget(mdiArea_);

	{
		libraryDock_ = new QDockWidget(tr("Library[*]"), this);
		libraryDock_->setObjectName("library");
		libraryWidget_ = new LibraryWidget(libraryDock_);
		libraryDock_->setWidget(libraryWidget_);
		addDockWidget(Qt::RightDockWidgetArea, libraryDock_);
		connect(libraryWidget_, SIGNAL(modificationChanged(bool)), libraryDock_, SLOT(setWindowModified(bool)));
	}

	{
		outputDock_ = new QDockWidget(tr("Output"), this);
        outputDock->setAllowedAreas(Qt::BottomDockWidgetArea);
        outputDock->setFeatures(DockWidgetFloatable);
        outputDock->setFloating(true);
		outputDock_->setObjectName("output");
		outputWidget_ = new QTextEditEx(outputDock_);
		connect(outputWidget_, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(outputMouseDoubleClick(QMouseEvent*)));
		outputWidget_->setReadOnly(true);
		outputDock_->setWidget(outputWidget_);
		addDockWidget(Qt::BottomDockWidgetArea, outputDock_);
	}


	{
		previewDock_ = new QDockWidget(tr("Preview"), this);
		previewDock_->setObjectName("preview");
		previewWidget_ = new PreviewWidget(previewDock_);
		previewDock_->setWidget(previewWidget_);
		addDockWidget(Qt::RightDockWidgetArea, previewDock_);
	}
#else

	libraryDock_ = NULL;
	outputDock_ = NULL;
	previewDock_ = NULL;

	libraryWidget_ = new LibraryWidget;
	//connect(libraryWidget_, SIGNAL(modificationChanged(bool)), libraryDock_, SLOT(setWindowModified(bool))); TODO

    outputWidget_ = new QTextEditEx;
	outputWidget_->setReadOnly(true);
	connect(outputWidget_, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(outputMouseDoubleClick(QMouseEvent*)));


	QWidget* outputContainer;
	{
		outputContainer = new QWidget;
		outputContainer->setLayout(new QVBoxLayout);
        outputContainer->layout()->setContentsMargins(0,0,0,0);
		outputContainer->layout()->setSpacing(0);

        QLineEdit *edit = new QLineEdit();
        edit->setPlaceholderText("Search output");
        outputContainer->layout()->addWidget(edit);
		outputContainer->layout()->addWidget(outputWidget_);

        connect(edit,SIGNAL(textEdited( const QString & )),this,SLOT(searchOutput( const QString & )));

		outputWidget_->setLineWidth(0);
		outputWidget_->setMidLineWidth(0);
		outputWidget_->setFrameShape(QFrame::NoFrame);
		outputWidget_->setFrameShadow(QFrame::Plain);
    }

    outputDock_ = new QDockWidget(tr("Output"), this);
    outputDock_->setAllowedAreas(Qt::BottomDockWidgetArea);
    outputDock_->setObjectName("output");
    outputDock_->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    outputDock_->setWidget(outputContainer);
    addDockWidget(Qt::BottomDockWidgetArea,outputDock_);

	QDockWidget *dock;
	previewWidget_ = new PreviewWidget;
	dock = new QDockWidget(tr("Preview"), this);
	dock->setAllowedAreas(Qt::RightDockWidgetArea|Qt::LeftDockWidgetArea);
	dock->setObjectName("preview");
	dock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
    dock->setWidget(previewWidget_);
	addDockWidget(Qt::LeftDockWidgetArea,dock);

	dock = new QDockWidget(tr("Library"), this);
	dock->setAllowedAreas(Qt::RightDockWidgetArea|Qt::LeftDockWidgetArea);
	dock->setObjectName("library");
	dock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
	dock->setWidget(libraryWidget_);
	addDockWidget(Qt::LeftDockWidgetArea,dock);

	outlineWidget_=new OutlineWidget();
	dock = new QDockWidget(tr("Outline"), this);
	dock->setAllowedAreas(Qt::RightDockWidgetArea|Qt::LeftDockWidgetArea);
	dock->setObjectName("outline");
	dock->setFeatures(QDockWidget::DockWidgetFloatable|QDockWidget::DockWidgetMovable);
	dock->setWidget(outlineWidget_);
    outlineDock_=dock;
	addDockWidget(Qt::RightDockWidgetArea,dock);

	//splitter3_->setSizes(QList<int>() << 200 << 200);

	setCentralWidget(mdiArea_);
#endif

	//Target player combobox
	players_ = new QComboBox;
	players_->setSizeAdjustPolicy(QComboBox::AdjustToContents);
	ui.mainToolBar->insertWidget(ui.actionStart_Player,players_);
    connect(players_,SIGNAL(currentIndexChanged(int)),this,SLOT(playerChanged(int)));

    //QSettings settings;
	QString playerip = settings.value("player ip", QString("127.0.0.1")).toString();
    ui.actionLocalhostToggle->setChecked(settings.value("player localhost", true).toBool());
    ui.actionLive_syntax_checking->setChecked(settings.value("syntaxcheck_live",true).toBool());
    ui.actionType_checking->setChecked(settings.value("typecheck_live",false).toBool());

#ifndef NEW_CLIENT
	client_ = new Client(qPrintable(playerip), 15000);
#else
	client_ = new GiderosNetworkClient(playerip, 15000);

	connect(client_, SIGNAL(connected()), this, SLOT(connected()));
	connect(client_, SIGNAL(disconnected()), this, SLOT(disconnected()));
	connect(client_, SIGNAL(dataReceived(const QByteArray&)), this, SLOT(dataReceived(const QByteArray&)));
	connect(client_, SIGNAL(ackReceived(unsigned int)), this, SLOT(ackReceived(unsigned int)));
    connect(client_, SIGNAL(advertisement(const QString&,unsigned short,unsigned short,const QString&)), this, SLOT(advertisement(const QString&,unsigned short,unsigned short,const QString&)));
#endif

//	startTimer(1);

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
    timer->start(30);

	QTimer::singleShot(1, this, SLOT(onSingleShot())); 

	connect(ui.actionNew_Project, SIGNAL(triggered()), this, SLOT(newProject()));
	connect(ui.actionClose_Project, SIGNAL(triggered()), this, SLOT(closeProject()));
	connect(ui.actionClose_Project, SIGNAL(triggered()), this, SLOT(showStartPage()));
	connect(ui.actionSave_Project, SIGNAL(triggered()), this, SLOT(saveProject()));
	connect(ui.actionOpen_Project, SIGNAL(triggered()), this, SLOT(openProject()));
	connect(ui.actionFile_Associations, SIGNAL(triggered()), this, SLOT(fileAssociations()));
	connect(ui.actionExport_Project, SIGNAL(triggered()), this, SLOT(exportProject()));
	connect(ui.actionExport_Pack, SIGNAL(triggered()), this, SLOT(exportPack()));

//	connect(libraryWidget_, SIGNAL(modified()), this, SLOT(onModified()));
	connect(libraryWidget_, SIGNAL(openRequest(const QString&, const QString&)), this, SLOT(onOpenRequest(const QString&, const QString&)));
	connect(libraryWidget_, SIGNAL(previewRequest(const QString&, const QString&)), this, SLOT(onPreviewRequest(const QString&, const QString&)));
	connect(libraryWidget_, SIGNAL(insertIntoDocument(const QString&)), this, SLOT(onInsertIntoDocument(const QString&)));
	connect(libraryWidget_, SIGNAL(automaticDownsizingEnabled(const QString&)), this, SLOT(downsize(const QString&)));

#if 0
	compileThread_ = NULL;
#endif

	connect(ui.actionProject1, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject2, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject3, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject4, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject5, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject6, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject7, SIGNAL(triggered()), this, SLOT(openRecentProject()));
    connect(ui.actionProject8, SIGNAL(triggered()), this, SLOT(openRecentProject()));
    connect(ui.actionProject9, SIGNAL(triggered()), this, SLOT(openRecentProject()));
    connect(ui.actionProject10, SIGNAL(triggered()), this, SLOT(openRecentProject()));
    connect(ui.actionProject11, SIGNAL(triggered()), this, SLOT(openRecentProject()));
    connect(ui.actionProject12, SIGNAL(triggered()), this, SLOT(openRecentProject()));
    connect(ui.actionProject13, SIGNAL(triggered()), this, SLOT(openRecentProject()));
    connect(ui.actionProject14, SIGNAL(triggered()), this, SLOT(openRecentProject()));
    connect(ui.actionProject15, SIGNAL(triggered()), this, SLOT(openRecentProject()));

	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));

    connect(ui.actionFullscreen, SIGNAL(triggered()), this, SLOT(toggleFullscreen()));

    connect(ui.actionPlayer_Settings, SIGNAL(triggered()), this, SLOT(playerSettings()));
    connect(ui.actionLocalhostToggle, SIGNAL(triggered(bool)), this, SLOT(actionLocalhostToggle(bool)));
    connect(ui.actionLive_syntax_checking, SIGNAL(triggered(bool)), this, SLOT(actionLiveSyntaxChecking(bool)));
    connect(ui.actionType_checking, SIGNAL(triggered(bool)), this, SLOT(actionLiveTypeChecking(bool)));
    connect(ui.actionAbout_Gideros_Studio, SIGNAL(triggered()), this, SLOT(openAboutDialog()));
    connect(ui.actionPreferences, SIGNAL(triggered()), this, SLOT(openPreferencesDialog()));
	connect(ui.actionHelp_Support, SIGNAL(triggered()), this, SLOT(helpAndSupport()));
    connect(ui.actionDocumentation, SIGNAL(triggered()), this, SLOT(giderosDocumentation()));

	connect(mdiArea_, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(updateUI()));
	connect(mdiArea_, SIGNAL(subWindowActivated(MyMdiSubWindow*)), this, SLOT(updateUI()));
	updateUI();
	setWindowTitle(tr("Gideros"));

	updateRecentProjectActions();

	// find
	{
		findDialog_ = new FindDialog(this);
		connect(ui.actionFind, SIGNAL(triggered()), this, SLOT(find()));
		connect(findDialog_, SIGNAL(findNext()), this, SLOT(findFirst()));
		connect(findDialog_, SIGNAL(findPrevious()), this, SLOT(findPrevious()));
		connect(ui.actionFind_Next, SIGNAL(triggered()), this, SLOT(findNext()));
		connect(ui.actionFind_Previous, SIGNAL(triggered()), this, SLOT(findPrevious()));

		replaceDialog_ = new ReplaceDialog(this);
		connect(ui.actionReplace, SIGNAL(triggered()), this, SLOT(replace()));

		connect(replaceDialog_, SIGNAL(findNext()), this, SLOT(replace_findNext()));
		connect(replaceDialog_, SIGNAL(replace()), this, SLOT(replace_replace()));
		connect(replaceDialog_, SIGNAL(replaceAll()), this, SLOT(replace_replaceAll()));

		findInFilesDialog_ = new FindInFilesDialog(this);
		connect(ui.actionFind_in_Files, SIGNAL(triggered()), this, SLOT(findInFiles()));
	}
	
	// bookmarks
	{
		connect(ui.actionToggle_Bookmark, SIGNAL(triggered()), this, SLOT(toogleBookmark()));
		connect(ui.actionNext_Bookmark, SIGNAL(triggered()), this, SLOT(nextBookmark()));
		connect(ui.actionPrevious_Bookmark, SIGNAL(triggered()), this, SLOT(previousBookmark()));
		connect(ui.actionClear_Bookmarks, SIGNAL(triggered()), this, SLOT(clearBookmarks()));
	}

	// undo-redo-indent
	{
		connect(ui.actionUndo, SIGNAL(triggered()), this, SLOT(undo()));
		connect(ui.actionRedo, SIGNAL(triggered()), this, SLOT(redo()));
		connect(ui.actionAuto_indent, SIGNAL(triggered()), this, SLOT(autoIndent()));
	}

	connect(ui.actionGo_To_Line, SIGNAL(triggered()), this, SLOT(goToLine()));

/*    splitter1_->setSizes(QList<int>() << 550 << 200);
    splitter2_->setSizes(QList<int>() << 200 << 800);
*/
	{
		QSettings settings;
		restoreGeometry(settings.value("geometry").toByteArray());
		restoreState(settings.value("windowState").toByteArray());

	/*	splitter1_->restoreState(settings.value("splitter1").toByteArray());
		splitter2_->restoreState(settings.value("splitter2").toByteArray());
		splitter3_->restoreState(settings.value("splitter3").toByteArray());*/
	}

	connect(ui.actionOutput_Panel, SIGNAL(triggered()), outputDock_, SLOT(show()));
	connect(ui.actionLibrary_Manager, SIGNAL(triggered()), libraryDock_, SLOT(show()));
	connect(ui.actionPreview, SIGNAL(triggered()), previewDock_, SLOT(show()));
//	connect(ui.actionStart_Page, SIGNAL(triggered()), this, SLOT(showStartPage()));

	QList<QVariant> v = settings.value("fileAssociations", QList<QVariant>()).toList();
	for (int i = 0; i < v.size(); ++i)
		fileAssociations_.push_back(v[i].toStringList());

	isTransferring_ = false;

    makeProcess_ = new QProcess(this);

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    QDir toolsDir = QDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_MAC)
    toolsDir.cdUp();
#endif
    toolsDir.cd("Tools");

#if defined(Q_OS_WIN)
    env.insert("PATH", toolsDir.path() + ";" + env.value("PATH"));
#else
    env.insert("PATH", toolsDir.path() + ":" + env.value("PATH"));
#endif

    makeProcess_->setProcessEnvironment(env);

    connect(makeProcess_, SIGNAL(readyReadStandardOutput()), this, SLOT(stdoutToOutput()));
    connect(makeProcess_, SIGNAL(readyReadStandardError()), this, SLOT(stderrToOutput()));
    connect(makeProcess_, SIGNAL(started()), this, SLOT(makeStarted()));
    connect(makeProcess_, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(makeFinished()));

    luaProcess_ = new QProcess(this);
#if defined(Q_OS_MAC)
    env.insert("LUA_CPATH", toolsDir.path() + "/?.so");
#endif
    luaProcess_->setProcessEnvironment(env);

    lua_instance=this;
    //Register addons
    lua_State *L=AddonsManager::getLua();
    luaL_Reg reg[] = {
        { "_notifyClient", ltw_notifyClient },
        { "saveAll", ltw_saveAll },
        { NULL, NULL }
    };
    luaL_register(L,"Studio",reg);
    lua_pop(L,1);

    addonsServer_=new AddonsServer(this);
    std::vector<Addon> addons=AddonsManager::loadAddons(true);
    for (std::vector<Addon>::iterator it=addons.begin();it!=addons.end();it++) {
    	QAction *action=new QAction(QString::fromStdString(it->title),this);
    	action->setData(QString::fromStdString(it->name));
    	ui.menuAddons->addAction(action);
        connect(action, SIGNAL(triggered()), this, SLOT(addonTriggered()));
    }

        isBreaked_=false;

#define ACTION(n) { #n, ui.n }
        struct {
            QString n;
            QAction *a;
        }
        ActionList[]={
            ACTION(actionNew_Project),
               ACTION(actionOpen_Project),
               ACTION(actionClose_Project),
               ACTION(actionSave_Project),
               ACTION(actionFile_Associations),
               ACTION(actionStart),
               ACTION(actionStop),
               ACTION(actionStart_Player),
               ACTION(actionNew),
               ACTION(actionOpen),
               ACTION(actionSave),
               ACTION(actionSave_All),
               ACTION(actionUndo),
               ACTION(actionRedo),
               ACTION(actionCut),
               ACTION(actionCopy),
               ACTION(actionPaste),
               ACTION(actionWrap),
               ACTION(actionColorPicker),
               ACTION(actionToggle_Bookmark),
               ACTION(actionNext_Bookmark),
               ACTION(actionPrevious_Bookmark),
               ACTION(actionClear_Bookmarks),
               ACTION(actionCheck_Syntax),
               ACTION(actionCheck_Syntax_All),
               ACTION(actionCancel),
               ACTION(actionExit),
               ACTION(actionProject1),
               ACTION(actionProject2),
               ACTION(actionProject3),
               ACTION(actionProject4),
               ACTION(actionProject5),
               ACTION(actionPlayer_Settings),
               ACTION(actionFind),
               ACTION(actionReplace),
               ACTION(actionFind_Next),
               ACTION(actionFind_Previous),
               ACTION(actionGo_To_Line),
               ACTION(actionOutput_Panel),
               ACTION(actionLibrary_Manager),
               ACTION(actionExport_Project),
               ACTION(actionFind_in_Files),
               ACTION(actionPreview),
               ACTION(actionStart_Page),
               ACTION(actionAbout_Gideros_Studio),
               ACTION(actionExport_Pack),
               ACTION(actionHelp_Support),
               ACTION(actionProject6),
               ACTION(actionProject7),
               ACTION(actionProject8),
               ACTION(actionProject9),
               ACTION(actionProject10),
               ACTION(actionProject11),
               ACTION(actionProject12),
               ACTION(actionProject13),
               ACTION(actionProject14),
               ACTION(actionProject15),
               ACTION(actionLocalhostToggle),
               ACTION(actionClear_Output),
               ACTION(actionDocumentation),
               ACTION(actionStartAll),
               ACTION(actionUI_Theme),
               ACTION(actionEditor_Theme),
               ACTION(actionUI_and_Editor_Theme),
               ACTION(actionReset_UI_and_Editor_Theme),
               ACTION(actionFold_Unfold_All),
               ACTION(actionFold_Unfold_Top),
               ACTION(actionMacro_Support),
               ACTION(actionAuto_indent),
               ACTION(actionLive_syntax_checking),
               ACTION(actionFullscreen),
               ACTION(actionDebug),
               ACTION(actionStepOver),
               ACTION(actionStepInto),
               ACTION(actionStepReturn),
               ACTION(actionResume),
               ACTION(actionPreferences),
               ACTION(actionProfile),
               ACTION(actionClone_Project),
               ACTION(actionConsolidate_Project),
               ACTION(actionType_checking),
               ACTION(actionBlock_un_comment),
               { QString(),nullptr },
        };
#undef ACTION
        QDir shared(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
        shared.mkpath("Gideros");
        bool sharedOk = shared.cd("Gideros");
        if (sharedOk) {
            QSettings kmap(shared.absoluteFilePath("keymap.ini"), QSettings::IniFormat);
            kmap.beginGroup("main");
            auto alist=ActionList;
            while (!alist->n.isEmpty()) {
                auto v=kmap.value(alist->n);
                if ((!v.isNull())&&alist->a)
                    alist->a->setShortcut(v.toString());
                alist++;
            }
            kmap.endGroup();
        }
}

MainWindow::~MainWindow()
{
	delete addonsServer_;
	delete client_;
    delete tempDir;
}

void MainWindow::addonTriggered() {
	QAction* action = (QAction *) sender();
	QString name=action->data().toString();
	launchAddon(name,QString());
}

static QString luaquote(QString q) {
    return q.replace("\\","\\\\").replace("\"","\\\"");
}
void MainWindow::launchAddon(QString name,QString forFile) {
    QString base = QFileInfo(projectFileName_).path();
	std::string env="{ ";
	env=env+"serverPort="+QString::number(addonsServer_->port()).toStdString()+",";
    env=env+"projectFile=\""+luaquote(projectFileName_).toStdString()+"\",";
    env=env+"projectDir=\""+luaquote(base).toStdString()+"\",";
	if (!forFile.isEmpty())
        env=env+"editFile=\""+luaquote(forFile).toStdString()+"\",";
	env+=" }";
    AddonsManager::launch(name.toStdString(),env);
}


void MainWindow::toggleToolBar(bool append)
{
    QSettings settings;
    const char* key = "toggleToolBar";

	QWidget* wrapper = new QWidget();
	wrapper->setLayout(new QHBoxLayout());
	wrapper->layout()->setContentsMargins(0,0,0,0);
    if (append)
    {
		settings.setValue(key, true);

		wrapper->layout()->addWidget(ui.menuBar);
		wrapper->layout()->addWidget(ui.mainToolBar);
		this->layout()->setMenuBar(wrapper);
    } else {
        settings.setValue(key, false);
		
		removeToolBar(ui.mainToolBar);
		wrapper->layout()->update();
		addToolBar(Qt::TopToolBarArea, ui.mainToolBar);
		ui.mainToolBar->show();
    }
}

void MainWindow::toggleFullscreen()
{
    QSettings settings;
    const char* key = "toggleFullscreen_wasMaximized";
    bool wasMaximized = settings.value(key, false).toBool();
    if (this->isFullScreen())
    {
        if (wasMaximized)
            this->showMaximized();
        else
            this->showNormal();
    } else {
        settings.setValue(key, isMaximized());
        this->showFullScreen();
    }
}

void MainWindow::closeMdiTab(int i)
{
	// asagidaki yontemi
	// https://tora.svn.sourceforge.net/svnroot/tora/trunk/tora/src/tomain.cpp dan bulduk
	MdiSubWindow *sub = mdiArea_->subWindowList()[i];

	if (sub != mdiArea_->activeSubWindow())
		mdiArea_->setActiveSubWindow(sub);

	mdiArea_->closeActiveSubWindow();
}

void MainWindow::onSingleShot()
{
    QStringList args=QCoreApplication::arguments();
    if (args.size()>1)
    	openProject(args[1].toUtf8().data());
    else
    	showStartPage();
}

void MainWindow::showStartPage()
{
	foreach (MdiSubWindow* window, mdiArea_->subWindowList())
	{
		StartPageWidget2* startPage = qobject_cast<StartPageWidget2*>(window);
		if (startPage)
		{
			mdiArea_->setActiveSubWindow(startPage);
			startPage->setFocus(Qt::OtherFocusReason);
			return;
		}
	}

	StartPageWidget2* startPage = new StartPageWidget2;
	connect(startPage, SIGNAL(newProject()), this, SLOT(newProject()));
	connect(startPage, SIGNAL(openProject()), this, SLOT(openProject()));
	connect(startPage, SIGNAL(openProject(const QString&)), this, SLOT(openProject(const QString&)));

//	startPage_->load();
	mdiArea_->addSubWindow(startPage);
	startPage->showMaximized();
}

void MainWindow::hideStartPage()
{
	foreach (MdiSubWindow* window, mdiArea_->subWindowList())
	{
		StartPageWidget2* startPage = qobject_cast<StartPageWidget2*>(window);
		if (startPage)
		{
			if (startPage != mdiArea_->activeSubWindow())
				mdiArea_->setActiveSubWindow(startPage);
			mdiArea_->closeActiveSubWindow();
			break;
		}
	}
}

void MainWindow::advertisement(const QString& host,unsigned short port,unsigned short flags,const QString& name)
{
	QString nitem=QString("%1|%2|%3").arg(host).arg(port).arg(flags);
	time_t ctime=time(NULL);
	QString nfull=QString("%1|%2|%3|%4").arg(host).arg(port).arg(flags).arg(ctime);
	for (int k=0;k<players_->count();k++)
	{
		QStringList parts=players_->itemData(k).toString().split('|');
		if (QString("%1|%2|%3").arg(parts[0]).arg(parts[1]).arg(parts[2])==nitem)
		{
			players_->setItemData(k,nfull);
            if (!client_->isConnected())
                playerChanged(0);
            return;
		}
 	}

	players_->addItem(QString("%1 (%2:%3)").arg(name).arg(host).arg(port),nfull);
    if (!client_->isConnected())
        playerChanged(0);
}

void MainWindow::playerChanged(int)
{
    QString hostData;
	if (players_->currentData().isValid())
        hostData=players_->currentData().toString();
    else
        return;
    QStringList parts=hostData.split('|');
	if (parts.count()==1)
		client_->connectToHost(parts[0],15000);
	else
		client_->connectToHost(parts[0],parts[1].toInt());
	clientIsWeb_=(parts[0]=="[ws]");
}

void MainWindow::start()
{
    if (isBreaked_) resumeDebug(0x00);
    isDebug_=false;
    isProfile_=false;
    if (prepareStartOnPlayer())
		startOnPlayer();
}

void MainWindow::startDebug()
{
    if (isBreaked_) resumeDebug(0x00);
    isDebug_=true;
    isProfile_=false;
    if (prepareStartOnPlayer())
		startOnPlayer();
	else
		isDebug_=false;
}

void MainWindow::startProfile()
{
    if (isBreaked_) resumeDebug(0x00);
    isDebug_=false;
    isProfile_=true;
    if (prepareStartOnPlayer())
        startOnPlayer();
    else
        isProfile_=false;
}


void MainWindow::resume() {
    resumeDebug(0x84); // Mask LINE + BKPT
}

void MainWindow::stepOver() {
    resumeDebug(0x44); //Mask LINE + ignore subs
}

void MainWindow::stepInto() {
    resumeDebug(0x04);    //Mask LINE
}

void MainWindow::stepReturn() {
    resumeDebug(0x46); //Mask LINE  + ignore subs + RET
}

void MainWindow::resumeDebug(int mode) {
    isBreaked_=false;
    clearDebugHighlights();
    ui.actionResume->setEnabled(false);
    ui.actionStepOver->setEnabled(false);
    ui.actionStepInto->setEnabled(false);
    ui.actionStepReturn->setEnabled(false);
    ByteBuffer buffer;
    buffer << (char) 22;
    buffer << (char) mode;
    int nbreakpoints=TextEdit::breakpoints.size();
	buffer << nbreakpoints;
    foreach(QString bp, TextEdit::breakpoints) {
        int ls=bp.lastIndexOf(':');
        std::string source=(QString("@")+bp.mid(0,ls)).toStdString();
        int line=bp.mid(ls+1).toInt()+1;
        buffer << line;
        buffer << source;
    }

    client_->sendCommand(buffer.data(),buffer.size());
}

bool MainWindow::prepareStartOnPlayer()
{
	if (projectFileName_.isEmpty() == true)
		return false;

	saveAll();

	// downsize changed images
	{
		std::vector<std::pair<QString, QString> > fileList = updateMD5(true);
		if (!fileList.empty())
		{
			saveMD5();

			QDir path(QFileInfo(projectFileName_).path());
			for (size_t i = 0; i < fileList.size(); ++i)
			{
				QString filename = fileList[i].second;
				QString absfilename = QDir::cleanPath(path.absoluteFilePath(filename));
				downsize(absfilename);
			}
		}
	}

	if (!updateMD5().empty())
		saveMD5();
	return true;
}

void MainWindow::startOnPlayer()
{
	if (client_->isConnected() == false)
		return;

	if (isTransferring_ == true)
		return;

	isTransferring_ = true;

	client_->sendStop();
	isBreaked_=false;
	client_->sendProjectName(QFileInfo(projectFileName_).baseName());
	client_->sendGetFileList();
}

void MainWindow::startAllPlayers()
{
	isDebug_=false;
    isProfile_=false;
	if (!prepareStartOnPlayer())
		return;
	int pc=players_->count();
	int pi=players_->currentIndex();
	allPlayersPlayList.push_back(players_->currentData().toString());
	for (int k=0;k<pc;k++)
		if (k!=pi)
			allPlayersPlayList.push_back(players_->itemData(k).toString());
	startNextPlayer();
}

void MainWindow::startNextPlayer()
{
	if (allPlayersPlayList.empty())
		return;

	QString hostData=allPlayersPlayList.back();
	QStringList parts=hostData.split('|');
	if (parts.count()==1)
		client_->connectToHost(parts[0],15000);
	else
		client_->connectToHost(parts[0],parts[1].toInt());
	clientIsWeb_=(parts[0]=="[ws]");
}

void MainWindow::playStarted()
{
	if (allPlayersPlayList.empty())
		return;
	allPlayersPlayList.pop_back();
	startNextPlayer();
}

#if 0
void MainWindow::start()
{
// type (first byte)
// 0: folder
// 1; file

	saveAll();
	client_->sendGetFileList();
//	client_->sendDeleteFiles();

	QDir path(QFileInfo(projectFileName_).path());

//	sentMap_.clear();

	QDomDocument doc = libraryWidget_->toXml();

	std::stack<QDomNode> stack;
	stack.push(doc.documentElement());

	std::vector<QString> dir;

	while (stack.empty() == false)
	{
		QDomNode n = stack.top();
		QDomElement e = n.toElement();
		stack.pop();

		if (n.isNull() == true)
		{
			dir.pop_back();
			continue;
		}

		QString type = e.tagName();

		if (type == "file")
		{
#ifndef NEW_CLIENT
			QString fileName = QDir::cleanPath(path.absoluteFilePath(e.attribute("file")));
			QFile file(fileName);
			if (file.open(QIODevice::ReadOnly) == false)
			{
				printf("cannot open file\n");
			}
			else
			{
				std::string n;
				for (std::size_t i = 0; i < dir.size(); ++i)
					n += qPrintable(dir[i] + "/");
				n += qPrintable(name);

				QByteArray b = file.readAll();

				std::vector<char> buffer(1 + n.size() + 1 + b.size());

				int pos = 0;
				buffer[pos] = 1;								// 1 for file
				pos += 1;
				strcpy(&buffer[pos], n.c_str());				// name
				pos += n.size() + 1;
				if (b.isEmpty() == false)
					memcpy(&buffer[pos], b.constData(), b.size());	// data
				pos += b.size();

				Q_ASSERT(buffer.size() == pos);
				int id = client_->sendData(&buffer[0], buffer.size());
				sentMap_[id] = name;
			}
#else
			QString fileName = e.attribute("file");
			QString name = QFileInfo(fileName).fileName();

			QString n;
			for (std::size_t i = 0; i < dir.size(); ++i)
				n += dir[i] + "/";
			n += name;

/*			int id = client_->sendFile(n, fileName);
			if (id == 0)
				printf("cannot open file\n");
			else
				sentMap_[id] = name; */

			fileQueue_.push_back(qMakePair(n, fileName));
#endif
			continue;
		}
		
		if (type == "folder")
		{
			QString name = e.attribute("name");
			dir.push_back(name);

#ifndef NEW_CLIENT
			std::string n;
			for (std::size_t i = 0; i < dir.size(); ++i)
				n += qPrintable(dir[i] + "/");

			std::vector<char> buffer(1 + n.size() + 1);
			int pos = 0;
			buffer[pos] = 0;						// 0 for folder
			pos += 1;
			strcpy(&buffer[pos], n.c_str());		// name
			pos += n.size() + 1;

			Q_ASSERT(buffer.size() == pos);
			int id = client_->sendData(&buffer[0], buffer.size());
			sentMap_[id] = "";
#else
			QString n;
			for (std::size_t i = 0; i < dir.size(); ++i)
				n += dir[i] + "/";
		
			client_->createFolder(n);

			//sentMap_[client_->createFolder(n)] = "";
#endif

			stack.push(QDomNode());
		}

		QDomNodeList childNodes = n.childNodes();
		for (int i = 0; i < childNodes.size(); ++i)
			stack.push(childNodes.item(i));
	}


//	QProgressDialog* p = new QProgressDialog("Sending files...", "Cancel", 0, 100, this);
//	p->exec();

//	if (fileQueue_.empty() == false)			//TODO: bunu birazdan yap
//		ui.actionStart->setEnabled(false);

	std::vector<QString> topologicalSort = libraryWidget_->topologicalSort();
	for (std::size_t i = 0; i < topologicalSort.size(); ++i)
	{
		int index = -1;
		for (std::size_t j = 0; j < fileQueue_.size(); ++j)
		{
			if (fileQueue_[j].second == topologicalSort[i])
			{
				index = j;
				break;
			}
		}
		
		if (index != -1)
		{
			QPair<QString, QString> item = fileQueue_[index];
			fileQueue_.erase(fileQueue_.begin() + index);
			fileQueue_.push_back(item);
		}
	}

	QStringList luaFiles;
	for (int i = 0; i < fileQueue_.size(); ++i)
		if (QFileInfo(fileQueue_[i].first).suffix().toLower() == "lua")
			luaFiles << fileQueue_[i].first;

	if (fileQueue_.empty() == false)
		fileQueue_.push_back(qMakePair(luaFiles.join("|"), QString("play")));
}
#endif

void MainWindow::onTimer()
{
	timerEvent(0);
}

void MainWindow::timerEvent(QTimerEvent*)
{
	time_t ctime=time(NULL);

	for (int k=0;k<players_->count();)
	{
		QStringList parts=players_->itemData(k).toString().split('|');
		int itime=parts[3].toInt();
		if ((itime>ctime)||(itime<(ctime-60)))
			players_->removeItem(k);
		else
			k++;
	}
#ifndef NEW_CLIENT
//	static int i = 0;
//	printf("tick: %d\n", i++);

	int dataTotal = 0;

	while (true)
	{
		int dataSent0 = client_->dataSent();
		int dataReceived0 = client_->dataReceived();

		Event event;
		client_->tick(&event);

//		if (event.eventCode != eNone)
//			printf("%s\n", eventCodeString(event.eventCode));

		int dataSent1 = client_->dataSent();
		int dataReceived1 = client_->dataReceived();

		if (event.eventCode == eOtherSideConnected)
		{
			ui.actionStart->setEnabled(true);
			printf("other side connected\n");
		}
		else if (event.eventCode == eOtherSideClosedConnection)
		{
			ui.actionStart->setEnabled(false);
			printf("other side closed connection\n");
		}
		else if (event.eventCode == eDataSent)
		{
			std::map<int, QString>::iterator iter = sentMap_.find(event.id);
			if (iter != sentMap_.end())
			{
				QString name = iter->second;
				if (name.isEmpty() == false)
				{
					outputWidget_->moveCursor(QTextCursor::End);
					outputWidget_->insertPlainText(iter->second + " is uploaded.\n");
				}
				sentMap_.erase(iter);
				if (sentMap_.empty() == true)
				{
					outputWidget_->moveCursor(QTextCursor::End);
					outputWidget_->insertPlainText("Uploading finished!\n");
					sendPlayMessage();
				}
			}
		}
		else if (event.eventCode == eDataReceived)
		{
			const std::vector<char>& data = event.data;

			if (data[0] == 4)
			{
				std::string str = &data[1];
				outputWidget_->moveCursor(QTextCursor::End);
				outputWidget_->insertPlainText(str.c_str());
			}
		}

		int dataDelta = (dataSent1 - dataSent0) + (dataReceived1 - dataReceived0);
		dataTotal += dataDelta;

		if (dataDelta == 0 || dataTotal > 1024)
			break;
	}
#else
	QDir path(QFileInfo(projectFileName_).path());

	if (client_ && client_->bytesToWrite() == 0)
	{
		if (fileQueue_.empty() == false)
		{
			const QString& s1 = fileQueue_.front().first;
			const QString& s2 = fileQueue_.front().second;

			if (s2 == "play")
			{
				QStringList luafiles = s1.split("|");	
				outputWidget_->append("Uploading finished.\n");
				client_->sendProjectProperties(libraryWidget_->getProjectProperties());
				sendPlayMessage(luafiles);
				isTransferring_ = false;
			}
			else
			{
				// create remote directories
				{
					QStringList paths = s1.split("/");
					QStringList dir;
					for (int i = 0; i < paths.size() - 1; ++i)
					{
						dir << paths[i];
						client_->sendCreateFolder(dir.join("/"));
					}
				}

				QString fileName = QDir::cleanPath(path.absoluteFilePath(s2));
                if (client_->sendFile(s1, fileName) == 0)
				{
					outputWidget_->append(s1 + " cannot be opened.\n");
				}
				else
				{
					outputWidget_->append(s1 + " is uploading.\n");
				}
			}

			fileQueue_.pop_front();
		}
	}
#endif
}

unsigned int MainWindow::sendPlayMessage(const QStringList& luafiles)
{
#ifndef NEW_CLIENT
	char play = 2;
	client_->sendData(&play, sizeof(char));
#else
	if (isDebug_)
	{
		//Collect breakpoints
		ByteBuffer buffer;
		buffer << (char) 21;
        buffer << (char) 0x84; //Break line
        int nbreakpoints=TextEdit::breakpoints.size();
		buffer << nbreakpoints;
        foreach(QString bp, TextEdit::breakpoints) {
            int ls=bp.lastIndexOf(':');
            std::string source=(QString("@")+bp.mid(0,ls)).toStdString();
            int line=bp.mid(ls+1).toInt()+1;
            buffer << line;
            buffer << source;
        }

		client_->sendCommand(buffer.data(),buffer.size());
	}
    else if (isProfile_) {
        ByteBuffer buffer;
        buffer << (char) 30;
        client_->sendCommand(buffer.data(),buffer.size());
    }
	unsigned int code=client_->sendPlay(luafiles);
	playStarted();
	return code;
#endif
}

unsigned int MainWindow::sendStopMessage()
{
#ifndef NEW_CLIENT
	char stop = 3;
	client_->sendData(&stop, sizeof(char));
#else
	return client_->sendStop();
#endif
}

void MainWindow::stop()
{
    if (isBreaked_) resumeDebug(0x00);
    fileQueue_.clear();
	sendStopMessage();
}

/*
void MainWindow::enableUI()
{
	libraryWidget_->setEnabled(true);
	outputWidget_->setEnabled(true);
	ui.actionClose_Project->setEnabled(true);
	setWindowTitle(projectName() + "[*] - " + tr("Gideros"));
//	setWindowModified(false);
}
*/
/*
void MainWindow::disableUI()
{
	libraryWidget_->setEnabled(false);
	outputWidget_->setEnabled(false);
	ui.actionClose_Project->setEnabled(false);
	setWindowTitle(tr("Gideros"));
//	setWindowModified(false);
}
*/
void MainWindow::newProject()
{
	if (maybeSave())
	{
		NewProjectDialog* newProjectDialog = new NewProjectDialog(this);
		if (newProjectDialog->exec() == QDialog::Accepted)
		{
			// try to create directory
			QString dir = newProjectDialog->fullDirectory();
			if (QDir().mkpath(dir) == false)
			{
				QMessageBox::information(this, "Information", "Could not create the directory: " + dir);
				return;
			}

			// try to create project
			QString fileName = newProjectDialog->fullName();
			QFile file(fileName);
			if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
			{
				QMessageBox::information(this, "Information", "Could not create the project: " + fileName);
				return;
			}

			closeProject();

			libraryWidget_->newProject(fileName);

			QTextStream(&file) << libraryWidget_->toXml().toString();

			file.close();

			hideStartPage();

			projectFileName_ = fileName;
			md5_.clear();

			updateUI();

			setWindowTitle(projectName() + "[*] - " + tr("Gideros"));

			QSettings settings;
			settings.setValue("location", newProjectDialog->location());

			addToRecentProjects(projectFileName_);
		}
	}
}

void MainWindow::closeProject()
{
	if (maybeSave())
	{
		storeOpenFiles();

		projectFileName_ = "";
		md5_.clear();

		libraryWidget_->clear();
		outputWidget_->clear();
		previewWidget_->setFileName("", "");

		foreach (MdiSubWindow* window, mdiArea_->subWindowList())
		{
			if (window != mdiArea_->activeSubWindow())
				mdiArea_->setActiveSubWindow(window);

			mdiArea_->closeActiveSubWindow();
		}

		updateUI();

		setWindowTitle(tr("Gideros"));
	}
}

void MainWindow::saveProject()
{
    QFile file(projectFileName_);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
    {
#if defined(Q_OS_MAC)
        QString ex = "../../Examples/";
#else
        QString ex = "Examples/";
#endif
        if (!projectFileName_.startsWith(ex))
            QMessageBox::information(this, "Information", "Could not save the project file: " + projectFileName_);
        else
            outputWidget_->append("Could not save the example project file: " + projectFileName_+"\n");
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << libraryWidget_->toXml().toString();
    file.close();
    libraryWidget_->setModified(false);
}

void MainWindow::cloneProject()
{
    if (maybeSave())
    {
        NewProjectDialog* newProjectDialog = new NewProjectDialog(this);
        if (newProjectDialog->exec() == QDialog::Accepted)
        {
            // try to create directory
            QString dir = newProjectDialog->fullDirectory();
            if (QDir().mkpath(dir) == false)
            {
                QMessageBox::information(this, "Information", "Could not create the directory: " + dir);
                return;
            }

            // try to create project
            QString fileName = newProjectDialog->fullName();
            QFile file(fileName);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
            {
                QMessageBox::information(this, "Information", "Could not create the project: " + fileName);
                return;
            }

            storeOpenFiles();

            outputWidget_->clear();
            previewWidget_->setFileName("", "");

            foreach (MdiSubWindow* window, mdiArea_->subWindowList())
            {
                if (window != mdiArea_->activeSubWindow())
                    mdiArea_->setActiveSubWindow(window);

                mdiArea_->closeActiveSubWindow();
            }

            libraryWidget_->cloneProject(fileName);

            QTextStream(&file) << libraryWidget_->toXml().toString();

            file.close();

            hideStartPage();

            projectFileName_ = fileName;
            md5_.clear();

            updateUI();

            setWindowTitle(projectName() + "[*] - " + tr("Gideros"));

            QSettings settings;
            settings.setValue("location", newProjectDialog->location());

            addToRecentProjects(projectFileName_);

            //shouldn't be needed, but due to a crash in library widget when trying to change project name, just close and reopen as a workaround
            closeProject();
            openProject(fileName);

        }
    }
}

void MainWindow::consolidateProject()
{
    int reply = QMessageBox::question(this, "Consolidate Project", "This operation will remove all links and copy all files into the project directory. Are you sure you want to do this ? This cannot be undone.",
                                  QMessageBox::Yes|QMessageBox::Cancel);
    if (reply == QMessageBox::Cancel)
        return;

    if (maybeSave())
    {
        storeOpenFiles();

        outputWidget_->clear();
        previewWidget_->setFileName("", "");

        foreach (MdiSubWindow* window, mdiArea_->subWindowList())
        {
            if (window != mdiArea_->activeSubWindow())
                mdiArea_->setActiveSubWindow(window);

            mdiArea_->closeActiveSubWindow();
        }

        libraryWidget_->consolidateProject();

        saveProject();
    }
}

void MainWindow::openProject()
{
	if (maybeSave())
	{
		QSettings settings;

		QString dir = settings.value("lastOpenProjectDir", QString()).toString();

		QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
			dir,
			tr("Gideros Project (*.gproj)"));

		if (fileName.isEmpty() == true)
			return;

		QDir newdir = QFileInfo(fileName).absoluteDir();
		newdir.cdUp();
		settings.setValue("lastOpenProjectDir", newdir.absolutePath());

		openProject(fileName);
	}
}


void MainWindow::openProject(const QString& fileName)
{
	QDomDocument doc;
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly))
	{
		QMessageBox::information(this, "Information", "Could not open the project file: " + fileName);
		return;
	}

	if (!doc.setContent(&file))
	{
		QMessageBox::information(this, "Information", "Could not parse the project file: " + fileName);
		file.close();
		return;
	}
	file.close();

	hideStartPage();

	closeProject();

	projectFileName_ = fileName;
	loadMD5();

	libraryWidget_->loadXml(projectFileName_, doc);

	updateUI();
	setWindowTitle(projectName() + "[*] - " + tr("Gideros"));
	addToRecentProjects(projectFileName_);

	restoreOpenFiles();
}

void MainWindow::reloadProject()
{
    TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

    if (textEdit)
    {
        int line, index;
#ifdef SCINTILLAEDIT_H
        sptr_t pos=textEdit->sciScintilla()->currentPos();
        index=textEdit->sciScintilla()->column(pos);
        line=textEdit->sciScintilla()->lineFromPosition(pos);
#else
        textEdit->sciScintilla()->getCursorPosition(&line, &index);
#endif
        QString rp = projectFileName_;
        closeProject();
        openProject(rp);
#ifdef SCINTILLAEDIT_H
        textEdit->sciScintilla()->setCurrentPos(textEdit->sciScintilla()->findColumn(line, index));
#else
        textEdit->sciScintilla()->setCursorPosition(line, index);
#endif
    }
}

QString MainWindow::projectName() const
{
	return QDir(projectFileName_).dirName();
}

QString MainWindow::projectDirectory() const
{
	return QDir::cleanPath(QDir(projectFileName_).absoluteFilePath(".."));
}

QString MainWindow::makeProjectRelativePath(const QString& path) const
{
    QString returnPath = path;
    returnPath.replace(projectDirectory(), "", Qt::CaseInsensitive);
    return returnPath;
}

/*
void MainWindow::onModified()
{
	setWindowModified(true);
}
*/

void MainWindow::startPlayer()
{
#if defined(Q_OS_MAC)
        QProcess::startDetached("open \"../../Gideros Player.app\"");
#elif defined(Q_OS_WIN)
	QProcess::startDetached("GiderosPlayer.exe");
#else
	QProcess::startDetached("GiderosPlayer");
#endif
}

void MainWindow::startTexturePacker()
{
#if defined(Q_OS_MAC)
        QProcess::startDetached("open \"../../Gideros Texture Packer.app\"");
#elif defined(Q_OS_WIN)
	QProcess::startDetached("GiderosTexturePacker.exe");
#else
	QProcess::startDetached("GiderosTexturePacker");
#endif
}

void MainWindow::startFontCreator()
{
#if defined(Q_OS_MAC)
        QProcess::startDetached("open \"../../Gideros Font Creator.app\"");
#elif defined(Q_OS_WIN)
	QProcess::startDetached("GiderosFontCreator.exe");
#else
	QProcess::startDetached("GiderosFontCreator");
#endif
}

void MainWindow::fileAssociations()
{
	FileAssociationsDialog dialog(fileAssociations_, this);

	if (dialog.exec() == QDialog::Accepted)
	{
		fileAssociations_ = dialog.fileAssociations();

		QList<QVariant> v;
		for (int i = 0; i < fileAssociations_.size(); ++i)
			v.push_back(fileAssociations_[i]);

		QSettings settings;
		settings.setValue("fileAssociations", v);
	}
}

TextEdit* MainWindow::createTextEdit()
{
	TextEdit* textEdit = new TextEdit;
	connect(textEdit, SIGNAL(copyAvailable(bool)), this, SLOT(updateUI()));
    connect(textEdit, SIGNAL(textChanged()), this, SLOT(updateUI()));
    connect(textEdit, SIGNAL(lookupSymbol(QString,int,int)), this, SLOT(lookupSymbol(QString,int,int)));
    return textEdit;
}

TextEdit* MainWindow::findTextEdit(const QString& fileName) const
{
	foreach (MdiSubWindow* window, mdiArea_->subWindowList())
	{
		TextEdit* textEdit = qobject_cast<TextEdit*>(window);
		if (textEdit && textEdit->fileName() == fileName)
			return textEdit;
	}

	return 0;
}

void MainWindow::insertColorInCode()
{
	
#ifdef SCINTILLAEDIT_H
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if(textEdit){
	QColor c;

	if (textEdit->hasSelectedText()){

		QString sel = textEdit->getSelectedText();

		if (!sel.isEmpty() && sel.startsWith("0x")){
			c.setNamedColor(sel.replace("0x", "#"));
		}else{
			QMessageBox::information(this, "Information", "The selected text is not a valid color syntax!");
			return;
		}
	}else{
		c = QColor("#ffffff");
	}

	QColorDialog* CD = new QColorDialog(c);
	CD->setWindowFlags(Qt::Widget);
	CD->setOptions(QColorDialog::DontUseNativeDialog);
	CD->layout()->setContentsMargins(8, 8, 8, 8);

	connect(CD, &QColorDialog::currentColorChanged, [=](const QColor &color) {
						textEdit->sciScintilla()->beginUndoAction();
						
						const QString& replaceText = color.name().replace("#", "0x");
						
						int from , to;
						if (textEdit->hasSelectedText()){
							from = textEdit->sciScintilla()->selectionStart();
							to = textEdit->sciScintilla()->selectionEnd();
							textEdit->sciScintilla()->setTargetRange(from,to);
							textEdit->sciScintilla()->replaceTarget(replaceText.length(), replaceText.toUtf8());
							textEdit->sciScintilla()->setSel(from, from + replaceText.length());
							textEdit->sciScintilla()->setFocus(true);
							return;
						}else{
							from = textEdit->sciScintilla()->currentPos();
							textEdit->sciScintilla()->insertText(from, replaceText.toUtf8());
							textEdit->sciScintilla()->setSel(from, from + replaceText.length());
							textEdit->sciScintilla()->setFocus(true);
							return;
						}
						
						textEdit->sciScintilla()->endUndoAction();
	});
	CD->exec();
#endif
	}
	//~ else
	//~ {
		//~ bool isProjectOpen = projectFileName_.isEmpty() == false;
		//~ if (isProjectOpen){
			//~ QMessageBox::information(this, "Information", "There is no selected color syntax!");
		//~ }
	//~ }
}

void MainWindow::setWrap()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());
	
	if (textEdit == NULL)
		return;
		
	bool wrap_mode;

	if (textEdit)
#ifdef SCINTILLAEDIT_H
	wrap_mode = textEdit->sciScintilla()->wrapMode();
	textEdit->sciScintilla()->setWrapMode(wrap_mode ? 0 : 1);
	textEdit->sciScintilla()->setWrapVisualFlags(SC_WRAPVISUALFLAG_END);
	
	if (wrap_mode)
	{
		ui.actionWrap->setIcon(IconLibrary::instance().icon(0, "wrap"));
	}
	else
	{
		ui.actionWrap->setIcon(IconLibrary::instance().icon(0, "unwrap"));
	}
#endif
}

void MainWindow::clearDebugHighlights() {
    foreach (MdiSubWindow* window, mdiArea_->subWindowList())
    {
        TextEdit* textEdit = qobject_cast<TextEdit*>(window);
        if (textEdit)
            textEdit->highlightDebugLine(-1);
    }
}

void MainWindow::onOpenRequest(const QString& itemName, const QString& fileName)
{
    Q_UNUSED(itemName);
	QString suffix = QFileInfo(fileName).suffix().toLower();

    if (suffix == "txt" || suffix == "lua" || suffix == "glsl" || suffix=="hlsl" || suffix=="xml" || suffix=="json")
		openFile(fileName);
	else {
		std::string aname=AddonsManager::addonForExtension(suffix.toStdString());
        if (aname.empty())
            QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
        else
            launchAddon(QString::fromStdString(aname), fileName);
	}
}

void MainWindow::autoIndent()
{

}

void MainWindow::updateUI()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	bool hasMdiChild = textEdit != NULL;
	bool hasSelectedText = textEdit && textEdit->hasSelectedText();
	bool isRedoAvailable = textEdit && textEdit->isRedoAvailable();
	bool isUndoAvailable = textEdit && textEdit->isUndoAvailable();

	ui.actionCut->setEnabled(hasSelectedText);
	ui.actionCopy->setEnabled(hasSelectedText);
	ui.actionPaste->setEnabled(hasMdiChild);

	ui.actionUndo->setEnabled(isUndoAvailable);
	ui.actionRedo->setEnabled(isRedoAvailable);

	bool isProjectOpen = projectFileName_.isEmpty() == false;

//	libraryWidget_->setEnabled(isProjectOpen);
//	outputWidget_->setEnabled(isProjectOpen);
	ui.actionSave_Project->setEnabled(isProjectOpen);
	ui.actionClose_Project->setEnabled(isProjectOpen);
//	setWindowTitle(projectName() + "[*] - " + tr("Gideros"));
	ui.actionExport_Project->setEnabled(isProjectOpen);
	ui.actionExport_Pack->setEnabled(isProjectOpen);
	ui.actionFind_in_Files->setEnabled(isProjectOpen);

	ui.actionFind->setEnabled(hasMdiChild);
	ui.actionReplace->setEnabled(hasMdiChild);
	ui.actionFind_Next->setEnabled(hasMdiChild);
	ui.actionFind_Previous->setEnabled(hasMdiChild);
	ui.actionGo_To_Line->setEnabled(hasMdiChild);

    outlineWidget_->setDocument(textEdit,ui.actionLive_syntax_checking->isChecked(),ui.actionType_checking->isChecked());
    outlineDock_->setVisible(isProjectOpen);
}

void MainWindow::save()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit && textEdit->save() == true)
		statusBar()->showMessage(tr("Items(s) Saved"), 2000);
}

void MainWindow::saveAll()
{
	bool itemsSaved = false;
	foreach (MdiSubWindow* window, mdiArea_->subWindowList())
	{
		TextEdit* textEdit = qobject_cast<TextEdit*>(window);
		if (textEdit != 0 && textEdit->save() == true)
			itemsSaved = true;
	}

	if (libraryWidget_->isModified())
		saveProject();

	if (itemsSaved == true)
		statusBar()->showMessage(tr("Items(s) Saved"), 2000);
}


void MainWindow::showStatusbarMessage(QString message, int timeout)
{
	statusBar()->showMessage(message, timeout);
}

#if 0
void MainWindow::compile()
{
	if (compileThread_ != 0)
		return;

	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit == NULL)
		return;

	QString fileName = textEdit->fileName();
	QFileInfo fileInfo(fileName);

	if (fileInfo.suffix().toLower() == "lua")
	{
		QDir dir(QFileInfo(projectFileName_).path());
		QString outdir = QDir::cleanPath(dir.absoluteFilePath("obj"));

		textEdit->save();
		outputWidget_->clear();
		compileThread_ = new CompileThread(QStringList() << fileName, false, ".bin", outdir, this);
		connect(compileThread_, SIGNAL(message(const QString&)), outputWidget_, SLOT(append(const QString&)));
		connect(compileThread_, SIGNAL(finished()), this, SLOT(compileThreadFinishedOrTerminated()));
		connect(compileThread_, SIGNAL(terminated()), this, SLOT(compileThreadFinishedOrTerminated()));
		connect(compileThread_, SIGNAL(compileFinished(bool)), this, SLOT(compileFinished(bool)));
		compileThread_->start();
	}
}
#endif

static QString escape(const QString &str)
{
    QString result = str;

    result.replace(" ", "\\ ");
    result.replace("#", "\\#");

    return result;
}

static QString quote(const QString &str)
{
    return "\"" + str + "\"";
}


void MainWindow::compile()
{
    if (makeProcess_->state() != QProcess::NotRunning)
        return;

    TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

    if (textEdit == NULL)
        return;

    textEdit->save();

    QDir toolsDir = QDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_MAC)
    toolsDir.cdUp();
#endif
    toolsDir.cd("Tools");

#if defined(Q_OS_WIN)
    QString make = toolsDir.filePath("make.exe");
    QString luac = toolsDir.filePath("luac.exe");
#else
    QString make = toolsDir.filePath("make");
    QString luac = toolsDir.filePath("luac");
#endif

    QDir dir = QFileInfo(projectFileName_).dir();
    dir.mkdir(".tmp");

    QFile file(dir.filePath(".tmp/makefile"));
    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream out(&file);

        out << "LUAC = " << quote(luac) << "\n";
        out << "\n";

        QString f = dir.relativeFilePath(textEdit->fileName());
        QString o = ".tmp/" + QFileInfo(f).fileName() + ".bin";

        out << "all:" << "\n";
        out << "\t" << "$(LUAC) -o " << quote(o) << " " << quote(f) << "\n";

        file.close();
    }

    makeProcess_->setWorkingDirectory(dir.path());
    makeProcess_->start(make, QStringList() << "-f" << ".tmp/makefile");
}

#if 0
CompileThread* MainWindow::createCompileAllThread() const
{
	if (projectFileName_.isEmpty() == true)
		return 0;

	QString path = QFileInfo(projectFileName_).path();
	QDir dir(path);

	QStringList fileNames;

	QDomDocument doc = libraryWidget_->toXml();

	std::stack<QDomNode> stack;
	stack.push(doc.documentElement());

	while (stack.empty() == false)
	{
		QDomNode n = stack.top();
		QDomElement e = n.toElement();
		stack.pop();

		QString type = e.tagName();
		QString name = e.attribute("name");

		if (type == "file")
		{
			QString fileName = e.attribute("source");
			if (fileName.isEmpty())
			{
				fileName = e.attribute("file");
			}
			if (QFileInfo(fileName).suffix().toLower() == "lua")
				fileNames << QDir::cleanPath(dir.absoluteFilePath(fileName));
		}

		QDomNodeList childNodes = n.childNodes();
		for (int i = 0; i < childNodes.size(); ++i)
			stack.push(childNodes.item(i));
	}

	// TODO: bunu baska bir fonksiyon yapip disariya al
	for (int i = 0; i < fileNames.size(); ++i)
	{
		TextEdit* textEdit = findTextEdit(fileNames[i]);

		if (textEdit != 0)
			textEdit->save();
	}

	if (fileNames.isEmpty() == false)
	{
		QDir dir(QFileInfo(projectFileName_).path());
		QString outdir = QDir::cleanPath(dir.absoluteFilePath("obj"));

		return new CompileThread(fileNames, true, ".bin", outdir, const_cast<MainWindow*>(this));
	}

	return 0;
}
#endif

#if 0
void MainWindow::compileAll()
{
	if (compileThread_ != 0)
		return;

	compileThread_ = createCompileAllThread();

	if (compileThread_ == 0)
		return;

	outputWidget_->clear();
	connect(compileThread_, SIGNAL(message(const QString&)), outputWidget_, SLOT(append(const QString&)));
	connect(compileThread_, SIGNAL(finished()), this, SLOT(compileThreadFinishedOrTerminated()));
	connect(compileThread_, SIGNAL(terminated()), this, SLOT(compileThreadFinishedOrTerminated()));
	compileThread_->start();
}
#endif




void MainWindow::compileAll()
{
    if (projectFileName_.isEmpty())
        return;

    if (makeProcess_->state() != QProcess::NotRunning)
        return;

    saveAll();

    QStringList fileNames;
    {
        QDir dir = QFileInfo(projectFileName_).dir();

        QDomDocument doc = libraryWidget_->toXml();

        std::stack<QDomNode> stack;
        stack.push(doc.documentElement());

        while (stack.empty() == false)
        {
            QDomNode n = stack.top();
            QDomElement e = n.toElement();
            stack.pop();

            if (e.tagName() == "file")
            {
                QString fileName = e.attribute("source");
				if (fileName.isEmpty())
				{
					fileName = e.attribute("file");
				}
                if (QFileInfo(fileName).suffix().toLower() == "lua")
                    fileNames << fileName;
            }

            QDomNodeList childNodes = n.childNodes();
            for (int i = 0; i < childNodes.size(); ++i)
                stack.push(childNodes.item(i));
        }
    }

    QDir toolsDir = QDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_MAC)
    toolsDir.cdUp();
#endif
    toolsDir.cd("Tools");

#if defined(Q_OS_WIN)
    QString make = toolsDir.filePath("make.exe");
    QString luac = toolsDir.filePath("luac.exe");
#else
    QString make = toolsDir.filePath("make");
    QString luac = toolsDir.filePath("luac");
#endif

    QString all = "all:";
    for (int i = 0; i < fileNames.size(); ++i)
    {
        QString f = fileNames[i];
        QString o = ".tmp/" + QFileInfo(f).fileName() + ".bin";
        all += " " + escape(o);
    }

    QDir dir = QFileInfo(projectFileName_).dir();
    dir.mkdir(".tmp");

    QFile file(dir.filePath(".tmp/makefile"));
    if (file.open(QFile::WriteOnly | QFile::Truncate))
    {
        QTextStream out(&file);

        out << "LUAC = " << quote(luac) << "\n";
        out << "\n";

        out << all << "\n";
        out << "\n";

        for (int i = 0; i < fileNames.size(); ++i)
        {
            QString f = fileNames[i];
            QString o = ".tmp/" + QFileInfo(f).fileName() + ".bin";

            out << escape(o) << ": " << escape(f) << "\n";
            out << "\t" << "$(LUAC) -o " << quote(o) << " " << quote(f) << "\n";

            out << "\n";
        }
        file.close();
    }

    makeProcess_->setWorkingDirectory(dir.path());
    makeProcess_->start(make, QStringList() << "-f" << ".tmp/makefile");
}

void MainWindow::cancel()
{
#if 0
	if (compileThread_ == 0)
		return;
#endif

}

#if 0
void MainWindow::compileThreadFinishedOrTerminated()
{
	delete compileThread_;
	compileThread_ = 0;
}
#endif

static bool parseFileLineString(const QString& l, QString* fileName = 0, unsigned int* lineNumber = 0)
{
	QString line = l.trimmed();

	QStringList list = line.split(':');

	if (list.size() < 3)
		return false;

	if (list[0].size() == 1)
	{
		if (list[1].isEmpty() == false)
		{
			if (list[1][0] == '/' || list[1][0] == '\\')
			{
				QString file = list[0] + ":" + list[1];
				list.pop_front();
				list[0] = file;
			}
		}
	}
	
	bool ok;
	unsigned int list1 = list[1].toUInt(&ok);

	if (ok == false)
		return false;

	if (fileName != 0)
		*fileName = list[0];

	if (lineNumber != 0)
		*lineNumber = list1;

	return true;
}

/*void MainWindow::outputMouseDoubleClick(QMouseEvent* e)
{
	QTextCursor	cursor = outputWidget_->cursorForPosition(e->pos());

	QString fileName;
	unsigned int lineNumber;
	if (parseFileLineString(cursor.block().text(), &fileName, &lineNumber) == true)
	{
		TextEdit* textEdit = openFile(fileName);

		if (textEdit)
			textEdit->setCursorPosition(lineNumber - 1, 0);
	}
}*/

// check if string starts with [space]*[digit]+[:]
static bool lineNumber(const QString& str, int* line = NULL)
{
    QRegularExpression regexp("^\\s*(\\d+)\\:");

    QRegularExpressionMatch match=regexp.match(str);
    if (!match.hasMatch())
		return false;

	if (line)
        *line = match.captured(1).toInt();

	return true;
}

void MainWindow::outputMouseDoubleClick(QMouseEvent* e)
{
	QTextCursor	cursor = outputWidget_->cursorForPosition(e->pos());
	QTextBlock block = cursor.block();
	QString text = block.text();

	{
		QString fileName;
		unsigned int lineNumber;
		if (parseFileLineString(text, &fileName, &lineNumber) == true)
		{
			TextEdit* textEdit = openFile(fileName);

			if (textEdit)
            {
				textEdit->setCursorPosition(lineNumber - 1, 0);
                textEdit->setFocusToEdit();
            }

			return;
		}
	}

	int line;
	if (lineNumber(text, &line))
	{
		while (true)
		{
			block = block.previous();

			QString text = block.text();
			if (text.isEmpty())
				break;
			
			if (text.right(1) == ":")
			{
				TextEdit* textEdit = openFile(text.left(text.size() - 1));

                if (textEdit) {
					textEdit->setCursorPosition(line - 1, 0);
                    textEdit->setFocusToEdit();
                }
//				printf("%s%d\n", qPrintable(text), line);
				break;
			}
		}
	}
}


TextEdit* MainWindow::openFile(const QString& fn, bool suppressErrors/* = false*/)
{
	QString fileName;
    QString itemName;

    QDir dir(QFileInfo(projectFileName_).path());
    if (QFileInfo(fn).isRelative())
	{
		fileName = QDir::cleanPath(dir.absoluteFilePath(libraryWidget_->fileName(fn)));
        itemName=fn;
	}
	else
	{
		fileName = fn;
        itemName = libraryWidget_->itemName(dir,fn);
	}

	TextEdit* existing = findTextEdit(fileName);
	if (existing)
	{
		mdiArea_->setActiveSubWindow(existing);
		existing->setFocus(Qt::OtherFocusReason);
		return existing;
	}

	TextEdit* child = createTextEdit();
    if (child->loadFile(fileName, itemName, suppressErrors) == true)
	{
		mdiArea_->addSubWindow(child);
		child->showMaximized();
	}
	else
	{
		child->close();
        child = nullptr;
	}

	return child;
}

#if 0
void MainWindow::compileFinished(bool success)
{

}
#endif

#define MAX_RECENT_FILES 15
void MainWindow::addToRecentProjects(const QString& fileName)
{
	QSettings settings;
	QStringList files = settings.value("recentProjectList").toStringList();
	files.removeAll(fileName);
	files.prepend(fileName);
	while (files.size() > MAX_RECENT_FILES)
		files.removeLast();
	settings.setValue("recentProjectList", files);

	updateRecentProjectActions();
}

void MainWindow::updateRecentProjectActions()
{
	QAction* recentProjectActions[MAX_RECENT_FILES];
	recentProjectActions[0] = ui.actionProject1;
	recentProjectActions[1] = ui.actionProject2;
	recentProjectActions[2] = ui.actionProject3;
	recentProjectActions[3] = ui.actionProject4;
	recentProjectActions[4] = ui.actionProject5;
	recentProjectActions[5] = ui.actionProject6;
	recentProjectActions[6] = ui.actionProject7;
    recentProjectActions[7] = ui.actionProject8;
    recentProjectActions[8] = ui.actionProject9;
    recentProjectActions[9] = ui.actionProject10;
    recentProjectActions[10] = ui.actionProject11;
    recentProjectActions[11] = ui.actionProject12;
    recentProjectActions[12] = ui.actionProject13;
    recentProjectActions[13] = ui.actionProject14;
    recentProjectActions[14] = ui.actionProject15;

	QSettings settings;
	QStringList files = settings.value("recentProjectList").toStringList();

	int numRecentFiles = qMin(files.size(), MAX_RECENT_FILES);

	for (int i = 0; i < numRecentFiles; ++i)
	{
		QString text = tr("&%1 %2").arg(i + 1).arg(files[i]);
		recentProjectActions[i]->setText(text);
		recentProjectActions[i]->setData(files[i]);
		recentProjectActions[i]->setVisible(true);
	}

	for (int i = numRecentFiles; i < MAX_RECENT_FILES; ++i)
		recentProjectActions[i]->setVisible(false);
}

void MainWindow::openRecentProject()
{
	QAction *action = qobject_cast<QAction *>(sender());

	if (action != 0)
		openProject(action->data().toString());
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (maybeSave())
	{
		storeOpenFiles();

		QSettings settings;
		settings.setValue("geometry", saveGeometry());
        settings.setValue("windowState", saveState());
/*		settings.setValue("splitter1", splitter1_->saveState());
		settings.setValue("splitter2", splitter2_->saveState());
		settings.setValue("splitter3", splitter3_->saveState());*/

        outlineWidget_->saveSettings();

		event->accept();
	} 
	else
	{
		event->ignore();
	}
}

void MainWindow::actionLocalhostToggle(bool checked){
    QSettings settings;
    settings.setValue("player localhost", checked);

    QString playerip = QString("127.0.0.1");

    if(!checked){
        playerip = settings.value("player original ip", QString("127.0.0.1")).toString();
    }

    settings.setValue("player ip", playerip);

    #ifndef NEW_CLIENT
        delete client_;
        client_ = new Client(qPrintable(playerip), 15000);
    #else
        client_->connectToHost(playerip, 15000);
    #endif
}

void MainWindow::actionLiveSyntaxChecking(bool checked){
    QSettings settings;
    settings.setValue("syntaxcheck_live", checked);
}

void MainWindow::actionLiveTypeChecking(bool checked){
    QSettings settings;
    settings.setValue("typecheck_live", checked);
}

void MainWindow::playerSettings()
{
	PlayerSettingsDialog dialog(this);

	if (dialog.exec() == QDialog::Accepted)
	{
		ui.actionStart->setEnabled(false);
		ui.actionDebug->setEnabled(false);
        ui.actionProfile->setEnabled(false);
        ui.actionStop->setEnabled(false);
		
		//sentMap_.clear();
		
		QSettings settings;
		QString playerip = settings.value("player ip", QString("127.0.0.1")).toString();
        ui.actionLocalhostToggle->setChecked(settings.value("player localhost", true).toBool());

#ifndef NEW_CLIENT
		delete client_;
		client_ = new Client(qPrintable(playerip), 15000);
#else
		client_->connectToHost(playerip, 15000);
#endif
	}
}

void MainWindow::cancelUpload()
{

}

void MainWindow::find()
{
    QPoint od_Pos = this->outputDock_->mapToGlobal(this->outputDock_->rect().topRight());
    QPoint fd_Pos = this->findDialog_->mapToGlobal(this->findDialog_->rect().topLeft());
    this->findDialog_->move(od_Pos.x() - findDialog_->width(), od_Pos.y());
    
	replaceDialog_->hide();
//	findInFilesDialog_->hide();
	findDialog_->show();
	
    findDialog_->raise();
    findDialog_->activateWindow();

	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());
    if (textEdit->hasSelectedText())
    {
		QString sel = textEdit->getSelectedText();
		if (!sel.isEmpty()){
			findDialog_->setSelectedText(sel);
		}

	}
	else
	{
		findDialog_->setSelectedText("");
	}
	findDialog_->focusToFindWhat();
}

void MainWindow::findFirst()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
	{
		findWhat_ = findDialog_->findWhat();
		matchCase_ = findDialog_->matchCase();
		wholeWord_ = findDialog_->wholeWord();
        regexp_ = findDialog_->regexp();
        wrapSearch_ = findDialog_->wrap();
		bool forward = true;

		if (findWhat_.isEmpty() == false)
            if (textEdit->findFirst(findWhat_, regexp_, matchCase_, wholeWord_, wrapSearch_, forward) == false)
				QMessageBox::information(findDialog_, tr("Gideros"), tr("The specified text could not be found."));
	}
}

void MainWindow::findNext()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit && textEdit->hasSelectedText())
	{
		findWhat_ = findDialog_->findWhat();
		matchCase_ = findDialog_->matchCase();
		wholeWord_ = findDialog_->wholeWord();
        	regexp_ = findDialog_->regexp();
        	wrapSearch_ = findDialog_->wrap();
        
		findWhat_ = textEdit->getSelectedText();
	            if (textEdit->findFirst(findWhat_, regexp_, matchCase_, wholeWord_, wrapSearch_, true) == false)
				QMessageBox::information(findDialog_, tr("Gideros"), tr("The specified text could not be found."));
	}
}

void MainWindow::findPrevious()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit && textEdit->hasSelectedText())
	{
		findWhat_ = findDialog_->findWhat();
		matchCase_ = findDialog_->matchCase();
		wholeWord_ = findDialog_->wholeWord();
		regexp_ = findDialog_->regexp();
		wrapSearch_ = findDialog_->wrap();
        
		findWhat_ = textEdit->getSelectedText();
	            if (textEdit->findFirst(findWhat_, regexp_, matchCase_, wholeWord_, wrapSearch_, false) == false)
				QMessageBox::information(findDialog_, tr("Gideros"), tr("The specified text could not be found."));
	}
}

void MainWindow::replace()
{
    QPoint od_Pos = this->outputDock_->mapToGlobal(this->outputDock_->rect().topRight());
    QPoint fd_Pos = this->replaceDialog_->mapToGlobal(this->replaceDialog_->rect().topLeft());
    this->replaceDialog_->move(od_Pos.x() - replaceDialog_->width(), od_Pos.y());
    
	findDialog_->hide();
	replaceDialog_->show();
	
    replaceDialog_->raise();
    replaceDialog_->activateWindow();
    
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());
    if (textEdit->hasSelectedText())
    {
		QString sel = textEdit->getSelectedText();
		if (!sel.isEmpty()){
			replaceDialog_->setSelectedText(sel);
		}

	}
	else
	{
		replaceDialog_->setSelectedText("");
	}
	
    replaceDialog_->focusToFindWhat();
}

void MainWindow::toogleBookmark()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
		textEdit->toogleBookmark();
}

void MainWindow::nextBookmark()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
		textEdit->nextBookmark();
}

void MainWindow::previousBookmark()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
		textEdit->previousBookmark();
}

void MainWindow::clearBookmarks()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
		textEdit->clearBookmarks();
}

void MainWindow::undo()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
		textEdit->undo();
}

void MainWindow::redo()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
		textEdit->redo();
}

void MainWindow::goToLine()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
	{
		int lineCount = textEdit->sciScintilla()->lineCount();
		bool ok;
		int line = QInputDialog::getInt(this, tr("Line number"), tr("Go to line"),
			1, 1, lineCount + 1, 1, &ok);
		if (ok) {
			textEdit->sciScintilla()->gotoLine(line - 1);
		}
		
		textEdit->sciScintilla()->setFirstVisibleLine(line - 12);

	}
}

#ifndef NEW_CLIENT

#else
void MainWindow::connected()
{
	fileQueue_.clear();
	isTransferring_ = false;

	ui.actionStart->setEnabled(true);
	ui.actionDebug->setEnabled(true);
    ui.actionProfile->setEnabled(true);
    ui.actionStop->setEnabled(true);
	printf("other side connected\n");

	if (!allPlayersPlayList.empty())
		startOnPlayer();
}

void MainWindow::disconnected()
{
    if (isBreaked_) resumeDebug(0);
	if (!allPlayersPlayList.empty())
		return;
	fileQueue_.clear();
	isTransferring_ = false;

	ui.actionStart->setEnabled(false);
	ui.actionDebug->setEnabled(false);
	ui.actionStop->setEnabled(false);
    ui.actionProfile->setEnabled(false);
    printf("other side closed connection\n");
}

QVariant MainWindow::deserializeValue(ByteBuffer &buffer, QString &vtype) {
    char type;
    buffer >> type;
    std::string value;
    QString stype;
    QMap<QString,QVariant> table;
    char b;
    vtype=(type==20)?"ref":QString(lua_typename(NULL,type));
    switch (type) {
        case LUA_TNONE: return QVariant();
        case LUA_TNIL: return QVariant();
        case LUA_TBOOLEAN:
            buffer >> b;
            return QVariant::fromValue((bool)b);
        case LUA_TTABLE:
            buffer >> value; //table ref, for future use
            while (true)  {
                QVariant key=deserializeValue(buffer,stype);
                if (key.isNull()) break;
                QVariant val=deserializeValue(buffer,stype);
                table[key.toString()]=val;
            }
            return QVariant::fromValue(table);
        case LUA_TNUMBER:
            buffer >> value;
            return QVariant::fromValue(QString::fromStdString(value).toDouble());
        case LUA_TSTRING:
            buffer >> value;
            return QVariant::fromValue(QString::fromStdString(value));
        default:
            buffer >> value;
            return QVariant::fromValue(QPair<QString,QString>(vtype,QString::fromStdString(value)));
    }

}

void MainWindow::dataReceived(const QByteArray& d)
{
//	if (!updateMD5().empty())	// bunu neden koymusuz acaba?
//		saveMD5();

	const char* data = d.constData();

	if (data[0] == 4)
	{
		std::string str = &data[1];
        //outputWidget_->append(QString::fromUtf8(str.c_str()));
        QString strU = QString::fromUtf8(str.c_str());
        if (strU.startsWith("<") && strU.endsWith(">"))
        {
            outputWidget_->insertHtml(strU);
            outputWidget_->moveCursor(QTextCursor::End);
        }
        else
        {
            outputWidget_->append(strU);
        }
	}
	if (data[0] == 6 && isTransferring_ == true)
	{
		printf("file list has got\n");

		fileQueue_.clear();

		std::map<QString, QString> localFileMap;
		std::map<QString, QString> localFileMapReverse;
		{
			std::vector<std::pair<QString, QString> > fileList = libraryFileList();

			for (std::size_t i = 0; i < fileList.size(); ++i)
			{
				localFileMap[fileList[i].first] = fileList[i].second;
				localFileMapReverse[fileList[i].second] = fileList[i].first;
			}
		}
        bool mainluaOnly=libraryWidget_->getProjectProperties().mainluaOnly;
        std::vector<std::pair<QString, bool> > topologicalSort = libraryWidget_->topologicalSort(localFileMapReverse);
		for (std::size_t i = 0; i < topologicalSort.size(); ++i)
            if ((topologicalSort[i].second == false)&&(!mainluaOnly)&&(localFileMapReverse[topologicalSort[i].first]!="main.lua")) {
            	//Masquerade all supposedly merged files except main.lua, to trigger 'require' issues
            	//on player as it would happen on exported project
            	QString fPath=topologicalSort[i].first;
            	QString luaFile=localFileMapReverse[fPath];
            	QString luaFileMasq=luaFile+".gideros_merged";
            	localFileMap.erase(luaFile);
				localFileMap[luaFileMasq] = fPath;
				localFileMapReverse[fPath] = luaFileMasq;
            }

		std::map<QString, std::pair<int, QByteArray> > remoteFileMap;

		ByteBuffer buffer(d.constData(), d.size());

		char chr;
		buffer >> chr;
	
		while (buffer.eob() == false)
		{
			std::string file;
			buffer >> file;

			if (file[0] == 'F')
			{
				int age;
				buffer >> age;

				unsigned char md5[16];
				buffer.get(md5, 16);

				remoteFileMap[file.c_str() + 1] = std::make_pair(age, QByteArray((char*)md5, 16));
			}
			else if (file[0] == 'D')
			{
			}
		}

		// delete unused files
		for (std::map<QString, std::pair<int, QByteArray> >::iterator iter = remoteFileMap.begin(); iter != remoteFileMap.end(); ++iter)
		{
			if (localFileMap.find(iter->first) == localFileMap.end())
			{
				printf("deleting: %s\n", qPrintable(iter->first));
				client_->sendDeleteFile(qPrintable(iter->first));
			}
		}

		// upload files
		QStringList luaFiles;
		QDir path(QFileInfo(projectFileName_).path());
		for (std::map<QString, QString>::iterator iter = localFileMap.begin(); iter != localFileMap.end(); ++iter)
		{
			std::map<QString, std::pair<int, QByteArray> >::iterator riter = remoteFileMap.find(iter->first);
			
			QString localfile = QDir::cleanPath(path.absoluteFilePath(iter->second));

			bool send = false;
			if (riter == remoteFileMap.end())
			{
				printf("always upload: %s\n", qPrintable(iter->first));
				send = true;
			}
			else
			{
				int localage = fileAge(qPrintable(localfile));
				int remoteage = riter->second.first;
				const QByteArray& localmd5 = md5_[iter->second].second;
				const QByteArray& remotemd5 = riter->second.second;

				if (localage < remoteage || localmd5 != remotemd5)
				{
					printf("upload new file: %s\n", qPrintable(iter->first));
					send = true;
				}
			}

			if (send == true)
				fileQueue_.push_back(qMakePair(iter->first, localfile));
			else
				printf("don't upload: %s\n", qPrintable(iter->first));

			if (iter->first.startsWith("_LuaPlugins_/")&&iter->first.endsWith("/_preload.lua"))
				luaFiles << iter->first;
		}

		for (std::size_t i = 0; i < topologicalSort.size(); ++i)
            if ((topologicalSort[i].second == false)&&((!mainluaOnly)||(localFileMapReverse[topologicalSort[i].first]=="main.lua")))
                luaFiles << localFileMapReverse[topologicalSort[i].first];

        if (luaFiles.empty() == false)
			fileQueue_.push_back(qMakePair(luaFiles.join("|"), QString("play")));
	}
	if (data[0]==23) //Breaked
	{
		ByteBuffer buffer(d.constData(), d.size());

		char chr;
		int line;
		std::string source;
		buffer >> chr;
		buffer >> line;
		buffer >> source;
		isBreaked_=true;
        ui.actionResume->setEnabled(true);
        ui.actionStepOver->setEnabled(true);
        ui.actionStepInto->setEnabled(true);
        ui.actionStepReturn->setEnabled(true);
        TextEdit *srcFile=openFile(QString::fromStdString(source.substr(1)),true);
        clearDebugHighlights();
        if (srcFile) {
            srcFile->highlightDebugLine(line-1);
        }
    }
    if (data[0]==24) //Symbol looked up
    {
        ByteBuffer buffer(d.constData(), d.size());

        char chr;
        buffer >> chr;
        QString vtype;
        QVariant value=deserializeValue(buffer,vtype);

        TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

        if (textEdit==lookupSymbolWidget) {
            QString tipInfo;
            if (vtype=="table")
            {
                tipInfo="<table>";
                QMap<QString,QVariant> tmap=value.value<QMap<QString,QVariant>>();
                QMap<QString,QString> smap;
                QStringList klst;
                for (const auto &sk : tmap.keys()) {
                    klst << sk;
                    auto val=tmap[sk];
                    QString vi=val.toString().toHtmlEscaped();
                    if (val.canConvert<QMap<QString,QVariant>>()) {
                        vi="<i>table</i>";
                    }
                    else if (val.canConvert<QPair<QString,QString>>()) {
                        vi="<i>"+val.value<QPair<QString,QString>>().first+"</i>";
                    }
                    else if (val.isNull())
                        vi="<i>nil</i>";
                    smap[sk]=QString("<tr><td><b>%1</b></td><td>%2</td></tr>").arg(sk.toHtmlEscaped()).arg(vi);
                }
                klst.sort();
                for (const auto &sk:klst)
                    tipInfo+=smap[sk];
                tipInfo+="</table>";
            }
            else {
                tipInfo=QString("<i>%1</i> <b>%2</b>").arg(vtype).arg(value.toString().toHtmlEscaped());
            }
            QToolTip::showText(textEdit->mapToGlobal(lookupSymbolPoint),tipInfo,textEdit);
        }
    }
    if (data[0]==31) { //Profiling report
        ByteBuffer buffer(d.constData(), d.size());

        char chr;
        buffer >> chr;
        QString vtype;
        QVariant value=deserializeValue(buffer,vtype);
        if (vtype=="table") {
            ProfilerReport::displayReport(value);
        }
    }
}

void MainWindow::lookupSymbol(QString sym,int x,int y)
{
    if (isBreaked_) {
        lookupSymbolWidget=(TextEdit *) QObject::sender();
        lookupSymbolPoint=QPoint(x,y);
        ByteBuffer buffer;
        buffer << (char) 20;
        buffer << sym.toStdString();
        client_->sendCommand(buffer.data(),buffer.size());
    }
}

void MainWindow::ackReceived(unsigned int id)
{
    Q_UNUSED(id);
/*	std::map<int, QString>::iterator iter = sentMap_.find(id);
	if (iter != sentMap_.end())
	{
		QString name = iter->second;
		if (name.isEmpty() == false)
		{
			outputWidget_->moveCursor(QTextCursor::End);
			outputWidget_->insertPlainText(iter->second + " is uploaded.\n");
		}
		sentMap_.erase(iter);
		if (sentMap_.empty() == true)
		{
			outputWidget_->moveCursor(QTextCursor::End);
			outputWidget_->insertPlainText("Uploading finished!\n");
			sendPlayMessage();
		}
	} */
}
#endif

void MainWindow::showOutputPanel()
{
	outputDock_->show();
}

void MainWindow::showLibraryManager()
{
	libraryDock_->show();
}

bool MainWindow::maybeSave()
{
	QStringList toBeSaved;

	foreach (MdiSubWindow* window, mdiArea_->subWindowList())
	{
		TextEdit* textEdit = qobject_cast<TextEdit*>(window);
		if (textEdit != 0 && textEdit->isModified() == true)
			toBeSaved.push_back(QFileInfo(textEdit->fileName()).fileName());
	}

	if (libraryWidget_->isModified())
		toBeSaved.push_back(QFileInfo(projectFileName_).fileName());

	if (toBeSaved.empty() == false)
	{
		SaveChangesDialog dialog(toBeSaved, this);
		int result = dialog.exec();
		if (result == QDialog::Accepted)			// yes
		{
			saveAll();
			return true;
		}
		else if (result == QDialog::Rejected)		// cancel
		{
			return false;
		}
		else if (result == 2)						// no
		{
			return true;
		}
	}

	return true;
}

void MainWindow::onPreviewRequest(const QString& itemName, const QString& fileName)
{
    Q_UNUSED(itemName);
	QString title = QFileInfo(projectFileName_).dir().relativeFilePath(fileName);
	previewWidget_->setFileName(fileName, title);
}

static void fileCopy(	const QString& srcName,
						const QString& destName,
						const QList<QStringList>& wildcards,
						const QList<QList<QPair<QByteArray, QByteArray> > >& replaceList)
{
    QString srcName2 = QFileInfo(srcName).fileName();

    int match = -1;
	for (int j = 0; j < wildcards.size(); ++j)
	{
		bool submatch = false;
		for (int i = 0; i < wildcards[j].size(); ++i)
		{
            QRegularExpression rx(QRegularExpression::anchoredPattern(QRegularExpression::wildcardToRegularExpression(wildcards[j][i])));
            if (rx.match(srcName2).hasMatch())
			{
				submatch = true;
				break;
			}
		}
		if (submatch)
		{
			match = j;
			break;
		}
	}

	if (match != -1)
	{
		QFile in(srcName);
		if (!in.open(QFile::ReadOnly))
			return;
		QByteArray data = in.readAll();
		in.close();;

		for (int i = 0; i < replaceList[match].size(); ++i)
			data.replace(replaceList[match][i].first, replaceList[match][i].second);

		QFile out(destName);
		if (!out.open(QFile::WriteOnly))
			return;
		out.write(data);
		out.close();
	}
	else
	{
        QFile::remove(destName);
		QFile::copy(srcName, destName);
	}
}

static bool shouldCopy(const QString &fileName, const QStringList &include, const QStringList &exclude)
{
    QString fileName2 = QFileInfo(fileName).fileName();

    bool result = false;

    for (int i = 0; i < include.size(); ++i)
    {
        QRegularExpression rx(QRegularExpression::anchoredPattern(QRegularExpression::wildcardToRegularExpression(include[i])));
        if (rx.match(fileName2).hasMatch())
        {
            result = true;
            break;
        }
    }

    for (int i = 0; i < exclude.size(); ++i)
    {
        QRegularExpression rx(QRegularExpression::anchoredPattern(QRegularExpression::wildcardToRegularExpression(exclude[i])));
        if (rx.match(fileName2).hasMatch())
        {
            result = false;
            break;
        }
    }

    return result;
}


static void copyFolder(	const QDir& sourceDir,
						const QDir& destDir,
						const QList<QPair<QString, QString> >& renameList,
						const QList<QStringList>& wildcards,
                        const QList<QList<QPair<QByteArray, QByteArray> > >& replaceList,
                        const QStringList &include,
                        const QStringList &exclude)
{
	if(!sourceDir.exists())
		return;

	QStringList files;

	files = sourceDir.entryList(QDir::Files | QDir::Hidden);
	for(int i = 0; i < files.count(); i++)
	{
		QString srcName = sourceDir.absoluteFilePath(files[i]);
		QString destFile = files[i];
		for (int i = 0; i < renameList.size(); ++i)
			destFile.replace(renameList[i].first, renameList[i].second);
		QString destName = destDir.absoluteFilePath(destFile);
        if (shouldCopy(srcName, include, exclude))
            fileCopy(srcName, destName, wildcards, replaceList);
	}

	files = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
	for(int i = 0; i < files.count(); i++)
	{
		QDir sourceDir2 = sourceDir;
		bool b1 = sourceDir2.cd(files[i]);

		QDir destDir2 = destDir;
		QString destFile = files[i];
		for (int i = 0; i < renameList.size(); ++i)
			destFile.replace(renameList[i].first, renameList[i].second);
		destDir2.mkdir(destFile);
		bool b2 = destDir2.cd(destFile);

		if (b1 && b2)
			copyFolder(sourceDir2,
					   destDir2,
					   renameList,
					   wildcards,
                       replaceList,
                       include,
                       exclude);
	}
}


#define BYTE_SWAP4(x) \
    (((x & 0xFF000000) >> 24) | \
     ((x & 0x00FF0000) >> 8)  | \
     ((x & 0x0000FF00) << 8)  | \
     ((x & 0x000000FF) << 24))

#define BYTE_SWAP2(x) \
    (((x & 0xFF00) >> 8) | \
     ((x & 0x00FF) << 8))

quint16 _htons(quint16 x) {
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return x;
    }
    else {
        return BYTE_SWAP2(x);
    }
}

quint16 _ntohs(quint16 x) {
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return x;
    }
    else {
        return BYTE_SWAP2(x);
    }
}

quint32 _htonl(quint32 x) {
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return x;
    }
    else {
        return BYTE_SWAP4(x);
    }
}

quint32 _ntohl(quint32 x) {
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return x;
    }
    else {
        return BYTE_SWAP4(x);
    }
}

void MainWindow::exportProject()
{
    ExportProjectDialog dialog(&libraryWidget_->getProjectProperties(), QFileInfo(projectFileName_).dir(), true, this);
	if (dialog.exec() == QDialog::Accepted)
	{
		QString exportType = dialog.exportType();

        QString program = "Tools/gdrexport";
        QStringList arguments;
        QString templatedir;
        QString templatename;
        QString templatenamews;

        if (exportType=="Apple")
        {
            arguments << "-platform" << "ios";
            arguments << "-bundle" << dialog.ios_bundle();
            arguments << "-bundle_atv" << dialog.atv_bundle();
            arguments << "-bundle_macos" << dialog.macos_bundle();
            arguments << "-category" << dialog.macos_category();
            templatedir = "Xcode4";
            templatename = "iOS Template";
            templatenamews = "iOS_Template";
         } else if (exportType=="WinRT") {
            templatedir = "VisualStudio";
            templatename = "WinRT Template";
            templatenamews = "WinRTTemplate";
            arguments << "-platform" << "winrt";
            arguments << "-organization" << dialog.winrt_org();
            arguments << "-package" << dialog.winrt_package();
        } else if (exportType=="Win32") {
            templatedir = "win32";
            templatename = "WindowsDesktopTemplate";
            templatenamews = "WindowsDesktopTemplate";
            arguments << "-platform" << "win32";
        } else if (exportType=="Windows") {
            templatedir = "Qt";
            templatename = "WindowsDesktopTemplate";
            templatenamews = "WindowsDesktopTemplate";
            arguments << "-platform" << "windows";
            arguments << "-organization" << dialog.win_org();
            arguments << "-domain" << dialog.win_domain();
        } else if (exportType=="MacOSX") {
            templatedir = "Qt";
            templatename = "MacOSXDesktopTemplate";
            templatenamews = "MacOSXDesktopTemplate";
            arguments << "-platform" << "macosx";
            arguments << "-organization" << dialog.osx_org();
            arguments << "-domain" << dialog.osx_domain();
            arguments << "-bundle" << dialog.osx_bundle();
            arguments << "-signingId" << dialog.osx_signingId();
            arguments << "-installerId" << dialog.osx_installerId();
            arguments << "-category" << dialog.osx_category();
        } else if (exportType=="GApp") {
            arguments << "-platform" << "gapp";
        } else if (exportType=="Html5") {
            templatedir = "Html5";
            templatename = "Html5";
            templatenamews = "Html5";
            arguments << "-platform" << "html5";
            if (!dialog.html5_host().isEmpty())
                arguments << "-hostname" << dialog.html5_host();
        }
        else
        {
            arguments << "-platform" << exportType;
        }


        QDir dir2 = QDir::currentPath();
        if(!dir2.cd(TEMPLATES_PATH)){
            QMessageBox::information(this, tr("Gideros"), tr("No Templates folder."));
            return;
        }
        if(!dir2.cd(templatedir) || !dir2.cd(templatename)){
            QMessageBox::information(this, tr("Gideros"), tr("No template found."));
            return;
        }

		QSettings settings;
        QString lastExportDirectory = settings.value(exportType+"lastExportDirectory", QString()).toString();

		QString output = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
			lastExportDirectory,
			QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

		if (output.isEmpty() == true)
			return;

        settings.setValue(exportType+"lastExportDirectory", output);

        if (dialog.encryptCode() && dialog.encryptAssets())
        {
            arguments << "-encrypt";
        }
        else if(dialog.encryptCode()){
            arguments << "-encrypt-code";
        }
        else if (dialog.encryptAssets()){
            arguments << "-encrypt-assets";
        }

        if(dialog.exportMode()==1){
            arguments << "-assets-only";
        }
        if(dialog.exportMode()==2){
            arguments << "-player";
        }


        arguments << projectFileName_ << output;

        saveAll();

        QString base = QFileInfo(projectFileName_).baseName();
        QProcess *exportProcess = new QProcess();
        QDir out = QDir(output);
        out.mkdir(base);
        out.cd(base);

        exportProcess->setProgram(program);
        exportProcess->setArguments(arguments);

        QString outPath = QDir::cleanPath(out.absolutePath());
        ExportProgress progress(exportProcess, outPath, this);
    	progress.exec();
    	delete exportProcess;

        //QMessageBox::information(this, tr("Gideros"), tr("Project is exported successfully."));
	}  // if dialog was accepted
}

std::vector<std::pair<QString, QString> > MainWindow::libraryFileList(bool downsizing)
{
    return libraryWidget_->fileList(downsizing,clientIsWeb_);
}

void MainWindow::replace_findNext()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
	{
		findWhat_ = replaceDialog_->findWhat();
		matchCase_ = replaceDialog_->matchCase();
		wholeWord_ = replaceDialog_->wholeWord();
        regexp_ = replaceDialog_->regexp();

		if (findWhat_.isEmpty() == false)
            if (textEdit->findFirst(findWhat_, regexp_, matchCase_, wholeWord_, false, true) == false)
				QMessageBox::information(replaceDialog_, tr("Gideros"), tr("The specified text could not be found."));
	}
}

void MainWindow::replace_replace()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
	{
		findWhat_ = replaceDialog_->findWhat();
		matchCase_ = replaceDialog_->matchCase();
		wholeWord_ = replaceDialog_->wholeWord();
        regexp_ = replaceDialog_->regexp();

		if (findWhat_.isEmpty() == false)
            if (textEdit->replace(findWhat_, replaceDialog_->replaceWith(), regexp_, matchCase_, wholeWord_, false) == false)
				QMessageBox::information(replaceDialog_, tr("Gideros"), tr("The specified text could not be found."));
	}
}


void MainWindow::replace_replaceAll()
{
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit)
	{
		findWhat_ = replaceDialog_->findWhat();
		matchCase_ = replaceDialog_->matchCase();
		wholeWord_ = replaceDialog_->wholeWord();
        regexp_ = replaceDialog_->regexp();

		if (findWhat_.isEmpty() == false)
		{
            int all = textEdit->replaceAll(findWhat_, replaceDialog_->replaceWith(), regexp_, matchCase_, wholeWord_, false);
			QMessageBox::information(replaceDialog_, tr("Gideros"), tr("%1 occurrences were replaced.").arg(all));
		}
	}
}

void MainWindow::findInFiles()
{
	findInFilesDialog_->focusToFindWhat();
	if (findInFilesDialog_->exec() == QDialog::Accepted)
	{
		QString findWhat = findInFilesDialog_->findWhat();
		bool matchCase = findInFilesDialog_->matchCase();
		bool wholeWord = findInFilesDialog_->wholeWord();
        bool regexp = findInFilesDialog_->regexp();

		QDir path(QFileInfo(projectFileName_).path());

		outputWidget_->setUpdatesEnabled(false);

		outputWidget_->clear();

		std::vector<std::pair<QString, QString> > fileListAll = libraryFileList();
		std::vector<std::pair<QString, QString> > fileList;

        QRegularExpression rx(QRegularExpression::wildcardToRegularExpression(findInFilesDialog_->filter(),QRegularExpression::UnanchoredWildcardConversion));
		for (std::size_t i = 0; i < fileListAll.size(); ++i)
		{
			QString filename = fileListAll[i].second;
			
            if (rx.match(filename).hasMatch())
				fileList.push_back(fileListAll[i]);
		}

		outputWidget_->append(tr("Searching %1 files for \"%2\"\n").arg(fileList.size()).arg(findWhat));

		int nfound = 0;
		for (std::size_t i = 0; i < fileList.size(); ++i)
		{
			QString filename = fileList[i].second;
			QString relfilename = fileList[i].first;
			QString absfilename = QDir::cleanPath(path.absoluteFilePath(filename));

			QFile file(absfilename);

			if (file.open(QIODevice::ReadOnly | QIODevice::Text))
			{
				QMap<int, QList<QPair<int, int> > > found;
                QTextStream in(&file);
                in.setEncoding(QStringConverter::Utf8);
#ifdef SCINTILLAEDIT_H
                ScintillaEdit sci;
                sci.setText(in.readAll().toUtf8());

                int start=0;
                int end=sci.textLength();
                sci.setSearchFlags((matchCase?SCFIND_MATCHCASE:0)|(wholeWord?SCFIND_WHOLEWORD:0)|(regexp?SCFIND_REGEXP:0));
                while (true)
                {
                    sci.setTargetRange(start,end);
                    if (sci.searchInTarget(findWhat.size(),findWhat.toUtf8())<0) break;
                    start=sci.targetEnd()+1;
                    int line=sci.lineFromPosition(sci.targetStart());
                    int lineStart=sci.positionFromLine(line);

                    found[line].push_back(qMakePair(sci.targetStart()-lineStart, sci.targetEnd()-lineStart));

                    nfound++;
                }
#else
				QsciScintilla sci;
                sci.setUtf8(true);
				sci.setText(in.readAll());

				int line = -1, index = -1;
                while (sci.findFirst(findWhat, regexp, matchCase, wholeWord, false, true, line, index))
				{
					int lineFrom, indexFrom, lineTo, indexTo;
					sci.getSelection(&lineFrom, &indexFrom, &lineTo, &indexTo);

					found[lineFrom].push_back(qMakePair(indexFrom, indexTo));

					line = lineTo;
					index = indexTo;

					nfound++;
				}

#endif
                if (!found.isEmpty())
                {
                    outputWidget_->append("\n");
                    outputWidget_->append(absfilename + ":\n");

                    QMap<int, QList<QPair<int, int> > >::iterator iter, e = found.end();
                    for (iter = found.begin(); iter != e; ++iter)
                    {
                        int line = iter.key();

                        outputWidget_->append(QString::number(line + 1).rightJustified(5) + ": ");
#ifdef SCINTILLAEDIT_H
                        outputWidget_->append(sci.textRange(sci.positionFromLine(line),sci.lineEndPosition(line)), iter.value());
                        outputWidget_->append("\n");
#else
                        outputWidget_->append(sci.text(line), iter.value());
#endif
                    }
                }
            }
		}

		outputWidget_->append("\n" + tr("%1 matches found.").arg(nfound) + "\n");

		outputWidget_->setUpdatesEnabled(true);
	}
}

void MainWindow::loadMD5()
{
	md5_.clear();

    QDir dir = QFileInfo(projectFileName_).dir();

    QFile file(dir.filePath(".tmp/" + QFileInfo(projectFileName_).completeBaseName() + ".md5"));
	if (!file.open(QIODevice::ReadOnly))
		return;

	QDataStream out(&file);
	out >> md5_;
}

void MainWindow::saveMD5()
{
    QDir dir = QFileInfo(projectFileName_).dir();
    dir.mkdir(".tmp");

    QFile file(dir.filePath(".tmp/" + QFileInfo(projectFileName_).completeBaseName() + ".md5"));
	if (!file.open(QIODevice::WriteOnly))
		return;

	QDataStream out(&file);
	out << md5_;
}

std::vector<std::pair<QString, QString> > MainWindow::updateMD5(bool downsizing)
{
	std::vector<std::pair<QString, QString> > updated;

	std::vector<std::pair<QString, QString> > fileList = libraryFileList(downsizing);

	QDir path(QFileInfo(projectFileName_).path());

	for (std::size_t i = 0; i < fileList.size(); ++i)
	{
		QString filename = fileList[i].second;
		QString absfilename = QDir::cleanPath(path.absoluteFilePath(filename));
		time_t mtime = fileLastModifiedTime(qPrintable(absfilename));

		QMap<QString, QPair<qint64, QByteArray> >::iterator iter = md5_.find(filename);

		if (iter == md5_.end() || mtime != iter.value().first)
		{
			QPair<qint64, QByteArray>& md5 = md5_[filename];

			md5.first = mtime;
			md5.second.resize(16);
			unsigned char* data = (unsigned char*)md5.second.data();
			md5_fromfile(qPrintable(absfilename), data);
//			updated = true;
			updated.push_back(fileList[i]);

			qDebug() << "update md5: " + filename;
		}
	}

	return updated;
}


void MainWindow::openAboutDialog()
{
	AboutDialog dialog(this);
	dialog.exec();
}

void MainWindow::openPreferencesDialog()
{
    PreferencesDialog dialog(this);
    //TextEdit* text_edit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());
    dialog.setMdiArea(mdiArea_);
    dialog.exec();
}

void MainWindow::onInsertIntoDocument(const QString& text)
{
	qDebug() << text;
	TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

	if (textEdit != 0)
	{
#ifdef SCINTILLAEDIT_H
        textEdit->sciScintilla()->replaceSel(text.toUtf8());
        textEdit->sciScintilla()->setFocus(true);
#else
        textEdit->sciScintilla()->replaceSelectedText(text);
        textEdit->sciScintilla()->setFocus();
#endif

		textEdit->setFocus();
	}
}

void MainWindow::exportPack()
{
	QString output = QFileDialog::getExistingDirectory(	this,
														tr("Open Directory"),
														QString(),
														QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);


	if (output.isEmpty() == true)
		return;

	QDir outputDir(output);
}


void MainWindow::downsize(const QString& filename)
{
	QFileInfo fileinfo(filename);

	QString ext = fileinfo.suffix();

	if (ext.isEmpty())
		return;

	QString basename = filename.left(filename.size() - ext.size() - 1);

	std::vector<std::pair<QString, double> > imageScales = libraryWidget_->getProjectProperties().imageScales;

	int match = -1;
	for (size_t i = 0; i < imageScales.size(); ++i)
	{
		if (!imageScales[i].first.isEmpty() && basename.endsWith(imageScales[i].first))
		{
			match = i;
			break;
		}
	}

	QString suffix;
	double scale;
	if (match == -1)
	{
		suffix = "";
		scale = 1;
	}
	else
	{
		suffix = imageScales[match].first;
		scale = imageScales[match].second;
		imageScales.push_back(std::make_pair(QString(), 1.0));
	}

	if (scale == 0)
		return;

	QImage image(filename);

	basename.remove(basename.size() - suffix.size(), suffix.size());

	std::set<QString> saved;

	for (size_t i = 0; i < imageScales.size(); ++i)
	{
        if ((int)i != match)
		{
            if (imageScales[i].second == 0)
				continue;

			double ratio = imageScales[i].second / scale;

			if (ratio >= 1)
				continue;		// do not upscale

			if (saved.find(imageScales[i].first) != saved.end())
				continue;		// do not save more than one time

			int w = image.width() * ratio + 0.5f;
			int h = image.height() * ratio + 0.5f;

			QImage newimage = image.scaled(w, h, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

			int quality = -1;
			if (ext == "jpg" || ext == "jpeg")
				quality = 90;

			newimage.save(basename + imageScales[i].first + "." + ext, 0, quality);

			saved.insert(imageScales[i].first);
		}
	}
}

void MainWindow::helpAndSupport()
{
    QDesktopServices::openUrl(QUrl("http://giderosmobile.com/forum"));
}

void MainWindow::giderosDocumentation()
{
    QDesktopServices::openUrl(QUrl("https://wiki.giderosmobile.com/"));
}


void MainWindow::storeOpenFiles()
{
	if (projectFileName_.isEmpty())
		return;

	QSettings settings;

	QList<QPair<QString, QMap<QString, QVariant> > > projectInfo;

	// read from settings
	{
		QByteArray b = settings.value("projectInfo").toByteArray();

		if (!b.isEmpty())
		{
			QDataStream d(b);
			d >> projectInfo;
		}
	}

	for (int i = projectInfo.size() - 1; i >= 0; --i)
	{
		if (projectInfo[i].first == projectFileName_)
			projectInfo.removeAt(i);
	}

	QMap<QString, QVariant> properties;

	QStringList fileNames;
	QList<QVariant> cursorPositions;
	foreach (MdiSubWindow* window, mdiArea_->subWindowList())
	{
		TextEdit* textEdit = qobject_cast<TextEdit*>(window);

		if (textEdit)
		{
			fileNames << textEdit->fileName();

            int line, index;
#ifdef SCINTILLAEDIT_H
            sptr_t pos=textEdit->sciScintilla()->currentPos();
            index=textEdit->sciScintilla()->column(pos);
            line=textEdit->sciScintilla()->lineFromPosition(pos);
#else
            textEdit->sciScintilla()->getCursorPosition(&line, &index);
#endif
			cursorPositions << QPoint(line, index);

			if (textEdit == mdiArea_->activeSubWindow())
				properties["activeSubWindow"] = textEdit->fileName();
		}
	}
	properties["fileNames"] = fileNames;
	properties["cursorPositions"] = cursorPositions;

	projectInfo.prepend(qMakePair(projectFileName_, properties));

	while (projectInfo.size() > 10)
		projectInfo.removeLast();

	// write settings
	{
		QByteArray b;
		QDataStream d(&b, QIODevice::WriteOnly);
		d << projectInfo;

		settings.setValue("projectInfo", b);
	}
}

void MainWindow::restoreOpenFiles()
{
	QSettings settings;

	QList<QPair<QString, QMap<QString, QVariant> > > projectInfo;

	// read from settings
	{
		QByteArray b = settings.value("projectInfo").toByteArray();

		if (!b.isEmpty())
		{
			QDataStream d(b);
			d >> projectInfo;
		}
	}

	for (int i = 0; i < projectInfo.size(); ++i)
	{
		if (projectInfo[i].first == projectFileName_)
		{
			QMap<QString, QVariant>& properties = projectInfo[i].second;

			QStringList fileNames = properties["fileNames"].toStringList();
			QList<QVariant> cursorPositions = properties["cursorPositions"].toList();
			QString activeSubWindow = properties["activeSubWindow"].toString();

            TextEdit* active = nullptr;
			for (int i = 0; i < fileNames.size(); ++i)
			{
				TextEdit* textEdit = openFile(fileNames[i], true);
				if (textEdit)
				{
					QPoint p = cursorPositions[i].toPoint();
#ifdef SCINTILLAEDIT_H
                    textEdit->sciScintilla()->setCurrentPos(textEdit->sciScintilla()->positionFromPoint(p.x(),p.y()));
#else
                    textEdit->sciScintilla()->setCursorPosition(p.x(), p.y());
#endif

					if (fileNames[i] == activeSubWindow)
						active = textEdit;
				}
			}

			if (active)
				mdiArea_->setActiveSubWindow(active);

			break;
		}
	}
}


void MainWindow::stdoutToOutput()
{
    QString output = makeProcess_->readAllStandardOutput();
    outputWidget_->append(output);
}

void MainWindow::stderrToOutput()
{
    QString output = makeProcess_->readAllStandardError();
    outputWidget_->append(output);
}

void MainWindow::makeStarted()
{
    outputWidget_->clear();
    QString dt = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    outputWidget_->append((tr("---------- Compile started at %1 ----------") + "\n").arg(dt));
}

void MainWindow::makeFinished()
{
    outputWidget_->append((tr("========== Compile finished ==========") + "\n"));
}

void MainWindow::clearOutput(){
    outputWidget_->clear();
}

void MainWindow::searchOutput( const QString &text){
    outputWidget_->search(text);
}

void MainWindow::on_actionUI_Theme_triggered()
{
    QSettings settings;

    QString themePath = QDir::currentPath()+"/Resources/Themes/";
    QDir dir(themePath);

    QString themeFile = QFileDialog::getOpenFileName(this, tr("Open UI Theme"),
        themePath, tr("UI Theme files (*.qss)"));

    QFile file(themeFile);
    QString theme;

    if (file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd()) theme = in.readAll();
        settings.setValue("uiTheme", themeFile);
        qApp->setStyleSheet(theme);
    }
}

void MainWindow::on_actionEditor_Theme_triggered()
{
    QString themePath = QDir::currentPath()+"/Resources/Themes/";
    QDir dir(themePath);

    QString theme = QFileDialog::getOpenFileName(this, tr("Open Editor Theme"),
                                                themePath, tr("Editor Theme files (*.ini)"));
    if (theme == "") return;

    QSettings settings;
    settings.setValue("editorTheme", theme);

    reloadProject();
}


void MainWindow::on_actionUI_and_Editor_Theme_triggered()
{
    QSettings settings;

    QString themePath = QDir::currentPath()+"/Resources/Themes/";
    QDir dir(themePath);

    QString themeFile = QFileDialog::getOpenFileName(this, tr("Open UI and Editor Theme"),
        themePath, tr("UI Theme files (*.qss)"));

    if (themeFile == "") return;

    QFile file(themeFile);
    QString theme;

    if (file.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QTextStream in(&file);
        while (!in.atEnd()) theme = in.readAll();
        settings.setValue("uiTheme", themeFile);
        qApp->setStyleSheet(theme);
    }

    themeFile.chop(3);
    themeFile.append("ini");
    QFile eFile(themeFile);
    if (eFile.exists())
    {
        settings.setValue("editorTheme", themeFile);
        reloadProject();
    }
}

void MainWindow::on_actionReset_UI_and_Editor_Theme_triggered()
{
    QSettings settings;
    settings.setValue("editorTheme", "");
    settings.setValue("uiTheme", "");
    qApp->setStyleSheet("");

    reloadProject();
}

void MainWindow::on_actionFold_Unfold_All_triggered()
{
    TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

    if (textEdit)
    {
#ifdef SCINTILLAEDIT_H
        int lineCount = textEdit->sciScintilla()->lineCount();
        bool expanding = true;

        for (int line = 0; line < lineCount; line++)
        {
           if (textEdit->sciScintilla()->foldLevel(line) & SC_FOLDLEVELHEADERFLAG)
           {
               expanding = !textEdit->sciScintilla()->foldExpanded(line);
               break;
           }
        }
        textEdit->sciScintilla()->foldAll(expanding?SC_FOLDACTION_EXPAND:SC_FOLDACTION_CONTRACT);
#else

        textEdit->sciScintilla()->foldAll(true);
#endif
    }
}

void MainWindow::on_actionFold_Unfold_Top_triggered()
{
    TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

    if (textEdit)
    {
#ifdef SCINTILLAEDIT_H
        int lineCount = textEdit->sciScintilla()->lineCount();
        bool expanding = true;

        for (int line = 0; line < lineCount; line++)
        {
           if (textEdit->sciScintilla()->foldLevel(line) & SC_FOLDLEVELHEADERFLAG)
           {
               expanding = !textEdit->sciScintilla()->foldExpanded(line);
               break;
           }
        }
        for (int line = 0; line < lineCount; line++)
        {
            int level = textEdit->sciScintilla()->foldLevel(line);
            if (!(level & SC_FOLDLEVELHEADERFLAG)) //Not foldable
                continue;

            if (SC_FOLDLEVELBASE == (level & SC_FOLDLEVELNUMBERMASK))
                textEdit->sciScintilla()->foldLine(line, expanding?SC_FOLDACTION_EXPAND:SC_FOLDACTION_CONTRACT);
        }
#else
        textEdit->sciScintilla()->foldAll(false);
#endif
    }
}

void MainWindow::on_actionBlock_un_comment_triggered()
{
    TextEdit* textEdit = qobject_cast<TextEdit*>(mdiArea_->activeSubWindow());

    if (textEdit)
        textEdit->BlockComment();
}

void MainWindow::keyPressEvent(QKeyEvent * event)
{
#if defined(Q_OS_WIN)
    if (event->key() == Qt::Key_Control)
    {
        changeTabKeyPressState_ = 1;
    }

    if (!inChooseTab_)
    {
        if (event->key() == Qt::Key_Tab)
        {
            if (changeTabKeyPressState_ == 1)
            {
                changeTabKeyPressState_ = 2;

                inChooseTab_ = true;

                event->accept();
                tabListWidget_ = new QListWidget();

                QList<QString> tabFilenameList = mdiArea_->tabFilenameOrderList();
                for (int i = 0; i < tabFilenameList.size(); ++i)
                {
                    QString relativePath = makeProjectRelativePath(tabFilenameList[i]);
                    tabListWidget_->addItem(new QListWidgetItem(relativePath));
                }
                tabListWidget_->setWindowFlags(Qt::Tool|Qt::FramelessWindowHint);
                tabListWidget_->setFocusPolicy(Qt::NoFocus);

#if QT_VERSION >= 0x040500
                tabListWidget_->setWindowFlags(Qt::ToolTip|Qt::WindowStaysOnTopHint);
#else
                tabListWidget_->setWindowFlags(Qt::Tool|Qt::FramelessWindowHint);
#endif
                tabListWidget_->setFocusProxy(this);
                tabListWidget_->setCurrentRow(std::min<int>(tabListWidget_->count(), 1));

                tabListWidget_->show();
            }
            else
            {
            }
        }
    }
    else
    {// Ctrl+Tab,Tab,Tab... : next of current selection
        int index = tabListWidget_->currentRow();
        if (index + 1 < tabListWidget_->count())
        {
            index++;
        }
        else
        {
            index = 0;
        }
        tabListWidget_->setCurrentRow(index);
    }

#endif
    if (!event->isAccepted())
    {
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::keyReleaseEvent(QKeyEvent * event)
{
#if defined(Q_OS_WIN)
    if (event->key() == Qt::Key_Control)
    {
        changeTabKeyPressState_ = 0;

        if (inChooseTab_)
        {
            inChooseTab_ = false;
            //event->accept();

            QListWidgetItem* selectedItem = tabListWidget_->item(tabListWidget_->currentRow());
            if (selectedItem != NULL)
            {
                mdiArea_->changeCurrentTabByFilename(projectDirectory() + selectedItem->text());
            }

            tabListWidget_->hide();
            tabListWidget_->deleteLater();
            tabListWidget_ = NULL;
        }
    }

#endif
    if (!event->isAccepted())
    {
        QMainWindow::keyReleaseEvent(event);
    }
}
