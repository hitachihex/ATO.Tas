#include "stdafx.h"
#include "Hooks.h"
#include "GMEvent.h"
#include "DebugTools.h"
#include "Ato.TAS.h"
#include "PlaybackManager.h"

#pragma warning(disable : 4996)
oEventLoop original_EventLoop = (oEventLoop)(0x0);
oYoyoUpdate original_YoyoUpdate = (oYoyoUpdate)(0x0);

YYGS_RetValDouble * pYYGSDoubleRet = new YYGS_RetValDouble();
YYGS_RetValUnknown * pYYGSUnkRet = new YYGS_RetValUnknown();
YYGNumberAndStringArgument nas;
YYGString k;

bool __cdecl EventLoop_Hook(void * pvArg1, void * pvArg2, void *pvArg3, void *pvArg4, void * pvArg5)
{
	GameMaker_Event * pEvent = (GameMaker_Event*)(pvArg3);

	if (!stricmp(pEvent->m_pszScriptName, "gml_Object_oInput_Step_0"))
	{
		if (g_pPlaybackMgr)
		{
			if (g_pPlaybackMgr->IsPlayingBack())
			{
				g_pPlaybackMgr->DoPlayback(g_bPressedFrameStepThisFrame);
			}
		}

	}

	if (!stricmp(pEvent->m_pszScriptName, "gml_Object_oPlayer_Draw_0"))
	{

	}
		

	// draw rng crap is crashing us at the beginning of the game now..
	if (g_pPlaybackMgr)	
	{
		if (g_pPlaybackMgr->IsPlayingBack())
		{
			g_bDrawRNGFix = false;

			auto * pInput = g_pPlaybackMgr->GetCurrentInput();
			if (pInput)
			{
				if (pInput->IsDRNGFix())
					g_bDrawRNGFix = true;
			}

		}
	}


	return original_EventLoop(pvArg1, pvArg2, pvArg3, pvArg4, pvArg5);

}


void __cdecl YoYoUpdate_Hook()
{
	static bool bOnce = false;

	if (!bOnce)
	{
		DebugOutput("YoyoUpdate_Hook, !bOnce");
		bOnce = true;
	}


	g_bPressedFrameStepThisFrame = false;



	if (GetAsyncKeyState(g_pause_key) & 1 && !g_bPaused)
	{
		g_bPaused = true;

		DebugOutput("Paused.");

		g_GameSpeedPlayerStep = 1;

		//Just return
		return;

	}

	// Start/Stop playback.
	if (GetAsyncKeyState(g_playback_key) & 1)
	{
		g_pPlaybackMgr->InitPlayback(true);
	}
	
	if (GetAsyncKeyState(VK_F9) & 1)
	{
		auto var_id = GetGlobalVariableIndexByName("PlayerAbility", varnames_list_addr);
		VariablePointer* p = get_variable_by_index(var_id, GetGlobalObjectInstance()->m_pInstProps);
		YYGS_RetValArray* pOutArray = new YYGS_RetValArray();
		YYGString yygs;
		yygs.Set("PlayerAbility");
		if (p) {
			
			variable_global_get(pOutArray, 0x00, 0x01, 0x02, &yygs);
			DebugOutput("PlayerAbility array pointer from get_variable_by_index at %p", p->m_pVar->valueArray);

		}
	}

	//playerspeedboostalarm needs to be set to at least 90? and we must be holding attack.

	// Set gamespeed to normal.
	if (GetAsyncKeyState(g_defaultspeed_key) & 1)
	{
		SetDrawEnabled(true);
		g_GameSpeedPlayerStep = 1;
	}

	// Increase game speed.
	if (GetAsyncKeyState(g_increasespeed_key) & 1)
	{

		// TODO: Make configurable hotkey crap

		if ((g_GameSpeedPlayerStep + 1) > 200)
			g_GameSpeedPlayerStep = 200;
		else
			g_GameSpeedPlayerStep++;

		DebugOutput("Game speed is now: %d", g_GameSpeedPlayerStep);
	}

	// Decrease game speed.
	if (GetAsyncKeyState(g_decreasespeed_key) & 1)
	{
		// can't go to 0
		if (!(g_GameSpeedPlayerStep - 1))
			g_GameSpeedPlayerStep = 1;
		else
			g_GameSpeedPlayerStep--;

		DebugOutput("Game speed is now: %d", g_GameSpeedPlayerStep);
	}

	// Not paused, calling Update amount of steps required.
	if (!g_bPaused)
	{
		if (g_GameSpeedPlayerStep > 1)
		{
			for (int i = 0; i < g_GameSpeedPlayerStep; i++)
				original_YoyoUpdate();

			if (g_pPlaybackMgr)
			{
				if (g_pPlaybackMgr->IsPlayingBack())
					g_pPlaybackMgr->FormatManagerString();
			}
			return;
		}
	}

	// Paused, check for framestepping or unpause.
	if (g_bPaused)
	{

		if (GetAsyncKeyState(g_framestep_key) & 1)
		{
			g_bPressedFrameStepThisFrame = true;
			original_YoyoUpdate();

			if (g_pPlaybackMgr)
			{
				if(g_pPlaybackMgr->IsPlayingBack())
					g_pPlaybackMgr->FormatManagerString();
			}

			return;
		}

		// Ugh, need a seperate unpause key. Window doesn't re-process the WM_KEYDOWN message for the original pause button.
		if (GetAsyncKeyState(g_unpause_key) & 1)
		{
			g_bPaused = false;
			DebugOutput("Unpausing.");
			goto fuckitleaveYoYoUpdate;
		}

		return;


	}
fuckitleaveYoYoUpdate:
	original_YoyoUpdate();

	if (g_pPlaybackMgr)
	{
		if (g_pPlaybackMgr->IsPlayingBack())
			g_pPlaybackMgr->FormatManagerString();
	}
}