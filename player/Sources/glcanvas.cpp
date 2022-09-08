#include "glcanvas.h"
#include "luaapplication.h"
#include "libnetwork.h"
#include "debugging.h"
#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include <QMouseEvent>
#include <QTouchEvent>
#include <QColorSpace>
#include <QList>
#include "platform.h"
#include "refptr.h"
#include <stack>
#include <string>
#include <QTimer>
#include <QDesktopServices>
#include <bytebuffer.h>
#include <event.h>
#include <application.h>
#include <QKeyEvent>
#include <QDebug>
#include <keycode.h>
#include <gevent.h>
#include <ginput.h>
#include <ginput-qt.h>
#include <glog.h>
#include <gstdio.h>
#include <gpath.h>
#include <gvfs-native.h>
#include "mainwindow.h"

#include <QOpenGLWindow>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QScreen>
#include <QStandardPaths>
#include <screen.h>
#include <Qt>

extern "C" {
int g_getFps();
void g_setFps(int fps);
}

class QtScreenManager : public ScreenManager {
public:
	QOpenGLWidget *master_;
	QtScreenManager(QOpenGLWidget *master);
	virtual Screen *openScreen(Application *application,int id);
	virtual void screenDestroyed();
};

class QtScreen : public Screen,protected QOpenGLWindow {
	virtual void tick();
protected:
	virtual void setVisible(bool);
	bool closed_;
public:
	virtual void setSize(int w,int h);
	virtual void getSize(int &w,int &h);
	virtual void setPosition(int w,int h);
	virtual void getPosition(int &w,int &h);
	virtual void setState(int state);
	virtual int getState();
	virtual void getMaxSize(int &w,int &h);
	virtual int getId();
	bool event(QEvent* ev);
	QtScreen(Application *application);
	~QtScreen();
};

bool QtScreen::event(QEvent* ev)
{
        if (ev->type() == QEvent::PlatformSurface) {
        	if (((QPlatformSurfaceEvent *)ev)->surfaceEventType()==QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed)
        	{
        		setContent(NULL);
        		closed_=true;
        	}
        	else
        		closed_=false;
        }
        // Make sure the rest of events are handled
        return QOpenGLWindow::event(ev);
}

void QtScreen::tick()
{
	if (isExposed())
	{
		Matrix4 m;
		QOpenGLContext *c=((QtScreenManager *)(ScreenManager::manager))->master_->context();
		c->makeCurrent(this);
		c->functions()->glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
		draw(m);
		c->swapBuffers(this);
	}
}

void QtScreen::setSize(int w,int h)
{
	resize(w,h);
}

void QtScreen::getSize(int &w,int &h)
{
	w=width();
	h=height();
}

void QtScreen::setState(int state)
{
	Qt::WindowState st=Qt::WindowNoState;
	if (state&MINIMIZED) st=Qt::WindowMinimized;
	if (state&MAXIMIZED) st=Qt::WindowMaximized;
	if (state&FULLSCREEN) st=Qt::WindowFullScreen;
	setWindowState(st);
}

int QtScreen::getState()
{
	Qt::WindowState state=windowState();
	int s=NORMAL;
	if (state==Qt::WindowMinimized) s=MINIMIZED;
	if (state==Qt::WindowMaximized) s=MAXIMIZED;
	if (state==Qt::WindowFullScreen) s=FULLSCREEN;
	if (!isVisible()) s|=HIDDEN;
	if (closed_) s|=CLOSED;
	return s;
}

void QtScreen::getMaxSize(int &w,int &h)
{
	QSize s=screen()->size();
	w=s.width();
	h=s.height();
}

void QtScreen::setPosition(int w,int h)
{
	QOpenGLWindow::setPosition(w,h);
}

void QtScreen::getPosition(int &w,int &h)
{
	w=x();
	h=y();
}

int QtScreen::getId()
{
	return 0;
}

void QtScreen::setVisible(bool visible)
{
	if (visible) show(); else hide();
    QOpenGLContext *c=((QtScreenManager *)(ScreenManager::manager))->master_->context();
    c->makeCurrent(this);
}

QtScreen::QtScreen(Application *application) : Screen(application), QOpenGLWindow()
{
	closed_=true;
}

QtScreen::~QtScreen()
{
}


QtScreenManager::QtScreenManager(QOpenGLWidget *master)
{
	master_=master;
}

void QtScreenManager::screenDestroyed()
{
	if (!QOpenGLContext::currentContext())
		master_->makeCurrent();
}


Screen *QtScreenManager::openScreen(Application *application,int id)
{
    G_UNUSED(id);
    Screen *s=new QtScreen(application);
    master_->makeCurrent();
    return s;
}

static int __mkdir(const char* path) {
#ifdef _WIN32
	return _mkdir(path);
#else
	return mkdir(path, 0755);
#endif
}

static Server* g_server = NULL;
QString GLCanvas::appPackage;
bool GLCanvas::EnableVSYNC=false;
bool GLCanvas::TabletActive=false;

static void printToServer(const char* str, int len, void* data) {
    G_UNUSED(data);
    unsigned int size = 1 + ((len < 0) ? strlen(str) : len) + 1;
	char* buffer = (char*) malloc(size);

	buffer[0] = gptPrint;
	memcpy(buffer + 1, str, size - 2);
	buffer[size - 1] = 0;

	g_server->sendData(buffer, size);

	free(buffer);
}

static void deltree(const char* dir) {
	std::stack<std::string> stack;

	std::string directory = dir;
	char back = directory[directory.size() - 1];
	if (back == '/' || back == '\\')
		directory.resize(directory.size() - 1);

	stack.push(directory);

	while (!stack.empty()) {
		std::string dir = stack.top();
		stack.pop();

		std::vector<std::string> files, directories;
		getDirectoryListing(dir.c_str(), &files, &directories);

		for (std::size_t i = 0; i < files.size(); ++i)
			remove((dir + "/" + files[i]).c_str());

		for (std::size_t i = 0; i < directories.size(); ++i)
			stack.push(dir + "/" + directories[i]);
	}
}

GLCanvas::GLCanvas(QWidget *parent) :
		QOpenGLWidget(parent) {
	setAttribute(Qt::WA_AcceptTouchEvents);
    for( int i=1; i<=4; ++i )
        mouseButtonPressed_[i] = false;
    setMouseTracking(true);
	//setFocusPolicy(Qt::WheelFocus);

	/*
	 QGLFormat formatGL;
	 formatGL.setVersion(2, 0); // Version : 2.0
	 formatGL.setDoubleBuffer(true); // Double Buffer : Activé
	 formatGL.setDepthBufferSize(24);
	 formatGL.setStencilBufferSize(8);
	 formatGL.setSwapInterval(1); // Synchronisation du Double Buffer et de l'écran
	 this->setFormat(formatGL);
	 */

    //setUpdateBehavior(QOpenGLWidget::PartialUpdate); // Prevent QT from calling glClear by itself
    isPlayer_ = true;
    exportedApp_=false;

    if (!appPackage.isEmpty())
    	setExportedApp(true);

	/*
	 startTimer(1);
	 setAccessFileCallback(accessFileCallback_s, this);

	 platformImplementation_ = new PlatformImplementation(application_);
	 setPlatformInterface(platformImplementation_);
	 */
}

