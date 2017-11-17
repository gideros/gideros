#ifndef PLUGINEDITOR_H
#define PLUGINEDITOR_H

#include <QDialog>
#include <QSet>
#include <QString>
#include <QtXml/QDomNode>
#include "projectproperties.h"
#include "propertyeditingtable.h"

namespace Ui {
    class PluginEditorDialog;
}

class PluginEditor : public QDialog
{
    Q_OBJECT

public:
    explicit PluginEditor(ProjectProperties::Plugin *selection, QWidget *parent = 0);
    ~PluginEditor();

private slots:
	void onAccepted();

private:
    Ui::PluginEditorDialog *ui;
    QMap<QString,QDomDocument> pluginsDesc;
    QMap<QString,QString> pluginProps;
    ProjectProperties::Plugin *sel;
    PropertyEditingTable *propsTable;
};

#endif // PLUGINSCHOOSER_H
