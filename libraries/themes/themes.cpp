#include <QApplication>
#include <QStyleFactory>
#include <QPalette>

void setDefaultTheme(){
    qApp->setStyle(QStyleFactory::create("fusion"));

    /*
    QPalette defaultTheme;

    defaultTheme.setColor(QPalette::Window,          QColor(51, 51, 51));
    defaultTheme.setColor(QPalette::WindowText,      Qt::white);
    defaultTheme.setColor(QPalette::Base,            QColor(25, 25, 25));
    defaultTheme.setColor(QPalette::AlternateBase,   QColor(51, 51, 51));
    defaultTheme.setColor(QPalette::ToolTipText,     Qt::white);
    defaultTheme.setColor(QPalette::Text,            Qt::white);
    defaultTheme.setColor(QPalette::Button,          QColor(51, 51, 51));
    defaultTheme.setColor(QPalette::ButtonText,      Qt::white);
    defaultTheme.setColor(QPalette::BrightText,      Qt::red);
    defaultTheme.setColor(QPalette::Link,            QColor(98, 159, 188));
    defaultTheme.setColor(QPalette::Highlight,       QColor(190, 223, 239));
    defaultTheme.setColor(QPalette::HighlightedText, Qt::black);

    qApp->setPalette(defaultTheme);

//    qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #394D56; border: 1px solid white; }");
    */
}
