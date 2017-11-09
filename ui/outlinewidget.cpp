#include "outlinewidget.h"
#include <QDebug>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qscilexerxml.h>
#include <Qsci/qscilexer.h>
#include <Qsci/qsciapis.h>
#include <Qsci/qscicommand.h>
#include <Qsci/qscicommandset.h>
#include <QFileInfo>
#include <QLabel>
#include <QHBoxLayout>

#include "iconlibrary.h"
#include "lua.hpp"
extern "C" {
#include "ldo.h"
#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstring.h"
#include "lundump.h"
}

#define TYPING_DELAY 1000

OutlineWidget::OutlineWidget(QWidget *parent)
    : QListWidget(parent)
{
    connect(this, SIGNAL(currentItemChanged(QListWidgetItem *,QListWidgetItem *)),
            this, SLOT  (onItemChanged(QListWidgetItem *,QListWidgetItem *)));
}

OutlineWidget::~OutlineWidget()
{
}

void OutlineWidget::onItemChanged(QListWidgetItem *i,QListWidgetItem *p)
{
    if (!i) return;
    int line=i->data(Qt::UserRole+1).toInt();
    if (doc_)
    {
        doc_->setCursorPosition(line-1,0);
        doc_->setFocusToEdit();
    }
}

#define OT_GLOBAL   0x10
#define OT_FUNCTION 1

void OutlineWidget::addEntry(QString name,int type,int line)
{
    QListWidgetItem *i=new QListWidgetItem();
    i->setData(Qt::UserRole+1,line);
    i->setData(Qt::UserRole+2,type);
    addItem(i);
    QString stype="variable";
    if (type&OT_FUNCTION) stype="function";
    QString iconName="purple dot";
    if (type&OT_GLOBAL) iconName="green dot";
    QWidget *item=new QWidget();
    QHBoxLayout *layout=new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    QLabel *icon=new QLabel();
    icon->setPixmap(IconLibrary::instance().icon(0,iconName).pixmap(QSize(16,16)));
    QLabel *label=new QLabel("<html>"+name+"<font color='gray'>&nbsp;"+stype+"</font></html>");
    layout->addWidget(icon);
    layout->addWidget(label);
    layout->addStretch(1);
    item->setLayout(layout);
    setItemWidget(i,item);
}

#define toproto(L,i) (clvalue(L->top+(i))->l.p)

static int decodeValType(int t)
{
    if (t<0) return 0;
    if (t&0x01000000)
        return OT_FUNCTION;
    return 0;
}

static int decodeRefType(int t)
{
    if (t<0) return 0;
    if (t&0x02000000)
        return OT_GLOBAL;
    return 0;
}

void OutlineWidget::parse() {
    clear();
    if (doc_) {
        QFileInfo fileInfo(doc_->fileName());
        if (!fileInfo.suffix().compare(QString("lua"),Qt::CaseInsensitive))
        {
            QsciScintilla *s=doc_->sciScintilla();
            s->clearAnnotations();
            QString text=s->text();
            lua_State *L = luaL_newstate();
            QByteArray btext=text.toUtf8();
            if (!luaL_loadbuffer(L,btext.constData(),btext.size(),doc_->fileName().toUtf8().constData()))
            {
                Proto *p=toproto(L,-1);
                /*for (int k=0;k<p->sizelocvars;k++)
                {
                  const char *lname=getstr(p->locvars[k].varname);
                  qDebug() << "LOCAL" << lname << p->lineinfo[p->locvars[k].startpc];
                }*/
                int kreg[512];
                for (int k=0;k<512;k++)
                    kreg[k]=-1;

                for (int k=0;k<p->sizecode;k++)
                {
                    int opc=p->code[k];
                    int op=GET_OPCODE(opc);
                    int a=GETARG_A(opc);
                    int b=GETARG_B(opc);
                    int c=GETARG_C(opc);
                    int bx=GETARG_Bx(opc);
                    //qDebug() << "OP" << op << a <<b <<c << bx << p->lineinfo[k];
                    switch (op)
                    {
                        case OP_MOVE: kreg[a]=kreg[b]; break;
                        case OP_LOADK: kreg[a]=bx; break;
                    case OP_LOADBOOL:
                    case OP_LOADNIL:
                    case OP_GETUPVAL:
                    case OP_GETGLOBAL: kreg[a]=bx|0x02000000; break; //GLOBAL
                    case OP_SETGLOBAL:
                    {
                        const char *gname=svalue(p->k+bx);
                        //qDebug() << "GLOBAL" << gname << p->lineinfo[k] << kreg[a];
                        addEntry(gname,OT_GLOBAL|decodeValType(kreg[a]),p->lineinfo[k]);
                    }
                        break;
                    case OP_SETTABLE:
                        if (ISK(b)&&(kreg[a]>=0))
                        {
                            const char *fname=svalue(p->k+INDEXK(b));
                            const char *hname=svalue(p->k+INDEXK(kreg[a]&0xFFFFFF));
                            QString tname=QString(hname)+"."+fname;
                            //qDebug() << "SETTABLE" << tname << p->lineinfo[k] << kreg[a]  << kreg[c];
                            addEntry(tname,decodeRefType(kreg[b])|decodeValType(kreg[c]),p->lineinfo[k]);
                        }
                        break;
                    case OP_SELF:
                        kreg[a+1]=kreg[b];
                    case OP_GETTABLE:
                        kreg[a]=-1;
                        break;
                    case OP_SETLIST:
                        kreg[a]=-1;
                        break;
                    case OP_CLOSURE:
                        kreg[a]=bx|0x01000000; //CLOSURE
                        break;
                    }
                }
            }
            else
            {
                QString error=QString(lua_tostring(L,-1));
                QRegularExpression re("\\[string [^\\]]+\\]:(\\d+):(.*)");
                QRegularExpressionMatch match = re.match(error);
                if (match.hasMatch()) {
                    QString line = match.captured(1);
                    QString err = match.captured(2);
                    int lineNum=line.toInt();
                    int stylenum=QsciScintilla::STYLE_LASTPREDEFINED+10;
                    s->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE,stylenum,QColor("white"));
                    s->SendScintilla(QsciScintillaBase::SCI_STYLESETBACK,stylenum,QColor("darkred"));
                    s->setAnnotationDisplay(QsciScintilla::AnnotationBoxed);
                    s->annotate(lineNum-1,err,stylenum);
                }
                //qDebug() << "LUAP" << error;
            }
            lua_close(L);
        }
    }
}

void OutlineWidget::checkParse() {
    if (refresh_.elapsed()>(TYPING_DELAY-100))
        parse();
}

void OutlineWidget::setDocument(TextEdit *doc)
{
    bool docChanged=(doc_!=doc);
    doc_=doc;
    if (docChanged)
        parse();
    else
    {
        refresh_.start();
        QTimer::singleShot(TYPING_DELAY, this, SLOT(checkParse()));
    }
}
