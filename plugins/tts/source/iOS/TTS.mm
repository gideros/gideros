#include "gideros.h"
#include "lua.h"
#include "lauxlib.h"
#include "gtts.h"

#import <AVFoundation/AVSpeechSynthesis.h>

/* 
 require "tts"
 

 local tts = TTS.new("en-us") OR TTS.new({language="en-us", speed=1, pitch=1})  OR TTS.new("en-us",1,1) OR TTS.new()
 tts:speak("utterance",utteranceId)
 
 --************** Additional methods *******
 tts:stop()
 tts:setSpeed(1) --Lua input changed to 1 to be consistent with Android. ios native is 0.5
 tts:setPitch(1)
 tts:setVolume(1)
 tts:setLanguage("en-gb")
 tts:setVoiceIdentifier("com.apple.ttsbundle.siri_female_en-GB_premium")
 
 tts:getSpeed()
 tts:getPitch()
 tts:getVolume()
 tts:getVoicesInstalled() - returns  array of key values for each voice: name, identifier, quality, language
 
 tts:addEventListener(Event.TTS_UTTERANCE_COMPLETE, ...)
 tts:addEventListener(Event.TTS_UTTERANCE_STARTED, ...)
 
*/

@interface TTSSpeechUtterance : AVSpeechUtterance
@property (nonatomic, retain ) NSString * utteranceId;
@end

@implementation TTSSpeechUtterance
@end

@interface TTSSpeechSynthesizer : AVSpeechSynthesizer
@property (nonatomic, retain ) NSString * language;
@property (nonatomic, retain ) NSString * voiceIdentifier;
@property (nonatomic, retain ) AVSpeechSynthesisVoice * voice;
@property (nonatomic) float  pitch;
@property (nonatomic) float  rate;
@property (nonatomic) float  volume;
@property (nonatomic) bool   stopped;
@end

@implementation TTSSpeechSynthesizer
@end

class TTS;
@interface TTSDelegate : NSObject<AVSpeechSynthesizerDelegate>
{
}

- (id)initWithTTS:(TTS *)tts;

@property (nonatomic, assign) TTS *tts;
@end

class TTS : public GTts
{
public:
    TTS(lua_State *L,const char *lang,float speed,float pitch);
    ~TTS();
    bool SetLanguage(const char *lang);
    bool SetPitch(float pitch);
    bool SetSpeed(float speed);
    void Stop();
    void Shutdown();
    bool Speak(const char *text,const char *utteranceId);
    float GetSpeed();
    float GetPitch();
    float GetVolume();
    bool SetVolume(float v);
    bool SetVoice(const char *v);
    void utteranceComplete(NSString * utteranceId);
    void utteranceStarted(NSString * utteranceId);
    void dispatchEvent(int type, NSString *utteranceId, NSString *state);
private:
    TTSDelegate *delegate_;
    TTSSpeechSynthesizer *synthesizer_;
};


//-----------------------------------

static gevent_Callback callback_;
static g_id gid = g_NextId();

void gtts_Init(gevent_Callback callback)
{
	callback_=callback;
}

