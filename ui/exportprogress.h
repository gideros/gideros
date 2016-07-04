#ifndef EXPORTPROGRESS_H
#define EXPORTPROGRESS_H

#include <QDialog>
#include <QProcess>
#include <QByteArray>

namespace Ui {
    class ExportProgress;
}

class ExportProgress : public QDialog
{
    Q_OBJECT
	QProcess *exportProcess;
    QByteArray errorLog;
    QByteArray outputLog;
public:
    explicit ExportProgress(QProcess *exportProcess_,QWidget *parent = 0);
    ~ExportProgress();

private slots:
	void onStandardError();
	void onStandardOutput();
	void onEnd();
	void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    Ui::ExportProgress *ui;

};

#endif // EXPORTPROGRESS_H
