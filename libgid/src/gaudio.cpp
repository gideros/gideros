#include "ggaudiomanager.h"
#include <stdlib.h>
#include <string>
#include <algorithm>
#include <ctype.h>
#include <stdio.h>
#include <cstdint>

#ifdef WINSTORE
#include "winrt/wave.h"
#undef interface  // silly Microsoft!
#endif

GGSoundManager::GGSoundManager()
{
    interfacesInit();
}

GGSoundManager::~GGSoundManager()
{
    while (!sounds_.empty())
    {
        Sound *sound = sounds_.begin()->second;
        SoundDelete(sound->gid);
    }

    interfacesCleanup();
}

g_id GGSoundManager::SoundCreateFromFile(const char *fileName, bool stream, gaudio_Error *error)
{
    const char *dot = strrchr(fileName, '.');

    if (dot == NULL)
    {
        if (error)
            *error = GAUDIO_UNSUPPORTED_FORMAT;
        return 0;
    }

    std::string dot2 = dot + 1;
    std::transform(dot2.begin(), dot2.end(), dot2.begin(), ::tolower);

    std::map<std::string, GGAudioLoader>::iterator iter = loaders_.find(dot2);

    if (iter == loaders_.end())
    {
        if (error)
            *error = GAUDIO_UNSUPPORTED_FORMAT;
        return 0;
    }

    const GGAudioLoader &loader = iter->second;

    if (stream == false)
    {
        int numChannels, sampleRate, bitsPerSample, numSamples;
        g_id handle = loader.open(fileName, &numChannels, &sampleRate, &bitsPerSample, &numSamples, error);

        if (handle == 0)
            return 0;

        size_t size = numChannels * (bitsPerSample / 8) * numSamples;
        void *data = malloc(size);
        loader.read(handle, size, data, NULL);

        loader.close(handle);

        g_id sound = sampleInterface_->SoundCreateFromBuffer(data, numChannels, sampleRate, bitsPerSample, numSamples);

        free(data);

		sounds_[sound] = new Sound(sound, sampleInterface_);

        return sound;
    }
    else
    {
        g_id sound = streamInterface_->SoundCreateFromFile(fileName, loader, error);

        if (sound == 0)
            return 0;

        sounds_[sound] = new Sound(sound, streamInterface_);

        return sound;
    }
}

#include <cmath>
namespace {

constexpr double Epsilon{1e-9};

using uint = unsigned int;

#define pi  3.141592654
/* This is the normalized cardinal sine (sinc) function.
 *
 *   sinc(x) = { 1,                   x = 0
 *             { sin(pi x) / (pi x),  otherwise.
 */
double Sinc(const double x)
{
    if(std::abs(x) < Epsilon)
        return 1.0;
    return std::sin(pi*x) / (pi*x);
}

/* The zero-order modified Bessel function of the first kind, used for the
 * Kaiser window.
 *
 *   I_0(x) = sum_{k=0}^inf (1 / k!)^2 (x / 2)^(2 k)
 *          = sum_{k=0}^inf ((x / 2)^k / k!)^2
 */
double BesselI_0(const double x)
{
    // Start at k=1 since k=0 is trivial.
    const double x2{x/2.0};
    double term{1.0};
    double sum{1.0};
    int k{1};

    // Let the integration converge until the term of the sum is no longer
    // significant.
    double last_sum{};
    do {
        const double y{x2 / k};
        ++k;
        last_sum = sum;
        term *= y * y;
        sum += term;
    } while(sum != last_sum);
    return sum;
}

double Kaiser(const double b, const double k)
{
    if(!(k >= -1.0 && k <= 1.0))
        return 0.0;
    return BesselI_0(b * std::sqrt(1.0 - k*k)) / BesselI_0(b);
}

// Calculates the greatest common divisor of a and b.
uint Gcd(uint x, uint y)
{
    while(y > 0)
    {
        const uint z{y};
        y = x % y;
        x = z;
    }
    return x;
}

/* Calculates the size (order) of the Kaiser window.  Rejection is in dB and
 * the transition width is normalized frequency (0.5 is nyquist).
 *
 *   M = { ceil((r - 7.95) / (2.285 2 pi f_t)),  r > 21
 *       { ceil(5.79 / 2 pi f_t),                r <= 21.
 *
 */
constexpr uint CalcKaiserOrder(const double rejection, const double transition)
{
    const double w_t{2.0 * pi * transition};
    if (rejection > 21.0)
        return static_cast<uint>(std::ceil((rejection - 7.95) / (2.285 * w_t)));
    return static_cast<uint>(std::ceil(5.79 / w_t));
}

// Calculates the beta value of the Kaiser window.  Rejection is in dB.
constexpr double CalcKaiserBeta(const double rejection)
{
    if (rejection > 50.0)
        return 0.1102 * (rejection - 8.7);
    if(rejection >= 21.0)
        return (0.5842 * std::pow(rejection - 21.0, 0.4)) +
               (0.07886 * (rejection - 21.0));
    return 0.0;
}

/* Calculates a point on the Kaiser-windowed sinc filter for the given half-
 * width, beta, gain, and cutoff.  The point is specified in non-normalized
 * samples, from 0 to M, where M = (2 l + 1).
 *
 *   w(k) 2 p f_t sinc(2 f_t x)
 *
 *   x    -- centered sample index (i - l)
 *   k    -- normalized and centered window index (x / l)
 *   w(k) -- window function (Kaiser)
 *   p    -- gain compensation factor when sampling
 *   f_t  -- normalized center frequency (or cutoff; 0.5 is nyquist)
 */
double SincFilter(const uint l, const double b, const double gain, const double cutoff,
    const uint i)
{
    const double x{static_cast<double>(i) - l};
    return Kaiser(b, x / l) * 2.0 * gain * cutoff * Sinc(2.0 * cutoff * x);
}

} // namespace

