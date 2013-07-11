#include "signplayerrunpage.h"
#include "ui_signplayerrunpage.h"
#include <QNetworkAccessManager>
#include <QDebug>
#include <QNetworkRequest>
#include <QByteArray>
#include <QNetworkReply>
#include <QFileInfo>
#include <QMessageBox>
#include <QApplication>
#include <openssl/pkcs12.h>

SignPlayerRunPage::SignPlayerRunPage(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::SignPlayerRunPage)
{
    ui->setupUi(this);
	http_ = new QNetworkAccessManager(this);
	isComplete_ = false;
}

SignPlayerRunPage::~SignPlayerRunPage()
{
    delete ui;
}
/*
http://blog.ilric.org/2010/11/20/qt-http-post-multi-partupload/
http://code.activestate.com/recipes/146306-http-client-to-post-using-multipartform-data/
http://stackoverflow.com/questions/3085990/post-a-file-string-using-curl-in-php
http://stackoverflow.com/questions/2627949/upload-file-with-post-method-on-qt4

  */
void SignPlayerRunPage::run()
{
	isComplete_ = false;
	ui->output->clear();
//	ui->run->setEnabled(false);
	ui->progress->setMaximum(0);
	ui->progress->setValue(0);

	QString privateKey = field("privateKey").toString();
	QString developerSigningCertificate = field("developerSigningCertificate").toString();
	QString provisioningProfile = field("provisioningProfile").toString();

	QList<QString> names;
	names.push_back("private_key");
	names.push_back("developer_certificate");
	names.push_back("mobile_provision");

	QList<QString> filenames;
	filenames.push_back(QFileInfo(privateKey).fileName());
	filenames.push_back(QFileInfo(developerSigningCertificate).fileName());
	filenames.push_back(QFileInfo(provisioningProfile).fileName());

	QList<QByteArray> contents;
	{
		QFile file(privateKey);
		if (!file.open(QIODevice::ReadOnly))
		{
			QMessageBox::warning(this, QApplication::applicationName(), tr("Cannot read file %1:\n%2.")
								 .arg(privateKey)
								 .arg(file.errorString()));
//			ui->run->setEnabled(true);
			ui->progress->setMaximum(100);
			isComplete_ = true;
			emit completeChanged();
			return;
		}
		contents.push_back(file.readAll());
	}
	{
		QFile file(developerSigningCertificate);
		if (!file.open(QIODevice::ReadOnly))
		{
			QMessageBox::warning(this, QApplication::applicationName(), tr("Cannot read file %1:\n%2.")
								 .arg(developerSigningCertificate)
								 .arg(file.errorString()));
//			ui->run->setEnabled(true);
			ui->progress->setMaximum(100);
			isComplete_ = true;
			emit completeChanged();
			return;
		}
		contents.push_back(file.readAll());
	}
	{
		QFile file(provisioningProfile);
		if (!file.open(QIODevice::ReadOnly))
		{
			QMessageBox::warning(this, QApplication::applicationName(), tr("Cannot read file %1:\n%2.")
								 .arg(provisioningProfile)
								 .arg(file.errorString()));
//			ui->run->setEnabled(true);
			ui->progress->setMaximum(100);
			isComplete_ = true;
			emit completeChanged();
			return;
		}
		contents.push_back(file.readAll());
	}

/*
	// calismadi. valid .p12 dosyasi icin cannot parse diyo
	{
		// open private key
		QFile file(privateKey);
		file.open(QIODevice::ReadOnly);
		QByteArray buffer = file.readAll();
		qDebug << buffer.constData();
		BIO* mem = BIO_new_mem_buf(buffer.data(), buffer.size());
		BIO_set_close(mem, BIO_NOCLOSE);

		PKCS12* p12 = d2i_PKCS12_bio(mem, NULL);

		if (!PKCS12_parse(p12, "", NULL, NULL, NULL))
		{
			qDebug() << "cannot parse .p12 file";
		}

		PKCS12_free(p12);
		BIO_free(mem);
	} */

	QNetworkRequest r(QUrl("http://192.168.170.128/upload_file.php"));

	QByteArray  data;

	const char CRLF[] = {0xd, 0xa, 0};
	QString BOUNDARY = "---------------------------723690991551375881941828858";

	// username
	data += "--" + BOUNDARY + CRLF;
	data += QString("Content-Disposition: form-data; name=\"%1\"").arg("user_name") + CRLF;
	data += CRLF;
	data += "Atilim Cetin 12345";
	data += CRLF;

	for (int i = 0; i < 3; ++i)
	{
		QString name = names[i];
		QString filename = filenames[i];
		QByteArray content = contents[i];

		data += "--" + BOUNDARY + CRLF;
		data += QString("Content-Disposition: form-data; name=\"%1\"; filename=\"%2\"").arg(name).arg(filename) + CRLF;
		data += QString("Content-Type: octet-stream") + CRLF;
		data += CRLF;
		data += content;
		data += CRLF;
	}
	data += "--" + BOUNDARY + "--" + CRLF;
	data += CRLF;

	r.setRawHeader(QString("Accept-Encoding").toAscii(), QString("gzip,deflate").toAscii());
	r.setRawHeader(QString("Content-Type").toAscii(),QString("multipart/form-data; boundary=" + BOUNDARY).toAscii());
	r.setRawHeader(QString("Content-Length").toAscii(), QString::number(data.length()).toAscii());


	QNetworkReply* rep = http_->post(r, data);
	connect(rep, SIGNAL(finished()), this, SLOT(uploadFinished()));
	connect(rep, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(uploadError(QNetworkReply::NetworkError)));
}

