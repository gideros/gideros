#include <gui.h>
#include <jni.h>
#include <android/log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <map>
#include <queue>

#include <stdexcept>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

class Widget
{
public:
	virtual ~Widget() {}
	virtual void show() = 0;
	virtual void hide() = 0;
	virtual bool isVisible() = 0;
};

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
    std::map<g_id, Widget*> map_;
};

class AlertBox : public Widget
{
public:
	AlertBox(const char *title,
			 const char *message,
			 const char *cancelButton,
			 const char *button1,
			 const char *button2,
			 gevent_Callback callback,
			 void *udata,
			 g_id gid)
	{
		callback_ = callback;
		udata_ = udata;
		gid_ = gid;

		JNIEnv *env = g_getJNIEnv();
	
		jclass localRefCls  = env->FindClass("com/giderosmobile/android/player/AlertBox");
		cls_ = (jclass)env->NewGlobalRef(localRefCls);
		env->DeleteLocalRef(localRefCls);
		
		ctorId_ = env->GetMethodID(cls_, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;J)V");
		showId_ = env->GetMethodID(cls_, "show", "()V");
		hideId_ = env->GetMethodID(cls_, "hide", "()V");
		deleteId_ = env->GetMethodID(cls_, "delete", "()V");
		isVisibleId_ = env->GetMethodID(cls_, "isVisible", "()Z");
		
		jstring jtitle = env->NewStringUTF(title);
		jstring jmessage = env->NewStringUTF(message);
		jstring jcancelButton = env->NewStringUTF(cancelButton);
		jstring jbutton1 = button1 ? env->NewStringUTF(button1) : NULL;
		jstring jbutton2 = button2 ? env->NewStringUTF(button2) : NULL;
		
		jobject localRefObj = env->NewObject(cls_, ctorId_, jtitle, jmessage, jcancelButton, jbutton1, jbutton2, (jlong)this);

		env->DeleteLocalRef(jtitle);
		env->DeleteLocalRef(jmessage);
		env->DeleteLocalRef(jcancelButton);
		if (jbutton1)
			env->DeleteLocalRef(jbutton1);
		if (jbutton2)
			env->DeleteLocalRef(jbutton2);

		obj_ = env->NewGlobalRef(localRefObj);
		env->DeleteLocalRef(localRefObj);
	}

	virtual ~AlertBox()
	{
		JNIEnv *env = g_getJNIEnv();
		
		env->CallVoidMethod(obj_, deleteId_);

		env->DeleteGlobalRef(obj_);
		env->DeleteGlobalRef(cls_);
	}
	
	virtual void show()
	{
		JNIEnv *env = g_getJNIEnv();
		env->CallVoidMethod(obj_, showId_);
	}
	
	virtual void hide()
	{
		JNIEnv *env = g_getJNIEnv();
		env->CallVoidMethod(obj_, hideId_);	
	}
	
	virtual bool isVisible()
	{
		JNIEnv *env = g_getJNIEnv();

		return env->CallBooleanMethod(obj_, isVisibleId_);
	}
	
	void callback(int buttonIndex, const char *buttonText)
	{
        size_t size = sizeof(gui_AlertDialogCompleteEvent) + strlen(buttonText) + 1;
        gui_AlertDialogCompleteEvent *event = (gui_AlertDialogCompleteEvent*)malloc(size);
        event->gid = gid_;
        event->buttonIndex = buttonIndex;
        event->buttonText = (char*)event + sizeof(gui_AlertDialogCompleteEvent);
        strcpy((char*)event->buttonText, buttonText);

		gevent_EnqueueEvent(gid_, callback_, GUI_ALERT_DIALOG_COMPLETE_EVENT, event, 1, udata_);
	}
	
private:
	jclass cls_;	
	jmethodID ctorId_, showId_, hideId_, deleteId_, isVisibleId_;
	jobject obj_;

	gevent_Callback callback_;
	void *udata_;
	g_id gid_;
};

