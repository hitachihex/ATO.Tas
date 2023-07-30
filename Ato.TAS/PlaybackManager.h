#pragma once
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include "InputRecord.h"
#include "EasyHookUtils.h"

extern bool g_bPaused;
extern unsigned long g_GameSpeedPlayerStep;
extern bool g_bPressedFrameStepThisFrame;

enum eBreakType
{
	BREAKTYPE_DEFAULT_NONE = 0x0,
	BREAKTYPE_FAST = 1 << 0,
	BREAKTYPE_NORMAL = 1 << 1
};

typedef struct t_BreakState
{
	// The file the break is in.
	char m_szCurrentFile[56];

	// The line number the break is in.
	unsigned int m_nLineNo;

	// The breakpoint type.
	eBreakType m_BreakType;
} BreakState;

class PlaybackManager
{
public:

	PlaybackManager(const char*);

	InputRecord * GetCurrentInput();

	InputRecord * GetCurrentInputIndexBased();

	unsigned long GetCurrentInputIndex();

	void DoPlayback(bool);

	unsigned long ReloadPlayback();

	bool ReadInputFile();

	bool ReadMultiLevelInputFile(const char *, unsigned long, unsigned long*, unsigned long*);

	void InitPlayback(bool);

	bool IsPlayingBack();

	unsigned long GetLastReadSeed();

	unsigned long GetTotalFrameCount();

	void FormatManagerString();


	char m_szCurrentManagerState[2000];

	unsigned long m_CurrentFrame;

	unsigned long m_nTotalFrameCount;

	bool m_bPlayingBack;

	std::string m_CWD;

private:

	const char * m_szDefaultFileName = "Ato.rec";

	FILE * m_pSegmentedFile;
	
	BreakState m_BreakState;

	char m_szCurrentDirectory[256];

	InputRecord * m_pCurrentInput;

	std::vector<InputRecord*> m_Inputs;

	unsigned long m_InputIndex;

	FILE * m_Fp;

	unsigned long m_nLastReadSeed;

	unsigned long m_RuntoLineNo;

	unsigned long m_WalktoLineNo;

	unsigned long m_TotalFrameCountOfInputFile;

	unsigned long m_FrameToNext;

	bool m_bPlaybackReady;

protected:

};

extern PlaybackManager * g_pPlaybackMgr;