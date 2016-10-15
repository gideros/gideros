#include <ginput.h>
#include <ginput-android.h>
#include <gevent.h>
#include <jni.h>
#include <vector>
#include <map>
#include <pthread.h>

extern "C" {
JavaVM *g_getJavaVM();
JNIEnv *g_getJNIEnv();
}

static double s_accelerometerx = 0, s_accelerometery = 0, s_accelerometerz = 0;

static double s_gyroscopex = 0, s_gyroscopey = 0, s_gyroscopez = 0;

extern "C" {

void Java_com_giderosmobile_android_player_Accelerometer_onSensorChanged(JNIEnv *env, jclass clz, jfloat x, jfloat y, jfloat z)
{
	s_accelerometerx = x;
	s_accelerometery = y;
	s_accelerometerz = z;
}

void Java_com_giderosmobile_android_player_Gyroscope_onSensorChanged(JNIEnv *env, jclass clz, jfloat x, jfloat y, jfloat z)
{
	s_gyroscopex = x;
	s_gyroscopey = y;
	s_gyroscopez = z;
}

}


class GGInputManager
{
public:
    GGInputManager()
    {
		accelerometerStartCount_ = 0;
		gyroscopeStartCount_ = 0;

        isMouseToTouchEnabled_ = 0;
        isTouchToMouseEnabled_ = 0;
        mouseTouchOrder_= 0;

		const int KEYCODE_SEARCH  = 84;
		const int KEYCODE_BACK = 4;
		const int KEYCODE_MENU  = 82;
		const int KEYCODE_DPAD_CENTER = 23;
		const int KEYCODE_DPAD_UP = 19;
		const int KEYCODE_DPAD_DOWN = 20;
		const int KEYCODE_DPAD_LEFT = 21;
		const int KEYCODE_DPAD_RIGHT = 22;

		const int KEYCODE_0 = 7;
		const int KEYCODE_1 = 8;
		const int KEYCODE_2 = 9;
		const int KEYCODE_3 = 10;
		const int KEYCODE_4 = 11;
		const int KEYCODE_5 = 12;
		const int KEYCODE_6 = 13;
		const int KEYCODE_7 = 14;
		const int KEYCODE_8 = 15;
		const int KEYCODE_9 = 16;

		const int KEYCODE_A = 29;
		const int KEYCODE_B = 30;
		const int KEYCODE_C = 31;
		const int KEYCODE_D = 32;
		const int KEYCODE_E = 33;
		const int KEYCODE_F = 34;
		const int KEYCODE_G = 35;
		const int KEYCODE_H = 36;
		const int KEYCODE_I = 37;
		const int KEYCODE_J = 38;
		const int KEYCODE_K = 39;
		const int KEYCODE_L = 40;
		const int KEYCODE_M = 41;
		const int KEYCODE_N = 42;
		const int KEYCODE_O = 43;
		const int KEYCODE_P = 44;
		const int KEYCODE_Q = 45;
		const int KEYCODE_R = 46;
		const int KEYCODE_S = 47;
		const int KEYCODE_T = 48;
		const int KEYCODE_U = 49;
		const int KEYCODE_V = 50;
		const int KEYCODE_W = 51;
		const int KEYCODE_X = 52;
		const int KEYCODE_Y = 53;
		const int KEYCODE_Z = 54;

		const int KEYCODE_BACKSPACE = 67;

		const int KEYCODE_BUTTON_A = 96;
		const int KEYCODE_BUTTON_B = 97;
		const int KEYCODE_BUTTON_C = 98;
		const int KEYCODE_BUTTON_X = 99;
		const int KEYCODE_BUTTON_Y = 100;
		const int KEYCODE_BUTTON_Z = 101;
		
		const int KEYCODE_BUTTON_SELECT = 109;
		const int KEYCODE_BUTTON_START = 108;
		const int KEYCODE_BUTTON_L1 = 102;
		const int KEYCODE_BUTTON_R1 = 103;
	
		keyMap_[KEYCODE_SEARCH] = GINPUT_KEY_SEARCH;
		keyMap_[KEYCODE_BACK] = GINPUT_KEY_BACK;
		keyMap_[KEYCODE_MENU] = GINPUT_KEY_MENU;
		keyMap_[KEYCODE_DPAD_CENTER] = GINPUT_KEY_CENTER;
		keyMap_[KEYCODE_DPAD_UP] = GINPUT_KEY_UP;
		keyMap_[KEYCODE_DPAD_DOWN] = GINPUT_KEY_DOWN;
		keyMap_[KEYCODE_DPAD_LEFT] = GINPUT_KEY_LEFT;
		keyMap_[KEYCODE_DPAD_RIGHT] = GINPUT_KEY_RIGHT;
		keyMap_[KEYCODE_BUTTON_SELECT] = GINPUT_KEY_SELECT;
		keyMap_[KEYCODE_BUTTON_START] = GINPUT_KEY_START;
		keyMap_[KEYCODE_BUTTON_L1] = GINPUT_KEY_L1;
		keyMap_[KEYCODE_BUTTON_R1] = GINPUT_KEY_R1;

		keyMap_[KEYCODE_0] = GINPUT_KEY_0;
		keyMap_[KEYCODE_1] = GINPUT_KEY_1;
		keyMap_[KEYCODE_2] = GINPUT_KEY_2;
		keyMap_[KEYCODE_3] = GINPUT_KEY_3;
		keyMap_[KEYCODE_4] = GINPUT_KEY_4;
		keyMap_[KEYCODE_5] = GINPUT_KEY_5;
		keyMap_[KEYCODE_6] = GINPUT_KEY_6;
		keyMap_[KEYCODE_7] = GINPUT_KEY_7;
		keyMap_[KEYCODE_8] = GINPUT_KEY_8;
		keyMap_[KEYCODE_9] = GINPUT_KEY_9;
		
		keyMap_[KEYCODE_A] = GINPUT_KEY_A;
		keyMap_[KEYCODE_B] = GINPUT_KEY_B;
		keyMap_[KEYCODE_C] = GINPUT_KEY_C;
		keyMap_[KEYCODE_D] = GINPUT_KEY_D;
		keyMap_[KEYCODE_E] = GINPUT_KEY_E;
		keyMap_[KEYCODE_F] = GINPUT_KEY_F;
		keyMap_[KEYCODE_G] = GINPUT_KEY_G;
		keyMap_[KEYCODE_H] = GINPUT_KEY_H;
		keyMap_[KEYCODE_I] = GINPUT_KEY_I;
		keyMap_[KEYCODE_J] = GINPUT_KEY_J;
		keyMap_[KEYCODE_K] = GINPUT_KEY_K;
		keyMap_[KEYCODE_L] = GINPUT_KEY_L;
		keyMap_[KEYCODE_M] = GINPUT_KEY_M;
		keyMap_[KEYCODE_N] = GINPUT_KEY_N;
		keyMap_[KEYCODE_O] = GINPUT_KEY_O;
		keyMap_[KEYCODE_P] = GINPUT_KEY_P;
		keyMap_[KEYCODE_Q] = GINPUT_KEY_Q;
		keyMap_[KEYCODE_R] = GINPUT_KEY_R;
		keyMap_[KEYCODE_S] = GINPUT_KEY_S;
		keyMap_[KEYCODE_T] = GINPUT_KEY_T;
		keyMap_[KEYCODE_U] = GINPUT_KEY_U;
		keyMap_[KEYCODE_V] = GINPUT_KEY_V;
		keyMap_[KEYCODE_W] = GINPUT_KEY_W;
		keyMap_[KEYCODE_X] = GINPUT_KEY_X;
		keyMap_[KEYCODE_Y] = GINPUT_KEY_Y;
		keyMap_[KEYCODE_Z] = GINPUT_KEY_Z;		

		keyMap_[KEYCODE_BACKSPACE] = GINPUT_KEY_BACKSPACE;
		keyMap_[KEYCODE_BUTTON_A] = GINPUT_KEY_A;
		keyMap_[KEYCODE_BUTTON_B] = GINPUT_KEY_B;
		keyMap_[KEYCODE_BUTTON_C] = GINPUT_KEY_C;
		keyMap_[KEYCODE_BUTTON_X] = GINPUT_KEY_X;
		keyMap_[KEYCODE_BUTTON_Y] = GINPUT_KEY_Y;
		keyMap_[KEYCODE_BUTTON_Z] = GINPUT_KEY_Z;

		pthread_mutex_init(&touchPoolMutex_, NULL);
		pthread_mutex_init(&keyPoolMutex_, NULL);
		pthread_mutex_init(&mousePoolMutex_, NULL);

        gevent_AddCallback(posttick_s, this);

        gid_ = g_NextId();
    }
    
