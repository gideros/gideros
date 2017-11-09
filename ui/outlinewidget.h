#ifndef OUTLINEWIDGET_H
#define OUTLINEWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QStandardItemModel>
#include <QTime>
#include "textedit.h"

class OutlineWidget : public QListWidget
{
	Q_OBJECT

public:
	OutlineWidget(QWidget *parent=0);
	~OutlineWidget();
    void addEntry(QString name,int type,int line);
	void setDocument(TextEdit *doc);
private:
    TextEdit *doc_;
    QTime refresh_;
    void parse();
private slots:
    void checkParse();
    void onItemChanged(QListWidgetItem *,QListWidgetItem *);
};

#endif // OUTPUTWIDGET_H
