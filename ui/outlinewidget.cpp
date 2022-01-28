#include "outlinewidget.h"
#include <QDebug>
#ifdef SCINTILLAEDIT_H
#else
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qscilexerxml.h>
#include <Qsci/qscilexer.h>
#include <Qsci/qsciapis.h>
#include <Qsci/qscicommand.h>
#include <Qsci/qscicommandset.h>
#endif
#include <QTimer>
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
#ifndef LUA_IS_LUAU
extern "C" {
#include "ldo.h"
#include "lfunc.h"
#include "lmem.h"
#include "lobject.h"
#include "lopcodes.h"
#include "lstring.h"
#include "lundump.h"
}
#else
#include "Luau/Compiler.h"
#include "Luau/Ast.h"
#include "Luau/Frontend.h"
#include "Luau/BuiltinDefinitions.h"
#endif

#define TYPING_DELAY 1000
#define OT_GLOBAL   0x10
#define OT_LOCAL    0x20
#define OT_FUNCTION 1

#ifndef LUA_IS_LUAU
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
#else
class OutlineFileResolver : public Luau::FileResolver
{
public:
    OutlineFileResolver(OutlineWidget *ow) : _ow(ow) { };
    OutlineWidget *_ow;
    std::optional<Luau::SourceCode> readSource(const Luau::ModuleName& name) override
    {
        TextEdit *doc=_ow->doc_;
        if (doc&&(QString(name.c_str())==doc->fileName()))
        {
#ifdef SCINTILLAEDIT_H
            ScintillaEdit *s=doc->sciScintilla();
            QString text;
            int size=s->textLength();
            if (size>0)
                text=s->getText(size);
#else
            QsciScintilla *s=doc->sciScintilla();
            QString text=s->text();
#endif
            return Luau::SourceCode{text.toStdString(), Luau::SourceCode::Module};
        }
        std::optional<std::string> source = std::nullopt;//readFile(name);
        if (!source)
            return std::nullopt;

        return Luau::SourceCode{*source, Luau::SourceCode::Module};
    }

    std::optional<Luau::ModuleInfo> resolveModule(const Luau::ModuleInfo* context, Luau::AstExpr* node) override
    {
        Q_UNUSED(context);
        if (Luau::AstExprConstantString* expr = node->as<Luau::AstExprConstantString>())
        {
            Luau::ModuleName name = std::string(expr->value.data, expr->value.size) + ".lua";
            return {{name}};
        }

        return std::nullopt;
    }
};


