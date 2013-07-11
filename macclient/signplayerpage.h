#ifndef SIGNPLAYERPAGE_H
#define SIGNPLAYERPAGE_H

#include <QWizardPage>

namespace Ui {
    class SignPlayerPage;
}

class SignPlayerPage : public QWizardPage
{
    Q_OBJECT

public:
    explicit SignPlayerPage(QWidget *parent = 0);
    ~SignPlayerPage();

private:
    Ui::SignPlayerPage *ui;

private slots:
	void on_browsePrivateKey_clicked();
	void on_browseProvisioningProfile_clicked();
	void on_browseDeveloperSigningCertificate_clicked();
	void on_browseOutput_clicked();
};

#endif // SIGNPLAYERPAGE_H
