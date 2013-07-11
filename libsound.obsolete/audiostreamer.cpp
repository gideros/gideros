#include "audiostreamer.h"

#include <stdlib.h>
#include <stdio.h>

#if defined(_WIN32) || defined(__APPLE__)
typedef long ssize_t;
#endif

#include <mpg123.h>

#if defined(OPENAL_SUBDIR_OPENAL)
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#elif defined(OPENAL_SUBDIR_AL)
#include <AL/al.h>
#include <AL/alc.h>
#else
#include <al.h>
#include <alc.h>
#endif

#include <math.h>

#include <time.h>

#include <gstdio.h>

//#include <sys/timeb.h>

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef __APPLE__
#include <sys/time.h>
#endif

#include <gfile.h>
#include <glog.h>

static ssize_t mpg123read(void* fd, void* buf, size_t size)
{
	G_FILE* file = (G_FILE*)fd;
	return g_fread(buf, size, 1, file) * size;
}

static off_t mpg123lseek(void* fd, off_t offset, int whence)
{
	G_FILE* file = (G_FILE*)fd;

	if (g_fseek(file, offset, whence) == 0)
		return g_ftell(file);

	return (off_t)-1;
}

static void mpg123cleanup(void* fd)
{
	G_FILE* file = (G_FILE*)fd;
	g_fclose(file);
}

