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

void ProjectProperties::toXml(QXmlStreamWriter &out) const
{
	// graphics options
    out.writeAttribute("scaleMode", QString::number(this->scaleMode));
    out.writeAttribute("logicalWidth", QString::number(this->logicalWidth));
    out.writeAttribute("logicalHeight", QString::number(this->logicalHeight));
    out.writeAttribute("windowWidth", QString::number(this->windowWidth));
    out.writeAttribute("windowHeight", QString::number(this->windowHeight));

    int ar2=0;
    switch (this->autorotation) {
        case 0:
           ar2=0; //no rotation
        break;
        case 1:
           ar2=4; //2-way
        break;
        case 2:
           ar2=3; //4-way
        break;
        default:
           ar2=this->autorotation+2; //Other modes
    }
    out.writeAttribute("autorotation", QString::number(ar2));
    out.writeAttribute("orientation", QString::number(this->orientation));
    out.writeAttribute("fps", QString::number(this->fps));
    out.writeAttribute("vsync", QString::number(this->vsync));
    out.writeAttribute("version", this->version);
    out.writeAttribute("version_code", QString::number(this->version_code));
    out.writeAttribute("build_number", QString::number(this->build_number));
    out.writeAttribute("mainluaOnly", QString::number(this->mainluaOnly));

    // iOS options
    out.writeAttribute("retinaDisplay", QString::number(this->retinaDisplay));

    // input options
    out.writeAttribute("mouseToTouch", QString::number(this->mouseToTouch ? 1 : 0));
    out.writeAttribute("touchToMouse", QString::number(this->touchToMouse ? 1 : 0));
    out.writeAttribute("mouseTouchOrder", QString::number(this->mouseTouchOrder));

    // export options
    out.writeAttribute("architecture", QString::number(this->architecture));
    out.writeAttribute("exportMode", QString::number(this->exportMode));
    out.writeAttribute("iosDevice", QString::number(this->iosDevice));
    out.writeAttribute("ios_bundle", this->ios_bundle);
    out.writeAttribute("atv_bundle", this->atv_bundle);
    out.writeAttribute("macos_bundle", this->macos_bundle);
    out.writeAttribute("macos_category", QString::number(this->macos_category));
    out.writeAttribute("qtexp_platform", QString::number(this->qtexp_platform));
    out.writeAttribute("qtexp_org", this->qtexp_org);
    out.writeAttribute("qtexp_domain", this->qtexp_domain);
    out.writeAttribute("osx_bundle", this->osx_bundle);
    out.writeAttribute("osx_signingId", this->osx_signingId);
    out.writeAttribute("osx_installerId", this->osx_installerId);
    out.writeAttribute("osx_category", QString::number(this->osx_category));
    out.writeAttribute("winrt_org", this->winrt_org);
    out.writeAttribute("winrt_package", this->winrt_package);
    out.writeAttribute("html5_host", this->html5_host);
    out.writeAttribute("html5_crash", this->html5_crash);
    out.writeAttribute("html5_mem", QString::number(this->html5_mem));
    out.writeAttribute("html5_pack", QString::number(this->html5_pack ? 1 : 0));
    out.writeAttribute("html5_wasm", QString::number(this->html5_wasm ? 1 : 0));
    out.writeAttribute("html5_symbols", QString::number(this->html5_symbols ? 1 : 0));
    out.writeAttribute("html5_fbinstant", QString::number(this->html5_fbinstant ? 1 : 0));
    out.writeAttribute("html5_pwa", QString::number(this->html5_pwa ? 1 : 0));
    out.writeAttribute("html5_fbload", QString::number(this->html5_fbload));
    out.writeAttribute("encryptCode", QString::number(this->encryptCode));
    out.writeAttribute("encryptAssets", QString::number(this->encryptAssets));
    out.writeAttribute("app_icon", this->app_icon);
    out.writeAttribute("tv_icon", this->tv_icon);
    out.writeAttribute("disableSplash", QString::number(this->disableSplash));
    out.writeAttribute("backgroundColor", this->backgroundColor);
    out.writeAttribute("splash_h_image", this->splash_h_image);
    out.writeAttribute("splash_v_image", this->splash_v_image);
    out.writeAttribute("app_name", this->app_name);

    out.writeAttribute("splashScaleMode", QString::number(this->splashScaleMode));

    out.writeStartElement("imageScales");
	for (size_t i = 0; i < this->imageScales.size(); ++i)
	{
        out.writeStartElement("scale");
        out.writeAttribute("suffix", this->imageScales[i].first);
        out.writeAttribute("scale", QString::number(this->imageScales[i].second));
        out.writeEndElement();
	}
    out.writeEndElement();

    //Plugins
    out.writeStartElement("plugins");
    QList<Plugin> pl=this->plugins.values();
    std::sort(pl.begin(),pl.end());
	for (QList<Plugin>::const_iterator it=pl.begin();it!=pl.end(); it++)
	{
        out.writeStartElement("plugin");
		Plugin p=*it;
        out.writeAttribute("name", p.name);
        out.writeAttribute("enabled", QString(p.enabled?"1":"0"));
		QList<QString> ml=p.properties.keys();
        std::sort(ml.begin(),ml.end());
		for (QList<QString>::const_iterator i=ml.begin();i!=ml.end();i++) {
            out.writeStartElement("property");
            out.writeAttribute("name",*i);
            out.writeAttribute("value",p.properties[*i]);
            out.writeEndElement();
        }
        out.writeEndElement();
    }
    out.writeEndElement();

	//Exports
    out.writeStartElement("exports");
    QList<Export> el=this->exports.values();
    std::sort(el.begin(),el.end());
	for (QList<Export>::const_iterator it=el.begin();it!=el.end(); it++)
	{
        out.writeStartElement("export");
		Export p=*it;
        out.writeAttribute("name", p.name);
		QList<QString> ml=p.properties.keys();
        std::sort(ml.begin(),ml.end());
		for (QList<QString>::const_iterator i=ml.begin();i!=ml.end();i++) {
            out.writeStartElement("property");
            out.writeAttribute("name",*i);
            out.writeAttribute("value",p.properties[*i]);
            out.writeEndElement();
        }
        out.writeEndElement();
    }
    out.writeEndElement();
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
		if (!properties.attribute("vsync").isEmpty())
			this->vsync = properties.attribute("vsync").toInt();
		if (!properties.attribute("mainluaOnly").isEmpty())
			this->mainluaOnly = properties.attribute("mainluaOnly").toInt();

		// iOS options
		if (!properties.attribute("retinaDisplay").isEmpty())
            this->retinaDisplay = properties.attribute("retinaDisplay").toInt();
        if (!properties.attribute("autorotation").isEmpty()) {
            // for backward compatibility, map old 1,2,3 values (iphone or ipad only) to full rotation
            int ar=properties.attribute("autorotation").toInt();
            int ar2=0;
            switch (ar) {
                case 0:
                    ar2=0; //no rotation
                break;
                case 1:
                case 2:
                case 3:
                   ar2=2; //4-way
                break;
                case 4:
                   ar2=1; //2-way
                break;
                default:
                   ar2=ar-2; //other modes
            }
            this->autorotation = ar2;
        }
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
		if (!properties.attribute("exportMode").isEmpty())
			this->exportMode = properties.attribute("exportMode").toInt();
		if (!properties.attribute("iosDevice").isEmpty())
			this->iosDevice = properties.attribute("iosDevice").toInt();
        if (!properties.attribute("app_name").isEmpty())
            this->app_name = properties.attribute("app_name");
        if (!properties.attribute("ios_bundle").isEmpty())
            this->ios_bundle = properties.attribute("ios_bundle");
        if (!properties.attribute("atv_bundle").isEmpty())
            this->atv_bundle = properties.attribute("atv_bundle");
        if (!properties.attribute("macos_bundle").isEmpty())
            this->macos_bundle = properties.attribute("macos_bundle");
        if (!properties.attribute("macos_category").isEmpty())
            this->macos_category = properties.attribute("macos_category").toInt();
        if (!properties.attribute("qtexp_platform").isEmpty())
            this->qtexp_platform = properties.attribute("qtexp_platform").toInt();
        if (!properties.attribute("qtexp_org").isEmpty())
            this->qtexp_org = properties.attribute("qtexp_org");
        if (!properties.attribute("qtexp_domain").isEmpty())
            this->qtexp_domain = properties.attribute("qtexp_domain");
        if (!properties.attribute("osx_bundle").isEmpty())
            this->osx_bundle = properties.attribute("osx_bundle");
        if (!properties.attribute("osx_signingId").isEmpty())
            this->osx_signingId = properties.attribute("osx_signingId");
        if (!properties.attribute("osx_installerId").isEmpty())
            this->osx_installerId = properties.attribute("osx_installerId");
        if (!properties.attribute("osx_category").isEmpty())
            this->osx_category = properties.attribute("osx_category").toInt();
        if (!properties.attribute("winrt_org").isEmpty())
            this->winrt_org = properties.attribute("winrt_org");
        if (!properties.attribute("winrt_package").isEmpty())
            this->winrt_package = properties.attribute("winrt_package");
        if (!properties.attribute("html5_host").isEmpty())
            this->html5_host = properties.attribute("html5_host");
        if (!properties.attribute("html5_crash").isEmpty())
            this->html5_crash = properties.attribute("html5_crash");
        if (!properties.attribute("html5_mem").isEmpty())
            this->html5_mem = properties.attribute("html5_mem").toInt();
        if (!properties.attribute("html5_pack").isEmpty())
            this->html5_pack = properties.attribute("html5_pack").toInt() != 0;
        if (!properties.attribute("html5_wasm").isEmpty())
            this->html5_wasm = properties.attribute("html5_wasm").toInt() != 0;
        if (!properties.attribute("html5_symbols").isEmpty())
            this->html5_symbols = properties.attribute("html5_symbols").toInt() != 0;
        if (!properties.attribute("html5_fbinstant").isEmpty())
            this->html5_fbinstant = properties.attribute("html5_fbinstant").toInt() != 0;
        if (!properties.attribute("html5_pwa").isEmpty())
            this->html5_pwa = properties.attribute("html5_pwa").toInt() != 0;
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
