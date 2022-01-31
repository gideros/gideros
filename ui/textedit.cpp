#include "textedit.h"
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QApplication>
#include <QFileInfo>
#include <QCloseEvent>
#include <QSettings>
#include <QToolTip>
#include <QRegularExpression>
#include "iconlibrary.h"
#include "settingskeys.h"
#include <ScintillaEdit/ILexer.h>
#include <Lexilla/Lexilla.h>
#include <Lexilla/SciLexer.h>
#include <QStandardPaths>
#include <QDir>

QSet<QString> TextEdit::breakpoints;
#ifdef Q_OS_MAC
static void keysForMac(ScintillaEdit* qscintilla)
{
#if 0    //We use a more recent scintilla, assume key mappings are correct for OSX, we'll check in the long run if tweaks are needed or not
	qscintilla->SendScintilla(	QsciScintillaBase::SCI_ASSIGNCMDKEY,
								(SCMOD_META << 16) | QsciScintillaBase::SCK_LEFT,
								QsciScintillaBase::SCI_WORDPARTLEFT);
	qscintilla->SendScintilla(	QsciScintillaBase::SCI_ASSIGNCMDKEY,
								(SCMOD_META << 16) | QsciScintillaBase::SCK_RIGHT,
								QsciScintillaBase::SCI_WORDPARTRIGHT);
	qscintilla->SendScintilla(	QsciScintillaBase::SCI_ASSIGNCMDKEY,
								((SCMOD_META | SCMOD_SHIFT) << 16) | QsciScintillaBase::SCK_LEFT,
								QsciScintillaBase::SCI_WORDPARTLEFTEXTEND);
	qscintilla->SendScintilla(	QsciScintillaBase::SCI_ASSIGNCMDKEY,
								((SCMOD_META | SCMOD_SHIFT) << 16) | QsciScintillaBase::SCK_RIGHT,
								QsciScintillaBase::SCI_WORDPARTRIGHTEXTEND);

	qscintilla->SendScintilla(	QsciScintillaBase::SCI_ASSIGNCMDKEY,
								(SCMOD_META << 16) | QsciScintillaBase::SCK_BACK,
								QsciScintillaBase::SCI_DELWORDLEFT);
#endif
}
#endif

static int rcolor(int c) {
    return( (c&0xFF)<<16)|(c&0xFF00FF00)|((c&0xFF0000)>>16);
}

// Restore the user settings.
static void readLexerSettings(QSettings &qs,const char *language,ScintillaEdit *editor)
{
    bool ok, flag;
    int num;
    QString key, full_key;
    QStringList fdesc;
    const char *prefix="Scintilla";

    // Read the styles.
    for (int i = 0; i <= STYLE_MAX; ++i)
    {
        key = QString("%1/%2/style%3/").arg(prefix).arg(language).arg(i);

        // Read the foreground colour.
        full_key = key + "color";

        ok = qs.contains(full_key);
        num = qs.value(full_key).toInt();

        if (ok)
            editor->styleSetFore(i, rcolor(num));

        // Read the end-of-line fill.
        full_key = key + "eolfill";

        ok = qs.contains(full_key);
        flag = qs.value(full_key, false).toBool();

        if (ok)
            editor->styleSetEOLFilled(i,flag);

        // Read the font.  First try the deprecated format that uses an integer
        // point size.
        full_key = key + "font";

        ok = qs.contains(full_key);
        fdesc = qs.value(full_key).toStringList();

        if (ok && fdesc.count() == 5)
        {
            editor->styleSetFont(i, fdesc[0].toUtf8());
            editor->styleSetSize(i, fdesc[1].toInt());
            editor->styleSetBold(i, fdesc[2].toInt());
            editor->styleSetItalic(i, fdesc[3].toInt());
            editor->styleSetUnderline(i, fdesc[4].toInt());
        }

        // Now try the newer font format that uses a floating point point size.
        // It is not an error if it doesn't exist.
        full_key = key + "font2";

        ok = qs.contains(full_key);
        fdesc = qs.value(full_key).toStringList();

        if (ok)
        {
            // Allow for future versions with more fields.
            if (fdesc.count() >= 5)
            {
                editor->styleSetFont(i, fdesc[0].toUtf8());
                editor->styleSetSize(i, fdesc[1].toDouble());
                editor->styleSetBold(i, fdesc[2].toInt());
                editor->styleSetItalic(i, fdesc[3].toInt());
                editor->styleSetUnderline(i, fdesc[4].toInt());
            }
        }

        // Read the background colour.
        full_key = key + "paper";

        ok = qs.contains(full_key);
        num = qs.value(full_key).toInt();

        if (ok)
            editor->styleSetBack(i, rcolor(num));
    }

    /*
    // Read the properties.
    key = QString("%1/%2/properties/").arg(prefix).arg(language);
*/

    // Read the rest.
    key = QString("%1/%2/").arg(prefix).arg(language);

    // Read the default foreground colour.
    full_key = key + "defaultcolor";

    ok = qs.contains(full_key);
    num = qs.value(full_key).toInt();

    if (ok)
        editor->styleSetFore(STYLE_DEFAULT, rcolor(num));

    // Read the default background colour.
    full_key = key + "defaultpaper";

    ok = qs.contains(full_key);
    num = qs.value(full_key).toInt();

    if (ok)
        editor->styleSetBack(STYLE_DEFAULT, rcolor(num));

    // Read the default font.  First try the deprecated format that uses an
    // integer point size.
    full_key = key + "defaultfont";

    ok = qs.contains(full_key);
    fdesc = qs.value(full_key).toStringList();

    if (ok && fdesc.count() == 5)
    {
        editor->styleSetFont(STYLE_DEFAULT, fdesc[0].toUtf8());
        editor->styleSetSize(STYLE_DEFAULT, fdesc[1].toInt());
        editor->styleSetBold(STYLE_DEFAULT, fdesc[2].toInt());
        editor->styleSetItalic(STYLE_DEFAULT, fdesc[3].toInt());
        editor->styleSetUnderline(STYLE_DEFAULT, fdesc[4].toInt());
    }

    // Now try the newer font format that uses a floating point point size.  It
    // is not an error if it doesn't exist.
    full_key = key + "defaultfont2";

    ok = qs.contains(full_key);
    fdesc = qs.value(full_key).toStringList();

    if (ok)
    {
        // Allow for future versions with more fields.
        if (fdesc.count() >= 5)
        {
            editor->styleSetFont(STYLE_DEFAULT, fdesc[0].toUtf8());
            editor->styleSetSize(STYLE_DEFAULT, fdesc[1].toDouble());
            editor->styleSetBold(STYLE_DEFAULT, fdesc[2].toInt());
            editor->styleSetItalic(STYLE_DEFAULT, fdesc[3].toInt());
            editor->styleSetUnderline(STYLE_DEFAULT, fdesc[4].toInt());
        }
    }
		
	full_key = key + "fold.compact";
	ok = qs.contains(full_key);
	
	if (ok)
	{
		flag = qs.value(full_key, true).toBool();
		editor->setProperty("fold.compact", flag ? "1" : "0");
	}
}

