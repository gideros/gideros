#ifndef PLUGINSCHOOSER_H
#define PLUGINSCHOOSER_H

#include <QDialog>
#include <QSet>
#include "projectproperties.h"

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

private:
    Ui::PluginsChooserDialog *ui;
    QSet<ProjectProperties::Plugin> sel;
};

#endif // PLUGINSCHOOSER_H
