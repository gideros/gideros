#ifndef _GUI_QT_H_
#define _GUI_QT_H_

#include <QDialog>
#include <gui.h>
#include <QMap>

class QAbstractButton;
class QStringList;
class UIManager;
class QLineEdit;

class AlertDialog : public QDialog
{
    Q_OBJECT

public:
    AlertDialog(const char *title,
                const char *message,
                const char *cancelButton,
                const char *button1,
                const char *button2,
                gevent_Callback callback,
                void *udata,
                g_id gid,
                QWidget *parent = 0);

protected:
    virtual void closeEvent(QCloseEvent *);

private slots:
    void clicked(QAbstractButton *button);

private:
    gevent_Callback callback_;
    void *udata_;
    g_id gid_;

private:
    QMap<QAbstractButton*, int> buttonIndices_;
    QAbstractButton *cancelButton_;
};

class TextInputDialog : public QDialog
{
    Q_OBJECT

public:
    TextInputDialog(const char *title,
                    const char *message,
                    const char *text,
                    const char *cancelButton,
                    const char *button1,
                    const char *button2,
                    gevent_Callback callback,
                    void *udata,
                    g_id gid,
                    QWidget *parent = 0);

public:
    void setText(const char* text);
    const char *getText() const;
    void setInputType(int inputType);
    int getInputType() const;
    void setSecureInput(bool secureInput);
    bool isSecureInput() const;

protected:
    virtual void closeEvent(QCloseEvent *);

private slots:
    void clicked(QAbstractButton *button);

private:
    gevent_Callback callback_;
    void *udata_;
    g_id gid_;
    QLineEdit *lineEdit_;
    mutable QByteArray lineEditBuffer_;

private:
    QMap<QAbstractButton*, int> buttonIndices_;
    QAbstractButton *cancelButton_;
};

#endif
