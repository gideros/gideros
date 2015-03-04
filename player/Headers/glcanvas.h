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
//#include <QUrl>



class GLCanvas : public QGLWidget{
	Q_OBJECT

public:
	GLCanvas(QWidget *parent);
	~GLCanvas();

	void setHardwareOrientation(Orientation orientation);
	void setResolution(int width, int height);
	void setFps(int fps);
    void setDrawInfos(bool drawInfos);
    void setScale(float scale);
    void setCanvasColor(float canvasColor[3]);
    void setInfoColor(float infoColor[3]);

	void sendRun();

    // function to play an application into player passing a directory
    void play(QDir directory);

    //std::set<std::string> accessedResourceFiles;
    std::set<std::string> allResourceFiles;

private slots:
	void onTimer();

private:
    LuaApplication* application_;
    Server* server_;
    ErrorDialog errorDialog_;
    QDir dir_;
    Orientation orientation_;
    int width_, height_;
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
    //	PlatformImplementation* platformImplementation_;

    void setupProperties();
    void setupApplicationProperties();
	virtual void initializeGL();

	virtual void paintGL();
	virtual void timerEvent(QTimerEvent *);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void keyReleaseEvent(QKeyEvent* event);
	void deleteFiles();
	void sendFileList();
	void loadMD5();
	void saveMD5();
	void calculateMD5(const char* file);
    void printMD5();

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
