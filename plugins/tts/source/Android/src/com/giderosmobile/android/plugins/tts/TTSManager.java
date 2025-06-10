package com.giderosmobile.android.plugins.tts;

import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.speech.tts.TextToSpeech;
import android.speech.tts.UtteranceProgressListener;
import android.util.Log;

import java.lang.ref.WeakReference;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Locale;
import java.util.Set;

public class TTSManager {

	private TextToSpeech mTts = null;
	private boolean isLoaded = false;
	private boolean mShutdown = false;
	private String mTtsLanguage = "EN";
	private float mTtsSpeechRate = 1.0f;
	private float mTtsPitch = 1.0f;
	public int ttsIndex;

	private static WeakReference<Activity> sActivity;

	public static void onCreate(Activity activity) {
		sActivity = new WeakReference<Activity>(activity);
	}

	static HashMap<Integer, TTSManager> ttsList = new HashMap<Integer, TTSManager>();
	static int ttsIndexAlloc = 0;

	// on destroy event
	public static void onDestroy() {
		Cleanup();
	}

	public static native void eventTtsInit(int tid);

	public static native void eventTtsError(int tid, String error);

	public static native void eventTtsUtterance(int tid, String state, String utterance);

	public static void Init() {

	}

	public static void Cleanup() {
		for (TTSManager tts : ttsList.values())
			tts.shutdown();
		ttsList.clear();
	}

	public static int Create(String language, float speed, float pitch) {
		TTSManager m = new TTSManager(language, speed, pitch);
		return m.ttsIndex;
	}

	public static void Destroy(int tid) {
		if (ttsList.containsKey(tid))
			ttsList.get(tid).shutdown();
		ttsList.remove(tid);
	}

	public static boolean SetSpeechRate(int tid, float speechRate) {
		return ttsList.get(tid).setSpeechRate(speechRate);
	}

	public static boolean SetPitch(int tid, float pitch) {
		return ttsList.get(tid).setPitch(pitch);
	}

	public static boolean SetLanguage(int tid, String language) {
		return ttsList.get(tid).setLanguage(language);
	}

	public static void Stop(int tid) {
		ttsList.get(tid).stop();
	}

	public static void Shutdown(int tid) {
		ttsList.get(tid).shutdown();
	}

	public static boolean Speak(int tid, String text, String utteranceId) {
		return ttsList.get(tid).speak(text, utteranceId);
	}

	public TTSManager(String language, float speed, float pitch) {
		this.mTtsLanguage = language;
		this.mTtsSpeechRate = speed;
		this.mTtsPitch = pitch;
		ttsIndex = ++ttsIndexAlloc;
		ttsList.put(ttsIndex, this);
		try {
			mTts = new TextToSpeech(sActivity.get(), onInitListener);
            if (android.os.Build.VERSION.SDK_INT>=15)
                mTts.setOnUtteranceProgressListener(new UtteranceProgressListener() {
                @Override
                public void onStart(String utterance) {
                    eventTtsUtterance(ttsIndex,"start",utterance);
                }
                @Override
                public void onDone(String utterance) {
                    eventTtsUtterance(ttsIndex,"done",utterance);
                }
                @Override
                public void onError(String utterance) {
                    eventTtsUtterance(ttsIndex,"error",utterance);
                }
            });
            else
                mTts.setOnUtteranceCompletedListener( new TextToSpeech.OnUtteranceCompletedListener() {
                    @Override
                    public void onUtteranceCompleted(String utterance) {
                        eventTtsUtterance(ttsIndex,"done",utterance);
                    }
                });
		} catch (Exception e) {
			eventTtsError(ttsIndex, "Exception during Initialization:" + e.toString());
			e.printStackTrace();
		}
	}

	// tts engine init listener
	private TextToSpeech.OnInitListener onInitListener = new TextToSpeech.OnInitListener() {
		@Override
		public void onInit(int status) {
			if (mShutdown) {
				eventTtsError(ttsIndex, "TTS shutdown during initialization");
				return;
			}
			if (status == TextToSpeech.SUCCESS) {
				isLoaded = true;
				setLanguage(mTtsLanguage);
				setSpeechRate(mTtsSpeechRate);
				setPitch(mTtsPitch);
				eventTtsInit(ttsIndex);
			} else {
				eventTtsError(ttsIndex, "Initialization failed, code:" + status);
			}
		}
	};

	public boolean setSpeechRate(float speechRate) {
		mTtsSpeechRate = speechRate;
		if (isLoaded) {
			mTts.setSpeechRate(mTtsSpeechRate);
		}
		return true;
	}

	public boolean setPitch(float pitch) {
		mTtsPitch = pitch;
		if (isLoaded) {
			mTts.setPitch(mTtsPitch);
		}
		return true;
	}

	public boolean setLanguage(String language) {
		mTtsLanguage = language;
		if (isLoaded) {
			Locale rl = new Locale(language);
			if (mTts.isLanguageAvailable(rl) >= 0)
				mTts.setLanguage(rl);
			else {
				if (android.os.Build.VERSION.SDK_INT >= 21) {
					if (mTts.getVoice()!=null)
						mTtsLanguage = mTts.getVoice().getLocale().getLanguage();
				}
				else
					mTtsLanguage = mTts.getLanguage().getLanguage();
				eventTtsError(ttsIndex, "Language '" + language + "' is not supported, using '" + mTtsLanguage + "'");
			}
		}
		return true;
	}

	public void stop() {
		mTts.stop();
	}

	public void shutdown() {
		mShutdown = true;
		isLoaded = false;
		mTts.stop();
		mTts.shutdown();
	}

	public boolean speak(String text, String utteranceId) {
		if (isLoaded) {
			if (android.os.Build.VERSION.SDK_INT >= 21)
				return mTts.speak(text, TextToSpeech.QUEUE_FLUSH, null, utteranceId) >= 0;
			else {
				HashMap<String, String> params = new HashMap<String, String>();
				params.put(TextToSpeech.Engine.KEY_PARAM_UTTERANCE_ID, utteranceId);
				return mTts.speak(text, TextToSpeech.QUEUE_FLUSH, params) >= 0;
			}
		}
		return false;
	}
}