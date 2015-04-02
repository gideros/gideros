#include "gmicrophone.h"
#include <AudioToolbox/AudioToolbox.h>
#include <map>
#include <glog.h>
#include <gapplication.h>
#include <AVFoundation/AVFoundation.h>

#define BUFFER_COUNT 4
    
static void calculateAmplitudeData(int numChannels, int bitsPerSample, gmicrophone_DataAvailableEvent *event)
{
    int n = event->sampleCount * numChannels;
    
    if (bitsPerSample == 8)
    {
        const unsigned char *data = (unsigned char*)event->data;
        
        float sum = 0;
        float peak = 0;
        
        for (int i = 0; i < n; ++i)
        {
            int d1 = abs((int)data[i] - 128);
            float d2 = d1 / 128.f;
            peak = std::max(peak, d2);
            sum += d2 * d2;
        }
        
        event->peakAmplitude = peak;
        event->averageAmplitude = sqrt(sum / n);
    }
    else if (bitsPerSample == 16)
    {
        const short *data = (short*)event->data;
        
        float sum = 0;
        float peak = 0;
        
        for (int i = 0; i < n; ++i)
        {
            int d1 = abs(data[i]);
            float d2 = d1 / 32768.f;
            peak = std::max(peak, d2);
            sum += d2 * d2;
        }
        
        event->peakAmplitude = peak;
        event->averageAmplitude = sqrt(sum / n);
    }
}

class GMicrophoneManager
{
public:
    GMicrophoneManager()
    {
        active_ = [UIApplication sharedApplication].applicationState == UIApplicationStateActive;

        microphoneCount_ = 0;
        currentAudioSessionCategory_ = nil;

        gapplication_addCallback(applicationCallback_s, this);
    }

    ~GMicrophoneManager()
    {
        gapplication_removeCallback(applicationCallback_s, this);
        
        while (!microphones_.empty())
            Delete(microphones_.begin()->first);
    }

    g_id Create(const char *deviceName, int numChannels, int sampleRate, int bitsPerSample, gmicrophone_Error *error)
    {
        if (numChannels != 1 && numChannels != 2)
        {
            if (error)
                *error = GMICROPHONE_UNSUPPORTED_FORMAT;
            return 0;
        }
        
        if (bitsPerSample != 8 && bitsPerSample != 16)
        {
            if (error)
                *error = GMICROPHONE_UNSUPPORTED_FORMAT;
            return 0;
        }
        
        if (sampleRate < 4000 || sampleRate > 44100)
        {
            if (error)
                *error = GMICROPHONE_UNSUPPORTED_FORMAT;
            return 0;
        }

        Microphone *microphone = new Microphone;

        AudioStreamBasicDescription &format = microphone->format;
        format.mSampleRate = sampleRate;
        format.mFormatID = kAudioFormatLinearPCM;
        format.mFormatFlags = kLinearPCMFormatFlagIsPacked;
        if (bitsPerSample == 16)
            format.mFormatFlags |= kLinearPCMFormatFlagIsSignedInteger;
        format.mBytesPerPacket = ((bitsPerSample + 7) / 8) * numChannels;
        format.mFramesPerPacket = 1;
        format.mBytesPerFrame = format.mBytesPerPacket;
        format.mChannelsPerFrame = numChannels;
        format.mBitsPerChannel = bitsPerSample;
        format.mReserved = 0;
        
        OSStatus status = AudioQueueNewInput(&format,
                                             handleInputBuffer_s,
                                             microphone,
                                             CFRunLoopGetCurrent(),
                                             kCFRunLoopCommonModes,
                                             0,
                                             &microphone->queue);
        if (status)
        {
            if (error)
                *error = GMICROPHONE_CANNOT_OPEN_DEVICE;
            delete microphone;
            return 0;
        }
        
		int bytesPerSample = ((bitsPerSample + 7) / 8) * numChannels;
        
		int bufferSize = (sampleRate / 10) * bytesPerSample;

        for(int i = 0; i < BUFFER_COUNT; i++)
        {
            OSStatus status = AudioQueueAllocateBuffer(microphone->queue, bufferSize, &microphone->buffers[i]);
            if (status)
                glog_e("Error while calling AudioQueueAllocateBuffer.");
        }

        g_id gid = g_NextId();
        
        microphone->gid = gid;
        
        microphones_[gid] = microphone;
        
        if (error)
            *error = GMICROPHONE_NO_ERROR;
        
        return gid;
    }

