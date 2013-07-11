#include "mainwindow.h"
#include "iconlibrary.h"
#include "librarywidget.h"
#include <QMessageBox>
#include "newprojectdialog.h"
#include <QTextStream>
#include <QFileInfo>
#include <QSettings>
#include <QFileDialog>
#include <QDesktopServices>
#include <QUrl>
#include "playersettingsdialog.h"
#include "giderosnetworkclient.h"
#include "bytebuffer.h"
#include <stack>
#include <QProcess>
#include <QTimer>
#include <QDockWidget>
#include "fileassociationsdialog.h"
#include <QProgressDialog>

#include <sys/stat.h>
#include <time.h>

#include <iostream>

#include <QtDebug>

#include <QTextCursor>
#include <QTextBlock>
#include <QMouseEvent>

#include <QLocalSocket>

static time_t fileLastModifiedTime(const char* file)
{
	struct stat s;
	stat(file, &s);

	return s.st_mtime;
}

static time_t fileAge(const char* file)
{
	return time(NULL) - fileLastModifiedTime(file);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
	ui.setupUi(this);

	QSettings settings;

	libraryWidget_ = new LibraryWidget;
	setCentralWidget(libraryWidget_);

	{
		outputDock_ = new QDockWidget(tr("Output"), this);
		outputDock_->setObjectName("output");
		outputWidget_ = new QTextEditEx(outputDock_);
		connect(outputWidget_, SIGNAL(mouseDoubleClick(QMouseEvent*)), this, SLOT(outputMouseDoubleClick(QMouseEvent*)));
		outputWidget_->setReadOnly(true);
		outputDock_->setWidget(outputWidget_);
		addDockWidget(Qt::BottomDockWidgetArea, outputDock_);
	}

	ui.actionStart_Player->setIcon(IconLibrary::instance().icon("gamepad"));
	connect(ui.actionStart_Player, SIGNAL(triggered()), this, SLOT(startPlayer()));

	ui.actionStart->setIcon(IconLibrary::instance().icon("start"));
	ui.actionStart->setEnabled(false);
	connect(ui.actionStart, SIGNAL(triggered()), this, SLOT(start()));

	ui.actionStop->setIcon(IconLibrary::instance().icon("stop"));
	ui.actionStop->setEnabled(false);
	connect(ui.actionStop, SIGNAL(triggered()), this, SLOT(stop()));

	connect(ui.actionNew_Project, SIGNAL(triggered()), this, SLOT(newProject()));
	connect(ui.actionClose_Project, SIGNAL(triggered()), this, SLOT(closeProject()));
	connect(ui.actionSave_Project, SIGNAL(triggered()), this, SLOT(saveProject()));
	connect(ui.actionOpen_Project, SIGNAL(triggered()), this, SLOT(openProject()));
	connect(ui.actionExport_Project, SIGNAL(triggered()), this, SLOT(exportProject()));

	connect(ui.actionProject1, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject2, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject3, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject4, SIGNAL(triggered()), this, SLOT(openRecentProject()));
	connect(ui.actionProject5, SIGNAL(triggered()), this, SLOT(openRecentProject()));

	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));

	connect(ui.actionPlayer_Settings, SIGNAL(triggered()), this, SLOT(playerSettings()));

	connect(ui.actionFile_Associations, SIGNAL(triggered()), this, SLOT(fileAssociations()));

	connect(libraryWidget_, SIGNAL(openRequest(const QString&, const QString&)), this, SLOT(onOpenRequest(const QString&, const QString&)));


	QString playerip = settings.value("player ip", QString("127.0.0.1")).toString();

	client_ = new GiderosNetworkClient(playerip, 15000);

	connect(client_, SIGNAL(connected()), this, SLOT(connected()));
	connect(client_, SIGNAL(disconnected()), this, SLOT(disconnected()));
	connect(client_, SIGNAL(dataReceived(const QByteArray&)), this, SLOT(dataReceived(const QByteArray&)));
	connect(client_, SIGNAL(ackReceived(unsigned int)), this, SLOT(ackReceived(unsigned int)));

	updateUI();

	setWindowTitle(tr("Gideros"));

	updateRecentProjectActions();

	QTimer* timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(onTimer()));
	timer->start(1);

	isTransferring_ = false;

	QList<QVariant> v = settings.value("fileAssociations", QList<QVariant>()).toList();
	for (int i = 0; i < v.size(); ++i)
		fileAssociations_.push_back(v[i].toStringList());

	localServer_.listen("GiderosProjectManager");
	connect(&localServer_, SIGNAL(newConnection()), this, SLOT(onLocalServerNewConnection()));

	restoreGeometry(settings.value("geometry").toByteArray());
	restoreState(settings.value("windowState").toByteArray());
}

