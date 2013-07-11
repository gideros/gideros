#ifndef DEACTIVATEALLDIALOG_H
#define DEACTIVATEALLDIALOG_H

#include <QDialog>

namespace Ui {
    class DeactivateAllDialog;
}

class QNetworkAccessManager;
class QNetworkReply;

class DeactivateAllDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DeactivateAllDialog(QWidget *parent = 0);
    ~DeactivateAllDialog();

private slots:
	void on_deactivateAll_clicked();
	void finished(QNetworkReply*);

private:
    Ui::DeactivateAllDialog *ui;

private:
	QNetworkAccessManager* manager_;
};

#endif // DEACTIVATEALLDIALOG_H
