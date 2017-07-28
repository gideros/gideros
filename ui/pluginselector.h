#ifndef PLUGINSELECTOR_H
#define PLUGINSELECTOR_H

#include <QDialog>
#include <QSet>
#include <QString>
#include <QtXml/QDomNode>
#include "projectproperties.h"
#include "propertyeditingtable.h"

namespace Ui {
    class PluginSelectorDialog;
}

class PluginSelector : public QDialog
{
    Q_OBJECT

public:
    explicit PluginSelector(QSet<ProjectProperties::Plugin> selection, QWidget *parent = 0);
    ~PluginSelector();

	QString selection() const;

private slots:
	void onAccepted();
    void onShowUserPlugins();
    void onShowStandardPlugins();

private:
    Ui::PluginSelectorDialog *ui;
    QString sel;
    QMap<QString,QDomDocument> pluginsDesc;
};

#endif // PLUGINSCHOOSER_H
