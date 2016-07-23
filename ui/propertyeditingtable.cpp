#include "propertyeditingtable.h"

#include <QDir>
#include <QFile>
#include <QCheckBox>
#include <QPushButton>
#include <QHeaderView>
#include <QFileDialog>

PropertyEditingTable::PropertyEditingTable(QWidget *parent) :
		QTableWidget(parent) {
	QStringList labels;
	labels << "Property" << "Value" << "";
	setColumnCount(3);
	setHorizontalHeaderLabels(labels);
    setSelectionBehavior(QAbstractItemView::SelectRows);
     setSelectionMode(QAbstractItemView::SingleSelection);

	mapper = new QSignalMapper(this);

connect(mapper, SIGNAL(mapped(int)),
		this, SLOT(onBrowse(int)));
}

void PropertyEditingTable::fill(QDomElement xml,
	QMap<QString, QString> values) {
	clearContents();
QDomNodeList exprops = xml.elementsByTagName("property");
setRowCount(exprops.count());
for (int k = 0; k < exprops.count(); k++) {
	QDomElement exprop = exprops.at(k).toElement();

	QTableWidgetItem *item;

	item = new QTableWidgetItem(exprop.attribute("title"));
	QString propDesc = exprop.attribute("description");
    item->setFlags((item->flags()|Qt::ItemIsEnabled)&(~Qt::ItemIsEditable));
	if (!propDesc.isEmpty())
		item->setToolTip(propDesc);
	setItem(k, 0, item);

	QString val = exprop.attribute("default");
	QString propKey = exprop.attribute("name");
	QString propType = exprop.attribute("type");

	if (values.contains(propKey))
		val = values[propKey];

	PropDesc pdesc;
	pdesc.name=propKey;
	pdesc.type=propType;
	pdesc.value=val;
	props.append(pdesc);

	if (propType == "boolean") {
		QCheckBox *cb = new QCheckBox("");
		cb->setChecked(val.toInt() != 0);
		if (!propDesc.isEmpty())
			cb->setToolTip(propDesc);
		setCellWidget(k, 1, cb);
	} else {
		item = new QTableWidgetItem(val);
		if (!propDesc.isEmpty())
			item->setToolTip(propDesc);
		setItem(k, 1, item);
	}

	if ((propType == "dir")||(propType == "file")) {
		QPushButton *bt = new QPushButton("Browse");
		mapper->setMapping(bt, k);
		connect(bt, SIGNAL(clicked()), mapper, SLOT(map()));
		setCellWidget(k, 2, bt);
	}

  }
  horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);

}

QMap<QString, QString> PropertyEditingTable::extract() {
	QMap < QString, QString > r;
	for (int row=0;row<rowCount();row++)
	{
		PropDesc pdesc=props.at(row);
		QString val="0";
		if (pdesc.type == "boolean") {
			if (((QCheckBox *)cellWidget(row,1))->isChecked())
				val="1";
		} else
			val=item(row,1)->text();
		r[pdesc.name]=val;
	}
	return r;
}

PropertyEditingTable::~PropertyEditingTable() {
}

void PropertyEditingTable::onBrowse(int row) {
 QString type=props.at(row).type;
 if (type=="dir")
 {
	 QString dir = QFileDialog::getExistingDirectory(this,"",item(row,1)->text(),
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
	 if (!dir.isEmpty())
		 item(row,1)->setText(dir);
 }
 else if (type=="file")
 {
	 QString file = QFileDialog::getOpenFileName(this,"",item(row,1)->text(),"");
	 if (!file.isEmpty())
		 item(row,1)->setText(file);
 }
}

