#ifndef RASPBERRY_PI
#include <GL/glew.h>
#endif

#include "glcanvas.h"
#include "luaapplication.h"
#include "libnetwork.h"

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#endif

#include <QMouseEvent>
#include <QTouchEvent>
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

static int __mkdir(const char* path) {
#ifdef _WIN32
	return _mkdir(path);
#else
	return mkdir(path, 0755);
#endif
}

static Server* g_server = NULL;
QString GLCanvas::appPackage;

static void printToServer(const char* str, int len, void* data) {
	unsigned int size = 1 + ((len < 0) ? strlen(str) : len) + 1;
	char* buffer = (char*) malloc(size);

	buffer[0] = 4;
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
		QGLWidget(parent) {
	setAttribute(Qt::WA_AcceptTouchEvents);
    for( int i=1; i<=4; ++i )
    {
        mouseButtonPressed_[i] = false;
    }
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
    isPlayer_ = true;

	setupProperties();

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
	timer->start(1);

	/*
	 startTimer(1);
	 setAccessFileCallback(accessFileCallback_s, this);

	 platformImplementation_ = new PlatformImplementation(application_);
	 setPlatformInterface(platformImplementation_);
	 */
}

GLCanvas::~GLCanvas() {
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

	if (isPlayer_) {
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

		// set the global server var to use in print to server function
		g_server = server_;
    }

	running_ = false;

	clock_ = iclock();

	if (!exportedApp_) {
		deviceScale_ = devicePixelRatio();

		setHardwareOrientation(ePortrait);
		setResolution(320, 480, false);
		setFps(60);
		setScale(1);
		setDrawInfos(false);
	}

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
#ifndef RASPBERRY_PI
	glewInit();
#endif

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
	GStatus status;
	application_->enterFrame(&status);

	checkLuaError(status);

	application_->clearBuffers();
	application_->renderScene();

	// if not running or if drawInfos enabled, and is not an exported app
	if ((!running_ || drawInfos_) && !exportedApp_) {
//		glMatrixMode(GL_MODELVIEW);
//		glLoadIdentity();
//		glScalef(1.f / scale_, 1.f / scale_, 1);

		int lWidth = application_->getLogicalWidth();
		int lHeight = application_->getLogicalHeight();
		float scale =
				round(
						(float) 100
								/ (float) ((float) scale_
										* (float) devicePixelRatio()));

		void drawInfoResolution(int width, int height, int scale, int lWidth,
				int lHeight, bool drawRunning, float canvasColor[3],
				float infoColor[3]);

		drawInfoResolution(width_, height_, scale, lWidth, lHeight,
				running_ && drawInfos_, canvasColor_, infoColor_);
	}
}

// TODO: TimerEvent.TIMER'da bi exception olursa, o event bir daha cagirilmiyor. Bunun nedeini bulmak lazim
void GLCanvas::timerEvent(QTimerEvent *){
    /*
    platformImplementation_->openUrls();
    printf(".");
    printf("%d\n", Referenced::instanceCount);
    */
    if(!projectDir_.isEmpty()){
        play(QDir(projectDir_));
        projectDir_.clear();
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

                switch(data[0]){
                    case 0:
                    {
                        std::string folderName = &data[1];
                        __mkdir(g_pathForFile(folderName.c_str()));
                        break;
                    }

                    case 1:
                    {
                        std::string fileName = &data[1];
                        FILE* fos = fopen(g_pathForFile(fileName.c_str()), "wb");
                        int pos = 1 + fileName.size() + 1;
                        if(data.size() > pos)
                            fwrite(&data[pos], data.size() - pos, 1, fos);
                        fclose(fos);
                        allResourceFiles.insert(fileName);
                        calculateMD5(fileName.c_str());
                        saveMD5();
                        break;
                    }

                    case 2:
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

                    case 3:
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

                    case 7:
                    {
                        sendFileList();
                        break;
                    }

                    case 8:
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

                            resourceDirectory_ = dir_.absoluteFilePath("resource").toStdString().c_str();

                            setDocumentsDirectory(dir_.absoluteFilePath("documents").toStdString().c_str());
                            setTemporaryDirectory(dir_.absoluteFilePath("temporary").toStdString().c_str());
                            setResourceDirectory(resourceDirectory_.c_str());
                        }

                        emit projectNameChanged(projectName_);
                        break;
                    }

                    case 9:
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

                    case 11:
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

        const char* documentsDirectory;
        const char* temporaryDirectory;

        if(exportedApp_){
            resourceDirectory_ = directory.absoluteFilePath("resource").toStdString().c_str();
            QString docLocation;
            #if defined(Q_OS_MAC)
                docLocation = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
            #elif defined(RASPBERRY_PI)
                docLocation = QStandardPaths::writableLocation(QStandardPaths::DataLocation);
            #else
                docLocation = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            #endif
            QString tempLocation = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
            directory.mkpath(docLocation);
            directory.mkpath(tempLocation);
            documentsDirectory = docLocation.toStdString().c_str();
            setDocumentsDirectory(documentsDirectory);
            temporaryDirectory = tempLocation.toStdString().c_str();
            setTemporaryDirectory(temporaryDirectory);
        }else{
            dir_ = QDir::temp();
            dir_.mkdir("gideros");
            dir_.cd("gideros");
            dir_.mkdir(projectName_);
            dir_.cd(projectName_);
            dir_.mkdir("documents");
            dir_.mkdir("temporary");

            resourceDirectory_ = dir_.absoluteFilePath("resource").toStdString().c_str();
            documentsDirectory = dir_.absoluteFilePath("documents").toStdString().c_str();
            setDocumentsDirectory(documentsDirectory);
            temporaryDirectory = dir_.absoluteFilePath("temporary").toStdString().c_str();
            setTemporaryDirectory(temporaryDirectory);
        }

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
	if (pksz < 16)
		return; //Invalid file size
	if (!gpkg.open(QIODevice::ReadOnly))
		return; //Not openable
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
            }
			return;
		}
	}

	projectName_ = gappname.baseName();
	emit projectNameChanged(projectName_);

	const char* documentsDirectory;
	const char* temporaryDirectory;

	dir_ = QDir::temp();
	dir_.mkdir("gideros");
	dir_.cd("gideros");
	dir_.mkdir(projectName_);
	dir_.cd(projectName_);
	dir_.mkdir("documents");
	dir_.mkdir("temporary");

	resourceDirectory_ = "";
	documentsDirectory = qPrintable(dir_.absoluteFilePath("documents"));
	temporaryDirectory = qPrintable(dir_.absoluteFilePath("temporary"));

	setDocumentsDirectory(documentsDirectory);
	setTemporaryDirectory(temporaryDirectory);
	setResourceDirectory("");

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

}