struct ResampleFilter {
    uint mP, mQ, mM, mL;
    std::vector<float> mF;
    void init(const uint srcRate, const uint dstRate)
    {
        const uint gcd{Gcd(srcRate, dstRate)};
        mP = dstRate / gcd;
        mQ = srcRate / gcd;

        /* The cutoff is adjusted by half the transition width, so the transition
         * ends before the nyquist (0.5).  Both are scaled by the downsampling
         * factor.
         */
        double cutoff, width;
        if(mP > mQ)
        {
            cutoff = 0.475 / mP;
            width = 0.05 / mP;
        }
        else
        {
            cutoff = 0.475 / mQ;
            width = 0.05 / mQ;
        }
        // A rejection of -180 dB is used for the stop band. Round up when
        // calculating the left offset to avoid increasing the transition width.
        const uint l{(CalcKaiserOrder(180.0, width)+1) / 2};
        const double beta{CalcKaiserBeta(180.0)};
        mM = l*2 + 1;
        mL = l;
        mF.resize(mM);
        for(uint i{0};i < mM;i++)
            mF[i] = SincFilter(l, beta, mP, cutoff, i);
    }
    void process(const uint inN, const float *in, const uint outN, float *out)
    {
        if (outN == 0)
            return;

        // Handle in-place operation.
        std::vector<float> workspace;
        float *work{out};
        if (work == in)
        {
            workspace.resize(outN);
            work = workspace.data();
        }

        // Resample the input.
        const uint p{mP}, q{mQ}, m{mM}, l{mL};
        const float *f{mF.data()};
        for(uint i{0};i < outN;i++)
        {
            // Input starts at l to compensate for the filter delay.  This will
            // drop any build-up from the first half of the filter.
            size_t j_f{(l + q*i) % p};
            size_t j_s{(l + q*i) / p};

            // Only take input when 0 <= j_s < inN.
            double r{0.0};
            if (j_f < m)
            {
                size_t filt_len{(m-j_f+p-1) / p};
                if (j_s+1 > inN)
                {
                    size_t skip{std::min<size_t>(j_s+1 - inN, filt_len)};
                    j_f += p*skip;
                    j_s -= skip;
                    filt_len -= skip;
                }
                if(size_t todo{std::min<size_t>(j_s+1, filt_len)})
                {
                    do {
                        r += f[j_f] * in[j_s];
                        j_f += p;
                        --j_s;
                    } while(--todo);
                }
            }
            work[i] = r;
        }
        // Clean up after in-place operation.
        if(work != out)
            std::copy_n(work, outN, out);
    }
};

