package com.giderosmobile.android.player;

import android.media.AudioTrack;
import java.nio.ByteBuffer;

public class AudioDevice implements Runnable
{
	private static int NUMCHANNELS = 2;

	private Thread mThread = null;
	private AudioTrack mTrack = null;
	private boolean mRunning = false;
	private boolean mInitialised = false;

	private static int INFO_SAMPLERATE = 0;
	private static int INFO_DSPBUFFERLENGTH = 1;
	private static int INFO_DSPNUMBUFFERS = 2;
	private static int INFO_MIXERRUNNING = 3;

	public void start()
	{
		if (this.mThread != null)
		{
			stop();
		}

		this.mThread = new Thread(this);
		this.mThread.setPriority(10);

		this.mRunning = true;
		this.mThread.start();
	}

	public void stop()
	{
		while (this.mThread != null)
		{
			this.mRunning = false;
			try
			{
				this.mThread.join();
				this.mThread = null;
			} catch (InterruptedException localInterruptedException)
			{
			}
		}
	}

	public void run()
	{
		ByteBuffer localByteBuffer = null;
		byte[] arrayOfByte = null;

		while (this.mRunning)
		{
			if (!this.mInitialised)
			{
				int i = getInfo(INFO_SAMPLERATE);
				int k = getInfo(INFO_DSPBUFFERLENGTH);
				int m = getInfo(INFO_DSPNUMBUFFERS);
				if (i == 0 || k == 0 || m == 0)
				{
					try { Thread.sleep(1L); } catch (InterruptedException localInterruptedException) {}
					continue;
				}
				
				if (this.mTrack != null)
				{
					this.mTrack.stop();
					this.mTrack.release();
					this.mTrack = null;
				}

				int j = AudioTrack.getMinBufferSize(i, 3, 2);

				if (k * m * 2 * NUMCHANNELS > j)
				{
					j = k * m * 2 * NUMCHANNELS;
				}

				this.mTrack = new AudioTrack(3, i, 3, 2, j, 1);
				
				if (this.mTrack.getState() != AudioTrack.STATE_INITIALIZED)
				{
					this.mTrack.release();
					this.mTrack = null;
					try { Thread.sleep(1L); } catch (InterruptedException localInterruptedException) {}
					continue;
				}
				
				this.mTrack.play();

				localByteBuffer = ByteBuffer.allocateDirect(k * 2 * NUMCHANNELS);
				arrayOfByte = new byte[localByteBuffer.capacity()];

				this.mInitialised = true;
			}

			int i = getInfo(INFO_MIXERRUNNING);
			if (i == 1)
			{
				process(localByteBuffer);

		        localByteBuffer.get(arrayOfByte);
		        localByteBuffer.position(0);
		        this.mTrack.write(arrayOfByte, 0, localByteBuffer.capacity());
		        
/*		        localByteBuffer.get(arrayOfByte, 0, localByteBuffer.capacity());
				this.mTrack.write(arrayOfByte, 0, localByteBuffer.capacity());
				localByteBuffer.position(0); */
			} else
			{
				localByteBuffer = null;
				arrayOfByte = null;

				this.mInitialised = false;
			}

		}

		if (this.mTrack != null)
		{
			this.mTrack.stop();
			this.mTrack.release();
			this.mTrack = null;
		}

		this.mInitialised = false;
	}

	private native int getInfo(int paramInt);

	private native int process(ByteBuffer paramByteBuffer);
}
