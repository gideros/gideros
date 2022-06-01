#include "textedit.h"
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include <QApplication>
#include <QFileInfo>
#include <Qsci/qscilexercpp.h>
#include <Qsci/qscilexerlua.h>
#include <Qsci/qscilexerxml.h>
#include <Qsci/qscilexer.h>
#include <QCloseEvent>
#include <Qsci/qsciapis.h>
#include <Qsci/qscicommand.h>
#include <Qsci/qscicommandset.h>
#include <QSettings>
#include <QToolTip>
#include "iconlibrary.h"
#include "settingskeys.h"

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

static QsciLexer* createLexerByExtension(QString ext)
{
	ext = ext.toLower();

	QsciLexer* lexer = 0;

    QSettings settings;
    QString themePath = settings.value(Keys::Editor::theme).toString();

	if (ext == "lua")
	{
		QsciLexerLua* lexerlua = new QsciLexerLua;
		lexer = lexerlua;

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
            lexer->setColor(Qt::blue, QsciLexerLua::Keyword);
            lexer->setColor(QColor(0xff, 0x80, 0x00), QsciLexerLua::Number);
        }
    }
    else if (ext == "xml")
    {
        lexer = new QsciLexerXML;
    }
    else if ((ext == "hlsl") || (ext == "glsl") )
    {
        lexer = new QsciLexerCPP;

        if (themePath != "")
        {
        QSettings editorTheme(themePath, QSettings::IniFormat);
        lexer->readSettings(editorTheme);
        }
    }

    if (lexer && themePath == "")
	{ 
#ifdef Q_OS_MAC
		lexer->setFont(QFont("Monaco", 12));
#else
		lexer->setFont(QFont("Courier New", 10));
#endif
		lexer->setPaper(QColor(255, 255, 255));
	}

	return lexer;
}

