
/*

   Porting notes
   -------------
   
   Need to provide window/framebuffer setup at program start
   
   ddkUnlock() is called when the window should be redrawn
   
   WinMain() at the bottom of this file is the entry point and main loop
   which handles messages and calling the app update function ddkCalcFrame()
   
   ddkscreen32  =  pointer to 640x480 DWORD pixel buffer
   mouse_*  =  mouse info, only need x/y/px/py/left/right/leftclick/rightclick

*/


#define _WIN32_WINDOWS 0xBAD
#include <windows.h>

extern "C" long _ftol( double ); //defined by VC6 C libs
extern "C" long _ftol2( double dblSource ) { return _ftol( dblSource ); }

void ddkInit();      // Will be called on startup
bool ddkCalcFrame(); // Will be called every frame, return true to continue running or false to quit
void ddkFree();      // Will be called on shutdown
bool ddkLock();   // Call immediately before drawing (once per frame)
void ddkUnlock(); // Call immediately after drawing (once per frame)
void ddkDrawPixel(int x, int y, int red, int green, int blue); // Draw a pixel
void ddkSetMode(int width, int height, int bpp, int refreshrate, int fullscreen, char *title);
#define DDK_WINDOW		0 // Run in a normal resizable window (stretch to fit)
#define DDK_FULLSCREEN	1 // Change display mode to the one specified, use vsync
#define DDK_STRETCH		2 // Run in a maximized window, looks like fullscreen
int ddkGetBpp(); // Use this to find out the current color depth
DWORD *ddkscreen32; // Use this for 32 bit modes
WORD  *ddkscreen16; // Use this for 16 bit modes
int   ddkpitch; // Offset in pixels from the start of one horizontal line to the start of the next

bool ddrawkit_initialised;
bool ddrawkit_released;
bool ddrawkit_fullscreen;
bool ddrawkit_fullwindow;
int ddrawkit_width;
int ddrawkit_height;
int ddrawkit_bpp;
int ddrawkit_refresh;
bool ddrawkit_timeravailable;

HWND hWndMain;
HINSTANCE hInstanceMain;

HCURSOR cursor_arrow;
int mouse_x, mouse_y, mouse_px, mouse_py;
bool mouse_left=false, mouse_right=false, mouse_middle=false;
bool mouse_leftclick=false, mouse_rightclick=false, mouse_middleclick=false;
bool mouse_doubleclick=false;
int mouse_wheel=0;


// --- DDB implementation

struct pBITMAPINFO
{
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD bmiColors[256];
} BMInfo;

HBITMAP hBM;

HDC hDC_comp;
DWORD* image_bitmap;

void InitImageBuffer()
{
	HPALETTE PalHan;
	HWND ActiveWindow;
	HDC hDC;

	image_bitmap=(DWORD*)malloc(ddrawkit_width*ddrawkit_height*sizeof(DWORD));

	ActiveWindow=hWndMain;
	hDC=GetDC(ActiveWindow);

	BMInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
	BMInfo.bmiHeader.biWidth=ddrawkit_width;
	BMInfo.bmiHeader.biHeight=-abs(ddrawkit_height);
	BMInfo.bmiHeader.biPlanes=1;
	BMInfo.bmiHeader.biBitCount=32;
	BMInfo.bmiHeader.biCompression=BI_RGB;
	BMInfo.bmiHeader.biSizeImage=ddrawkit_width*ddrawkit_height*4;
	BMInfo.bmiHeader.biXPelsPerMeter=0;
	BMInfo.bmiHeader.biYPelsPerMeter=0;

	hBM=CreateDIBSection(hDC, (BITMAPINFO*)&BMInfo, DIB_RGB_COLORS, (void**)&image_bitmap, 0, 0);
	ReleaseDC(ActiveWindow, hDC);
	
	ddkscreen32=image_bitmap;
	ddkpitch=ddrawkit_width;
}

void DestroyImageBuffer()
{
	free(image_bitmap);
}

