#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QNetworkReply>

namespace Ui {
    class Dialog;
}

class Dialog : public QDialog
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

private:
    Ui::Dialog *ui;

private slots:
	void on_requestlicense_clicked();
	void requestFinished();
	void requestError(QNetworkReply::NetworkError error);

private:
	QNetworkAccessManager* http_;
};


#endif // DIALOG_H
