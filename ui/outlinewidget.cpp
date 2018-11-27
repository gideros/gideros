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
#include <QVBoxLayout>
#include <QToolBar>
#include <QList>
#include <QScrollBar>
#include <QAction>
#include <QPainter>
#include <QSettings>

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
#define OT_GLOBAL   0x10
#define OT_LOCAL    0x20
#define OT_FUNCTION 1

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

static QString getLuaString(TValue *v,bool field=false)
{
    if (ttisstring(v))
        return field?QString(".")+QString(svalue(v)):QString(svalue(v));
    else if (ttisnumber(v))
        return QString("[%1]").arg(QString::number(nvalue(v)));
    else
        return field?".?":"?";
}

void OutlineWorkerThread::run()
{
  QList<OutLineItem> outline;

  lua_State *L = luaL_newstate();
  if (!luaL_loadbuffer(L,btext.constData(),btext.size(),filename.toUtf8().constData()))
  {
      Proto *p=toproto(L,-1);
      for (int k=0;k<p->sizelocvars;k++)
      {
        const char *lname=getstr(p->locvars[k].varname);
        //qDebug() << "LOCAL" << lname << p->locvars[k].line << p->locvars[k].isfunction << p->locvars[k].level;
        if (!p->locvars[k].level)
          outline << OutLineItem(lname,OT_LOCAL|(p->locvars[k].isfunction?OT_FUNCTION:0),p->locvars[k].line);
      }
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
          //Check if a is a local in this context
          int lac=a;
          int la=-1;
          for (int kl=0;kl<p->sizelocvars;kl++)
          {
              if ((p->locvars[kl].startpc<=k)&&(p->locvars[kl].endpc>=k))
              {
                  if (lac==0) la=kl;
                  lac--;
              }
          }
          int line=p->lineinfo[k];

          //qDebug() << "OP" << op << a <<b <<c << bx << la << line;
          switch (op)
          {
              case OP_MOVE: kreg[a]=kreg[b]; break;
              case OP_LOADK: kreg[a]=bx; break;
          case OP_LOADBOOL:
          case OP_LOADNIL:
          case OP_GETUPVAL:
          case OP_NEWTABLE:
          case OP_ADD:
          case OP_SUB:
          case OP_MUL:
          case OP_DIV:
          case OP_MOD:
          case OP_POW:
          case OP_BAND:
          case OP_BXOR:
          case OP_BLSHFT:
          case OP_BRSHFT:
          case OP_BNOT:
          case OP_INTDIV:
          case OP_UNM:
          case OP_NOT:
          case OP_LEN:
          case OP_CONCAT:
              kreg[a]=-1;
              break;
          case OP_FORPREP:
              kreg[a+3]=-1; //Ensure loop var is invalidated
              break;
          case OP_GETGLOBAL: kreg[a]=bx|0x02000000; break; //GLOBAL
          case OP_SETGLOBAL:
          {
              QString gname=getLuaString(p->k+bx);
              //qDebug() << "GLOBAL" << gname << p->lineinfo[k] << kreg[a];
              outline << OutLineItem(gname,OT_GLOBAL|decodeValType(kreg[a]),p->lineinfo[k]);
          }
              break;
          case OP_SETTABLE:
              if (kreg[a]>=0)
              {
                  int kval=-1;
                  if (!ISK(b))
                      kval=kreg[b];
                  else
                      kval=INDEXK(b);
                  if (kval>=0)
                  {
                    QString fname=getLuaString(p->k+(kval&0xFFFFFF),true);
                    QString hname=getLuaString(p->k+(kreg[a]&0xFFFFFF));
                    QString tname=hname+fname;
                    //qDebug() << "SETTABLE" << tname << p->lineinfo[k] << kreg[a]  << kreg[c];
                    outline << OutLineItem(tname,decodeRefType(kval)|decodeValType(kreg[c]),p->lineinfo[k]);
                  }
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
          case OP_CLOSE: break; //No effect
          case OP_CLOSURE:
              kreg[a]=bx|0x01000000; //CLOSURE
              break;
          case OP_VARARG: break; //Not relevant in toplevel chhunk
          case OP_MAX:
          case OP_MIN:
          case OP_DEG:
          case OP_RAD:
          case OP_ADD_EQ:
          case OP_SUB_EQ:
          case OP_MUL_EQ:
          case OP_DIV_EQ:
          case OP_MOD_EQ:
          case OP_POW_EQ:
              kreg[a]=-1;
              break;
          }
      }
      emit reportError("");
  }
  else
  {
      QString error=QString(lua_tostring(L,-1));
      emit reportError(error);
      //qDebug() << "LUAP" << error;
  }
  lua_close(L);

  emit updateOutline(outline);
}