    void Delete(g_id microphone)
    {
        std::map<g_id, Microphone*>::iterator iter = microphones_.find(microphone);
        
        if (iter == microphones_.end())
            return;
        
        Microphone *microphone2 = iter->second;
        
        Stop(microphone);

        for(int i = 0; i < BUFFER_COUNT; i++)
            AudioQueueFreeBuffer(microphone2->queue, microphone2->buffers[i]);
        
        AudioQueueDispose(microphone2->queue, YES);

        delete microphone2;
        
        microphones_.erase(microphone);
    }

    void Start(g_id microphone)
    {
        std::map<g_id, Microphone*>::iterator iter = microphones_.find(microphone);
        
        if (iter == microphones_.end())
            return;
        
        Microphone *microphone2 = iter->second;
        
		if (active_ == false && microphone2->toBeResumed == false) {
			microphone2->toBeResumed = true;
			return;
		}
        
        startMicrophone(microphone2);
    }

    void Stop(g_id microphone)
    {
        std::map<g_id, Microphone*>::iterator iter = microphones_.find(microphone);
        
        if (iter == microphones_.end())
            return;
        
        Microphone *microphone2 = iter->second;

		if (active_ == false && microphone2->toBeResumed == true) {
			microphone2->toBeResumed = false;
			return;
		}
        
        stopMicrophone(microphone2);
            
        gevent_RemoveEventsWithGid(microphone);
    }

    g_id AddCallback(g_id microphone, gevent_Callback callback, void *udata)
    {
        std::map<g_id, Microphone*>::iterator iter = microphones_.find(microphone);
        
        if (iter == microphones_.end())
            return 0;
        
        Microphone *microphone2 = iter->second;
        
        return microphone2->addCallback(callback, udata);
    }
    
    void RemoveCallback(g_id microphone, gevent_Callback callback, void *udata)
    {
        std::map<g_id, Microphone*>::iterator iter = microphones_.find(microphone);
        
        if (iter == microphones_.end())
            return;
        
        Microphone *microphone2 = iter->second;
        
        microphone2->removeCallback(callback, udata);
    }
    
    void RemoveCallbackWithId(g_id microphone, g_id callback)
    {
        std::map<g_id, Microphone*>::iterator iter = microphones_.find(microphone);
        
        if (iter == microphones_.end())
            return;
        
        Microphone *microphone2 = iter->second;
        
        microphone2->removeCallbackWithId(callback);
    }
    
private:    
    static void applicationCallback_s(int type, void *event, void *udata)
    {
        GMicrophoneManager *manager = static_cast<GMicrophoneManager*>(udata);
        if (type == GAPPLICATION_PAUSE_EVENT)
            manager->pause();
        else if (type == GAPPLICATION_RESUME_EVENT)
            manager->resume();
    }

    void pause()
    {
        std::map<g_id, Microphone*>::iterator iter, e = microphones_.end();
        for (iter = microphones_.begin(); iter != e; ++iter)
        {
            Microphone *microphone = iter->second;
            if (microphone->recording)
            {
                stopMicrophone(microphone);
                microphone->toBeResumed = true;
            }
        }
        
        active_ = false;
    }
    
    void resume()
    {
        std::map<g_id, Microphone*>::iterator iter, e = microphones_.end();
        for (iter = microphones_.begin(); iter != e; ++iter)
        {
            Microphone *microphone = iter->second;
            if (microphone->toBeResumed)
            {
                startMicrophone(microphone);
                microphone->toBeResumed = false;
            }
        }
        
        active_ = true;
    }
    
private:
    static void handleInputBuffer_s(void *inUserData,
                                    AudioQueueRef inAQ,
                                    AudioQueueBufferRef inBuffer,
                                    const AudioTimeStamp *inStartTime,
                                    UInt32 inNumberPacketDescriptions,
                                    const AudioStreamPacketDescription *inPacketDescs)
    {
        static_cast<Microphone*>(inUserData)->handleInputBuffer(inBuffer);
    }

private:
    struct Microphone
    {
        Microphone() :
            recording(false),
            toBeResumed(false)
        {
            
        }

        g_id addCallback(gevent_Callback callback, void *udata)
        {
            return callbackList_.addCallback(callback, udata);
        }
        
        void removeCallback(gevent_Callback callback, void *udata)
        {
            callbackList_.removeCallback(callback, udata);
        }
        
