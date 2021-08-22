#include "StdAfx.h"
#include "PcmSpeaker.h"

CPcmSpeaker::CPcmSpeaker(int bufferSize, int bufferCnt)
{
	m_hWaveOut = NULL;
	m_hBufferEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	InitializeCriticalSection(&m_BufferOpCriticalSection);

	//申请内存
	m_headAndDatas = new WaveHeadandData[bufferCnt];
	for (int i = 0; i < bufferCnt; i++)
	{
		memset(&m_headAndDatas[i].header, 0, sizeof(WAVEHDR));
		m_headAndDatas[i].header.dwFlags = WHDR_DONE;
		m_headAndDatas[i].data = new char[bufferSize];
	}

	m_maxBufferSize = bufferSize;
	m_maxBufferCnt = bufferCnt;
}


CPcmSpeaker::~CPcmSpeaker()
{
	//关闭Wave
	if (m_hWaveOut != NULL)
	{
		clearPcmData();
		waveOutClose(m_hWaveOut);
		m_hWaveOut = NULL;
	}

	//关闭一些句柄
	CloseHandle(m_hBufferEvent);

	//删除临界区
	DeleteCriticalSection(&m_BufferOpCriticalSection);

	//释放内存
	for (int i = 0; i < m_maxBufferCnt; i++)
		delete[] m_headAndDatas[i].data;

	delete[] m_headAndDatas;
}


int CPcmSpeaker::init(int channels, int samplePerSec, int bitsPerSample)
{
	if (m_hWaveOut != NULL) {
		return 0;// 已经进行了初始化
	}

	// 第一步: 获取waveformat信息
	m_waveFormat.wFormatTag = WAVE_FORMAT_PCM;
	m_waveFormat.nChannels = channels;
	m_waveFormat.wBitsPerSample = bitsPerSample;
	m_waveFormat.nSamplesPerSec = samplePerSec;
	m_waveFormat.nBlockAlign =
		m_waveFormat.nChannels * m_waveFormat.wBitsPerSample / 8;
	m_waveFormat.nAvgBytesPerSec =
		m_waveFormat.nSamplesPerSec * m_waveFormat.nBlockAlign;
	m_waveFormat.cbSize = sizeof(m_waveFormat);

	MMRESULT ret = waveOutOpen(NULL, WAVE_MAPPER, &m_waveFormat,
		NULL, NULL, WAVE_FORMAT_QUERY);
	if (MMSYSERR_NOERROR != ret) {
		return -1;
	}

	// 第二步: 获取WAVEOUT句柄
	ret = waveOutOpen(&m_hWaveOut, WAVE_MAPPER, &m_waveFormat,
		(DWORD_PTR)waveOutProc, (DWORD_PTR)this, CALLBACK_FUNCTION);

	if (MMSYSERR_NOERROR != ret) {
		return -1;
	}

	return 0;
}

void CALLBACK CPcmSpeaker::waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
	CPcmSpeaker *render = (CPcmSpeaker *)dwInstance;
	//WAVEHDR *header = (WAVEHDR *)dwParam1;
	int i = 0;
	switch (uMsg)
	{
	case WOM_DONE:
		EnterCriticalSection(&render->m_BufferOpCriticalSection);
		SetEvent(render->m_hBufferEvent);
		LeaveCriticalSection(&render->m_BufferOpCriticalSection);
		break;
	case WOM_CLOSE:
		i = 1;
		break;
	case WOM_OPEN:
		i = 2;
		break;
	}
}

int CPcmSpeaker::clearPcmData()
{
	if (m_hWaveOut != NULL)
	{
		EnterCriticalSection(&m_BufferOpCriticalSection);
		for (int i = 0; i < m_maxBufferCnt; i++)
		{
			if (m_headAndDatas[i].header.dwFlags & WHDR_PREPARED) //有数据被Prepered
				waveOutUnprepareHeader(m_hWaveOut, &m_headAndDatas[i].header, sizeof(WAVEHDR));
		}

		waveOutReset(m_hWaveOut);
		LeaveCriticalSection(&m_BufferOpCriticalSection);
	}
	return 0;
}

int CPcmSpeaker::writeToWave(const void *data, int len)
{
	MMRESULT mmres;
	int i;
	EnterCriticalSection(&m_BufferOpCriticalSection);
	for (i = 0; i < m_maxBufferCnt; i++)
		if (m_headAndDatas[i].header.dwFlags & WHDR_DONE)
		{
			//查看是否需要释放之前已经Prepared资源
			if (m_headAndDatas[i].header.dwFlags & WHDR_PREPARED) //有数据被Prepered
				waveOutUnprepareHeader(m_hWaveOut, &m_headAndDatas[i].header, sizeof(WAVEHDR));

			//写入新的数据到音频缓冲区      
			memcpy(m_headAndDatas[i].data, data, len);
			m_headAndDatas[i].header.lpData = m_headAndDatas[i].data;
			m_headAndDatas[i].header.dwBufferLength = len;
			m_headAndDatas[i].header.dwFlags = 0;
			Sleep(100);
			mmres = waveOutPrepareHeader(m_hWaveOut, &m_headAndDatas[i].header, sizeof(WAVEHDR));
			
			if (MMSYSERR_NOERROR == mmres)
				mmres = waveOutWrite(m_hWaveOut, &m_headAndDatas[i].header, sizeof(WAVEHDR));
			Sleep(100);
			break;
		}
		LeaveCriticalSection(&m_BufferOpCriticalSection);

		if (i == m_maxBufferCnt)
			return -2;

		return (mmres == MMSYSERR_NOERROR) ? 0 : -1;
}

//添加PCM音频数据，等待播放
int CPcmSpeaker::pcmtoWave(const void *data, int len, int timeout)
{
	int res;

	if (len > m_maxBufferSize)
		return -1;

	res = writeToWave(data, len);

	
	//缓冲区已满，需要等待
	if (res == -2)
	{       
		if (WAIT_OBJECT_0 == WaitForSingleObject(m_hBufferEvent, timeout))
			res = writeToWave(data, len);
	}
	

	return res;
}

int CPcmSpeaker::toSpeaker(const void *data, int len, int timeout)
{
	int res;
	int n, l, ptr;

	//对大数据做分段处理
	n = len / m_maxBufferSize;
	l = len % m_maxBufferSize;
	ptr = 0;

	for (int i = 0; i < n; i++)
	{
		Sleep(2000);
		res = pcmtoWave(((char *)data) + ptr, m_maxBufferSize, timeout);
		ptr += m_maxBufferSize;
		if (res != 0)
			return -1;
	}

	return pcmtoWave(((char *)data) + ptr, l, timeout);
}
