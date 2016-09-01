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
	bool loadFile(const QString& fileName, bool suppressErrors = false);
	const QString& fileName() const;
	bool save();

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

protected:
	virtual void closeEvent(QCloseEvent* event);

public slots:
	// bookmarks
    void setBookmark(int margin, int line, Qt::KeyboardModifiers state);
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

private slots:
	void onModificationChanged(bool m);

private:
	bool maybeSave();

private:
	QsciScintilla* sciScintilla_;
	bool isUntitled_;
	QString fileName_;
};

#endif // MDICHILD_H