size_t GGSoundManager::SoundReadFile(const char *fileName, double start, double length, std::vector<unsigned char> &data,int &numChannels,int &bytesPerSample, int &sampleRate, gaudio_Error *error)
{
    const char *dot = strrchr(fileName, '.');

    if (dot == NULL)
    {
        if (error)
            *error = GAUDIO_UNSUPPORTED_FORMAT;
        return 0;
    }

    std::string dot2 = dot + 1;
    std::transform(dot2.begin(), dot2.end(), dot2.begin(), ::tolower);

    std::map<std::string, GGAudioLoader>::iterator iter = loaders_.find(dot2);

    if (iter == loaders_.end())
    {
        if (error)
            *error = GAUDIO_UNSUPPORTED_FORMAT;
        return 0;
    }

    const GGAudioLoader &loader = iter->second;

    int bitsPerSample, numSamples;
    int oChannels=numChannels;
    int oSr=sampleRate;
    int oBps=bytesPerSample;
    g_id handle = loader.open(fileName, &numChannels, &sampleRate, &bitsPerSample, &numSamples, error);

    if (handle == 0)
        return 0;

    if (((bitsPerSample!=8)&&(bitsPerSample!=16))||
        ((numChannels!=1)&&(numChannels!=2)))
    {
        loader.close(handle);
        return 0;
    }
    bytesPerSample=(bitsPerSample / 8);

    if (start>0)
        loader.seek(handle,start*sampleRate,SEEK_SET);

    if (length>0) {
        long int nums=length*sampleRate;
        if (numSamples>nums) numSamples=nums;
    }

    if (oChannels==0) oChannels=numChannels;
    if (oSr==0) oSr=sampleRate;
    if (oBps==0) oBps=bytesPerSample;

    if ((oChannels!=numChannels)||(oSr!=sampleRate)||(oBps!=bytesPerSample))
    {
        if (((oBps!=1)&&(oBps!=2))||
            ((oChannels!=1)&&(oChannels!=2)))
        {
            loader.close(handle);
            return 0;
        }
        size_t oSamples=((int64_t)numSamples)*oSr/sampleRate;
        size_t size = oChannels * oSamples * oBps;
        data.resize(size);

        size_t isize = numChannels * numSamples * bytesPerSample;
        signed char *idata = (signed char *) malloc(isize);
        signed char *iidata=idata;
        loader.read(handle, isize, idata, NULL);
        signed char *odata=(signed char *)data.data();

#if 1
        ResampleFilter filter;
        filter.init(sampleRate,oSr);
        std::vector<float> ifloat(numSamples);
        bool stereo=((numChannels==2)&&(oChannels==2));
        std::vector<float> ifloat2(stereo?numSamples:0);
        std::vector<float> ofloat(oSamples);
        std::vector<float> ofloat2(stereo?oSamples:0);
        if (bytesPerSample==2) {
            signed short *si=(signed short *)idata;
            for (int k=0;k<numSamples;k++)
            {
                float sa=*(si++);
                if (numChannels==2) {
                    float sb=*(si++);
                    if (stereo)
                        ifloat2[k]=sb;
                    else
                        sa=(sa+sb)/2;
                }
                ifloat[k]=sa;
            }
        }
        else {
            signed char *si=(signed char *)idata;
            for (int k=0;k<numSamples;k++)
            {
                float sa=*(si++)*256;
                if (numChannels==2) {
                    float sb=*(si++)*256;
                    if (stereo)
                        ifloat2[k]=sb;
                    else
                        sa=(sa+sb)/2;
                }
                ifloat[k]=sa;
            }
        }
        filter.process(numSamples,ifloat.data(),oSamples,ofloat.data());
        if (stereo)
            filter.process(numSamples,ifloat2.data(),oSamples,ofloat2.data());
        if (oBps==2) {
            signed short *so=(signed short *)odata;
            for (size_t k=0;k<oSamples;k++)
            {
                float sa=ofloat[k];
                *(so++)=sa;
                if (oChannels==2) {
                    float sb=stereo?ofloat2[k]:sa;
                    *(so++)=sb;
                }
            }
        }
        else {
            signed char *so=(signed char *)odata;
            for (size_t k=0;k<oSamples;k++)
            {
                float sa=ofloat[k]/256;
                *(so++)=sa;
                if (oChannels==2) {
                    float sb=stereo?ofloat2[k]/256:sa;
                    *(so++)=sb;
                }
            }
        }
#else
        double nsmp=((double)sampleRate)/oSr;
        double snum=0;
        numSamples=oSamples;
        while (oSamples--) {
            while (snum>=1) {
                snum--;
                idata+=bytesPerSample*numChannels;
            }
            //Extract
            int sm1a=(bytesPerSample==2)?((signed short *)idata)[0]:(idata[0]<<8);
            int sm1b=0;
            if (numChannels==2)
                sm1b=(bytesPerSample==2)?((signed short *)idata)[1]:(idata[1]<<8);
            if (snum>0) {
                int sm2a=(bytesPerSample==2)?((signed short *)idata)[numChannels]:(idata[numChannels]*256);
                int sm2b=0;
                if (numChannels==2)
                    sm2b=(bytesPerSample==2)?((signed short *)idata)[numChannels+1]:(idata[numChannels+1]*256);
                sm1a=snum*sm2a+(1-snum)*sm1a;
                sm1b=snum*sm2b+(1-snum)*sm1b;
            }
            //Output
            if (oBps==2) {
                if (oChannels==2)
                    ((signed short *)odata)[1]=sm1b;
                else
                    sm1a=(sm1a+sm1b)/2;
                ((signed short *)odata)[0]=sm1a;
                odata+=2*oChannels;
            }
            else {
                if (oChannels==2)
                    odata[1]=sm1b/256;
                else
                    sm1a=(sm1a+sm1b)/2;
                odata[0]=sm1a/256;
                odata+=oChannels;
            }
            //Next
            snum+=nsmp;
        }
#endif
        free(iidata);
        bytesPerSample=oBps;
        sampleRate=oSr;
        numChannels=oChannels;
    }
    else {
        size_t size = numChannels * numSamples * bytesPerSample;
        data.resize(size);
        loader.read(handle, size, data.data(), NULL);
    }
    loader.close(handle);

    return numSamples;
}

g_id GGSoundManager::SoundCreateFromData(const signed short *samples,size_t sampleCount,int rate, bool stereo)
{
    g_id sound = sampleInterface_->SoundCreateFromBuffer(samples, stereo?2:1, rate,16,stereo?(sampleCount/2):sampleCount);

    sounds_[sound] = new Sound(sound, sampleInterface_);

    return sound;
}

void GGSoundManager::SoundDelete(g_id sound)
{
    std::map<g_id, Sound*>::iterator iter = sounds_.find(sound);
    if (iter == sounds_.end())
        return;

    Sound *sound2 = iter->second;

    std::set<Channel*>::iterator iter2, e = sound2->channels.end();
    for (iter2 = sound2->channels.begin(); iter2 != e; ++iter2)
    {
        Channel *channel = *iter2;

        channel->interface->ChannelStop(channel->gid);

        channels_.erase(channel->gid);

        delete channel;
    }

    sound2->interface->SoundDelete(sound);

    delete sound2;

    sounds_.erase(iter);
}

