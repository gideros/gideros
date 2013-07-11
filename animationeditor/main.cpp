#include <refptr.h>
#include <QtGui/QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
	int result = 0;
	{
		QApplication a(argc, argv);
		MainWindow w;
		w.show();

		result = a.exec();
	}

	printf("%d\n", Referenced::instanceCount);
	
	return result;
}
