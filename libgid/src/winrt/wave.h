#ifndef WAVE_H
#define WAVE_H

#include <xaudio2.h>
#include <fstream>
#include <assert.h>

// The Wave class contains all information needed about a buffer including WAVEFORMATEX (bits per second, channels)
// and XAUDIO2_BUFFER (the actual XAUDIO2 buffer): essentially just a pointer to the raw data in memory (duplicated as m_waveData)
// The WAVEFORMATEX is used when createing a Source Voice. The XAUDIO2_BUFFER is used when submitting a buffer to the voice
// To play WAV files, we use Create to get all info then create a source voice and submit a buffer immediately so using all info
// To play MP3 files, we use Create but ignore the WAVEFORMATEX stuff. Only the XAUDIO2_BUFFER info is used.
// Also has a load method to load a WAV file off disk but this is not used in Gideros.

class Wave
{
private:
	WAVEFORMATEX m_wf;
	XAUDIO2_BUFFER m_xa;
	BYTE* m_waveData;
	int m_capacity;
public:
	Wave(const char* szFile = NULL) : m_waveData(NULL), m_capacity(0)
	{
		ZeroMemory(&m_wf, sizeof(m_wf));
		ZeroMemory(&m_xa, sizeof(m_xa));

		load(szFile);
	}

	Wave(const Wave& c) : m_waveData(NULL) 
	{
		m_wf = c.m_wf;
		m_xa = c.m_xa;
		if(c.m_waveData)
		{
			m_waveData = new BYTE[m_xa.AudioBytes];
			memcpy( m_waveData, c.m_waveData, m_xa.AudioBytes );
			m_xa.pAudioData = m_waveData;
		}
	}

	~Wave() 
	{
		if(m_waveData)
			delete [] m_waveData;

		m_waveData = NULL;
		m_xa.pAudioData = NULL;
	}

	void Destroy()
	{
	  if (m_waveData)
	    delete [] m_waveData;
	  m_waveData=NULL;
	}

	void Create(const void *data, int numChannels, int sampleRate, int bitsPerSample, int numSamples, int size=0)
	{
		m_wf.wFormatTag = WAVE_FORMAT_PCM;
		m_wf.nChannels = numChannels;
		m_wf.nSamplesPerSec = sampleRate;
		m_wf.nAvgBytesPerSec = numChannels * sampleRate * bitsPerSample / 8;
		m_wf.nBlockAlign = bitsPerSample * numChannels / 8;
		m_wf.wBitsPerSample = bitsPerSample;
		m_wf.cbSize = 0;

		int dsize;
		if (size == 0)
			dsize = numSamples * numChannels * (bitsPerSample / 8);
		else
			dsize = size;

		if ((m_waveData == NULL) || (dsize > m_capacity)) {  // new or enlarge
			if (m_waveData != NULL)
				delete [] m_waveData;

			m_waveData = new BYTE[dsize];
			m_xa.pAudioData = m_waveData;
			m_capacity = dsize;
		}
	
		memcpy(m_waveData, data, dsize);
		m_xa.AudioBytes = dsize;
		m_xa.PlayBegin = 0;
		m_xa.PlayLength = 0;
		m_xa.LoopBegin = 0;
		m_xa.LoopLength = 0;
		m_xa.LoopCount = 0;
		m_xa.pContext = this;
	}

	const XAUDIO2_BUFFER* xaBuffer() const 
	{
		return &m_xa;
	}

	const WAVEFORMATEX* wf() const 
	{
		return &m_wf;
	}