void GLCanvas::setupCanvas() {
	setupProperties();

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
	timer->start(1);
	ScreenManager::manager=new QtScreenManager(this);
}

GLCanvas::~GLCanvas() {
	makeCurrent();
	if (running_ == true) {
		Event event(Event::APPLICATION_EXIT);
		GStatus status;
		application_->broadcastEvent(&event, &status);

		if (status.error()) {
            if (isPlayer_) {
                printToServer(status.errorString(), -1, NULL);
                printToServer("\n", -1, NULL);
            }
            else{
                errorDialog_.appendString(status.errorString());
                errorDialog_.show();
            }
		}
	}

	/*
	 setAccessFileCallback(NULL, NULL);
	 application_->broadcastApplicationWillTerminate();
	 */

	timerEvent(0); // TODO: network bufferinda ne kalmissa send etmek icin baska bi fonksiyon yaz

	application_->deinitialize();
	delete application_;

	/*
	 setPlatformInterface(NULL);
	 delete platformImplementation_;
	 */

	delete ScreenManager::manager;
	ScreenManager::manager=NULL;

	if (isPlayer_) {
		LuaDebugging::studioLink(NULL);
		delete server_;
		g_server = 0;
	}
}

void GLCanvas::setupProperties() {
    //isPlayer_ = appPackage.isEmpty();

	/*
	 QFile Props(":/Resources/properties.bin");
	 QFile LuaFiles(":/Resources/luafiles.txt");

	 if(Props.exists() && LuaFiles.exists()){
	 isPlayer_ = false;
	 play(QDir(":/Resources"));
	 }
	 */

    //exportedApp_ = !appPackage.isEmpty();

	application_ = new LuaApplication;

	application_->setPlayerMode(isPlayer_);
	application_->enableExceptions();
    if (isPlayer_) {
        application_->setPrintFunc(printToServer);
		server_ = new Server(0, ::getDeviceName().c_str()); //Default port
		LuaDebugging::studioLink(server_);

		// set the global server var to use in print to server function
		g_server = server_;
    }

	running_ = false;

	clock_ = iclock();

	deviceScale_ = devicePixelRatio();

	setHardwareOrientation(ePortrait);
	setResolution(320, 480, false);
	setFps(60);
	setScale(1);
	setDrawInfos(false);

	float canvasColor[3] = { 1, 1, 1 };
	setCanvasColor(canvasColor);

	float infoColor[3] = { 0, 0, 0 };
	setInfoColor(infoColor);
}

void GLCanvas::setupApplicationProperties() {
	application_->setHardwareOrientation(orientation_);
	application_->setResolution(width_, height_);
	application_->setScale(scale_);
}

void GLCanvas::initializeGL() {
	QSurfaceFormat f=QOpenGLContext::currentContext()->format();
	sync_=f.swapInterval();
	qDebug() << "GLFMT:SWAP:" << sync_;
	qDebug() << "GLFMT:STENCIL:" << f.stencilBufferSize();
	qDebug() << "GLFMT:DEPTH:" << f.depthBufferSize();
	qDebug() << "GLFMT:BUFFER:" << f.swapBehavior();
    qDebug() << "GLFMT:COLORSPACE:" << f.colorSpace();
    qDebug() << "GLFMT:SAMPLES:" << f.samples();
    if (QOpenGLContext::currentContext()->hasExtension("WGL_EXT_swap_control")) {
		QFunctionPointer getSwapInterval= QOpenGLContext::currentContext()->getProcAddress("wglGetSwapIntervalEXT");
		QFunctionPointer setSwapInterval= QOpenGLContext::currentContext()->getProcAddress("wglSwapIntervalEXT");
		qDebug() << "GLFMT:FSWAP:" << ((void *)getSwapInterval) << ((void *)setSwapInterval);
		if (getSwapInterval&&setSwapInterval)
		{
			int (*getSwap)()=(int (*)())getSwapInterval;
			qDebug() << "GLFMT:NSWAP:" << getSwap();
			sync_=getSwap();

		}
	}
	else
		qDebug() << "GLFMT:WGL Swap control not available";
	if (!EnableVSYNC) //If VSYNC wasn't requested, use standard timed method
		sync_=0;
	application_->initialize();
	setupApplicationProperties();
}

/*
 TODO: renderScene'e full try-catch icine aldigimiz icin. mesela enterFrame event'inde bi exception olursa,
 geriye kalanlar calistirilmadigi icin ekrana hicbirsey cizilmiyor. renderScene'inin icini parcalamak lazim.
 her logical kismi ayri try-catch icine alabiliriz belki.
 TODO: bu hata olayini iyi dusunmek lazim. bi timer event'inde hata olursa, o timer tekrar cagirilmiyordu. sebebi bu yuzden olsa gerek.
 TODO: belki de lua'yi exception'li derlemek lazim. koda baktigimda oyle birseyi destekliyordu
 */

bool GLCanvas::checkLuaError(GStatus status)
{
	if (status.error()) {
		running_ = false;

        if (isPlayer_) {
            printToServer(status.errorString(), -1, NULL);
            printToServer("\n", -1, NULL);
        }
        else{
            errorDialog_.appendString(status.errorString());
            errorDialog_.show();
        }

		application_->deinitialize();
		application_->initialize();
		return true;
	}
	return false;
}

void GLCanvas::paintGL() {

    bool dnow;
    if (!application_->onDemandDraw(dnow)) {
        GStatus status;
        application_->enterFrame(&status);

        checkLuaError(status);
    }

	application_->clearBuffers();
	application_->renderScene();
	// if not running or if drawInfos enabled, and is not an exported app
	if ((!running_ || drawInfos_) && !exportedApp_) {
//		glMatrixMode(GL_MODELVIEW);
//		glLoadIdentity();
//		glScalef(1.f / scale_, 1.f / scale_, 1);
		Matrix4 ident;
		ShaderEngine::Engine->setModel(ident);

		int lWidth = application_->getLogicalWidth();
		int lHeight = application_->getLogicalHeight();
		float scale =
				round(
						(float) 100
								/ (float) ((float) scale_
										* (float) devicePixelRatio()));

		void drawInfoResolution(int width, int height, int scale, int lWidth,
				int lHeight, bool drawRunning, float canvasColor[3],
				float infoColor[3],int ho, int ao, float fps, float cpu);

		drawInfoResolution(width_, height_, scale, lWidth, lHeight,
				running_ && drawInfos_, canvasColor_, infoColor_, (int) application_->hardwareOrientation(), (int) application_->orientation(),
				1.0/application_->meanFrameTime_,1-(application_->meanFreeTime_/application_->meanFrameTime_));
	}
}

