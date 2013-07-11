#ifndef DEACTIVATEWIDGET_H
#define DEACTIVATEWIDGET_H

#include <QWidget>

namespace Ui {
    class DeactivateWidget;
}

class QNetworkAccessManager;
class QNetworkReply;

class DeactivateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DeactivateWidget(QWidget *parent = 0);
    ~DeactivateWidget();

signals:
	void updateUI();

private slots:
	void on_updateLicense_clicked();
	void on_deactivate_clicked();

	void updateLicenseFinished(QNetworkReply* reply);
	void deactivateFinished(QNetworkReply* reply);

private:
    Ui::DeactivateWidget *ui;

private:
	void disableUI();
	void enableUI();

	QNetworkAccessManager* updateLicenseManager_;
	QNetworkAccessManager* deactivateManager_;

	void askUser(const QString& message);
};

#endif // DEACTIVATEWIDGET_H