void GLCanvas::playLoadedFiles(std::vector<std::string> luafiles) {
	GStatus status;
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
    if (event->source() == Qt::MouseEventSynthesizedBySystem) {
	  event->accept();
	  return;
	}
    if (event->button() <= 4){
        mouseButtonPressed_[event->button()] = true;
    }
    ginputp_mouseDown(event->x() * deviceScale_, event->y() * deviceScale_, event->button());
}

void GLCanvas::mouseMoveEvent(QMouseEvent* event) {
   if (event->source() == Qt::MouseEventSynthesizedBySystem) {
	  event->accept();
	  return;
	}
    bool mousePressed = false;
    for( int i=1; i<=4; ++i )
    {
        if( mouseButtonPressed_[i])
        {
            ginputp_mouseMove(event->x() * deviceScale_, event->y() * deviceScale_, i);
            mousePressed = true;
        }
    }

    if (mousePressed == false) {
        ginputp_mouseHover(event->x() * deviceScale_, event->y() * deviceScale_, 0);
    }
}

void GLCanvas::mouseReleaseEvent(QMouseEvent* event) {
    if (event->source() == Qt::MouseEventSynthesizedBySystem) {
	  event->accept();
	  return;
	}
    if (event->button() <= 4){
        if(mouseButtonPressed_[event->button()]) mouseButtonPressed_[event->button()] = false;
    }
    ginputp_mouseUp(event->x() * deviceScale_, event->y() * deviceScale_, event->button());
}

void GLCanvas::wheelEvent(QWheelEvent* event) {
	ginputp_mouseWheel(event->x() * deviceScale_, event->y() * deviceScale_,
            event->buttons(), event->delta());
}

void GLCanvas::keyPressEvent(QKeyEvent* event) {
	if (!event->isAutoRepeat())
		ginputp_keyDown(event->key());
	if (!event->text().isEmpty()) //Don't bother propagating empty key strokes
		ginputp_keyChar(event->text().toUtf8().constData());
}