// TODO: TimerEvent.TIMER'da bi exception olursa, o event bir daha cagirilmiyor. Bunun nedeini bulmak lazim
void GLCanvas::timerEvent(QTimerEvent *){
    /*
    platformImplementation_->openUrls();
    printf(".");
    printf("%d\n", Referenced::instanceCount);
    */
	makeCurrent();
    if(!projectDir_.isEmpty()){
        play(QDir(projectDir_));
        projectDir_.clear();
    }

    if (!appPackage.isEmpty()&&(!running_)) {
    	play(appPackage);
     }

    if(isPlayer_){
        int dataTotal = 0;

        while(true){
            if(!projectDir_.isEmpty()){
                play(QDir(projectDir_));
                projectDir_.clear();
            }

            int dataSent0 = server_->dataSent();
            int dataReceived0 = server_->dataReceived();

            NetworkEvent event;
            server_->tick(&event);

            /*
            if (event.eventCode != eNone)
                printf("%s\n", eventCodeString(event.eventCode));
            */

            int dataSent1 = server_->dataSent();
            int dataReceived1 = server_->dataReceived();

            if(isPlayer_ && event.eventCode == eDataReceived){
                const std::vector<char>& data = event.data;
                LuaDebugging::studioCommand(data);
                switch(data[0]){
                    case gptMakeDir:
                    {
                        std::string folderName = &data[1];
                        __mkdir(g_pathForFile(folderName.c_str()));
                        break;
                    }

                    case gptWriteFile:
                    {
                        std::string fileName = &data[1];
                        FILE* fos = fopen(g_pathForFile(fileName.c_str()), "wb");
                        size_t pos = 1 + fileName.size() + 1;
                        if(data.size() > pos)
                            fwrite(&data[pos], data.size() - pos, 1, fos);
                        fclose(fos);
                        allResourceFiles.insert(fileName);
                        calculateMD5(fileName.c_str());
                        saveMD5();
                        break;
                    }

                    case gptPlay:
                    {
                        glog_v("play message is received\n");

                        running_ = true;

                        //accessedResourceFiles.clear();

                        dir_ = QDir::temp();
                        dir_.mkdir("gideros");
                        dir_.cd("gideros");
                        dir_.mkdir(projectName_);
                        dir_.cd(projectName_);

                        QByteArray ba;
                        QDataStream datastream(&ba,QIODevice::ReadWrite);
                        datastream.writeRawData((const char*)&data[0], data.size());
                        QFile file(dir_.absolutePath()+"/luafiles.txt");
                        file.open(QIODevice::WriteOnly);
                        file.write(ba);
                        file.close();

                        loadFiles(data);

                        break;
                    }

                    case gptStop:
                    {
                        glog_v("stop message is received\n");

                        if (running_ == true)
                        {
                            Event event(Event::APPLICATION_EXIT);
                            GStatus status;
                            application_->broadcastEvent(&event, &status);

                            if (status.error())
                            {
                                //errorDialog_.appendString(status.errorString());
                                //errorDialog_.show();
                                printToServer(status.errorString(), -1, NULL);
                                printToServer("\n", -1, NULL);
                            }
                        }

                        running_ = false;

                        application_->deinitialize();
                        application_->initialize();
                        setupApplicationProperties();
                        break;
                    }

                    /*
                    case 5:
                    {
                        // deleteFiles();
                        break;
                    }
                    */

                    case gptGetFileList:
                    {
                        sendFileList();
                        break;
                    }

                    case gptSetProjectName:
                    {
                        ByteBuffer buffer(&data[0], data.size());

                        char chr;
                        buffer >> chr;

                        std::string str;
                        buffer >> str;

                        projectName_ = str.c_str();

                        if(projectName_.isEmpty() == false){
                            dir_ = QDir::temp();
                            dir_.mkdir("gideros");
                            dir_.cd("gideros");
                            dir_.mkdir(projectName_);
                            dir_.cd(projectName_);

                            md5filename_ = dir_.absoluteFilePath("md5.txt").toStdString().c_str();
                            loadMD5();

                            dir_.mkdir("documents");
                            dir_.mkdir("temporary");
                            dir_.mkdir("resource");

                            resourceDirectory_ = dir_.absoluteFilePath("resource").toStdString();
                            documentsDirectory_ = dir_.absoluteFilePath("documents").toStdString();
                            temporaryDirectory_ = dir_.absoluteFilePath("temporary").toStdString();
                            setDocumentsDirectory(documentsDirectory_.c_str());
                            setTemporaryDirectory(temporaryDirectory_.c_str());
                            setResourceDirectory(resourceDirectory_.c_str());
                        }

                        emit projectNameChanged(projectName_);
                        break;
                    }

                    case gptDeleteFile:
                    {
                        ByteBuffer buffer(&data[0], data.size());

                        char chr;
                        buffer >> chr;

                        std::string fileName;
                        buffer >> fileName;

                        remove(g_pathForFile(fileName.c_str()));

                        {
                            std::set<std::string>::iterator iter = allResourceFiles.find(fileName);
                            if (iter != allResourceFiles.end())		// this if statement is unnecessary, but we put it "ne olur ne olmaz"
                                allResourceFiles.erase(iter);
                        }

                        {
                            std::map<std::string, std::vector<unsigned char> >::iterator iter = md5_.find(fileName);
                            if (iter != md5_.end())
                            {
                                md5_.erase(iter);
                                saveMD5();
                            }
                        }

                        break;
                    }

                    case gptSetProperties:
                    {
                        dir_ = QDir::temp();
                        dir_.mkdir("gideros");
                        dir_.cd("gideros");
                        dir_.mkdir(projectName_);
                        dir_.cd(projectName_);

                        QByteArray ba;
                        QDataStream datastream(&ba,QIODevice::ReadWrite);
                        datastream.writeRawData((const char*)&data[0], data.size());
                        QFile file(dir_.absolutePath()+"/properties.bin");
                        file.open(QIODevice::WriteOnly);
                        file.write(ba);
                        file.close();

                        loadProperties(data);

                        break;
                    }
                }
            }

            int dataDelta = (dataSent1 - dataSent0) + (dataReceived1 - dataReceived0);
            dataTotal += dataDelta;

            if(dataDelta == 0 || dataTotal > 1024)
                break;
        }
    }

    ScreenManager::manager->tick();
    bool dnow;
    if (application_->onDemandDraw(dnow)) {
        GStatus status;
        application_->enterFrame(&status);

        checkLuaError(status);
    }
    else
        dnow=true;
    if (dnow)
        update();
}

