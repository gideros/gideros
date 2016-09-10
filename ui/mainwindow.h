#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include <QImage>
#if 0
#include "compilethread.h"
#endif
#include <QList>
#include <deque>
#include <Qsci/qsciscintilla.h>
#include <QFontMetrics>
#include <QListWidget>

#define NEW_CLIENT

class QAction;
class LibraryWidget;
class Client;
class QTextEdit;
class QMdiArea;
class TextEdit;
class QsciScintilla;
class FindDialog;
class ReplaceDialog;
class GiderosNetworkClient;
class PreviewWidget;
class FindInFilesDialog;
class QSplitter;
class QProcess;
class QComboBox;

#include "mdiarea.h"

#include <QTextEdit>

class QTextEditEx : public QTextEdit
{
	Q_OBJECT

private:
	QTextCharFormat df_;
    bool found;

public:
	QTextEditEx(QWidget* parent = 0) : QTextEdit(parent)
	{
#ifdef Q_OS_MAC
		QFont f("Monaco", 10);
#else
		QFont f("Courier New", 8);
#endif
		QFontMetrics fm(f);
		setTabStopWidth(fm.width("1234"));
		df_.setFont(f);

        found = false;

//		setAcceptRichText(true);
//		setTextColor(Qt::red);
		//setFont(QFont("Courier New", 10));
		//setTabWidth(4);
		//setIndentationsUseTabs(true);
	}

	void moveCursorEnd()
	{
		moveCursor(QTextCursor::End);
	}

public slots:
    void search(const QString& searchString){
        QTextDocument *document = this->document();

        if (found){
            document->undo();
            found = false;
        }

        QTextCursor highlightCursor(document);
        QTextCursor cursor(document);

        cursor.beginEditBlock();

        QTextCharFormat plainFormat(highlightCursor.charFormat());
        QTextCharFormat colorFormat = plainFormat;
        colorFormat.setFontWeight(75);
        colorFormat.setForeground(Qt::white);
        colorFormat.setBackground(Qt::red);

        while (!highlightCursor.isNull() && !highlightCursor.atEnd()) {
            highlightCursor = document->find(searchString, highlightCursor, QTextDocument::FindWholeWords);

            if (!highlightCursor.isNull()) {
                highlightCursor.movePosition(QTextCursor::WordRight, QTextCursor::KeepAnchor);
                highlightCursor.mergeCharFormat(colorFormat);
                found = true;
            }
        }

        cursor.endEditBlock();
    }

	void append(const QString& text)
	{
		moveCursor(QTextCursor::End);
		textCursor().insertText(text, df_);
	}

	void append(const QString& text, const QTextCharFormat & format)
	{
		moveCursor(QTextCursor::End);
		textCursor().insertText(text, format);
	}

	void append(const QString& str, const QList<QPair<int, int> >& highlight)
	{
		QTextCharFormat f1 = df_;
		f1.setForeground(Qt::red);
		f1.setBackground(QColor(0xFF, 0xFF, 0xBF));

		int curr = 0;
		for (int i = 0; i < highlight.size(); ++i)
		{
			int start = highlight[i].first;
			int end = highlight[i].second;

			QString s1 = str.mid(curr, start - curr);
			QString s2 = str.mid(start, end - start);

			append(s1, df_);
			append(s2, f1);

			curr = end;
		}
		QString s3 = str.mid(curr);
		append(s3, df_);
    }

signals:
	void mouseDoubleClick(QMouseEvent* e);

protected:
	virtual void mouseDoubleClickEvent(QMouseEvent* e)
	{
		QTextEdit::mouseDoubleClickEvent(e);
		emit mouseDoubleClick(e);
	}
};


class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
	~MainWindow();

private:
	virtual void timerEvent(QTimerEvent*);
	virtual void closeEvent(QCloseEvent* event);

    virtual void keyPressEvent(QKeyEvent * event);
    virtual void keyReleaseEvent(QKeyEvent * event);

private slots:
	void onSingleShot();
	void onTimer();
	void start();
	void stop();
	void startAllPlayers();
	void newProject();
	void closeProject();
	void saveProject();
	void openProject();
	void openProject(const QString& fileName);
    void reloadProject();
//	void onModified();
	void startPlayer();
	void fileAssociations();
	void exportProject();
	void exportPack();

	void save();
	void saveAll();
	bool maybeSave();

	void compile();
	void compileAll();
	void cancel();

	void playerSettings();

	void showStartPage();
	void hideStartPage();

	void openAboutDialog();
	void developerCenter();
	void helpAndSupport();
	void apiDocumentation();
    void giderosDocumentation();

	void closeMdiTab(int);