MainWindow::~MainWindow()
{
	delete client_;
}


void MainWindow::start()
{
	if (client_->isConnected() == false)
		return;

	if (projectFileName_.isEmpty() == true)
		return;

	if (isTransferring_ == true)
		return;

	isTransferring_ = true;

//	saveAll();
	client_->sendStop();
	client_->sendProjectName(QFileInfo(projectFileName_).baseName());
	client_->sendGetFileList();
}

void MainWindow::stop()
{
	if (client_->isConnected() == false)
		return;

	fileQueue_.clear();
	client_->sendStop();
}

void MainWindow::startPlayer()
{
#if defined(Q_OS_MAC)
	QProcess::startDetached("open -a \"/Applications/Gideros/Gideros Player.app\"");
#elif defined(Q_OS_WIN)
	QProcess::startDetached("player.exe");
#else
	QProcess::startDetached("player");
#endif
}

void MainWindow::newProject()
{
	if (maybeSave())
	{
		closeProject();

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

			QTextStream(&file) << libraryWidget_->toXml().toString();

			file.close();

			projectFileName_ = fileName;
			libraryWidget_->setProjectFileName(projectFileName_);

			updateUI();

			setWindowTitle(projectName() + "[*] - " + tr("Gideros"));

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
		QString fileName = QFileDialog::getOpenFileName(this, tr("Open Project"),
			QString(),
			tr("Gideros Project (*.gproj)"));

		if (fileName.isEmpty() == true)
			return;

		openProject(fileName);
	}
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

	closeProject();

	projectFileName_ = fileName;

	libraryWidget_->loadXml(projectFileName_, doc);

	updateUI();
	setWindowTitle(projectName() + "[*] - " + tr("Gideros"));
	addToRecentProjects(projectFileName_);
}

