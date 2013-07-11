#include "fileassociationsdialog.h"
#include "ui_fileassociationsdialog.h"
#include "fileassociationeditdialog.h"

FileAssociationsDialog::FileAssociationsDialog(const QList<QStringList>& fileAssociations, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FileAssociationsDialog)
{
    ui->setupUi(this);

	for (int i = 0; i < fileAssociations.size(); ++i)
	{
		QStringList data = fileAssociations[i];

		QString extension = data[0];
		QString applicationPath = data[1];
		QString arguments = data[2];

		QTreeWidgetItem* item = new QTreeWidgetItem;

		item->setText(0, extension);
		item->setText(1, applicationPath + " " + arguments);
		item->setData(0, Qt::UserRole, data);

		ui->extensions->addTopLevelItem(item);
	}

	connect(ui->add, SIGNAL(clicked()), this, SLOT(add()));
	connect(ui->remove, SIGNAL(clicked()), this, SLOT(remove()));
	connect(ui->edit, SIGNAL(clicked()), this, SLOT(edit()));

	connect(ui->extensions, SIGNAL(itemDoubleClicked  (QTreeWidgetItem*, int)),
			this,			SLOT  (onItemDoubleClicked(QTreeWidgetItem*, int)));
}

FileAssociationsDialog::~FileAssociationsDialog()
{
    delete ui;
}

void FileAssociationsDialog::add()
{
/*
		QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << "<Extension>" << "<Application Path>");
	ui->extensions->addTopLevelItem(item);
	item->setSelected(true);
		*/

	FileAssociationEditDialog dialog(QString(), QString(), QString(), this);
	if (dialog.exec() == QDialog::Accepted)
	{
		QTreeWidgetItem* item = new QTreeWidgetItem;

		item->setText(0, dialog.extension());
		item->setText(1, dialog.applicationPath() + " " + dialog.arguments());

		QStringList data;
		data << dialog.extension();
		data << dialog.applicationPath();
		data << dialog.arguments();

		item->setData(0, Qt::UserRole, data);
		ui->extensions->addTopLevelItem(item);
	}
}

void FileAssociationsDialog::remove()
{
	QList<QTreeWidgetItem*> items = ui->extensions->selectedItems();

	for (int i = 0; i < items.size(); ++i)
		delete items[i];
}

void FileAssociationsDialog::edit()
{
	// http://msdn.microsoft.com/en-us/library/ms648069%28VS.85%29.aspx
	// http://lists.trolltech.com/qt-interest/2007-07/thread00170-0.html
	// http://msdn.microsoft.com/en-us/library/bb762179%28v=vs.85%29.aspx

	QList<QTreeWidgetItem*> items = ui->extensions->selectedItems();

	if (items.size() == 1)
	{
		QTreeWidgetItem* item = items[0];

		edit(item);
	}
}

void FileAssociationsDialog::edit(QTreeWidgetItem* item)
{
	QString extension, applicationPath, arguments;

	if (item->data(0, Qt::UserRole).isNull() == false)
	{
		QStringList data = item->data(0, Qt::UserRole).toStringList();

		extension = data[0];
		applicationPath = data[1];
		arguments = data[2];
	}

	FileAssociationEditDialog dialog(extension, applicationPath, arguments, this);
	if (dialog.exec() == QDialog::Accepted)
	{
		item->setText(0, dialog.extension());
		item->setText(1, dialog.applicationPath() + " " + dialog.arguments());

		QStringList data;
		data << dialog.extension();
		data << dialog.applicationPath();
		data << dialog.arguments();

		item->setData(0, Qt::UserRole, data);
	}
}


QList<QStringList> FileAssociationsDialog::fileAssociations() const
{
	QList<QStringList> result;

	for (int i = 0; i < ui->extensions->topLevelItemCount(); ++i)
	{
		QTreeWidgetItem* item = ui->extensions->topLevelItem(i);

		if (item->data(0, Qt::UserRole).isNull() == false)
			result.push_back(item->data(0, Qt::UserRole).toStringList());
	}

	return result;
}

void FileAssociationsDialog::onItemDoubleClicked(QTreeWidgetItem* item, int column)
{
	edit(item);
}
