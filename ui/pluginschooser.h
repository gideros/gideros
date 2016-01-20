#ifndef PLUGINSCHOOSER_H
#define PLUGINSCHOOSER_H

#include <QDialog>
#include <QSet>

namespace Ui {
    class PluginsChooserDialog;
}

class PluginsChooser : public QDialog
{
    Q_OBJECT

public:
    explicit PluginsChooser(QSet<QString> selection, QWidget *parent = 0);
    ~PluginsChooser();

	QSet<QString> selection() const;

private slots:
	void onAccepted();

private:
    Ui::PluginsChooserDialog *ui;
    QSet<QString> sel;
};

#endif // PLUGINSCHOOSER_H
