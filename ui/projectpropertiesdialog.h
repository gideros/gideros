#ifndef PROJECTPROPERTIESDIALOG_H
#define PROJECTPROPERTIESDIALOG_H

#include <QDialog>
#include <QLabel>

struct ProjectProperties;

namespace Ui {
    class ProjectPropertiesDialog;
}

class ProjectPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
	explicit ProjectPropertiesDialog(QString projectFileName,ProjectProperties* properies, QWidget *parent = 0);
    ~ProjectPropertiesDialog();

private slots:
	void onAccepted();
	void add();
	void remove();
    void addAppIcon();
    void addTvIcon();
    void addSplashHImage();
    void addSplashVImage();
    void chooseColor();
    void showImage(QString fileName, QLabel* label);
    void loadImages();


private:
    Ui::ProjectPropertiesDialog *ui;
	ProjectProperties* properties_;
	QString projectFileName_;
    QString app_icon;
    QString tv_icon;
    QString splash_h_image;
    QString splash_v_image;
    QColor backgroundColor;
};

#endif // PROJECTPROPERTIESDIALOG_H
