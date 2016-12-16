#include "filedownloader.h"
#include "ExportCommon.h"
#include <math.h>

FileDownloader::FileDownloader(QUrl url, bool check, QObject *parent) :
 QObject(parent)
{
 connect(
  &m_WebCtrl, SIGNAL (finished(QNetworkReply*)),
  this, SLOT (fileDownloaded(QNetworkReply*))
  );

 QNetworkRequest request(url);
 request.setAttribute(QNetworkRequest::FollowRedirectsAttribute,QVariant(true));
 QNetworkReply *ret=check?m_WebCtrl.head(request):m_WebCtrl.get(request);
 if (!check)
	 connect(ret, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT (progress(qint64,qint64)) );

}

FileDownloader::~FileDownloader() { }

void FileDownloader::fileDownloaded(QNetworkReply* pReply) {
 m_DownloadedData = pReply->readAll();
 m_FileSize = pReply->header(QNetworkRequest::ContentLengthHeader).toUInt();
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
		tsteps=(total>0)?ceil(total/(1024*1024)):0;
		if (tsteps)
			ExportCommon::progressSteps(tsteps);
		started=true;
		dsteps=0;
	}
	if (tsteps)
	{
		int asteps=ceil(amount/(1024*1024));
		while (dsteps<asteps)
		{
			dsteps++;
			ExportCommon::progressStep("Downloading...");
		}
	}
}

QByteArray FileDownloader::downloadedData() const {
 return m_DownloadedData;
}

quint64 FileDownloader::fileSize() const {
 return m_FileSize;
}
