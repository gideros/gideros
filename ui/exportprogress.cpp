#include "exportprogress.h"
#include "ui_exportprogress.h"
#include <QDebug>
#include <QLineEdit>
#include <QInputDialog>

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
    exportProcess->start(QProcess::Unbuffered | QProcess::ReadWrite);

}

void ExportProgress::onStandardError()
{
	errorLog.append(exportProcess->readAllStandardError());
	int sepPos;
	while ((sepPos=errorLog.indexOf('\n'))>=0)
	{
		QString line=QString(errorLog.left(sepPos+1)).trimmed();
		errorLog.remove(0,sepPos+1);
		QColor col=ui->lbExport->palette().color(QPalette::Text);
		ui->lbExport->setTextColor(QColor("red"));
		ui->lbExport->append(line);
		ui->lbExport->setTextColor(col);
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
		if (line.startsWith("$:$"))
		{
			int s1=line.indexOf(":",3);
			if (s1>=0)
			{
				ui->pgExport->setMaximum(line.mid(3,s1-3).toInt());
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
				ui->pgExport->setMaximum(line.mid(3).toInt());
		}
		else if (line.startsWith("?:?"))
		{
			QChar type=line[3];
			int s1=line.indexOf("|",4);
			if (s1>=0)
			{
				int s2=line.indexOf("|",s1+1);
				if (s2>=0)
				{
					bool ok;
					QString text = QInputDialog::getText(this, line.mid(4,s1-4),
				                                         line.mid(s1+1,s2-s1-1), QLineEdit::Normal,
				                                         line.mid(s2+1), &ok);
					if (!ok)
						text=line.mid(s2+1);
					exportProcess->write((text+"\n").toUtf8());
				}
			}
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
	if (exitStatus==QProcess::NormalExit)
	{
		ui->lbExport->append("Export done.");
	}
	else
	{
		ui->lbExport->setTextColor(QColor("red"));
		ui->lbExport->append("Export failed! See details above.");
	}

	ui->btEnd->setText("Done");
}

ExportProgress::~ExportProgress()
{
    delete ui;
}
