#pragma once

#include <Windows.h> 
#include "mmsystem.h" 

#pragma comment(lib, "winmm.lib") 

#define DEF_MAX_BUFFER_SIZE (1024 * 16)
#define DEF_MAX_BUFFER_COUNT 16

class CPcmSpeaker
{
public:
	CPcmSpeaker(int bufferSize = DEF_MAX_BUFFER_SIZE, int bufferCnt = DEF_MAX_BUFFER_COUNT);
	~CPcmSpeaker();

	int init(int channels, int samplePerSec, int bitsPerSample);

	//添加PCM音频数据，等待播放
	int toSpeaker(const void *data, int len, int timeout = INFINITE);
	int clearPcmData();

private:

	typedef struct
	{
		WAVEHDR header;
		char *data;
	}WaveHeadandData;

	int m_maxBufferSize;
	int m_maxBufferCnt;

	WaveHeadandData *m_headAndDatas;
	static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
	int writeToWave(const void *data, int len);
	int pcmtoWave(const void *data, int len, int timeout = INFINITE);

	// 公共信息
	WAVEFORMATEX m_waveFormat;
	HWAVEOUT m_hWaveOut; // WAVEOUT句柄
	HANDLE m_hBufferEvent;
	CRITICAL_SECTION m_BufferOpCriticalSection;
};