void GLCanvas::play(QDir directory){
    QFile file(directory.absolutePath()+"/properties.bin");
    QFile luafiles(directory.absolutePath()+"/luafiles.txt");

    if(file.exists() && luafiles.exists()){
        if(running_ == true){
            Event event(Event::APPLICATION_EXIT);
            GStatus status;
            application_->broadcastEvent(&event, &status);
            running_ = false;

            if(status.error()){
                if (isPlayer_) {
                    printToServer(status.errorString(), -1, NULL);
                    printToServer("\n", -1, NULL);
                }
                else{
                    errorDialog_.appendString(status.errorString());
                    errorDialog_.show();
                }
                return;
            }
        }

        projectName_ = directory.dirName();
        emit projectNameChanged(projectName_);

        QString docLocation;
        #if defined(Q_OS_MAC)
        docLocation = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
        #elif defined(RASPBERRY_PI)
        docLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
        #else
        docLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        #endif
        directory.mkpath(docLocation);
        if(exportedApp_){
            resourceDirectory_ = directory.absoluteFilePath("resource").toStdString().c_str();
            QString tempLocation = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
            directory.mkpath(tempLocation);
            documentsDirectory_ = docLocation.toStdString();
            temporaryDirectory_ = tempLocation.toStdString();
        }else{
            dir_ = QDir::temp();
            dir_.mkdir("gideros");
            dir_.cd("gideros");
            dir_.mkdir(projectName_);
            dir_.cd(projectName_);
            dir_.mkdir("temporary");

            temporaryDirectory_ = dir_.absoluteFilePath("temporary").toStdString();
            //This code path is used when openging or restarting a project from the player menu, which assumes all files are in a single directory
 /*           dir_ = QDir(docLocation);
            dir_.mkdir("gideros");
            dir_.cd("gideros");
            dir_.mkdir("documents");*/

            resourceDirectory_ = dir_.absoluteFilePath("resource").toStdString();
            documentsDirectory_ = dir_.absoluteFilePath("documents").toStdString();
        }

        setDocumentsDirectory(documentsDirectory_.c_str());
        setTemporaryDirectory(temporaryDirectory_.c_str());
        setResourceDirectory(resourceDirectory_.c_str());

        file.open(QIODevice::ReadOnly);
        QByteArray ba = file.readAll();
        file.close();
        std::vector<char> data(ba.data(), ba.data() + ba.size());

        loadProperties(data);

        running_ = true;

        luafiles.open(QIODevice::ReadOnly);

        if(exportedApp_){
            std::vector<std::string> lines;

            QTextStream in(&luafiles);
            while(!in.atEnd()){
                QString line = in.readLine();

                if(!line.isEmpty()){
                    lines.push_back(line.toStdString());
                }
            }

            playLoadedFiles(lines);

        }else{
            QByteArray bas = luafiles.readAll();
            luafiles.close();
            std::vector<char> data2(bas.data(), bas.data() + bas.size());

            loadFiles(data2);
        }
    }
    else{
        if(exportedApp_){
            errorDialog_.appendString("An error occured, please reinstall the application");
            errorDialog_.show();

            if(errorDialog_.exec() == QDialog::Rejected){
                exit(0);
            }

        }else{
            errorDialog_.appendString("Please relaunch project from Gideros Studio");
            errorDialog_.show();
        }
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
	} else {
		return BYTE_SWAP2(x);
	}
}

quint16 _ntohs(quint16 x) {
	if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
		return x;
	} else {
		return BYTE_SWAP2(x);
	}
}

quint32 _htonl(quint32 x) {
	if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
		return x;
	} else {
		return BYTE_SWAP4(x);
	}
}

quint32 _ntohl(quint32 x) {
	if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
		return x;
	} else {
		return BYTE_SWAP4(x);
	}
}

void GLCanvas::play(QString gapp) {
	QFileInfo gappname(gapp);
	QFile gpkg(gapp);
	qint64 pksz = gpkg.size();
    if (pksz < 16) {
        glog_d("GAPP-FILE:Invalid size %s,%d", gapp.toUtf8().constData(),pksz);
        return; //Invalid file size
    }
    if (!gpkg.open(QIODevice::ReadOnly)) {
        glog_d("GAPP-FILE:Can't open %s", gapp.toUtf8().constData());
        return; //Not openable
    }
	struct {
		quint32 flistOffset;
		quint32 version;
		char signature[8];
	} tlr;
	gpkg.seek(pksz - 16);
	gpkg.read((char *) &tlr, 16);
	tlr.version = _ntohl(tlr.version);
	tlr.flistOffset = _ntohl(tlr.flistOffset);
	if ((!strncmp(tlr.signature, "GiDeRoS", 7)) && (tlr.version == 0)) {
		glog_v("GAPP-ARCH: %s", gapp.toStdString().c_str());
		gvfs_setZipFile(gapp.toStdString().c_str());
		char *buffer = (char *) malloc(pksz - tlr.flistOffset);
		gpkg.seek(tlr.flistOffset);
		gpkg.read(buffer, pksz - tlr.flistOffset);
		int offset = 0;
		while (offset < (pksz - tlr.flistOffset)) {
			int plen = strlen(buffer + offset);
			if (!plen)
				break; //End of list
			quint32 foffset;
			quint32 fsize;
			memcpy(&foffset, buffer + offset + plen + 1, sizeof(quint32));
			memcpy(&fsize, buffer + offset + plen + 1 + sizeof(quint32),
					sizeof(quint32));
			foffset = _ntohl(foffset);
			fsize = _ntohl(fsize);
			const char *norm = gpath_normalizeArchivePath(buffer + offset);
			gvfs_addFile(norm, 0, foffset, fsize);
			glog_d("GAPP-FILE: %s,%d,%d", norm, foffset, fsize);
			offset += (plen + 1 + 2 * sizeof(quint32));
		}
		free(buffer);
	} else
		glog_w("GAPP: Invalid signature/version");
	gpkg.close();

	if (running_ == true) {
		Event event(Event::APPLICATION_EXIT);
		GStatus status;
		application_->broadcastEvent(&event, &status);
		running_ = false;

		if (status.error()) {
            if (isPlayer_) {
                printToServer(status.errorString(), -1, NULL);
                printToServer("\n", -1, NULL);
            }
            else{
                errorDialog_.appendString(status.errorString());
                errorDialog_.show();
                if (!appPackage.isEmpty())
                	exit(1);
            }
			return;
		}
	}

	projectName_ = gappname.baseName();
	emit projectNameChanged(projectName_);

	dir_ = QDir::temp();
	dir_.mkdir("gideros");
	dir_.cd("gideros");
	dir_.mkdir(projectName_);
	dir_.cd(projectName_);
	dir_.mkdir("documents");
	dir_.mkdir("temporary");

	resourceDirectory_ = "";
	documentsDirectory_ = dir_.absoluteFilePath("documents").toStdString();
	temporaryDirectory_ = dir_.absoluteFilePath("temporary").toStdString();

	setDocumentsDirectory(documentsDirectory_.c_str());
	setTemporaryDirectory(temporaryDirectory_.c_str());
	setResourceDirectory(resourceDirectory_.c_str());

	G_FILE* fis = g_fopen("properties.bin", "rb");
	if (fis) {
		g_fseek(fis, 0, SEEK_END);
		int len = g_ftell(fis);
		g_fseek(fis, 0, SEEK_SET);
		glog_d("properties.bin len=%d", len);
		std::vector<char> data(len);
		g_fread(&data[0], 1, len, fis);
		g_fclose(fis);

		glog_d("GAPP: Setting properties");
		loadProperties(data);
		glog_d("GAPP: Properties loaded");
	} else
		glog_w("GAPP: Missing properties.bin");

	running_ = true;
	fis = g_fopen("luafiles.txt", "rb");
	if (fis) {
		g_fseek(fis, 0, SEEK_END);
		int len = g_ftell(fis);
		g_fseek(fis, 0, SEEK_SET);
		std::vector<char> data2(len);
		g_fread(&data2[0], 1, len, fis);
		g_fclose(fis);

		std::vector<std::string> lines;
		QTextStream in(QByteArray(&data2[0], len));
		while (!in.atEnd()) {
			QString line = in.readLine();
			if (!line.isEmpty()) {
				lines.push_back(line.toStdString());
			}
		}

		playLoadedFiles(lines);
	} else
		glog_w("GAPP: Missing luafiles.txt");
}

