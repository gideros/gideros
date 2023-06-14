#include <stdio.h>
#define OEMRESOURCE
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

#include <wchar.h>

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

std::string getAppId()
{
  return "";
}

void getSafeDisplayArea(int &x,int &y,int &w,int &h)
{
}

void setWindowSize(int width, int height)
{
  Orientation app_orient=application_->orientation();

  if (app_orient==ePortrait || app_orient==ePortraitUpsideDown){
    RECT rect;
    rect.left=0;
    rect.top=0;
    rect.right=width;
    rect.bottom=height;

    AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,FALSE);
    SetWindowPos(hwndcopy,HWND_TOP,0,0,rect.right-rect.left, rect.bottom-rect.top, SWP_NOMOVE);
  }
  else {
    RECT rect;
    rect.left=0;
    rect.top=0;
    rect.right=height;
    rect.bottom=width;

    AdjustWindowRect(&rect,WS_OVERLAPPEDWINDOW,FALSE);
    SetWindowPos(hwndcopy,HWND_TOP,0,0,rect.right-rect.left, rect.bottom-rect.top, SWP_NOMOVE);
  }

  //application_->setHardwareOrientation(app_orient); // previously eFixed
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

      // On expand, if we're given a window_rect, grow to it, otherwise do not resize.
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

bool setKeyboardVisibility(bool visible)
{
	return false;
}

bool setTextInput(int type,const char *buffer,int selstart,int selend,const char *label,const char *actionLabel, const char *hintText, const char *context)
{
	return false;
}

int setClipboard(std::string data,std::string mimeType, int luaFunc)
{
	return -1;
}

int getClipboard(std::string &data,std::string &mimeType, int luaFunc)
{
	return -1;
}

