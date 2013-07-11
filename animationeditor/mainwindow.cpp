#include <platform.h>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "timelinewidget.h"
#include "canvaswidget.h"
#include <QUndoStack>
#include "controller.h"
#include "document.h"
#include <QTimer>
#include <QSplitter>
#include <QScrollArea>
#include <QUndoView>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{	
	setDocumentsDirectory(".");
	setResourceDirectory(".");
	setTemporaryDirectory(".");

	ui->setupUi(this);

	undoStack_ = new QUndoStack;

	layer_ = new LayerWidget;
	timeline_ = new TimelineWidget();

	canvas_ = new CanvasWidget();

	document_ = new Document;

	controller_ = new Controller(undoStack_, timeline_, canvas_, layer_, document_);

	timeline_->setController(controller_);
	canvas_->setController(controller_);
	layer_->setController(controller_);

	QSplitter* hsplitter = new QSplitter(Qt::Horizontal);
	QUndoView* undoview = new QUndoView(undoStack_);

	QSplitter* splitter = new QSplitter(Qt::Vertical);
	QScrollArea* scrollArea = new QScrollArea;
	scrollArea->setWidget(timeline_);

	QSplitter* tsplitter = new QSplitter(Qt::Horizontal);
	tsplitter->addWidget(layer_);
	tsplitter->addWidget(scrollArea);

	splitter->addWidget(tsplitter);
	splitter->addWidget(canvas_);

	hsplitter->addWidget(splitter);
	hsplitter->addWidget(undoview);

	setCentralWidget(hsplitter);

	undoAction_ = undoStack_->createUndoAction(this, tr("&Undo"));
	undoAction_->setShortcuts(QKeySequence::Undo);

	redoAction_ = undoStack_->createRedoAction(this, tr("&Redo"));
	redoAction_->setShortcuts(QKeySequence::Redo);

	ui->actionDelete->setShortcut(QKeySequence::Delete);
	ui->actionCopy->setShortcut(QKeySequence::Copy);
	ui->actionPaste->setShortcut(QKeySequence::Paste);

	ui->menuEdit->addAction(undoAction_);
	ui->menuEdit->addAction(redoAction_);

	QTimer::singleShot(0, this, SLOT(initView()));
}

MainWindow::~MainWindow()
{
	delete controller_;
	delete document_;
	delete undoStack_;
    delete ui;
}

void MainWindow::initView()
{
	controller_->load();
}


void MainWindow::on_actionDelete_triggered()
{
	controller_->deleteSelected();
}

void MainWindow::on_actionCopy_triggered()
{
	controller_->copySelected();
}

void MainWindow::on_actionPaste_triggered()
{
	controller_->pasteClipboard();
}
