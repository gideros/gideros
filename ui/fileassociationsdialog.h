#ifndef FILEASSOCIATIONSDIALOG_H
#define FILEASSOCIATIONSDIALOG_H

#include <QDialog>

namespace Ui {
    class FileAssociationsDialog;
}

class QTreeWidgetItem;

class FileAssociationsDialog : public QDialog
{
    Q_OBJECT

public:
	explicit FileAssociationsDialog(const QList<QStringList>& fileAssociations, QWidget *parent = 0);
    ~FileAssociationsDialog();

	QList<QStringList> fileAssociations() const;

private slots:
	void add();
	void remove();
	void edit();
	void onItemDoubleClicked(QTreeWidgetItem* item, int column);

private:
	void edit(QTreeWidgetItem* item);

private:
    Ui::FileAssociationsDialog *ui;
};

#endif // FILEASSOCIATIONSDIALOG_H