unsigned int GGSoundManager::SoundGetLength(g_id sound)
{
    std::map<g_id, Sound*>::iterator iter = sounds_.find(sound);
    if (iter == sounds_.end())
        return 0;

    Sound *sound2 = iter->second;

    return sound2->interface->SoundGetLength(sound);
}

g_id GGSoundManager::SoundPlay(g_id sound, bool paused, bool streaming)
{
    std::map<g_id, Sound*>::iterator iter = sounds_.find(sound);
    if (iter == sounds_.end())
        return 0;

    Sound *sound2 = iter->second;

    g_id channel = sound2->interface->SoundPlay(sound, paused, streaming);

    Channel *channel2 = new Channel(channel, sound2, sound2->interface);

    sound2->channels.insert(channel2);

    channels_[channel] = channel2;

    return channel;
}

void GGSoundManager::SoundListener(float x,float y,float z,float vx,float vy,float vz,float dx,float dy,float dz,float ux,float uy,float uz)
{
	  sampleInterface_->SoundListener(x,y,z,vx,vy,vz,dx,dy,dz,ux,uy,uz);
	  streamInterface_->SoundListener(x,y,z,vx,vy,vz,dx,dy,dz,ux,uy,uz);
}

bool GGSoundManager::SoundHasEffect(const char *effect)
{
    return streamInterface_->SoundHasEffect(effect); //Assume both iinterfaces support effects
}


void GGSoundManager::ChannelStop(g_id channel)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return;

    Channel *channel2 = iter->second;

    channel2->interface->ChannelStop(channel);

    channel2->sound->channels.erase(channel2);

    delete channel2;

    channels_.erase(iter);
}

void GGSoundManager::ChannelSetPosition(g_id channel, unsigned int position)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return;

    Channel *channel2 = iter->second;

    channel2->interface->ChannelSetPosition(channel, position);
}

unsigned int GGSoundManager::ChannelGetPosition(g_id channel)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return 0;

    Channel *channel2 = iter->second;

    return channel2->interface->ChannelGetPosition(channel);
}

void GGSoundManager::ChannelSetPaused(g_id channel, bool paused)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return;

    Channel *channel2 = iter->second;

    channel2->interface->ChannelSetPaused(channel, paused);
}

bool GGSoundManager::ChannelIsPaused(g_id channel)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return false;

    Channel *channel2 = iter->second;

    return channel2->interface->ChannelIsPaused(channel);
}

bool GGSoundManager::ChannelIsPlaying(g_id channel, int *bufferSize, float *bufferSeconds)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return false;

    Channel *channel2 = iter->second;

    return channel2->interface->ChannelIsPlaying(channel, bufferSize, bufferSeconds);
}

void GGSoundManager::ChannelSetVolume(g_id channel, float volume, float balance)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return;

    Channel *channel2 = iter->second;

    channel2->interface->ChannelSetVolume(channel, volume, balance);
}

float GGSoundManager::ChannelGetVolume(g_id channel)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return 0.f;

    Channel *channel2 = iter->second;

    return channel2->interface->ChannelGetVolume(channel);
}

g_id GGSoundManager::ChannelGetStreamId(g_id channel)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return 0;

    Channel *channel2 = iter->second;

    return channel2->interface->ChannelGetStreamId(channel);
}

void GGSoundManager::ChannelSetPitch(g_id channel, float pitch)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return;

    Channel *channel2 = iter->second;

    channel2->interface->ChannelSetPitch(channel, pitch);
}

float GGSoundManager::ChannelGetPitch(g_id channel)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return 0.f;

    Channel *channel2 = iter->second;

    return channel2->interface->ChannelGetPitch(channel);
}

void GGSoundManager::ChannelSetLooping(g_id channel, bool looping)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return;

    Channel *channel2 = iter->second;

    channel2->interface->ChannelSetLooping(channel, looping);
}

bool GGSoundManager::ChannelIsLooping(g_id channel)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return false;

    Channel *channel2 = iter->second;

    return channel2->interface->ChannelIsLooping(channel);
}

void GGSoundManager::ChannelSetWorldPosition(g_id channel, float x,float y,float z,float vx,float vy,float vz)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return ;

    Channel *channel2 = iter->second;

    channel2->interface->ChannelSetWorldPosition(channel, x,y,z,vx,vy,vz);
}

void GGSoundManager::ChannelSetEffect(g_id channel, const char *effect, float *params)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return ;

    Channel *channel2 = iter->second;

    channel2->interface->ChannelSetEffect(channel, effect, params);
}

g_id GGSoundManager::ChannelAddCallback(g_id channel, gevent_Callback callback, void *udata)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return 0;

    Channel *channel2 = iter->second;

    return channel2->interface->ChannelAddCallback(channel, callback, udata);
}

void GGSoundManager::ChannelRemoveCallback(g_id channel, gevent_Callback callback, void *udata)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return;

    Channel *channel2 = iter->second;

    channel2->interface->ChannelRemoveCallback(channel, callback, udata);
}

void GGSoundManager::ChannelRemoveCallbackWithGid(g_id channel, g_id gid)
{
    std::map<g_id, Channel*>::iterator iter = channels_.find(channel);
    if (iter == channels_.end())
        return;

    Channel *channel2 = iter->second;

    channel2->interface->ChannelRemoveCallbackWithGid(channel, gid);
}