        void removeCallbackWithId(g_id callback)
        {
            callbackList_.removeCallbackWithGid(callback);
        }
        
        g_id gid;
        AudioStreamBasicDescription format;
		AudioQueueRef queue;
        AudioQueueBufferRef buffers[BUFFER_COUNT];
        bool recording;
        bool toBeResumed;
        
        void handleInputBuffer(AudioQueueBufferRef inBuffer)
        {
            size_t structSize = sizeof(gmicrophone_DataAvailableEvent);
            size_t dataSize = inBuffer->mAudioDataByteSize;
            
            if (dataSize != 0)
            {
                gmicrophone_DataAvailableEvent *event = (gmicrophone_DataAvailableEvent*)malloc(structSize + dataSize);
                
                event->microphone = gid;
                event->data = (char*)event + structSize;
                event->sampleCount = dataSize / format.mBytesPerFrame;
                
                memcpy(event->data, inBuffer->mAudioData, dataSize);
                
                calculateAmplitudeData(format.mChannelsPerFrame, format.mBitsPerChannel, event);

                gevent_EnqueueEvent(gid, callback_s, GMICROPHONE_DATA_AVAILABLE_EVENT, event, 1, this);
            }
            
            AudioQueueEnqueueBuffer(queue, inBuffer, 0, NULL);
        }
        
    private:
        static void callback_s(int type, void *event, void *udata)
        {
            static_cast<Microphone*>(udata)->callback(type, event);
        }
        
        void callback(int type, void *event)
        {
            callbackList_.dispatchEvent(type, event);
        }
    
    private:
        gevent_CallbackList callbackList_;
    };
    
    std::map<g_id, Microphone*> microphones_;
    bool active_;
    int microphoneCount_;
    NSString *currentAudioSessionCategory_;
    
private:
    void startMicrophone(Microphone *microphone)
    {
        if (microphone->recording)
            return;
        
        if (++microphoneCount_ == 1)
        {
            AVAudioSession *audioSession = [AVAudioSession sharedInstance];
            currentAudioSessionCategory_ = audioSession.category;
            [audioSession setCategory:AVAudioSessionCategoryPlayAndRecord error:nil];
        }

        for (int i = 0; i < BUFFER_COUNT; ++i)
        {
            OSStatus status = AudioQueueEnqueueBuffer(microphone->queue, microphone->buffers[i], 0, NULL);
            if (status)
                glog_e("Error while calling AudioQueueEnqueueBuffer.");
        }
        
        OSStatus status = AudioQueueStart(microphone->queue, NULL);
        if (status)
            glog_e("Error while calling AudioQueueStart.");
        microphone->recording = true;
    }
    
    void stopMicrophone(Microphone *microphone)
    {
        if (!microphone->recording)
            return;

        OSStatus status = AudioQueueStop(microphone->queue, YES);
        if (status)
            glog_e("Error while calling AudioQueueStop.");
        microphone->recording = false;

        if (--microphoneCount_ == 0)
        {
            [[AVAudioSession sharedInstance] setCategory:currentAudioSessionCategory_ error:nil];
        }
    }
};

static GMicrophoneManager *s_manager = NULL;

extern "C" {

void gmicrophone_Init()
{
    s_manager = new GMicrophoneManager;
}

void gmicrophone_Cleanup()
{
    delete s_manager;
    s_manager = NULL;
}

g_id gmicrophone_Create(const char *deviceName, int numChannels, int sampleRate, int bitsPerSample, gmicrophone_Error *error)
{
    return s_manager->Create(deviceName, numChannels, sampleRate, bitsPerSample, error);
}

void gmicrophone_Delete(g_id microphone)
{
    s_manager->Delete(microphone);
}

void gmicrophone_Start(g_id microphone)
{
    s_manager->Start(microphone);
}

void gmicrophone_Stop(g_id microphone)
{
    s_manager->Stop(microphone);
}

g_id gmicrophone_AddCallback(g_id microphone, gevent_Callback callback, void *udata)
{
    return s_manager->AddCallback(microphone, callback, udata);
}

void gmicrophone_RemoveCallback(g_id microphone, gevent_Callback callback, void *udata)
{
    s_manager->RemoveCallback(microphone, callback, udata);
}

void gmicrophone_RemoveCallbackWithId(g_id microphone, g_id callback)
{
    s_manager->RemoveCallbackWithId(microphone, callback);
}

}