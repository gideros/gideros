#include "plugineditor.h"
#include "ui_plugineditor.h"
#include <QCheckBox>
#include <QTableWidgetItem>
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QtXml/QDomNode>
#include <QDebug>
#include <QStandardPaths>
#include "qtutils.h"

#ifdef Q_OS_MACX
#define ALL_PLUGINS_PATH "../../All Plugins"
#else
#define ALL_PLUGINS_PATH "All Plugins"
#endif

PluginEditor::PluginEditor(ProjectProperties::Plugin *selection,QDir projectDir,
		QWidget *parent) :
		QDialog(parent), ui(new Ui::PluginEditorDialog) {

	sel=selection;

	ui->setupUi(this);

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
		if (!pluginsDesc.contains(name)) {
			pluginsDesc[name] = doc;
		}
	}

    propsTable = new PropertyEditingTable(projectDir);
	ui->propTable->addWidget(propsTable);

	ui->pluginName->setText(selection->name);
	if (pluginsDesc.contains(selection->name)) {
		QDomElement plugin = pluginsDesc[selection->name].documentElement();
		propsTable->fill(plugin, sel->properties);
	}

	connect(this, SIGNAL(accepted()), this, SLOT(onAccepted()));
}

PluginEditor::~PluginEditor() {
	delete ui;
}


void PluginEditor::onAccepted() {
	sel->properties = propsTable->extract();
}
