#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>

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
	void timerEvent(QTimerEvent *);

private:
	static void s_callback(int, int, void*, size_t, void*);
	void callback(int, int, void*, size_t);

private:
    Ui::MainWindow *ui;
	std::vector<int> requests_;
};

#endif // MAINWINDOW_H