class TextInputBox : public Widget
{
public:
	TextInputBox(const char *title,
				 const char *message,
				 const char *text,
				 const char *cancelButton,
				 const char *button1,
				 const char *button2,
				 gevent_Callback callback,
				 void *udata,
				 g_id gid)
	{
		callback_ = callback;
		udata_ = udata;
		gid_ = gid;

		JNIEnv *env = g_getJNIEnv();
	
		jclass localRefCls  = env->FindClass("com/giderosmobile/android/player/TextInputBox");
		cls_ = (jclass)env->NewGlobalRef(localRefCls);
		env->DeleteLocalRef(localRefCls);
		
		ctorId_ = env->GetMethodID(cls_, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;J)V");
		showId_ = env->GetMethodID(cls_, "show", "()V");
		hideId_ = env->GetMethodID(cls_, "hide", "()V");
		deleteId_ = env->GetMethodID(cls_, "delete", "()V");
		isVisibleId_ = env->GetMethodID(cls_, "isVisible", "()Z");
		setTextId_ = env->GetMethodID(cls_, "setText", "(Ljava/lang/String;)V");
		getTextId_ = env->GetMethodID(cls_, "getText", "()Ljava/lang/String;");
		setInputTypeId_ = env->GetMethodID(cls_, "setInputType", "(I)V");
		getInputTypeId_ = env->GetMethodID(cls_, "getInputType", "()I");
		setSecureInputId_ = env->GetMethodID(cls_, "setSecureInput", "(Z)V");
		isSecureInputId_ = env->GetMethodID(cls_, "isSecureInput", "()Z");
		
		jstring jtitle = env->NewStringUTF(title);
		jstring jmessage = env->NewStringUTF(message);
		jstring jtext = env->NewStringUTF(text);
		jstring jcancelButton = env->NewStringUTF(cancelButton);
		jstring jbutton1 = button1 ? env->NewStringUTF(button1) : NULL;
		jstring jbutton2 = button2 ? env->NewStringUTF(button2) : NULL;
		
		jobject localRefObj = env->NewObject(cls_, ctorId_, jtitle, jmessage, jtext, jcancelButton, jbutton1, jbutton2, (jlong)this);

		env->DeleteLocalRef(jtitle);
		env->DeleteLocalRef(jmessage);
		env->DeleteLocalRef(jtext);
		env->DeleteLocalRef(jcancelButton);
		if (jbutton1)
			env->DeleteLocalRef(jbutton1);
		if (jbutton2)
			env->DeleteLocalRef(jbutton2);

		obj_ = env->NewGlobalRef(localRefObj);
		env->DeleteLocalRef(localRefObj);
	}

	virtual ~TextInputBox()
	{
		JNIEnv *env = g_getJNIEnv();
		
		env->CallVoidMethod(obj_, deleteId_);

		env->DeleteGlobalRef(obj_);
		env->DeleteGlobalRef(cls_);
	}
	
	virtual void show()
	{
		JNIEnv *env = g_getJNIEnv();
		env->CallVoidMethod(obj_, showId_);
	}
	
	virtual void hide()
	{
		JNIEnv *env = g_getJNIEnv();
		env->CallVoidMethod(obj_, hideId_);	
	}
	
	virtual bool isVisible()
	{
		JNIEnv *env = g_getJNIEnv();

		return env->CallBooleanMethod(obj_, isVisibleId_);
	}
	
	void setText(const char *text)
	{
		JNIEnv *env = g_getJNIEnv();

		jstring jtext = env->NewStringUTF(text);
		env->CallVoidMethod(obj_, setTextId_, jtext);
		env->DeleteLocalRef(jtext);
	}
	
	const char *getText()
	{	
		JNIEnv *env = g_getJNIEnv();
		
		jstring jtext = (jstring)env->CallObjectMethod(obj_, getTextId_);
		const char *text = env->GetStringUTFChars(jtext, NULL);
		textBuffer_ = text;
		env->ReleaseStringUTFChars(jtext, text);

		return textBuffer_.c_str();
	}
	
