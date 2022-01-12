#include "textedit.h"
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QApplication>
#include <QFileInfo>
#include <QCloseEvent>
#include <QSettings>
#include <QToolTip>
#include "iconlibrary.h"
#include "settingskeys.h"
#include <ScintillaEdit/ILexer.h>
#include <Lexillia/Lexilla.h>
#include <Lexillia/SciLexer.h>

QSet<QString> TextEdit::breakpoints;
#ifdef Q_OS_MAC
static void keysForMac(QsciScintilla* qscintilla)
{
	QsciCommandSet* commandSet = qscintilla->standardCommands();
	QList<QsciCommand*> commands = commandSet->commands();

	for (int i = 0; i < commands.size(); ++i)
	{
		switch (commands[i]->key())
		{
		// case insensitive left/right (option+left/right)
		case Qt::Key_Left | Qt::CTRL:
			commands[i]->setKey(Qt::Key_Left | Qt::ALT);
			break;
		case Qt::Key_Right | Qt::CTRL:
			commands[i]->setKey(Qt::Key_Right | Qt::ALT);
			break;
		case Qt::Key_Left | Qt::CTRL | Qt::SHIFT:
			commands[i]->setKey(Qt::Key_Left | Qt::ALT | Qt::SHIFT);
			break;
		case Qt::Key_Right | Qt::CTRL | Qt::SHIFT:
			commands[i]->setKey(Qt::Key_Right | Qt::ALT | Qt::SHIFT);
			break;

		// start of line / end of line (command + left/right)
		case Qt::Key_Home:
			commands[i]->setAlternateKey(Qt::Key_Left | Qt::CTRL);
			break;
		case Qt::Key_End:
			commands[i]->setAlternateKey(Qt::Key_Right | Qt::CTRL);
			break;
		case Qt::Key_Home | Qt::SHIFT:
			commands[i]->setAlternateKey(Qt::Key_Left | Qt::CTRL | Qt::SHIFT);
			break;
		case Qt::Key_End | Qt::SHIFT:
			commands[i]->setAlternateKey(Qt::Key_Right | Qt::CTRL | Qt::SHIFT);
			break;

		// begin of document / end of document (command + up/down)
		case Qt::Key_Home | Qt::CTRL:
			commands[i]->setAlternateKey(Qt::Key_Up | Qt::CTRL);
			break;
		case Qt::Key_End | Qt::CTRL:
			commands[i]->setAlternateKey(Qt::Key_Down | Qt::CTRL);
			break;
		case Qt::Key_Home | Qt::CTRL | Qt::SHIFT:
			commands[i]->setAlternateKey(Qt::Key_Up | Qt::CTRL | Qt::SHIFT);
			break;
		case Qt::Key_End | Qt::CTRL | Qt::SHIFT:
			commands[i]->setAlternateKey(Qt::Key_Down | Qt::CTRL | Qt::SHIFT);
			break;

		// delete word to left (option + backspace)
		case Qt::Key_Backspace | Qt::CTRL:
			commands[i]->setKey(Qt::Key_Backspace | Qt::ALT);
			break;

		// delete line to the left (command + backspace)
		case Qt::Key_Backspace | Qt::CTRL | Qt::SHIFT:
			commands[i]->setKey(Qt::Key_Backspace | Qt::CTRL);
			break;

		// page up / page down (control + up/down)
		case Qt::Key_PageUp:
			commands[i]->setAlternateKey(Qt::Key_Up | Qt::META);
			break;
		case Qt::Key_PageDown:
			commands[i]->setAlternateKey(Qt::Key_Down | Qt::META);
			break;
		case Qt::Key_PageUp | Qt::SHIFT:
			commands[i]->setAlternateKey(Qt::Key_Up | Qt::META | Qt::SHIFT);
			break;
		case Qt::Key_PageDown | Qt::SHIFT:
			commands[i]->setAlternateKey(Qt::Key_Down | Qt::META | Qt::SHIFT);
			break;
		}
	}

	// case sensitive left right (control + left/right)
	const int SCMOD_SHIFT = QsciScintillaBase::SCMOD_SHIFT;
	const int SCMOD_META = 16;
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
}
#endif