void ddrawkit_BlitWindowDDB()
{
	HDC hDC;
	HBITMAP DefaultBitmap;

	hDC=GetDC(hWndMain);
	DefaultBitmap=(HBITMAP)SelectObject(hDC_comp, hBM);
//	BitBlt(hDC, x1, y1, x2-x1, y2-y1, hDC_comp, x1, y1, SRCCOPY);
	BitBlt(hDC, 0, 0, ddrawkit_width, ddrawkit_height, hDC_comp, 0, 0, SRCCOPY);
	SelectObject(hDC_comp, DefaultBitmap);
	DeleteDC(hDC);
}

// --- DDB implementation


void ddrawkit_SafeDestroy()
{
	if(!ddrawkit_released)
	{
		ddrawkit_released=true;
		ddkFree();
		DestroyWindow(hWndMain);
		DestroyImageBuffer();
	}
}

LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		ddrawkit_SafeDestroy();
		return 0;

	case WM_SETCURSOR:
		if(ddrawkit_fullscreen || ddrawkit_fullwindow)
		{
			SetCursor(NULL);
			return 0;
		}
		else
			SetCursor(cursor_arrow);
		break;
			
	case WM_MOUSEMOVE:
		int curwidth, curheight;
		RECT rect;
		GetClientRect(hWndMain, &rect);
		curwidth=rect.right-rect.left;
		curheight=rect.bottom-rect.top;
		mouse_x=(int)((float)LOWORD(lParam)/curwidth*ddrawkit_width);
		mouse_y=(int)((float)HIWORD(lParam)/curheight*ddrawkit_height);
		return 0;
	case WM_MOUSEWHEEL:
		if((wParam>>16)&0x7FFF)
		{
			if((wParam>>16)&0x8000)
				mouse_wheel=-1;
			else
				mouse_wheel=1;
		}
		return 0;
	case WM_LBUTTONDBLCLK:
		mouse_doubleclick=true;
		return 0;
	case WM_LBUTTONDOWN:
		mouse_left=true;
		mouse_leftclick=true;
		return 0;
	case WM_LBUTTONUP:
		mouse_left=false;
		return 0;
	case WM_RBUTTONDOWN:
		mouse_right=true;
		mouse_rightclick=true;
		return 0;
	case WM_RBUTTONUP:
		mouse_right=false;
		return 0;
	case WM_MBUTTONDOWN:
		mouse_middle=true;
		mouse_middleclick=true;
		return 0;
	case WM_MBUTTONUP:
		mouse_middle=false;
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void ddrawkit_RegisterWindowClass()
{
	WNDCLASSEX	wcx;

	ZeroMemory(&wcx, sizeof(WNDCLASSEX));
	wcx.cbSize=sizeof(WNDCLASSEX);
	wcx.lpfnWndProc=MainWindowProc;
	wcx.hInstance=GetModuleHandle(NULL);
	wcx.hIcon=LoadIcon(NULL, IDI_APPLICATION);
	wcx.hCursor=NULL;
//	wcx.hbrBackground=(HBRUSH)GetStockObject(BLACK_BRUSH);
	wcx.hbrBackground=NULL;
	wcx.lpszMenuName=NULL;
	wcx.lpszClassName="DDrawKitClass";
	RegisterClassEx(&wcx);
}

bool ddkLock()
{
	return true;
}

void ddkUnlock()
{
	ddrawkit_BlitWindowDDB();
}

void ddkDrawPixel(int x, int y, int red, int green, int blue)
{
	if(x<0 || y<0 || x>=ddrawkit_width || y>=ddrawkit_height)
		return;
		
	if(ddrawkit_bpp==32)
	{
		DWORD color=(red<<16)|(green<<8)|blue;
		ddkscreen32[y*ddkpitch+x]=color;
	}
	if(ddrawkit_bpp==16)
	{
		WORD color=((red>>3)<<11)|((green>>2)<<5)|(blue>>3);
		ddkscreen16[y*ddkpitch+x]=color;
	}
}