class OutlineVisitor : public Luau::AstVisitor
{
    virtual bool visit(class Luau::AstExprFunction* node)
    {
        Q_UNUSED(node);
        return false;
    }
    virtual bool visit(class Luau::AstExprLocal* node)
    {
        Q_UNUSED(node);
        //*ol << OutLineItem(node->local->name.value,OT_LOCAL|0,node->location.begin.line+1);
        return false;
    }
    virtual bool visit(class Luau::AstExprGlobal* node)
    {
        Q_UNUSED(node);
        return false;
    }
    virtual bool visit(class Luau::AstStatLocal* node)
    {
        for (Luau::AstLocal* var : node->vars)
        {
            *ol << OutLineItem(var->name.value,OT_LOCAL|0,node->location.begin.line+1);
        }
        return false;
    }
    std::string toExprName(Luau::AstExpr *expr, bool &glb) {
        if (expr->is<Luau::AstExprGlobal>()) {
            glb=true;
            return expr->as<Luau::AstExprGlobal>()->name.value;
        }
        else if (expr->is<Luau::AstExprLocal>())
            return expr->as<Luau::AstExprLocal>()->local->name.value;
        else if (expr->is<Luau::AstExprIndexName>())
            return toExprName(expr->as<Luau::AstExprIndexName>()->expr,glb) + expr->as<Luau::AstExprIndexName>()->op + expr->as<Luau::AstExprIndexName>()->index.value;
        return "Undef";
    }
    virtual bool visit(class Luau::AstStatAssign* node)
    {
        for (Luau::AstExpr* var : node->vars)
        {
            bool glb=false;
            *ol << OutLineItem(toExprName(var,glb).c_str(),(glb?OT_GLOBAL:OT_LOCAL)|0,node->location.begin.line+1);
        }
        return false;
    }
    virtual bool visit(class Luau::AstStatFunction* node)
    {
        bool glb=false;
        *ol << OutLineItem(toExprName(node->name,glb).c_str(),(glb?OT_GLOBAL:OT_LOCAL)|OT_FUNCTION,node->location.begin.line+1);
        return false;
    }
    virtual bool visit(class Luau::AstStatLocalFunction* node)
    {
        *ol << OutLineItem(node->name->name.value,OT_LOCAL|OT_FUNCTION,node->location.begin.line+1);
         return false;
    }
    virtual bool visit(class Luau::AstStatDeclareFunction* node)
    {
        Q_UNUSED(node);
        return false;
    }
    virtual bool visit(class Luau::AstStatDeclareGlobal* node)
    {
        Q_UNUSED(node);
        return false;
    }
    virtual bool visit(class Luau::AstStatDeclareClass* node)
    {
        Q_UNUSED(node);
        return false;
    }

public:
    QList<OutLineItem> *ol;
    OutlineVisitor(QList<OutLineItem> *outline) {
        ol=outline;
    }
};

class AutocompleteVisitor : public Luau::AstVisitor
{
    QString toExprName(Luau::AstExpr *expr) {
        if (expr->is<Luau::AstExprGlobal>()) {
            return expr->as<Luau::AstExprGlobal>()->name.value;
        }
        else if (expr->is<Luau::AstExprLocal>())
            return expr->as<Luau::AstExprLocal>()->local->name.value;
        else if (expr->is<Luau::AstExprIndexName>())
            return toExprName(expr->as<Luau::AstExprIndexName>()->expr) + QString(expr->as<Luau::AstExprIndexName>()->op) + expr->as<Luau::AstExprIndexName>()->index.value;
        return "_";
    }
    virtual bool visit(class Luau::AstExprLocal* node)
    {
        autocomplete << toExprName(node);
        return true;
    }
    virtual bool visit(class Luau::AstExprGlobal* node)
    {
        autocomplete << toExprName(node);
        return true;
    }
    virtual bool visit(class Luau::AstExprIndexName* node)
    {
        autocomplete << toExprName(node);
        return true;
    }
    virtual bool visit(class Luau::AstStatLocal* node)
    {
        for (Luau::AstLocal* var : node->vars)
        {
            autocomplete << var->name.value;
        }
        return true;
    }
    virtual bool visit(class Luau::AstStatAssign* node)
    {
        for (Luau::AstExpr* var : node->vars)
            autocomplete << toExprName(var);
        return true;
    }
    virtual bool visit(class Luau::AstStatFunction* node)
    {
        autocomplete << toExprName(node->name);
        return true;
    }
    virtual bool visit(class Luau::AstStatLocalFunction* node)
    {
        autocomplete << node->name->name.value;
         return true;
    }
    virtual bool visit(class Luau::AstStatDeclareFunction* node)
    {
        Q_UNUSED(node);
        return true;
    }
    virtual bool visit(class Luau::AstStatDeclareGlobal* node)
    {
        Q_UNUSED(node);
        return true;
    }
    virtual bool visit(class Luau::AstStatDeclareClass* node)
    {
        Q_UNUSED(node);
        return true;
    }

public:
    QSet<QString> autocomplete;
    AutocompleteVisitor() {
    }
};


#endif

bool OutlineWorkerThread::typeCheck=false;