static ILexer5 *createLexerByExtension(QString ext,ScintillaEdit *editor)
{
	ext = ext.toLower();

    ILexer5 *lexer = NULL;

    QSettings settings;
    QString themePath = settings.value(Keys::Editor::theme).toString();

	if (ext == "lua")
	{
        lexer = CreateLexer(LexerNameFromID(SCLEX_LUA));
        editor->setILexer((sptr_t)lexer);

        editor->setKeyWords(0,"and break const continue declare do else elseif end export false for function if "
                                           "in local nil not or repeat return then true type until "
                                           "while");
        editor->setKeyWords(1,"_ALERT _ERRORMESSAGE _INPUT _PROMPT _OUTPUT _STDERR "
                                            "_STDIN _STDOUT call dostring foreach foreachi getn "
                                            "globals newtype rawget rawset require sort tinsert "
                                            "tremove "

                                            "G getfenv getmetatable ipairs loadlib next pairs "
                                            "pcall rawegal rawget rawset require setfenv "
                                            "setmetatable xpcall string table math coroutine io "
                                            "os debug");
        editor->setKeyWords(2,"abs acos asin atan atan2 ceil cos deg exp floor "
                                            "format frexp gsub ldexp log log10 max min mod rad "
                                            "random randomseed sin sqrt strbyte strchar strfind "
                                            "strlen strlower strrep strsub strupper tan "

                                            "string.byte string.char string.dump string.find "
                                            "string.len string.lower string.rep string.sub "
                                            "string.upper string.format string.gfind string.gsub "
                                            "table.concat table.foreach table.foreachi table.getn "
                                            "table.sort table.insert table.remove table.setn "
                                            "math.abs math.acos math.asin math.atan math.atan2 "
                                            "math.ceil math.cos math.deg math.exp math.floor "
                                            "math.frexp math.ldexp math.log math.log10 math.max "
                                            "math.min math.mod math.pi math.rad math.random "
                                            "math.randomseed math.sin math.sqrt math.tan "
											"math.inside math.dot math.cosh math.round "
											"math.tanh math.modf math.huge math.cross math.pow "
											"math.edge math.raycast math.normalize math.nearest math.distances "
											"math.clamp math.length math.noise math.sign math.distance "
											"math.sinh math.fmod string.split string.match string.gmatch "
											"string.pack string.dumpPseudocode string.decodeValue string.packsize string.reverse "
											"string.unpack string.encodeValue table.clear table.pack table.move "
											"table.isfrozen table.maxn table.unpack table.find table.create");
        editor->setKeyWords(3,"openfile closefile readfrom writeto appendto remove "
                                            "rename flush seek tmpfile tmpname read write clock "
                                            "date difftime execute exit getenv setlocale time "

                                            "coroutine.create coroutine.resume coroutine.status "
                                            "coroutine.wrap coroutine.yield io.close io.flush "
                                            "io.input io.lines io.open io.output io.read "
                                            "io.tmpfile io.type io.write io.stdin io.stdout "
                                            "io.stderr os.clock os.date os.difftime os.execute "
                                            "os.exit os.getenv os.remove os.rename os.setlocale "
                                            "os.time os.tmpname coroutine.running coroutine.close "
											"coroutine.isyieldable os.timer");
		
		editor->setProperty("fold","1");
		editor->setProperty("fold.compact", settings.value(Keys::Prefs::foldCompact).toBool() ? "1" : "0");
        if (themePath == "")
        {
            editor->styleSetFore(SCE_LUA_COMMENT,0x007F00);
            editor->styleSetFore(SCE_LUA_COMMENTLINE,0x007F00);
            editor->styleSetFore(SCE_LUA_NUMBER,0x0080FF);
            editor->styleSetFore(SCE_LUA_WORD,0xFF0000);
            editor->styleSetFore(SCE_LUA_WORD2,0x7F0000);
            editor->styleSetFore(SCE_LUA_WORD3,0x7F0000);
            editor->styleSetFore(SCE_LUA_WORD4,0x7F0000);

            editor->styleSetFore(SCE_LUA_STRING,0x7F007F);
            editor->styleSetFore(SCE_LUA_CHARACTER,0x7F007F);
            editor->styleSetFore(SCE_LUA_LITERALSTRING,0x7F007F);
            editor->styleSetFore(SCE_LUA_PREPROCESSOR,0x007F7F);
            editor->styleSetFore(SCE_LUA_LABEL,0x007F7F);
        }
    }
    else if (ext == "xml")
    {
        lexer = CreateLexer(LexerNameFromID(SCLEX_XML));
        editor->setILexer((sptr_t)lexer);
    }
    else if ((ext == "hlsl") || (ext == "glsl") )
    {
        lexer = CreateLexer(LexerNameFromID(SCLEX_CPP));
        editor->setILexer((sptr_t)lexer);
    }
    else if (ext == "json")
    {
        lexer = CreateLexer(LexerNameFromID(SCLEX_JSON));
        editor->setILexer((sptr_t)lexer);
    }

    if (lexer)
	{ 
#ifdef Q_OS_MAC
        editor->styleSetFont(STYLE_DEFAULT,"Monaco");
        editor->styleSetSize(STYLE_DEFAULT,12);
#else
        editor->styleSetFont(STYLE_DEFAULT,"Courier New");
        editor->styleSetSize(STYLE_DEFAULT,10);
#endif
        editor->styleSetBack(STYLE_DEFAULT,0xFFFFFF);
        if (themePath != "") {
            QSettings editorTheme(themePath, QSettings::IniFormat);
            readLexerSettings(editorTheme,lexer->GetName(),editor);
        }
	}

	return lexer;
}