    ~GGInputManager()
    {
		if (accelerometerStartCount_ > 0)
			stopAccelerometerHelper();
		if (gyroscopeStartCount_ > 0)
			stopGyroscopeHelper();

        gevent_RemoveCallbackWithGid(gid_);

        gevent_RemoveCallback(posttick_s, this);
		
		pthread_mutex_lock(&touchPoolMutex_);
		std::map<size_t, std::vector<ginput_TouchEvent*> >::iterator iter;
		for (iter = touchPool1_.begin(); iter != touchPool1_.end(); ++iter)
		{
			const std::vector<ginput_TouchEvent*> &v = iter->second;
			for (size_t i = 0; i < v.size(); ++i)
			{
				delete [] v[i]->allTouches;
				delete v[i];
			}
		}
		for (iter = touchPool2_.begin(); iter != touchPool2_.end(); ++iter)
		{
			const std::vector<ginput_TouchEvent*> &v = iter->second;
			for (size_t i = 0; i < v.size(); ++i)
			{
				delete [] v[i]->allTouches;
				delete v[i];
			}
		}
		pthread_mutex_unlock(&touchPoolMutex_);

        pthread_mutex_destroy(&touchPoolMutex_);
		
		
		pthread_mutex_lock(&keyPoolMutex_);
		for (size_t i = 0; i < keyPool1_.size(); ++i)
            delete keyPool1_[i];
        for (size_t i = 0; i < keyPool2_.size(); ++i)
            delete keyPool2_[i];
		pthread_mutex_unlock(&keyPoolMutex_);

        pthread_mutex_destroy(&keyPoolMutex_);


		pthread_mutex_lock(&mousePoolMutex_);
        for (size_t i = 0; i < mousePool1_.size(); ++i)
            delete mousePool1_[i];
        for (size_t i = 0; i < mousePool2_.size(); ++i)
            delete mousePool2_[i];
		pthread_mutex_unlock(&mousePoolMutex_);

        pthread_mutex_destroy(&mousePoolMutex_);
	}
    