void GGSoundManager::registerLoader(const char *name, GGAudioLoader &loader)
{
	loaders_[name]=loader;
}

void GGSoundManager::unregisterLoader(const char *name)
{
	loaders_.erase(name);
}

void GGSoundManager::registerEncoder(const char *name, GGAudioEncoder &encoder)
{
	encoders_[name]=encoder;
}

void GGSoundManager::unregisterEncoder(const char *name)
{
	encoders_.erase(name);
}

GGAudioEncoder *GGSoundManager::lookupEncoder(const char *filename)
{
    const char *dot = strrchr(filename, '.');

    if (dot == NULL) return NULL;

    std::string dot2 = dot + 1;
    std::transform(dot2.begin(), dot2.end(), dot2.begin(), ::tolower);

    std::map<std::string, GGAudioEncoder>::iterator iter = encoders_.find(dot2);

    if (iter == encoders_.end()) return NULL;

    return &(iter->second);
}


void GGSoundManager::preTick()
{
    sampleInterface_->preTick();
    streamInterface_->preTick();
}

void GGSoundManager::postTick()
{
    sampleInterface_->postTick();
    streamInterface_->postTick();

    std::map<g_id, Channel*>::iterator iter = channels_.begin(), end = channels_.end();
    while (iter != end)
    {
        Channel *channel2 = iter->second;

        g_bool valid = channel2->interface->ChannelIsValid(channel2->gid);

        if (!valid)
        {
            channel2->sound->channels.erase(channel2);
            delete channel2;
            channels_.erase(iter++);
        }
        else
        {
            ++iter;
        }
    }
}


GGAudioManager::GGAudioManager()
{
    systemInit();
    interrupted_ = false;

    soundManager_ = new GGSoundManager();
    createBackgroundMusicInterface();

    gevent_AddCallback(tick_s, this);
}

GGAudioManager::~GGAudioManager()
{
    gevent_RemoveCallback(tick_s, this);

    delete soundManager_;
    deleteBackgroundMusicInterface();

    systemCleanup();
}

g_id GGAudioManager::SoundCreateFromFile(const char *fileName, bool stream, gaudio_Error *error)
{
    return soundManager_->SoundCreateFromFile(fileName, stream, error);
}

g_id GGAudioManager::SoundCreateFromData(const signed short *samples,size_t sampleCount,int rate, bool stereo)
{
    return soundManager_->SoundCreateFromData(samples,sampleCount,rate,stereo);
}

size_t GGAudioManager::SoundReadFile(const char *fileName, double start, double length, std::vector<unsigned char> &data,int &numChannels,int &bytesPerSample, int &sampleRate, gaudio_Error *error)
{
    return soundManager_->SoundReadFile(fileName,start,length,data,numChannels,bytesPerSample,sampleRate,error);
}

void GGAudioManager::SoundDelete(g_id sound)
{
    soundManager_->SoundDelete(sound);
}

unsigned int GGAudioManager::SoundGetLength(g_id sound)
{
    return soundManager_->SoundGetLength(sound);
}

void GGAudioManager::SoundListener(float x,float y,float z,float vx,float vy,float vz,float dx,float dy,float dz,float ux,float uy,float uz)
{
	soundManager_->SoundListener(x,y,z,vx,vy,vz,dx,dy,dz,ux,uy,uz);
}

g_id GGAudioManager::SoundPlay(g_id sound, bool paused, bool streaming)
{
    return soundManager_->SoundPlay(sound, paused, streaming);
}

bool GGAudioManager::SoundHasEffect(const char *effect)
{
    return soundManager_->SoundHasEffect(effect);
}

void GGAudioManager::ChannelStop(g_id channel)
{
    soundManager_->ChannelStop(channel);
}

void GGAudioManager::ChannelSetPosition(g_id channel, unsigned int position)
{
    soundManager_->ChannelSetPosition(channel, position);
}

unsigned int GGAudioManager::ChannelGetPosition(g_id channel)
{
    return soundManager_->ChannelGetPosition(channel);
}

void GGAudioManager::ChannelSetPaused(g_id channel, bool paused)
{
    soundManager_->ChannelSetPaused(channel, paused);
}

bool GGAudioManager::ChannelIsPaused(g_id channel)
{
    return soundManager_->ChannelIsPaused(channel);
}

bool GGAudioManager::ChannelIsPlaying(g_id channel, int *bufferSize, float *bufferSeconds)
{
    return soundManager_->ChannelIsPlaying(channel, bufferSize, bufferSeconds);
}

void GGAudioManager::ChannelSetVolume(g_id channel, float volume, float balance)
{
    soundManager_->ChannelSetVolume(channel, volume, balance);
}

float GGAudioManager::ChannelGetVolume(g_id channel)
{
    return soundManager_->ChannelGetVolume(channel);
}

g_id GGAudioManager::ChannelGetStreamId(g_id channel)
{
    return soundManager_->ChannelGetStreamId(channel);
}

void GGAudioManager::ChannelSetPitch(g_id channel, float pitch)
{
    soundManager_->ChannelSetPitch(channel, pitch);
}

float GGAudioManager::ChannelGetPitch(g_id channel)
{
    return soundManager_->ChannelGetPitch(channel);
}

void GGAudioManager::ChannelSetLooping(g_id channel, bool looping)
{
    soundManager_->ChannelSetLooping(channel, looping);
}

