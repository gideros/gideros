#ifndef OUTLINEWIDGET_H
#define OUTLINEWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QStandardItemModel>
#include <QTime>
#include <QThread>
#include <QLabel>
#include "textedit.h"

struct OutLineItem {
    OutLineItem() : name(""),type(0),line(0) { }
    OutLineItem(QString n,int t,int l) : name(n), type(t), line(l) {}
    QString name;
    int type;
    int line;
};
typedef QList<OutLineItem> OutLineItemList;
Q_DECLARE_METATYPE(OutLineItem)

class OutlineWorkerThread : public QThread
{
    Q_OBJECT
    void run();
    QByteArray btext;
    QString filename;
public:
    OutlineWorkerThread(QObject *parent = Q_NULLPTR,QString name=QString(),QByteArray file=QByteArray()) { btext=file; filename=name; }
    virtual ~OutlineWorkerThread() { }
signals:
    void updateOutline(QList<OutLineItem> s);
    void reportError(const QString error);
};

class OutlineWidgetItem : public QWidget
{
    Q_OBJECT

public:
    OutlineWidgetItem(QWidget *parent=0);
    ~OutlineWidgetItem();
    OutLineItem value_;
    void setValue(OutLineItem &item);
    OutLineItem getValue() { return value_; }
private:
    QLabel *icon;
    QLabel *label;
};

class OutlineWidget : public QWidget
{
	Q_OBJECT

public:
	OutlineWidget(QWidget *parent=0);
	~OutlineWidget();
	void setDocument(TextEdit *doc);
private:
    TextEdit *doc_;
    QTime refresh_;
    QListWidget *list_;
    bool working_;
    bool needParse_;
    void parse();
    void sort();
    QAction *actType_;
    QAction *actSort_;
    QAction *actGlb_;
    QAction *actLoc_;
    QAction *actTbl_;
private slots:
    void checkParse();
    void onItemChanged(QListWidgetItem *,QListWidgetItem *);
    void updateOutline(QList<OutLineItem> s);
    void reportError(const QString error);
};

#endif // OUTPUTWIDGET_H