void OutlineWorkerThread::run()
{
  QList<OutLineItem> outline;

  lua_State *L = luaL_newstate();
#ifndef LUA_IS_LUAU
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
#else
      Luau::CompileOptions opts;
      Luau::ParseOptions popts;
      Luau::ParseResult result;
      Luau::Allocator allocator;
      Luau::AstNameTable names(allocator);
      result = Luau::Parser::parse(btext.constData(),btext.size(), names, allocator, popts);

      Luau::LintResult lr = ((OutlineWidget *)this->parent())->frontend->lint(filename.toStdString().c_str());
      QList<OutlineLinterItem> li;
      for (const Luau::LintWarning &e: lr.warnings)
          li.append(OutlineLinterItem(QString(e.text.c_str()),OutlineLinterItem::LinterType::Warning,filename,e.location.begin.line));
      for (const Luau::LintWarning &e: lr.errors)
          li.append(OutlineLinterItem(QString(e.text.c_str()),OutlineLinterItem::LinterType::Error,filename,e.location.begin.line));

      if (typeCheck) {
          Luau::CheckResult cr = ((OutlineWidget *)this->parent())->frontend->check(filename.toStdString().c_str());
          for (const Luau::TypeError &e: cr.errors) {
              const Luau::UnknownSymbol *uk=e.data.get_if<Luau::UnknownSymbol>();
              if ((!uk)||(uk->context!=Luau::UnknownSymbol::Binding))
                li.append(OutlineLinterItem(QString(Luau::toString(e).c_str()),OutlineLinterItem::LinterType::TypeError,filename,e.location.begin.line));
          }
      }
      //COMPILER
      std::string error = Luau::compile(std::string(btext.constData(),btext.size()), filename.toStdString(), opts,popts,nullptr,&result);
      if (error.at(0)==0) {
          QString qerror=QString(error.substr(1).c_str());
          emit reportError("[string "+filename+"]"+qerror,li,QSet<QString>());
      }
      else  {
          OutlineVisitor visitor(&outline);
          result.root->visit(&visitor);
          AutocompleteVisitor autocv;
          result.root->visit(&autocv);
          autocv.autocomplete << "_"; //Non empty
          emit reportError("",li,autocv.autocomplete);
      }
#endif
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
    connect(list_, SIGNAL(clicked(QModelIndex)),
            this, SLOT  (onItemClicked(QModelIndex)));
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
#ifdef LUA_IS_LUAU
    //LINTER
    Luau::FrontendOptions frontendOptions;
    frontendOptions.retainFullTypeGraphs = true; //Annotate
    fileResolver=new OutlineFileResolver(this);
    configResolver.defaultConfig.mode=Luau::Mode::Nonstrict;
    configResolver.defaultConfig.enabledLint.disableWarning(Luau::LintWarning::Code_UnknownGlobal);
    configResolver.defaultConfig.enabledLint.disableWarning(Luau::LintWarning::Code_ImplicitReturn);
    configResolver.defaultConfig.enabledLint.disableWarning(Luau::LintWarning::Code_FunctionUnused);
    configResolver.defaultConfig.enabledLint.disableWarning(Luau::LintWarning::Code_SameLineStatement);
    configResolver.defaultConfig.enabledLint.disableWarning(Luau::LintWarning::Code_MultiLineStatement);

    frontend=new Luau::Frontend(fileResolver, &configResolver, frontendOptions);
    Luau::registerBuiltinTypes(frontend->typeChecker);

    std::stringstream gid_api;
    gid_api << "export type Shader = any\n";
    //load API
    QFile file("Resources/gideros_annot.api");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream textStream(&file);
        QRegularExpression re("^([^.:]*)[.:]?([^?.:]*)\\?\\d(\\([^)]*\\))?(.*)");
        QMap<QString,QStringList> methods;
        while (!textStream.atEnd()) {
            auto m=re.match(textStream.readLine());
            if (m.hasMatch()) {
                QString cl=m.captured(1);
                QString mtd=m.captured(2);
                if (mtd.isEmpty()) {
                    mtd=cl;
                    cl="";
                }
                if (cl.isEmpty()) {
                    cl="(globals)";
                    if (m.captured(3).isEmpty())
                        methods[cl] << "declare "+mtd+": any";
                    else
                        methods[cl] << "declare function "+mtd+"<A...,R...>(...:A...): R...";
                }
                else {
                    if (m.captured(3).isEmpty())
                        methods[cl] << mtd+": any";
                    else
                        methods[cl] << mtd+": <A...,R...>(A...) -> R...";
                }
            }
        }
        for (auto &m:methods.keys()) {
            if (m!="(globals)") {
                gid_api << "declare " << m.toStdString() << ": {\n";
                for (auto &mm:methods[m]) {
                    gid_api << mm.toStdString() << ",\n";
                }
                gid_api << "}\n";
            }
            else {
                for (auto &mm:methods[m]) {
                    gid_api << mm.toStdString() << "\n";
                }
            }
        }
        file.close();
    }

    Luau::loadDefinitionFile(frontend->typeChecker, frontend->typeChecker.globalScope, gid_api.str(), "@gideros");

    //Luau::freeze(frontend->typeChecker.globalTypes);