bool MainWindow::maybeSave()
{
	if (libraryWidget_->isModified())
	{
		QMessageBox::StandardButton button = QMessageBox::question(this, "Save", QString("Save project \"%1\"?").arg(projectName()),
											  QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

		switch(button)
		{
			case QMessageBox::Yes:
				saveProject();
				return true;
			break;
			case QMessageBox::No:
				return true;
			break;
			case QMessageBox::Cancel:
				return false;
			break;
			default:
			break;
		}
	}

	return true;
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
	libraryWidget_->setModified(false);
}

void MainWindow::closeProject()
{
	if (maybeSave())
	{
		projectFileName_ = "";

		libraryWidget_->clear();
		outputWidget_->clear();

		updateUI();

		setWindowTitle(tr("Gideros"));
	}
}


void MainWindow::updateUI()
{
	bool isProjectOpen = projectFileName_.isEmpty() == false;

	libraryWidget_->setEnabled(isProjectOpen);
	outputWidget_->setEnabled(isProjectOpen);

	ui.actionSave_Project->setEnabled(isProjectOpen);
	ui.actionClose_Project->setEnabled(isProjectOpen);
	ui.actionExport_Project->setEnabled(isProjectOpen);
}

QString MainWindow::projectName() const
{
	return QFileInfo(projectFileName_).fileName();
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

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (maybeSave())
	{
		QSettings settings;
		settings.setValue("geometry", saveGeometry());
		settings.setValue("windowState", saveState());

		event->accept();
	}
	else
	{
		event->ignore();
	}
}

void MainWindow::openRecentProject()
{
	QAction *action = qobject_cast<QAction *>(sender());

	if (action != 0)
		openProject(action->data().toString());
}

void MainWindow::openFile(const QString& fileName, int line)
{
	bool isStarted = false;

	for (int i = 0; i < fileAssociations_.size(); ++i)
	{
		QString extension = fileAssociations_[i][0];
		QString application = fileAssociations_[i][1];
		QString arguments = fileAssociations_[i][2];

		if (extension.startsWith(".") == false)
			extension = "." + extension;

		if (fileName.endsWith(extension) == true)
		{
			// find [] pairs
			QList<QPair<int, int> > optionals;

			int from = 0;
			while (true)
			{
				int open = arguments.indexOf('[', from);
				if (open == -1)
					break;
				int close = arguments.indexOf(']', open + 1);
				if (close == -1)
					break;

				optionals.push_back(qMakePair(open, close));

				from = close + 1;
			}

			if (line != -1)
			{
				while (true)
				{
					int index = arguments.indexOf("<line>");
					if (index == -1)
						break;

					// optional?
					int open = arguments.lastIndexOf('[', index);
					int close = arguments.indexOf(']', index);

					if (open == -1 || close == -1)
					{
						// not optional
						arguments.replace(index, 6, QString::number(line));
					}
					else
					{
						// optional
						arguments.remove(close, 1);
						arguments.replace(index, 6, QString::number(line));
						arguments.remove(open, 1);
					}
				}
			}
			else
			{
				while (true)
				{
					int index = arguments.indexOf("<line>");
					if (index == -1)
						break;

					// optional?
					int open = arguments.lastIndexOf('[', index);
					int close = arguments.indexOf(']', index);

					if (open == -1 || close == -1)
					{
						// not optional
						arguments.remove(index, 6);
					}
					else
					{
						// optional
						arguments.remove(open, close - open + 1);
					}
				}
			}

			QString proc = "\"" + application + "\"" + " " + "\"" + fileName + "\"";
			QString proc_arg = "\"" + application + "\"" + " " + "\"" + fileName + "\"" + " " + arguments;

#ifdef Q_OS_MAC
		if (QFileInfo(application).suffix().toLower() == "app")
		{
			QProcess::startDetached("open -a " + proc);
		}
		else
		{
			QProcess::startDetached(proc_arg);
		}
#else
		QProcess::startDetached(proc_arg);
#endif

			isStarted = true;
			break;
		}
	}

	if (isStarted == false)
		QDesktopServices::openUrl(QUrl::fromLocalFile(fileName));
}

void MainWindow::onOpenRequest(const QString& itemName, const QString& fileName)
{
	openFile(fileName);
}

void MainWindow::playerSettings()
{
	PlayerSettingsDialog dialog(this);

	if (dialog.exec() == QDialog::Accepted)
	{
		ui.actionStart->setEnabled(false);
		ui.actionStop->setEnabled(false);

		//sentMap_.clear();

		QSettings settings;
		QString playerip = settings.value("player ip", QString("127.0.0.1")).toString();

		client_->connectToHost(playerip, 15000);
	}
}

void MainWindow::connected()
{
	fileQueue_.clear();
	isTransferring_ = false;

	ui.actionStart->setEnabled(true);
	ui.actionStop->setEnabled(true);
	printf("other side connected\n");
}

void MainWindow::disconnected()
{
	fileQueue_.clear();
	isTransferring_ = false;

	ui.actionStart->setEnabled(false);
	ui.actionStop->setEnabled(false);
	printf("other side closed connection\n");
}

void MainWindow::dataReceived(const QByteArray& d)
{
	const char* data = d.constData();

	if (data[0] == 4)
	{
		std::string str = &data[1];
		outputWidget_->moveCursor(QTextCursor::End);		// TODO
		outputWidget_->insertPlainText(str.c_str());		// TODO
	}
	if (data[0] == 10)
	{
		start();
	}
	if (data[0] == 6 && isTransferring_ == true)
	{
		printf("file list has got\n");

		fileQueue_.clear();

		std::map<std::string, std::string> localFileMap;
		std::map<QString, QString> localFileMapReverse;
		{
			std::vector<std::pair<std::string, std::string> > fileList = libraryFileList();

			for (std::size_t i = 0; i < fileList.size(); ++i)
			{
				localFileMap[fileList[i].first] = fileList[i].second;
				localFileMapReverse[fileList[i].second.c_str()] = fileList[i].first.c_str();
			}
		}

		std::map<std::string, int> remoteFileMap;

		ByteBuffer buffer(d.constData(), d.size());

		char chr;
		buffer >> chr;
	
		while (buffer.eob() == false)
		{
			std::string file;
			buffer >> file;

			if (file[0] == 'F')
			{
				int age;
				buffer >> age;

				remoteFileMap[file.c_str() + 1] = age;
			}
			else if (file[0] == 'D')
			{
			}
		}

		// delete unused files
		for (std::map<std::string, int>::iterator iter = remoteFileMap.begin(); iter != remoteFileMap.end(); ++iter)
		{
			if (localFileMap.find(iter->first) == localFileMap.end())
			{
				printf("deleting: %s\n", iter->first.c_str());
				client_->sendDeleteFile(iter->first.c_str());
			}
		}

		// upload files
		QDir path(QFileInfo(projectFileName_).path());
		for (std::map<std::string, std::string>::iterator iter = localFileMap.begin(); iter != localFileMap.end(); ++iter)
		{
			std::map<std::string, int>::iterator riter = remoteFileMap.find(iter->first);
			
			QString localfile = QDir::cleanPath(path.absoluteFilePath(iter->second.c_str()));

			bool send = false;
			if (riter == remoteFileMap.end())
			{
				printf("always upload: %s\n", iter->first.c_str());
				send = true;
			}
			else
			{
				int localage = fileAge(qPrintable(localfile));
				int remoteage = riter->second;

				if (localage < remoteage)
				{
					printf("upload new file: %d %d %s\n", localage, remoteage, iter->first.c_str());
					send = true;
				}
			}

			if (send == true)
				fileQueue_.push_back(qMakePair(QString(iter->first.c_str()), localfile));
			else
				printf("don't upload: %s\n", iter->first.c_str());
		}

		std::vector<QString> topologicalSort = libraryWidget_->topologicalSort();

		QStringList luaFiles;
		for (std::size_t i = 0; i < topologicalSort.size(); ++i)
			luaFiles << localFileMapReverse[topologicalSort[i]];

		if (luaFiles.empty() == false)
			fileQueue_.push_back(qMakePair(luaFiles.join("|"), QString("play")));
	}
}


void MainWindow::ackReceived(unsigned int id)
{
}

std::vector<std::pair<std::string, std::string> > MainWindow::libraryFileList()
{
	std::vector<std::pair<std::string, std::string> > result;

	QDomDocument doc = libraryWidget_->toXml();

	std::stack<QDomNode> stack;
	stack.push(doc.documentElement());

	std::vector<QString> dir;

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

			QString n;
			for (std::size_t i = 0; i < dir.size(); ++i)
				n += dir[i] + "/";
			n += name;

			result.push_back(std::make_pair(n.toStdString(), fileName.toStdString()));

			continue;
		}

		if (type == "folder")
		{
			QString name = e.attribute("name");
			dir.push_back(name);

			QString n;
			for (std::size_t i = 0; i < dir.size(); ++i)
				n += dir[i] + "/";

			stack.push(QDomNode());
		}

		QDomNodeList childNodes = n.childNodes();
		for (int i = 0; i < childNodes.size(); ++i)
			stack.push(childNodes.item(i));
	}

	return result;
}

