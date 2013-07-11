#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ui_mainwindow.h"
#include <deque>
#include <QLocalServer>

class LibraryWidget;
class GiderosNetworkClient;
class QDockWidget;
class QMouseEvent;


namespace Ui {
    class MainWindow;
}


#include <QTextEdit>

class QTextEditEx : public QTextEdit
{
	Q_OBJECT

public:
	QTextEditEx(QWidget* parent = 0) : QTextEdit(parent) {}
	QTextEditEx(const QString& text, QWidget* parent = 0) : QTextEdit(text, parent) {}

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
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


private:
	bool maybeSave();
	void updateUI();
	QString projectName() const;

	void addToRecentProjects(const QString& fileName);
	void updateRecentProjectActions();
	std::vector<std::pair<std::string, std::string> > libraryFileList();

	QTextEditEx* outputWidget_;
	QDockWidget* outputDock_;

	void openFile(const QString& fileName, int line = -1);

private slots:
	void start();
	void stop();
	void startPlayer();

	void newProject();
	void openProject();
	void openProject(const QString& fileName);
	void saveProject();
	void closeProject();
	void openRecentProject();

	void playerSettings();

	void exportProject();

	void fileAssociations();

	void onOpenRequest(const QString& itemName, const QString& fileName);

	void outputMouseDoubleClick(QMouseEvent* e);

	void connected();
	void disconnected();
	void dataReceived(const QByteArray& d);
	void ackReceived(unsigned int id);

	void onTimer();

private:
	LibraryWidget* libraryWidget_;

private:
	QString projectFileName_;
	GiderosNetworkClient* client_;
	std::deque<QPair<QString, QString> > fileQueue_;
	bool isTransferring_;

private:
	QList<QStringList> fileAssociations_;

private:
    Ui::MainWindow ui;

protected:
	virtual void closeEvent(QCloseEvent* event);

private:
	QLocalServer localServer_;
	QList<QLocalSocket*> localSockets_;
private slots:
	void onLocalServerNewConnection();
	void onLocalSocketReadyRead();
};

#endif // MAINWINDOW_H