bool GGAudioManager::ChannelIsLooping(g_id channel)
{
    return soundManager_->ChannelIsLooping(channel);
}

void GGAudioManager::ChannelSetWorldPosition(g_id channel, float x,float y,float z,float vx,float vy,float vz)
{
    return soundManager_->ChannelSetWorldPosition(channel,x,y,z,vx,vy,vz);
}

void GGAudioManager::ChannelSetEffect(g_id channel, const char *effect, float *params)
{
    return soundManager_->ChannelSetEffect(channel, effect, params);
}

g_id GGAudioManager::ChannelAddCallback(g_id channel, gevent_Callback callback, void *udata)
{
    return soundManager_->ChannelAddCallback(channel, callback, udata);
}

void GGAudioManager::ChannelRemoveCallback(g_id channel, gevent_Callback callback, void *udata)
{
    soundManager_->ChannelRemoveCallback(channel, callback, udata);
}

void GGAudioManager::ChannelRemoveCallbackWithGid(g_id channel, g_id gid)
{
    soundManager_->ChannelRemoveCallbackWithGid(channel, gid);
}

g_bool GGAudioManager::BackgroundMusicIsAvailable()
{
    return backgroundMusicInterface_ != NULL;
}

g_id GGAudioManager::BackgroundMusicCreateFromFile(const char *fileName, gaudio_Error *error)
{
    if (backgroundMusicInterface_ == NULL)
        return 0;

    return backgroundMusicInterface_->BackgroundMusicCreateFromFile(fileName, error);
}

void GGAudioManager::BackgroundMusicDelete(g_id backgroundMusic)
{
    if (backgroundMusicInterface_ == NULL)
        return;

    backgroundMusicInterface_->BackgroundMusicDelete(backgroundMusic);
}

unsigned int GGAudioManager::BackgroundMusicGetLength(g_id backgroundMusic)
{
    if (backgroundMusicInterface_ == NULL)
        return 0;

    return backgroundMusicInterface_->BackgroundMusicGetLength(backgroundMusic);
}

g_id GGAudioManager::BackgroundMusicPlay(g_id backgroundMusic, bool paused)
{
    if (backgroundMusicInterface_ == NULL)
        return 0;

    return backgroundMusicInterface_->BackgroundMusicPlay(backgroundMusic, paused);
}

void GGAudioManager::BackgroundChannelStop(g_id backgroundChannel)
{
    if (backgroundMusicInterface_ == NULL)
        return;

    backgroundMusicInterface_->BackgroundChannelStop(backgroundChannel);
}

void GGAudioManager::BackgroundChannelSetPosition(g_id backgroundChannel, unsigned int position)
{
    if (backgroundMusicInterface_ == NULL)
        return;

    backgroundMusicInterface_->BackgroundChannelSetPosition(backgroundChannel, position);
}

unsigned int GGAudioManager::BackgroundChannelGetPosition(g_id backgroundChannel)
{
    if (backgroundMusicInterface_ == NULL)
        return 0;

    return backgroundMusicInterface_->BackgroundChannelGetPosition(backgroundChannel);
}

void GGAudioManager::BackgroundChannelSetPaused(g_id backgroundChannel, bool paused)
{
    if (backgroundMusicInterface_ == NULL)
        return;

    backgroundMusicInterface_->BackgroundChannelSetPaused(backgroundChannel, paused);
}

bool GGAudioManager::BackgroundChannelIsPaused(g_id backgroundChannel)
{
    if (backgroundMusicInterface_ == NULL)
        return false;

    return backgroundMusicInterface_->BackgroundChannelIsPaused(backgroundChannel);
}

bool GGAudioManager::BackgroundChannelIsPlaying(g_id backgroundChannel, int *bufferSize, float *bufferSeconds)
{
    if (backgroundMusicInterface_ == NULL)
        return false;

    return backgroundMusicInterface_->BackgroundChannelIsPlaying(backgroundChannel, bufferSize, bufferSeconds);
}

void GGAudioManager::BackgroundChannelSetVolume(g_id backgroundChannel, float volume)
{
    if (backgroundMusicInterface_ == NULL)
        return;

    backgroundMusicInterface_->BackgroundChannelSetVolume(backgroundChannel, volume);
}

float GGAudioManager::BackgroundChannelGetVolume(g_id backgroundChannel)
{
    if (backgroundMusicInterface_ == NULL)
        return 0.f;

    return backgroundMusicInterface_->BackgroundChannelGetVolume(backgroundChannel);
}

void GGAudioManager::BackgroundChannelSetLooping(g_id backgroundChannel, bool looping)
{
    if (backgroundMusicInterface_ == NULL)
        return;

    backgroundMusicInterface_->BackgroundChannelSetLooping(backgroundChannel, looping);
}

bool GGAudioManager::BackgroundChannelIsLooping(g_id backgroundChannel)
{
    if (backgroundMusicInterface_ == NULL)
        return false;

    return backgroundMusicInterface_->BackgroundChannelIsLooping(backgroundChannel);
}

g_id GGAudioManager::BackgroundChannelAddCallback(g_id backgroundChannel, gevent_Callback callback, void *udata)
{
    if (backgroundMusicInterface_ == NULL)
        return 0;

    return backgroundMusicInterface_->BackgroundChannelAddCallback(backgroundChannel, callback, udata);
}