void ddkSetMode(int width, int height, int bpp, int refreshrate, int fullscreen, char *title)
{	
	if(!ddrawkit_initialised)
	{
		ddrawkit_width=width;
		ddrawkit_height=height;
		ddrawkit_bpp=bpp;
		ddrawkit_refresh=refreshrate;
		switch(fullscreen)
		{
		case 0:
			ddrawkit_fullscreen=false;
			ddrawkit_fullwindow=false;
			break;
		case 1:
			ddrawkit_fullscreen=true;
			ddrawkit_fullwindow=false;
			break;
		case 2:
			ddrawkit_fullscreen=false;
			ddrawkit_fullwindow=true;
			break;
		}

		ddrawkit_RegisterWindowClass();

		if(ddrawkit_fullscreen || ddrawkit_fullwindow)
		{
			// Screen sized window (possibly stretched)
			hWndMain=CreateWindowEx(WS_EX_APPWINDOW,
				"DDrawKitClass", title, WS_POPUP,
				0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN),
				NULL, NULL, hInstanceMain, NULL);
		}
		else
		{
			// Normal window
//			hWndMain=CreateWindow("DDrawKitClass", title, WS_OVERLAPPEDWINDOW,
			hWndMain=CreateWindow("DDrawKitClass", title, WS_OVERLAPPED|WS_BORDER|WS_CAPTION|WS_SYSMENU|WS_MINIMIZEBOX,
//				0, 0, ddrawkit_width+GetSystemMetrics(SM_CXSIZEFRAME)*2,
//					  ddrawkit_height+GetSystemMetrics(SM_CYSIZEFRAME)*2+GetSystemMetrics(SM_CYCAPTION),
				GetSystemMetrics(SM_CXSCREEN)/2-320, GetSystemMetrics(SM_CYSCREEN)/2-300,
				ddrawkit_width+GetSystemMetrics(SM_CXEDGE)*2,
				ddrawkit_height+GetSystemMetrics(SM_CYEDGE)*2+GetSystemMetrics(SM_CYCAPTION),
				NULL, NULL, hInstanceMain, NULL);
		}

		// init DDB implementation
		hDC_comp=CreateCompatibleDC(NULL);
	 	InitImageBuffer();
		
		ShowWindow(hWndMain, SW_SHOW);
	
		ddrawkit_initialised=true;
		ddrawkit_released=false;

		// Clear screen
		ddkLock();
		for(int y=0;y<ddrawkit_height;y++)
			for(int x=0;x<ddrawkit_width;x++)
			{
				if(ddrawkit_bpp==32)
					ddkscreen32[y*ddkpitch+x]=0;
				else
					ddkscreen16[y*ddkpitch+x]=0;
			}
		ddkUnlock();
	}
}

int ddkGetBpp()
{
	return ddrawkit_bpp;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int) 
{
	MSG	msg;
	
	hInstanceMain=hInstance;

	cursor_arrow=LoadCursor(NULL, IDC_ARROW);

	ddkInit();

	LARGE_INTEGER startTime;
	LARGE_INTEGER frequency;

	if(!QueryPerformanceFrequency(&frequency))
		ddrawkit_timeravailable=false;
	else
	{
		ddrawkit_timeravailable=true;
		QueryPerformanceCounter(&startTime);
	}

	for(;;)
	{
		while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if(!GetMessage(&msg, NULL, 0, 0))
				break;
			DispatchMessage(&msg);
		}

		if(ddrawkit_released)
			break;
		
		if(!ddkCalcFrame())
		{
			ddrawkit_SafeDestroy();
			break;
		}
		mouse_px=mouse_x;
		mouse_py=mouse_y;

		mouse_leftclick=false;
		mouse_rightclick=false;
		mouse_middleclick=false;
		mouse_doubleclick=false;
	}

	return 0;
}

