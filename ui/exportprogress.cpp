#include "exportprogress.h"
#include "ui_exportprogress.h"
#include <QDebug>

ExportProgress::ExportProgress(QProcess *exportProcess_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportProgress)
{
	ui->setupUi(this);
	connect(ui->btEnd, SIGNAL(clicked()), this, SLOT(onEnd()));

	exportProcess=exportProcess_;

    //exportProcess->setStandardErrorFile(out.absoluteFilePath("error.log"));
	connect(exportProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(onStandardOutput()));
	connect(exportProcess, SIGNAL(readyReadStandardError()), this, SLOT(onStandardError()));
	connect(exportProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onFinished(int, QProcess::ExitStatus)));
    exportProcess->start();

}

void ExportProgress::onStandardError()
{
	errorLog.append(exportProcess->readAllStandardError());
	int sepPos;
	while ((sepPos=errorLog.indexOf('\n'))>=0)
	{
		QString line=QString(errorLog.left(sepPos+1)).trimmed();
		errorLog.remove(0,sepPos+1);
		ui->lbExport->append(line);
	}
}

void ExportProgress::onStandardOutput()
{
	outputLog.append(exportProcess->readAllStandardOutput());
	int sepPos;
	while ((sepPos=outputLog.indexOf('\n'))>=0)
	{
		QString line=QString(outputLog.left(sepPos+1)).trimmed();
		outputLog.remove(0,sepPos+1);
		if (line.startsWith(':'))
		{
			int s1=line.indexOf(":",1);
			if (s1>=0)
			{
				ui->pgExport->setMaximum(line.mid(1,s1-1).toInt());
				int s2=line.indexOf(":",s1+1);
				if (s2>=0)
				{
					ui->pgExport->setValue(line.mid(s1+1,s2-s1-1).toInt());
					ui->pgExport->setFormat(line.mid(s2+1));
					ui->pgExport->setTextVisible(true);
				}
				else
				{
					ui->pgExport->setValue(line.mid(s1+1).toInt());
					ui->pgExport->setTextVisible(false);
				}
			}
			else
				ui->pgExport->setMaximum(line.mid(1).toInt());
		}
		else
			ui->lbExport->append(line);
	}
}

void ExportProgress::onEnd()
{
	accept();
}

void ExportProgress::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
	ui->btEnd->setText("Done");
}

ExportProgress::~ExportProgress()
{
    delete ui;
}
