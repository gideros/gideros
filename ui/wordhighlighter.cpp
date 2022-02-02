#include "wordhighlighter.h"
#include <Lexilla/SciLexer.h>

static int rcolor(int c) 
{
    return( (c&0xFF)<<16)|(c&0xFF00FF00)|((c&0xFF0000)>>16);
}

WordHighlighter::WordHighlighter(ScintillaEdit* editor, QSettings& settings):
    cachedPos(0),    
    cachedSelectionStart(0),
    cachedSelectionEnd(0),
    enabled(true),
    simpleMode(true)
{
    editor_ = editor;
    updateTimer = new QTimer(this);
    
    editor_->indicSetStyle(IND_HIGHLIGHT_STYLE, 
        settings.value("WordHighlightStyle", INDIC_ROUNDBOX).toInt());
    editor_->indicSetAlpha(IND_HIGHLIGHT_STYLE, 
        settings.value("WordHighlightAlpha", 100).toInt());
    editor_->indicSetOutlineAlpha(IND_HIGHLIGHT_STYLE, 
        settings.value("WordHighlightOutlineAlpha", 255).toInt());
    editor_->indicSetStrokeWidth(IND_HIGHLIGHT_STYLE, 
        settings.value("WordHighlightStrokeWidth", 2).toInt());
    editor_->indicSetFore(IND_HIGHLIGHT_STYLE, 
        rcolor(settings.value("WordHighlightColor", 23204).toInt()));
    editor_->indicSetUnder(IND_HIGHLIGHT_STYLE, false);
    
    connect(editor_, SIGNAL(textAreaClicked(Scintilla::Position, int)), this, SLOT(textAreaClicked(Scintilla::Position, int)));
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(timerTimeout()));
    
    updateTimer->start(UPDATE_DELAY);
}

WordHighlighter::~WordHighlighter()
{
    delete updateTimer;
}


void WordHighlighter::textAreaClicked(Scintilla::Position line, int modifiers)
{
    Q_UNUSED(line);
    Q_UNUSED(modifiers);
    
    updateTimer->start(UPDATE_DELAY);
}

void WordHighlighter::timerTimeout()
{
    run();
}

void WordHighlighter::setSimpleMode(bool mode)
{
    simpleMode = mode;
    reset();
    resetCache();
    updateTimer->start();
}

void WordHighlighter::setEnabled(bool state)
{
    enabled = state;
    if (!state)
	{
		reset();
		updateTimer->stop();
	}
    else
	{
		updateTimer->start();
	}
}

void WordHighlighter::reset()
{
    editor_->setIndicatorCurrent(IND_HIGHLIGHT_STYLE);
    const sptr_t textLength = editor_->textLength();
    editor_->indicatorClearRange(0, textLength);
}

void WordHighlighter::resetCache()
{
    cachedPos = 0;
    cachedSelectionStart = 0;
    cachedSelectionEnd = 0;
}

bool WordHighlighter::extendBounds(sptr_t& selStart, sptr_t& selEnd)
{    
    if (selStart == selEnd) 
    {
        // Try and find a word at the caret
        // On the left...
        selStart = editor_->wordStartPosition(selStart, true);
        // and on the right
        selEnd = editor_->wordEndPosition(selEnd, true);
    }
    else
    {
        // anyWord1:anyWord2 - text to check
        
        sptr_t currentPos = editor_->currentPos();
        if (currentPos == selStart)
        {
            // | - current cursor position
            // ] - selection end
            sptr_t rightPos = editor_->wordEndPosition(selEnd, true);
            // anyWord1:a|nyWor]d2  OK
            if (editor_->wordEndPosition(currentPos, true) == rightPos)
            {
                selEnd = rightPos;
                selStart = editor_->wordStartPosition(currentPos, true);
            }
            // anyW|ord1:anyWor]d2  NOT OK
            else
                return false;
        }
        else
        {
            // | current cursor position
            // [ selection start
            sptr_t leftPos = editor_->wordStartPosition(selStart, true);
            // anyWord1:a[nyWor|d2  OK
            if (editor_->wordStartPosition(currentPos, true) == leftPos)
            {
                selEnd = editor_->wordEndPosition(currentPos, true);
                selStart = leftPos;
            }
            // anyW[ord1:anyWor|d2  NOT OK
            else
                return false;
        }
    }
    
    return true;
}