#endif
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
#ifdef SCINTILLAEDIT_H
        int hl=std::min(ml+5,(int)(doc_->sciScintilla()->lineFromPosition(doc_->sciScintilla()->textLength())-1));
#else
        int hl=std::min(ml+5,doc_->sciScintilla()->lines()-1);
#endif
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

void OutlineWidget::reportError(const QString error, QList<OutlineLinterItem> lint,QSet<QString> autocomplete)
{
    if (!doc_) return;
#ifdef SCINTILLAEDIT_H
    doc_->setIdentifiers(autocomplete.values());
    ScintillaEdit *s=doc_->sciScintilla();
    QString text;
    int size=s->textLength();
    if (size>0)
        text=s->getText(size);
    s->eOLAnnotationClearAll();
    if ((error.isEmpty()&&lint.isEmpty())||(!checkSyntax_)) return;
    int stylenum=STYLE_LASTPREDEFINED+10;
    //Error
    s->styleSetFore(stylenum,0xFFFFFF);
    s->styleSetBack(stylenum,0x000080);
    //Linter Note
    s->styleSetFore(stylenum+1+OutlineLinterItem::Note,0xFFFFFF);
    s->styleSetBack(stylenum+1+OutlineLinterItem::Note,0x804040);
    //Linter Warning
    s->styleSetFore(stylenum+1+OutlineLinterItem::Warning,0xFFFFFF);
    s->styleSetBack(stylenum+1+OutlineLinterItem::Warning,0x008080);
    //Linter Error
    s->styleSetFore(stylenum+1+OutlineLinterItem::Error,0xFFFFFF);
    s->styleSetBack(stylenum+1+OutlineLinterItem::Error,0x004080);
    //Typer Error
    s->styleSetFore(stylenum+1+OutlineLinterItem::TypeError,0xFFFFFF);
    s->styleSetBack(stylenum+1+OutlineLinterItem::TypeError,0x400040);

    s->eOLAnnotationSetVisible(EOLANNOTATION_STADIUM);

    QRegularExpression re("\\[string [^\\]]+\\]:(\\d+):(.*)");
    QRegularExpressionMatch match = re.match(error);
    if (match.hasMatch()) {
        QString line = match.captured(1);
        QString err = match.captured(2);
        int lineNum=line.toInt();
        s->eOLAnnotationSetText(lineNum-1,err.toUtf8());
        s->eOLAnnotationSetStyle(lineNum-1,stylenum);
    }
    for (const auto &e:lint) {
        if (e.file==doc_->fileName()) {
            s->eOLAnnotationSetText(e.line,e.message.toUtf8());
            s->eOLAnnotationSetStyle(e.line,stylenum+1+e.type);
        }
    }
#else
    QsciScintilla *s=doc_->sciScintilla();
    s->clearAnnotations();
    if ((error.isEmpty()&&lint.isEmpty())||(!checkSyntax_)) return;
    int stylenum=QsciScintilla::STYLE_LASTPREDEFINED+10;
    //Error
    s->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE,stylenum,QColor("white"));
    s->SendScintilla(QsciScintillaBase::SCI_STYLESETBACK,stylenum,QColor(0x80,0x00,0x00));
    //Linter Note
    s->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE,stylenum+1+OutlineLinterItem::Note,QColor("white"));
    s->SendScintilla(QsciScintillaBase::SCI_STYLESETBACK,stylenum+1+OutlineLinterItem::Note,QColor(0x80,0x80,0x80));
    //Linter Warning
    s->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE,stylenum+1+OutlineLinterItem::Warning,QColor("white"));
    s->SendScintilla(QsciScintillaBase::SCI_STYLESETBACK,stylenum+1+OutlineLinterItem::Warning,QColor(0x80,0x80,0x00));
    //Linter Error
    s->SendScintilla(QsciScintillaBase::SCI_STYLESETFORE,stylenum+1+OutlineLinterItem::Error,QColor("white"));
    s->SendScintilla(QsciScintillaBase::SCI_STYLESETBACK,stylenum+1+OutlineLinterItem::Error,QColor(0x80,0x40,0x00));
    s->setAnnotationDisplay(QsciScintilla::AnnotationBoxed);

    QRegularExpression re("\\[string [^\\]]+\\]:(\\d+):(.*)");
    QRegularExpressionMatch match = re.match(error);
    if (match.hasMatch()) {
        QString line = match.captured(1);
        QString err = match.captured(2);
        int lineNum=line.toInt();
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
    for (const auto &e:lint) {
        if (e.file==doc_->fileName()) {
            s->setAnnotationDisplay(QsciScintilla::AnnotationBoxed);
            s->annotate(e.line,e.message,stylenum+1+e.type);
        }
    }
#endif
}

