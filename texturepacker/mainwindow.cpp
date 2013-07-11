#include "mainwindow.h"
#include "librarytreewidget.h"
#include "optionswidget.h"
#include <QDockWidget>
#include <stack>
#include <QMessageBox>
#include <QCloseEvent>
#include "newprojectdialog.h"
#include <QTextStream>
#include <QFile>
#include <QSettings>
#include <QFileDialog>
#include <QSplitter>
#include "canvas.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
	ui.setupUi(this);

	optionsWidget_ = new OptionsWidget;
	ui.mainToolBar->addWidget(optionsWidget_);

	libraryDock_ = NULL;

#if 0
	libraryDock_ = new QDockWidget(tr("Library[*]"), this);
	libraryDock_->setObjectName("library");
	libraryWidget_ = new LibraryTreeWidget(libraryDock_);
	libraryDock_->setWidget(libraryWidget_);
	addDockWidget(Qt::RightDockWidgetArea, libraryDock_);
	connect(libraryWidget_, SIGNAL(modificationChanged(bool)), libraryDock_, SLOT(setWindowModified(bool)));

	canvas_ = new Canvas;
	setCentralWidget(canvas_);
#else
	libraryWidget_ = new LibraryTreeWidget;
	connect(libraryWidget_, SIGNAL(modificationChanged(bool)), this, SLOT(setLibraryWidgetModified(bool)));
	connect(libraryWidget_, SIGNAL(changed()), this, SLOT(onUpdateTexture()));

	QWidget* libraryContainer;
	{
		libraryContainer = new QWidget;
		libraryContainer->setLayout(new QVBoxLayout);
		libraryContainer->layout()->setMargin(0);
		libraryContainer->layout()->setSpacing(0);

		libraryLabel_ = new QLabel("Project");
		libraryLabel_->setMargin(2);
		libraryLabel_->setStyleSheet(
			"border: 1px solid #AAAAAA;"
			"background-color: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1, stop: 0 #FCFCFC, stop: 1 #E2E2E2);"
			);
		libraryContainer->layout()->addWidget(libraryLabel_);
		libraryContainer->layout()->addWidget(libraryWidget_);

		libraryWidget_->setLineWidth(0);
		libraryWidget_->setMidLineWidth(0);
		libraryWidget_->setFrameShape(QFrame::NoFrame);
		libraryWidget_->setFrameShadow(QFrame::Plain);
	}


	splitter1_ = new QSplitter;
	
	canvas_ = new Canvas;

	splitter1_->addWidget(libraryContainer);
	splitter1_->addWidget(canvas_);

	setCentralWidget(splitter1_);
#endif

	connect(optionsWidget_, SIGNAL(updateTexture()), this, SLOT(onUpdateTexture()));

	connect(ui.actionNew_Project, SIGNAL(triggered()), this, SLOT(newProject()));
	connect(ui.actionClose_Project, SIGNAL(triggered()), this, SLOT(closeProject()));
	connect(ui.actionSave_Project, SIGNAL(triggered()), this, SLOT(saveProject()));
	connect(ui.actionOpen_Project, SIGNAL(triggered()), this, SLOT(openProject()));


	connect(ui.actionProject1, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject2, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject3, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject4, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject5, SIGNAL(triggered()), this, SLOT(openRecentProject()));

	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));

	connect(ui.actionExport_Texture, SIGNAL(triggered()), this, SLOT(onExportTexture()));

	updateRecentProjectActions();

	updateUI();

	{
		QSettings settings;
		restoreGeometry(settings.value("geometry").toByteArray());
		restoreState(settings.value("windowState").toByteArray());
		splitter1_->restoreState(settings.value("splitter1").toByteArray());
	}
}

MainWindow::~MainWindow()
{

}

void MainWindow::setLibraryWidgetModified(bool modified)
{
	if (modified)
		libraryLabel_->setText("Project *");
	else
		libraryLabel_->setText("Project");
}


