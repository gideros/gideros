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

#include <QUrl>

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

class GLCanvas : public QGLWidget
{
	Q_OBJECT

public:
	GLCanvas(QWidget *parent);
	~GLCanvas();

	void setHardwareOrientation(Orientation orientation);
	void setResolution(int width, int height);

	void setFps(int fps);

    //std::set<std::string> accessedResourceFiles;
	std::set<std::string> allResourceFiles;

	void sendRun();

	void setScale(int scale);

private:
    //static void accessFileCallback_s(FileType type, const char* filename, void* data);
    //void accessFileCallback(FileType type, const char* filename);

private slots:
	void onTimer();

private:
	virtual void initializeGL();
	virtual void paintGL();
	virtual void timerEvent(QTimerEvent *);

	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);

	virtual void keyPressEvent(QKeyEvent* event);
	virtual void keyReleaseEvent(QKeyEvent* event);

private:
	void deleteFiles();
	void sendFileList();

private:
	void loadMD5();
	void saveMD5();
	void calculateMD5(const char* file);
	void printMD5();

signals:
	void projectNameChanged(const QString& projectName);

private:
	LuaApplication* application_;
	Server* server_;
	ErrorDialog errorDialog_;
//	PlatformImplementation* platformImplementation_;
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
	int scale_;
};

#endif // GLCANVAS_H