OutlineWidgetItem::OutlineWidgetItem(QObject *parent) : QStyledItemDelegate(parent){
	container=new QWidget();
    icon=new QLabel();
    label=new QLabel();

    QHBoxLayout *layout=new QHBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(icon);
    layout->addWidget(label);
    layout->addStretch(1);
    container->setLayout(layout);
}

void OutlineWidgetItem::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (option.state & QStyle::State_Selected)
        painter->fillRect(option.rect, option.palette.highlight());

    QStandardItem *li=((QStandardItemModel *)index.model())->item(index.row());
    int type=li->data(Qt::UserRole+2).toInt();
    QString name=li->text();
    QString stype="variable";
    if (type&OT_FUNCTION) stype="function";
    QString iconName="purple dot";
    if (type&OT_GLOBAL) iconName="green dot";
    if (type&OT_LOCAL) iconName="red dot";
    icon->setPixmap(IconLibrary::instance().icon(0,iconName).pixmap(QSize(16,16)));
    label->setText("<html>"+name+"<font color='gray'>&nbsp;"+stype+"</font></html>");

    painter->save();
    container->resize( option.rect.size() );
    painter->translate(option.rect.topLeft());
    container->render(painter, QPoint(), QRegion(), QWidget::DrawChildren );
    painter->restore();
}

void OutlineWidgetItem::setValue(OutLineItem &item)
{
    value_=item;
    int type=item.type;
    QString stype="variable";
    if (type&OT_FUNCTION) stype="function";
    QString iconName="purple dot";
    if (type&OT_GLOBAL) iconName="green dot";
    if (type&OT_LOCAL) iconName="red dot";
    icon->setPixmap(IconLibrary::instance().icon(0,iconName).pixmap(QSize(16,16)));
    label->setText("<html>"+item.name+"<font color='gray'>&nbsp;"+stype+"</font></html>");
}

OutlineWidgetItem::~OutlineWidgetItem()
{
}

OutlineWidget::OutlineWidget(QWidget *parent)
    : QWidget(parent)
{
    qRegisterMetaType<OutLineItemList>("OutLineItemList");
    list_=new QListView();
    connect(list_, SIGNAL(clicked(const QModelIndex &)),
            this, SLOT  (onItemClicked(const QModelIndex &)));
    working_=false;
    QVBoxLayout *layout=new QVBoxLayout();
    layout->setContentsMargins(0,0,0,0);
    QToolBar *toolbar=new QToolBar();
    QSettings settings;
    int s=settings.value("outline_state",31).toInt();
    actType_=toolbar->addAction(IconLibrary::instance().icon(0,"sort cat"),"Group by type",this, &OutlineWidget::sort);
    actType_->setCheckable(true); actType_->setChecked(s&1);
    actSort_=toolbar->addAction(IconLibrary::instance().icon(0,"sort alpha"),"Sort alphabetically",this, &OutlineWidget::sort);
    actSort_->setCheckable(true); actSort_->setChecked(s&2);
    actGlb_=toolbar->addAction(IconLibrary::instance().icon(0,"green dot"),"Show globals",this, &OutlineWidget::sort);
    actGlb_->setCheckable(true); actGlb_->setChecked(s&4);
    actLoc_=toolbar->addAction(IconLibrary::instance().icon(0,"red dot"),"Show locals",this, &OutlineWidget::sort);
    actLoc_->setCheckable(true); actLoc_->setChecked(s&8);
    actTbl_=toolbar->addAction(IconLibrary::instance().icon(0,"purple dot"),"Show table fields",this, &OutlineWidget::sort);
    actTbl_->setCheckable(true); actTbl_->setChecked(s&16);
    layout->addWidget(toolbar);
    layout->addWidget(list_);
    layout->setStretchFactor(list_,1);
    setLayout(layout);
    model_=new QStandardItemModel();
    list_->setModel(model_);
    list_->setItemDelegate(new OutlineWidgetItem());
}

OutlineWidget::~OutlineWidget()
{
}

void OutlineWidget::saveSettings()
{
    QSettings settings;
    int s=0;
    if (actType_->isChecked()) s|=1;
    if (actSort_->isChecked()) s|=2;
    if (actGlb_->isChecked()) s|=4;
    if (actLoc_->isChecked()) s|=8;
    if (actTbl_->isChecked()) s|=16;
    settings.setValue("outline_state",s);
}