TextEdit::TextEdit(QWidget* parent)
	: MdiSubWindow(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);

    sciScintilla_ = new ScintillaEdit(this);
	setWidget(sciScintilla_);

#ifdef Q_OS_MAC
	keysForMac(sciScintilla_);
#endif
    //KeyMaps
    QDir shared(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation));
    shared.mkpath("Gideros");
    bool sharedOk = shared.cd("Gideros");
    if (sharedOk) {
        QSettings kmap(shared.absoluteFilePath("keymap.ini"), QSettings::IniFormat);
        kmap.beginGroup("scintilla");
        for (auto k:kmap.childKeys()) {
            auto v=kmap.value(k).toUInt();
            int kmod=0;
            while (true) {
                k=k.trimmed();
                if (k.startsWith("Shift+",Qt::CaseInsensitive)) {
                    k=k.mid(6);
                    kmod|=SCMOD_SHIFT;
                    continue;
                }
                if (k.startsWith("Alt+",Qt::CaseInsensitive)) {
                    k=k.mid(4);
                    kmod|=SCMOD_ALT;
                    continue;
                }
                if (k.startsWith("Ctrl+",Qt::CaseInsensitive)) {
                    k=k.mid(5);
                    kmod|=SCMOD_CTRL;
                    continue;
                }
                break;
            }
            kmod=(kmod<<16)|(k.toUpper().at(0).unicode()&0xFFFF);
            sciScintilla_->clearCmdKey(kmod);
            if (v>0)
                sciScintilla_->assignCmdKey(kmod,v);
        }
        kmap.endGroup();
    }
	isUntitled_ = true;

// settings
QSettings settings;
QString theme = settings.value(Keys::Editor::theme).toString();

QSettings lls(theme, QSettings::IniFormat);

#ifdef Q_OS_MAC
    sciScintilla_->styleSetFont(STYLE_DEFAULT,lls.value("FontFamily", "Monaco").toString().toUtf8());
    sciScintilla_->styleSetSize(STYLE_DEFAULT,lls.value("FontSize", 12).toInt());
#else
    sciScintilla_->styleSetFont(STYLE_DEFAULT,lls.value("FontFamily", "Lucida Console").toString().toUtf8());
    sciScintilla_->styleSetSize(STYLE_DEFAULT,lls.value("FontSize", 12).toInt());
