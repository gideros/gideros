#include "filedownloader.h"
#include "ExportCommon.h"
#include <math.h>

FileDownloader::FileDownloader(QUrl url, QObject *parent) :
 QObject(parent)
{
 connect(
  &m_WebCtrl, SIGNAL (finished(QNetworkReply*)),
  this, SLOT (fileDownloaded(QNetworkReply*))
  );

 QNetworkRequest request(url);
 request.setAttribute(QNetworkRequest::FollowRedirectsAttribute,QVariant(true));
 QNetworkReply *ret=m_WebCtrl.get(request);
 connect(ret, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT (progress(qint64,qint64)) );
}

FileDownloader::~FileDownloader() { }

void FileDownloader::fileDownloaded(QNetworkReply* pReply) {
 m_DownloadedData = pReply->readAll();
 //emit a signal
 pReply->deleteLater();
while (dsteps<tsteps)
{
	dsteps++;
	ExportCommon::progressStep("Downloading...");
 }
 emit downloaded();
}

void FileDownloader::progress(qint64 amount,qint64 total)
{
	if (!started)
	{
		tsteps=ceil(total/(1024*1024));
		ExportCommon::progressSteps(tsteps);
		started=true;
		dsteps=0;
	}
	int asteps=ceil(amount/(1024*1024));
	while (dsteps<asteps)
	{
		dsteps++;
		ExportCommon::progressStep("Downloading...");
	}
}

QByteArray FileDownloader::downloadedData() const {
 return m_DownloadedData;
}
