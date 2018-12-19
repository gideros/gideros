#ifndef PROPERTYEDITINGTABLE_H
#define PROPERTYEDITINGTABLE_H

#include <QMap>
#include <QList>
#include <QDir>
#include <QTableWidget>
#include <QDomDocument>
#include <QSignalMapper>

class PropertyEditingTable : public QTableWidget
{
    Q_OBJECT

public:

    explicit PropertyEditingTable(QDir projectDir, QWidget *parent = 0);
    ~PropertyEditingTable();
    void fill(QDomElement xml,QMap<QString,QString> values);
    QMap<QString,QString> extract();

private slots:
	void onBrowse(int);

private:
	struct PropDesc {
		QString name;
		QString value;
		QString type;
	};
    QSignalMapper *mapper;
	QList<PropDesc> props;
    QDir projectDir_;
};

#endif // EXPORTPROJECTDIALOG_H
