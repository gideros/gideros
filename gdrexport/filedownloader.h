#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

class FileDownloader : public QObject
{
 Q_OBJECT
 public:
  explicit FileDownloader(QUrl url, bool check=false, quint64 expectedSize=0,QObject *parent = 0);
  virtual ~FileDownloader();
  QByteArray downloadedData() const;
    quint64 fileSize() const ;

 signals:
  void downloaded();

 private slots:
  void fileDownloaded(QNetworkReply* pReply);
  void progress(qint64 amount,qint64 total);
  void canRead();

 private:
  bool started;
  int tsteps,dsteps;
  QNetworkAccessManager m_WebCtrl;
  QNetworkReply *netRet;
  QByteArray m_DownloadedData;
  quint64 m_FileSize;
};

#endif // FILEDOWNLOADER_H
