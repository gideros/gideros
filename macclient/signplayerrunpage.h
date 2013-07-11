#ifndef SIGNPLAYERRUNPAGE_H
#define SIGNPLAYERRUNPAGE_H

#include <QWizardPage>
#include <QNetworkReply>

namespace Ui {
    class SignPlayerRunPage;
}

class QNetworkAccessManager;

class SignPlayerRunPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit SignPlayerRunPage(QWidget *parent = 0);
    ~SignPlayerRunPage();

	virtual void initializePage()
	{
		run();
	}

	bool isComplete() const
	{
		return isComplete_;
	}

private:
    Ui::SignPlayerRunPage *ui;

private slots:
	void run();
	void uploadFinished();
	void downloadFinished();
	void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
	void uploadError(QNetworkReply::NetworkError error);
	void downloadError(QNetworkReply::NetworkError error);

private:
	QNetworkAccessManager* http_;
	bool isComplete_;
};

#endif // SIGNPLAYERRUNPAGE_H