#endif

    sciScintilla_->setElementColour(SC_ELEMENT_SELECTION_TEXT,rcolor(QApplication::palette().highlightedText().color().rgba()));
    sciScintilla_->setElementColour(SC_ELEMENT_SELECTION_BACK,rcolor(QApplication::palette().highlight().color().rgba()));


    sciScintilla_->setMarginTypeN(4, SC_MARGIN_SYMBOL);
    sciScintilla_->setMarginWidthN(4, 16);
    sciScintilla_->setMarginMaskN(4, SC_MASK_FOLDERS);
    sciScintilla_->setMarginSensitiveN(4, true);
    sciScintilla_->markerDefine(SC_MARKNUM_FOLDER,SC_MARK_BOXPLUS);
    sciScintilla_->markerDefine(SC_MARKNUM_FOLDEROPEN,SC_MARK_BOXMINUS);
    sciScintilla_->markerDefine(SC_MARKNUM_FOLDERSUB,SC_MARK_VLINE);
    sciScintilla_->markerDefine(SC_MARKNUM_FOLDERTAIL,SC_MARK_LCORNER);
    sciScintilla_->markerDefine(SC_MARKNUM_FOLDEREND,SC_MARK_BOXPLUSCONNECTED);
    sciScintilla_->markerDefine(SC_MARKNUM_FOLDEROPENMID,SC_MARK_BOXMINUSCONNECTED);
    sciScintilla_->markerDefine(SC_MARKNUM_FOLDERMIDTAIL,SC_MARK_TCORNER);
    sciScintilla_->setAutomaticFold(SC_AUTOMATICFOLD_CHANGE|SC_AUTOMATICFOLD_CLICK|SC_AUTOMATICFOLD_SHOW);

    sciScintilla_->setFoldMarginColour(true,rcolor(lls.value("FoldMarginFirstColor", 16777215).toInt()));

    sciScintilla_->setFoldMarginHiColour(true,rcolor(lls.value("FoldMarginSecondColor", 10066329).toInt()));

    sciScintilla_->setTabWidth(settings.value(Keys::Prefs::tabSize, 4).toInt());
    sciScintilla_->setIndent(0); //Same as tabs
    sciScintilla_->setUseTabs(!settings.value(Keys::Prefs::tabsVsSpaces, 0).toBool());
    sciScintilla_->setIndentationGuides(settings.value(Keys::Prefs::indentGuides, true).toBool());
    sciScintilla_->setBackSpaceUnIndents(settings.value(Keys::Prefs::backspaceUnindents, false).toBool());

    sciScintilla_->setViewWS((sptr_t) (settings.value(Keys::Prefs::whitespaceVisibility, 0).toInt()));
	
    if (settings.value(Keys::Prefs::showLineNumbers, true).toBool()) {
        sciScintilla_->setMarginTypeN(2, SC_MARGIN_NUMBER);
        sciScintilla_->setMarginWidthN(2, sciScintilla_->textWidth(STYLE_LINENUMBER,"10000"));
        sciScintilla_->setMarginMaskN(2, 0);		// we dont want any markers at line number margin
    }
    else
        sciScintilla_->setMarginWidthN(2, 0);

    sciScintilla_->braceHighlightIndicator(true,INDIC_STRAIGHTBOX);

	sciScintilla_->setCaretLineVisible(true);

    sciScintilla_->setCaretFore(
        rcolor(lls.value("CaretForegroundColor", 0).toInt()));

    sciScintilla_->setCaretLineBack(
        rcolor(lls.value("CaretLineBackgroundColor", 15658734).toInt()));


    sciScintilla_->styleSetFore(STYLE_BRACELIGHT,
        rcolor(lls.value("MatchedBraceForegroundColor", 0).toInt()));
    sciScintilla_->styleSetBack(STYLE_BRACELIGHT,
        rcolor(lls.value("MatchedBraceBackgroundColor", 15658734).toInt()));

    sciScintilla_->styleSetFore(STYLE_BRACEBAD,
        rcolor(lls.value("UnmatchedBraceForegroundColor", 0).toInt()));
    sciScintilla_->styleSetBack(STYLE_BRACEBAD,
        rcolor(lls.value("UnmatchedBraceBackgroundColor", 10085887).toInt()));

    connect(sciScintilla_, SIGNAL(savePointChanged(bool)), this, SLOT(onModificationChanged(bool)));
	connect(sciScintilla_, SIGNAL(copyAvailable(bool)), this, SIGNAL(copyAvailable(bool)));
    connect(sciScintilla_, SIGNAL(notifyChange()), this, SIGNAL(textChanged()));
    connect(sciScintilla_, SIGNAL(marginClicked(Scintilla::Position,Scintilla::KeyMod,int)),
            this, SLOT(setBookmark(Scintilla::Position,Scintilla::KeyMod,int)));


    sciScintilla_->setMarginWidthN(1, 14);			// margin 1 is breakpoint
    sciScintilla_->markerDefine(2,SC_MARK_CIRCLE); //Marker 2 is breakpoint
    sciScintilla_->markerDefine(3, SC_MARK_SHORTARROW); //Marker 3 is current debug line
    sciScintilla_->setMarginMaskN(1, 3 << 2);
    sciScintilla_->setMarginSensitiveN(1, true);

    sciScintilla_->setMarginWidthN(3, 14);			// margin 3 is bookmark margin
    sciScintilla_->markerDefine(1, SC_MARK_BOOKMARK); // Marker 1 is bookmark
    sciScintilla_->setMarginMaskN(3, 1 << 1);
    sciScintilla_->setMarginSensitiveN(3, true);


    registerIcon(1,IconLibrary::instance().icon(0,"method"));
    registerIcon(2,IconLibrary::instance().icon(0,"constant"));
    registerIcon(3,IconLibrary::instance().icon(0,"event"));
    registerIcon(4,IconLibrary::instance().icon(0,"class"));

    sciScintilla_->markerSetFore(1,rcolor(lls.value("MarkerForegroundColor", 0x272822).toInt()));
    sciScintilla_->markerSetBack(1,rcolor(lls.value("MarkerBackgroundColor", 0x519ACF).toInt()));
    sciScintilla_->markerSetFore(2,rcolor(lls.value("DebugDotForeground", 0x000000).toInt()));
    sciScintilla_->markerSetBack(2,rcolor(lls.value("DebugDotBackground", 0xFF3030).toInt()));
    sciScintilla_->markerSetFore(3,rcolor(lls.value("DebugArrowForeground", 0x000000).toInt()));
    sciScintilla_->markerSetBack(3,rcolor(lls.value("DebugArrowBackgroundr", 0xFFFF30).toInt()));
    for (int k=SC_MARKNUM_FOLDEREND;k<=SC_MARKNUM_FOLDEROPEN;k++) {
        sciScintilla_->markerSetFore(k,rcolor(lls.value("MarkerForegroundColor", 0xFFFFFF).toInt()));
        sciScintilla_->markerSetBack(k,rcolor(lls.value("MarkerBackgroundColor", 0).toInt()));
    }

    sciScintilla_->setEOLMode(SC_EOL_LF);

    sciScintilla_->setMultipleSelection(true);
    // multi-line typing with Alt+drag/Alt_shift+cursor multi-line selections
    sciScintilla_->setAdditionalSelectionTyping(true);
    sciScintilla_->setMultiPaste(SC_MULTIPASTE_EACH);

    autoCompleteThreshold=settings.value(Keys::Prefs::autoCompleteChars, 2).toInt();

    sciScintilla_->styleSetFore(STYLE_INDENTGUIDE,rcolor(lls.value("IndentationGuidesForegroundColor", 0).toInt()));
    sciScintilla_->styleSetBack(STYLE_INDENTGUIDE,rcolor(lls.value("IndentationGuidesBackgroundColor", 8421504).toInt()));

    sciScintilla_->styleSetFore(STYLE_LINENUMBER, rcolor(lls.value("MarginsForegroundColor", 2566178).toInt()));
    sciScintilla_->styleSetBack(STYLE_LINENUMBER, rcolor(lls.value("MarginsBackgroundColor", 15658734).toInt()));

    sciScintilla_->setMouseDwellTime(500);
    connect(sciScintilla_, SIGNAL(dwellStart(int,int)), this, SLOT(dwellStart(int,int)));
    connect(sciScintilla_, SIGNAL(dwellEnd(int,int)), this, SLOT(dwellEnd(int,int)));
    connect(sciScintilla_, SIGNAL(updateUi(Scintilla::Update)), this, SLOT(updateUi(Scintilla::Update)));
    connect(sciScintilla_, SIGNAL(charAdded(int)), this, SLOT(charAdded(int)));
    connect(sciScintilla_, SIGNAL(callTipClick(Scintilla::Position)), this, SLOT(callTipClick(Scintilla::Position)));
}

