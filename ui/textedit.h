#ifndef MDICHILD_H
#define MDICHILD_H

#include <QMdiSubWindow>
#include <Qsci/qsciscintilla.h>
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

	QsciScintilla* sciScintilla() const
	{
		return sciScintilla_;
	}

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

    void setFocusToEdit();
    void highlightDebugLine(int line);

protected:
	virtual void closeEvent(QCloseEvent* event);

public slots:
	// bookmarks
    void setBookmark(int margin, int line, Qt::KeyboardModifiers state);
	void toogleBookmark();
	void nextBookmark();
	void previousBookmark();
	void clearBookmarks();

    // preferences
    void setTabWidth(int size);
    void setUseTabs(bool use_tabs);

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
    void dwellStart(int pos, int x, int y);
    void dwellEnd(int pos, int x, int y);

private:
	bool maybeSave();

private:
	QsciScintilla* sciScintilla_;
	bool isUntitled_;
    QString fileName_;
    QString itemName_;

public:
    static QSet<QString> breakpoints;
};

#endif // MDICHILD_H
