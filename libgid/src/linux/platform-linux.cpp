#include <vector>
#include <string>

#include <stdlib.h>
#include "platform.h"
#include "luaapplication.h"
#include <application.h>
#include <gapplication-linux.h>
#include <GLFW/glfw3.h>
#include <unistd.h>
#include <sys/utsname.h>
#include <limits.h>
#include <sys/stat.h>
#include <gfile_p.h>

extern GLFWwindow *glfw_win;
extern LuaApplication *application_;

void GetDesktopResolution(int& horizontal, int& vertical)
{
  GLFWmonitor *m=glfwGetPrimaryMonitor();
  const GLFWvidmode *vm=glfwGetVideoMode(m);
  horizontal=vm->width;
  vertical=vm->height;
}

std::vector<std::string> getDeviceInfo()
{
  std::vector<std::string> result;
  struct utsname u;

  result.push_back("Linux");
  if (uname(&u)==0) {
	  result.push_back(u.release);
	  result.push_back(u.version);
	  result.push_back(u.machine);
  }
  
  return result;
}

void openUrl(const char* url)
{
  std::string cmd="xdg-open "+std::string(url);
  system(cmd.c_str());
}

bool canOpenUrl(const char *url)
{
  return true;
}

std::string getLocale()
{
  std::string l=getenv("LOCALE");
  size_t dot=l.find_first_of('.');
  if (dot!=std::string::npos)
	  l=l.substr(0,dot-1);
  return l;
}

std::string getLanguage()
{
  std::string l=getenv("LANG");
  size_t dot=l.find_first_of('.');
  if (dot!=std::string::npos)
	  l=l.substr(0,dot-1);
  return l;
}

std::string getAppId(){
	return "";
}

void getSafeDisplayArea(int &x,int &y,int &w,int &h)
{
}

void setWindowSize(int width, int height)
{
  Orientation app_orient=application_->orientation();
  if (app_orient==ePortrait || app_orient==ePortraitUpsideDown)
	glfwSetWindowSize(glfw_win,width,height);
  else 
	glfwSetWindowSize(glfw_win,height,width);
}

void setFullScreen(bool fullScreen)
{
}

void vibrate(int ms)
{
}

void setKeepAwake(bool awake)
{
}

bool setKeyboardVisibility(bool visible){
	return false;
}

bool setTextInput(int type,const char *buffer,int selstart,int selend,const char *label,const char *actionLabel, const char *hintText, const char *context)
{
	return false;
}

int setClipboard(std::string data,std::string mimeType, int luaFunc) {
	if (mimeType=="text/plain") {
		glfwSetClipboardString(NULL,data.c_str());
		return 1;
	}
	return -1;
}

int getClipboard(std::string &data,std::string &mimeType, int luaFunc) {
	const char *text=glfwGetClipboardString(NULL);
	if (text) {
		data=text;
		mimeType="text/plain";
		return 1;
	}
	return -1;
}

extern int getMods(GLFWwindow *win); //defined in main.cpp
int getKeyboardModifiers() {
   return getMods(glfw_win);
}

static int s_fps = 60;

extern "C" {

int g_getFps()
{
  return s_fps;
}

void g_setFps(int fps)
{
  s_fps = fps;
}

}

void g_exit()
{
  exit(0);
}

static void _mkdirp(const char *dir) {
	char tmp[PATH_MAX];
	char *p = NULL;
	size_t len;

	snprintf(tmp, sizeof(tmp), "%s", dir);
	len = strlen(tmp);
	if (tmp[len - 1] == '/')
		tmp[len - 1] = 0;
	for (p = tmp + 1; *p; p++)
		if (*p == '/') {
			*p = 0;
			mkdir(tmp,S_IRWXU);
			*p = '/';
		}
	mkdir(tmp,S_IRWXU);
}

extern std::string PATH_Executable;
extern std::string PATH_Temp;
extern std::string PATH_Cache;
extern std::string PATH_AppName;
extern std::vector<std::string> PATH_CommandLine;