namespace gmod {

void AudioSource::deleteStreamer(AudioStreamer* streamer)
{
	delete streamer;
}

AudioSourceMpg123::AudioSourceMpg123(const char* filename) : AudioSource(filename)
{
	rate_ = 0;
	channelCount_ = 0;
	sampleCount_ = 0;

	int err = MPG123_OK;
	mpg123_handle* mh = mpg123_new(NULL, &err);

	if (mh == NULL || err != MPG123_OK)
		return;

/*	if (mpg123_open(mh, filename) != MPG123_OK)
	{
		mpg123_delete(mh);
		return;
	} */
	G_FILE* file = g_fopen(filename, "rb");
	if (file == NULL)
	{
		mpg123_delete(mh);
		return;
	}
	mpg123_replace_reader_handle(mh, mpg123read, mpg123lseek, mpg123cleanup);
	if (mpg123_open_handle(mh, file) != MPG123_OK)
	{
		mpg123_delete(mh);
		return;
	}

	int  channels = 0, encoding = 0;
	long rate = 0;

	if (mpg123_getformat(mh, &rate, &channels, &encoding) !=  MPG123_OK)
	{
		mpg123_delete(mh);
		return;
	}

	// Signed 16 is the default output format anyways (encoding == MPG123_ENC_SIGNED_16); 
	// it would actually by only different if we forced it.

	// Ensure that this output format will not change (it could, when we allow it).
	mpg123_format_none(mh);
	mpg123_format(mh, rate, channels, encoding);

	rate_ = rate;
	channelCount_ = channels;
	mpg123_scan(mh);
	sampleCount_ = mpg123_length(mh);

	mpg123_close(mh);
	mpg123_delete(mh);
}

int AudioSourceMpg123::channelCount() const
{
	return channelCount_;
}

int AudioSourceMpg123::rate() const
{
	return rate_;
}
int AudioSourceMpg123::sampleCount() const
{
	return sampleCount_;
}

AudioStreamerMpg123* AudioSourceMpg123::newStremaer(int loops)
{
	return new AudioStreamerMpg123(this, loops);
}

size_t AudioStreamer::read(unsigned char* buffer, size_t size)
{
	size_t done = read_impl(buffer, size);

	if (done < size)
	{
		loops_--;

		if (loops_ < 0)
		{
			return done;
		}
		else
		{
			seek_impl(0);
			size_t newdone = read_impl(buffer + done, size - done);
			return done + newdone;
		}
	}

	return done;
}

AudioStreamerMpg123::AudioStreamerMpg123(AudioSourceMpg123* source, int loops) : AudioStreamer(source, loops)
{
	handle_ = NULL;

	int err = MPG123_OK;
	mpg123_handle* mh = mpg123_new(NULL, &err);

	if (mh == NULL || err != MPG123_OK)
		return;

/*	if (mpg123_open(mh, source->fileName()) != MPG123_OK)
	{
		mpg123_delete(mh);
		return;
	}*/
	G_FILE* file = g_fopen(source->fileName(), "rb");
	if (file == NULL)
	{
		mpg123_delete(mh);
		return;
	}
	mpg123_replace_reader_handle(mh, mpg123read, mpg123lseek, mpg123cleanup);
	if (mpg123_open_handle(mh, file) != MPG123_OK)
	{
		mpg123_delete(mh);
		return;
	}


	int  channels = 0, encoding = 0;
	long rate = 0;

	if (mpg123_getformat(mh, &rate, &channels, &encoding) !=  MPG123_OK)
	{
		mpg123_delete(mh);
		return;
	}

	// Signed 16 is the default output format anyways (encoding == MPG123_ENC_SIGNED_16); 
	// it would actually by only different if we forced it.

	// Ensure that this output format will not change (it could, when we allow it).
	mpg123_format_none(mh);
	mpg123_format(mh, rate, channels, encoding);

	handle_ = mh;
}

AudioStreamerMpg123::~AudioStreamerMpg123()
{
	if (handle_)
	{
		mpg123_handle* mh = static_cast<mpg123_handle*>(handle_);

		mpg123_close(mh);
		mpg123_delete(mh);
	}
}

size_t AudioStreamerMpg123::read_impl(unsigned char* buffer, size_t size)
{
	mpg123_handle* mh = static_cast<mpg123_handle*>(handle_);

	size_t done;
	int err = mpg123_read(mh, buffer, size, &done);

	return done;
}

void AudioStreamerMpg123::seek_impl(ptrdiff_t offset)
{
	mpg123_handle* mh = static_cast<mpg123_handle*>(handle_);
	mpg123_seek(mh, offset, SEEK_SET);
}

ptrdiff_t AudioStreamerMpg123::tell_impl() const
{
	mpg123_handle* mh = static_cast<mpg123_handle*>(handle_);
	return mpg123_tell(mh);
}


#define NUM_BUFFERS 4
#define BUFFER_SIZE (4096 * 8)

#ifdef _WIN32

/*
 * time between jan 1, 1601 and jan 1, 1970 in units of 100 nanoseconds
 */
#define PTW32_TIMESPEC_TO_FILETIME_OFFSET \
	  ( ((LONGLONG) 27111902 << 32) + (LONGLONG) 3577643008 )

void
ptw32_filetime_to_timespec (const FILETIME * ft, struct timespec *ts)
     /*
      * -------------------------------------------------------------------
      * converts FILETIME (as set by GetSystemTimeAsFileTime), where the time is
      * expressed in 100 nanoseconds from Jan 1, 1601,
      * into struct timespec
      * where the time is expressed in seconds and nanoseconds from Jan 1, 1970.
      * -------------------------------------------------------------------
      */
{
  ts->tv_sec =
    (int) ((*(LONGLONG *) ft - PTW32_TIMESPEC_TO_FILETIME_OFFSET) / 10000000);
  ts->tv_nsec =
    (int) ((*(LONGLONG *) ft - PTW32_TIMESPEC_TO_FILETIME_OFFSET -
	    ((LONGLONG) ts->tv_sec * (LONGLONG) 10000000)) * 100);
}

#endif


void* AudioPlayer::tick_s(void *ptr)
{
	struct timespec ts;

	AudioPlayer* that = static_cast<AudioPlayer*>(ptr);

	while (true)
	{
		if (that->playing_ == false)
			break;
		
		that->tick_p();

		if (that->playing_ == false)
			break;
		/*
		struct _timeb timebuffer;
		 _ftime_s(&timebuffer);

		 ts.tv_sec = timebuffer.time;
		 ts.tv_nsec = timebuffer.millitm * 1000;
		 ts.tv_nsec += 900000000;
		 */
#if defined(_WIN32)
		FILETIME ft;
		GetSystemTimeAsFileTime(&ft);
		ptw32_filetime_to_timespec(&ft, &ts);
		ts.tv_nsec += 1000000000/30;		// 1/30 of sec
#elif defined(__APPLE__)
		struct timeval tv;
		gettimeofday(&tv, NULL);
		ts.tv_sec = tv.tv_sec;
		ts.tv_nsec = tv.tv_usec * 1000;
		ts.tv_nsec += 1000000000/30;		// 1/30 of sec
#else
		clock_gettime(CLOCK_REALTIME, &ts);
		ts.tv_nsec += 1000000000/30;		// 1/30 of sec
#endif

//		sem_timedwait(&that->sem_, &ts);
		pthread_mutex_lock(&that->mutex2_);
		pthread_cond_timedwait(&that->cond_, &that->mutex2_, &ts);
		pthread_mutex_unlock(&that->mutex2_);
	}

//	pthread_exit(NULL);

	return NULL;
}

AudioPlayer::AudioPlayer(AudioSource* audiosource, double msec, int loops) : audiosource_(audiosource), volume_(1)
{
    glog_v("AudioPlayer\n");

	streamer_ = audiosource_->newStremaer(loops);

	alGenSources(1, &source_);

	buffer_ = (unsigned char*)malloc(BUFFER_SIZE);

	ALuint buffer;
	alGenBuffers(1, &buffer);
//	printf("alGenBuffers: %d\n", buffer);

	streamer_->seek(msec);
	double pos = streamer_->tell();
//	printf("%g\n", pos);
	size_t done = streamer_->read(buffer_, BUFFER_SIZE);
//	printf("%d\n", done);
	alBufferData(buffer, (audiosource_->channelCount() == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, buffer_, done, audiosource_->rate());
	alSourceQueueBuffers(source_, 1, &buffer);
	//	printf("alSourceQueueBuffers\n");
	positions_.push_back(pos);

	alSourcePlay(source_);

	emptyqueue_ = false;
	playing_ = true;
	finished_ = false;

	callback_ = NULL;
	data_ = NULL;

//	sem_init(&sem_, 0, 0);
	pthread_mutex_init(&mutex_, NULL);
	pthread_mutex_init(&mutex2_, NULL);
	pthread_cond_init(&cond_, NULL);
	pthread_create(&thread_, NULL, tick_s, this);
}

AudioPlayer::~AudioPlayer()
{
    glog_v("~AudioPlayer\n");

	stop();

	//	sem_destroy(&sem_);
	pthread_cond_destroy(&cond_);

	pthread_mutex_destroy(&mutex_);
	pthread_mutex_destroy(&mutex2_);

	free(buffer_);
	buffer_ = NULL;

	alDeleteSources(1, &source_);

	audiosource_->deleteStreamer(streamer_);
	streamer_ = NULL;
}

void AudioPlayer::tick_p()
{
	pthread_mutex_lock(&mutex_);

	if (playing_ == false)
	{
		pthread_mutex_unlock(&mutex_);
		return;
	}

	ALuint buffer;

	if (emptyqueue_)
	{
		ALint state;
		alGetSourcei(source_, AL_SOURCE_STATE, &state);

		if (state == AL_PLAYING)
		{
			pthread_mutex_unlock(&mutex_);
			return;
		}

		finished_ = true;

		playing_ = false;
		freebuffers();

		pthread_mutex_unlock(&mutex_);
		return;
	}

	ALint queued;
	alGetSourcei(source_, AL_BUFFERS_QUEUED, &queued);
	if (queued < NUM_BUFFERS)
	{
		alGenBuffers(1, &buffer);
//		printf("alGenBuffers: %d\n", buffer);
	}
	else
	{
		ALint processed;
		alGetSourcei(source_, AL_BUFFERS_PROCESSED, &processed);

		if (processed == 0)
		{
			pthread_mutex_unlock(&mutex_);
			return;
		}

		alSourceUnqueueBuffers(source_, 1, &buffer);
		//		printf("alSourceUnqueueBuffers\n");
		positions_.pop_front();
	}

	double pos = streamer_->tell();
//	printf("pos: %g\n", pos);
	size_t done = streamer_->read(buffer_, BUFFER_SIZE);
//	printf("done: %d\n", done);
	if (done != 0)
	{
		alBufferData(buffer, (audiosource_->channelCount() == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, buffer_, done, audiosource_->rate());
		alSourceQueueBuffers(source_, 1, &buffer);
		//		printf("alSourceQueueBuffers\n");
		positions_.push_back(pos);

		ALint state;
		alGetSourcei(source_, AL_SOURCE_STATE, &state);
		if (state == AL_STOPPED)
			alSourcePlay(source_);
	}
	else
	{
		alDeleteBuffers(1, &buffer);
//		printf("alDeleteBuffers: %d\n", buffer);
		emptyqueue_ = true;
	}

	pthread_mutex_unlock(&mutex_);
	return;
}

void AudioPlayer::freebuffers()
{
	// free all queued buffers
	ALint queued;
	alGetSourcei(source_, AL_BUFFERS_QUEUED, &queued);
	for (int i = 0; i < queued; ++i)
	{
		ALuint buffer;
		alSourceUnqueueBuffers(source_, 1, &buffer);
		positions_.pop_front();
		alDeleteBuffers(1, &buffer);
	}
}

void AudioPlayer::stop()
{
	playing_ = false;

//	sem_post(&sem_);
	pthread_cond_signal(&cond_);

	pthread_join(thread_, NULL);

	alSourceStop(source_);

	freebuffers();
}

double AudioPlayer::position() const
{
	pthread_mutex_lock(&mutex_);

	double result = 0;

	ALfloat offset;
	alGetSourcef(source_, AL_SEC_OFFSET, &offset);
	offset *= 1000.0;

	if (!positions_.empty())
		result = fmod(offset + positions_[0], audiosource_->length());

	pthread_mutex_unlock(&mutex_);

	return result;
}

void AudioPlayer::setVolume(float volume)
{
	volume_ = volume;
	alSourcef(source_, AL_GAIN, volume);
}

float AudioPlayer::getVolume() const
{
	return volume_;
}

void AudioPlayer::tick()
{
	if (finished_ == true)
	{
		finished_ = false;

		if (callback_)
			callback_(data_);
	}
}

void AudioPlayer::setSoundCompleteCallback(void(*callback)(void*), void* data)
{
	callback_ = callback;
	data_ = data;
}

}

