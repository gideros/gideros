#ifndef EXPORTPROJECTDIALOG_H
#define EXPORTPROJECTDIALOG_H

#include <QDialog>
#include <QMap>
#include <QSet>
#include <QDir>
#include "projectproperties.h"

#ifdef Q_OS_MACX
#define ALL_PLUGINS_PATH "../../All Plugins"
#define TEMPLATES_PATH "../../Templates"
#else
#define ALL_PLUGINS_PATH "All Plugins"
#define TEMPLATES_PATH "Templates"
#endif

namespace Ui {
    class ExportProjectDialog;
}

class ExportProjectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ExportProjectDialog(ProjectProperties* properties, QDir projectDir, bool licensed, QWidget *parent = 0);
    ~ExportProjectDialog();

	QString exportType() const;
    QString ios_bundle() const;
    QString atv_bundle() const;
    QString macos_bundle() const;
    QString macos_category() const;
    QString packageName() const;
    QString androidTemplate() const;
    QString qtexp_platform() const;
    QString qtexp_org() const;
    QString qtexp_domain() const;
    QString osx_bundle() const;
    QString osx_category() const;
    QString osx_signingId() const;
    QString osx_installerId() const;
    QString winrt_org() const;
    QString winrt_package() const;
    QString html5_host() const;
    int exportMode() const;
    bool encryptCode() const;
    bool encryptAssets() const;

private slots:
	void onAccepted();
	void actionHtml5FbInstant(int);
	//void onSelectPlugins();

private:
    Ui::ExportProjectDialog *ui;
	ProjectProperties* properties_;
	//QSet<ProjectProperties::Plugin> plugins;
	int xmlTabCount;
	QStringList exportTypes;
    QStringList qtPlatforms;
    QMap<QString, QString> osxCat;
};

#endif // EXPORTPROJECTDIALOG_H