TextEdit::~TextEdit()
{

}

void TextEdit::registerIcon(int num,QIcon icon)
{
    QImage img=icon.pixmap(16).toImage();
    if (img.isNull()) return;
    img=img.convertToFormat(QImage::Format_RGBA8888);
    sciScintilla_->rGBAImageSetWidth(img.width());
    sciScintilla_->rGBAImageSetHeight(img.height());
    sciScintilla_->registerRGBAImage(num,(const char *)img.bits());
}

void TextEdit::newFile()
{
	static int sequenceNumber = 1;
	isUntitled_ = true;
	fileName_ = tr("new %1").arg(sequenceNumber++);
	setWindowTitle(fileName_ + "[*]");
}

void TextEdit::setLuaLanguage(QString lang) {
    if (lang=="FR") {
        sciScintilla_->setKeyWords(0,"et arreter continuer faire sinon fin faux pour si "
                                     "dans local rien non ou repeter retourne alors vrai jusqua tantque"
                                     );
    }
}

bool TextEdit::loadFile(const QString& fileName, const QString& itemName, bool suppressErrors/* = false*/)
{
	QFile file(fileName);
	if (file.open(QFile::ReadOnly | QFile::Text) == false)
	{
		if (suppressErrors == false)
		{
			QMessageBox::warning(0, tr("Warning"),
				tr("Cannot read file %1:\n%2.")
                .arg(fileName,file.errorString()));
		}
		return false;
	}

	isUntitled_ = true;
	fileName_ = fileName;
    itemName_ = itemName;

	QFileInfo fileInfo(fileName_);

    ILexer5 *lexer = createLexerByExtension(fileInfo.suffix(),sciScintilla_);
    api.clear();
    if (fileInfo.suffix()=="lua") {
        //load API
        QFile file("Resources/gideros_annot.api");
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream textStream(&file);
            while (!textStream.atEnd())
                api << textStream.readLine();
            file.close();
        }
    }

	setWindowTitle(fileInfo.fileName() + "[*]");

	QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	QString txt=in.readAll();
    if ((lexer!=NULL)&&(!strcmp(fileInfo.suffix().toUtf8(),"lua"))&&txt.startsWith("!")&&(txt.length()>3))
        setLuaLanguage(txt.mid(1,2).toUpper());
    sciScintilla_->setText(txt.toUtf8());
	QApplication::restoreOverrideCursor();

    sciScintilla_->clearSelections();
    sciScintilla_->colourise(0,-1);

    modified=false;
    sciScintilla_->emptyUndoBuffer();
	setWindowModified(false);

    foreach(QString bp, breakpoints) {
        int ls=bp.lastIndexOf(':');
        if (bp.mid(0,ls)==itemName_) {
            int line=bp.mid(ls+1).toInt();
            sciScintilla_->markerAdd(line, 2);
        }
    }
	
	return true;
}

void TextEdit::BlockComment() {
    int from=sciScintilla_->lineFromPosition(sciScintilla_->selectionStart());
    int to=sciScintilla_->lineFromPosition(sciScintilla_->selectionEnd());
    if (from<0) return;
    sciScintilla_->setTargetRange(sciScintilla_->positionFromLine(from),sciScintilla_->positionBefore(sciScintilla_->positionFromLine(from+1)));
    bool uncomment=sciScintilla_->searchInTarget(2,"--")>=0;
    for (int l=from;l<=to;l++) {
        if (uncomment) {
            sciScintilla_->setTargetRange(sciScintilla_->positionFromLine(l),sciScintilla_->positionBefore(sciScintilla_->positionFromLine(l+1)));
            if (sciScintilla_->searchInTarget(2,"--")>=0)
                sciScintilla_->replaceTarget(0,"");
        }
        else
            sciScintilla_->insertText(sciScintilla_->positionFromLine(l),"--");
    }
}

const QString& TextEdit::fileName() const
{
	return fileName_;
}

void TextEdit::onModificationChanged(bool m)
{
    modified=m;
	setWindowModified(m);
}

bool TextEdit::hasSelectedText() const
{
    return !sciScintilla_->selectionEmpty();
}

bool TextEdit::isRedoAvailable() const
{
    return sciScintilla_->canRedo();
}

bool TextEdit::isUndoAvailable() const
{
    return sciScintilla_->canUndo();
}

bool TextEdit::save()
{
    int line=-1;
    QStringList bpRemove;
    foreach(QString bp, breakpoints) {
        int ls=bp.lastIndexOf(':');
        if (bp.mid(0,ls)==itemName_)
            bpRemove << bp;
    }
    foreach(QString bp, bpRemove)
        breakpoints.remove(bp);
    while ((line = sciScintilla_->markerNext(line+1, (1 << 2)))>=0)
        breakpoints.insert(itemName_+":"+QString::number(line));

    if (modified == false)
		return false;

	QFile file(fileName_);
	if (!file.open(QFile::WriteOnly | QFile::Text))
	{
		QMessageBox::warning(0, tr("Warning"),
			tr("Cannot write file %1:\n%2.")
			.arg(fileName_)
			.arg(file.errorString()));
		return false;
	}

	QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
	QApplication::setOverrideCursor(Qt::WaitCursor);
    int size=sciScintilla_->textLength();
    if (size>0) {
        QString text=sciScintilla_->getText(size);
        out << text;
    }
	QApplication::restoreOverrideCursor();

    modified=false;
	setWindowModified(false);
    sciScintilla_->setSavePoint();

	return true;
}