    bool isAccelerometerAvailable()
    {
		JNIEnv *env = g_getJNIEnv();

		jclass cls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
		jboolean result = env->CallStaticBooleanMethod(cls, env->GetStaticMethodID(cls, "isAccelerometerAvailable_s", "()Z"));
		env->DeleteLocalRef(cls);
		
		return result;
    }
    
    void startAccelerometer()
    {
		accelerometerStartCount_++;
		if (accelerometerStartCount_ == 1)
			startAccelerometerHelper();
    }
    
    void stopAccelerometer()
    {
		if (accelerometerStartCount_ > 0)
		{
			accelerometerStartCount_--;
			if (accelerometerStartCount_ == 0)
				stopAccelerometerHelper();
		}
    }
    
    void getAcceleration(double *x, double *y, double *z)
    {
        double x2 = 0, y2 = 0, z2 = 0;

        if (accelerometerStartCount_ > 0)
        {
            x2 = s_accelerometerx;
            y2 = s_accelerometery;
            z2 = s_accelerometerz;
        }
        
        if (x)
            *x = x2;
        if (y)
            *y = y2;
        if (z)
            *z = z2;
    }

    bool isGyroscopeAvailable()
    {
		JNIEnv *env = g_getJNIEnv();

		jclass cls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
		jboolean result = env->CallStaticBooleanMethod(cls, env->GetStaticMethodID(cls, "isGyroscopeAvailable_s", "()Z"));
		env->DeleteLocalRef(cls);
		
		return result;
    }
    
