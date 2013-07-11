#include "dialog.h"
#include "ui_dialog.h"
#include <QNetworkInterface>
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QCryptographicHash>
#include <QMessageBox>

Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

	http_ = new QNetworkAccessManager(this);

	QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

	for (int i = 0; i < interfaces.size(); ++i)
	{
		QString mac = interfaces[i].hardwareAddress();
		if (!mac.isEmpty())
		{
			mac.replace(":", "");
			ui->mac->addItem(mac);
		}
	}
}


inline unsigned char n2a(unsigned char c)		// nibble to ascii
{
	if (c<=9)
	{
		return c+0x30;
	}
	return c-0x0A+'a';
}

inline QString ba2str(const QByteArray& b)
{
	QString result;
	for (int i = 0; i < b.size(); ++i)
	{
		unsigned char l = (b[i] >> 4) & 0x0F;
		unsigned char r = b[i] & 0x0F;
		QChar c[2] = {n2a(l), n2a(r)};
		result += QString(c, 2);
	}
	return result;
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_requestlicense_clicked()
{
	ui->requestlicense->setEnabled(false);

	QUrl url("http://localhost/request-license.php");

	url.addQueryItem("username", ui->username->text());
	url.addQueryItem("password", ba2str(QCryptographicHash::hash(ui->password->text().toUtf8(), QCryptographicHash::Sha1)));
	url.addQueryItem("mac_address", ui->mac->currentText());

	QNetworkRequest r(url);
	QNetworkReply* rep = http_->get(r);
	connect(rep, SIGNAL(finished()), this, SLOT(requestFinished()));
	connect(rep, SIGNAL(error(QNetworkReply::NetworkError)), this, SLOT(requestError(QNetworkReply::NetworkError)));
}

void Dialog::requestFinished()
{
	ui->requestlicense->setEnabled(true);

	QNetworkReply* rep = qobject_cast<QNetworkReply*>(sender());
	rep->disconnect();
	rep->deleteLater();

	QByteArray all = rep->readAll();

//	qDebug() << all.constData();
	QMessageBox::information(this, QString(), all.constData());
}

void Dialog::requestError(QNetworkReply::NetworkError error)
{
	ui->requestlicense->setEnabled(true);

	QNetworkReply* rep = qobject_cast<QNetworkReply*>(sender());
	rep->disconnect();
	rep->deleteLater();

	QMessageBox::information(this, QString(), "Cannot connect to license server.");
}
