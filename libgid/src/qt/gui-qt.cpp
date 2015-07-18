#include <gglobal.h>
#include <gui-qt.h>

#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QAbstractButton>
#include <QPushButton>
#include <QDebug>
#include <QApplication>
#include <QCloseEvent>
#include <QLineEdit>

#include <map>
#include <queue>

#include <stdexcept>

class UIManager
{
public:
    UIManager();
    ~UIManager();

    g_id createAlertDialog(const char *title,
                           const char *message,
                           const char *cancelButton,
                           const char *button1,
                           const char *button2,
                           gevent_Callback callback,
                           void *udata);

    g_id createTextInputDialog(const char *title,
                               const char *message,
                               const char *text,
                               const char *cancelButton,
                               const char *button1,
                               const char *button2,
                               gevent_Callback callback,
                               void *udata);

    void show(g_id gid);
    void hide(g_id gid);
    void deleteWidget(g_id gid);
    bool isVisible(g_id gid);

    void setText(g_id gid, const char* text);
    const char *getText(g_id gid);
    void setInputType(g_id gid, int inputType);
    int getInputType(g_id gid);
    void setSecureInput(g_id gid, bool secureInput);
    bool isSecureInput(g_id gid);

private:
    std::map<g_id, QWidget*> map_;
};

AlertDialog::AlertDialog(const char *title,
                         const char *message,
                         const char *cancelButton,
                         const char *button1,
                         const char *button2,
                         gevent_Callback callback,
                         void *udata,
                         g_id gid,
                         QWidget *parent) :
    QDialog(parent)
{
    callback_ = callback;
    udata_ = udata;
    gid_ = gid;

    setModal(true);

    setWindowTitle(QString::fromUtf8(title));

    QLabel *label = new QLabel(QString::fromUtf8(message), this);
    label->setWordWrap(true);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setOrientation(Qt::Horizontal);

    buttonIndices_[cancelButton_ = buttonBox->addButton(QString::fromUtf8(cancelButton), QDialogButtonBox::RejectRole)] = 0;

    if (button1)
        buttonIndices_[buttonBox->addButton(QString::fromUtf8(button1), QDialogButtonBox::AcceptRole)] = 1;

    if (button2)
        buttonIndices_[buttonBox->addButton(QString::fromUtf8(button2), QDialogButtonBox::AcceptRole)] = 2;

    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(clicked(QAbstractButton*)));

    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    verticalLayout->addWidget(label);
    verticalLayout->addWidget(buttonBox);
}

void AlertDialog::clicked(QAbstractButton *button)
{
    QByteArray buttonText = button->text().toUtf8();
    size_t size = sizeof(gui_AlertDialogCompleteEvent) + buttonText.size() + 1;
    gui_AlertDialogCompleteEvent *event = (gui_AlertDialogCompleteEvent*)malloc(size);
    event->gid = gid_;
    event->buttonIndex = buttonIndices_[button];
    event->buttonText = (char*)event + sizeof(gui_AlertDialogCompleteEvent);
    strcpy((char*)event->buttonText, buttonText.data());

    gevent_EnqueueEvent(gid_, callback_, GUI_ALERT_DIALOG_COMPLETE_EVENT, event, 1, udata_);

    hide();
}

void AlertDialog::closeEvent(QCloseEvent *closeEvent)
{
    QAbstractButton *button = cancelButton_;

    QByteArray buttonText = button->text().toUtf8();
    size_t size = sizeof(gui_AlertDialogCompleteEvent) + buttonText.size() + 1;
    gui_AlertDialogCompleteEvent *event = (gui_AlertDialogCompleteEvent*)malloc(size);
    event->gid = gid_;
    event->buttonIndex = buttonIndices_[button];
    event->buttonText = (char*)event + sizeof(gui_AlertDialogCompleteEvent);
    strcpy((char*)event->buttonText, buttonText.data());

    gevent_EnqueueEvent(gid_, callback_, GUI_ALERT_DIALOG_COMPLETE_EVENT, event, 1, udata_);

    closeEvent->accept();
}


