#ifndef FILEASSOCIATIONEDITDIALOG_H
#define FILEASSOCIATIONEDITDIALOG_H

#include <QDialog>

namespace Ui {
    class FileAssociationEditDialog;
}

class FileAssociationEditDialog : public QDialog
{
    Q_OBJECT

public:
	explicit FileAssociationEditDialog(	const QString& extension,
										const QString& applicationPath,
										const QString& arguments,
									   QWidget *parent = 0);
    ~FileAssociationEditDialog();

	QString extension() const;
	QString applicationPath() const;
	QString arguments() const;

private slots:
	void on_choose_clicked();

private:
    Ui::FileAssociationEditDialog *ui;
};

#endif // FILEASSOCIATIONEDITDIALOG_H
