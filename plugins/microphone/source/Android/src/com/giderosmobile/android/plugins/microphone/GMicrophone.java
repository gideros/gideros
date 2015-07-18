package com.giderosmobile.android.plugins.microphone;

import java.util.HashMap;

import android.app.Activity;
import android.media.AudioFormat;
import android.media.AudioRecord;
import android.media.MediaRecorder;

class GMicrophone {
	private static GMicrophone sInstance;
	private static long sData;
	private static boolean sActive = false;

	private static int GMICROPHONE_NO_ERROR = 0;
	private static int GMICROPHONE_CANNOT_OPEN_DEVICE = 1;
	private static int GMICROPHONE_UNSUPPORTED_FORMAT = 2;

	public static void onCreate(Activity activity) {
	}

	synchronized public static void onPause() {
		if (sInstance != null) {
			for (Microphone microphone : sInstance.mMicrophones.values()) {
				if (microphone.thread != null) {
					stopThread(microphone);
					microphone.toBeResumed = true;
				}
			}
		}
		sActive = false;
	}

	synchronized public static void onResume() {
		if (sInstance != null) {
			for (Microphone microphone : sInstance.mMicrophones.values()) {
				if (microphone.toBeResumed) {
					startThread(microphone);
					microphone.toBeResumed = false;
				}
			}
		}
		sActive = true;
	}

	synchronized public static void Init(long udata) {
		sData = udata;
		sInstance = new GMicrophone();
	}

	synchronized public static void Cleanup() {
		while (!sInstance.mMicrophones.isEmpty())
			deleteNoSync(sInstance.mMicrophones.entrySet().iterator().next().getKey());

		sData = 0;
		sInstance = null;
	}

	synchronized public static int Create(long id, int numChannels, int sampleRate, int bitsPerSample) {
		int channelConfig;
		if (numChannels == 1) {
			channelConfig = AudioFormat.CHANNEL_IN_MONO;
		} else if (numChannels == 2) {
			channelConfig = AudioFormat.CHANNEL_IN_STEREO;
		} else {
			return GMICROPHONE_UNSUPPORTED_FORMAT;
		}

		if (sampleRate < 4000 && sampleRate > 44100) {
			return GMICROPHONE_UNSUPPORTED_FORMAT;
		}

		int audioFormat;
		if (bitsPerSample == 8) {
			audioFormat = AudioFormat.ENCODING_PCM_8BIT;
		} else if (bitsPerSample == 16) {
			audioFormat = AudioFormat.ENCODING_PCM_16BIT;
		} else {
			return GMICROPHONE_UNSUPPORTED_FORMAT;
		}

		int bytesPerSample = ((bitsPerSample + 7) / 8) * numChannels;

		int bufferSize = (sampleRate / 10) * bytesPerSample;

		AudioRecord audioRecord;
		try {
			audioRecord = new AudioRecord(MediaRecorder.AudioSource.DEFAULT, sampleRate, channelConfig, audioFormat, bufferSize);
		} catch (IllegalArgumentException e) {
			return GMICROPHONE_UNSUPPORTED_FORMAT;
		}

		if (audioRecord.getState() == AudioRecord.STATE_UNINITIALIZED)
			return GMICROPHONE_CANNOT_OPEN_DEVICE;

		sInstance.mMicrophones.put(id, new Microphone(id, audioRecord, sampleRate, channelConfig, audioFormat, bufferSize, sData));

		return GMICROPHONE_NO_ERROR;
	}

	synchronized public static void Delete(long id) {
		deleteNoSync(id);
	}

	synchronized public static void Start(long id) {
		Microphone microphone = sInstance.mMicrophones.get(id);
		if (microphone == null)
			return;

		if (sActive == false && microphone.toBeResumed == false) {
			microphone.toBeResumed = true;
			return;
		}

		startThread(microphone);
	}

	synchronized public static void Stop(long id) {
		Microphone microphone = sInstance.mMicrophones.get(id);
		if (microphone == null)
			return;

		if (sActive == false && microphone.toBeResumed == true) {
			microphone.toBeResumed = false;
			return;
		}

		stopThread(microphone);
	}

	private static void deleteNoSync(long id) {
		Microphone microphone = sInstance.mMicrophones.get(id);
		if (microphone == null)
			return;

		stopThread(microphone);

		sInstance.mMicrophones.remove(id);
	}

	private static void startThread(Microphone microphone) {
		if (microphone.thread != null)
			return;

		microphone.exit = false;
		microphone.thread = new Thread(microphone);
		microphone.thread.start();
	}

	private static void stopThread(Microphone microphone) {
		if (microphone.thread == null)
			return;

		microphone.exit = true;
		try {
			microphone.thread.join();
		} catch (InterruptedException e) {
		}
		microphone.thread = null;
	}

	static class Microphone implements Runnable {
		public long id;
		public AudioRecord audioRecord;
		int sampleRate, channelConfig, audioFormat, bufferSize;
		public long data;
		public volatile boolean exit;
		public boolean toBeResumed;
		public Thread thread;

		public Microphone(long id, AudioRecord audioRecord, int sampleRate, int channelConfig, int audioFormat, int bufferSize, long data) {
			this.id = id;
			this.audioRecord = audioRecord;
			this.sampleRate = sampleRate;
			this.channelConfig = channelConfig;
			this.audioFormat = audioFormat;
			this.bufferSize = bufferSize;
			this.data = data;
			this.toBeResumed = false;
		}

		@Override
		public void run() {
			if (audioRecord == null)
				audioRecord = new AudioRecord(MediaRecorder.AudioSource.DEFAULT, sampleRate, channelConfig, audioFormat, bufferSize);
			
			if (audioRecord.getState() == AudioRecord.STATE_UNINITIALIZED) {
				while (!exit) {
					try {
						Thread.sleep(100);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
				}

				audioRecord.release();
				audioRecord = null;

				return;
			}			
			
			byte[] audioDataByte = null;
			short[] audioDataShort = null;

			int bufferSize = (audioRecord.getSampleRate() / 10) * audioRecord.getChannelCount();

			if (audioRecord.getAudioFormat() == AudioFormat.ENCODING_PCM_8BIT) {
				audioDataByte = new byte[bufferSize];
			} else {
				audioDataShort = new short[bufferSize];
			}

			audioRecord.startRecording();

			while (!exit) {
				if (audioRecord.getAudioFormat() == AudioFormat.ENCODING_PCM_8BIT) {
					int size = audioRecord.read(audioDataByte, 0, bufferSize);
					if (size != 0)
						onDataAvailableByte(id, audioDataByte, size, data);
				} else {
					int size = audioRecord.read(audioDataShort, 0, bufferSize);
					if (size != 0)
						onDataAvailableShort(id, audioDataShort, size, data);
				}
			}

			audioRecord.stop();
			audioRecord.release();
			audioRecord = null;
		}
	}

	native private static void onDataAvailableByte(long id, byte[] audioData, int size, long data);

	native private static void onDataAvailableShort(long id, short[] audioData, int size, long data);

	private HashMap<Long, Microphone> mMicrophones = new HashMap<Long, Microphone>();
}