TextInputDialog::TextInputDialog(const char *title,
                                 const char *message,
                                 const char *text,
                                 const char *cancelButton,
                                 const char *button1,
                                 const char *button2,
                                 gevent_Callback callback,
                                 void *udata,
                                 g_id gid,
                                 QWidget *parent) :
    QDialog(parent)
{
    callback_ = callback;
    udata_ = udata;
    gid_ = gid;

    setModal(true);

    setWindowTitle(QString::fromUtf8(title));

    QLabel *label = new QLabel(QString::fromUtf8(message), this);
    label->setWordWrap(true);

    lineEdit_ = new QLineEdit(QString::fromUtf8(text), this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);
    buttonBox->setOrientation(Qt::Horizontal);

    buttonIndices_[cancelButton_ = buttonBox->addButton(QString::fromUtf8(cancelButton), QDialogButtonBox::RejectRole)] = 0;

    if (button1)
        buttonIndices_[buttonBox->addButton(QString::fromUtf8(button1), QDialogButtonBox::AcceptRole)] = 1;

    if (button2)
        buttonIndices_[buttonBox->addButton(QString::fromUtf8(button2), QDialogButtonBox::AcceptRole)] = 2;

    connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(clicked(QAbstractButton*)));

    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    verticalLayout->addWidget(label);
    verticalLayout->addWidget(lineEdit_);
    verticalLayout->addWidget(buttonBox);
}

void TextInputDialog::clicked(QAbstractButton *button)
{
    QByteArray text = lineEdit_->text().toUtf8();
    QByteArray buttonText = button->text().toUtf8();
    size_t size = sizeof(gui_TextInputDialogCompleteEvent) + text.size() + 1 + buttonText.size() + 1;
    gui_TextInputDialogCompleteEvent *event = (gui_TextInputDialogCompleteEvent*)malloc(size);
    event->gid = gid_;
    event->text = (char*)event + sizeof(gui_TextInputDialogCompleteEvent);
    event->buttonIndex = buttonIndices_[button];
    event->buttonText = (char*)event + sizeof(gui_TextInputDialogCompleteEvent) + text.size() + 1;
    strcpy((char*)event->text, text.data());
    strcpy((char*)event->buttonText, buttonText.data());

    gevent_EnqueueEvent(gid_, callback_, GUI_TEXT_INPUT_DIALOG_COMPLETE_EVENT, event, 1, udata_);

    hide();
}

void TextInputDialog::closeEvent(QCloseEvent *closeEvent)
{
    QAbstractButton *button = cancelButton_;

    QByteArray text = lineEdit_->text().toUtf8();
    QByteArray buttonText = button->text().toUtf8();
    size_t size = sizeof(gui_TextInputDialogCompleteEvent) + text.size() + 1 + buttonText.size() + 1;
    gui_TextInputDialogCompleteEvent *event = (gui_TextInputDialogCompleteEvent*)malloc(size);
    event->gid = gid_;
    event->text = (char*)event + sizeof(gui_TextInputDialogCompleteEvent);
    event->buttonIndex = buttonIndices_[button];
    event->buttonText = (char*)event + sizeof(gui_TextInputDialogCompleteEvent) + text.size() + 1;
    strcpy((char*)event->text, text.data());
    strcpy((char*)event->buttonText, buttonText.data());

    gevent_EnqueueEvent(gid_, callback_, GUI_TEXT_INPUT_DIALOG_COMPLETE_EVENT, event, 1, udata_);

    closeEvent->accept();
}

void TextInputDialog::setText(const char* text)
{
    lineEdit_->setText(QString::fromUtf8(text));
}

const char *TextInputDialog::getText() const
{
    lineEditBuffer_ = lineEdit_->text().toUtf8();
    return lineEditBuffer_.data();
}

void TextInputDialog::setInputType(int inputType)
{
    // do nothing
}

int TextInputDialog::getInputType() const
{
    return GUI_TEXT_INPUT_DIALOG_TEXT;
}

void TextInputDialog::setSecureInput(bool secureInput)
{
    lineEdit_->setEchoMode(secureInput ? QLineEdit::Password : QLineEdit::Normal);
}

bool TextInputDialog::isSecureInput() const
{
    return lineEdit_->echoMode() == QLineEdit::Password;
}

UIManager::UIManager()
{
}

UIManager::~UIManager()
{
    std::map<g_id, QWidget*>::iterator iter, e = map_.end();
    for (iter = map_.begin(); iter != e; ++iter)
        delete iter->second;
}

g_id UIManager::createAlertDialog(const char *title,
                                  const char *message,
                                  const char *cancelButton,
                                  const char *button1,
                                  const char *button2,
                                  gevent_Callback callback,
                                  void *udata)
{
    g_id gid = g_NextId();

    AlertDialog *alertDialog = new AlertDialog(title, message,
                                               cancelButton, button1, button2,
                                               callback, udata,
                                               gid,
                                               QApplication::activeWindow());
    map_[gid] = alertDialog;

    return gid;
}