    void startGyroscope()
    {
		gyroscopeStartCount_++;
		if (gyroscopeStartCount_ == 1)
			startGyroscopeHelper();
    }
    
    void stopGyroscope()
    {
		if (gyroscopeStartCount_ > 0)
		{
			gyroscopeStartCount_--;
			if (gyroscopeStartCount_ == 0)
				stopGyroscopeHelper();
		}
    }

    void getGyroscopeRotationRate(double *x, double *y, double *z)
    {
        double x2 = 0, y2 = 0, z2 = 0;

        if (gyroscopeStartCount_ > 0)
        {
            x2 = s_gyroscopex;
            y2 = s_gyroscopey;
            z2 = s_gyroscopez;
        }
        
        if (x)
            *x = x2;
        if (y)
            *y = y2;
        if (z)
            *z = z2;
    }

public:
    static void posttick_s(int type, void *event, void *udata)
    {
        if (type == GEVENT_POST_TICK_EVENT)
			static_cast<GGInputManager*>(udata)->posttick();
    }

    void posttick()
    {
		pthread_mutex_lock(&touchPoolMutex_);
		std::map<size_t, std::vector<ginput_TouchEvent*> >::iterator iter;
		for (iter = touchPool2_.begin(); iter != touchPool2_.end(); ++iter)
		{
			const std::vector<ginput_TouchEvent*> &v = iter->second;
			for (size_t i = 0; i < v.size(); ++i)
				touchPool1_[iter->first].push_back(v[i]);
		}
		touchPool2_.clear();
		pthread_mutex_unlock(&touchPoolMutex_);

		pthread_mutex_lock(&keyPoolMutex_);
        for (size_t i = 0; i < keyPool2_.size(); ++i)
            keyPool1_.push_back(keyPool2_[i]);
        keyPool2_.clear();
		pthread_mutex_unlock(&keyPoolMutex_);

		pthread_mutex_lock(&mousePoolMutex_);
        for (size_t i = 0; i < mousePool2_.size(); ++i)
            mousePool1_.push_back(mousePool2_[i]);
        mousePool2_.clear();
		pthread_mutex_unlock(&mousePoolMutex_);
    }

public:
    void setMouseToTouchEnabled(int enabled)
    {
        isMouseToTouchEnabled_ = enabled;
    }

    void setTouchToMouseEnabled(int enabled)
    {
        isTouchToMouseEnabled_ = enabled;
    }

