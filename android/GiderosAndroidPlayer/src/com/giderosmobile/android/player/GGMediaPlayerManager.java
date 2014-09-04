package com.giderosmobile.android.player;

import java.io.File;
import java.io.IOException;
import java.util.HashSet;
import java.util.Iterator;

import android.content.res.AssetFileDescriptor;
import android.media.MediaPlayer;

public class GGMediaPlayerManager
{
	public static final int GAUDIO_NO_ERROR = 0;
	public static final int GAUDIO_CANNOT_OPEN_FILE = 1;
	public static final int GAUDIO_UNRECOGNIZED_FORMAT = 2;
	public static final int GAUDIO_ERROR_WHILE_READING = 3;
	public static final int GAUDIO_UNSUPPORTED_FORMAT = 4;
	public static final int GAUDIO_INTERNAL_ERROR = 5;
    
	private ZipResourceFile mainFile_, patchFile_;
    private boolean suspended_ = false;
	
	public GGMediaPlayerManager(ZipResourceFile mainFile, ZipResourceFile patchFile)
	{
		mainFile_ = mainFile;
		patchFile_ = patchFile;
	}

	public synchronized void onPause()
	{
		if (suspended_)
			return;

		for (int i = 0; i < channels_.size(); ++i)
		{
			Channel channel = channels_.valueAt(i);
			
			if (channel.player == null)
				continue;
			
			if (channel.player.isPlaying())
			{
				channel.player.pause();
				channel.suspended = true;
			}
		}	

		suspended_ = true;
	}

	public synchronized void onResume()
	{
		if (!suspended_)
			return;		
	
		for (int i = 0; i < channels_.size(); ++i)
		{
			Channel channel = channels_.valueAt(i);
			
			if (channel.player == null)
				continue;
			
			if (channel.suspended)
			{
				channel.player.start();
				channel.suspended = false;
			}
		}	

		suspended_ = false;
	}
	
	public synchronized long BackgroundMusicCreateFromFile(String fileName, int[] error)
	{
		if (fileName.equals(""))
		{
			error[0] = GAUDIO_CANNOT_OPEN_FILE;
			return 0;			
		}
		
		AssetFileDescriptor fd = null;

		if (fileName.startsWith("/"))
		{
			File file = new File(fileName);
			if (!file.isFile())
			{
				error[0] = GAUDIO_CANNOT_OPEN_FILE;
				return 0;
			}			
		}
		else
		{
			if (patchFile_ != null)
				fd = patchFile_.getAssetFileDescriptor(fileName);
			if (fd == null && mainFile_ != null)
				fd = mainFile_.getAssetFileDescriptor(fileName);
			if (fd == null)
				try { fd = WeakActivityHolder.get().getAssets().openFd("assets/" + fileName); } catch (IOException e) {}
			
			if (fd == null)
			{
				error[0] = GAUDIO_CANNOT_OPEN_FILE;
				return 0;				
			}
		}

		MediaPlayer player = new MediaPlayer();

		int duration = 0;

		try
		{
			if (fd != null)
			{
				player.setDataSource(fd.getFileDescriptor(), fd.getStartOffset(), fd.getLength());
		        fd.close();
			}
			else
			{
				player.setDataSource(fileName);
			}
			player.prepare();
			duration = player.getDuration();
			player.release();
			player = null;
		} catch (Exception e)
		{
			error[0] = GAUDIO_UNRECOGNIZED_FORMAT;
			return 0;
		}

		long gid = nextgid();

		sounds_.put(gid, new Sound(fileName, duration));

		return gid;
	}

	public synchronized void BackgroundMusicDelete(long backgroundMusic)
	{
		Sound sound2 = sounds_.get(backgroundMusic);
		
		if (sound2 == null)
			return;
				
		Iterator<Channel> iter2 = sound2.channels.iterator();
		while (iter2.hasNext())
		{
			Channel channel = iter2.next();

			if (channel.player != null)
			{
				channel.player.release();
				channel.player = null;
			}
			
			channels_.remove(channel.gid);
		}
		
		sounds_.remove(backgroundMusic);
	}

	public synchronized int BackgroundMusicGetLength(long backgroundMusic)
	{
		Sound sound2 = sounds_.get(backgroundMusic);

		if (sound2 == null)
			return 0;

		return sound2.length;
	}
	
	public synchronized long BackgroundMusicPlay(long backgroundMusic, boolean paused, long data)
	{
		Sound sound2 = sounds_.get(backgroundMusic);
		
		if (sound2 == null)
			return 0;
		
		String fileName = sound2.fileName;
		
		AssetFileDescriptor fd = null;

		if (fileName.startsWith("/"))
		{
			File file = new File(fileName);
			if (!file.isFile())
				return 0;
		}
		else
		{
			if (patchFile_ != null)
				fd = patchFile_.getAssetFileDescriptor(fileName);
			if (fd == null && mainFile_ != null)
				fd = mainFile_.getAssetFileDescriptor(fileName);
			if (fd == null)
				try { fd = WeakActivityHolder.get().getAssets().openFd("assets/" + fileName); } catch (IOException e) {}
			
			if (fd == null)
				return 0;				
		}		
		
		MediaPlayer player = new MediaPlayer();
		
		try
		{
			if (fd != null)
			{
				player.setDataSource(fd.getFileDescriptor(), fd.getStartOffset(), fd.getLength());
		        fd.close();
			}
			else
			{
				player.setDataSource(fileName);
			}
			player.prepare();
		} catch (Exception e)
		{
			return 0;
		}		
		
		long gid = nextgid();

		player.setOnCompletionListener(new OnCompletionListener(this, gid));

		Channel channel = new Channel(gid, player, sound2, data);
		
		sound2.channels.add(channel);
		
	    channels_.put(gid, channel);
	    
	    channel.paused = paused;
	    if (!paused)
	    	channel.player.start();
	    
	    return gid;
	}
	