void OutlineWidget::onItemClicked(const QModelIndex &idx)
{
    int line=model_->itemFromIndex(idx)->data(Qt::UserRole+1).toInt();
    if (doc_)
    {
        int ml=line-1;
        int bl=std::max(ml-5,0);
        int hl=std::min(ml+5,doc_->sciScintilla()->lines()-1);
        doc_->setCursorPosition(bl,0);
        doc_->setCursorPosition(hl,0);
        doc_->setCursorPosition(ml,0);
        doc_->setFocusToEdit();
    }
}

void OutlineWidget::sort()
{
    updateOutline(currentOutline_);
}

void OutlineWidget::updateOutline(QList<OutLineItem> s)
{
    currentOutline_=s;
    bool sAlpha=actSort_->isChecked();
    bool sGroup=actType_->isChecked();
    if (s.size()>1)
    std::sort(s.begin(),s.end(),[sGroup,sAlpha](OutLineItem &a,OutLineItem &b) {
        if (sGroup&&((a.type&0x0F)>(b.type&0x0F))) return true;
        if (sGroup&&((a.type&0x0F)<(b.type&0x0F))) return false;
        if (sAlpha) return a.name.toLower()<b.name.toLower();
        return a.line<b.line;
    });
    bool vTable=actTbl_->isChecked();
    bool vGlobal=actGlb_->isChecked();
    bool vLocal=actLoc_->isChecked();
    QScrollBar *vb = list_->verticalScrollBar();
    int oldValue = vb->value();

    model_->clear();
    foreach( OutLineItem item, s )
    {
        int t=item.type&0xF0;
        bool hidden=(!(((t==OT_GLOBAL)&&vGlobal)||((t==OT_LOCAL)&&vLocal)||((t==0)&&vTable)));
        if (hidden) continue;
        QStandardItem *li=new QStandardItem();
        li->setData(item.line,Qt::UserRole+1);
        li->setData(item.type,Qt::UserRole+2);
        li->setText(item.name);
        model_->appendRow(li);
    }
    vb->setValue(oldValue);
    working_=false;
    if (needParse_)
        parse();
}

void OutlineWidget::reportError(const QString error)
{
    if (!doc_) return;
    QsciScintilla *s=doc_->sciScintilla();
    s->clearAnnotations();
    if (error.isEmpty()||(!checkSyntax_)) return;
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
        if (s->SendScintilla(QsciScintillaBase::SCI_AUTOCACTIVE))
        		return;
        if (lineNum>=s->lines()) //Last line has error
        {
            int cline,cindex;
            s->getCursorPosition(&cline,&cindex);
            if ((lineNum-cline)<4) //Faulty line is around editing point, so scroll
                s->SendScintilla(QsciScintillaBase::SCI_SCROLLTOEND,0,0L);
        }
    }

}

void OutlineWidget::parse() {
    bool noContent=true;
    needParse_=working_;
    if (working_)
        return;
    if (doc_) {
        QFileInfo fileInfo(doc_->fileName());
        if (!fileInfo.suffix().compare(QString("lua"),Qt::CaseInsensitive))
        {
            noContent=false;
            QsciScintilla *s=doc_->sciScintilla();
            QString text=s->text();
            QByteArray btext=text.toUtf8();

            OutlineWorkerThread *workerThread = new OutlineWorkerThread(this,doc_->fileName(),btext);
            connect(workerThread, &OutlineWorkerThread::updateOutline, this, &OutlineWidget::updateOutline);
            connect(workerThread, &OutlineWorkerThread::reportError, this, &OutlineWidget::reportError);
            connect(workerThread, &OutlineWorkerThread::finished, workerThread, &QObject::deleteLater);
            workerThread->start();
            working_=true;
        }
    }
    if (noContent)
        updateOutline(OutLineItemList());
}

void OutlineWidget::checkParse() {
    if (refresh_.elapsed()>(TYPING_DELAY-100))
        parse();
}

void OutlineWidget::setDocument(TextEdit *doc,bool checkSyntax)
{
    bool docChanged=(doc_!=doc);
    doc_=doc;
    checkSyntax_=checkSyntax;
    if (docChanged)
        parse();
    else
    {
        refresh_.start();
        QTimer::singleShot(TYPING_DELAY, this, SLOT(checkParse()));
    }
}