void GLCanvas::loadProperties(std::vector<char> data) {
	ByteBuffer buffer(&data[0], data.size());
	makeCurrent();
	if (!exportedApp_) {
		char chr;
		buffer >> chr;
	}

	int scaleMode, logicalWidth, logicalHeight, windowWidth, windowHeight;
	buffer >> scaleMode;
	buffer >> logicalWidth;
	buffer >> logicalHeight;

	application_->deinitialize();
	application_->initialize();
	setupApplicationProperties();
//				application_->orientationChange(orientation_);
	application_->setLogicalDimensions(logicalWidth, logicalHeight);
	application_->setLogicalScaleMode((LogicalScaleMode) scaleMode);

	int scaleCount;
	buffer >> scaleCount;
	std::vector<std::pair<std::string, float> > imageScales(scaleCount);
	for (int i = 0; i < scaleCount; ++i) {
		buffer >> imageScales[i].first;
		buffer >> imageScales[i].second;
	}

	application_->setImageScales(imageScales);

	int orientation;
	buffer >> orientation;
	application_->setOrientation((Orientation) orientation);


	application_->getApplication()->setDeviceOrientation(
			(Orientation) orientation);

	int fps;
	buffer >> fps;
    //setFps(fp); //XXX: This is windows player, let the fps setting be controlled by menu for ease of use

	int retinaDisplay;
	buffer >> retinaDisplay;

	int autorotation;
	buffer >> autorotation;

	int mouseToTouch;
	buffer >> mouseToTouch;
	ginput_setMouseToTouchEnabled(mouseToTouch);

	int touchToMouse;
	buffer >> touchToMouse;
	ginput_setTouchToMouseEnabled(touchToMouse);

	int mouseTouchOrder;
	buffer >> mouseTouchOrder;
	ginput_setMouseTouchOrder(mouseTouchOrder);

	buffer >> windowWidth;
	buffer >> windowHeight;
    if (windowWidth == 0 && windowHeight == 0) {
        windowWidth = logicalWidth;
        windowHeight = logicalHeight;
        //setFixedSize(false);
    }else{
        //width0_ = windowWidth;
        //height0_ = windowHeight;
    }
    if (!appPackage.isEmpty()) {
        MainWindow::getInstance()->setOrientation((Orientation) orientation);
        setHardwareOrientation((Orientation) orientation);
    	setWindowSize(windowWidth, windowHeight);
    }

}

void GLCanvas::playLoadedFiles(std::vector<std::string> luafiles) {
	GStatus status;
	makeCurrent();
	for (std::size_t i = 0; i < luafiles.size(); ++i) {
		application_->loadFile(luafiles[i].c_str(), &status);
		if (status.error())
			break;
	}

	if (!status.error()) {
		Event event(Event::APPLICATION_START);
		application_->broadcastEvent(&event, &status);
	}

	if (status.error()) {
		running_ = false;

        if (isPlayer_) {
            printToServer(status.errorString(), -1, NULL);
            printToServer("\n", -1, NULL);
        }
        else{
            errorDialog_.appendString(status.errorString());
            errorDialog_.show();
        }
		application_->deinitialize();
		application_->initialize();
	}
}

void GLCanvas::loadFiles(std::vector<char> data) {
	std::vector<std::string> luafiles;

	ByteBuffer buffer(&data[0], data.size());

	char chr;
	buffer >> chr;

	while (buffer.eob() == false) {
		std::string str;
		buffer >> str;
		luafiles.push_back(str);
	}

	playLoadedFiles(luafiles);
}

void GLCanvas::mousePressEvent(QMouseEvent* event) {
    if ((event->source() == Qt::MouseEventSynthesizedBySystem)||TabletActive) {
      event->accept();
	  return;
	}
    if (event->button() <= 4){
        mouseButtonPressed_[event->button()] = true;
    }
    Qt::KeyboardModifiers qmod=event->modifiers();
    int m=0;
    if (qmod&Qt::ShiftModifier) m|=GINPUT_SHIFT_MODIFIER;
    if (qmod&Qt::AltModifier) m|=GINPUT_ALT_MODIFIER;
    if (qmod&Qt::ControlModifier) m|=GINPUT_CTRL_MODIFIER;
    if (qmod&Qt::MetaModifier) m|=GINPUT_META_MODIFIER;
    ginputp_mouseDown(event->position().x() * deviceScale_, event->position().y() * deviceScale_, event->button(),m);
}

void GLCanvas::mouseMoveEvent(QMouseEvent* event) {
   if ((event->source() == Qt::MouseEventSynthesizedBySystem)||TabletActive) {
	  event->accept();
	  return;
	}
   Qt::KeyboardModifiers qmod=event->modifiers();
   int m=0;
   if (qmod&Qt::ShiftModifier) m|=GINPUT_SHIFT_MODIFIER;
   if (qmod&Qt::AltModifier) m|=GINPUT_ALT_MODIFIER;
   if (qmod&Qt::ControlModifier) m|=GINPUT_CTRL_MODIFIER;
   if (qmod&Qt::MetaModifier) m|=GINPUT_META_MODIFIER;
    bool mousePressed = false;
    for( int i=1; i<=4; ++i )
    {
        if( mouseButtonPressed_[i])
        {
            ginputp_mouseMove(event->position().x() * deviceScale_, event->position().y() * deviceScale_, i,m);
            mousePressed = true;
        }
    }

    if (mousePressed == false) {
        ginputp_mouseHover(event->position().x() * deviceScale_, event->position().y() * deviceScale_, 0,m);
    }
}