	void setInputType(int inputType)
	{
		JNIEnv *env = g_getJNIEnv();		
		env->CallVoidMethod(obj_, setInputTypeId_, (jint)inputType);
	}
		
	int getInputType()
	{
		JNIEnv *env = g_getJNIEnv();		
		return env->CallIntMethod(obj_, getInputTypeId_);
	}
	
	void setSecureInput(bool secureInput)
	{
		JNIEnv *env = g_getJNIEnv();		
		env->CallVoidMethod(obj_, setSecureInputId_, (jboolean)secureInput);
	}
	
	bool isSecureInput()
	{
		JNIEnv *env = g_getJNIEnv();		
		return env->CallBooleanMethod(obj_, isSecureInputId_);
	}
	
	void callback(const char *text, int buttonIndex, const char *buttonText)
	{
        size_t size = sizeof(gui_TextInputDialogCompleteEvent) + strlen(text) + 1 + strlen(buttonText) + 1;
        gui_TextInputDialogCompleteEvent *event = (gui_TextInputDialogCompleteEvent*)malloc(size);
		event->gid = gid_;
		event->text = (char*)event + sizeof(gui_TextInputDialogCompleteEvent);
		event->buttonIndex = buttonIndex;
		event->buttonText = (char*)event + sizeof(gui_TextInputDialogCompleteEvent) + strlen(text) + 1;
        strcpy((char*)event->text, text);
        strcpy((char*)event->buttonText, buttonText);

		gevent_EnqueueEvent(gid_, callback_, GUI_TEXT_INPUT_DIALOG_COMPLETE_EVENT, event, 1, udata_);
	}
	
private:
	jclass cls_;	
	jmethodID ctorId_, showId_, hideId_, deleteId_, isVisibleId_;
	jmethodID setTextId_, getTextId_, setInputTypeId_, getInputTypeId_, setSecureInputId_, isSecureInputId_;
	jobject obj_;

	gevent_Callback callback_;
	void *udata_;
	g_id gid_;
	
	std::string textBuffer_;
};

UIManager::UIManager()
{
}

UIManager::~UIManager()
{
    std::map<g_id, Widget*>::iterator iter, e = map_.end();
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

    AlertBox *alertBox = new AlertBox(title, message,
									  cancelButton, button1, button2,
									  callback, udata,
									  gid);
    map_[gid] = alertBox;

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

    TextInputBox *textInputBox = new TextInputBox(title, message, text,
												  cancelButton, button1, button2,
												  callback, udata,
												  gid);
    map_[gid] = textInputBox;

    return gid;
}

void UIManager::show(g_id gid)
{
    std::map<g_id, Widget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");

    iter->second->show();
}

void UIManager::hide(g_id gid)
{
    std::map<g_id, Widget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");

    iter->second->hide();
}

void UIManager::deleteWidget(g_id gid)
{
    std::map<g_id, Widget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");

    gevent_RemoveEventsWithGid(gid);

    delete iter->second;
    map_.erase(iter);
}

bool UIManager::isVisible(g_id gid)
{
    std::map<g_id, Widget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");

    return iter->second->isVisible();
}

void UIManager::setText(g_id gid, const char* text)
{
    std::map<g_id, Widget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");
		
	TextInputBox *textInputBox = dynamic_cast<TextInputBox*>(iter->second);
	
	if (textInputBox == NULL)
        throw std::runtime_error("invalid gid");
		
	textInputBox->setText(text);
}

const char *UIManager::getText(g_id gid)
{
    std::map<g_id, Widget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");
		
	TextInputBox *textInputBox = dynamic_cast<TextInputBox*>(iter->second);
	
	if (textInputBox == NULL)
        throw std::runtime_error("invalid gid");
		
	return textInputBox->getText();
}