private slots:
	void onOpenRequest(const QString& itemName, const QString& fileName);
	void onPreviewRequest(const QString& itemName, const QString& fileName);
	void updateUI();
	void onInsertIntoDocument(const QString& text);

private:
	TextEdit* openFile(const QString& fileName, bool suppressErrors = false);

private slots:
	void outputMouseDoubleClick(QMouseEvent* e);

private:
	TextEdit* createTextEdit();
	TextEdit* findTextEdit(const QString& fileName) const;

private:
	std::vector<QString> allPlayersPlayList;
	bool prepareStartOnPlayer();
	void startOnPlayer();
	void startNextPlayer();
	void playStarted();
	unsigned int sendPlayMessage(const QStringList& luafiles);
	unsigned int sendStopMessage();

	Ui::MainWindowClass ui;

	LibraryWidget* libraryWidget_;
	QTextEditEx* outputWidget_;
	PreviewWidget* previewWidget_;
	QComboBox *players_;
#ifndef NEW_CLIENT
	Client* client_;
#else
	GiderosNetworkClient* client_;
#endif
//	std::map<int, QString> sentMap_;
	std::deque<QPair<QString, QString> > fileQueue_;

	void cancelUpload();
	bool isTransferring_;


private:
//	void enableUI();
//	void disableUI();

private:
	QDockWidget* libraryDock_;
	QDockWidget* outputDock_;
	QDockWidget* previewDock_;
	MdiArea* mdiArea_;

	QSplitter* splitter1_;
	QSplitter* splitter2_;
	QSplitter* splitter3_;

private:
	QString projectName() const;
	QString projectDirectory() const;
	QString projectFileName_;
    QString makeProjectRelativePath(const QString& path) const;

#if 0
private slots:
	void compileThreadFinishedOrTerminated();
	void compileFinished(bool success);

private:
	CompileThread* compileThread_;
	CompileThread* createCompileAllThread() const;
#endif

private:
	void addToRecentProjects(const QString& fileName);
	void updateRecentProjectActions();

private slots:
	void openRecentProject();

private:
	FindDialog* findDialog_;
	ReplaceDialog* replaceDialog_;
	FindInFilesDialog* findInFilesDialog_;
	QString findWhat_;
	bool wholeWord_;
	bool matchCase_;
    bool wrapSearch_;
    bool regexp_;

    bool inChooseTab_;
    int changeTabKeyPressState_;
    QListWidget* tabListWidget_;
private:
	QList<QStringList> fileAssociations_;

	
private slots:
	void find();
	void findFirst();
	void findNext();
	void findPrevious();
	void replace();

	void replace_findNext();
	void replace_replace();
	void replace_replaceAll();

	void findInFiles();

    void actionLocalhostToggle(bool checked);
    void clearOutput();
    void searchOutput( const QString &text);

private slots:
	void toogleBookmark();
	void nextBookmark();
	void previousBookmark();
	void clearBookmarks();

private slots:
	void undo();
	void redo();

private slots:
	void goToLine();

#ifndef NEW_CLIENT

#else
private slots:
	void connected();
	void disconnected();
	void dataReceived(const QByteArray& data);
	void ackReceived(unsigned int id);
    void advertisement(const QString&,unsigned short,unsigned short,const QString&);

#endif

private slots:
	void playerChanged(const QString & text);
	void showOutputPanel();
	void showLibraryManager();

private:
	std::vector<std::pair<QString, QString> > libraryFileList(bool downsizing = false);

private:
/*	struct TimedMD5
	{
		time_t mtime;
		QByteArray md5;
//		unsigned char md5[16];
	}; */

	QMap<QString, QPair<qint64, QByteArray> > md5_;

	void loadMD5();
	void saveMD5();
	std::vector<std::pair<QString, QString> > updateMD5(bool downsizing = false);

private slots:
	void downsize(const QString& filename);

private:
	void storeOpenFiles();
	void restoreOpenFiles();

private:
    QProcess *makeProcess_;
    QProcess *luaProcess_;
private slots:
    void stdoutToOutput();
    void stderrToOutput();
    void makeStarted();
    void makeFinished();
    void on_actionUI_Theme_triggered();
    void on_actionEditor_Theme_triggered();
    void on_actionUI_and_Editor_Theme_triggered();
    void on_actionReset_UI_and_Editor_Theme_triggered();
    void on_actionFold_Unfold_All_triggered();
    void on_actionFold_Unfold_Top_triggered();
};

#endif // MAINWINDOW_H
