#import <UIKit/UIKit.h>

extern "C" {
UIViewController *g_getRootViewController();
}

static int s_fps = 60;

extern "C" {

int g_getFps()
{
    return s_fps;
}

void g_setFps(int fps)
{
	if (fps != s_fps)
	{
		UIViewController *viewController = g_getRootViewController();
		switch (fps)
		{
		case 30:
			[viewController setAnimationFrameInterval:2];
			break;
		case 60:
			[viewController setAnimationFrameInterval:1];
			break;
		}
		s_fps = fps;
	}
}

}