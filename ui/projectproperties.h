#ifndef PROJECTPROPERTIES_H
#define PROJECTPROPERTIES_H

#include <vector>
#include <QString>

struct ProjectProperties
{
	ProjectProperties()
	{
		clear();
	}

	void clear()
	{
		// graphics options
		scaleMode = 0;
		logicalWidth = 320;
		logicalHeight = 480;
        windowWidth = 0;
        windowHeight = 0;
		imageScales.clear();
		orientation = 0;
		fps = 60;

		// iOS options
        retinaDisplay = 0;
		autorotation = 0;

        // input options
        mouseToTouch = true;
        touchToMouse = true;
        mouseTouchOrder = 0;

        // export options
		architecture = 0;
        android_template = 0;
		assetsOnly = false;
		iosDevice = 0;
        version = "1.0";
        version_code = 1;
        ios_bundle = "com.yourdomain.";
		packageName = "com.yourdomain.yourapp";
        osx_org = "GiderosMobile";
        osx_domain = "giderosmobile.com";
        osx_bundle = "com.yourdomain.";
        win_org = "GiderosMobile";
        win_domain = "giderosmobile.com";
        winrt_org = "GiderosMobile";
        winrt_package = "com.yourdomain.yourapp";
        html5_host = "";
        encryptCode = false;
        encryptAssets = false;
    }

	// graphics options
	int scaleMode;
	int logicalWidth;
	int logicalHeight;
    int windowWidth;
    int windowHeight;
	std::vector<std::pair<QString, double> > imageScales;
	int orientation;
	int fps;

	// iOS options
    int retinaDisplay;
	int autorotation;

    // input options
    bool mouseToTouch;
    bool touchToMouse;
    int mouseTouchOrder;

    // export options
	int architecture;
    int android_template;
	bool assetsOnly;
	int iosDevice;
    int version_code;
    QString version;
    QString ios_bundle;
	QString packageName;
    QString osx_org;
    QString osx_domain;
    QString osx_bundle;
    QString win_org;
    QString win_domain;
    QString winrt_org;
    QString winrt_package;
    QString html5_host;
    bool encryptCode;
    bool encryptAssets;
};

#endif
