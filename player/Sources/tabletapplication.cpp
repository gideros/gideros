#include "tabletapplication.h"

TabletApplication::TabletApplication(int& argv, char** argc): QApplication(argv,argc)
{

}

bool TabletApplication::event(QEvent* event){
    if (event->type() == QEvent::TabletEnterProximity || event->type() == QEvent::TabletLeaveProximity) {
        bool active = event->type() == QEvent::TabletEnterProximity? 1:0;
        GLCanvas::TabletActive=active;
        return true;
    }
    return QApplication::event(event);
}