void SignPlayerRunPage::uploadFinished()
{
	qDebug() << "uploadFinished";

	QNetworkReply* rep = qobject_cast<QNetworkReply*>(sender());

//	ui->output->appendPlainText(rep->readAll());
	QString result;
	while (true)
	{
		QByteArray line = rep->readLine();
		if (line.isEmpty())
			break;

		if (line.endsWith(0x0a))
			line.chop(1);
		if (line.endsWith(0x0d))
			line.chop(1);

		ui->output->appendPlainText(line);

		if (line.startsWith("--result--"))
			result = line.right(line.size() - 10);
	}

	rep->disconnect();
	rep->deleteLater();

	if (!result.isEmpty())
	{
		QNetworkReply* rep = http_->get(QNetworkRequest(QUrl("http://192.168.170.128" + result)));
		connect(rep, SIGNAL(finished()), this, SLOT(downloadFinished()));
		connect(rep, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(downloadError(QNetworkReply::NetworkError)));
		connect(rep, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
	}
	else
	{
		QMessageBox::warning(this, QApplication::applicationName(), tr("Error while signing the application."));
//		ui->run->setEnabled(true);
		ui->progress->setMaximum(100);
		isComplete_ = true;
		emit completeChanged();
	}
}

void SignPlayerRunPage::downloadFinished()
{
	qDebug() << "downloadFinished";

	QNetworkReply* rep = qobject_cast<QNetworkReply*>(sender());

	QByteArray all = rep->readAll();

	if (all.size() > 1024)
	{
		QString output = field("output").toString();
		QFile file(output);
		if (!file.open(QIODevice::WriteOnly))
		{
			QMessageBox::warning(this, QApplication::applicationName(), tr("Cannot write file %1:\n%2.")
								 .arg(output)
								 .arg(file.errorString()));
		}
		else
		{
			ui->output->appendPlainText("writing to: " + output);
			file.write(all);
			file.close();
			QMessageBox::information(this, QApplication::applicationName(), tr(".ipa file is downloaded and saved successfully."));
		}
	}
	else
	{
		QMessageBox::warning(this, QApplication::applicationName(), tr("Cannot download .ipa file."));
		ui->progress->setMaximum(100);
	}

	rep->disconnect();
	rep->deleteLater();

//	ui->run->setEnabled(true);
	isComplete_ = true;
	emit completeChanged();
}

void SignPlayerRunPage::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
	if (bytesTotal != 0)
	{
		ui->progress->setMaximum(bytesTotal);
		ui->progress->setValue(bytesReceived);
	}
//	qDebug() << bytesReceived << " " << bytesTotal;
}

void SignPlayerRunPage::uploadError(QNetworkReply::NetworkError error)
{
	qDebug() << "uploadError";

	QNetworkReply* rep = qobject_cast<QNetworkReply*>(sender());
	rep->disconnect();
	rep->deleteLater();
	ui->progress->setMaximum(100);
	QMessageBox::warning(this, QApplication::applicationName(), tr("Cannot connect signing server."));
//	ui->run->setEnabled(true);
	isComplete_ = true;
	emit completeChanged();
}

void SignPlayerRunPage::downloadError(QNetworkReply::NetworkError error)
{
	qDebug() << "downloadError";
	return;

	QNetworkReply* rep = qobject_cast<QNetworkReply*>(sender());
	rep->disconnect();
	rep->deleteLater();
	ui->progress->setMaximum(100);
	QMessageBox::warning(this, QApplication::applicationName(), tr("Cannot download .ipa file."));
//	ui->run->setEnabled(true);
	isComplete_ = true;
	emit completeChanged();
}
