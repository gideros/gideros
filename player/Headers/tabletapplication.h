#ifndef TABLETAPPLICATION_H
#define TABLETAPPLICATION_H

#include <qapplication.h>
#include "glcanvas.h"

class TabletApplication : public QApplication {
    Q_OBJECT
public:
    TabletApplication(int& argv, char** argc);
    bool event(QEvent* event);
};

#endif // TABLETAPPLICATION_H
