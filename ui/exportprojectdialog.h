#ifndef EXPORTPROJECTDIALOG_H
#define EXPORTPROJECTDIALOG_H

#include <QDialog>
#include <QMap>
#include <QSet>
#include "projectproperties.h"

namespace Ui {
    class ExportProjectDialog;
}

class ExportProjectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportProjectDialog(ProjectProperties* properties, bool licensed, QWidget *parent = 0);
    ~ExportProjectDialog();

	QString exportType() const;
    QString ios_bundle() const;
	QString packageName() const;
    QString androidTemplate() const;
    QString osx_org() const;
    QString osx_domain() const;
    QString osx_bundle() const;
    QString osx_category() const;
    QString win_org() const;
    QString win_domain() const;
    QString winrt_org() const;
    QString winrt_package() const;
    QString html5_host() const;
    int exportMode() const;
    bool encryptCode() const;
    bool encryptAssets() const;

private slots:
	void onAccepted();
	void onSelectPlugins();

private:
    Ui::ExportProjectDialog *ui;
	ProjectProperties* properties_;
	QSet<ProjectProperties::Plugin> plugins;
	int xmlTabStart;
	QStringList exportTypes;
    QMap<QString, QString> osxCat;
};

#endif // EXPORTPROJECTDIALOG_H
