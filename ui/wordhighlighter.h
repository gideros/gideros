#ifndef WORDHIGHLIGHTER_H
#define WORDHIGHLIGHTER_H

#define IND_HIGHLIGHT_STYLE 9
#define UPDATE_DELAY 1000

#ifdef USE_SCINTILLAEDIT
#include <ScintillaEdit/ScintillaEdit.h>
#endif

#include <QSettings>
#include <QTimer>

class WordHighlighter : public QObject
{
	Q_OBJECT
	
public:
	WordHighlighter(ScintillaEdit* editor, QSettings& settings);
	~WordHighlighter();
	
	bool simpleMode()
	{
		return simpleMode_;
	}
	
	bool enabled()
	{
		return enabled_;
	}
	
	void setSimpleMode(bool mode);
	void setEnabled(bool state);
	void reset();
	void resetUpdate();
	void update();
private:
	ScintillaEdit* editor_;
	QByteArray cachedWord;
	
	QTimer* updateTimer;
	bool enabled_;
	bool simpleMode_; // detect words ONLY at current cursor position
	bool filterWord(int style);
	bool extendBounds(sptr_t& selStart, sptr_t& selEnd);
private slots:
	void textAreaClicked(Scintilla::Position line, int modifiers);
	void timerTimeout();
};

#endif // WORDHIGHLIGHTER_H
