#include <stdio.h>
#include <windows.h>
#include <vector>
#include <string>
#include <stdlib.h>

#include "luaapplication.h"
#include <application.h>
#include <gapplication-win32.h>
#include "platform.h"
#include <gfile_p.h>
#include <shlobj.h>
#include <combaseapi.h>
#include <knownfolders.h>

extern HWND hwndcopy;
extern std::string commandLine;
extern LuaApplication *application_;


static std::wstring ws(const char *str)
{
    if (!str) return std::wstring();
    int sl=strlen(str);
    int sz = MultiByteToWideChar(CP_UTF8, 0, str, sl, 0, 0);
    std::wstring res(sz, 0);
    MultiByteToWideChar(CP_UTF8, 0, str, sl, &res[0], sz);
    return res;
}

static std::string us(const wchar_t *str)
{
    if (!str) return std::string();
    int sl=wcslen(str);
    int sz = WideCharToMultiByte(CP_UTF8, 0, str, sl, 0, 0,NULL,NULL);
    std::string res(sz, 0);
    WideCharToMultiByte(CP_UTF8, 0, str, sl, &res[0], sz,NULL,NULL);
    return res;
}
void GetDesktopResolution(int& horizontal, int& vertical)
{
  RECT desktop;
  // Get a handle to the desktop window
  const HWND hDesktop = GetDesktopWindow();
  // Get the size of screen to the variable desktop
  GetClientRect(hDesktop, &desktop);
  // The top left corner will have coordinates (0,0)
  // and the bottom right corner will have coordinates
  // (horizontal, vertical)
  horizontal = desktop.right;
  vertical = desktop.bottom;
}

std::vector<std::string> getDeviceInfo()
{
  std::vector<std::string> result;

  result.push_back("Win32");
  
  return result;
}

void openUrl(const char* url)
{
  std::wstring w=ws(url);
  ShellExecute(hwndcopy,NULL,w.c_str(),NULL,NULL,SW_SHOWNORMAL);
}

bool canOpenUrl(const char *url)
{
  return true;
}

std::string getLocale()
{
  TCHAR szBuff1[10], szBuff2[10]; 

  LCID lcid = GetUserDefaultLCID(); 

  GetLocaleInfo(lcid, LOCALE_SISO639LANGNAME, szBuff1, 10); 
  GetLocaleInfo(lcid, LOCALE_SISO3166CTRYNAME, szBuff2, 10); 
  std::string s=us(szBuff1)+"_"+us(szBuff2);

  return s;
}

std::string getLanguage()
{
  TCHAR szBuff[10]; 
  LCID lcid = GetUserDefaultLCID(); 
  GetLocaleInfo(lcid, LOCALE_SISO639LANGNAME, szBuff, 10); 
  return us(szBuff);
}

std::string getAppId(){
	return "";
}

void getSafeDisplayArea(int &x,int &y,int &w,int &h)
{
}

void setWindowSize(int width, int height)
{
  printf("setWindowSize: %d x %d. hwndcopy=%p\n",width,height,hwndcopy);

  Orientation app_orient=application_->orientation();

  if (app_orient==ePortrait || app_orient==ePortraitUpsideDown){
    RECT rect;
    rect.left=0;
    rect.top=0;
    rect.right=width;
    rect.bottom=height;

    AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,FALSE);

    SetWindowPos(hwndcopy,HWND_TOP,0,0,rect.right-rect.left, rect.bottom-rect.top, SWP_NOMOVE);
    printf("SetWindowPos: %d %d\n",rect.right-rect.left, rect.bottom-rect.top);
  }
  else {
    RECT rect;
    rect.left=0;
    rect.top=0;
    rect.right=height;
    rect.bottom=width;

    AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,FALSE);

    SetWindowPos(hwndcopy,HWND_TOP,0,0,rect.right-rect.left, rect.bottom-rect.top, SWP_NOMOVE);
    printf("SetWindowPos: %d %d\n",rect.right-rect.left, rect.bottom-rect.top);
  }

  //application_->setHardwareOrientation(app_orient);   // previously eFixed
  //application_->getApplication()->setDeviceOrientation(app_orient);
}