    void setMouseTouchOrder(int order)
    {
        mouseTouchOrder_ = order;
    }	

public:
	void touchBegin(int size, int *id, int *x, int *y, float *pressure, int actionIndex)
	{
		ginput_TouchEvent *touchEvent = newTouchEvent(size);
		
		touchEvent->touch.x = x[actionIndex];
		touchEvent->touch.y = y[actionIndex];
        touchEvent->touch.pressure = pressure[actionIndex];
        touchEvent->touch.touchType = 0;
		touchEvent->touch.id = id[actionIndex];
		
		for (int i = 0; i < size; ++i)
		{
			touchEvent->allTouches[i].x = x[i];
			touchEvent->allTouches[i].y = y[i];
            touchEvent->allTouches[i].pressure = pressure[i];
            touchEvent->allTouches[i].touchType = 0;
			touchEvent->allTouches[i].id = id[i];
		}
		
		
		ginput_MouseEvent *mouseEvent = NULL;
		if (isTouchToMouseEnabled_ && touchEvent->touch.id == 0)
			mouseEvent = newMouseEvent(touchEvent->touch.x, touchEvent->touch.y, GINPUT_LEFT_BUTTON);
		
		if (mouseTouchOrder_ == 0)
		{
			if (mouseEvent)
			{
				gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_DOWN_EVENT, mouseEvent, 0, this);
				deleteMouseEvent(mouseEvent);
			}
			gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_BEGIN_EVENT, touchEvent, 0, this);
			deleteTouchEvent(touchEvent);
		}
		else
		{
			gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_BEGIN_EVENT, touchEvent, 0, this);
			deleteTouchEvent(touchEvent);
			if (mouseEvent)
			{
				gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_DOWN_EVENT, mouseEvent, 0, this);
				deleteMouseEvent(mouseEvent);
			}
		}
	}

	void touchesMove(int size, int *id, int *x, int *y, float *pressure)
	{
		for (int i = 0; i < size; ++i)
		{
			ginput_TouchEvent *touchEvent = newTouchEvent(size);
			
			touchEvent->touch.x = x[i];
			touchEvent->touch.y = y[i];
            touchEvent->touch.pressure = pressure[i];
            touchEvent->touch.touchType = 0;
			touchEvent->touch.id = id[i];
			
			for (int j = 0; j < size; ++j)
			{
				touchEvent->allTouches[j].x = x[j];
				touchEvent->allTouches[j].y = y[j];
	            touchEvent->allTouches[j].pressure = pressure[j];
	            touchEvent->allTouches[j].touchType = 0;
				touchEvent->allTouches[j].id = id[j];
			}
			
			ginput_MouseEvent *mouseEvent = NULL;
			if (isTouchToMouseEnabled_ && touchEvent->touch.id == 0)
				mouseEvent = newMouseEvent(touchEvent->touch.x, touchEvent->touch.y, GINPUT_NO_BUTTON);

			if (mouseTouchOrder_ == 0)
			{
				if (mouseEvent)
				{
					gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_MOVE_EVENT, mouseEvent, 0, this);
					deleteMouseEvent(mouseEvent);
				}
				gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_MOVE_EVENT, touchEvent, 0, this);
				deleteTouchEvent(touchEvent);
			}
			else
			{
				gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_MOVE_EVENT, touchEvent, 0, this);
				deleteTouchEvent(touchEvent);
				if (mouseEvent)
				{
					gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_MOVE_EVENT, mouseEvent, 0, this);
					deleteMouseEvent(mouseEvent);
				}
			}
		}
	}

	void touchEnd(int size, int *id, int *x, int *y, float *pressure, int actionIndex)
	{
		ginput_TouchEvent *touchEvent = newTouchEvent(size);

		touchEvent->touch.x = x[actionIndex];
		touchEvent->touch.y = y[actionIndex];
        touchEvent->touch.pressure = pressure[actionIndex];
        touchEvent->touch.touchType = 0;
		touchEvent->touch.id = id[actionIndex];
		
		for (int i = 0; i < size; ++i)
		{
			touchEvent->allTouches[i].x = x[i];
			touchEvent->allTouches[i].y = y[i];
            touchEvent->allTouches[i].pressure = pressure[i];
            touchEvent->allTouches[i].touchType = 0;
			touchEvent->allTouches[i].id = id[i];
		}

		ginput_MouseEvent *mouseEvent = NULL;
		if (isTouchToMouseEnabled_ && touchEvent->touch.id == 0)
			mouseEvent = newMouseEvent(touchEvent->touch.x, touchEvent->touch.y, GINPUT_LEFT_BUTTON);

		if (mouseTouchOrder_ == 0)
		{
			if (mouseEvent)
			{
				gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_UP_EVENT, mouseEvent, 0, this);
				deleteMouseEvent(mouseEvent);
			}
			gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_END_EVENT, touchEvent, 0, this);
			deleteTouchEvent(touchEvent);
		}
		else
		{
			gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_END_EVENT, touchEvent, 0, this);
			deleteTouchEvent(touchEvent);
			if (mouseEvent)
			{
				gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_UP_EVENT, mouseEvent, 0, this);
				deleteMouseEvent(mouseEvent);
			}
		}
	}

	void touchesCancel(int size, int *id, int *x, int *y, float *pressure)
	{
		for (int i = 0; i < size; ++i)
		{
			ginput_TouchEvent *touchEvent = newTouchEvent(size);
			
			touchEvent->touch.x = x[i];
			touchEvent->touch.y = y[i];
            touchEvent->touch.pressure = pressure[i];
            touchEvent->touch.touchType = 0;
			touchEvent->touch.id = id[i];
			
			for (int j = 0; j < size; ++j)
			{
				touchEvent->allTouches[j].x = x[j];
				touchEvent->allTouches[j].y = y[j];
	            touchEvent->allTouches[j].pressure = pressure[j];
	            touchEvent->allTouches[j].touchType = 0;
				touchEvent->allTouches[j].id = id[j];
			}
			
			ginput_MouseEvent *mouseEvent = NULL;
			if (isTouchToMouseEnabled_ && touchEvent->touch.id == 0)
				mouseEvent = newMouseEvent(touchEvent->touch.x, touchEvent->touch.y, GINPUT_LEFT_BUTTON);

			if (mouseTouchOrder_ == 0)
			{
				if (mouseEvent)
				{
					gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_UP_EVENT, mouseEvent, 0, this);
					deleteMouseEvent(mouseEvent);
				}
				gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_CANCEL_EVENT, touchEvent, 0, this);
				deleteTouchEvent(touchEvent);
			}
			else
			{
				gevent_EnqueueEvent(gid_, callback_s, GINPUT_TOUCH_CANCEL_EVENT, touchEvent, 0, this);
				deleteTouchEvent(touchEvent);
				if (mouseEvent)
				{
					gevent_EnqueueEvent(gid_, callback_s, GINPUT_MOUSE_UP_EVENT, mouseEvent, 0, this);
					deleteMouseEvent(mouseEvent);
				}
			}
		}
	}
	
