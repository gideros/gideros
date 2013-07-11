#ifndef ACTIVATEWIDGET_H
#define ACTIVATEWIDGET_H

#include <QWidget>

class QNetworkAccessManager;
class QNetworkReply;

namespace Ui {
    class ActivateWidget;
}

class ActivateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ActivateWidget(QWidget *parent = 0);
    ~ActivateWidget();

signals:
	void updateUI();

private:
    Ui::ActivateWidget *ui;

private slots:
	void on_loginButton_clicked();
	void on_login_returnPressed();
	void on_password_returnPressed();

	void finished(QNetworkReply*);

private:
	QNetworkAccessManager* manager_;
};

#endif // ACTIVATEWIDGET_H