static std::map<std::string,int> cursorMap={
		{ "arrow", GLFW_ARROW_CURSOR },
//		{ "upArrow", OCR_UP },
		{ "cross", GLFW_CROSSHAIR_CURSOR },
//		{ "wait", OCR_WAIT },
		{ "IBeam", GLFW_IBEAM_CURSOR },
		{ "sizeVer",GLFW_VRESIZE_CURSOR },
		{ "sizeHor", GLFW_HRESIZE_CURSOR },
//		{ "sizeBDiag",OCR_SIZENWSE },
//		{ "sizeFDiag", OCR_SIZENESW },
//		{ "sizeAll", OCR_SIZEALL }, //Doesn't really fit, but best so far'
//		{ "blank",  "none"},
		{ "splitV", GLFW_VRESIZE_CURSOR }, //need better
		{ "splitH", GLFW_HRESIZE_CURSOR }, //need better
		{ "pointingHand", GLFW_HAND_CURSOR },
//		{ "forbidden", OCR_NO },
//		{ "whatsThis", OCR_HELP },
//		{ "busy", OCR_APPSTARTING },
//		{ "openHand", "grab" },
//		{ "closedHand", "grabbing" },
//		{ "dragCopy", "copy" },
//		{ "dragMove", "move" },
//		{ "dragLink", "alias" },
};
static std::map<int,GLFWcursor *> customCursors;

