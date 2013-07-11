//#include <QDir>

//#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
//#include <crtdbg.h>

//#include <GL/glut.h>
#include <GL/freeglut.h>

#include <platform.h>
#include <libnetwork.h>
#include <luaapplication.h>

#include <string>
#include <refptr.h>


#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
static void _mkdir(const char* path)
{
	mkdir(path, 0755);
}
#endif



static Server* server_;
static LuaApplication* application_;
static std::string resourceDirectory_;
static std::vector<std::string> fileList_;

static bool play_ = false;

static Server* g_server = NULL;
static void printToServer(const char* str, void* data)
{
	unsigned int size = 1 + strlen(str) + 1;
	char* buffer = (char*)malloc(size);

	int pos = 0;
	buffer[pos] = 4;
	pos += 1;
	strcpy(buffer + pos, str);
	pos += strlen(str) + 1;
//	Q_ASSERT(pos == size);

	g_server->sendData(buffer, size);

	free(buffer);
}

// TODO: TimerEvent.TIMER'da bi exception olursa, o event bir daha cagirilmiyor. Bunun nedeini bulmak lazim
void timerEvent()
{
	//	printf("%d\n", Referenced::instanceCount);
	int dataTotal = 0;

	while (true)
	{
		int dataSent0 = server_->dataSent();
		int dataReceived0 = server_->dataReceived();

		NetworkEvent event;
		server_->tick(&event);

		//		if (event.eventCode != eNone)
		//			printf("%s\n", eventCodeString(event.eventCode));

		int dataSent1 = server_->dataSent();
		int dataReceived1 = server_->dataReceived();

		if (event.eventCode == eDataReceived)
		{
			const std::vector<char>& data = event.data;

			switch (data[0])
			{
			case 0:			// create folder
				{
					std::string folderName = &data[1];
					_mkdir(pathForFile(folderName.c_str()));
					break;
				}

			case 1:			// create file
				{
					std::string fileName = &data[1];
					FILE* fos = fopen(pathForFile(fileName.c_str()), "wb");
					int pos = 1 + fileName.size() + 1;
					if (data.size() > pos)
						fwrite(&data[pos], data.size() - pos, 1, fos);
					fclose(fos);
					fileList_.push_back(fileName);
					break;
				}
			case 2:
				{
					printf("play message is received\n");
					play_ = true;

					try
					{
						application_->deinitialize();
						application_->initialize();

						for (std::size_t i = 0; i < fileList_.size(); ++i)
						{
							if (fileList_[i].size() >= 5)
							{
								std::string ext = fileList_[i].substr(fileList_[i].size() - 4);

								for (std::size_t j = 0; j < ext.size(); ++j)
									ext[j] = tolower(ext[j]);

								if (ext == ".lua")
									application_->loadFile(fileList_[i].c_str());
							}
						}
					}
					catch (LuaException e)
					{
						printf("%s\n", e.what());
					}

					break;
				}
			case 3:
				{
					printf("stop message is received\n");
					application_->deinitialize();
					application_->initialize();
//					application_->setHardwareOrientation(orientation_);
					break;
				}
			case 5:
				{
//					deleteFiles();
					break;
				}
			}
		}

		int dataDelta = (dataSent1 - dataSent0) + (dataReceived1 - dataReceived0);
		dataTotal += dataDelta;

		if (dataDelta == 0 || dataTotal > 1024)
			break;
	}

}



void displayCallback()
{
	timerEvent();

	if (play_ == true)
	{
		try
		{
			application_->renderScene();
		}
		catch (LuaException e)
		{
			printf("%s\n", e.what());
		}
	}

	glutSwapBuffers();
	glutPostRedisplay();
}


void reshapeCallback(int w, int h)
{

}

void initialize()
{
	server_ = new Server(15000);
	g_server = server_;
	application_ = new LuaApplication;
	application_->enableExceptions();
	application_->setPrintFunc(printToServer);

	_mkdir("temp");
	_mkdir("temp/documents");
	_mkdir("temp/temporary");
	_mkdir("temp/resource");

	setDocumentsDirectory("./temp/documents");
	setTemporaryDirectory("./temp/temporary");
	setResourceDirectory("./temp/resource"); 



/*	dir_ = QDir::temp();
	dir_.mkdir("gideros");
	dir_.cd("gideros");

	dir_.mkdir("documents");
	dir_.mkdir("temporary");
	dir_.mkdir("resource");

	resourceDirectory_ = qPrintable(dir_.absoluteFilePath("resource"));

	setDocumentsDirectory(qPrintable(dir_.absoluteFilePath("documents")));
	setTemporaryDirectory(qPrintable(dir_.absoluteFilePath("temporary")));
	setResourceDirectory(resourceDirectory_.c_str()); 
	
	*/


	application_->initialize();
//	application_->loadFile("main.lua");
}

void finalize()
{
	printf("finalize\n");
	application_->deinitialize();
	delete application_;

	delete server_;
	g_server = 0;

	Referenced::emptyPool();
}

int main(int argc, char* argv[])
{
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//	_CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
//	_CrtSetBreakAlloc(145233);

	glutInit(&argc, argv);
	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowSize(320, 480);

	glutCreateWindow("gideros player");

	glutDisplayFunc(displayCallback);
	glutReshapeFunc(reshapeCallback);

	initialize();
	glutMainLoop();
	finalize();

	return 0;
}