void GLCanvas::mouseReleaseEvent(QMouseEvent* event) {
    if ((event->source() == Qt::MouseEventSynthesizedBySystem)||TabletActive) {
      event->accept();
	  return;
	}
    if (event->button() <= 4){
        if(mouseButtonPressed_[event->button()]) mouseButtonPressed_[event->button()] = false;
    }
    Qt::KeyboardModifiers qmod=event->modifiers();
    int m=0;
    if (qmod&Qt::ShiftModifier) m|=GINPUT_SHIFT_MODIFIER;
    if (qmod&Qt::AltModifier) m|=GINPUT_ALT_MODIFIER;
    if (qmod&Qt::ControlModifier) m|=GINPUT_CTRL_MODIFIER;
    if (qmod&Qt::MetaModifier) m|=GINPUT_META_MODIFIER;
    ginputp_mouseUp(event->position().x() * deviceScale_, event->position().y() * deviceScale_, event->button(),m);
}

void GLCanvas::wheelEvent(QWheelEvent* event) {
    Qt::KeyboardModifiers qmod=event->modifiers();
    int m=0;
    if (qmod&Qt::ShiftModifier) m|=GINPUT_SHIFT_MODIFIER;
    if (qmod&Qt::AltModifier) m|=GINPUT_ALT_MODIFIER;
    if (qmod&Qt::ControlModifier) m|=GINPUT_CTRL_MODIFIER;
    if (qmod&Qt::MetaModifier) m|=GINPUT_META_MODIFIER;
    ginputp_mouseWheel(event->position().x() * deviceScale_, event->position().y() * deviceScale_,
            event->buttons(), event->angleDelta().y(),m);
}

void GLCanvas::keyPressEvent(QKeyEvent* event) {
    Qt::KeyboardModifiers qmod=event->modifiers();
    int m=0;
    if (qmod&Qt::ShiftModifier) m|=GINPUT_SHIFT_MODIFIER;
    if (qmod&Qt::AltModifier) m|=GINPUT_ALT_MODIFIER;
    if (qmod&Qt::ControlModifier) m|=GINPUT_CTRL_MODIFIER;
    if (qmod&Qt::MetaModifier) m|=GINPUT_META_MODIFIER;
	if (!event->isAutoRepeat())
		ginputp_keyDown(event->key(),m);
	if (!event->text().isEmpty()) //Don't bother propagating empty key strokes
		ginputp_keyChar(event->text().toUtf8().constData());
}

void GLCanvas::keyReleaseEvent(QKeyEvent* event) {
	if (event->isAutoRepeat())
		return;

    Qt::KeyboardModifiers qmod=event->modifiers();
    int m=0;
    if (qmod&Qt::ShiftModifier) m|=GINPUT_SHIFT_MODIFIER;
    if (qmod&Qt::AltModifier) m|=GINPUT_ALT_MODIFIER;
    if (qmod&Qt::ControlModifier) m|=GINPUT_CTRL_MODIFIER;
    if (qmod&Qt::MetaModifier) m|=GINPUT_META_MODIFIER;
	ginputp_keyUp(event->key(),m);
}

void GLCanvas::tabletEvent(QTabletEvent* event) {

    int xs[1];
    int ys[1];
    int ids[1];
    float pressures[1];
    int touchTypes[1];

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    xs[0] = event->x() * deviceScale_;
    ys[0] = event->y() * deviceScale_;
#else
    xs[0] = event->position().x() * deviceScale_;
    ys[0] = event->position().y() * deviceScale_;
#endif    
    ids[0] = 0;
    pressures[0] = event->pressure();
    touchTypes[0] = 3;
    Qt::KeyboardModifiers qmod=event->modifiers();
    int m=0;
    if (qmod&Qt::ShiftModifier) m|=GINPUT_SHIFT_MODIFIER;
    if (qmod&Qt::AltModifier) m|=GINPUT_ALT_MODIFIER;
    if (qmod&Qt::ControlModifier) m|=GINPUT_CTRL_MODIFIER;
    if (qmod&Qt::MetaModifier) m|=GINPUT_META_MODIFIER;


    if(event->type() == QEvent::TabletPress){
        ginputp_touchesBegin(xs[0], ys[0], ids[0], pressures[0], touchTypes[0], 1, xs, ys, ids, pressures, touchTypes,m,event->button());

    }else if(event->type() == QEvent::TabletMove){
        ginputp_touchesMove(xs[0], ys[0], ids[0], pressures[0], touchTypes[0], 1, xs, ys, ids, pressures, touchTypes,m,event->buttons());

    }else if(event->type() == QEvent::TabletRelease){
        ginputp_touchesEnd(xs[0], ys[0], ids[0], pressures[0], touchTypes[0], 1, xs, ys, ids, pressures, touchTypes,m,event->button());
    }
    event->accept();
}