std::vector<gapplication_Variant> g_getsetProperty(bool set, const char* what, std::vector<gapplication_Variant> &args)
{
	std::vector<gapplication_Variant> rets;
   gapplication_Variant r;
    if (!set) { // GET
		if (!strcmp(what,"commandLine"))
		{
			r.type=gapplication_Variant::STRING;
			for (auto it=PATH_CommandLine.begin();it!=PATH_CommandLine.end();it++) {
				r.s=*it;
				rets.push_back(r);
			}
            /*------------------------------------------------------------------*/
		}else if (strcmp(what, "windowPosition") == 0)
        {
			int x,y;
			glfwGetWindowPos(glfw_win,&x,&y);
            r.type=gapplication_Variant::DOUBLE;
            r.d=x;
            rets.push_back(r);
            r.d=y;
            rets.push_back(r);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "windowSize") == 0)
        {
			int x,y;
			glfwGetWindowSize(glfw_win,&x,&y);
            r.type=gapplication_Variant::DOUBLE;
            r.d=x;
            rets.push_back(r);
            r.d=y;
            rets.push_back(r);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "screenSize") == 0)
        {
			GLFWmonitor *m=glfwGetPrimaryMonitor();
			const GLFWvidmode *vm=glfwGetVideoMode(m);
            r.type=gapplication_Variant::DOUBLE;
            r.d=vm->width;
            rets.push_back(r);
            r.d=vm->height;
            rets.push_back(r);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "cursorPosition") == 0)
        {
        	double x,y;
			glfwGetCursorPos(glfw_win,&x,&y);
            r.type=gapplication_Variant::DOUBLE;
            r.d=x;
            rets.push_back(r);
            r.d=y;
            rets.push_back(r);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "clipboard") == 0)
        {
			const char *text=glfwGetClipboardString(NULL);
			if (text) {
				r.type=gapplication_Variant::STRING;
				r.s=text;
				rets.push_back(r);
			}
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "windowTitle") == 0)
        {
			//Settable only
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "directory") == 0)
        {
        	if (args.size()>0)
        	{
        		std::string &s=args[0].s;
        		std::string home=getenv("HOME");
        		if (s=="executable") {
					r.type=gapplication_Variant::STRING;
					r.s=PATH_Executable;
					rets.push_back(r);
					return rets;
        		}
        		else if (s=="documents") home=home+"/Documents";
        		else if (s=="desktop") home=home+"/Desktop";
        		else if (s=="temporary") {
					r.type=gapplication_Variant::STRING;
					r.s=PATH_Temp;
					rets.push_back(r);
					return rets;
        		}
        		else if (s=="data") home=home+"/.local/share";
        		else if (s=="music") home=home+"/Music";
        		else if (s=="movies") home=home+"/Videos";
        		else if (s=="pictures") home=home+"/Pictures";
        		else if (s=="cache") {
					r.type=gapplication_Variant::STRING;
					r.s=PATH_Cache;
					rets.push_back(r);
					return rets;
        		}
        		else if (s=="download") home=home+"/Downloads";
        		else if (s=="home") home=home+'/'+PATH_AppName;
        		else return rets;
				r.type=gapplication_Variant::STRING;
				r.s=home;
				rets.push_back(r);
            }else{
                /* INFO SHOWN IN GIDEROS STUDIO DEBUGGER, IMPLEMENTED IN QT, NOT NEEDED HERE? */
            }
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "pathfileexists") == 0) 
        {
             int m = 0; // modes: 0=Existence only, 2=Write-only, 4=Read-only, 6=Read and write
            if (args.size() >= 2) {
                int mm = args[1].d;
				if (mm) {
					if (mm&2) m|=W_OK;
					if (mm&4) m|=R_OK;
					if (mm&8) m|=X_OK;
				}
				else
					m=F_OK;
            }
            int retValue = access(args[0].s.c_str(), m); // 0 = OK, else -1
            if (retValue == 0) {
                r.type=gapplication_Variant::DOUBLE;
                r.d=1; // for lua 1=OK otherwise nil, looks better?!
                rets.push_back(r);
            }
            /*------------------------------------------------------------------*/
        }else if ((strcmp(what, "openDirectoryDialog") == 0)
                || (strcmp(what, "openFileDialog") == 0)
                || (strcmp(what, "saveFileDialog") == 0))
        {
			//TODO
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "temporaryDirectory") == 0)
        {
            r.type=gapplication_Variant::STRING;
            r.s=getTemporaryDirectory();
            rets.push_back(r);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "documentDirectory") == 0)
        {
            r.type=gapplication_Variant::STRING;
            r.s=getDocumentsDirectory();
            rets.push_back(r);
            /*------------------------------------------------------------------*/
        }else{
            /* INFO SHOWN IN GIDEROS STUDIO DEBUGGER, IMPLEMENTED IN QT, NOT NEEDED HERE? */
        }
    }
    else { // SET
        if (strcmp(what, "cursor") == 0)
        {
            int cursorIdx=cursorMap[args[0].s];
            if (cursorIdx==0) {
				glfwSetCursor(glfw_win,NULL);
			}
			else {
				GLFWcursor *cc=customCursors[cursorIdx];
				if (!cc) {
					cc=glfwCreateStandardCursor(cursorIdx);
					customCursors[cursorIdx]=cc;
				}
				glfwSetCursor(glfw_win,cc);
			}
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "windowPosition") == 0)
        {
            if (args.size()>=2)
            	glfwSetWindowPos(glfw_win,args[0].d,args[1].d);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "windowSize") == 0)
        {
            if (args.size()>=2)
            	glfwSetWindowSize(glfw_win,args[0].d,args[1].d);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "minimumSize") == 0)
        {
            /*TODO
            if (args.size()>=2)
                SetMinimumSize(hwndcopy,0,0,0,args[0].d,args[1].d,SWP_NOMOVE);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "maximumSize") == 0)
        {
            /*TODO
            if (args.size()>=2)
                MainWindow::getInstance()->setMaximumSize(QSize(args[0].d,args[1].d));
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "windowColor") == 0)
        {
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "windowTitle") == 0)
        {
            if (args.size()>=1) {
            	glfwSetWindowTitle(glfw_win,args[0].s.c_str());
            }
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "windowModel") == 0)
        {
        	/* TODO
            QStringList acceptedValue;
            acceptedValue << "reset" << "stayOnTop" << "stayOnBottom" << "frameless" << "noTitleBar";
            acceptedValue << "noButton" << "onlyMinimize" << "onlyMaximize" << "onlyClose" << "noMinimize";
            acceptedValue << "noMaximize" << "noClose" << "helpButton";

            if ((args.size()>0)&&acceptedValue.contains(args[0].s)) {
                Qt::WindowFlags flags = MainWindow::getInstance()->windowFlags();
                QString argString=QString::fromUtf8(args[0].s);

                if (argString == "reset"){
                    flags = Qt::Window;
                }else if (argString == "stayOnTop"){
                    flags |= Qt::WindowStaysOnTopHint;
                }else if (argString == "stayOnBottom"){
                    flags |= Qt::WindowStaysOnBottomHint;
                }else if (argString == "frameless"){
                    flags |= Qt::FramelessWindowHint;
                }else if (argString == "noTitleBar"){
                    flags = Qt::Window;
                    flags |= Qt::CustomizeWindowHint;
                }else if (argString == "noButton"){
                    flags = Qt::Window;
                    flags |= Qt::WindowTitleHint;
                }else if (argString == "noClose"){
                    flags = Qt::Window;
                    flags |= Qt::WindowMinimizeButtonHint;
                }else if (argString == "onlyMaximize"){
                    flags = Qt::Window;
                    flags |= Qt::WindowMaximizeButtonHint;
                }else if (argString == "onlyClose"){
                    flags = Qt::Window;
                    flags |= Qt::WindowCloseButtonHint;
                }else if (argString == "noMinimize"){
                    flags = Qt::Window;
                    flags |= Qt::WindowMaximizeButtonHint;
                    flags |= Qt::WindowCloseButtonHint;
                }else if (argString == "noMaximize"){
                    flags = Qt::Window;
                    flags |= Qt::WindowMinimizeButtonHint;
                    flags |= Qt::WindowCloseButtonHint;
                }else if (argString == "noClose"){
                    flags = Qt::Window;
                    flags |= Qt::WindowMinimizeButtonHint;
                    flags |= Qt::WindowMaximizeButtonHint;
                }else if (argString == "helpButton"){
                    flags = Qt::Window;
                    flags |= Qt::WindowContextHelpButtonHint;
                    flags |= Qt::WindowCloseButtonHint;
                }

                MainWindow::getInstance()->setWindowFlags(flags);
                if (MainWindow::getInstance()->fullScreen()){
                    MainWindow::getInstance()->showFullScreen();
                }else{
                    MainWindow::getInstance()->showNormal();
                }

            }else{
                QString info = "Accepted value for ";
                info.append(what);
                info.append(" :");
                MainWindow::getInstance()->printToOutput(info.toStdString().c_str());
                for( int i=0; i<acceptedValue.size(); ++i ){
                    MainWindow::getInstance()->printToOutput( QString("- ").append(acceptedValue.at(i)).toStdString().c_str() );
                }
            }
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "wintabMode") == 0)
        {
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "cursorPosition") == 0)
        {
			/*
            if (args.size()>=2)
                glfwSetCursorPos(args[0].d,args[1].d);
			*/
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "clipboard") == 0)
        {
            if (args.size()>=1) {
				glfwSetClipboardString(NULL,args[0].s.c_str());
            }
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "mkDir") == 0)
        {
            _mkdirp(args[0].s.c_str());
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "documentDirectory") == 0)
        {
            if(args.size() >= 1)
                setDocumentsDirectory(args[0].s.c_str());
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "temporaryDirectory") == 0)
        {
            if(args.size() >= 1)
                setTemporaryDirectory(args[0].s.c_str());
        }else{
            /* INFO SHOWN IN GIDEROS STUDIO DEBUGGER, IMPLEMENTED IN QT, NOT NEEDED HERE? */
        }
    }

	return rets;
}