	bool load(const char* szFile) 
	{
		if(szFile == NULL)
			return false;

		std::ifstream inFile(szFile, std::ios::binary | std::ios::in);
		if(inFile.bad())
			return false;
		
		DWORD dwChunkId = 0, dwFileSize = 0, dwChunkSize = 0, dwExtra = 0;

		//look for 'RIFF' chunk identifier
		inFile.seekg(0, std::ios::beg);
		inFile.read(reinterpret_cast<char*>(&dwChunkId), sizeof(dwChunkId));
		if(dwChunkId != 'FFIR')
		{
			inFile.close();
			return false;
		}
		inFile.seekg(4, std::ios::beg); //get file size
		inFile.read(reinterpret_cast<char*>(&dwFileSize), sizeof(dwFileSize));
		if(dwFileSize <= 16)
		{
			inFile.close();
			return false;
		}
		inFile.seekg(8, std::ios::beg); //get file format
		inFile.read(reinterpret_cast<char*>(&dwExtra), sizeof(dwExtra));
		if(dwExtra != 'EVAW')
		{
			inFile.close();
			return false;
		}

		//look for 'fmt ' chunk id
		bool bFilledFormat = false;
		for(unsigned int i = 12; i < dwFileSize; )
		{
			inFile.seekg(i, std::ios::beg);
			inFile.read(reinterpret_cast<char*>(&dwChunkId), sizeof(dwChunkId));
			inFile.seekg(i + 4, std::ios::beg);
			inFile.read(reinterpret_cast<char*>(&dwChunkSize), sizeof(dwChunkSize));
			if(dwChunkId == ' tmf')
			{
				//I don't know what I was thinking with the following code, but I
				//never did solve it back 6 months, and didn't touch it since; oh well... :S

				//switch(dwChunkSize)
				//{
				//case sizeof(WAVEFORMATEX):
				//	{
				//		inFile.seekg(i + 8, std::ios::beg);
				//		inFile.read(reinterpret_cast<char*>(&m_wf), sizeof(m_wf));
				//	}
				//	break;
				//case sizeof(WAVEFORMATEXTENSIBLE):
				//	{
				//		WAVEFORMATEXTENSIBLE wfe;
				//		inFile.seekg(i + 8, std::ios::beg);
				//		inFile.read(reinterpret_cast<char*>(&wfe), sizeof(wfe));
				//		m_wf = wfe.Format;
				//	}
				//	break;
				//default:
				//	inFile.close();
				//	return;
				//}
				inFile.seekg(i + 8, std::ios::beg);
				inFile.read(reinterpret_cast<char*>(&m_wf), sizeof(m_wf));
				bFilledFormat = true;
				break;
			}
			dwChunkSize += 8; //add offsets of the chunk id, and chunk size data entries
			dwChunkSize += 1;
			dwChunkSize &= 0xfffffffe; //guarantees WORD padding alignment
			i += dwChunkSize;
		}
		if(!bFilledFormat)
		{
			inFile.close();
			return false;
		}

		//look for 'data' chunk id
		bool bFilledData = false;
		for(unsigned int i = 12; i < dwFileSize; )
		{
			inFile.seekg(i, std::ios::beg);
			inFile.read(reinterpret_cast<char*>(&dwChunkId), sizeof(dwChunkId));
			inFile.seekg(i + 4, std::ios::beg);
			inFile.read(reinterpret_cast<char*>(&dwChunkSize), sizeof(dwChunkSize));
			if(dwChunkId == 'atad')
			{
				m_waveData = new BYTE[dwChunkSize];
				inFile.seekg(i + 8, std::ios::beg);
				inFile.read(reinterpret_cast<char*>(m_waveData), dwChunkSize);
				m_xa.AudioBytes = dwChunkSize;
				m_xa.pAudioData = m_waveData;
				m_xa.PlayBegin = 0;
				m_xa.PlayLength = 0;
				bFilledData = true;
				break;
			}
			dwChunkSize += 8; //add offsets of the chunk id, and chunk size data entries
			dwChunkSize += 1;
			dwChunkSize &= 0xfffffffe; //guarantees WORD padding alignment
			i += dwChunkSize;
		}
		if(!bFilledData)
		{
			inFile.close();
			return false;
		}

		inFile.close();
		return true;
	}
};

#endif
