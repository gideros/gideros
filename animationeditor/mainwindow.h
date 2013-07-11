#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class TimelineWidget;
class CanvasWidget;
class QUndoStack;
class Controller;
class Document;
class LayerWidget;

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

	TimelineWidget* timeline_;
	CanvasWidget* canvas_;
	LayerWidget* layer_;

	QUndoStack* undoStack_;

	Controller* controller_;

	Document* document_;

private slots:
	void initView();
	void on_actionDelete_triggered();
	void on_actionCopy_triggered();
	void on_actionPaste_triggered();

private:
	QAction* undoAction_;
	QAction* redoAction_;
};

#endif // MAINWINDOW_H