void MainWindow::onUpdateTexture()
{
	QDir path = QFileInfo(projectFileName_).dir();

	QDomDocument doc = libraryWidget_->toXml();

	std::stack<QDomNode> stack;
	stack.push(doc.documentElement());

	std::vector<QString> dir;

	QStringList fileNames;
	QStringList names;

	while (stack.empty() == false)
	{
		QDomNode n = stack.top();
		QDomElement e = n.toElement();
		stack.pop();

		if (n.isNull() == true)
		{
			dir.pop_back();
			continue;
		}

		QString type = e.tagName();

		if (type == "file")
		{
			QString fileName = e.attribute("file");

			QString name = QFileInfo(fileName).fileName();

			fileName = QDir::cleanPath(path.absoluteFilePath(fileName));

			QString n;
			for (std::size_t i = 0; i < dir.size(); ++i)
				n += dir[i] + "/";
			n += name;

			fileNames.push_back(fileName);
			names.push_back(n);
		}
		else if (type == "folder")
		{
			QString name = e.attribute("name");
			dir.push_back(name);
			stack.push(QDomNode());
		}

		QDomNodeList childNodes = n.childNodes();
		for (int i = 0; i < childNodes.size(); ++i)
			stack.push(childNodes.item(i));
	}

    QString msg = canvas_->packTextures(fileNames, names,
                                        optionsWidget_->padding(),
                                        optionsWidget_->extrude(),
                                        optionsWidget_->removeAlphaBorder(),
                                        optionsWidget_->alphaThreshold(),
                                        optionsWidget_->forceSquare(),
                                        optionsWidget_->showUnusedAreas());

    optionsWidget_->setText(msg);
//	printf("%s\n", qPrintable(canvas_->textureLocationList()));

	ProjectProperties& properties = libraryWidget_->getProjectProperties();

    properties.padding = optionsWidget_->padding();
    properties.extrude = optionsWidget_->extrude();
    properties.forceSquare = optionsWidget_->forceSquare();
	properties.removeAlphaBorder = optionsWidget_->removeAlphaBorder();
	properties.alphaThreshold = optionsWidget_->alphaThreshold();
	properties.showUnusedAreas = optionsWidget_->showUnusedAreas();
}

void MainWindow::newProject()
{
	if (maybeSave())
	{
		NewProjectDialog* newProjectDialog = new NewProjectDialog(this);
		if (newProjectDialog->exec() == QDialog::Accepted)
		{
			// try to create directory
			QString dir = newProjectDialog->fullDirectory();
			if (QDir().mkpath(dir) == false)
			{
				QMessageBox::information(this, "Information", "Could not create the directory: " + dir);
				return;
			}

			// try to create project
			QString fileName = newProjectDialog->fullName();
			QFile file(fileName);
			if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
			{
				QMessageBox::information(this, "Information", "Could not create the project: " + fileName);
				return;
			}

			closeProject(0);

			libraryWidget_->newProject(fileName);

			QTextStream(&file) << libraryWidget_->toXml().toString();

			file.close();

			projectFileName_ = fileName;
//			libraryWidget_->setProjectFileName(projectFileName_);

			updateUI();

			setWindowTitle(QFileInfo(projectFileName_).fileName() + "[*] - " + tr("Texture Packer"));

			QSettings settings;
			settings.setValue("location", newProjectDialog->location());

			addToRecentProjects(projectFileName_);
		}
	}
}

void MainWindow::openProject()
{
	if (maybeSave())
	{
		QSettings settings;

		QString dir = settings.value("lastOpenProjectDir", QString()).toString();

		QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
			dir,
			tr("Texture Packer Project (*.tpproj)"));

		if (fileName.isEmpty() == true)
			return;

		QDir newdir = QFileInfo(fileName).absoluteDir();
		newdir.cdUp();
		settings.setValue("lastOpenProjectDir", newdir.absolutePath());

		openProject(fileName);
	}
}

void MainWindow::saveProject()
{
	QFile file(projectFileName_);

	if (file.open(QIODevice::WriteOnly | QIODevice::Text) == false)
	{
		QMessageBox::information(this, "Information", "Could not save the project file: " + projectFileName_);
		return;
	}

	QTextStream(&file) << libraryWidget_->toXml().toString();
	file.close();
	libraryWidget_->setWindowModified(false);
}

void MainWindow::closeProject()
{
	if (maybeSave())
		closeProject(0);
}

void MainWindow::closeProject(int)
{
	projectFileName_ = "";

	libraryWidget_->clear();

	updateUI();

	setWindowTitle(tr("Texture Packer"));

	optionsWidget_->setText("");
	canvas_->clear();
}

