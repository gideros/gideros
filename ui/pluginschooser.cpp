#include "pluginschooser.h"
#include "ui_pluginschooser.h"
#include <QCheckBox>
#include <QTableWidgetItem>
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QtXml/QDomNode>
#include <QDebug>


#ifdef Q_OS_MACX
#define ALL_PLUGINS_PATH "../../All Plugins"
#else
#define ALL_PLUGINS_PATH "All Plugins"
#endif

PluginsChooser::PluginsChooser(QSet<ProjectProperties::Plugin> selection, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PluginsChooserDialog)
{
	sel=selection;

	QSet<QString> enabledPlugins;
	for (QSet<ProjectProperties::Plugin>::iterator it=sel.begin();it!=sel.end();it++)
	{
		if ((*it).enabled)
			enabledPlugins.insert((*it).name);
        pluginsProps[(*it).name]=(*it).properties;
	}

	ui->setupUi(this);

    QStringList plugins;
    QStringList dirs;

    QDir sourceDir(ALL_PLUGINS_PATH);
    dirs = sourceDir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);
    for(int i = 0; i < dirs.count(); i++)
    {
        QDir sourceDir2 = sourceDir;
        if (sourceDir2.cd(dirs[i]))
        {
        	QStringList filters;
        	filters << "*.gplugin";
        	sourceDir2.setNameFilters(filters);
        	QStringList files = sourceDir2.entryList(QDir::Files | QDir::Hidden);
        	for(int i = 0; i < files.count(); i++)
       	       plugins << sourceDir2.absoluteFilePath(files[i]);
        }
    }

    ui->plugins->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->plugins->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->plugins->setRowCount(plugins.count());
    int rows=0;
	for(int i = 0; i < plugins.count(); i++)
	{
	       QDomDocument doc("plugin");
	       QFile file(plugins[i]);
           if (!file.open(QIODevice::ReadOnly))
                continue;
            if (!doc.setContent(&file))
            {
                file.close();
                continue;
            }
            file.close();
            QDomElement plugin = doc.documentElement();
            QString name=plugin.attribute("name");
            pluginsDesc[name]=doc;
            QDomNodeList targets=plugin.elementsByTagName("target");
            QStringList targetList;
            for (int k=0;k<targets.count();k++)
            	targetList << targets.at(k).toElement().attribute("name");

            QCheckBox *cb=new QCheckBox("");
	      ui->plugins->setCellWidget(rows,0,cb);
	      cb->setChecked(enabledPlugins.contains(name));

	      QTableWidgetItem *item;

	      item=new QTableWidgetItem(name);
	      item->setFlags((item->flags()|Qt::ItemIsEnabled)&(~Qt::ItemIsEditable));
	      ui->plugins->setItem(rows,1,item);

	      item=new QTableWidgetItem(plugin.attribute("description"));
	      item->setFlags((item->flags()|Qt::ItemIsEnabled)&(~Qt::ItemIsEditable));
	      ui->plugins->setItem(rows,2,item);

	      item=new QTableWidgetItem(targetList.join(','));
	      item->setFlags((item->flags()|Qt::ItemIsEnabled)&(~Qt::ItemIsEditable));
	      ui->plugins->setItem(rows,3,item);

	      rows++;
	}
    ui->plugins->setRowCount(rows);
    ui->plugins->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

	propsTable=new PropertyEditingTable();
    ui->propTable->addWidget(propsTable);

	connect(this, SIGNAL(accepted()), this, SLOT(onAccepted()));
	connect(ui->plugins, SIGNAL(itemSelectionChanged()), this, SLOT(onSelectionChanged()));
}

PluginsChooser::~PluginsChooser()
{
    delete ui;
}

QSet<ProjectProperties::Plugin> PluginsChooser::selection() const
{
    return sel;
}

void PluginsChooser::onSelectionChanged()
{
	if (!currentPlugin.isEmpty())
		pluginsProps[currentPlugin]=propsTable->extract();
	currentPlugin=QString();
 	propsTable->clearContents();
	QModelIndexList sels=ui->plugins->selectionModel()->selectedRows();
    ui->pluginName->setText(QString("-"));
	if (sels.count())
	{
		QString name=ui->plugins->item(sels.at(0).row(),1)->text();
	    QDomElement plugin = pluginsDesc[name].documentElement();
	    ui->pluginName->setText(name);
		propsTable->fill(plugin,pluginsProps[name]);
		currentPlugin=name;
	}
}

void PluginsChooser::onAccepted()
{
	if (!currentPlugin.isEmpty())
		pluginsProps[currentPlugin]=propsTable->extract();
	sel.clear();
	for(int i = 0; i < ui->plugins->rowCount(); i++)
	{
	 ProjectProperties::Plugin p;
	 p.name=ui->plugins->item(i,1)->text();
	 p.enabled=((QCheckBox *)ui->plugins->cellWidget(i,0))->isChecked();
	 p.properties=pluginsProps[p.name];
	 sel.insert(p);
	}
}