void MainWindow::onTimer()
{
	QDir path(QFileInfo(projectFileName_).path());

	if (client_ && client_->bytesToWrite() == 0)
	{
		if (fileQueue_.empty() == false)
		{
			const QString& s1 = fileQueue_.front().first;
			const QString& s2 = fileQueue_.front().second;

			if (s2 == "play")
			{
				QStringList luafiles = s1.split("|");	
				outputWidget_->moveCursor(QTextCursor::End);
				outputWidget_->insertPlainText("Uploading finished.\n");
				client_->sendPlay(luafiles);
				isTransferring_ = false;
			}
			else
			{
				// create remote directories
				{
					QStringList paths = s1.split("/");
					QStringList dir;
					for (int i = 0; i < paths.size() - 1; ++i)
					{
						dir << paths[i];
						client_->sendCreateFolder(dir.join("/"));
					}
				}

				QString fileName = QDir::cleanPath(path.absoluteFilePath(s2));
				if (client_->sendFile(s1, fileName) == 0)
				{
					outputWidget_->moveCursor(QTextCursor::End);
					outputWidget_->insertPlainText(s1 + " cannot be opened.\n");
				}
				else
				{
					outputWidget_->moveCursor(QTextCursor::End);
					outputWidget_->insertPlainText(s1 + " is uploading.\n");
				}
			}

			fileQueue_.pop_front();
		}
	}
}

void MainWindow::fileAssociations()
{
	FileAssociationsDialog dialog(fileAssociations_, this);

	if (dialog.exec() == QDialog::Accepted)
	{
		fileAssociations_ = dialog.fileAssociations();

		QList<QVariant> v;
		for (int i = 0; i < fileAssociations_.size(); ++i)
			v.push_back(fileAssociations_[i]);

		QSettings settings;
		settings.setValue("fileAssociations", v);
	}
}

static bool parseFileLineString(const QString& l, QString* fileName = 0, unsigned int* lineNumber = 0)
{
	QString line = l.trimmed();

	QStringList list = line.split(':');

	if (list.size() < 3)
		return false;

	if (list[0].size() == 1)
	{
		if (list[1].isEmpty() == false)
		{
			if (list[1][0] == '/' || list[1][0] == '\\')
			{
				QString file = list[0] + ":" + list[1];
				list.pop_front();
				list[0] = file;
			}
		}
	}

	bool ok;
	unsigned int list1 = list[1].toUInt(&ok);

	if (ok == false)
		return false;

	if (fileName != 0)
		*fileName = list[0];

	if (lineNumber != 0)
		*lineNumber = list1;

	return true;
}


