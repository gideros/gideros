#include <vector>
#include <luaapplication.h>
#include <map>
#include <libnetwork.h>
#include <string>
#include <stdlib.h>

struct ProjectProperties {
	ProjectProperties() {
		scaleMode = 0;
		logicalWidth = 320;
		logicalHeight = 480;
		orientation = 0;
		fps = 60;
		retinaDisplay = 0;
		autorotation = 0;
		mouseToTouch = 1;
		touchToMouse = 1;
		mouseTouchOrder = 0;
	}

	int scaleMode;
	int logicalWidth;
	int logicalHeight;
	std::vector<std::pair<std::string, float> > imageScales;
	int orientation;
	int fps;
	int retinaDisplay;
	int autorotation;
	int mouseToTouch;
	int touchToMouse;
	int mouseTouchOrder;
};

class ApplicationManager;
class NetworkManager {
public:
	NetworkManager(ApplicationManager *application);
	~NetworkManager();
	void tick();
	std::string openProject_;

	void setResourceDirectory(const char* resourceDirectory) {
		resourceDirectory_ = resourceDirectory;
	}

	void setMd5FileName(const char *md5FileName) {
		md5filename_ = md5FileName;
		loadMD5();
	}

	static void printToServer_s(const char *str, int len, void *data) {
		static_cast<NetworkManager*>(data)->printToServer(str, len);
	}

	void printToServer(const char *str, int len) {
		unsigned int size = 1 + ((len < 0) ? strlen(str) : len) + 1;
		char* buffer = (char*) malloc(size);

		buffer[0] = 4;
		memcpy(buffer + 1, str, size - 2);
		buffer[size - 1] = 0;

		server_->sendData(buffer, size);

		free(buffer);
	}

private:
	void createFolder(const std::vector<char> &data);
	void createFile(const std::vector<char> &data);
	void play(const std::vector<char> &data);
	void stop();
	void sendFileList();
	void setProjectName(const std::vector<char> &data);
	void deleteFile(const std::vector<char> &data);
	void setProperties(const std::vector<char> &data);

private:
	std::string md5filename_;
	std::map<std::string, std::vector<unsigned char> > md5_;
	void loadMD5();
	void saveMD5();
	void calculateMD5(const char *file);

private:
	ApplicationManager *application_;
	Server *server_;
	std::string resourceDirectory_;
};

class ApplicationManager {
public:
	ApplicationManager(bool player,const char *appname,const char *urlpath);
	~ApplicationManager();

	void luaError(const char *msg);

	void surfaceCreated();
	void surfaceChanged(int width, int height, int rotation);
	void updateHardwareOrientation();
	void drawFrame();

	void setDirectories(const char *externalDir, const char *internalDir,
			const char *cacheDir);
	void setFileSystem(const char *files);

	void openProject(const char* project);
	void setOpenProject(const char* project);
	void playFiles(const char* pfile,const char *lfile);
	void play(const char *gapp);
	void play(const std::vector<std::string>& luafiles);
	void stop();
	void setProjectName(const char *projectName);
	void setProjectProperties(const ProjectProperties &properties);
	bool isRunning();

	void touchesBegin(int size, int *id, int *x, int *y, int actionIndex);
	void touchesMove(int size, int *id, int *x, int *y);
	void touchesEnd(int size, int *id, int *x, int *y, int actionIndex);
	void touchesCancel(int size, int *id, int *x, int *y);

	bool keyDown(int keyCode, int repeatCount);
	bool keyUp(int keyCode, int repeatCount);

	void pause();
	void resume();

	void lowMemory();

	void background();
	void foreground();

private:
	void loadProperties();
	void loadLuaFiles();
	void drawIPs();
	int convertKeyCode(int keyCode);

private:
	bool player_;
	std::string appName;
	std::string appPath;
	LuaApplication *application_;
	NetworkManager *networkManager_;

	bool init_;

	bool running_;

	int width_, height_;

//  SplashScreen *splashScreen_;

	std::string externalDir_, internalDir_, cacheDir_;

	ProjectProperties properties_;

	Orientation hardwareOrientation_;

	Orientation deviceOrientation_;

	int nframe_;

	bool applicationStarted_;

	bool skipFirstEnterFrame_;
};