bool GLCanvas::event(QEvent *event){
    if (event->type() == QEvent::TouchBegin || event->type() == QEvent::TouchUpdate || event->type() == QEvent::TouchEnd || event->type() == QEvent::TouchCancel)
    {
        QTouchEvent* touchEvent = (QTouchEvent*)event;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        const QList<QTouchEvent::TouchPoint> &list = touchEvent->touchPoints();
#else
        const QList<QTouchEvent::TouchPoint> &list = touchEvent->points();
#endif
        int size = list.count();

        Qt::KeyboardModifiers qmod=touchEvent->modifiers();
        int m=0;
        if (qmod&Qt::ShiftModifier) m|=GINPUT_SHIFT_MODIFIER;
        if (qmod&Qt::AltModifier) m|=GINPUT_ALT_MODIFIER;
        if (qmod&Qt::ControlModifier) m|=GINPUT_CTRL_MODIFIER;
        if (qmod&Qt::MetaModifier) m|=GINPUT_META_MODIFIER;

        int xs[size];
        int ys[size];
        int ids[size];
        float pressures[size];
        int touchTypes[size];

        //Mark all previously known touches
        quint32 curIds=0;
        for( int i=0; i<size; ++i )
        {
            QTouchEvent::TouchPoint p = list[i];
            int tid=touchIdMap[p.id()];
            if (tid)
            	curIds|=(1<<(tid-1));
        }

        //Free up dropped touches
        touchIdUsed&=curIds;
        QList<int> knownIds=touchIdMap.keys();
        for (int i=0;i<knownIds.size();i++)
        {
        	int ctid=touchIdMap[knownIds[i]];
        	if (ctid&&(!(curIds&(1<<(ctid-1)))))
        		touchIdMap.remove(knownIds[i]);
        }

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        int ttype=0;
        switch (touchEvent->pointerType()) {
            case QPointingDevice::PointerType::Generic: ttype=2; break;
            case QPointingDevice::PointerType::Finger: ttype=0; break;
            case QPointingDevice::PointerType::Pen: ttype=1; break;
            case QPointingDevice::PointerType::Eraser: ttype=4; break;    // eraser end of a stylus
            case QPointingDevice::PointerType::Cursor: ttype=5; break;    // digitizer with crosshairs
            default: ttype=0;
        }
#endif

        for( int i=0; i<size; ++i )
        {
            QTouchEvent::TouchPoint p = list[i];
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
            xs[i] = p.pos().x() * deviceScale_;
            ys[i] = p.pos().y() * deviceScale_;
            touchTypes[i] = p.flags();
#else
            xs[i] = p.position().x() * deviceScale_;
            ys[i] = p.position().y() * deviceScale_;
            touchTypes[i] = ttype;
#endif
            pressures[i] = p.pressure();
            int tid=touchIdMap[p.id()];
            if (!tid) //New touch, allocate a touch id
            {
            	for (int k=0;k<32;k++)
            		if (!(touchIdUsed&(1<<k)))
            		{
            			touchIdMap[p.id()]=k+1;
            			touchIdUsed|=(1<<k);
            			tid=k+1;
            			break;
            		}
            }
            ids[i] = tid-1;
            curIds|=(1<<(tid-1));
        }

        for( int i=0; i<size; ++i )
        {
            QTouchEvent::TouchPoint p = list[i];
            int tid=touchIdMap[p.id()]-1;
            if(event->type() == QEvent::TouchCancel){
                ginputp_touchesCancel(p.position().x() * deviceScale_, p.position().y() * deviceScale_, tid, p.pressure(), touchTypes[i], size, xs, ys, ids, pressures, touchTypes,m,GINPUT_NO_BUTTON);
            }
            else if(p.state() == QEventPoint::Pressed){
                ginputp_touchesBegin(p.position().x() * deviceScale_, p.position().y() * deviceScale_,tid,p.pressure(), touchTypes[i], size, xs, ys, ids, pressures, touchTypes,m,GINPUT_NO_BUTTON);
            }
            else if(p.state() == QEventPoint::Updated){
                ginputp_touchesMove(p.position().x() * deviceScale_, p.position().y() * deviceScale_,tid,p.pressure(), touchTypes[i], size, xs, ys, ids, pressures, touchTypes,m,GINPUT_NO_BUTTON);
            }
            else if(p.state() == QEventPoint::Released){
                ginputp_touchesEnd(p.position().x() * deviceScale_, p.position().y() * deviceScale_,tid,p.pressure(), touchTypes[i], size, xs, ys, ids, pressures, touchTypes,m,GINPUT_NO_BUTTON);
            }
        }
        touchIdUsed=curIds;
        return true;
    }
    else if(event->type() == QEvent::MouseButtonPress){
        mousePressEvent((QMouseEvent*)event);
        return true;
    }
    else if(event->type() == QEvent::MouseMove){
        mouseMoveEvent((QMouseEvent*)event);
        return true;
    }
    else if(event->type() == QEvent::MouseButtonRelease){
        mouseReleaseEvent((QMouseEvent*)event);
        return true;
    }
    else if(event->type() == QEvent::Wheel){
        wheelEvent((QWheelEvent*)event);
        return true;
    }
    else if(event->type() == QEvent::KeyPress){
        //keyPressEvent((QKeyEvent*) event);
        return true;
    }
    else if(event->type() == QEvent::ShortcutOverride){
        keyPressEvent((QKeyEvent*) event);
        return false;
    }
    else if(event->type() == QEvent::KeyRelease){
        keyReleaseEvent((QKeyEvent*) event);
        return true;
    }
    else if(event->type() == QEvent::Timer){
        timerEvent((QTimerEvent *) event);
        return true;
    }
    else if(event->type() == QEvent::FocusIn){
        if(running_){
        	makeCurrent();
            Event event(Event::APPLICATION_RESUME);
            GStatus status;
            application_->broadcastEvent(&event, &status);
            checkLuaError(status);
        }
    }
    else if(event->type() == QEvent::WindowUnblocked){
        if(running_){
        	makeCurrent();
            Event event(Event::APPLICATION_RESUME);
            GStatus status;
            application_->broadcastEvent(&event, &status);
            checkLuaError(status);
        }
    }
    else if(event->type() == QEvent::FocusOut){
        if(running_){
        	makeCurrent();
            Event event(Event::APPLICATION_SUSPEND);
            GStatus status;
            application_->broadcastEvent(&event, &status);
            checkLuaError(status);
        }
        for( int i=1; i<=4; ++i )
            mouseButtonPressed_[i] = false;
    }
    else if(event->type() == QEvent::WindowBlocked){
        if(running_){
        	makeCurrent();
            Event event(Event::APPLICATION_SUSPEND);
            GStatus status;
            application_->broadcastEvent(&event, &status);
            checkLuaError(status);
        }
        for( int i=1; i<=4; ++i )
            mouseButtonPressed_[i] = false;
    }
    return QOpenGLWidget::event(event);
}

void GLCanvas::onTimer() {
    double deltat = 1.0 / g_getFps();
	if (sync_) //if we are synced, try to go a little faster and let vsync regulate things for us
		deltat-=0.005;

	/*
	 if(deltat == 0){
	 timerEvent(0);
	 return;
	 }
	 */

	bool update = false;

	double now = iclock();
	while (now > clock_ + deltat) {
		update = true;
		clock_ += deltat;
	}

	if (update)
		timerEvent(0);
}

void GLCanvas::deleteFiles() {
	deltree(resourceDirectory_.c_str());
//	fileList_.clear();
}

void GLCanvas::sendFileList() {
	allResourceFiles.clear();

	ByteBuffer buffer;

	// type(byte) 6
	// D or F, file (zero ended string), age (int)
	// D or F, file (zero ended string), age (int)
	// ....

	buffer.append((char) gptFileList);

	std::vector<std::string> files, directories;
	getDirectoryListingR(resourceDirectory_.c_str(), &files, &directories);

	for (std::size_t i = 0; i < files.size(); ++i) {
		buffer.append('F');
		buffer.append(files[i]);
		int age = fileAge(g_pathForFile(files[i].c_str()));
		buffer.append(age);

		std::map<std::string, std::vector<unsigned char> >::iterator iter =
				md5_.find(files[i]);
		if (iter == md5_.end()) {
			calculateMD5(files[i].c_str());
			saveMD5();
			iter = md5_.find(files[i]);
		}
		buffer.append(&iter->second[0], 16);

		allResourceFiles.insert(files[i]);
	}

	for (std::size_t i = 0; i < directories.size(); ++i) {
		buffer.append('D');
		buffer.append(directories[i]);
	}

	server_->sendData(buffer.data(), buffer.size());
}

void GLCanvas::sendRun() {
	ByteBuffer buffer;
	buffer.append((char) gptRunning);
	server_->sendData(buffer.data(), buffer.size());
}

