#include "projectproperties.h"
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QStandardPaths>

#ifdef Q_OS_MACX
#define ALL_PLUGINS_PATH "../../All Plugins"
#else
#define ALL_PLUGINS_PATH "All Plugins"
#endif

QMap<QString, QString> ProjectProperties::availablePlugins() {
	QMap < QString, QString > xmlPlugins;
	QStringList plugins;
	QStringList dirs;

	QDir shared(
			QStandardPaths::writableLocation(
					QStandardPaths::GenericDataLocation));
	shared.mkpath("Gideros/UserPlugins");
	bool sharedOk = shared.cd("Gideros") && shared.cd("UserPlugins");
	if (sharedOk) {
		dirs = shared.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
		for (int i = 0; i < dirs.count(); i++) {
			QDir sourceDir2 = shared;
			if (sourceDir2.cd(dirs[i])) {
				QStringList filters;
				filters << "*.gplugin";
				sourceDir2.setNameFilters(filters);
				QStringList files = sourceDir2.entryList(
						QDir::Files | QDir::Hidden);
				for (int i = 0; i < files.count(); i++)
					plugins << sourceDir2.absoluteFilePath(files[i]);
			}
		}
	}

	QDir sourceDir(ALL_PLUGINS_PATH);
	dirs = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
	for (int i = 0; i < dirs.count(); i++) {
		QDir sourceDir2 = sourceDir;
		if (sourceDir2.cd(dirs[i])) {
			QStringList filters;
			filters << "*.gplugin";
			sourceDir2.setNameFilters(filters);
			QStringList files = sourceDir2.entryList(
					QDir::Files | QDir::Hidden);
			for (int i = 0; i < files.count(); i++)
				plugins << sourceDir2.absoluteFilePath(files[i]);
		}
	}

	for (int i = 0; i < plugins.count(); i++) {
		QDomDocument doc("plugin");
		QFile file(plugins[i]);
		if (!file.open(QIODevice::ReadOnly))
			continue;
		if (!doc.setContent(&file)) {
			file.close();
			continue;
		}
		file.close();
		QDomElement exporter = doc.documentElement();
		QString exname = exporter.attribute("name");
		if (!xmlPlugins.contains(exname))
			xmlPlugins[exname] = plugins[i];
	}
	return xmlPlugins;
}

void ProjectProperties::toXml(QDomDocument doc,QDomElement properties) const
{
	// graphics options
	properties.setAttribute("scaleMode", this->scaleMode);
	properties.setAttribute("logicalWidth", this->logicalWidth);
	properties.setAttribute("logicalHeight", this->logicalHeight);
    properties.setAttribute("windowWidth", this->windowWidth);
    properties.setAttribute("windowHeight", this->windowHeight);
	QDomElement imageScales = doc.createElement("imageScales");
	for (size_t i = 0; i < this->imageScales.size(); ++i)
	{
		QDomElement scale = doc.createElement("scale");

		scale.setAttribute("suffix", this->imageScales[i].first);
		scale.setAttribute("scale", this->imageScales[i].second);

		imageScales.appendChild(scale);
	}
	properties.appendChild(imageScales);
	properties.setAttribute("orientation", this->orientation);
	properties.setAttribute("fps", this->fps);
    properties.setAttribute("version", this->version);
    properties.setAttribute("version_code", this->version_code);
    properties.setAttribute("build_number", this->build_number);

	// iOS options
    properties.setAttribute("retinaDisplay", this->retinaDisplay);
	properties.setAttribute("autorotation", this->autorotation);

    // input options
    properties.setAttribute("mouseToTouch", this->mouseToTouch ? 1 : 0);
    properties.setAttribute("touchToMouse", this->touchToMouse ? 1 : 0);
    properties.setAttribute("mouseTouchOrder", this->mouseTouchOrder);

	// export options
	properties.setAttribute("architecture", this->architecture);
    properties.setAttribute("android_template", this->android_template);
	properties.setAttribute("exportMode", this->exportMode);
	properties.setAttribute("iosDevice", this->iosDevice);
    properties.setAttribute("ios_bundle", this->ios_bundle);
	properties.setAttribute("packageName", this->packageName);
	properties.setAttribute("osx_org", this->osx_org);
	properties.setAttribute("osx_domain", this->osx_domain);
    properties.setAttribute("osx_bundle", this->osx_bundle);
    properties.setAttribute("osx_signingId", this->osx_signingId);
    properties.setAttribute("osx_installerId", this->osx_installerId);
    properties.setAttribute("osx_category", this->osx_category);
    properties.setAttribute("win_org", this->win_org);
	properties.setAttribute("win_domain", this->win_domain);
    properties.setAttribute("winrt_org", this->winrt_org);
    properties.setAttribute("winrt_package", this->winrt_package);
    properties.setAttribute("html5_host", this->html5_host);
    properties.setAttribute("html5_mem", this->html5_mem);
    properties.setAttribute("html5_pack", this->html5_pack ? 1 : 0);
    properties.setAttribute("html5_fbinstant", this->html5_fbinstant ? 1 : 0);
    properties.setAttribute("html5_fbload", this->html5_fbload);
    properties.setAttribute("encryptCode", this->encryptCode);
    properties.setAttribute("encryptAssets", this->encryptAssets);
    properties.setAttribute("app_icon", this->app_icon);
    properties.setAttribute("tv_icon", this->tv_icon);
    properties.setAttribute("disableSplash", this->disableSplash);
    properties.setAttribute("backgroundColor", this->backgroundColor);
    properties.setAttribute("splash_h_image", this->splash_h_image);
    properties.setAttribute("splash_v_image", this->splash_v_image);
    properties.setAttribute("app_name", this->app_name);

    properties.setAttribute("splashScaleMode", this->splashScaleMode);

    //Plugins
	QDomElement plugins = doc.createElement("plugins");
	QList<Plugin> pl=this->plugins.toList();
	qSort(pl);
	for (QList<Plugin>::const_iterator it=pl.begin();it!=pl.end(); it++)
	{
		QDomElement plugin = doc.createElement("plugin");
		Plugin p=*it;
		plugin.setAttribute("name", p.name);
		plugin.setAttribute("enabled", QString(p.enabled?"1":"0"));
		QList<QString> ml=p.properties.keys();
		qSort(ml);
		for (QList<QString>::const_iterator i=ml.begin();i!=ml.end();i++) {
			QDomElement attr = doc.createElement("property");
			attr.setAttribute("name",*i);
			attr.setAttribute("value",p.properties[*i]);
			plugin.appendChild(attr);
		}
		plugins.appendChild(plugin);
	}
	properties.appendChild(plugins);

	//Exports
	QDomElement exports = doc.createElement("exports");
	QList<Export> el=this->exports.toList();
	qSort(el);
	for (QList<Export>::const_iterator it=el.begin();it!=el.end(); it++)
	{
		QDomElement plugin = doc.createElement("export");
		Export p=*it;
		plugin.setAttribute("name", p.name);
		QList<QString> ml=p.properties.keys();
		qSort(ml);
		for (QList<QString>::const_iterator i=ml.begin();i!=ml.end();i++) {
			QDomElement attr = doc.createElement("property");
			attr.setAttribute("name",*i);
			attr.setAttribute("value",p.properties[*i]);
			plugin.appendChild(attr);
		}
		exports.appendChild(plugin);
	}
	properties.appendChild(exports);
}

