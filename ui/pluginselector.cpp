#include "pluginselector.h"
#include "ui_pluginselector.h"
#include <QCheckBox>
#include <QTableWidgetItem>
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QtXml/QDomNode>
#include <QDebug>
#include <QStandardPaths>
#include <QPainter>
#include <QLabel>
#include "qtutils.h"

#ifdef Q_OS_MACX
#define ALL_PLUGINS_PATH "../../All Plugins"
#else
#define ALL_PLUGINS_PATH "All Plugins"
#endif

PluginSelector::PluginSelector(QSet<ProjectProperties::Plugin> selection,
		QWidget *parent) :
		QDialog(parent), ui(new Ui::PluginSelectorDialog) {
	sel = "";

	QSet < QString > enabledPlugins;
	for (QSet<ProjectProperties::Plugin>::iterator it = selection.begin();
			it != selection.end(); it++) {
		if ((*it).enabled)
			enabledPlugins.insert((*it).name);
	}

	ui->setupUi(this);
    connect(ui->userPlugins, SIGNAL(clicked()), this, SLOT(onShowUserPlugins()));
    connect(ui->standardPlugins, SIGNAL(clicked()), this, SLOT(onShowStandardPlugins()));

	QStringList plugins;
	QStringList dirs;

	QDir shared(
			QStandardPaths::writableLocation(
					QStandardPaths::GenericDataLocation));
	shared.mkpath("Gideros/UserPlugins");
	bool sharedOk = shared.cd("Gideros");
	sharedOk=sharedOk&&shared.cd("UserPlugins");
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

	QMap<QString,QImage> iconMap;
	iconMap["APK"]=QImage("Resources/platforms/android.png");
	iconMap["iOS"]=QImage("Resources/platforms/ios.jpeg");
	iconMap["Html5"]=QImage("Resources/platforms/html5.png");
	iconMap["WinRT"]=QImage("Resources/platforms/winrt.png");
	iconMap["Windows"]=QImage("Resources/platforms/pc.png");
	iconMap["MacOSX"]=QImage("Resources/platforms/mac.png");
	iconMap["Win32"]=QImage("Resources/platforms/win32.png");

	ui->plugins->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->plugins->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->plugins->setRowCount(plugins.count());
	int rows = 0;
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
		QDomElement plugin = doc.documentElement();
		QString name = plugin.attribute("name");
		if ((!pluginsDesc.contains(name))&&(!enabledPlugins.contains(name))) {
			pluginsDesc[name] = doc;
			QStringList targetList;
			QDomNodeList targets = plugin.elementsByTagName("target");
			for (int k = 0; k < targets.count(); k++)
				targetList << targets.at(k).toElement().attribute("name").split(',', QString::SkipEmptyParts);
			int tcount=0;
			for (int k = 0; k < targetList.count(); k++)
				if (iconMap.contains(targetList.at(k))) tcount++;
			QImage timage(32*tcount,32,QImage::Format_RGB32);
			timage.fill(Qt::white);
			QPainter tpainter(&timage);
			tcount=0;
			for (int k = 0; k < targetList.count(); k++)
			{
				QString iname=targetList.at(k);
				if (iconMap.contains(iname))
				{
					tpainter.drawImage(tcount*32,0,iconMap[iname]);
					tcount++;
				}
			}

			QTableWidgetItem *item;
			item = new QTableWidgetItem;
			QLabel *lbl = new QLabel;
			timage.setDevicePixelRatio(2.0);
			lbl->setPixmap(QPixmap::fromImage(timage));
/*			item->setFlags(
					(item->flags() | Qt::ItemIsEnabled)
							& (~Qt::ItemIsEditable));*/
			ui->plugins->setCellWidget(rows, 0, lbl);

			item = new QTableWidgetItem(name);
			item->setFlags(
					(item->flags() | Qt::ItemIsEnabled)
							& (~Qt::ItemIsEditable));
			ui->plugins->setItem(rows, 1, item);

			item = new QTableWidgetItem(plugin.attribute("description"));
			item->setFlags(
					(item->flags() | Qt::ItemIsEnabled)
							& (~Qt::ItemIsEditable));
			ui->plugins->setItem(rows, 2, item);

			rows++;
		}
	}
	ui->plugins->setRowCount(rows);
	ui->plugins->horizontalHeader()->resizeSections(
			QHeaderView::ResizeToContents);

	connect(this, SIGNAL(accepted()), this, SLOT(onAccepted()));
}

PluginSelector::~PluginSelector() {
	delete ui;
}

QString PluginSelector::selection() const {
	return sel;
}

void PluginSelector::onAccepted() {
	sel="";
	QModelIndexList sels = ui->plugins->selectionModel()->selectedRows();
	if (sels.count()) {
			sel = ui->plugins->item(sels.at(0).row(), 1)->text();
	}
}

void PluginSelector::onShowUserPlugins()
{
	QDir shared(
				QStandardPaths::writableLocation(
						QStandardPaths::GenericDataLocation));
	shared.cd("Gideros");
	shared.cd("UserPlugins");

    doShowInFinder(shared.absolutePath());
}

void PluginSelector::onShowStandardPlugins()
{
	QDir sourceDir(ALL_PLUGINS_PATH);
	doShowInFinder(sourceDir.absolutePath());
}
