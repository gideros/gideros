#include "errordialog.h"
#include <QFont>
#include <QFontMetrics>

ErrorDialog::ErrorDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    ui.setupUi(this);

#ifdef Q_OS_MAC
        QFont f("Monaco", 10);
#else
        QFont f("Courier New", 8);
#endif

    QFontMetrics fm(f);
    ui.textEdit->setFont(f);
    ui.textEdit->setTabStopWidth(fm.width("1234"));

    connect(ui.clear, SIGNAL(clicked()), ui.textEdit, SLOT(clear()));
    connect(ui.close, SIGNAL(clicked()), this, SLOT(hide()));
}

ErrorDialog::~ErrorDialog()
{

}

void ErrorDialog::appendString(const QString& string)
{
    ui.textEdit->selectAll();
    ui.textEdit->setTextColor(Qt::gray);
    ui.textEdit->moveCursor(QTextCursor::End, QTextCursor::MoveAnchor);
    ui.textEdit->setTextColor(Qt::black);
    ui.textEdit->append(string);
}
