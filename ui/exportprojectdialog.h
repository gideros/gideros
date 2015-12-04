#ifndef EXPORTPROJECTDIALOG_H
#define EXPORTPROJECTDIALOG_H

#include <QDialog>
#include <QMap>
class ProjectProperties;

namespace Ui {
    class ExportProjectDialog;
}

class ExportProjectDialog : public QDialog
{
    Q_OBJECT

public:
	enum DeviceFamily
	{
	  e_iOS,
	  e_Android,
      e_WindowsDesktop,
      e_MacOSXDesktop,
	  e_WinRT,
	  e_GApp,
	  e_Win32,
	  e_Html5
	};

    explicit ExportProjectDialog(ProjectProperties* properties, bool licensed, QWidget *parent = 0);
    ~ExportProjectDialog();

	DeviceFamily deviceFamily() const;
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
    bool assetsOnly() const;
    bool encryptCode() const;
    bool encryptAssets() const;

private slots:
	void onAccepted();

private:
    Ui::ExportProjectDialog *ui;
	ProjectProperties* properties_;
    QMap<QString, QString> osxCat;
};

#endif // EXPORTPROJECTDIALOG_H