private:
	ginput_TouchEvent *newTouchEvent(size_t allTouchesCount)
	{
		pthread_mutex_lock(&touchPoolMutex_);
		std::vector<ginput_TouchEvent*> &pool = touchPool1_[allTouchesCount];

        ginput_TouchEvent *event;

        if (pool.empty())
        {
            event = new ginput_TouchEvent;
			event->allTouches = new ginput_Touch[allTouchesCount];
        }
        else
        {
            event = pool.back();
            pool.pop_back();
        }
		pthread_mutex_unlock(&touchPoolMutex_);

        event->allTouchesCount = allTouchesCount;

        return event;
	}

	void deleteTouchEvent(ginput_TouchEvent *event)
	{
		pthread_mutex_lock(&touchPoolMutex_);
		touchPool2_[event->allTouchesCount].push_back(event);
		pthread_mutex_unlock(&touchPoolMutex_);
	}

    ginput_MouseEvent *newMouseEvent(int x, int y, int button)
    {
        pthread_mutex_lock(&mousePoolMutex_);
        ginput_MouseEvent *event;

        if (mousePool1_.empty())
        {
            event = new ginput_MouseEvent;
        }
        else
        {
            event = mousePool1_.back();
            mousePool1_.pop_back();
        }
        pthread_mutex_unlock(&mousePoolMutex_);

        event->x = x;
        event->y = y;
        event->button = button;
        event->wheel = 0;

        return event;
    }

    void deleteMouseEvent(ginput_MouseEvent *event)
    {
        pthread_mutex_lock(&mousePoolMutex_);
        mousePool2_.push_back(event);
        pthread_mutex_unlock(&mousePoolMutex_);
    }

	std::map<size_t, std::vector<ginput_TouchEvent*> > touchPool1_;
	std::map<size_t, std::vector<ginput_TouchEvent*> > touchPool2_;
    std::vector<ginput_MouseEvent*> mousePool1_;
    std::vector<ginput_MouseEvent*> mousePool2_;
    pthread_mutex_t touchPoolMutex_;
    pthread_mutex_t mousePoolMutex_;

    int isMouseToTouchEnabled_;
    int isTouchToMouseEnabled_;
    int mouseTouchOrder_;

