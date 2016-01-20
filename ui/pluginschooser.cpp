#include "pluginschooser.h"
#include "ui_pluginschooser.h"
#include <QCheckBox>
#include <QTableWidgetItem>
#include <QDir>
#include <QFile>
#include <QStringList>
#include <QtXml/QDomNode>


#define ALL_PLUGINS_PATH "All Plugins"

PluginsChooser::PluginsChooser(QSet<QString> selection, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PluginsChooserDialog)
{
	sel=selection;

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
            QDomNodeList targets=plugin.elementsByTagName("target");
            QStringList targetList;
            for (int k=0;k<targets.count();k++)
            	targetList << targets.at(k).toElement().attribute("name");

            QCheckBox *cb=new QCheckBox("");
	      ui->plugins->setCellWidget(rows,0,cb);
	      QString name=plugin.attribute("name");
	      cb->setChecked(selection.contains(name));
	      ui->plugins->setItem(rows,1,new QTableWidgetItem(name));
	      ui->plugins->setItem(rows,2,new QTableWidgetItem(plugin.attribute("description")));
	      ui->plugins->setItem(rows,3,new QTableWidgetItem(targetList.join(',')));

	      rows++;
	}
    ui->plugins->setRowCount(rows);


	connect(this, SIGNAL(accepted()), this, SLOT(onAccepted()));
}

PluginsChooser::~PluginsChooser()
{
    delete ui;
}

QSet<QString> PluginsChooser::selection() const
{
    return sel;
}

void PluginsChooser::onAccepted()
{
	sel.clear();
	for(int i = 0; i < ui->plugins->rowCount(); i++)
	{
	 if (((QCheckBox *)ui->plugins->cellWidget(i,0))->isChecked())
			 sel.insert(ui->plugins->item(i,1)->text());
	}
}
