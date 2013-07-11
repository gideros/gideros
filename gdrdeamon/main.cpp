#include <QtSingleCoreApplication>
#include "application.h"
#include <stdio.h>

int main(int argc, char *argv[])
{
    QtSingleCoreApplication a(argc, argv);

    if (a.isRunning())
    {
        fprintf(stderr, "* deamon already running *\n");
        return EXIT_FAILURE;
    }

    Application b;

    return a.exec();
}

