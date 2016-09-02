#include "projectproperties.h"


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
    properties.setAttribute("osx_category", this->osx_category);
    properties.setAttribute("win_org", this->win_org);
	properties.setAttribute("win_domain", this->win_domain);
    properties.setAttribute("winrt_org", this->winrt_org);
    properties.setAttribute("winrt_package", this->winrt_package);
    properties.setAttribute("html5_host", this->html5_host);
    properties.setAttribute("html5_mem", this->html5_mem);
    properties.setAttribute("encryptCode", this->encryptCode);
    properties.setAttribute("encryptAssets", this->encryptAssets);
    properties.setAttribute("app_icon", this->app_icon);
    properties.setAttribute("tv_icon", this->tv_icon);
    properties.setAttribute("disableSplash", this->disableSplash);
    properties.setAttribute("backgroundColor", this->backgroundColor);
    properties.setAttribute("splash_h_image", this->splash_h_image);
    properties.setAttribute("splash_v_image", this->splash_v_image);

    //Plugins
	QDomElement plugins = doc.createElement("plugins");
	for (QSet<Plugin>::const_iterator it=this->plugins.begin();it!=this->plugins.end(); it++)
	{
		QDomElement plugin = doc.createElement("plugin");
		Plugin p=*it;
		plugin.setAttribute("name", p.name);
		plugin.setAttribute("enabled", QString(p.enabled?"1":"0"));
		QMap<QString, QString>::const_iterator i = p.properties.cbegin();
		while (i != p.properties.cend()) {
			QDomElement attr = doc.createElement("property");
			attr.setAttribute("name",i.key());
			attr.setAttribute("value",i.value());
			plugin.appendChild(attr);
		    ++i;
		}
		plugins.appendChild(plugin);
	}
	properties.appendChild(plugins);

	//Exports
	QDomElement exports = doc.createElement("exports");
	for (QSet<Export>::const_iterator it=this->exports.begin();it!=this->exports.end(); it++)
	{
		QDomElement plugin = doc.createElement("export");
		Export p=*it;
		plugin.setAttribute("name", p.name);
		QMap<QString, QString>::const_iterator i = p.properties.cbegin();
		while (i != p.properties.cend()) {
			QDomElement attr = doc.createElement("property");
			attr.setAttribute("name",i.key());
			attr.setAttribute("value",i.value());
			plugin.appendChild(attr);
		    ++i;
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