void GGAudioManager::BackgroundChannelRemoveCallback(g_id backgroundChannel, gevent_Callback callback, void *udata)
{
    if (backgroundMusicInterface_ == NULL)
        return;

    backgroundMusicInterface_->BackgroundChannelRemoveCallback(backgroundChannel, callback, udata);
}

void GGAudioManager::BackgroundChannelRemoveCallbackWithGid(g_id backgroundChannel, g_id gid)
{
    if (backgroundMusicInterface_ == NULL)
        return;

    backgroundMusicInterface_->BackgroundChannelRemoveCallbackWithGid(backgroundChannel, gid);
}

void GGAudioManager::tick_s(int type, void *event, void *udata)
{
    GGAudioManager *manager = static_cast<GGAudioManager*>(udata);

    if (type == GEVENT_PRE_TICK_EVENT)
        manager->preTick();
    else if (type == GEVENT_POST_TICK_EVENT)
        manager->postTick();
}

void GGAudioManager::preTick()
{
    soundManager_->preTick();
    if (backgroundMusicInterface_)
        backgroundMusicInterface_->preTick();
}

void GGAudioManager::postTick()
{
    soundManager_->postTick();
    if (backgroundMusicInterface_)
        backgroundMusicInterface_->postTick();
}

void GGAudioManager::RegisterType(const char *name,GGAudioLoader &loader)
{
	soundManager_->registerLoader(name, loader);
}

void GGAudioManager::UnregisterType(const char *name)
{
	soundManager_->unregisterLoader(name);
}

void GGAudioManager::RegisterEncoderType(const char *name,GGAudioEncoder &loader)
{
	soundManager_->registerEncoder(name, loader);
}

void GGAudioManager::UnregisterEncoderType(const char *name)
{
	soundManager_->unregisterEncoder(name);
}

GGAudioEncoder *GGAudioManager::LookupEncoder(const char *filename)
{
	return soundManager_->lookupEncoder(filename);
}

static GGAudioManager *s_manager = NULL;