g_id UIManager::createTextInputDialog(const char *title,
                                      const char *message,
                                      const char *text,
                                      const char *cancelButton,
                                      const char *button1,
                                      const char *button2,
                                      gevent_Callback callback,
                                      void *udata)
{
    g_id gid = g_NextId();

    TextInputDialog *textInputDialog = new TextInputDialog(title, message, text,
                                                           cancelButton, button1, button2,
                                                           callback, udata,
                                                           gid,
                                                           QApplication::activeWindow());
    map_[gid] = textInputDialog;

    return gid;
}

void UIManager::show(g_id gid)
{
    std::map<g_id, QWidget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");

    iter->second->show();
}

void UIManager::hide(g_id gid)
{
    std::map<g_id, QWidget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");

    iter->second->hide();
}

void UIManager::deleteWidget(g_id gid)
{
    std::map<g_id, QWidget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");

    gevent_RemoveEventsWithGid(gid);

    delete iter->second;
    map_.erase(iter);
}

bool UIManager::isVisible(g_id gid)
{
    std::map<g_id, QWidget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");

    return iter->second->isVisible();
}

void UIManager::setText(g_id gid, const char* text)
{
    std::map<g_id, QWidget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");

    TextInputDialog *dialog = qobject_cast<TextInputDialog*>(iter->second);

    if (dialog == NULL)
        throw std::runtime_error("invalid gid");

    dialog->setText(text);
}

const char *UIManager::getText(g_id gid)
{
    std::map<g_id, QWidget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");

    TextInputDialog *dialog = qobject_cast<TextInputDialog*>(iter->second);

    if (dialog == NULL)
        throw std::runtime_error("invalid gid");

    return dialog->getText();
}

void UIManager::setInputType(g_id gid, int inputType)
{
    std::map<g_id, QWidget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");

    TextInputDialog *dialog = qobject_cast<TextInputDialog*>(iter->second);

    if (dialog == NULL)
        throw std::runtime_error("invalid gid");

    dialog->setInputType(inputType);
}

int UIManager::getInputType(g_id gid)
{
    std::map<g_id, QWidget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");

    TextInputDialog *dialog = qobject_cast<TextInputDialog*>(iter->second);

    if (dialog == NULL)
        throw std::runtime_error("invalid gid");

    return dialog->getInputType();
}

void UIManager::setSecureInput(g_id gid, bool secureInput)
{
    std::map<g_id, QWidget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");

    TextInputDialog *dialog = qobject_cast<TextInputDialog*>(iter->second);

    if (dialog == NULL)
        throw std::runtime_error("invalid gid");

    dialog->setSecureInput(secureInput);
}

bool UIManager::isSecureInput(g_id gid)
{
    std::map<g_id, QWidget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");

    TextInputDialog *dialog = qobject_cast<TextInputDialog*>(iter->second);

    if (dialog == NULL)
        throw std::runtime_error("invalid gid");

    return dialog->isSecureInput();
}

static UIManager *s_manager = NULL;

extern "C" {

G_API void gui_init()
{
    s_manager = new UIManager;
}

G_API void gui_cleanup()
{
    delete s_manager;
    s_manager = NULL;
}

G_API g_id gui_createAlertDialog(const char *title,
                                 const char *message,
                                 const char *cancelButton,
                                 const char *button1,
                                 const char *button2,
                                 gevent_Callback callback,
                                 void *udata)
{
    return s_manager->createAlertDialog(title, message, cancelButton, button1, button2, callback, udata);
}

G_API g_id gui_createTextInputDialog(const char *title,
                                     const char *message,
                                     const char *text,
                                     const char *cancelButton,
                                     const char *button1,
                                     const char *button2,
                                     gevent_Callback callback,
                                     void *udata)
{
    return s_manager->createTextInputDialog(title, message, text, cancelButton, button1, button2, callback, udata);
}

G_API void gui_show(g_id gid)
{
    s_manager->show(gid);
}

G_API void gui_hide(g_id gid)
{
    s_manager->hide(gid);
}

G_API void gui_delete(g_id gid)
{
    s_manager->deleteWidget(gid);
}

G_API int gui_isVisible(g_id gid)
{
    return s_manager->isVisible(gid);
}

G_API void gui_setText(g_id gid, const char* text)
{
    s_manager->setText(gid, text);
}

G_API const char *gui_getText(g_id gid)
{
    return s_manager->getText(gid);
}

G_API void gui_setInputType(g_id gid, int inputType)
{
    s_manager->setInputType(gid, inputType);
}

G_API int gui_getInputType(g_id gid)
{
    return s_manager->getInputType(gid);
}

G_API void gui_setSecureInput(g_id gid, int secureInput)
{
    s_manager->setSecureInput(gid, secureInput);
}

G_API int gui_isSecureInput(g_id gid)
{
    return s_manager->isSecureInput(gid);
}

}
