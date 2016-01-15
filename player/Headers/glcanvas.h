#ifndef GLCANVAS_H
#define GLCANVAS_H

#include <QGLWidget>
#include <QDir>

class LuaApplication;
class Server;

#include "errordialog.h"
#include "orientation.h"
#include "platform.h"
#include <gfile.h>
#include <gfile_p.h>
#include <gstatus.h>

class GLCanvas : public QGLWidget{
	Q_OBJECT

public:
	GLCanvas(QWidget *parent);
	~GLCanvas();

    QString projectDir_;
	void setHardwareOrientation(Orientation orientation);
	void setResolution(int width, int height, bool event);
	void setFps(int fps);
    void setDrawInfos(bool drawInfos);
    void setScale(float scale);
    void setCanvasColor(float canvasColor[3]);
    void setInfoColor(float infoColor[3]);
    void setExportedApp(bool exportedApp);
    void printToOutput(const char* text);
	void sendRun();
    Orientation getHardwareOrientation();

    void play(QDir directory);
    void play(QString gapp);

    //std::set<std::string> accessedResourceFiles;
    std::set<std::string> allResourceFiles;
    static QString appPackage;

private slots:
	void onTimer();

private:
    LuaApplication* application_;
    Server* server_;
    ErrorDialog errorDialog_;
    QDir dir_;
    Orientation orientation_;
    int width_, height_;
    bool isPlayer_;
    std::string resourceDirectory_;
    std::string md5filename_;
    int fps_;
    double clock_;
    QString projectName_;
    bool running_;
    std::map<std::string, std::vector<unsigned char> > md5_;
    float scale_;
    float deviceScale_;
    bool drawInfos_;
    float canvasColor_[3];
    float infoColor_[3];
    bool exportedApp_;
    bool mouseButtonPressed_[5];
    //	PlatformImplementation* platformImplementation_;

    void setupProperties();
    void setupApplicationProperties();
	virtual void initializeGL();

	virtual void paintGL();
	bool checkLuaError(GStatus status);
    void timerEvent(QTimerEvent *);
    void mousePressEvent(QMouseEvent* event);
    void mouseMoveEvent(QMouseEvent* event);
    void mouseReleaseEvent(QMouseEvent* event);
    void wheelEvent(QWheelEvent* event);
    void keyPressEvent(QKeyEvent* event);
    void keyReleaseEvent(QKeyEvent* event);
    void tabletEvent(QTabletEvent* event);
    virtual bool event(QEvent *event);
	void deleteFiles();
	void sendFileList();
	void loadMD5();
	void saveMD5();
	void calculateMD5(const char* file);
    void printMD5();
    void loadProperties(std::vector<char> data);
    void loadFiles(std::vector<char> data);
    void playLoadedFiles(std::vector<std::string> luafiles);

    //static void accessFileCallback_s(FileType type, const char* filename, void* data);
    //void accessFileCallback(FileType type, const char* filename);

signals:
	void projectNameChanged(const QString& projectName);
};

#endif // GLCANVAS_H

/*
class PlatformImplementation : public PlatformInterface
{
public:
    PlatformImplementation(LuaApplication* application) : application_(application) {}

    virtual void openUrl(const char* url);
    virtual void memoryWarning();

    void openUrls();

private:
    QTimer* timer_;
    std::vector<QUrl> urls_;
    LuaApplication* application_;
};
*/