void GLCanvas::loadMD5() {
	/*
	 md5_.clear();

	 FILE* fis = fopen(md5filename_.c_str(), "rt");

	 if (fis == NULL)
	 return;

	 std::string filename;

	 char line[1024];
	 int nline = 0;
	 while (true)
	 {
	 line[0] = '\0';
	 fgets(line, 1024, fis);

	 int len = strlen(line);

	 if (len > 0 && line[len - 1] == 0x0a)
	 {
	 line[len - 1] = '\0';
	 len--;
	 }

	 if (len > 0 && line[len - 1] == 0x0d)
	 {
	 line[len - 1] = '\0';
	 len--;
	 }

	 if (len == 0)
	 break;

	 if ((nline % 2) == 0)
	 filename = line;
	 else
	 md5_[filename] = line;

	 nline++;
	 }

	 fclose(fis);
	 */

	md5_.clear();

	FILE* fis = fopen(md5filename_.c_str(), "rb");

	if (fis == NULL)
		return;

	int nfiles;
	fread(&nfiles, sizeof(int), 1, fis);

	for (int i = 0; i < nfiles; ++i) {
		int strlen;
		fread(&strlen, sizeof(int), 1, fis);

		char* buffer = (char*) malloc(strlen);
		fread(buffer, 1, strlen, fis);
		std::string filename(buffer, strlen);
		free(buffer);

		std::vector<unsigned char> md5(16);
		fread(&md5[0], 1, 16, fis);

		md5_[filename] = md5;
	}

//	printMD5();
}

void GLCanvas::saveMD5() {
	/*
	 FILE* fos = fopen(md5filename_.c_str(), "wt");

	 if (fos == NULL)
	 return;

	 std::map<std::string, std::string>::iterator iter, end = md5_.end();
	 for (iter = md5_.begin(); iter != end; ++iter)
	 {
	 fprintf(fos, "%s\n", iter->first.c_str());
	 fprintf(fos, "%s\n", iter->second.c_str());
	 }
	 fclose(fos);
	 */

	FILE* fos = fopen(md5filename_.c_str(), "wb");
	if (fos == NULL)
		return;

	int nfiles = md5_.size();
	fwrite(&nfiles, sizeof(int), 1, fos);

	int i = 0;
	std::map<std::string, std::vector<unsigned char> >::iterator iter, end =
			md5_.end();
	for (iter = md5_.begin(); iter != end; ++iter, ++i) {
		int strlen = iter->first.size();
		fwrite(&strlen, sizeof(int), 1, fos);
		fwrite(iter->first.c_str(), 1, strlen, fos);
		fwrite(&iter->second[0], 1, 16, fos);
	}

	fclose(fos);

//	printMD5();
}

void GLCanvas::calculateMD5(const char* file) {
	std::vector<unsigned char> md5(16);
	if (md5_fromfile(g_pathForFile(file), &md5[0]))
		md5_[file] = md5;
}

void GLCanvas::printMD5() {
	std::map<std::string, std::vector<unsigned char> >::iterator iter, end =
			md5_.end();
	for (iter = md5_.begin(); iter != end; ++iter) {
		qDebug() << iter->first.c_str();
		const std::vector<unsigned char>& m = iter->second;

		char buffer[33];
		sprintf(buffer,
				"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
				m[0], m[1], m[2], m[3], m[4], m[5], m[6], m[7], m[8], m[9],
				m[10], m[11], m[12], m[13], m[14], m[15]);

		qDebug() << buffer;
	}
}

void GLCanvas::setScale(float scale) {
	scale_ = scale;
	deviceScale_ = (float) scale * (float) devicePixelRatio();

	if (application_->isInitialized())
		application_->setScale(scale_);
}

void GLCanvas::setFps(int fps) {
	g_setFps(fps);
	clock_ = iclock();
}

void GLCanvas::setDrawInfos(bool drawInfos) {
	drawInfos_ = drawInfos;
}

void GLCanvas::setCanvasColor(float canvasColor[3]) {
	canvasColor_[0] = canvasColor[0];
	canvasColor_[1] = canvasColor[1];
	canvasColor_[2] = canvasColor[2];
}

void GLCanvas::setInfoColor(float infoColor[3]) {
	infoColor_[0] = infoColor[0];
	infoColor_[1] = infoColor[1];
	infoColor_[2] = infoColor[2];
}

void GLCanvas::setHardwareOrientation(Orientation orientation) {
	orientation_ = orientation;

	if (application_->isInitialized()) {
		application_->setHardwareOrientation(orientation_);
//		application_->orientationChange(orientation_);
	}
}

Orientation GLCanvas::getHardwareOrientation() {
	return orientation_;
}

void GLCanvas::setResolution(int width, int height,bool event) {
	width_ = width;
	height_ = height;

	if (application_->isInitialized())
		application_->setResolution(width_, height_);

    if(event&&running_){
        Event event(Event::APPLICATION_RESIZE);
        GStatus status;
        application_->broadcastEvent(&event, &status);
        checkLuaError(status);
    }
}

void GLCanvas::setExportedApp(bool exportedApp) {
	exportedApp_ = exportedApp;
    isPlayer_ = false;
}

void GLCanvas::printToOutput(const char* text) {
    if (isPlayer_) {
        printToServer(text, -1, NULL);
        printToServer("\n", -1, NULL);
    }
}

/*
 void PlatformImplementation::openUrl(const char* url)
 {
 urls_.push_back(QUrl(url, QUrl::TolerantMode));
 }

 void PlatformImplementation::openUrls()
 {
 for (std::size_t i = 0; i < urls_.size(); ++i)
 QDesktopServices::openUrl(urls_[i]);
 urls_.clear();
 }

 void PlatformImplementation::memoryWarning()
 {
 application_->broadcastMemoryWarning();
 }
 */

/*
 void GLCanvas::accessFileCallback_s(FileType type, const char* filename, void* data)
 {
 GLCanvas* that = static_cast<GLCanvas*>(data);
 that->accessFileCallback(type, filename);
 }

 void GLCanvas::accessFileCallback(FileType type, const char* filename)
 {
 if (type == eResourceFile)
 accessedResourceFiles.insert(filename);
 }
 */

/*
 static void getDirectoryListing(const std::string& dir, std::vector<std::string>* files, std::vector<std::string>* directories)
 {
 files->clear();
 directories->clear();

 WIN32_FIND_DATAA ffd;
 HANDLE hFind;

 std::string dirstar = dir + "/*";

 hFind = FindFirstFileA(dirstar.c_str(), &ffd);

 do
 {
 if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
 if (strcmp(ffd.cFileName, ".") == 0 || strcmp(ffd.cFileName, "..") == 0)
 continue;

 if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
 directories->push_back(ffd.cFileName);
 else
 files->push_back(ffd.cFileName);
 } while (FindNextFileA(hFind, &ffd) != 0);

 FindClose(hFind);
 }
 */
