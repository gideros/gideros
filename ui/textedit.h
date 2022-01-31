#ifndef MDICHILD_H
#define MDICHILD_H

#include <QMdiSubWindow>
#ifdef USE_SCINTILLAEDIT
#include <ScintillaEdit/ScintillaEdit.h>
#else
#include <Qsci/qsciscintilla.h>
#endif
#include "mdisubwindow.h"

class TextEdit : public MdiSubWindow
{
	Q_OBJECT

public:
	TextEdit(QWidget *parent = 0);
	~TextEdit();

	void newFile();
    bool loadFile(const QString& fileName, const QString &itemName, bool suppressErrors = false);
	const QString& fileName() const;
	bool save();
	virtual void background();
#ifdef SCINTILLAEDIT_H
    ScintillaEdit* sciScintilla() const
    {
        return sciScintilla_;
    }
    void setIdentifiers(const QStringList &ilist);
#else
	QsciScintilla* sciScintilla() const
	{
		return sciScintilla_;
	}
#endif
	bool hasSelectedText() const;
	bool isRedoAvailable() const;
	bool isUndoAvailable() const;

	void setCursorPosition(int line, int index);

	bool isModified() const;

    bool findFirst(	const QString &expr, bool re, bool cs, bool wo,
					bool wrap, bool forward = true);

	bool replace(	const QString &expr, const QString &replaceStr, bool re, bool cs, bool wo,
					bool wrap);

	int replaceAll(const QString &expr, const QString &replaceStr, bool re, bool cs, bool wo,
					bool wrap);

    void BlockComment();

    void setFocusToEdit();
    void highlightDebugLine(int line);

    // preferences
    void setTabWidth(int size);
    void setUseTabs(bool use_tabs);
    void setIndentGuide(bool index);
    void setShowLineNumbers(bool show);
    void setBackspaceUnindents(bool use);
    void setWhitespaceVisibility(int mode);
	void setCompactFolding(int mode);

protected:
	virtual void closeEvent(QCloseEvent* event);
    void setLuaLanguage(QString lang);

public slots:
	// bookmarks
#ifdef SCINTILLAEDIT_H
    void setBookmark(Scintilla::Position position, Scintilla::KeyMod modifiers, int margin);
#else
    void setBookmark(int margin, int line, Qt::KeyboardModifiers state);
#endif
	void toogleBookmark();
	void nextBookmark();
	void previousBookmark();
	void clearBookmarks();

	// undo-redo
	void undo();
	void redo();

signals:
	void copyAvailable(bool yes);
    void textChanged();
    void marginClicked(int, int, Qt::KeyboardModifiers);
    void lookupSymbol(QString,int,int);

private slots:
    void onModificationChanged(bool m);
#ifdef SCINTILLAEDIT_H
    void updateUi(Scintilla::Update updated);
    void charAdded(int ch);
    void dwellStart(int x, int y);
    void dwellEnd(int x, int y);
    void callTipClick(Scintilla::Position);
#else
    void dwellStart(int pos, int x, int y);
    void dwellEnd(int pos, int x, int y);
#endif

private:
	bool maybeSave();

private:
#ifdef SCINTILLAEDIT_H
    ScintillaEdit* sciScintilla_;
    bool modified;
    int autoCompleteThreshold;
    QStringList api;
    QStringList autocIdentifiers;
    QStringList currentCallTipList;
    size_t currentCallTipIndex;
    size_t currentCallTipPos;
    void registerIcon(int num,QIcon icon);
#else
    QsciScintilla* sciScintilla_;
#endif
	bool isUntitled_;
    QString fileName_;
    QString itemName_;

public:
    static QSet<QString> breakpoints;
};

#endif // MDICHILD_H