extern "C" {

void gaudio_Init()
{
    s_manager = new GGAudioManager();
}

void gaudio_Cleanup()
{
    delete s_manager;
    s_manager = NULL;
}


g_id gaudio_SoundCreateFromFile(const char *fileName, g_bool stream, gaudio_Error *error)
{
    return s_manager->SoundCreateFromFile(fileName, stream, error);
}

g_id gaudio_SoundCreateFromData(const signed short *samples,size_t sampleCount,int rate, bool stereo)
{
    return s_manager->SoundCreateFromData(samples,sampleCount,rate,stereo);
}

size_t gaudio_SoundReadFile(const char *fileName, double start, double length, std::vector<unsigned char> &data,int &numChannels,int &bytesPerSample, int &sampleRate, gaudio_Error *error)
{
    return s_manager->SoundReadFile(fileName, start, length, data, numChannels, bytesPerSample, sampleRate, error);
}

void gaudio_SoundDelete(g_id sound)
{
    s_manager->SoundDelete(sound);
}

unsigned int gaudio_SoundGetLength(g_id sound)
{
    return s_manager->SoundGetLength(sound);
}


g_id gaudio_SoundPlay(g_id sound, g_bool paused, g_bool streaming)
{
    return s_manager->SoundPlay(sound, paused, streaming);
}

void gaudio_SoundListener(float x,float y,float z,float vx,float vy,float vz,float dx,float dy,float dz,float ux,float uy,float uz)
{
	s_manager->SoundListener(x,y,z,vx,vy,vz,dx,dy,dz,ux,uy,uz);
}

bool gaudio_SoundHasEffect(const char *effect)
{
	return s_manager->SoundHasEffect(effect);
}

void gaudio_ChannelStop(g_id channel)
{
    s_manager->ChannelStop(channel);
}

void gaudio_ChannelSetPosition(g_id channel, unsigned int position)
{
    s_manager->ChannelSetPosition(channel, position);
}

unsigned int gaudio_ChannelGetPosition(g_id channel)
{
    return s_manager->ChannelGetPosition(channel);
}

void gaudio_ChannelSetPaused(g_id channel, g_bool paused)
{
    s_manager->ChannelSetPaused(channel, paused);
}

g_bool gaudio_ChannelIsPaused(g_id channel)
{
    return s_manager->ChannelIsPaused(channel);
}

g_bool gaudio_ChannelIsPlaying(g_id channel, int *bufferSize, float *bufferSeconds)
{
    return s_manager->ChannelIsPlaying(channel, bufferSize, bufferSeconds);
}

void gaudio_ChannelSetVolume(g_id channel, float volume, float balance)
{
    s_manager->ChannelSetVolume(channel, volume, balance);
}

float gaudio_ChannelGetVolume(g_id channel)
{
    return s_manager->ChannelGetVolume(channel);
}

g_id gaudio_ChannelGetStreamId(g_id channel)
{
    return s_manager->ChannelGetStreamId(channel);
}

void gaudio_ChannelSetPitch(g_id channel, float pitch)
{
    s_manager->ChannelSetPitch(channel, pitch);
}

float gaudio_ChannelGetPitch(g_id channel)
{
    return s_manager->ChannelGetPitch(channel);
}

void gaudio_ChannelSetLooping(g_id channel, g_bool looping)
{
    s_manager->ChannelSetLooping(channel, looping);
}

g_bool gaudio_ChannelIsLooping(g_id channel)
{
    return s_manager->ChannelIsLooping(channel);
}

void gaudio_ChannelSetWorldPosition(g_id channel, float x,float y,float z,float vx,float vy,float vz)
{
	s_manager->ChannelSetWorldPosition(channel, x,y,z,vx,vy,vz);
}

void gaudio_ChannelSetEffect(g_id channel, const char *effect, float *params)
{
	s_manager->ChannelSetEffect(channel, effect, params);
}

g_id gaudio_ChannelAddCallback(g_id channel, gevent_Callback callback, void *udata)
{
    return s_manager->ChannelAddCallback(channel, callback, udata);
}

void gaudio_ChannelRemoveCallback(g_id channel, gevent_Callback callback, void *udata)
{
    s_manager->ChannelRemoveCallback(channel, callback, udata);
}

void gaudio_ChannelRemoveCallbackWithGid(g_id channel, g_id gid)
{
    s_manager->ChannelRemoveCallbackWithGid(channel, gid);
}

g_bool gaudio_BackgroundMusicIsAvailable()
{
    return s_manager->BackgroundMusicIsAvailable();
}

g_id gaudio_BackgroundMusicCreateFromFile(const char *fileName, gaudio_Error *error)
{
    return s_manager->BackgroundMusicCreateFromFile(fileName, error);
}

void gaudio_BackgroundMusicDelete(g_id backgroundMusic)
{
    s_manager->BackgroundMusicDelete(backgroundMusic);
}

unsigned int gaudio_BackgroundMusicGetLength(g_id backgroundMusic)
{
    return s_manager->BackgroundMusicGetLength(backgroundMusic);
}

g_id gaudio_BackgroundMusicPlay(g_id backgroundMusic, g_bool paused, g_bool streaming)
{
    return s_manager->BackgroundMusicPlay(backgroundMusic, paused);
}

void gaudio_BackgroundChannelStop(g_id backgroundChannel)
{
    s_manager->BackgroundChannelStop(backgroundChannel);
}

void gaudio_BackgroundChannelSetPosition(g_id backgroundChannel, unsigned int position)
{
    s_manager->BackgroundChannelSetPosition(backgroundChannel, position);
}

unsigned int gaudio_BackgroundChannelGetPosition(g_id backgroundChannel)
{
    return s_manager->BackgroundChannelGetPosition(backgroundChannel);
}

void gaudio_BackgroundChannelSetPaused(g_id backgroundChannel, g_bool paused)
{
    s_manager->BackgroundChannelSetPaused(backgroundChannel, paused);
}

g_bool gaudio_BackgroundChannelIsPaused(g_id backgroundChannel)
{
    return s_manager->BackgroundChannelIsPaused(backgroundChannel);
}

g_bool gaudio_BackgroundChannelIsPlaying(g_id backgroundChannel, int *bufferSize, float *bufferSeconds)
{
    return s_manager->BackgroundChannelIsPlaying(backgroundChannel, bufferSize, bufferSeconds);
}

void gaudio_BackgroundChannelSetVolume(g_id backgroundChannel, float volume, float balance)
{
    s_manager->BackgroundChannelSetVolume(backgroundChannel, volume);
}

float gaudio_BackgroundChannelGetVolume(g_id backgroundChannel)
{
    return s_manager->BackgroundChannelGetVolume(backgroundChannel);
}

void gaudio_BackgroundChannelSetLooping(g_id backgroundChannel, g_bool looping)
{
    s_manager->BackgroundChannelSetLooping(backgroundChannel, looping);
}

g_bool gaudio_BackgroundChannelIsLooping(g_id backgroundChannel)
{
    return s_manager->BackgroundChannelIsLooping(backgroundChannel);
}

g_id gaudio_BackgroundChannelAddCallback(g_id backgroundChannel, gevent_Callback callback, void *udata)
{
    return s_manager->BackgroundChannelAddCallback(backgroundChannel, callback, udata);
}

void gaudio_BackgroundChannelRemoveCallback(g_id backgroundChannel, gevent_Callback callback, void *udata)
{
    s_manager->BackgroundChannelRemoveCallback(backgroundChannel, callback, udata);
}

void gaudio_BackgroundChannelRemoveCallbackWithGid(g_id backgroundChannel, g_id gid)
{
    s_manager->BackgroundChannelRemoveCallbackWithGid(backgroundChannel, gid);
}

void gaudio_AdvanceStreamBuffers()
{
	s_manager->AdvanceStreamBuffers();
}

void gaudio_registerType(const char *name,GGAudioLoader &loader) {
	s_manager->RegisterType(name,loader);
}

void gaudio_unregisterType(const char *name) {
	s_manager->UnregisterType(name);
}

void gaudio_registerEncoderType(const char *name,GGAudioEncoder &encoder) {
	s_manager->RegisterEncoderType(name,encoder);
}

void gaudio_unregisterEncoderType(const char *name) {
	s_manager->UnregisterEncoderType(name);
}

GGAudioEncoder *gaudio_lookupEncoder(const char *filename) {
	return s_manager->LookupEncoder(filename);
}


}