void MainWindow::outputMouseDoubleClick(QMouseEvent* e)
{
	QTextCursor	cursor = outputWidget_->cursorForPosition(e->pos());

	QString itemName;
	unsigned int lineNumber;
	if (parseFileLineString(cursor.block().text(), &itemName, &lineNumber) == true)
	{
		QString fileName = libraryWidget_->fileName(itemName);

		if (fileName.isEmpty() == false)
		{
			QDir dir = QFileInfo(projectFileName_).dir();
			QString fullFileName = QDir::cleanPath(dir.absoluteFilePath(fileName));

			openFile(fullFileName, lineNumber);
		}
	}
}

void MainWindow::onLocalServerNewConnection()
{
	QLocalSocket* socket = localServer_.nextPendingConnection();
	connect(socket, SIGNAL(disconnected()), socket, SLOT(deleteLater()));
	connect(socket, SIGNAL(readyRead()), this, SLOT(onLocalSocketReadyRead()));
}

void MainWindow::onLocalSocketReadyRead()
{
	QLocalSocket* socket = qobject_cast<QLocalSocket*>(sender());
	QByteArray buffer = socket->readAll();

	if (buffer.size() >= 1)
	{
		switch (buffer[0])
		{
		case 0:
			start();
			break;
		case 1:
			stop();
			break;
		}
	}
}

void MainWindow::exportProject()
{
	printf("exportProject\n");

	QString output = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
		QFileInfo(projectFileName_).path(),
		QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

	if (output.isEmpty() == true)
		return;

	QDir outputDir(output);

	outputDir.mkdir("export");
	outputDir.cd("export");

//	saveAll();

	std::deque<QPair<QString, QString> > fileQueue;

	QDomDocument doc = libraryWidget_->toXml();

	std::stack<QDomNode> stack;
	stack.push(doc.documentElement());

	std::vector<QString> dir;

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

			QString n;
			for (std::size_t i = 0; i < dir.size(); ++i)
				n += dir[i] + "/";
			n += name;

			fileQueue.push_back(qMakePair(n, fileName));

			continue;
		}
		
		if (type == "folder")
		{
			QString name = e.attribute("name");
			dir.push_back(name);


			QString n;
			for (std::size_t i = 0; i < dir.size(); ++i)
				n += dir[i] + "/";
		
			outputDir.mkdir(n);

			stack.push(QDomNode());
		}

		QDomNodeList childNodes = n.childNodes();
		for (int i = 0; i < childNodes.size(); ++i)
			stack.push(childNodes.item(i));
	}


	std::vector<QString> topologicalSort = libraryWidget_->topologicalSort();
	for (std::size_t i = 0; i < topologicalSort.size(); ++i)
	{
		int index = -1;
		for (std::size_t j = 0; j < fileQueue.size(); ++j)
		{
			if (fileQueue[j].second == topologicalSort[i])
			{
				index = j;
				break;
			}
		}

		if (index != -1)
		{
			QPair<QString, QString> item = fileQueue[index];
			fileQueue.erase(fileQueue.begin() + index);
			fileQueue.push_back(item);
		}
	}

	QStringList luafiles;
	QStringList luafiles_abs;

	QProgressDialog progress("Copying files...", QString(), 0, fileQueue.size(), this);
	progress.setWindowModality(Qt::WindowModal);

	QDir path(QFileInfo(projectFileName_).path());

	for (std::size_t i = 0; i < fileQueue.size(); ++i)
	{
		const QString& s1 = fileQueue[i].first;
		const QString& s2 = fileQueue[i].second;

		QString src = QDir::cleanPath(path.absoluteFilePath(s2));
		QString dst = QDir::cleanPath(outputDir.absoluteFilePath(s1));

		if (QFileInfo(dst).suffix().toLower() == "lua")
		{
			luafiles.push_back(s1);
			luafiles_abs.push_back(dst);
		}

		progress.setValue(i);
		
		QFile::remove(dst);
		QFile::copy(src, dst);
	}

	// compile lua files
#if 0
	if (true)
	{
		compileThread_ = new CompileThread(luafiles_abs, false, "", this);
		compileThread_->start();
		compileThread_->wait();
		delete compileThread_;
	}
#endif

	// write luafiles.txt
	{
		QFile file(QDir::cleanPath(outputDir.absoluteFilePath("luafiles.txt")));
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
			return;

		QTextStream out(&file);

		for (int i = 0; i < luafiles.size(); ++i)
			out << luafiles[i] << "\n";
	}

	progress.setValue(fileQueue.size());
}
