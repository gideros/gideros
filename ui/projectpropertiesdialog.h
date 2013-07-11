#ifndef PROJECTPROPERTIESDIALOG_H
#define PROJECTPROPERTIESDIALOG_H

#include <QDialog>

struct ProjectProperties;

namespace Ui {
    class ProjectPropertiesDialog;
}

class ProjectPropertiesDialog : public QDialog
{
    Q_OBJECT

public:
	explicit ProjectPropertiesDialog(ProjectProperties* properies, QWidget *parent = 0);
    ~ProjectPropertiesDialog();

private slots:
	void onAccepted();
	void add();
	void remove();


private:
    Ui::ProjectPropertiesDialog *ui;
	ProjectProperties* properties_;
};

#endif // PROJECTPROPERTIESDIALOG_H