bool MainWindow::maybeSave()
{
	if (libraryWidget_->isWindowModified())
	{
		QMessageBox msgBox;
		msgBox.setText("The project has been modified.");
		msgBox.setInformativeText("Do you want to save your changes?");
		msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
		msgBox.setDefaultButton(QMessageBox::Yes);
		int ret = msgBox.exec();

		switch (ret)
		{
			case QMessageBox::Yes:
				saveProject();
				return true;
			case QMessageBox::No:
				return true;
			case QMessageBox::Cancel:
				return false;
		}
	}

	return true;
}


void MainWindow::closeEvent(QCloseEvent* event)
{
	if (maybeSave())
	{
		QSettings settings;
		settings.setValue("geometry", saveGeometry());
		settings.setValue("windowState", saveState());
		settings.setValue("splitter1", splitter1_->saveState());

		event->accept();
	} 
	else
	{
		event->ignore();
	}
}

void MainWindow::updateUI()
{
	bool isProjectOpen = projectFileName_.isEmpty() == false;
//	libraryWidget_->setEnabled(isProjectOpen);
	optionsWidget_->setEnabled(isProjectOpen);
	optionsWidget_->updateUI(libraryWidget_->getProjectProperties());
	ui.actionSave_Project->setEnabled(isProjectOpen);
	ui.actionClose_Project->setEnabled(isProjectOpen);
	ui.actionExport_Texture->setEnabled(isProjectOpen);
}

void MainWindow::openProject(const QString& fileName)
{
	QDomDocument doc;
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly))
	{
		QMessageBox::information(this, "Information", "Could not open the project file: " + fileName);
		return;
	}

	if (!doc.setContent(&file))
	{
		QMessageBox::information(this, "Information", "Could not parse the project file: " + fileName);
		file.close();
		return;
	}
	file.close();

	closeProject(0);

	projectFileName_ = fileName;

	libraryWidget_->loadXml(projectFileName_, doc);

	updateUI();
	setWindowTitle(QFileInfo(projectFileName_).fileName() + "[*] - " + tr("Texture Packer"));
	addToRecentProjects(projectFileName_);

	onUpdateTexture();
}



#define MAX_RECENT_FILES 5
void MainWindow::addToRecentProjects(const QString& fileName)
{
	QSettings settings;
	QStringList files = settings.value("recentProjectList").toStringList();
	files.removeAll(fileName);
	files.prepend(fileName);
	while (files.size() > MAX_RECENT_FILES)
		files.removeLast();
	settings.setValue("recentProjectList", files);

	updateRecentProjectActions();
}

void MainWindow::updateRecentProjectActions()
{
	QAction* recentProjectActions[MAX_RECENT_FILES];
	recentProjectActions[0] = ui.actionProject1;
	recentProjectActions[1] = ui.actionProject2;
	recentProjectActions[2] = ui.actionProject3;
	recentProjectActions[3] = ui.actionProject4;
	recentProjectActions[4] = ui.actionProject5;

	QSettings settings;
	QStringList files = settings.value("recentProjectList").toStringList();

	int numRecentFiles = qMin(files.size(), MAX_RECENT_FILES);

	for (int i = 0; i < numRecentFiles; ++i)
	{
		QString text = tr("&%1 %2").arg(i + 1).arg(files[i]);
		recentProjectActions[i]->setText(text);
		recentProjectActions[i]->setData(files[i]);
		recentProjectActions[i]->setVisible(true);
	}

	for (int i = numRecentFiles; i < MAX_RECENT_FILES; ++i)
		recentProjectActions[i]->setVisible(false);
}

void MainWindow::openRecentProject()
{
	QAction *action = qobject_cast<QAction *>(sender());

	if (action != 0)
	{
		if (maybeSave())
		{
			openProject(action->data().toString());
		}
	}
}

void MainWindow::onExportTexture()
{
	QFileInfo fileInfo(projectFileName_);

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"),
		fileInfo.dir().absoluteFilePath(fileInfo.completeBaseName() + ".png"),
		tr("PNG File (*.png)"));

	if (fileName.isEmpty() == false)
	{
		canvas_->texture().save(fileName);

		// export texture location list as .txt file
		QFileInfo fileInfo(fileName);
		QString txtFileName = fileInfo.dir().absoluteFilePath(fileInfo.completeBaseName() + ".txt");

		QFile file(txtFileName);
		if (file.open(QFile::WriteOnly | QFile::Truncate | QIODevice::Text))
		{
			QTextStream out(&file);
			out << canvas_->textureLocationList();
		}
	}
}