void UIManager::setInputType(g_id gid, int inputType)
{
    std::map<g_id, Widget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");
		
	TextInputBox *textInputBox = dynamic_cast<TextInputBox*>(iter->second);
	
	if (textInputBox == NULL)
        throw std::runtime_error("invalid gid");
		
	textInputBox->setInputType(inputType);
}

int UIManager::getInputType(g_id gid)
{
    std::map<g_id, Widget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");
		
	TextInputBox *textInputBox = dynamic_cast<TextInputBox*>(iter->second);
	
	if (textInputBox == NULL)
        throw std::runtime_error("invalid gid");
		
	return textInputBox->getInputType();
}

void UIManager::setSecureInput(g_id gid, bool secureInput)
{
    std::map<g_id, Widget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");
		
	TextInputBox *textInputBox = dynamic_cast<TextInputBox*>(iter->second);
	
	if (textInputBox == NULL)
        throw std::runtime_error("invalid gid");
		
	textInputBox->setSecureInput(secureInput);
}

bool UIManager::isSecureInput(g_id gid)
{
    std::map<g_id, Widget*>::iterator iter = map_.find(gid);

    if (iter == map_.end())
        throw std::runtime_error("invalid gid");
		
	TextInputBox *textInputBox = dynamic_cast<TextInputBox*>(iter->second);
	
	if (textInputBox == NULL)
        throw std::runtime_error("invalid gid");
		
	return textInputBox->isSecureInput();
}


extern "C"
{
void Java_com_giderosmobile_android_player_AlertBox_completeCallback(JNIEnv *env, jobject thiz, jint buttonIndex, jstring jbuttonText, jlong udata)
{
	AlertBox* alertBox = (AlertBox*)udata;

	const char *buttonText = env->GetStringUTFChars(jbuttonText, NULL);
	alertBox->callback(buttonIndex, buttonText);
	env->ReleaseStringUTFChars(jbuttonText, buttonText);
}

void Java_com_giderosmobile_android_player_TextInputBox_completeCallback(JNIEnv *env, jobject thiz, jstring jtext, jint buttonIndex, jstring jbuttonText, jlong udata)
{
	TextInputBox* textInputBox = (TextInputBox*)udata;

	const char *text = env->GetStringUTFChars(jtext, NULL);
	const char *buttonText = env->GetStringUTFChars(jbuttonText, NULL);
	textInputBox->callback(text, buttonIndex, buttonText);
	env->ReleaseStringUTFChars(jtext, text);
	env->ReleaseStringUTFChars(jbuttonText, buttonText);
}

}

static UIManager *s_manager = NULL;

extern "C" {

void gui_init()
{
    s_manager = new UIManager;
}

void gui_cleanup()
{
    delete s_manager;
    s_manager = NULL;
}

g_id gui_createAlertDialog(const char *title,
                           const char *message,
                           const char *cancelButton,
                           const char *button1,
                           const char *button2,
                           gevent_Callback callback,
                           void *udata)
{
    return s_manager->createAlertDialog(title, message, cancelButton, button1, button2, callback, udata);
}

g_id gui_createTextInputDialog(const char *title,
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

void gui_show(g_id gid)
{
    s_manager->show(gid);
}

void gui_hide(g_id gid)
{
    s_manager->hide(gid);
}

void gui_delete(g_id gid)
{
    s_manager->deleteWidget(gid);
}

int gui_isVisible(g_id gid)
{
    return s_manager->isVisible(gid);
}

void gui_setText(g_id gid, const char* text)
{
    s_manager->setText(gid, text);
}

const char *gui_getText(g_id gid)
{
    return s_manager->getText(gid);
}

void gui_setInputType(g_id gid, int inputType)
{
    s_manager->setInputType(gid, inputType);
}

int gui_getInputType(g_id gid)
{
    return s_manager->getInputType(gid);
}

void gui_setSecureInput(g_id gid, int secureInput)
{
    s_manager->setSecureInput(gid, secureInput);
}

int gui_isSecureInput(g_id gid)
{
    return s_manager->isSecureInput(gid);
}

}
