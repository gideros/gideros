#ifndef PROJECTPROPERTIES_H
#define PROJECTPROPERTIES_H

#include <vector>
#include <QString>
#include <QSet>
#include <QMap>
#include <QDomDocument>

struct ProjectProperties
{
	struct Plugin
	{
		QString name;
		bool enabled;
		QMap<QString,QString> properties;
		bool operator <(const struct Plugin &p) const { return name<p.name; }
	};
	struct Export
	{
		QString name;
		QMap<QString,QString> properties;
		bool operator <(const struct Export &p) const { return name<p.name; }
	};
	ProjectProperties()
	{
		clear();
	}

	void loadXml(QDomElement xml);
	void toXml(QDomDocument doc,QDomElement xml) const;

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
        app_name="";
		architecture = 0;
        android_template = 0;
        exportMode = 0;
		iosDevice = 0;
        version = "1.0";
        version_code = 1;
        build_number = 1;

        ios_bundle = "com.yourdomain.";
		packageName = "com.yourdomain.yourapp";
        osx_org = "GiderosMobile";
        osx_domain = "giderosmobile.com";
        osx_bundle = "com.yourdomain.";
        osx_category = 5;
        win_org = "GiderosMobile";
        win_domain = "giderosmobile.com";
        winrt_org = "GiderosMobile";
        winrt_package = "com.yourdomain.yourapp";
        html5_host = "";
        html5_mem = 256;
        html5_pack = false;
        encryptCode = false;
        encryptAssets = false;
        app_icon="";
        tv_icon="";
        splash_h_image="";
        splash_v_image="";
        disableSplash = false;
        backgroundColor = "#ffffff";
        splashScaleMode = 0;
        plugins.clear();
        exports.clear();
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
	int exportMode;
	int iosDevice;
	QString app_name;
    int version_code;
    QString version;
    int build_number;
    QString ios_bundle;
	QString packageName;
    QString osx_org;
    QString osx_domain;
    QString osx_bundle;
    int osx_category;
    QString win_org;
    QString win_domain;
    QString winrt_org;
    QString winrt_package;
    QString html5_host;
    int html5_mem;
    bool html5_pack;
    QString app_icon;
    QString tv_icon;
    QString splash_h_image;
    QString splash_v_image;
    bool disableSplash;
    QString backgroundColor;
    QSet<Plugin> plugins;
    QSet<Export> exports;
    bool encryptCode;
    bool encryptAssets;
    int splashScaleMode;
};

inline bool operator==(const ProjectProperties::Plugin &p1, const ProjectProperties::Plugin &p2) { return (p1.name==p2.name)&&(p1.properties==p2.properties); };
inline uint qHash(const ProjectProperties::Plugin &p) { return qHash(p.name); };
inline bool operator==(const ProjectProperties::Export &p1, const ProjectProperties::Export &p2) { return (p1.name==p2.name)&&(p1.properties==p2.properties); };
inline uint qHash(const ProjectProperties::Export &p) { return qHash(p.name); };

#endif