void TextEdit::setCursorPosition(int line, int index)
{
    sciScintilla_->gotoPos(sciScintilla_->findColumn(line, index));
}

bool TextEdit::isModified() const
{
    return modified;
}

bool TextEdit::findFirst(	const QString &expr, bool re, bool cs, bool wo,
							bool wrap, bool forward/* = true*/)
{
    sciScintilla_->setSearchFlags((cs?SCFIND_MATCHCASE:0)|(wo?SCFIND_WHOLEWORD:0)|(re?SCFIND_REGEXP:0));

    int from,to;
    int sf,st;
    from=sciScintilla_->selectionStart();
    to=sciScintilla_->selectionEnd();

    if (forward)
	{
        sf=sciScintilla_->positionAfter(to);
        st=sciScintilla_->textLength();
    }
	else
	{
        sf=sciScintilla_->positionBefore(from);
        st=0;
	}

    sciScintilla_->setTargetRange(sf,st);
    if (sciScintilla_->searchInTarget(expr.size(),expr.toUtf8())>=0) {
        sciScintilla_->setSel(sciScintilla_->targetStart(),sciScintilla_->targetEnd());
        return true;
    }
    if (!wrap) return false;

    if (forward)
    {
        sf=0;
        st=sciScintilla_->positionBefore(from);
    }
    else
    {
        sf=sciScintilla_->textLength();
        st=sciScintilla_->positionAfter(to);
    }

    sciScintilla_->setTargetRange(sf,st);
    if (sciScintilla_->searchInTarget(expr.size(),expr.toUtf8())>=0) {
        sciScintilla_->setSel(sciScintilla_->targetStart(),sciScintilla_->targetEnd());
        return true;
    }

    return false;
}

bool TextEdit::replace(	const QString &expr, const QString &replaceStr, bool re, bool cs, bool wo,
						 bool wrap)
{
    sciScintilla_->setSearchFlags((cs?SCFIND_MATCHCASE:0)|(wo?SCFIND_WHOLEWORD:0)|(re?SCFIND_REGEXP:0));

    int from=sciScintilla_->selectionStart();
    int to=sciScintilla_->selectionEnd();
    int sf=sciScintilla_->positionAfter(to);
    int st=sciScintilla_->textLength();

    if (from==to)
    {
        // if there is no selected text, do find and return
        sciScintilla_->setTargetRange(from,st);
        if (sciScintilla_->searchInTarget(expr.size(),expr.toUtf8())>=0) {
            sciScintilla_->setSel(sciScintilla_->targetStart(),sciScintilla_->targetEnd());
            return true;
        }
        if (wrap) {
            sf=0;
            st=sciScintilla_->positionBefore(from);
            sciScintilla_->setTargetRange(sf,st);
            if (sciScintilla_->searchInTarget(expr.size(),expr.toUtf8())>0) {
                sciScintilla_->setSel(sciScintilla_->targetStart(),sciScintilla_->targetEnd());
                return true;
            }
        }
        return false;
    }

	// if there is selected text, first do find from the *beginning* of selection
	bool found;
    sciScintilla_->setTargetRange(from,st);
    found=sciScintilla_->searchInTarget(expr.size(),expr.toUtf8())>0;

    if (found == false) {
        if (wrap&&sf) {
            sf=0;
            st=sciScintilla_->positionBefore(from);
            sciScintilla_->setTargetRange(sf,st);
            found=sciScintilla_->searchInTarget(expr.size(),expr.toUtf8())>0;
        }
        if (found == false)
            return false;
    }

    int nfrom=sciScintilla_->targetStart();
    int nto=sciScintilla_->targetEnd();

	// check if new selection is between old selection
    if (from <= nfrom && to >= nto)
	{
		// replace text and find again
        sciScintilla_->replaceTarget(replaceStr.size(),replaceStr.toUtf8());
        sciScintilla_->setTargetRange(nto,st);
        if (sciScintilla_->searchInTarget(expr.size(),expr.toUtf8())>0)
            sciScintilla_->setSel(sciScintilla_->targetStart(),sciScintilla_->targetEnd());
        else {
            if (wrap&&sf) {
                sf=0;
                st=sciScintilla_->positionBefore(nfrom);
                sciScintilla_->setTargetRange(sf,st);
                if (sciScintilla_->searchInTarget(expr.size(),expr.toUtf8())>0)
                    sciScintilla_->setSel(sciScintilla_->targetStart(),sciScintilla_->targetEnd());
            }
        }
	}
	else
	{
        // if not, select the found item
        sciScintilla_->setSel(nfrom,nto);
	}

    return found;
}

int TextEdit::replaceAll(const QString &expr, const QString &replaceStr, bool re, bool cs, bool wo,
				bool wrap)
{
    sciScintilla_->setSearchFlags((cs?SCFIND_MATCHCASE:0)|(wo?SCFIND_WHOLEWORD:0)|(re?SCFIND_REGEXP:0));
    sciScintilla_->beginUndoAction();

    int pos=sciScintilla_->currentPos();
    int sf=pos;
    int st=sciScintilla_->textLength();

	int replaced = 0;
	while (true)
	{
        bool found;
        sciScintilla_->setTargetRange(pos,st);
        found=sciScintilla_->searchInTarget(expr.size(),expr.toUtf8())>0;
        if (found==false) {
            if (wrap&&sf) {
                sf=0;
                st=sciScintilla_->positionBefore(pos);
                pos=0;
                sciScintilla_->setTargetRange(pos,st);
                found=sciScintilla_->searchInTarget(expr.size(),expr.toUtf8())>0;
            }
            if (!found) break;
        }

        sciScintilla_->replaceTarget(replaceStr.size(),replaceStr.toUtf8());
		replaced++;
        pos=sciScintilla_->targetEnd();
	}

	sciScintilla_->endUndoAction();

    return replaced;
}

