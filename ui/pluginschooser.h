#ifndef PLUGINSCHOOSER_H
#define PLUGINSCHOOSER_H

#include <QDialog>
#include <QSet>
#include <QString>
#include <QtXml/QDomNode>
#include "projectproperties.h"
#include "propertyeditingtable.h"

namespace Ui {
    class PluginsChooserDialog;
}

class PluginsChooser : public QDialog
{
    Q_OBJECT

public:
    explicit PluginsChooser(QSet<ProjectProperties::Plugin> selection, QWidget *parent = 0);
    ~PluginsChooser();

	QSet<ProjectProperties::Plugin> selection() const;

private slots:
	void onAccepted();
	void onSelectionChanged();

private:
    Ui::PluginsChooserDialog *ui;
    QSet<ProjectProperties::Plugin> sel;
    QMap<QString,QDomDocument> pluginsDesc;
    QMap<QString,QMap<QString,QString> > pluginsProps;
    QString currentPlugin;
    PropertyEditingTable *propsTable;
};

#endif // PLUGINSCHOOSER_H