public:
    int keyDown(int realCode, int repeatCount)
    {
        int keyCode = convertKeyCode(realCode);

		if (repeatCount == 0)
		{
			ginput_KeyEvent *event = newKeyEvent(keyCode, realCode);
			gevent_EnqueueEvent(gid_, callback_s, GINPUT_KEY_DOWN_EVENT, event, 0, this);
			deleteKeyEvent(event);
		}
		
		return 1;
    }

    int keyUp(int realCode, int repeatCount)
    {
        int keyCode = convertKeyCode(realCode);

		if (repeatCount == 0)
		{
			ginput_KeyEvent *event = newKeyEvent(keyCode, realCode);
			gevent_EnqueueEvent(gid_, callback_s, GINPUT_KEY_UP_EVENT, event, 0, this);
			deleteKeyEvent(event);
		}

		return 1;
    }
	
    void keyChar(const char *keychar)
    {
        ginput_KeyEvent *event = newKeyEvent(0,0);
    	if (strlen(keychar)<(sizeof(event->charCode)))
    	{
    		strcpy(event->charCode,keychar);
            gevent_EnqueueEvent(gid_, callback_s, GINPUT_KEY_CHAR_EVENT, event, 0, this);
    	}
        deleteKeyEvent(event);
    }


private:
    ginput_KeyEvent *newKeyEvent(int keyCode, int realCode)
    {
		pthread_mutex_lock(&keyPoolMutex_);
        ginput_KeyEvent *event;

        if (keyPool1_.empty())
        {
            event = new ginput_KeyEvent;
        }
        else
        {
            event = keyPool1_.back();
            keyPool1_.pop_back();
        }
		pthread_mutex_unlock(&keyPoolMutex_);

        event->keyCode = keyCode;
        event->realCode = realCode;

        return event;
    }

    void deleteKeyEvent(ginput_KeyEvent *event)
    {
		pthread_mutex_lock(&keyPoolMutex_);
        keyPool2_.push_back(event);
		pthread_mutex_unlock(&keyPoolMutex_);
    }

    int convertKeyCode(int keyCode)
    {
        std::map<int, int>::const_iterator iter = keyMap_.find(keyCode);

        if (iter == keyMap_.end())
            return 0;

        return iter->second;
    }	
    std::vector<ginput_KeyEvent*> keyPool1_;
    std::vector<ginput_KeyEvent*> keyPool2_;
    pthread_mutex_t keyPoolMutex_;
    std::map<int, int> keyMap_;

private:
	void startAccelerometerHelper()
	{
		JNIEnv *env = g_getJNIEnv();

		jclass cls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
		env->CallStaticVoidMethod(cls, env->GetStaticMethodID(cls, "startAccelerometer_s", "()V"));
		env->DeleteLocalRef(cls);
	}

	void stopAccelerometerHelper()
	{
		JNIEnv *env = g_getJNIEnv();

		jclass cls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
		env->CallStaticVoidMethod(cls, env->GetStaticMethodID(cls, "stopAccelerometer_s", "()V"));
		env->DeleteLocalRef(cls);
	}
	
	void startGyroscopeHelper()
	{
		JNIEnv *env = g_getJNIEnv();

		jclass cls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
		env->CallStaticVoidMethod(cls, env->GetStaticMethodID(cls, "startGyroscope_s", "()V"));
		env->DeleteLocalRef(cls);
	}

	void stopGyroscopeHelper()
	{
		JNIEnv *env = g_getJNIEnv();

		jclass cls = env->FindClass("com/giderosmobile/android/player/GiderosApplication");
		env->CallStaticVoidMethod(cls, env->GetStaticMethodID(cls, "stopGyroscope_s", "()V"));
		env->DeleteLocalRef(cls);
	}
	