void GLCanvas::keyReleaseEvent(QKeyEvent* event) {
	if (event->isAutoRepeat())
		return;

	ginputp_keyUp(event->key());
}

void GLCanvas::tabletEvent(QTabletEvent* event) {

    int xs[1];
    int ys[1];
    int ids[1];
    float pressures[1];
    int touchTypes[1];

    xs[0] = event->x() * deviceScale_;
    ys[0] = event->y() * deviceScale_;
    ids[0] = 0;
    pressures[0] = event->pressure();
    touchTypes[0] = 3;


    if(event->type() == QEvent::TabletPress){
        ginputp_touchesBegin(xs[0], ys[0], ids[0], pressures[0], touchTypes[0], 1, xs, ys, ids, pressures, touchTypes);

    }else if(event->type() == QEvent::TabletMove){
        ginputp_touchesMove(xs[0], ys[0], ids[0], pressures[0], touchTypes[0], 1, xs, ys, ids, pressures, touchTypes);

    }else if(event->type() == QEvent::TabletRelease){
        ginputp_touchesEnd(xs[0], ys[0], ids[0], pressures[0], touchTypes[0], 1, xs, ys, ids, pressures, touchTypes);
    }
    event->accept();
}

bool GLCanvas::event(QEvent *event){
    if (event->type() == QEvent::TouchBegin || event->type() == QEvent::TouchUpdate || event->type() == QEvent::TouchEnd || event->type() == QEvent::TouchCancel)
    {
        QTouchEvent* touchEvent = (QTouchEvent*)event;
        const QList<QTouchEvent::TouchPoint> &list = touchEvent->touchPoints();
        int size = list.count();

        int xs[size];
        int ys[size];
        int ids[size];
        float pressures[size];
        int touchTypes[size];

        for( int i=0; i<size; ++i )
        {
            QTouchEvent::TouchPoint p = list[i];
            xs[i] = p.pos().x() * deviceScale_;
            ys[i] = p.pos().y() * deviceScale_;
            ids[i] = i;
            pressures[i] = p.pressure();
            touchTypes[i] = p.flags();
        }

        for( int i=0; i<size; ++i )
        {
            QTouchEvent::TouchPoint p = list[i];
            if(event->type() == QEvent::TouchCancel){
                ginputp_touchesCancel(p.pos().x() * deviceScale_, p.pos().y() * deviceScale_, p.pressure(), p.flags(), i, size, xs, ys, ids, pressures, touchTypes);
            }
            else if(p.state() == Qt::TouchPointPressed){
                ginputp_touchesBegin(p.pos().x() * deviceScale_, p.pos().y() * deviceScale_,p.pressure(), p.flags(), i, size, xs, ys, ids, pressures, touchTypes);
            }
            else if(p.state() == Qt::TouchPointMoved){
                ginputp_touchesMove(p.pos().x() * deviceScale_, p.pos().y() * deviceScale_,p.pressure(), p.flags(), i, size, xs, ys, ids, pressures, touchTypes);
            }
            else if(p.state() == Qt::TouchPointReleased){
                ginputp_touchesEnd(p.pos().x() * deviceScale_, p.pos().y() * deviceScale_,p.pressure(), p.flags(), i, size, xs, ys, ids, pressures, touchTypes);
            }
        }
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
        keyPressEvent((QKeyEvent*) event);
        return true;
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
            Event event(Event::APPLICATION_RESUME);
            GStatus status;
            application_->broadcastEvent(&event, &status);
            checkLuaError(status);
        }
    }
    else if(event->type() == QEvent::WindowUnblocked){
        if(running_){
            Event event(Event::APPLICATION_RESUME);
            GStatus status;
            application_->broadcastEvent(&event, &status);
            checkLuaError(status);
        }
    }
    else if(event->type() == QEvent::FocusOut){
        if(running_){
            Event event(Event::APPLICATION_SUSPEND);
            GStatus status;
            application_->broadcastEvent(&event, &status);
            checkLuaError(status);
        }
    }
    else if(event->type() == QEvent::WindowBlocked){
        if(running_){
            Event event(Event::APPLICATION_SUSPEND);
            GStatus status;
            application_->broadcastEvent(&event, &status);
            checkLuaError(status);
        }
    }
    return QGLWidget::event(event);
}

void GLCanvas::onTimer() {
	double deltat = 1.0 / fps_;

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

	buffer.append((char) 6);

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
	buffer.append((char) 10);
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
	fps_ = fps;
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
