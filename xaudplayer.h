
/* xaudio general includes */
#include "media\xaudio\decoder.h"

/* xaudio plug-in modules */
#include "media\xaudio\memory_input.h"
#include "media\xaudio\audio_output.h"
#include "media\xaudio\mpeg_codec.h"

class XAudPlayer
	{
public:
	XAudPlayer::XAudPlayer();
	XAudPlayer::~XAudPlayer();

	int Init(char *szFileName, int volume);

	int Tick();

	void End();

	void Pause();
	void Unpause();

	XA_DecoderInfo *m_decoder;

	FILE           *file;

	BOOL m_fStarted;

	BOOL m_fEndData; // all data has been decoded - wait for buffer to play
	int m_cDataLeft;
	int m_lastplaypos;

	HRESULT CreateBuffer(int volume); 
	HRESULT CreateStreamingBuffer(WAVEFORMATEX *pwfx);

#define NUM_PLAY_NOTIFICATIONS  16

	//LPDIRECTSOUND       m_pDS            = NULL;
	LPDIRECTSOUNDBUFFER m_pDSBuffer;
	LPDIRECTSOUNDNOTIFY m_pDSNotify;

	DSBPOSITIONNOTIFY   m_aPosNotify;  
	HANDLE              m_hNotificationEvent;

	DWORD               m_dwBufferSize;
	DWORD               m_dwNotifySize;
	DWORD               m_dwNextWriteOffset;
	DWORD               m_dwProgress;
	DWORD               m_dwLastPos;
	BOOL                m_bFoundEnd;
	};