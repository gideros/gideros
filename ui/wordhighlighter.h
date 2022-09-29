#ifndef WORDHIGHLIGHTER_H
#define WORDHIGHLIGHTER_H

#define IND_HIGHLIGHT_STYLE 9

#ifdef USE_SCINTILLAEDIT
#include <ScintillaEdit/ScintillaEdit.h>
#endif

#include <QSettings>
#include <QTimer>
#include "mainwindow.h"

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
    void setDelay(int delay);
    void reset();
    void resetUpdate();
    void update();
    MainWindow* getMainWindow();

private:
    int delay_;
    ScintillaEdit* editor_;
    QByteArray cachedWord;

    QTimer* updateTimer;
    bool enabled_;
    bool simpleMode_; // detect words ONLY at current cursor position
    bool filterWord(int style);
    bool extendBounds(sptr_t& selStart, sptr_t& selEnd);
private slots:
    void timerTimeout();
};

#endif // WORDHIGHLIGHTER_H