void TextEdit::setBookmark(Scintilla::Position position, Scintilla::KeyMod modifiers, int margin)
{
    Q_UNUSED(modifiers);
    int line=sciScintilla_->lineFromPosition(position);
    int marker=(margin==3)?1:2;
    if (sciScintilla_->markerGet(line) & (1 << marker))
    {
        sciScintilla_->markerDelete(line, marker);
        if (marker==2)
            breakpoints.remove(itemName_+":"+QString::number(line));
    }
    else {
        sciScintilla_->markerAdd(line, marker);
        if (marker==2)
            breakpoints.insert(itemName_+":"+QString::number(line));
    }
}

void TextEdit::toogleBookmark()
{
    int line=sciScintilla_->lineFromPosition(sciScintilla_->currentPos());

    if (sciScintilla_->markerGet(line) & (1 << 1))
		sciScintilla_->markerDelete(line, 1);
	else
		sciScintilla_->markerAdd(line, 1);
}

void TextEdit::nextBookmark()
{
    int line=sciScintilla_->lineFromPosition(sciScintilla_->currentPos());

	int next = -1;
	
    next = sciScintilla_->markerNext(line + 1, (1 << 1));

	if (next < 0)
        next = sciScintilla_->markerNext(0, (1 << 1));

	if (next >= 0)
        sciScintilla_->gotoLine(next);
}

void TextEdit::previousBookmark()
{
    int line=sciScintilla_->lineFromPosition(sciScintilla_->currentPos());

	int prev = -1;
	
	if (line > 0)
        prev = sciScintilla_->markerPrevious(line - 1, (1 << 1));

	if (prev < 0)
        prev = sciScintilla_->markerPrevious(sciScintilla_->lineFromPosition(sciScintilla_->textLength()), (1 << 1));

	if (prev >= 0)
        sciScintilla_->gotoLine(prev);
}

void TextEdit::clearBookmarks()
{
	sciScintilla_->markerDeleteAll(1);
}

// preferencesdialog
void TextEdit::setTabWidth(int size)
{
    sciScintilla_->setTabWidth(size);
}

void TextEdit::setUseTabs(bool use_tabs)
{
    sciScintilla_->setTabIndents(use_tabs);
}

void TextEdit::setIndentGuide(bool index)
{
    sciScintilla_->setIndentationGuides(index);
}

void TextEdit::setShowLineNumbers(bool show)
{
    if (show) {
        sciScintilla_->setMarginTypeN(2, SC_MARGIN_NUMBER);
        sciScintilla_->setMarginWidthN(2, sciScintilla_->textWidth(STYLE_LINENUMBER,"10000"));
        sciScintilla_->setMarginMaskN(2, 0);		// we dont want any markers at line number margin
    }
    else
        sciScintilla_->setMarginWidthN(2, 0);
}

void TextEdit::setBackspaceUnindents(bool use)
{
    sciScintilla_->setBackSpaceUnIndents(use);
}

void TextEdit::setWhitespaceVisibility(int mode)
{
    sciScintilla_->setViewWS(mode);
}

void TextEdit::setCompactFolding(int mode)
{
	sciScintilla_->setProperty("fold.compact", mode ? "1" : "0");
	sciScintilla_->clearDocumentStyle(); 
	sciScintilla_->colourise(0, -1);
}

void TextEdit::undo()
{
	sciScintilla_->undo();
}

void TextEdit::redo()
{
	sciScintilla_->redo();
}

void TextEdit::background() {
    sciScintilla_->autoCCancel();
    sciScintilla_->callTipCancel();
}

bool TextEdit::maybeSave()
{
	if (isModified())
	{
		QMessageBox msgBox(this);
		msgBox.setWindowTitle("Save");
		msgBox.setText(QString("Save file\"%1\" ?").arg(fileName_));
		msgBox.setIcon(QMessageBox::Question);
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Yes);
		int ret = msgBox.exec();		
		switch (ret)
        {
			case QMessageBox::Yes:
				save();
				return true;
			case QMessageBox::No:
				return true;
			case QMessageBox::Cancel:
				return false;
		}		
	}

	return true;
}

void TextEdit::closeEvent(QCloseEvent* event)
{
	if (maybeSave())
	{
		event->accept();
	} 
	else
	{
		event->ignore();
	}
}

void TextEdit::setFocusToEdit()
{
    //sciScintilla_->setFocus(true);
    sciScintilla_->grabFocus();
}

void TextEdit::highlightDebugLine(int line) {
    sciScintilla_->markerDeleteAll(3);
    if (line>=0)
    {
        sciScintilla_->gotoLine(line);
        sciScintilla_->markerAdd(line, 3);
    }
}

void TextEdit::dwellStart(int x,int y)
{
    int pos=sciScintilla_->positionFromPoint(x,y);
    if (pos<0) return;
    int size=sciScintilla_->textLength();
    if (size<=0) return;
    int ipos=pos;
    QString text=sciScintilla_->getText(size);
    QByteArray bu=text.toUtf8();
    pos=QString::fromUtf8(bu.mid(0,pos)).length();
    while ((pos>0)&&(
    		(text.at(pos-1)=='_')||
			text.at(pos-1).isLetterOrNumber()||
			((text.at(pos-1)=='.')&&(pos>1)&&(text.at(pos-2)!='.'))
			))
        pos--;
    int pend=pos;
    while ((pend<text.size())&&(
    		(text.at(pend)=='_')||
			text.at(pend).isLetterOrNumber()||
			((pend<ipos)&&(text.at(pend)=='.')&&(pend<(text.size()-1))&&(text.at(pend+1)!='.'))
			))
        pend++;
    QString id=text.mid(pos,pend-pos);
    if (id.size()>0) {
        emit lookupSymbol(id,x,y);
    }
}