TextEdit::TextEdit(QWidget* parent)
	: MdiSubWindow(parent)
{
	setAttribute(Qt::WA_DeleteOnClose);

	sciScintilla_ = new QsciScintilla(this);
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
    sciScintilla_->setFont(QFont(
        lls.value("FontFamily", "Monaco").toString(),
        lls.value("FontSize", 12).toInt()));
#else
    sciScintilla_->setFont(QFont(
        lls.value("FontFamily", "Lucida Console").toString(),
        lls.value("FontSize", 12).toInt()));
#endif

	sciScintilla_->setFolding(QsciScintilla::BoxedTreeFoldStyle, 4);
	sciScintilla_->setAutoIndent(true);

    sciScintilla_->setTabWidth(settings.value(Keys::Prefs::tabSize, 4).toInt());
    sciScintilla_->setIndentationsUseTabs(!settings.value(Keys::Prefs::tabsVsSpaces, 0).toBool());
    sciScintilla_->setIndentationGuides(settings.value(Keys::Prefs::indentGuides, true).toBool());
    sciScintilla_->setBackspaceUnindents(settings.value(Keys::Prefs::backspaceUnindents, false).toBool());

    sciScintilla_->setWhitespaceVisibility(static_cast<QsciScintilla::WhitespaceVisibility>
                                           (settings.value(Keys::Prefs::whitespaceVisibility, 0).toInt()));

    sciScintilla_->setMarginLineNumbers(2, settings.value(Keys::Prefs::showLineNumbers, true).toBool());
	sciScintilla_->setMarginWidth(2, QString("10000"));
	sciScintilla_->setMarginMarkerMask(2, 0);		// we dont want any markers at line number margin

	sciScintilla_->setBraceMatching(QsciScintilla::SloppyBraceMatch);

	sciScintilla_->setUtf8(true);

	sciScintilla_->setCaretLineVisible(true);

    sciScintilla_->setCaretForegroundColor (
        lls.value("CaretForegroundColor", 0).toInt());

    sciScintilla_->setCaretLineBackgroundColor(
        lls.value("CaretLineBackgroundColor", 15658734).toInt());

    sciScintilla_->setMatchedBraceForegroundColor(
        lls.value("MatchedBraceForegroundColor", 0).toInt());
    sciScintilla_->setMatchedBraceBackgroundColor(
        lls.value("MatchedBraceBackgroundColor", 15658734).toInt());

    sciScintilla_->setUnmatchedBraceForegroundColor(
        lls.value("UnmatchedBraceForegroundColor", 0).toInt());
    sciScintilla_->setUnmatchedBraceBackgroundColor(
        lls.value("UnmatchedBraceBackgroundColor", 10085887).toInt());


	connect(sciScintilla_, SIGNAL(modificationChanged(bool)), this, SLOT(onModificationChanged(bool)));
	connect(sciScintilla_, SIGNAL(copyAvailable(bool)), this, SIGNAL(copyAvailable(bool)));
    connect(sciScintilla_, SIGNAL(textChanged()), this, SIGNAL(textChanged()));
    connect(sciScintilla_, SIGNAL(marginClicked(int, int, Qt::KeyboardModifiers)),
            this, SLOT(setBookmark(int, int, Qt::KeyboardModifiers)));


	sciScintilla_->setMarginWidth(1, 14);			// margin 1 is breakpoint
    sciScintilla_->markerDefine(QsciScintilla::Circle, 2); //Marker 2 is breakpoint
	sciScintilla_->setMarginMarkerMask(1, 1 << 2);
    sciScintilla_->setMarginSensitivity(1, true);

	sciScintilla_->setMarginWidth(3, 14);			// margin 3 is bookmark margin
    sciScintilla_->markerDefine(QsciScintilla::RightTriangle, 1); // Marker 1 is bookmark
    sciScintilla_->setMarginMarkerMask(3, 1 << 1);
    sciScintilla_->setMarginSensitivity(3, true);

    sciScintilla_->markerDefine(QsciScintilla::Background, 3); //Marker 3 is current debug line

	sciScintilla_->registerImage(1,IconLibrary::instance().icon(0,"method").pixmap(16));
	sciScintilla_->registerImage(2,IconLibrary::instance().icon(0,"constant").pixmap(16));
	sciScintilla_->registerImage(3,IconLibrary::instance().icon(0,"event").pixmap(16));
	sciScintilla_->registerImage(4,IconLibrary::instance().icon(0,"class").pixmap(16));

    sciScintilla_->setMarkerForegroundColor(
                lls.value("MarkerForegroundColor", 2566178).toInt(), 1);
    sciScintilla_->setMarkerBackgroundColor(
                lls.value("MarkerBackgroundColor", 5348047).toInt(), 1);
    sciScintilla_->setMarkerForegroundColor(
                lls.value("MarkerForegroundColor", 2566178).toInt(), 2);
    sciScintilla_->setMarkerBackgroundColor(
                lls.value("MarkerBackgroundColor", 5348047).toInt(), 2);
    sciScintilla_->setMarkerBackgroundColor(
                lls.value("DebuggedLineColor", 0x3030FF).toInt(), 3);

	sciScintilla_->setEolMode(QsciScintilla::EolUnix);

    sciScintilla_->SendScintilla(QsciScintillaBase::SCI_SETMULTIPLESELECTION, true);
    // multi-line typing with Alt+drag/Alt_shift+cursor multi-line selections
    sciScintilla_->SendScintilla(QsciScintillaBase::SCI_SETADDITIONALSELECTIONTYPING, true);

    sciScintilla_->setAutoCompletionThreshold(settings.value(Keys::Prefs::autoCompleteChars, 2).toInt());
	sciScintilla_->setAutoCompletionSource(QsciScintilla::AcsAll);

    sciScintilla_->setFoldMarginColors(
                lls.value("FoldMarginFirstColor", 16777215).toInt(),
                lls.value("FoldMarginSecondColor", 10066329).toInt()
    );

    sciScintilla_->setIndentationGuidesForegroundColor(lls.value("IndentationGuidesForegroundColor", 0).toInt());
    sciScintilla_->setIndentationGuidesBackgroundColor(lls.value("IndentationGuidesBackgroundColor", 8421504).toInt());

    sciScintilla_->setIndicatorForegroundColor(lls.value("IndicatorForegroundColor", 0).toInt());
    sciScintilla_->setIndicatorOutlineColor(lls.value("IndicatorOutlineColor", 8421504).toInt());

    sciScintilla_->setMarginsForegroundColor(lls.value("MarginsForegroundColor", 2566178).toInt());
    sciScintilla_->setMarginsBackgroundColor(lls.value("MarginsBackgroundColor", 15658734).toInt());

    sciScintilla_->SendScintilla(QsciScintillaBase::SCI_SETMOUSEDWELLTIME,500);
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
        sciScintilla_->SendScintilla(QsciScintillaBase::SCI_SETKEYWORDS, (int) 0, (const char *)
                                     "et arreter faire sinon fin faux pour si "
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

	QsciLexer* lexer = createLexerByExtension(fileInfo.suffix());
	sciScintilla_->setLexer(lexer);

	setWindowTitle(fileInfo.fileName() + "[*]");

	QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
	QApplication::setOverrideCursor(Qt::WaitCursor);
	sciScintilla_->setUtf8(true);
	QString txt=in.readAll();
    if ((lexer!=NULL)&&(lexer->lexer()!=NULL)&&(!strcmp(lexer->lexer(),"lua"))&&txt.startsWith("!")&&(txt.length()>3))
        setLuaLanguage(txt.mid(1,2).toUpper());
    sciScintilla_->setText(txt);
	QApplication::restoreOverrideCursor();

	sciScintilla_->setModified(false);
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
	return sciScintilla_->hasSelectedText();
}

bool TextEdit::isRedoAvailable() const
{
	return sciScintilla_->isRedoAvailable();
}

bool TextEdit::isUndoAvailable() const
{
	return sciScintilla_->isUndoAvailable();
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
    while ((line = sciScintilla_->markerFindNext(line+1, (1 << 2)))>=0)
        breakpoints.insert(itemName_+":"+QString::number(line));

    if (sciScintilla_->isModified() == false)
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
	out << sciScintilla_->text();
	QApplication::restoreOverrideCursor();

	sciScintilla_->setModified(false);
	setWindowModified(false);

	return true;
}

void TextEdit::setCursorPosition(int line, int index)
{
	sciScintilla_->setCursorPosition(line, index);
}

bool TextEdit::isModified() const
{
	return sciScintilla_->isModified();
}

bool TextEdit::findFirst(	const QString &expr, bool re, bool cs, bool wo,
							bool wrap, bool forward/* = true*/)
{

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
}

bool TextEdit::replace(	const QString &expr, const QString &replaceStr, bool re, bool cs, bool wo,
						 bool wrap)
{
	bool forward = true;

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
}

int TextEdit::replaceAll(const QString &expr, const QString &replaceStr, bool re, bool cs, bool wo,
				bool wrap)
{
	sciScintilla_->beginUndoAction();

	bool forward = true;

	int line, index;
	sciScintilla_->getCursorPosition(&line, &index);

	int lineFrom, indexFrom, lineTo, indexTo;
	sciScintilla_->getSelection (&lineFrom, &indexFrom, &lineTo, &indexTo);

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
		bool found = sciScintilla_->findFirst(expr, re, cs, wo, wrap, forward, replaceline, replaceindex);
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

	sciScintilla_->endUndoAction();

	return replaced;
}

void TextEdit::setBookmark(int margin, int line, Qt::KeyboardModifiers state)
{
    Q_UNUSED(state);
    int marker=(margin==3)?1:2;
    if (sciScintilla_->markersAtLine(line) & (1 << marker))
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
	int line, index;
	sciScintilla_->getCursorPosition(&line, &index);

    if (sciScintilla_->markersAtLine(line) & (1 << 1))
		sciScintilla_->markerDelete(line, 1);
	else
		sciScintilla_->markerAdd(line, 1);
}

void TextEdit::nextBookmark()
{
	int line, index;
	sciScintilla_->getCursorPosition(&line, &index);

	int next = -1;
	
	next = sciScintilla_->markerFindNext(line + 1, (1 << 1));

	if (next < 0)
		next = sciScintilla_->markerFindNext(0, (1 << 1));

	if (next >= 0)
		sciScintilla_->setCursorPosition(next, 0);
}

void TextEdit::previousBookmark()
{
	int line, index;
	sciScintilla_->getCursorPosition(&line, &index);

	int prev = -1;
	
	if (line > 0)
		prev = sciScintilla_->markerFindPrevious(line - 1, (1 << 1));

	if (prev < 0)
		prev = sciScintilla_->markerFindPrevious(sciScintilla_->lines(), (1 << 1));

	if (prev >= 0)
		sciScintilla_->setCursorPosition(prev, 0);
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
    sciScintilla_->setIndentationsUseTabs(use_tabs);
}

void TextEdit::setIndentGuide(bool index)
{
    sciScintilla_->setIndentationGuides(index);
}

void TextEdit::setShowLineNumbers(bool show)
{
    sciScintilla_->setMarginLineNumbers(2, show);
}

void TextEdit::setBackspaceUnindents(bool use)
{
    sciScintilla_->setBackspaceUnindents(use);
}

void TextEdit::setWhitespaceVisibility(int mode)
{
    sciScintilla_->setWhitespaceVisibility(static_cast<QsciScintilla::WhitespaceVisibility>(mode));
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
	sciScintilla_->cancelList();
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
    sciScintilla_->setFocus();
}

void TextEdit::highlightDebugLine(int line) {
    sciScintilla_->markerDeleteAll(3);
    if (line>=0)
    {
        sciScintilla_->setCursorPosition(line, 0);
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
    QString text=sciScintilla_->text();
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