void OutlineWidget::parse() {
    bool noContent=true;
    needParse_=working_;
    OutlineWorkerThread::typeCheck=typeCheck_;
    if (working_)
        return;
    if (doc_) {
        QFileInfo fileInfo(doc_->fileName());
        if (!fileInfo.suffix().compare(QString("lua"),Qt::CaseInsensitive))
        {
#ifdef SCINTILLAEDIT_H
            ScintillaEdit *s=doc_->sciScintilla();
            int size=s->textLength();
            if (size>0) {
                QString text=s->getText(size);
#else
                QsciScintilla *s=doc_->sciScintilla();
                QString text=s->text();
#endif
                QByteArray btext=text.toUtf8();
                noContent=false;

                frontend->markDirty(doc_->fileName().toStdString());

                OutlineWorkerThread *workerThread = new OutlineWorkerThread(this,doc_->fileName(),btext);
                connect(workerThread, &OutlineWorkerThread::updateOutline, this, &OutlineWidget::updateOutline);
                connect(workerThread, &OutlineWorkerThread::reportError, this, &OutlineWidget::reportError);
                connect(workerThread, &OutlineWorkerThread::finished, workerThread, &QObject::deleteLater);
                workerThread->start();
                working_=true;
#ifdef SCINTILLAEDIT_H
            }
#endif
        }
    }
    if (noContent)
        updateOutline(OutLineItemList());
}

void OutlineWidget::checkParse() {
    if (refresh_.elapsed()>(TYPING_DELAY-100))
        parse();
}

void OutlineWidget::setDocument(TextEdit *doc,bool checkSyntax,bool typeCheck)
{
    bool docChanged=(doc_!=doc);
    doc_=doc;
    checkSyntax_=checkSyntax;
    typeCheck_=typeCheck;
    if (docChanged)
        parse();
    else
    {
        refresh_.start();
        QTimer::singleShot(TYPING_DELAY, this, SLOT(checkParse()));
    }
}