bool WordHighlighter::filterWord(int style)
{
    switch (style) 
    {
        case SCE_LUA_DEFAULT:
		case SCE_LUA_COMMENT:
		case SCE_LUA_COMMENTLINE:
		case SCE_LUA_COMMENTDOC:
		case SCE_LUA_NUMBER:
		case SCE_LUA_STRING:
		case SCE_LUA_CHARACTER:
		case SCE_LUA_LITERALSTRING:
		case SCE_LUA_PREPROCESSOR:
		case SCE_LUA_OPERATOR:
		case SCE_LUA_STRINGEOL:
		case SCE_LUA_LABEL:
        case SCE_LUA_WORD:
        case SCE_LUA_WORD5:
		case SCE_LUA_WORD6:
		case SCE_LUA_WORD7:
		case SCE_LUA_WORD8:
            return false;
        case SCE_LUA_IDENTIFIER:
        case SCE_LUA_WORD2:
        case SCE_LUA_WORD3:
		case SCE_LUA_WORD4:
            return true;
        default:
            return false;
    }
}

void WordHighlighter::run()
{   
    if (!enabled)
        return;
    
    updateTimer->start(UPDATE_DELAY);
    
    sptr_t currentPos = editor_->currentPos();    
    sptr_t selStart = editor_->selectionStart();
    sptr_t selEnd = editor_->selectionEnd();
    
    // Do not update if cursor not moved?
    if (simpleMode)
    {
        if (cachedPos == currentPos) 
            return;
                
    }
    else if (cachedSelectionStart == selStart && cachedSelectionEnd == selEnd)
    {
        return;
    }
    
    cachedPos = currentPos;
    cachedSelectionStart = selStart;
    cachedSelectionEnd = selEnd;
    
    reset();
    
    const sptr_t textLength = editor_->textLength();
    
    // Allow hightlight when using multiple cursors only in simple mode 
    if (editor_->selections() > 1 && !simpleMode)
    {
        return;
    }
    
    // Find symbol under the cursor
    if (simpleMode)
    {
        sptr_t lastCursorPos = editor_->selectionNCaret(editor_->selections() - 1);
        selStart = editor_->wordStartPosition(lastCursorPos, true);
        selEnd = editor_->wordEndPosition(lastCursorPos, true);
    }
    else 
    {    
        // Extend to left & right to find a word
        if (!extendBounds(selStart, selEnd))
        {
            return;
        }
    }
    
    QByteArray wordToFind = editor_->textRange(selStart, selEnd);
    auto wordLength = wordToFind.length();
    
    // No highlight when no selection or multi-lines selection.
    if (wordLength == 0 || wordToFind.contains('\n') || wordToFind.contains('\r'))
    {
        return;
    }
    
    editor_->setIndicatorCurrent(IND_HIGHLIGHT_STYLE);
    
    // Case sensitive & whole word only.
    editor_->setSearchFlags(SCFIND_MATCHCASE | SCFIND_WHOLEWORD);
    editor_->setTargetRange(0, textLength);
    const char* cstr = wordToFind.constData();
    
    qDebug("Update highlighter, style: %lld", editor_->styleAt(selStart));
    
    sptr_t index = editor_->searchInTarget(wordLength, cstr);
    while (index != -1)
    {
        sptr_t tEnd = editor_->targetEnd();
        if (filterWord(editor_->styleAt(index)))
        {
            sptr_t tStart = editor_->targetStart();
            editor_->indicatorFillRange(tStart, tEnd - tStart);
        }
        editor_->setTargetRange(tEnd, textLength);
        
        index = editor_->searchInTarget(wordLength, cstr);
    }
}