void ProjectProperties::loadXml(QDomElement properties)
{
		// graphics options
		if (!properties.attribute("scaleMode").isEmpty())
			this->scaleMode = properties.attribute("scaleMode").toInt();
		if (!properties.attribute("logicalWidth").isEmpty())
			this->logicalWidth = properties.attribute("logicalWidth").toInt();
		if (!properties.attribute("logicalHeight").isEmpty())
			this->logicalHeight = properties.attribute("logicalHeight").toInt();
        if (!properties.attribute("windowWidth").isEmpty())
            this->windowWidth = properties.attribute("windowWidth").toInt();
        if (!properties.attribute("windowHeight").isEmpty())
            this->windowHeight = properties.attribute("windowHeight").toInt();
		QDomElement imageScales = properties.firstChildElement("imageScales");
		for(QDomNode n = imageScales.firstChild(); !n.isNull(); n = n.nextSibling())
		{
			QDomElement scale = n.toElement();
			if(!scale.isNull())
				this->imageScales.push_back(std::make_pair(scale.attribute("suffix"), scale.attribute("scale").toDouble()));
		}
		if (!properties.attribute("orientation").isEmpty())
			this->orientation = properties.attribute("orientation").toInt();
		if (!properties.attribute("fps").isEmpty())
			this->fps = properties.attribute("fps").toInt();

		// iOS options
		if (!properties.attribute("retinaDisplay").isEmpty())
            this->retinaDisplay = properties.attribute("retinaDisplay").toInt();
		if (!properties.attribute("autorotation").isEmpty())
			this->autorotation = properties.attribute("autorotation").toInt();
        if (!properties.attribute("version").isEmpty())
            this->version = properties.attribute("version");
        if (!properties.attribute("version_code").isEmpty())
            this->version_code = properties.attribute("version_code").toInt();
        if (!properties.attribute("build_number").isEmpty())
            this->build_number = properties.attribute("build_number").toInt();

        // input options
        if (!properties.attribute("mouseToTouch").isEmpty())
            this->mouseToTouch = properties.attribute("mouseToTouch").toInt() != 0;
        if (!properties.attribute("touchToMouse").isEmpty())
            this->touchToMouse = properties.attribute("touchToMouse").toInt() != 0;
        if (!properties.attribute("mouseTouchOrder").isEmpty())
            this->mouseTouchOrder = properties.attribute("mouseTouchOrder").toInt();

		// export options
		if (!properties.attribute("architecture").isEmpty())
			this->architecture = properties.attribute("architecture").toInt();
        if (!properties.attribute("android_template").isEmpty())
            this->android_template = properties.attribute("android_template").toInt();
		if (!properties.attribute("exportMode").isEmpty())
			this->exportMode = properties.attribute("exportMode").toInt();
		if (!properties.attribute("iosDevice").isEmpty())
			this->iosDevice = properties.attribute("iosDevice").toInt();
        if (!properties.attribute("app_name").isEmpty())
            this->app_name = properties.attribute("app_name");
        if (!properties.attribute("ios_bundle").isEmpty())
            this->ios_bundle = properties.attribute("ios_bundle");
		if (!properties.attribute("packageName").isEmpty())
			this->packageName = properties.attribute("packageName");
        if (!properties.attribute("osx_org").isEmpty())
			this->osx_org = properties.attribute("osx_org");
        if (!properties.attribute("osx_domain").isEmpty())
			this->osx_domain = properties.attribute("osx_domain");
        if (!properties.attribute("osx_bundle").isEmpty())
            this->osx_bundle = properties.attribute("osx_bundle");
        if (!properties.attribute("osx_signingId").isEmpty())
            this->osx_signingId = properties.attribute("osx_signingId");
        if (!properties.attribute("osx_installerId").isEmpty())
            this->osx_installerId = properties.attribute("osx_installerId");
        if (!properties.attribute("osx_category").isEmpty())
            this->osx_category = properties.attribute("osx_category").toInt();
        if (!properties.attribute("win_org").isEmpty())
			this->win_org = properties.attribute("win_org");
        if (!properties.attribute("win_domain").isEmpty())
			this->win_domain = properties.attribute("win_domain");
        if (!properties.attribute("winrt_org").isEmpty())
            this->winrt_org = properties.attribute("winrt_org");
        if (!properties.attribute("winrt_package").isEmpty())
            this->winrt_package = properties.attribute("winrt_package");
        if (!properties.attribute("html5_host").isEmpty())
            this->html5_host = properties.attribute("html5_host");
        if (!properties.attribute("html5_mem").isEmpty())
            this->html5_mem = properties.attribute("html5_mem").toInt();
        if (!properties.attribute("html5_pack").isEmpty())
            this->html5_pack = properties.attribute("html5_pack").toInt() != 0;
        if (!properties.attribute("html5_fbinstant").isEmpty())
            this->html5_fbinstant = properties.attribute("html5_fbinstant").toInt() != 0;
        if (!properties.attribute("html5_fbload").isEmpty())
            this->html5_fbload = properties.attribute("html5_fbload").toInt();
        if (!properties.attribute("encryptCode").isEmpty())
            this->encryptCode = properties.attribute("encryptCode").toInt() != 0;
        if (!properties.attribute("encryptAssets").isEmpty())
            this->encryptAssets = properties.attribute("encryptAssets").toInt() != 0;
        if (!properties.attribute("app_icon").isEmpty())
            this->app_icon = properties.attribute("app_icon");
        if (!properties.attribute("tv_icon").isEmpty())
            this->tv_icon = properties.attribute("tv_icon");
        if (!properties.attribute("splash_h_image").isEmpty())
            this->splash_h_image = properties.attribute("splash_h_image");
        if (!properties.attribute("splash_v_image").isEmpty())
            this->splash_v_image = properties.attribute("splash_v_image");
        if(!properties.attribute("disableSplash").isEmpty())
            this->disableSplash = properties.attribute("disableSplash").toInt() != 0;
        if(!properties.attribute("backgroundColor").isEmpty())
            this->backgroundColor = properties.attribute("backgroundColor");
        if (!properties.attribute("splashScaleMode").isEmpty())
            this->splashScaleMode = properties.attribute("splashScaleMode").toInt();

        //Plugins
        this->plugins.clear();
		QDomElement plugins = properties.firstChildElement("plugins");
		for(QDomNode n = plugins.firstChild(); !n.isNull(); n = n.nextSibling())
		{
			QDomElement plugin = n.toElement();
			if ((!plugin.isNull())&&(plugin.tagName()=="plugin"))
			{
				Plugin p;
				p.name=plugin.attribute("name");
				p.enabled=plugin.attribute("enabled").toInt()?1:0;
				for(QDomNode n = plugin.firstChild(); !n.isNull(); n = n.nextSibling())
				{
					QDomElement attr = n.toElement();
					if ((!attr.isNull())&&(attr.tagName()=="property"))
						p.properties.insert(attr.attribute("name"),attr.attribute("value"));
				}
				this->plugins.insert(p);
			}
		}

        //Exports
        this->exports.clear();
		QDomElement exports = properties.firstChildElement("exports");
		for(QDomNode n = exports.firstChild(); !n.isNull(); n = n.nextSibling())
		{
			QDomElement plugin = n.toElement();
			if ((!plugin.isNull())&&(plugin.tagName()=="export"))
			{
				Export p;
				p.name=plugin.attribute("name");
				for(QDomNode n = plugin.firstChild(); !n.isNull(); n = n.nextSibling())
				{
					QDomElement attr = n.toElement();
					if ((!attr.isNull())&&(attr.tagName()=="property"))
						p.properties.insert(attr.attribute("name"),attr.attribute("value"));
				}
				this->exports.insert(p);
			}
		}
}
