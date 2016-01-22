#ifndef PROPERTYEDITINGTABLE_H
#define PROPERTYEDITINGTABLE_H

#include <QMap>
#include <QList>
#include <QTableWidget>
#include <QDomDocument>
#include <QSignalMapper>

class PropertyEditingTable : public QTableWidget
{
    Q_OBJECT

public:

    explicit PropertyEditingTable(QWidget *parent = 0);
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
};

#endif // EXPORTPROJECTDIALOG_H
