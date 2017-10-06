#include <stdio.h>
#include <windows.h>
#include <vector>
#include <string>
#include <stdlib.h>

#include "luaapplication.h"
#include <application.h>
#include <gapplication-win32.h>

extern HWND hwndcopy;
extern char commandLine[];
// extern int dxChrome,dyChrome;
extern LuaApplication *application_;

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
  ShellExecute(hwndcopy,NULL,url,NULL,NULL,SW_SHOWNORMAL);
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

  strcat(szBuff1,"_");
  strcat(szBuff1,szBuff2);

  return szBuff1;
}

std::string getLanguage()
{
  TCHAR szBuff[10]; 
  LCID lcid = GetUserDefaultLCID(); 
  GetLocaleInfo(lcid, LOCALE_SISO639LANGNAME, szBuff, 10); 
  return szBuff;
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

bool g_checkStringProperty(bool isSet, const char* what){
    return false;
}

void g_setProperty(const char* what, const char* arg){

}

const char* g_getProperty(const char* what, const char* arg)
{
  if (strcmp(what,"commandLine")==0)
    return commandLine;
}