void gtts_Cleanup()
{
	gevent_RemoveEventsWithGid(gid);
}


	TTS::TTS(lua_State *L,const char *lang,float speed,float pitch) : GTts(L)
	{
        synthesizer_ = [[TTSSpeechSynthesizer alloc] init];
        synthesizer_.voiceIdentifier=@"com.apple.ttsbundle.siri_female_en-US_compact" ;
        synthesizer_.stopped=false;
        synthesizer_.language= [NSString stringWithUTF8String:((*lang)?lang:"en-US")];
        synthesizer_.rate=speed*0.5;
        synthesizer_.pitch=pitch;
        synthesizer_.volume=1;
        synthesizer_.voice = [AVSpeechSynthesisVoice voiceWithLanguage:synthesizer_.language];
		delegate_ = [[TTSDelegate alloc] initWithTTS:this];
		synthesizer_.delegate = delegate_;
		gevent_EnqueueEvent(gid, callback_, GTts::GTTS_INIT_COMPLETE_EVENT, NULL, 1, this);		
	}

    
    void TTS::Shutdown()
    {
        synthesizer_.stopped=true;
        [synthesizer_ stopSpeakingAtBoundary:AVSpeechBoundaryImmediate];
    }

	TTS::~TTS()
	{
        NSLog(@"~TTS()");
        Shutdown();
		synthesizer_.delegate = nil;
        delegate_.tts = NULL;
        [synthesizer_ release];
		[delegate_ release];
	}
	
	bool TTS::Speak(const char *text,const char *utteranceId)
	{
        synthesizer_.stopped=false;
        
        TTSSpeechUtterance *utterance = [[TTSSpeechUtterance alloc] initWithString:[NSString stringWithUTF8String:text?text:""]];
        utterance.utteranceId= [NSString stringWithUTF8String:utteranceId?utteranceId:""];
        [utterance setRate:synthesizer_.rate];
        [utterance setPitchMultiplier:synthesizer_.pitch];
        [utterance setVolume:synthesizer_.volume];
        utterance.voice = synthesizer_.voice;
        
        [synthesizer_ speakUtterance:utterance];
        return true;
	}
    

    
    void TTS::Stop()
    {
        synthesizer_.stopped=true;
        [synthesizer_ stopSpeakingAtBoundary:AVSpeechBoundaryImmediate];
    }
	
    bool TTS::SetLanguage(const char *lang)
    {
        synthesizer_.language = [NSString stringWithUTF8String:lang];
        synthesizer_.voice=[AVSpeechSynthesisVoice voiceWithLanguage:synthesizer_.language];
        return true;
    }
    
    bool TTS::SetVoice(const char *v)
    {
        synthesizer_.voiceIdentifier = [NSString stringWithUTF8String:v];
        synthesizer_.voice=[AVSpeechSynthesisVoice voiceWithIdentifier:synthesizer_.voiceIdentifier];
        return true;
    }
    
    bool TTS::SetSpeed(float rateReceived)
    {
        synthesizer_.rate = MIN(MAX(rateReceived*0.5,AVSpeechUtteranceMinimumSpeechRate),AVSpeechUtteranceMaximumSpeechRate);
        return true;
    }
    
    bool TTS::SetPitch(float v)
    {
        synthesizer_.pitch = MIN(MAX(v,0.5),2);
        return true;
    }
    
    bool TTS::SetVolume(float v)
    {
        synthesizer_.volume = MIN(MAX(v,0),1);
        return true;
    }
    
    float TTS::GetSpeed()
    {
        return synthesizer_.rate*2;
    }
    
    float TTS::GetPitch()
    {
        return synthesizer_.pitch;
    }
    
    float TTS::GetVolume()
    {
        return synthesizer_.volume;
    }
    
	void TTS::utteranceComplete(NSString * utteranceId)
    {   if (!synthesizer_.stopped){
        NSString *state = @"done";
		dispatchEvent(GTTS_UTTERANCE_COMPLETE_EVENT, utteranceId, state);
        }
	}
    
    void TTS::utteranceStarted(NSString * utteranceId)
    {
        if (!synthesizer_.stopped){
        NSString *state = @"start";
        dispatchEvent(GTTS_UTTERANCE_COMPLETE_EVENT, utteranceId, state);
      //  dispatchEvent(GTTS_UTTERANCE_STARTED_EVENT, utteranceId, state);
        }
    }
	
void TTS::dispatchEvent(int type, NSString *utteranceId, NSString *state)
	{
		const char *cstate= [state UTF8String];
		const char *cutterance=[utteranceId UTF8String];
		char *event=(char *)malloc(strlen(cstate)+strlen(cutterance)+2);
		strcpy(event,cstate);
		strcpy(event+strlen(cstate)+1,cutterance);
		gevent_EnqueueEvent(gid, callback_, type, event, 1, this);
	}
	
@implementation TTSDelegate

@synthesize tts = tts_;


- (id)initWithTTS:(TTS *)tts
{
	if (self = [super init])
	{
        tts_ = tts;
	}
	
	return self;
}

- (void)dealloc
{
   // tts_->shutdown();
    [AVSpeechUtterance release];
    [TTSSpeechSynthesizer release];
    [TTSDelegate release];
    [super dealloc];
}

-(void)speechSynthesizer:(TTSSpeechSynthesizer *)synthesizer didFinishSpeechUtterance:(TTSSpeechUtterance *)utterance {
    NSLog(@"Playback finished");
    tts_->utteranceComplete(utterance.utteranceId);
}

-(void)speechSynthesizer:(TTSSpeechSynthesizer *)synthesizer didStartSpeechUtterance:(TTSSpeechUtterance *)utterance {
    NSLog(@"Playback started");
    tts_->utteranceStarted(utterance.utteranceId);
}

@end

GTts *gtts_Create(lua_State *L,const char *lang,float speed,float pitch)
{
	return new TTS(L,lang,speed,pitch);
}

std::vector<struct VoiceInfo> gtts_GetVoicesInstalled(){
	std::vector<struct VoiceInfo> voices;
    NSArray *allVoices = [AVSpeechSynthesisVoice speechVoices];
    for (AVSpeechSynthesisVoice *voice in allVoices) {
        // NSLog(@"Voice Name: %@, Identifier: %@, Quality: %ld@, Language: %@", voice.name, voice.identifier, (long)voice.quality, voice.language);
        struct VoiceInfo v;
		v.identifier=[voice.identifier UTF8String];
		v.name=[voice.name UTF8String];
        v.quality=voice.quality;
        v.language=[voice.language UTF8String];
        voices.push_back(v);
    }
    return voices;
}
