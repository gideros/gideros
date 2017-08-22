#include "exportprogress.h"
#include "ui_exportprogress.h"
#include <QDebug>
#include <QLineEdit>
#include <QInputDialog>
#include "qtutils.h"

static QMap<QString,QString> responseCache;
ExportProgress::ExportProgress(QProcess *exportProcess_, QString& out, QWidget *parent) :
    _out(out),
    QDialog(parent),
    ui(new Ui::ExportProgress)
{
	ui->setupUi(this);
	connect(ui->btEnd, SIGNAL(clicked()), this, SLOT(onEnd()));
    connect(ui->btShow, SIGNAL(clicked()), this, SLOT(onShowInFinder()));

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
					int s3=line.indexOf("|",s2+1);
					if (s3>=0)
					{
						bool ok;
						QString title=line.mid(4,s1-4);
						QString question=line.mid(s1+1,s2-s1-1);
						QString def=line.mid(s2+1,s3-s2-1);
						QString uid=line.mid(s3+1);
						QString text;
						if ((!uid.isEmpty())&&(responseCache.contains(uid)))
							def=responseCache[uid];
						text = QInputDialog::getText(this, title, question,
									(type=='K')?QLineEdit::Password:QLineEdit::Normal,
									def, &ok);
						if (ok)
						{
							if (!uid.isEmpty())
								responseCache[uid]=text;
						}
						else
							text=def;
						exportProcess->write((text+"\n").toUtf8());
					}
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

void ExportProgress::onShowInFinder()
{
    doShowInFinder(_out);
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
