#include "mainwindow.h"
#include <QFileDialog>
#include "optionswidget.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	ui.setupUi(this);

	optionsWidget_ = new OptionsWidget;
	ui.mainToolBar->addWidget(optionsWidget_);

	ui.fontCanvas->setFontSize(optionsWidget_->fontSize());
	ui.fontCanvas->setChars(optionsWidget_->chars());

    connect(ui.actionImport_Font, SIGNAL(triggered()), this, SLOT(importFont()));
    connect(ui.actionExport_Font, SIGNAL(triggered()), this, SLOT(exportFont()));

	connect(optionsWidget_, SIGNAL(fontSize_valueChanged(int)), ui.fontCanvas, SLOT(setFontSize(int)));
	connect(optionsWidget_, SIGNAL(chars_textChanged(const QString&)), ui.fontCanvas, SLOT(setChars(const QString&)));
}

MainWindow::~MainWindow()
{

}


void MainWindow::importFont()
{
	fontFile_ = QFileDialog::getOpenFileName(this, tr("Open"), QString(), tr("Font Files (*.ttf *.otf *.ttc)"));

	if (fontFile_.isEmpty() == false)
		ui.fontCanvas->setFontFile(fontFile_);
}


void MainWindow::exportFont()
{
	QFileInfo fileInfo(fontFile_);

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
													fileInfo.dir().absoluteFilePath(fileInfo.completeBaseName() + ".png"),
													tr("PNG File (*.png)"));

	if (fileName.isEmpty() == false)
	{
		ui.fontCanvas->exportFont(fileName);
	}
}