void TextEdit::dwellEnd(int x,int y)
{
    QToolTip::showText(QPoint(x,y),QString(),this);
}

void TextEdit::setIdentifiers(const QStringList &ilist)
{
    if (!ilist.empty())
        autocIdentifiers=ilist;
};

void TextEdit::charAdded(int ch)
{
    if (ch=='\n') {
        //AUTOINDENT
        int cline=sciScintilla_->lineFromPosition(sciScintilla_->currentPos());
        int pIndent=sciScintilla_->lineIndentation(cline-1);
        int level=sciScintilla_->foldLevel(cline-1);
        if (level & SC_FOLDLEVELHEADERFLAG) //Foldable: Indent
            pIndent+=sciScintilla_->tabWidth();
        sciScintilla_->setLineIndentation(cline,pIndent);
        sciScintilla_->gotoPos(sciScintilla_->lineIndentPosition(cline));
    }
    else if (ch=='(') {
        //CALL-TIP
        //		api->add(QString("addEventListener(type, listener, [data]) Registers a listener function and an optional data value"));
        int pos=sciScintilla_->positionBefore(sciScintilla_->currentPos());
        int ws=sciScintilla_->wordStartPosition(pos,true);
        int we=sciScintilla_->wordEndPosition(pos,true);
        if ((pos==we)&&(we>ws)) {
            QString word=sciScintilla_->textRange(ws,we);
            if (word.size()>=autoCompleteThreshold) {
                //Check matches in API
                QRegularExpression re("^([^.:]*)[.:]?("+word+")\\?\\d(\\([^)]*\\))(.*)");
                QStringList ctip;
                for (const QString &c:api) {
                    auto m=re.match(c);
                    if (m.hasMatch()) {
                        QString cl=m.captured(1);
                        if (cl.isEmpty()) cl="global";
                        ctip << m.captured(2)+m.captured(3)+" ["+cl+"]"+m.captured(4);
                    }
                }
                if (!ctip.isEmpty()) {
                    currentCallTipList.clear();
                    currentCallTipIndex=0;
                    ctip.sort();
                    ctip.removeDuplicates();
                    if (ctip.size()>4) {
                        size_t n=1;
                        size_t l=ctip.size();
                        for (const QString &c:ctip) {
                            currentCallTipList << QString("\001 %1 of %2 \002%3").arg(n++).arg(l).arg(c);
                        }
                        currentCallTipPos=ws;
                        sciScintilla_->callTipShow(currentCallTipPos,currentCallTipList[currentCallTipIndex].toUtf8());
                    }
                    else
                        sciScintilla_->callTipShow(ws,ctip.join('\n').toUtf8());
                }
            }
        }
    }
    else if (ch==')') {
        sciScintilla_->callTipCancel();
    }
    else {
        //AUTOCOMPLETE
        int pos=sciScintilla_->currentPos();
        int ws=sciScintilla_->wordStartPosition(pos,true);
        int we=sciScintilla_->wordEndPosition(pos,true);
        if ((pos==we)&&(we>ws)) {
            QString word=sciScintilla_->textRange(ws,we);
            if (word.size()>=autoCompleteThreshold) {
                //Check matches in API
                QRegularExpression re("^([^.:])*[.:]?("+word+"[^?.:]*\\?\\d).*");
                QStringList autoc;
                for (const QString &c:api) {
                    auto m=re.match(c);
                    if (m.hasMatch())
                        autoc << m.captured(2);
                }
                //Add from current identifier set
                for (const QString &c:autocIdentifiers)
                    if (c.startsWith(word))
                        autoc << c;
                if (!autoc.isEmpty()) {
                    autoc.sort();
                    autoc.removeDuplicates();
                    sciScintilla_->autoCShow(word.size(),autoc.join(' ').toUtf8());
                }
            }
        }
    }
}

void TextEdit::callTipClick(Scintilla::Position position) {
    if (position==1) {
        if (currentCallTipIndex==0)
            currentCallTipIndex=currentCallTipList.size()-1;
        else
            currentCallTipIndex--;
        sciScintilla_->callTipShow(currentCallTipPos,currentCallTipList[currentCallTipIndex].toUtf8());
    }
    if (position==2) {
        if (currentCallTipIndex==(size_t)(currentCallTipList.size()-1))
            currentCallTipIndex=0;
        else
            currentCallTipIndex++;
        sciScintilla_->callTipShow(currentCallTipPos,currentCallTipList[currentCallTipIndex].toUtf8());
    }
}

void TextEdit::updateUi(Scintilla::Update updated)
{
    Q_UNUSED(updated);
    int brace_position = sciScintilla_->positionBefore(sciScintilla_->currentPos());
    int character_before = sciScintilla_->charAt(brace_position);
    char ch = (char) character_before;
    if(ch == ']' || ch == '[' || ch == '{' || ch == '}' || ch == '(' || ch == ')' || ch == '<' || ch == '>')
    {
         int has_match = sciScintilla_->braceMatch(brace_position,0);
         if(has_match > -1)
             sciScintilla_->braceHighlight(has_match, brace_position);
         else
             sciScintilla_->braceBadLight(brace_position);
     }
     else
     {
        char ch = (char) sciScintilla_->charAt(brace_position);
        if(ch == ']' || ch == '[' || ch == '{' || ch == '}' || ch == '(' || ch == ')' || ch == '<' || ch == '>')
        {
             int has_match = sciScintilla_->braceMatch(brace_position,0);
             if(has_match > -1)
                 sciScintilla_->braceHighlight(has_match, brace_position);
             else
                 sciScintilla_->braceBadLight(brace_position);
         }
         else
         {
            sciScintilla_->braceHighlight(-1,-1);
         }
     }
}