	public synchronized void BackgroundChannelStop(long backgroundChannel)
	{
		Channel channel2 = channels_.get(backgroundChannel);
		
		if (channel2 == null)
			return;
		
		if (channel2.player != null)
		{
			channel2.player.release();
			channel2.player = null;			
		}

        channel2.sound.channels.remove(channel2);

        channels_.remove(backgroundChannel);
	}
	
	public synchronized void BackgroundChannelSetPosition(long backgroundChannel, int position)
	{
		Channel channel2 = channels_.get(backgroundChannel);
		
		if (channel2 == null)
			return;

		if (channel2.player == null)
            return;
        
        channel2.player.seekTo(position);
	}
	
	public synchronized int BackgroundChannelGetPosition(long backgroundChannel)
	{
		Channel channel2 = channels_.get(backgroundChannel);

		if (channel2 == null)
			return 0;
		
        if (channel2.player == null)
            return channel2.lastPosition;
        
        return channel2.player.getCurrentPosition();
	}
	
	public synchronized void BackgroundChannelSetPaused(long backgroundChannel, boolean paused)
	{
		Channel channel2 = channels_.get(backgroundChannel);
		
		if (channel2 == null)
			return;

		if (paused == channel2.paused)
			return;

        channel2.paused = paused;

        if (channel2.player != null)
        {
            if (paused)
                channel2.player.pause();
            else
            	channel2.player.start();
        }
	}
	
	public synchronized boolean BackgroundChannelIsPaused(long backgroundChannel)
	{
		Channel channel2 = channels_.get(backgroundChannel);

		if (channel2 == null)
			return false;
		
		return channel2.paused;
	}
	
	public synchronized boolean BackgroundChannelIsPlaying(long backgroundChannel)
	{
		Channel channel2 = channels_.get(backgroundChannel);
		
		if (channel2 == null)
			return false;

		if (channel2.player == null)
			return false;
		
		return channel2.player.isPlaying();
	}
	
	public synchronized void BackgroundChannelSetVolume(long backgroundChannel, float volume)
	{
		Channel channel2 = channels_.get(backgroundChannel);
		
		if (channel2 == null)
			return;

		channel2.volume = volume;
		
		if (channel2.player != null)
			channel2.player.setVolume(volume, volume);		
	}
	
	public synchronized float BackgroundChannelGetVolume(long backgroundChannel)
	{
		Channel channel2 = channels_.get(backgroundChannel);

		if (channel2 == null)
			return 0.f;
		
		return channel2.volume;
	}
	
	public synchronized void BackgroundChannelSetLooping(long backgroundChannel, boolean looping)
	{
		Channel channel2 = channels_.get(backgroundChannel);

		if (channel2 == null)
			return;
		
		channel2.looping = looping;

		if (channel2.player != null)
			channel2.player.setLooping(looping);		
	}
	
	public synchronized boolean BackgroundChannelIsLooping(long backgroundChannel)
	{
		Channel channel2 = channels_.get(backgroundChannel);

		if (channel2 == null)
			return false;
		
		return channel2.looping;
	}

	synchronized void onCompletion(long backgroundChannel)
	{
		Channel channel = channels_.get(backgroundChannel);

		if (channel == null)
			return;
		
		channel.lastPosition = channel.sound.length;

		if (channel.player != null)
		{
			channel.player.release();
			channel.player = null;
		}
		
		onChannelComplete(backgroundChannel, channel.data);
	}

	private LongSparseArray<Sound> sounds_ = new LongSparseArray<Sound>();
	private LongSparseArray<Channel> channels_ = new LongSparseArray<Channel>();

	static private native long nextgid();
	static private native void onChannelComplete(long backgroundChannel, long data);
}

class Sound
{
	public Sound(String fileName, int length)
	{
		this.fileName = fileName;
		this.length = length;
	}

	public String fileName;
	public int zipFile;
	public int length;
	public HashSet<Channel> channels = new HashSet<Channel>();
}

class Channel
{
	public Channel(long gid, MediaPlayer player, Sound sound, long data)
	{
		this.gid = gid;
		this.player = player;
		this.sound = sound;
		this.data = data;
        this.paused = true;
        this.volume = 1.f;
        this.looping = false;
        this.suspended = false;
        this.lastPosition = 0;
	}

    public long gid;
    public MediaPlayer player;
    public Sound sound;
    public long data;
    public boolean paused;
    public float volume;
    public boolean looping;
    public boolean suspended;
    public int lastPosition;
}

class OnCompletionListener implements MediaPlayer.OnCompletionListener
{
	public OnCompletionListener(GGMediaPlayerManager manager, long backgroundChannel)
	{
		manager_ = manager;
		backgroundChannel_ = backgroundChannel;		
	}

	@Override
	public void onCompletion(MediaPlayer mediaPlayer)
	{
		manager_.onCompletion(backgroundChannel_);
	}

	GGMediaPlayerManager manager_;
	long backgroundChannel_;
}