static ILexer5 *createLexerByExtension(QString ext,ScintillaEdit *editor)
{
	ext = ext.toLower();

    ILexer5 *lexer = NULL;

    QSettings settings;
    QString themePath = settings.value(Keys::Editor::theme).toString();

	if (ext == "lua")
	{
        lexer = CreateLexer(LexerNameFromID(SCLEX_LUA));
#if 0
		QsciAPIs* api = new QsciAPIs(lexer);
//		api->add(QString("addEventListener(type, listener, [data]) Registers a listener function and an optional data value"));
		api->load("Resources/gideros_annot.api");
		api->prepare();
		lexer->setAPIs(api);
        if (themePath != "")
        {
        QSettings editorTheme(themePath, QSettings::IniFormat);
        lexer->readSettings(editorTheme);
        }
        else
        {
        }
#endif
        editor->setILexer((sptr_t)lexer);

        editor->setKeyWords(0,"and break continue do else elseif end false for function if "
                                           "in local nil not or repeat return then true until "
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
                                            "math.randomseed math.sin math.sqrt math.tan");
        editor->setKeyWords(3,"openfile closefile readfrom writeto appendto remove "
                                            "rename flush seek tmpfile tmpname read write clock "
                                            "date difftime execute exit getenv setlocale time "

                                            "coroutine.create coroutine.resume coroutine.status "
                                            "coroutine.wrap coroutine.yield io.close io.flush "
                                            "io.input io.lines io.open io.output io.read "
                                            "io.tmpfile io.type io.write io.stdin io.stdout "
                                            "io.stderr os.clock os.date os.difftime os.execute "
                                            "os.exit os.getenv os.remove os.rename os.setlocale "
                                            "os.time os.tmpname");

        editor->setProperty("fold","1");
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

    if (lexer && themePath == "")
	{ 
#ifdef Q_OS_MAC
        editor->styleSetFont(STYLE_DEFAULT,"Monaco");
        editor->styleSetSize(STYLE_DEFAULT,12);
#else
        editor->styleSetFont(STYLE_DEFAULT,"Courier New");
        editor->styleSetSize(STYLE_DEFAULT,10);
#endif
        editor->styleSetBack(STYLE_DEFAULT,0xFFFFFF);
	}
    else if (lexer) {
#if 0
        QSettings editorTheme(themePath, QSettings::IniFormat);
        lexer->readSettings(editorTheme);
#endif
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

    sciScintilla_->setMarginTypeN(4, SC_MARGIN_SYMBOL);
    sciScintilla_->setMarginWidthN(4, 16);
    sciScintilla_->setMarginMaskN(4, SC_MASK_FOLDERS);
    sciScintilla_->markerDefine(SC_MARKNUM_FOLDER,SC_MARK_BOXPLUS);
    sciScintilla_->markerDefine(SC_MARKNUM_FOLDEROPEN,SC_MARK_BOXMINUS);
    sciScintilla_->markerDefine(SC_MARKNUM_FOLDERSUB,SC_MARK_VLINE);
    sciScintilla_->markerDefine(SC_MARKNUM_FOLDERTAIL,SC_MARK_LCORNER);
    sciScintilla_->markerDefine(SC_MARKNUM_FOLDEREND,SC_MARK_BOXPLUSCONNECTED);
    sciScintilla_->markerDefine(SC_MARKNUM_FOLDEROPENMID,SC_MARK_BOXMINUSCONNECTED);
    sciScintilla_->markerDefine(SC_MARKNUM_FOLDERMIDTAIL,SC_MARK_TCORNER);
    sciScintilla_->setAutomaticFold(SC_AUTOMATICFOLD_CHANGE|SC_AUTOMATICFOLD_CLICK|SC_AUTOMATICFOLD_SHOW);

/*    sciScintilla_->setFoldMarginColors(
                lls.value("FoldMarginFirstColor", 16777215).toInt(),
                lls.value("FoldMarginSecondColor", 10066329).toInt()
    );*/

#if 0
    sciScintilla_-> setAutoIndent(true);
#endif
    sciScintilla_->setTabWidth(settings.value(Keys::Prefs::tabSize, 4).toInt());
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
        lls.value("CaretForegroundColor", 0).toInt());

    sciScintilla_->setCaretLineBack(
        lls.value("CaretLineBackgroundColor", 15658734).toInt());


    sciScintilla_->styleSetFore(STYLE_BRACELIGHT,
        lls.value("MatchedBraceForegroundColor", 0).toInt());
    sciScintilla_->styleSetBack(STYLE_BRACELIGHT,
        lls.value("MatchedBraceBackgroundColor", 15658734).toInt());

    sciScintilla_->styleSetFore(STYLE_BRACEBAD,
        lls.value("UnmatchedBraceForegroundColor", 0).toInt());
    sciScintilla_->styleSetBack(STYLE_BRACEBAD,
        lls.value("UnmatchedBraceBackgroundColor", 10085887).toInt());

	connect(sciScintilla_, SIGNAL(modificationChanged(bool)), this, SLOT(onModificationChanged(bool)));
	connect(sciScintilla_, SIGNAL(copyAvailable(bool)), this, SIGNAL(copyAvailable(bool)));
    connect(sciScintilla_, SIGNAL(textChanged()), this, SIGNAL(textChanged()));
    connect(sciScintilla_, SIGNAL(marginClicked(int, int, Qt::KeyboardModifiers)),
            this, SLOT(setBookmark(int, int, Qt::KeyboardModifiers)));


    sciScintilla_->setMarginWidthN(1, 14);			// margin 1 is breakpoint
    sciScintilla_->markerDefine(2,SC_MARK_CIRCLE); //Marker 2 is breakpoint
    sciScintilla_->setMarginMaskN(1, 1 << 2);
    sciScintilla_->setMarginSensitiveN(1, true);

    sciScintilla_->setMarginWidthN(3, 14);			// margin 3 is bookmark margin
    sciScintilla_->markerDefine(1, SC_MARK_BOOKMARK); // Marker 1 is bookmark
    sciScintilla_->setMarginMaskN(3, 1 << 1);
    sciScintilla_->setMarginSensitiveN(3, true);

    sciScintilla_->markerDefine(3, SC_MARK_BACKGROUND); //Marker 3 is current debug line

    sciScintilla_->registerRGBAImage(1,(const char*)IconLibrary::instance().icon(0,"method").pixmap(16).toImage().convertToFormat(QImage::Format_RGB888).bits());
    sciScintilla_->registerRGBAImage(2,(const char*)IconLibrary::instance().icon(0,"constant").pixmap(16).toImage().convertToFormat(QImage::Format_RGB888).bits());
    sciScintilla_->registerRGBAImage(3,(const char*)IconLibrary::instance().icon(0,"event").pixmap(16).toImage().convertToFormat(QImage::Format_RGB888).bits());
    sciScintilla_->registerRGBAImage(4,(const char*)IconLibrary::instance().icon(0,"class").pixmap(16).toImage().convertToFormat(QImage::Format_RGB888).bits());

    sciScintilla_->markerSetFore(
                lls.value("MarkerForegroundColor", 2566178).toInt(), 1);
    sciScintilla_->markerSetBack(
                lls.value("MarkerBackgroundColor", 5348047).toInt(), 1);
    sciScintilla_->markerSetFore(
                lls.value("MarkerForegroundColor", 2566178).toInt(), 2);
    sciScintilla_->markerSetBack(
                lls.value("MarkerBackgroundColor", 5348047).toInt(), 2);
    sciScintilla_->markerSetBack(
                lls.value("DebuggedLineColor", 0x3030FF).toInt(), 3);

    sciScintilla_->setEOLMode(SC_EOL_LF);

    sciScintilla_->setMultipleSelection(true);
    // multi-line typing with Alt+drag/Alt_shift+cursor multi-line selections
    sciScintilla_->setAdditionalSelectionTyping(true);

#if 0
    sciScintilla_->setAutoCompletionThreshold(settings.value(Keys::Prefs::autoCompleteChars, 2).toInt());
	sciScintilla_->setAutoCompletionSource(QsciScintilla::AcsAll);
#endif

    sciScintilla_->styleSetFore(STYLE_INDENTGUIDE,lls.value("IndentationGuidesForegroundColor", 0).toInt());
    sciScintilla_->styleSetBack(STYLE_INDENTGUIDE,lls.value("IndentationGuidesBackgroundColor", 8421504).toInt());

    sciScintilla_->styleSetFore(STYLE_LINENUMBER, lls.value("MarginsForegroundColor", 2566178).toInt());
    sciScintilla_->styleSetBack(STYLE_LINENUMBER, lls.value("MarginsBackgroundColor", 15658734).toInt());

    sciScintilla_->setMouseDwellTime(500);
    connect(sciScintilla_, SIGNAL(SCN_DWELLSTART(int,int,int)), this, SLOT(dwellStart(int,int,int)));
    connect(sciScintilla_, SIGNAL(SCN_DWELLEND(int,int,int)), this, SLOT(dwellEnd(int,int,int)));
}

TextEdit::~TextEdit()
{

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
#if 0
        sciScintilla_->SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (int) 0, (const char *)
                                     "et arreter faire sinon fin faux pour si "
                                     "dans local rien non ou repeter retourne alors vrai jusqua tantque"
                                     );
#endif
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

	setWindowTitle(fileInfo.fileName() + "[*]");

	QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	QString txt=in.readAll();
    if ((lexer!=NULL)&&(!strcmp(fileInfo.suffix().toUtf8(),"lua"))&&txt.startsWith("!")&&(txt.length()>3))
        setLuaLanguage(txt.mid(1,2).toUpper());
    sciScintilla_->setText(txt.toUtf8());
	QApplication::restoreOverrideCursor();

    modified=false;
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

const QString& TextEdit::fileName() const
{
	return fileName_;
}

void TextEdit::onModificationChanged(bool m)
{
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
    out << sciScintilla_->getText(sciScintilla_->textLength());
	QApplication::restoreOverrideCursor();

    modified=false;
	setWindowModified(false);

	return true;
}

void TextEdit::setCursorPosition(int line, int index)
{
    sciScintilla_->setCurrentPos(sciScintilla_->findColumn(line, index));
}

bool TextEdit::isModified() const
{
    return modified;
}

bool TextEdit::findFirst(	const QString &expr, bool re, bool cs, bool wo,
							bool wrap, bool forward/* = true*/)
{
#if 0
	int lineFrom, indexFrom, lineTo, indexTo;
	sciScintilla_->getSelection (&lineFrom, &indexFrom, &lineTo, &indexTo);

	if (lineFrom == -1 || indexFrom == -1 || lineTo == -1 || indexTo == -1)
	{
		sciScintilla_->getCursorPosition(&lineFrom, &indexFrom);
		lineTo = lineFrom;
		indexTo = indexFrom;
	}

	int line, index;
	if (forward == true)
	{
		line = lineTo;
		index = indexTo;
	}
	else
	{
		line = lineFrom;
		index = indexFrom;
	}

	return sciScintilla_->findFirst(expr, re, cs, wo, wrap, forward, line, index);
#endif
    return false;
}

bool TextEdit::replace(	const QString &expr, const QString &replaceStr, bool re, bool cs, bool wo,
						 bool wrap)
{
	bool forward = true;
#if 0
	int lineFrom, indexFrom, lineTo, indexTo;
	sciScintilla_->getSelection (&lineFrom, &indexFrom, &lineTo, &indexTo);

	if (lineFrom == -1 || indexFrom == -1 || lineTo == -1 || indexTo == -1)
	{
		// if there is no selected text, do find and return
		int line, index;
		sciScintilla_->getCursorPosition(&line, &index);
		return sciScintilla_->findFirst(expr, re, cs, wo, wrap, forward, line, index);
	}

	// if there is selected text, first do find from the *beginning* of selection
	bool found;
	
	found = sciScintilla_->findFirst(expr, re, cs, wo, wrap, forward, lineFrom, indexFrom);

	if (found == false)
		return false;

	int newlineFrom, newindexFrom, newlineTo, newindexTo;
	sciScintilla_->getSelection(&newlineFrom, &newindexFrom, &newlineTo, &newindexTo);

	// check if new selection is between old selection
	if (lineFrom <= newlineFrom && indexFrom <= newindexFrom && lineTo >= newlineTo && indexTo >= newindexTo)
	{
		// replace text and find again
		sciScintilla_->replace(replaceStr);
		sciScintilla_->getSelection(&newlineFrom, &newindexFrom, &newlineTo, &newindexTo);
		sciScintilla_->findFirst(expr, re, cs, wo, wrap, forward, newlineTo, newindexTo);
	}
	else
	{
		// if not, restore the selection and find from the *end* of selection
		sciScintilla_->setSelection(lineFrom, indexFrom, lineTo, indexTo);
		found = sciScintilla_->findFirst(expr, re, cs, wo, wrap, forward, lineTo, indexTo);
	}
    return found;
#endif
    return false;
}

int TextEdit::replaceAll(const QString &expr, const QString &replaceStr, bool re, bool cs, bool wo,
				bool wrap)
{
	sciScintilla_->beginUndoAction();
#if 0
	bool forward = true;

	int line, index;
    int pos=sciScintilla_->currentPos();
    sciScintilla_->lineFromPosition(pos);
    sciScintilla_->column(pos);

	int lineFrom, indexFrom, lineTo, indexTo;
    sciScintilla_->selection (&lineFrom, &indexFrom, &lineTo, &indexTo);

	int replaceline, replaceindex;

	if (lineFrom == -1 || indexFrom == -1 || lineTo == -1 || indexTo == -1)
	{
		replaceline = line;
		replaceindex = index;
	}
	else
	{
		replaceline = lineFrom;		// replaceAll starts from *beginning* of selection
		replaceindex = indexFrom;
	}

	int replaced = 0;
	while (true)
	{
        bool found = sciScintilla_->find (expr, re, cs, wo, wrap, forward, replaceline, replaceindex);
		if (found == false)
			break;
		
		sciScintilla_->replace(replaceStr);
		replaced++;

		int newlineFrom, newindexFrom, newlineTo, newindexTo;
		sciScintilla_->getSelection(&newlineFrom, &newindexFrom, &newlineTo, &newindexTo);

		replaceline = newlineTo;
		replaceindex = newindexTo;
	}

	// restore old selection
	sciScintilla_->setSelection(lineFrom, indexFrom, lineTo, indexTo);

	// restore old cursor position
	sciScintilla_->setCursorPosition(line, index);
#endif
	sciScintilla_->endUndoAction();

    return 0;//replaced;
}

void TextEdit::setBookmark(int margin, int line, Qt::KeyboardModifiers state)
{
    Q_UNUSED(state);
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
        sciScintilla_->setCurrentPos(sciScintilla_->positionFromLine(next));
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
        sciScintilla_->setCurrentPos(sciScintilla_->positionFromLine(prev));
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
    sciScintilla_->setFocus(true);
}

void TextEdit::highlightDebugLine(int line) {
    sciScintilla_->markerDeleteAll(3);
    if (line>=0)
    {
        sciScintilla_->setCurrentPos(sciScintilla_->positionFromLine(line));
        sciScintilla_->markerAdd(line, 3);
        sciScintilla_->setReadOnly(true);
        sciScintilla_->setCaretLineVisible(false);
    }
    else {
        sciScintilla_->setReadOnly(false);
        sciScintilla_->setCaretLineVisible(true);
    }
}

void TextEdit::dwellStart(int pos,int x,int y)
{
    if (pos<0) return;
    int ipos=pos;
    QString text=sciScintilla_->getText(sciScintilla_->textLength());
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

void TextEdit::dwellEnd(int pos,int x,int y)
{
    Q_UNUSED(pos);
    QToolTip::showText(QPoint(x,y),QString(),this);
}