int getKeyboardModifiers()
{
	int m=0;
	if (GetAsyncKeyState(VK_CONTROL)) m|=GINPUT_CTRL_MODIFIER; // new 20221114 XXX
	if (GetAsyncKeyState(VK_SHIFT)) m|=GINPUT_SHIFT_MODIFIER; // new 20221114 XXX
	if (GetAsyncKeyState(VK_MENU)) m|=GINPUT_ALT_MODIFIER; // new 20221114 XXX
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

#ifndef OCR_HELP
#define OCR_HELP 32651
#endif
static std::map<std::string,int> cursorMap={
		{ "arrow", OCR_NORMAL },
		{ "upArrow", OCR_UP },
		{ "cross", OCR_CROSS },
		{ "wait", OCR_WAIT },
		{ "IBeam", OCR_IBEAM },
		{ "sizeVer",OCR_SIZENS },
		{ "sizeHor", OCR_SIZEWE },
		{ "sizeBDiag",OCR_SIZENWSE },
		{ "sizeFDiag", OCR_SIZENESW },
		{ "sizeAll", OCR_SIZEALL }, //Doesn't really fit, but best so far'
//		{ "blank",  "none"},
		{ "splitV", OCR_SIZENS }, //need better
		{ "splitH", OCR_SIZEWE }, //need better
		{ "pointingHand", OCR_HAND },
		{ "forbidden", OCR_NO },
		{ "whatsThis", OCR_HELP },
		{ "busy", OCR_APPSTARTING },
//		{ "openHand", "grab" },
//		{ "closedHand", "grabbing" },
//		{ "dragCopy", "copy" },
//		{ "dragMove", "move" },
//		{ "dragLink", "alias" },
};

std::vector<gapplication_Variant> g_getsetProperty(bool set, const char* what, std::vector<gapplication_Variant> &args)
{
    std::vector<gapplication_Variant> rets;
    gapplication_Variant r;
    RECT rr;
    rr.left=rr.right=rr.top=rr.bottom=0;
    if (!set) { // GET
		if (!strcmp(what,"commandLine"))
		{
			r.type=gapplication_Variant::STRING;
			r.s=commandLine;
			rets.push_back(r);
            /*------------------------------------------------------------------*/
		}else if (strcmp(what, "windowPosition") == 0)
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
                /* INFO SHOWN IN GIDEROS STUDIO DEBUGGER, IMPLEMENTED IN QT, NOT NEEDED HERE? */
            }
            /*------------------------------------------------------------------*/
        }else if (strcmp(what, "pathfileexists") == 0) // new 20221116 XXX
        {
            std::wstring path = ws(args[0].s.c_str());
            int m = 0; // modes: 0=Existence only, 2=Write-only, 4=Read-only, 6=Read and write
            if (args.size() >= 2) {
                m = args[1].d;
                if (m != 0 || m != 2 || m != 4 || m != 6)
                    m = 0;
            }
            int retValue = _waccess(path.c_str(), m); // 0 = OK, else -1
            if (retValue == 0) {
                r.type=gapplication_Variant::DOUBLE;
//              r.d=retValue; // 0 = OK, else -1, not so good in lua!?
                r.d=1; // for lua 1=OK otherwise nil, looks better?!
                rets.push_back(r);
            }
            /*------------------------------------------------------------------*/
        }else if ((strcmp(what, "openDirectoryDialog") == 0)
                || (strcmp(what, "openFileDialog") == 0)
                || (strcmp(what, "saveFileDialog") == 0))
        {
            if (args.size() >= 2) {
                std::wstring title = ws(args[0].s.c_str());
                std::wstring place = ws(args[1].s.c_str());
                std::vector<std::pair<std::wstring,std::wstring>> filters;
                if (args.size()>=3) {
                    std::wstring ext = ws(args[2].s.c_str());
                    while (!ext.empty()) {
                        std::wstring next;
                        size_t semicolon=ext.find(L";;");
                        if (semicolon!=std::wstring::npos) {
                            next=ext.substr(semicolon+2);
                            ext=ext.substr(0,semicolon);
                        }
                        size_t p1=ext.find_first_of(L'(');
                        size_t p2=ext.find_last_of(L')');
                        if ((p1!=std::wstring::npos)&&(p2!=std::wstring::npos)&&(p2>p1))
                        {
                            //Valid filter, extract label and extensions
                            std::wstring label=ext.substr(0,p1);
                            std::wstring exts=ext.substr(p1+1,p2-p1-1);
                            //QT uses space for extensions separator, while windows expects semicolon. Convert them.
                            std::replace(exts.begin(),exts.end(),L' ',L';');
                            filters.push_back(std::pair<std::wstring,std::wstring>(label,exts));
                        }
                        ext=next;
                    }
                }

                DWORD dwFlags;
                HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
                if (SUCCEEDED(hr))
                {
                    COMDLG_FILTERSPEC *fileTypes=new COMDLG_FILTERSPEC[filters.size()];
                    for (size_t i=0;i<filters.size();i++) {
                        fileTypes[i].pszName=filters[i].first.c_str();
                        fileTypes[i].pszSpec=filters[i].second.c_str();
                    }
                    IFileDialog *pFile;
                    if (strcmp(what, "saveFileDialog") == 0)
                        hr = CoCreateInstance(CLSID_FileSaveDialog, NULL, CLSCTX_ALL,
                                IID_IFileSaveDialog, reinterpret_cast<void**>(&pFile));
                    else
                        hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
                                IID_IFileOpenDialog, reinterpret_cast<void**>(&pFile));
                    if (SUCCEEDED(hr))
                    {
                        bool isFolder=(strcmp(what, "openDirectoryDialog") == 0);
                        // get/set options
                        pFile->GetOptions(&dwFlags);
                        pFile->SetOptions(dwFlags | FOS_FORCEFILESYSTEM | (isFolder?FOS_PICKFOLDERS:0));
                        if (!isFolder) {
                            pFile->SetFileTypes(filters.size(), fileTypes);
                            pFile->SetFileTypeIndex(1); // index starts at 1
                            //pFile->SetDefaultExtension(L"obj;fbx"); // append default extension
                            //printf("* fileTypes *, set default extension to %ls\n", fileTypes[0].pszSpec); OK
                            pFile->SetDefaultExtension(fileTypes[0].pszSpec); // append default 1st extension
                        }
                        hr = pFile->SetTitle(title.c_str()); // need more check?

                        // set starting folder
                        IShellItem *pItem = NULL;
                        hr = SHCreateItemFromParsingName(place.c_str(), NULL, IID_IShellItem, (LPVOID *)&pItem);
                        if (SUCCEEDED(hr))
                        {
                            pFile->SetFolder(pItem);
                            pItem->Release();
                            pItem = NULL;
                        }

                        // Show the Open dialog box.
                        hr = pFile->Show(hwndcopy);
                        // Get the file name from the dialog box.
                        if (SUCCEEDED(hr))
                        {
                            IShellItem *pItem;
                            hr = pFile->GetResult(&pItem);
                            if (SUCCEEDED(hr))
                            {
                                PWSTR pszFilePath;
                                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
                                if (SUCCEEDED(hr))
                                {
                                    r.type=gapplication_Variant::STRING;
                                    r.s=us(pszFilePath);
                                    rets.push_back(r);

                                    CoTaskMemFree(pszFilePath);
                                }
                                pItem->Release();
                            }
                        }
                        pFile->Release();
                    }
                    CoUninitialize();
                    delete[] fileTypes;
                }
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
            /* INFO SHOWN IN GIDEROS STUDIO DEBUGGER, IMPLEMENTED IN QT, NOT NEEDED HERE? */
        }
    }
    else { // SET
        if (strcmp(what, "cursor") == 0)
        {
            int cursorIdx=cursorMap[args[0].s];
            if (cursorIdx==0) cursorIdx=OCR_NORMAL;
            HANDLE cursor=LoadImage(NULL,MAKEINTRESOURCE(cursorIdx),IMAGE_CURSOR,0,0,LR_DEFAULTSIZE|LR_SHARED);
            SetCursor((HCURSOR)cursor);
            SetClassLongPtr(hwndcopy,GCLP_HCURSOR,(LONG_PTR)cursor);
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
            /*------------------------------------------------------------------*/
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
            // new 20221114 XXX
            std::wstring dir = ws(args[0].s.c_str());
            if (_wmkdir(dir.c_str()) == 0)
                printf("mkDir OK: %S\n", dir.c_str()); // can be useful for the w32 console
            else
                printf("mkDir failed or Dir may already exist: %S\n", dir.c_str()); // can be useful for the w32 console
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
