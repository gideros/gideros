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
	properties.setAttribute("assetsOnly", this->assetsOnly ? 1 : 0);
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
    properties.setAttribute("app_icon_noexport", this->app_icon_noexport);

    //Plugins
	QDomElement plugins = doc.createElement("plugins");
	for (QSet<QString>::const_iterator it=this->plugins.begin();it!=this->plugins.end(); it++)
	{
		QDomElement plugin = doc.createElement("plugin");
		plugin.setAttribute("name", *it);
		plugins.appendChild(plugin);
	}
	properties.appendChild(plugins);
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
		if (!properties.attribute("assetsOnly").isEmpty())
			this->assetsOnly = properties.attribute("assetsOnly").toInt() != 0;
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
        if (!properties.attribute("app_icon_noexport").isEmpty())
            this->app_icon_noexport = properties.attribute("app_icon_noexport").toInt() != 0;

        //Plugins
        this->plugins.clear();
		QDomElement plugins = properties.firstChildElement("plugins");
		for(QDomNode n = plugins.firstChild(); !n.isNull(); n = n.nextSibling())
		{
			QDomElement plugin = n.toElement();
			if(!plugin.isNull())
				this->plugins.insert(plugin.attribute("name"));
		}
}
