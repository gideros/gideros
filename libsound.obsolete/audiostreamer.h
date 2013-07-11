#ifndef AUDIOSTREAMER_H
#define AUDIOSTREAMER_H

#include <stdlib.h>
#include <string>
#include <deque>
#include <pthread.h>
//#include <semaphore.h>


namespace gmod {

// output is always signed 16 bit
// endinaness will be in native byte order
// if the audio is stereo, the channels are interleaved in the output buffer

class AudioStreamer;

class AudioSource
{
public:
	AudioSource(const char* filename) : filename_(filename) {}
	virtual ~AudioSource() {}

	virtual int channelCount() const = 0;
	virtual int rate() const = 0;
	virtual int sampleCount() const = 0;
	
	double length() const
	{
		return ((double)sampleCount() / (double)rate()) * 1000.0;
	}

	const char* fileName() const
	{
		return filename_.c_str();
	}

	virtual AudioStreamer* newStremaer(int loops = 0) = 0;
	virtual void deleteStreamer(AudioStreamer*);

private:
	std::string filename_;
};


class AudioStreamer
{
public:
	AudioStreamer(AudioSource* source, int loops = 0) : source_(source), loops_(loops)
	{
		loops_--;
		if (loops_ < 0)
			loops_ = 0;
	};

	virtual ~AudioStreamer() {}

	int channelCount() const
	{
		return source_->channelCount();
	}

	int rate() const
	{
		return source_->rate();
	}

	int sampleCount() const
	{
		return source_->sampleCount();
	}

	size_t read(unsigned char* buffer, size_t size);

	void seek(double msec)
	{
		double sec = msec / 1000.0;
		ptrdiff_t off = (ptrdiff_t)(sec * rate());
		seek_impl(off);
	}

	double tell()
	{
		return ((double)tell_impl() / (double)rate()) * 1000.0;
	}


protected:
	virtual ptrdiff_t tell_impl() const = 0;		// returns the current position in samples
	virtual void seek_impl(ptrdiff_t offset) = 0;	// seek to a desired sample offset
	virtual size_t read_impl(unsigned char* buffer, size_t size) = 0;
	int loops_;

	AudioSource* source_;
};


class AudioSourceMpg123;

class AudioStreamerMpg123 : public AudioStreamer
{
public:
	AudioStreamerMpg123(AudioSourceMpg123* source, int loops = 0);
	virtual ~AudioStreamerMpg123();

private:
	virtual size_t read_impl(unsigned char* buffer, size_t size);
	virtual ptrdiff_t tell_impl() const;
	virtual void seek_impl(ptrdiff_t offset);

	void* handle_;
};


class AudioSourceMpg123 : public AudioSource
{
public:
	AudioSourceMpg123(const char* filename);
	virtual ~AudioSourceMpg123() {}

	virtual int channelCount() const;
	virtual int rate() const;
	virtual int sampleCount() const;

	virtual AudioStreamerMpg123* newStremaer(int loops = 0);

private:
	int rate_;
	int channelCount_;
	int sampleCount_;
};


class AudioPlayer
{
public:
	AudioPlayer(AudioSource* source, double msec = 0, int loops = 0);
	virtual ~AudioPlayer();

	void stop();
	double position() const;
	bool isPlaying() const
	{
		return playing_;
	}

	void setVolume(float volume);
	float getVolume() const;

	void tick();

	void setSoundCompleteCallback(void(*callback)(void*), void* data);

private:
	void tick_p();
	static void* tick_s(void *ptr);
	AudioSource* audiosource_;
	AudioStreamer* streamer_;

	unsigned int source_;

	unsigned char* buffer_;

	bool emptyqueue_;
	bool playing_;
	bool finished_;

	float volume_;

	std::deque<double> positions_;

	void freebuffers();

	void(*callback_)(void*);
	void* data_;

private:
	pthread_t thread_;
	mutable pthread_mutex_t mutex_;
//	sem_t sem_;
	mutable pthread_mutex_t mutex2_;
	pthread_cond_t cond_;
};

}

#endif // AUDIOSTREAMER_H
