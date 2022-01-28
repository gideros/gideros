#ifndef OUTLINEWIDGET_H
#define OUTLINEWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QStandardItemModel>
#include <QTime>
#include <QThread>
#include <QLabel>
#include <QStyledItemDelegate>
#include "textedit.h"
#include "lua.hpp"
#ifdef LUA_IS_LUAU
#include "Luau/Frontend.h"
#endif

struct OutLineItem {
    OutLineItem() : name(""),type(0),line(0) { }
    OutLineItem(QString n,int t,int l) : name(n), type(t), line(l) {}
    QString name;
    int type;
    int line;
};
typedef QList<OutLineItem> OutLineItemList;
Q_DECLARE_METATYPE(OutLineItem)
struct OutlineLinterItem {
    enum LinterType {
        Note=0,
        Warning=1,
        Error=2,
        TypeError=3,
    };
    OutlineLinterItem(QString m,LinterType t,QString f, int l) : message(m), type(t), file(f), line(l) {}
    QString message;
    LinterType type;
    QString file;
    int line;
};
Q_DECLARE_METATYPE(OutlineLinterItem)

class OutlineWorkerThread : public QThread
{
    Q_OBJECT
    void run();
    QByteArray btext;
    QString filename;
public:
    static bool typeCheck;
    OutlineWorkerThread(QObject *parent = Q_NULLPTR,QString name=QString(),QByteArray file=QByteArray()) : QThread(parent) { btext=file; filename=name; }
    virtual ~OutlineWorkerThread() { }
signals:
    void updateOutline(QList<OutLineItem> s);
    void reportError(const QString error,QList<OutlineLinterItem>,QSet<QString>);
};

class OutlineWidgetItem : public QStyledItemDelegate
{
    Q_OBJECT

public:
    OutlineWidgetItem(QObject *parent=0);
    ~OutlineWidgetItem();
    OutLineItem value_;
    void setValue(OutLineItem &item);
    OutLineItem getValue() { return value_; }
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
private:
    QWidget *container;
    QLabel *icon;
    QLabel *label;
};

class OutlineWidget : public QWidget
{
	Q_OBJECT

public:
	OutlineWidget(QWidget *parent=0);
	~OutlineWidget();
    void setDocument(TextEdit *doc,bool checkSyntax,bool typeCheck);
    void saveSettings();
    friend class OutlineWorkerThread;
protected:
    TextEdit *doc_;
    QElapsedTimer refresh_;
    QListView *list_;
    QStandardItemModel *model_;
    bool working_;
    bool needParse_;
    bool checkSyntax_;
    bool typeCheck_;
    void parse();
    void sort();
    QAction *actType_;
    QAction *actSort_;
    QAction *actGlb_;
    QAction *actLoc_;
    QAction *actTbl_;
    OutLineItemList currentOutline_;
#ifdef LUA_IS_LUAU
    friend class OutlineFileResolver;
    Luau::Frontend *frontend;
    Luau::FileResolver *fileResolver;
    Luau::NullConfigResolver configResolver;
#endif
private slots:
    void checkParse();
    void onItemClicked(const QModelIndex &);
    void updateOutline(QList<OutLineItem> s);
    void reportError(const QString error,QList<OutlineLinterItem> lint,QSet<QString> autocomplete);
};

#endif // OUTPUTWIDGET_H
