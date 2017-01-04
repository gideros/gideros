#include "filedownloader.h"
#include "ExportCommon.h"
#include <math.h>

FileDownloader::FileDownloader(QUrl url, bool check, quint64 expectedSize, QObject *parent) :
 QObject(parent)
{
    m_FileSize=expectedSize;
 connect(
  &m_WebCtrl, SIGNAL (finished(QNetworkReply*)),
  this, SLOT (fileDownloaded(QNetworkReply*))
  );

    started=false;
    tsteps=0;
    dsteps=0;
 QNetworkRequest request(url);
 request.setAttribute(QNetworkRequest::FollowRedirectsAttribute,QVariant(true));
 netRet=check?m_WebCtrl.head(request):m_WebCtrl.get(request);
 if (expectedSize)
     m_DownloadedData.reserve(expectedSize);
 if (!check)
 {
	 connect(netRet, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT (progress(qint64,qint64)) );
     connect(netRet, SIGNAL(readyRead()), this, SLOT (canRead()) );
 }
}

FileDownloader::~FileDownloader() { }

void FileDownloader::fileDownloaded(QNetworkReply* pReply) {
 m_DownloadedData.append(pReply->readAll());
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

void FileDownloader::canRead()
{
    m_DownloadedData.append(netRet->readAll());
}

void FileDownloader::progress(qint64 amount,qint64 total)
{
    if (total<0) total=m_FileSize;
    //ExportCommon::exportInfo("Downloaded %lld/%lld bytes\n",amount,total);
	if (!started)
	{
		tsteps=ceil(total/(1024*1024));
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