void W32SetFullScreen(bool fullScreen,HWND wnd,W32FullScreen *save)
{
  bool for_metro=false;
  if (fullScreen==save->isFullScreen) return;

  // Save current window state if not already fullscreen.
  if (!save->isFullScreen) {
      // Save current window information.  We force the window into restored mode
      // before going fullscreen because Windows doesn't seem to hide the
      // taskbar if the window is in the maximized state.
	  save->maximized = !!::IsZoomed(wnd);
      if (save->maximized)
        ::SendMessage(wnd, WM_SYSCOMMAND, SC_RESTORE, 0);
      save->style = GetWindowLong(wnd, GWL_STYLE);
      save->ex_style = GetWindowLong(wnd, GWL_EXSTYLE);
      GetWindowRect(wnd, &save->window_rect);
    }

    if (fullScreen) {
      // Set new window style and size.
      SetWindowLong(wnd, GWL_STYLE,
    		  save->style & ~(WS_CAPTION | WS_THICKFRAME));
      SetWindowLong(wnd, GWL_EXSTYLE,
    		  save->ex_style & ~(WS_EX_DLGMODALFRAME |
                    WS_EX_WINDOWEDGE | WS_EX_CLIENTEDGE | WS_EX_STATICEDGE));

      // On expand, if we're given a window_rect, grow to it, otherwise do
      // not resize.
      if (!for_metro) {
        MONITORINFO monitor_info;
        monitor_info.cbSize = sizeof(monitor_info);
        GetMonitorInfo(MonitorFromWindow(wnd, MONITOR_DEFAULTTONEAREST),
                       &monitor_info);
        SetWindowPos(wnd, NULL, monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
        		monitor_info.rcMonitor.right-monitor_info.rcMonitor.left,
				monitor_info.rcMonitor.bottom-monitor_info.rcMonitor.top,
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
      }
    } else {
      // Reset original window style and size.  The multiple window size/moves
      // here are ugly, but if SetWindowPos() doesn't redraw, the taskbar won't be
      // repainted.  Better-looking methods welcome.
      SetWindowLong(wnd, GWL_STYLE, save->style);
      SetWindowLong(wnd, GWL_EXSTYLE, save->ex_style);

      if (!for_metro) {
        // On restore, resize to the previous saved rect size.
        SetWindowPos(wnd, NULL, save->window_rect.left,save->window_rect.top,
        		save->window_rect.right-save->window_rect.left,
				save->window_rect.bottom-save->window_rect.top,
                     SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
      }
      if (save->maximized)
        SendMessage(wnd, WM_SYSCOMMAND, SC_MAXIMIZE, 0);
    }

  save->isFullScreen=fullScreen;
}

static W32FullScreen saved_window_info_;

void setFullScreen(bool fullScreen)
{
	W32SetFullScreen(fullScreen,hwndcopy,&saved_window_info_);
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

bool setTextInput(int type,const char *buffer,int selstart,int selend,const char *label,const char *actionLabel, const char *hintText)
{
	return false;
}

int setClipboard(std::string data,std::string mimeType, int luaFunc) {
	return -1;
}

int getClipboard(std::string &data,std::string &mimeType, int luaFunc) {
	return -1;
}

int getKeyboardModifiers() {
	int m=0;
	if (GetKeyState(VK_CONTROL)) m|=GINPUT_CTRL_MODIFIER;
	if (GetKeyState(VK_SHIFT)) m|=GINPUT_SHIFT_MODIFIER;
	if (GetKeyState(VK_MENU)) m|=GINPUT_ALT_MODIFIER;
	return m;
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


extern std::string PATH_Executable;
extern std::string PATH_Temp;
extern std::string PATH_Cache;
extern std::string PATH_AppName;
std::vector<gapplication_Variant> g_getsetProperty(bool set, const char* what, std::vector<gapplication_Variant> &args)
{
    std::vector<gapplication_Variant> rets;
    gapplication_Variant r;
    RECT rr;
    rr.left=rr.right=rr.top=rr.bottom=0;
    if (!set) {
        /*------------------------------------------------------------------*/
		if (!strcmp(what,"commandLine"))
		{
			r.type=gapplication_Variant::STRING;
			r.s=commandLine;
			rets.push_back(r);
		}
		else if (strcmp(what, "windowPosition") == 0)
        {
			GetWindowRect(hwndcopy,&rr);
            r.type=gapplication_Variant::DOUBLE;
            r.d=rr.left;
            rets.push_back(r);
            r.d=rr.top;
            rets.push_back(r);
            /*------------------------------------------------------------------*/

        }else if (strcmp(what, "windowSize") == 0)
        {
			GetWindowRect(hwndcopy,&rr);
            r.type=gapplication_Variant::DOUBLE;
            r.d=rr.right-rr.left;
            rets.push_back(r);
            r.d=rr.bottom-rr.top;
            rets.push_back(r);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "screenSize") == 0)
        {
            r.type=gapplication_Variant::DOUBLE;
            const HWND hDesktop = GetDesktopWindow();
            GetClientRect(hDesktop, &rr);
            r.d=rr.right;
            rets.push_back(r);
            r.d=rr.bottom;
            rets.push_back(r);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "cursorPosition") == 0)
        {
        	POINT p;
        	GetCursorPos(&p);
            r.type=gapplication_Variant::DOUBLE;
            r.d=p.x;
            rets.push_back(r);
            r.d=p.y;
            rets.push_back(r);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "clipboard") == 0)
        {
        	  // Try opening the clipboard
        	  if (OpenClipboard(nullptr))
        	  {
            	  HANDLE hData = GetClipboardData(CF_UNICODETEXT);
            	  if (hData != nullptr)
            	  {
                	  wchar_t * pszText = static_cast<wchar_t*>( GlobalLock(hData) );
                	  if (pszText != nullptr)
                	  {
                          r.type=gapplication_Variant::STRING;
                          r.s=us(pszText);
                          rets.push_back(r);
                	  }
                	  GlobalUnlock( hData );
            	  }
            	  CloseClipboard();
        	  }
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "windowTitle") == 0)
        {
        	wchar_t wtitle[1024];
        	GetWindowText(hwndcopy,wtitle,1024);
            r.type=gapplication_Variant::STRING;
            r.s=us(wtitle);
            rets.push_back(r);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "directory") == 0)
        {
        	if (args.size()>0)
        	{
        		std::string &s=args[0].s;
        		KNOWNFOLDERID fid;
        		if (s=="executable") {
					r.type=gapplication_Variant::STRING;
					r.s=PATH_Executable;
					rets.push_back(r);
					return rets;
        		}
        		else if (s=="documents") fid=FOLDERID_Documents;
        		else if (s=="desktop") fid=FOLDERID_Desktop;
        		else if (s=="temporary") {
					r.type=gapplication_Variant::STRING;
					r.s=PATH_Temp;
					rets.push_back(r);
					return rets;
        		}
        		else if (s=="data") fid=FOLDERID_LocalAppData;
        		else if (s=="music") fid=FOLDERID_Music;
        		else if (s=="movies") fid=FOLDERID_Videos;
        		else if (s=="pictures") fid=FOLDERID_Pictures;
        		else if (s=="cache") {
					r.type=gapplication_Variant::STRING;
					r.s=PATH_Cache;
					rets.push_back(r);
					return rets;
        		}
        		else if (s=="download") fid=FOLDERID_Downloads;
        		else if (s=="home") fid=FOLDERID_Profile;
        		else return rets;
				PWSTR ppszPath; // variable to receive the path memory block pointer
				HRESULT hr = SHGetKnownFolderPath(fid, 0, NULL, &ppszPath);
				if (SUCCEEDED(hr)) {
					r.type=gapplication_Variant::STRING;
					r.s=us(ppszPath);
					if (s=="data")
						r.s=r.s+'\\'+PATH_AppName;
					rets.push_back(r);
				}
				CoTaskMemFree(ppszPath); // free up the path memory block
            }else{
/*                QString info = "Accepted value for ";
                info.append(what);
                info.append(" :");
                MainWindow::getInstance()->printToOutput(info.toStdString().c_str());
                for( int i=0; i<acceptedValue.size(); ++i ){
                    MainWindow::getInstance()->printToOutput( QString("- ").append(acceptedValue.at(i)).toStdString().c_str() );
                }*/
            }

            /*------------------------------------------------------------------*/
        }else if ((strcmp(what, "openDirectoryDialog") == 0)
                || (strcmp(what, "openFileDialog") == 0)
                || (strcmp(what, "saveFileDialog") == 0))
            {
        	/* TODO
            if(args.size() == 1){

                MainWindow::getInstance()->printToOutput("[[Usage Example]]");
                if (strcmp(what, "openDirectoryDialog") == 0){
                    MainWindow::getInstance()->printToOutput("application:get(\"openDirectoryDialog\",\"Open Directory|C:/)\")");
                }else if (strcmp(what, "openFileDialog") == 0){
                    MainWindow::getInstance()->printToOutput("application:get(\"openFileDialog\",\"Open File|C:/|Text File (*.txt);;Image File (*.jpg *.png)\")");
                }else if (strcmp(what, "saveFileDialog") == 0){
                    MainWindow::getInstance()->printToOutput("application:get(\"saveFileDialog\",\"Save File|C:/|Text File (*.txt);;Image File (*.jpg *.png)\")");
                }
            }else{

                QString title = QString::fromUtf8(args[0].s.c_str());
                QString place = "";
                QString extension = "";
                if (args.size() >= 2){place = QString::fromUtf8(args[1].s.c_str());}
                if (args.size() >= 3){extension = QString::fromUtf8(args[2].s.c_str());}

                QString fileName = "";
                if (strcmp(what, "openDirectoryDialog") == 0){
                    fileName = QFileDialog::getExistingDirectory(MainWindow::getInstance(),title ,place, QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
                }else if (strcmp(what, "openFileDialog") == 0){
                    fileName = QFileDialog::getOpenFileName(MainWindow::getInstance(), title,place,extension);

                }else if (strcmp(what, "saveFileDialog") == 0){
                    fileName = QFileDialog::getSaveFileName(MainWindow::getInstance(), title,place,extension);

                }
                r.type=gapplication_Variant::STRING;
                r.s=fileName.toStdString();
                rets.push_back(r);
            }
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
            // feel free to change this list
        	/* TODO
            QStringList acceptedWhat;
            acceptedWhat << "[x,y] windowPosition";
            acceptedWhat << "[w,h] windowSize";
            acceptedWhat << "[w,h] screenSize";
            acceptedWhat << "[x,y] cursorPosition";
            acceptedWhat << "[text] clipboard";
            acceptedWhat << "[text] windowTitle";
            acceptedWhat << "[path] directory(where//help)";
            acceptedWhat << "[path] openDirectoryDialog(title|path//help)";
            acceptedWhat << "[path] openFileDialog(title|path|extensions//help)";
            acceptedWhat << "[path] saveFileDialog(title|path|extensions//help)";
            acceptedWhat << "[path] documentDirectory";
            acceptedWhat << "[path] temporaryDirectory";

            MainWindow::getInstance()->printToOutput("Accepted value for Desktop's application:get()");
            for( int i=0; i<acceptedWhat.size(); ++i ){
                MainWindow::getInstance()->printToOutput( QString("- ").append(acceptedWhat.at(i)).toStdString().c_str() );
            }
            */
        }
    }
    else {
        /*------------------------------------------------------------------*/
        if (strcmp(what, "cursor") == 0)
        {
        	/* TODO
            QStringList acceptedValue;
            acceptedValue << "arrow" << "upArrow" << "cross" << "wait" << "IBeam";
            acceptedValue << "sizeVer" << "sizeHor" << "sizeBDiag" << "sizeFDiag" << "sizeAll";
            acceptedValue << "blank" << "splitV" << "splitH" << "pointingHand" << "forbidden";
            acceptedValue << "whatsThis" << "busy" << "openHand" << "closedHand" << "dragCopy";
            acceptedValue << "dragMove" << "dragLink";
            // value of cursor also taken from index of the text, do not change the list

            if (args.size()>0)&&(acceptedValue.contains(args[0].s)){
                arg1 = acceptedValue.indexOf(args[0].s);
                MainWindow::getInstance()->setCursor((Qt::CursorShape) arg1);
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
        }else if (strcmp(what, "windowPosition") == 0)
        {
            if (args.size()>=2)
            	SetWindowPos(hwndcopy,0,args[0].d,args[1].d,0,0,SWP_NOSIZE);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "windowSize") == 0)
        {
            if (args.size()>=2)
            	SetWindowPos(hwndcopy,0,0,0,args[0].d,args[1].d,SWP_NOMOVE);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "minimumSize") == 0)
        {
            /*TODO
              if (args.size()>=2)
                MainWindow::getInstance()->setMinimumSize(QSize(args[0].d,args[1].d));
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "maximumSize") == 0)
        {
            /*TODO
            if (args.size()>=2)
                MainWindow::getInstance()->setMaximumSize(QSize(args[0].d,args[1].d));
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "windowColor") == 0)
        {
            /*TODO
            if (args.size()>=3) {
                if (args[0].d > 255) {args[0].d = 255;}else if(args[0].d < 0) {args[0].d = 255;}
                if (args[1].d > 255) {args[1].d = 255;}else if(args[1].d < 0) {args[1].d = 255;}
                if (args[2].d > 255) {args[2].d = 255;}else if(args[2].d < 0) {args[2].d = 255;}
                QPalette palette;
                QColor backgroundColor = QColor(args[0].d, args[1].d, args[2].d);
                palette.setColor(QPalette::Window, backgroundColor);
                MainWindow::getInstance()->setPalette(palette);
            }
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "windowTitle") == 0)
        {
            if (args.size()>=1) {
            	std::wstring w=ws(args[0].s.c_str());
            	SetWindowText(hwndcopy,w.c_str());
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
    #ifdef Q_OS_WIN
            auto nativeWindowsApp = dynamic_cast<QNativeInterface::Private::QWindowsApplication *>(QGuiApplicationPrivate::platformIntegration());
            if (args.size()>=1)
                nativeWindowsApp->setWinTabEnabled(args[0].d);
    #endif
        }else if (strcmp(what, "cursorPosition") == 0)
        {
            if (args.size()>=2)
                SetCursorPos(args[0].d,args[1].d);
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "clipboard") == 0)
        {
            if (args.size()>=1) {
          	  if (OpenClipboard(nullptr))
          	  {
          		  EmptyClipboard();
              	  std::wstring w=ws(args[0].s.c_str());
              	  HANDLE hData = GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT,w.size()*sizeof(wchar_t)+2);
              	  if (hData != nullptr)
              	  {
                  	  wchar_t * pszText = static_cast<wchar_t*>( GlobalLock(hData) );
                  	  if (pszText != nullptr)
                      	  memcpy(pszText,w.c_str(),w.size()*sizeof(wchar_t));
                  	  GlobalUnlock( hData );
              	  }
              	  SetClipboardData(CF_UNICODETEXT,hData);
              	  CloseClipboard();
          	  }

            }
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "mkDir") == 0)
        {
        	/* TODO
            if(args.size() <= 1){
                MainWindow::getInstance()->printToOutput("[[Usage Example]]");
                MainWindow::getInstance()->printToOutput("application:set(\"mkDir\",application:get(\"directory\",\"executable\")..\"|dirName\")");
            }else{
                QDir dirPath = QDir::temp();
                dirPath.setPath(QString::fromUtf8(args[0].s));
                dirPath.mkdir(QString::fromUtf8(args[1].s));
            }
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
        	/* TODO

            // feel free to change this list
            QStringList acceptedWhat;
            acceptedWhat << "windowPosition(x,y)";
            acceptedWhat << "windowSize(w,h)";
            acceptedWhat << "minimumSize(w,h)";
            acceptedWhat << "maximumSize(w,h)";
            acceptedWhat << "windowColor(r,g,b)";
            acceptedWhat << "windowTitle(text)";
            acceptedWhat << "wintabMode(enable)";
            acceptedWhat << "windowModel(type//help)";
            acceptedWhat << "cursor(type//help)";
            acceptedWhat << "cursorPosition(x,y)";
            acceptedWhat << "clipboard(text)";
            acceptedWhat << "mkdir(path|dirName//help)";
            acceptedWhat << "documentDirectory(path)";
            acceptedWhat << "temporaryDirectory(path)";

            MainWindow::getInstance()->printToOutput("Accepted value for Desktop's application:set()");
            for( int i=0; i<acceptedWhat.size(); ++i ){
                MainWindow::getInstance()->printToOutput( QString("- ").append(acceptedWhat.at(i)).toStdString().c_str() );
            }
            */
        }

    }
    return rets;
}