private:
	int accelerometerStartCount_;
	int gyroscopeStartCount_;

public:
    g_id addCallback(gevent_Callback callback, void *udata)
    {
        return callbackList_.addCallback(callback, udata);
    }

    void removeCallback(gevent_Callback callback, void *udata)
    {
        callbackList_.removeCallback(callback, udata);
    }

    void removeCallbackWithGid(g_id gid)
    {
        callbackList_.removeCallbackWithGid(gid);
    }

    static void callback_s(int type, void *event, void *udata)
    {
        static_cast<GGInputManager*>(udata)->callbackList_.dispatchEvent(type, event);
    }

private:
    gevent_CallbackList callbackList_;
    g_id gid_;
};

static GGInputManager *s_manager = NULL;

extern "C" {

void ginput_init()
{
    s_manager = new GGInputManager;    
}

void ginput_cleanup()
{
    delete s_manager;
    s_manager = NULL;
}

int ginput_isAccelerometerAvailable()
{
    return s_manager->isAccelerometerAvailable();
}

void ginput_startAccelerometer()
{
    s_manager->startAccelerometer();
}

void ginput_stopAccelerometer()
{
    s_manager->stopAccelerometer();    
}

void ginput_getAcceleration(double *x, double *y, double *z)
{
    s_manager->getAcceleration(x, y, z);
}


int ginput_isGyroscopeAvailable()
{
    return s_manager->isGyroscopeAvailable();
}

void ginput_startGyroscope()
{
    s_manager->startGyroscope();
}

void ginput_stopGyroscope()
{
    s_manager->stopGyroscope();    
}

void ginput_getGyroscopeRotationRate(double *x, double *y, double *z)
{
    s_manager->getGyroscopeRotationRate(x, y, z);
}

void ginputp_touchBegin(int size, int *id, int *x, int *y, float *pressure, int actionIndex)
{
    if (s_manager)
        s_manager->touchBegin(size, id, x, y, pressure, actionIndex);
}
void ginputp_touchesMove(int size, int *id, int *x, int *y, float *pressure)
{
    if (s_manager)
        s_manager->touchesMove(size, id, x, y, pressure);
}
void ginputp_touchEnd(int size, int *id, int *x, int *y, float *pressure, int actionIndex)
{
    if (s_manager)
        s_manager->touchEnd(size, id, x, y, pressure, actionIndex);
}
void ginputp_touchesCancel(int size, int *id, int *x, int *y, float *pressure)
{
    if (s_manager)
        s_manager->touchesCancel(size, id, x, y, pressure);
}

g_bool ginputp_keyDown(int keyCode, int repeatCount)
{
    if (s_manager)
        return s_manager->keyDown(keyCode, repeatCount);
    return g_false;
}

g_bool ginputp_keyUp(int keyCode, int repeatCount)
{
    if (s_manager)
        return s_manager->keyUp(keyCode, repeatCount);
    return g_false;
}

void ginputp_keyChar(const char *keyChar)
{
    if (s_manager)
        s_manager->keyChar(keyChar);
}

void ginput_setMouseToTouchEnabled(int enabled)
{
    s_manager->setMouseToTouchEnabled(enabled);
}

void ginput_setTouchToMouseEnabled(int enabled)
{
    s_manager->setTouchToMouseEnabled(enabled);
}

void ginput_setMouseTouchOrder(int order)
{
    s_manager->setMouseTouchOrder(order);
}

g_id ginput_addCallback(gevent_Callback callback, void *udata)
{
    return s_manager->addCallback(callback, udata);
}

void ginput_removeCallback(gevent_Callback callback, void *udata)
{
    s_manager->removeCallback(callback, udata);
}

void ginput_removeCallbackWithGid(g_id gid)
{
    s_manager->removeCallbackWithGid(gid);
}

}
