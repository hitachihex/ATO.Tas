#include "stdafx.h"
#include "PlaybackManager.h"
#include "Addresses.h"
#include "DebugTools.h"
#include "Ato.TAS.h"
#include "Globals.h"

bool g_bPaused = false;
unsigned long g_GameSpeedPlayerStep = 1;
bool g_bPressedFrameStepThisFrame = false;
PlaybackManager * g_pPlaybackMgr = nullptr;

YYGString yygsForManager;
YYGS_RetValDouble * pOutDbl = new YYGS_RetValDouble();
YYGS_RetValArray * pOutArray = new YYGS_RetValArray();

#pragma warning(disable: 4996)
__declspec(noinline) void PressKey(unsigned int keyCode, unsigned char v)
{
	unsigned char * ptr = (unsigned char*)(keyCode + keypressedptr_addr);
	*ptr = v;
}

__declspec(noinline) void HoldKey(unsigned int keyCode, unsigned char v)
{
	unsigned char * ptr = (unsigned char*)(keyCode + keyholdptr_addr);
	*ptr = v;
}

__declspec(noinline) void ReleaseKey(unsigned int keyCode, unsigned char v)
{
	unsigned char * ptr = (unsigned char*)(keyCode + keyreleasedptr_addr);
	*ptr = v;
}

PlaybackManager::PlaybackManager(const char * pszFileName)
{
	yygsForManager.Set("PlayerAbility");
	this->m_BreakState.m_BreakType = eBreakType::BREAKTYPE_DEFAULT_NONE;
	memset(&this->m_BreakState.m_szCurrentFile[0], 0, 56);
	this->m_BreakState.m_nLineNo = -1;

	this->m_nTotalFrameCount = 0;
	this->m_nLastReadSeed = 16;
	this->m_Fp = NULL;
	this->m_pSegmentedFile = NULL;
	this->m_bPlaybackReady = false;
	this->m_bPlayingBack = false;
	memset(&this->m_szCurrentManagerState[0], 0, 2000);

	auto nLen = GetModuleFileNameA(nullptr, this->m_szCurrentDirectory, 256);
	this->m_CWD.assign(this->m_szCurrentDirectory);
	auto indexOfLastBackSlash = this->m_CWD.rfind("\\");
	this->m_CWD.erase(indexOfLastBackSlash + 1, this->m_CWD.length() - indexOfLastBackSlash + 1);
	this->m_CWD += "\\Includes\\";

	if (pszFileName)
	{
		// _SH_DENYNO for shared access.
		this->m_Fp = _fsopen(pszFileName, "r", _SH_DENYNO);
	}
}


bool PlaybackManager::ReadMultiLevelInputFile(const char * _szFileName, unsigned long otherFileCount, unsigned long* pOutRecordsRead, unsigned long *pOutLinesRead)
{
	char LineBuffer[2048] = { 0 };
	unsigned int linecount = otherFileCount;

	unsigned int otherLineCount = 0;

	if (this->m_pSegmentedFile)
	{
		fclose(this->m_pSegmentedFile);
		this->m_pSegmentedFile = nullptr;
	}

	std::string pathToCurrentFile = this->m_CWD;
	pathToCurrentFile += _szFileName;

	this->m_pSegmentedFile = _fsopen(pathToCurrentFile.c_str(), "r", _SH_DENYNO);

	if (this->m_pSegmentedFile == nullptr)
	{
		DebugOutput("!this->m_pSegmentedFile, null pointer from _fsopen , errno=%u", errno);
		return false;
	}

	rewind(this->m_pSegmentedFile);

	while (true)
	{
		if (fgets(LineBuffer, 2048, this->m_pSegmentedFile) == NULL)
			break;

		LineBuffer[strcspn(LineBuffer, "\n")] = 0;

		if (strlen(LineBuffer) == 0)
		{
			++linecount;
			++otherLineCount;
			memset(LineBuffer, 0, sizeof(LineBuffer) / sizeof(LineBuffer[0]));
			continue;
		}

		if (LineBuffer[0] == '#')
		{
			++linecount;
			++otherLineCount;
			memset(LineBuffer, 0, sizeof(LineBuffer) / sizeof(LineBuffer[0]));
			continue;
		}
		
		std::string stringBuffer(LineBuffer);

		unsigned long long indexRunto = stringBuffer.find("Runto");
		unsigned long long  indexWalkto = stringBuffer.find("Walkto");


		if (indexRunto != std::string::npos)
		{
			// ADDED - Setting break state variables
			strcpy(this->m_BreakState.m_szCurrentFile, _szFileName);
			this->m_BreakState.m_nLineNo = otherLineCount;
			this->m_BreakState.m_BreakType = eBreakType::BREAKTYPE_FAST;

			this->m_RuntoLineNo = otherLineCount;
			// still increase linecount
			linecount++;
			otherLineCount++;


			continue;
		}
		else if (indexWalkto != std::string::npos)
		{
			// ADDED - Setting break state variables
			strcpy(this->m_BreakState.m_szCurrentFile, _szFileName);
			this->m_BreakState.m_nLineNo = otherLineCount;
			this->m_BreakState.m_BreakType = eBreakType::BREAKTYPE_NORMAL;

			this->m_WalktoLineNo = otherLineCount;
			// still increase linecount
			linecount++;
			otherLineCount++;


			continue;
		}

		InputRecord * p = new InputRecord(std::string(LineBuffer), ++linecount, _szFileName, ++otherLineCount);

		if (p->m_Frames == -1)
		{
			delete p;
			memset(LineBuffer, 0, sizeof(LineBuffer) / sizeof(LineBuffer[0]));
			continue;
		}

		this->m_nTotalFrameCount += p->m_Frames;
		this->m_Inputs.push_back(p);
		++*pOutRecordsRead;
		memset(LineBuffer, 0, sizeof(LineBuffer) / sizeof(LineBuffer[0]));
	}

	fclose(this->m_pSegmentedFile);
	this->m_pSegmentedFile = nullptr;

	return true;
}

bool PlaybackManager::ReadInputFile()
{
	bool first = true;

	char LineBuffer[2048] = { 0 };
	unsigned int linecount = 0;

	if (this->m_Fp == NULL)
		return false;

	rewind(this->m_Fp);
	this->m_Inputs.clear();

	try
	{

		while (true)
		{

			if (fgets(LineBuffer, 2048, this->m_Fp) == NULL)
				break;

			// Remove the newline
			LineBuffer[strcspn(LineBuffer, "\n")] = 0;

			if (strlen(LineBuffer) == 0)
			{
				++linecount;
				memset(LineBuffer, 0, sizeof(LineBuffer) / sizeof(LineBuffer[0]));
				continue;
			}

			if (LineBuffer[0] == '#')
			{
				++linecount;
				memset(LineBuffer, 0, sizeof(LineBuffer) / sizeof(LineBuffer[0]));
				continue;
			}

			std::string stringBuffer(LineBuffer);
			unsigned int indexRunto = stringBuffer.find("Runto");
			unsigned int indexWalkto = stringBuffer.find("Walkto");
			unsigned int indexRead = stringBuffer.find("Read");

			if (indexRunto != std::string::npos)
			{
				strcpy(this->m_BreakState.m_szCurrentFile, this->m_szDefaultFileName);
				this->m_BreakState.m_nLineNo = linecount;
				this->m_RuntoLineNo = linecount;
				// still increase linecount
				linecount++;
				continue;
			}
			else if (indexWalkto != std::string::npos)
			{
				strcpy(this->m_BreakState.m_szCurrentFile, this->m_szDefaultFileName);
				this->m_BreakState.m_nLineNo = linecount;
				this->m_WalktoLineNo = linecount;
				// still increase linecount
				linecount++;
				continue;
			}

			if (indexRead != std::string::npos)
			{
				unsigned int iod = stringBuffer.find(",");
				if (iod != std::string::npos)
				{
					std::string fn = stringBuffer.substr(iod + 1);
					unsigned long inRecordsRead = 0, outLinesRead = 0;
					
					bool multiResult = this->ReadMultiLevelInputFile(fn.c_str(), linecount, &inRecordsRead, &outLinesRead);


					linecount++;
					continue;
				}

			}

			try
			{

				InputRecord * p = new InputRecord(std::string(LineBuffer), ++linecount, this->m_szDefaultFileName, 0);
				this->m_nTotalFrameCount += p->m_Frames;
				this->m_Inputs.push_back(p);
			}
			catch (std::exception& e)
			{
				DebugOutput("Caught exception: %s", e.what());
			}


			memset(LineBuffer, 0, sizeof(LineBuffer) / sizeof(LineBuffer[0]));

		}

	}
	// shut up c4101
#pragma warning(disable : 4101)
	catch (std::exception& e)
	{

	}

	return true;
}

unsigned long PlaybackManager::GetTotalFrameCount()
{
	return this->m_nTotalFrameCount;
}

void PlaybackManager::InitPlayback(bool bReload = true)
{
	// Init playback is resetting this shit
	this->m_BreakState.m_nLineNo = -1;

	this->m_RuntoLineNo = -1;
	this->m_WalktoLineNo = -1;
	this->m_nTotalFrameCount = 0;

	if (this->m_bPlayingBack && bReload)
	{
		this->m_bPlayingBack = false;
		this->m_bPlaybackReady = false;
		memset(&m_szCurrentManagerState[0], 0, 2000);

		return;
	}


	bool result = this->ReadInputFile();

	if (!result)
	{
		DebugOutput("failed to read input file");
		return;
	}


	this->m_bPlayingBack = true;

	this->m_CurrentFrame = 0;
	this->m_InputIndex = 0;

	if (this->m_Inputs.size() > 0)
	{
		this->m_pCurrentInput = this->m_Inputs[0];
		this->m_FrameToNext = m_pCurrentInput->m_Frames;
	}
	else
	{
		this->m_FrameToNext = 1;
		// Disable playback
		this->m_bPlaybackReady = false;
		this->m_bPlayingBack = false;
		return;
	}

	this->m_bPlaybackReady = true;
}

unsigned long PlaybackManager::ReloadPlayback()
{
	// Save it
	unsigned long dwPlayedBackFrames = this->m_CurrentFrame;
	this->InitPlayback(false);

	// Restore it
	this->m_CurrentFrame = dwPlayedBackFrames;

	// Step on the index until we get  back to where we were.
	while (this->m_CurrentFrame > this->m_FrameToNext)
	{
		if (this->m_InputIndex + 1 >= this->m_Inputs.size())
		{
			this->m_InputIndex++;
			return this->m_Inputs.size();
		}

		this->m_pCurrentInput = this->m_Inputs[++this->m_InputIndex];
		this->m_FrameToNext += this->m_pCurrentInput->m_Frames;
	}

	return this->m_Inputs.size();
}

bool PlaybackManager::IsPlayingBack()
{
	return this->m_bPlayingBack;
}

unsigned long PlaybackManager::GetCurrentInputIndex()
{
	return this->m_InputIndex;
}

void PlaybackManager::DoPlayback(bool wasFramestepped)
{
	if (!this->m_bPlayingBack)
		return;


	InputRecord * prev = nullptr;
	InputRecord * next = nullptr;

	if (this->m_InputIndex < this->m_Inputs.size())
	{
		if (wasFramestepped)
		{
			unsigned long OldInputDoneCount = m_pCurrentInput->m_Done;
			unsigned long ReloadedCount = this->ReloadPlayback();
			m_pCurrentInput->m_Done += OldInputDoneCount;
		}

		if (this->m_CurrentFrame >= this->m_FrameToNext)
		{
			if (this->m_InputIndex + 1 >= this->m_Inputs.size())
			{
				if (wasFramestepped)
				{
					unsigned long ReloadedCountScope2 = this->ReloadPlayback();
					if (this->m_InputIndex + 1 >= ReloadedCountScope2)
					{
						this->m_InputIndex++;

						// disable playback
						this->m_bPlaybackReady = false;
						this->m_bPlayingBack = false;
						memset(&m_szCurrentManagerState[0], 0, 2000);

						return;
					}
				}
				else
				{
					if (this->m_InputIndex + 1 >= this->m_Inputs.size())
					{
						this->m_InputIndex++;

						// Disable playback
						this->m_bPlaybackReady = false;
						this->m_bPlayingBack = false;
						memset(&m_szCurrentManagerState[0], 0, 2000);

						return;
					}
				}

			} 

			this->m_pCurrentInput = this->m_Inputs[++this->m_InputIndex];

			if (this->m_InputIndex > 0)
				prev = this->m_Inputs[this->m_InputIndex - 1];

			// Seed set
			if (m_pCurrentInput->m_nSeed != -1)
			{
				g_dwConstantSeed = m_pCurrentInput->m_nSeed;
				g_mt1997.seed(m_pCurrentInput->m_nSeed);
				g_pUnifDist->reset();
				this->m_nLastReadSeed = m_pCurrentInput->m_nSeed;
			}

			if (this->m_BreakState.m_nLineNo != -1)
			{
				if (this->m_BreakState.m_BreakType == eBreakType::BREAKTYPE_FAST)
				{
					// it's a multi level file, and we're *IN* the file
					if (this->m_pCurrentInput->m_bMultiLevelFile &&
						!strcmpi(this->m_pCurrentInput->m_szFromFile, this->m_BreakState.m_szCurrentFile))
					{
						// Right now we haven't reached the internal line number, just fast forward, unless it's a slow.
						if (this->m_pCurrentInput->m_nInternalLineNo < this->m_BreakState.m_nLineNo)
						{
							// Is slow, don't speed up.
							if (this->m_pCurrentInput->IsSlow()) { g_GameSpeedPlayerStep = 1; SetDrawEnabled(true); }

							// Isn't slow, speed up.
							else { g_GameSpeedPlayerStep = 200; SetDrawEnabled(false); }
						}
						else
						{
							// we've met the breakpoint condition, stop!
							this->m_BreakState.m_nLineNo = -1;
							this->m_RuntoLineNo = -1;
							g_bPaused = true;
							SetDrawEnabled(true);
							g_GameSpeedPlayerStep = 1;
						}
					}
					else
					{
						// It's not a multi level file but we have a break point set.
						// Handle this!
					}
				}
				// walkto
				else
				{
					// it's a multi level file, and we're *IN* the file
					if (this->m_pCurrentInput->m_bMultiLevelFile &&
						!strcmpi(this->m_pCurrentInput->m_szFromFile, this->m_BreakState.m_szCurrentFile))
					{
						// Right now we haven't reached the internal line number, just keep the normal speed.
						if (this->m_pCurrentInput->m_nInternalLineNo < this->m_BreakState.m_nLineNo)
						{
							g_GameSpeedPlayerStep = 1;
						}
						else
						{
							// we've met the breakpoint condition, stop!
							this->m_WalktoLineNo = -1;
							this->m_BreakState.m_nLineNo = -1;
							g_bPaused = true;
							SetDrawEnabled(true);
							g_GameSpeedPlayerStep = 1;
						}
					}
				}
			}

			this->m_FrameToNext += this->m_pCurrentInput->m_Frames;
		} // frame to next scope end
		else
		{
			this->m_pCurrentInput->m_Done++;
		}

		//  Increase current frame.
		this->m_CurrentFrame++;


		if ((this->m_InputIndex + 1) < this->m_Inputs.size())
			next = this->m_Inputs[this->m_InputIndex + 1];

		if (m_pCurrentInput->HasPos)
		{
			auto pPlayer = GetPlayerRoomObjectInstance();
			if (pPlayer)
			{
				pPlayer->m_fX = m_pCurrentInput->xPos;
				pPlayer->m_fY = m_pCurrentInput->yPos;
			}
		}


		if (m_pCurrentInput->fixDashTimer)
		{
			GMLRoomObjectInstance* pGlobalObject = GetGlobalObjectInstance();
			unsigned int cagrace_index = GetGlobalVariableIndexByName("playerchargealarmGrace", ATO_VARIABLE_NAMESLIST_ADDR);
			unsigned int sba_index = GetGlobalVariableIndexByName("playerspeedboostalarm", ATO_VARIABLE_NAMESLIST_ADDR);
			VariablePointer* pGrace = get_variable_by_index(cagrace_index, pGlobalObject->m_pInstProps);
			VariablePointer* pSba = get_variable_by_index(sba_index, pGlobalObject->m_pInstProps);

			/*
			double * pGrace = GetGlobalVariableAsDoublePtrWithOffsets(caGrace_Offsets, 2);
			double * pSba = GetGlobalVariableAsDoublePtrWithOffsets(sba_Offsets, 2);
			*pGrace = 5.0;
			*pSba = 100.0;*/

			*pGrace->m_Value = 5.0;
			*pSba->m_Value = 100.0;
		}


		if (m_pCurrentInput->IsDeleteSave())
		{

			std::string saveFile = getenv("APPDATA");
			saveFile += "\\..\\Local\\Ato\\";
			saveFile += "savedataSAVE" + std::to_string(m_pCurrentInput->saveToDelete) + ".dat";

			remove(saveFile.c_str());
		}

		// Copy can't be any of the delete or clear actions
		if (m_pCurrentInput->IsCopySave() && !m_pCurrentInput->IsDeleteSave() && !m_pCurrentInput->IsClearSave())
		{
			const char* env = getenv("APPDATA");
			std::string fromFile = env;
			fromFile += "\\..\\Local\\Ato\\savedataSAVE";
			fromFile += std::to_string(m_pCurrentInput->saveCopyFrom) + ".dat";

			std::string toFile = env;
			toFile += "\\..\\Local\\Ato\\";
			toFile += "savedataSAVE" + std::to_string(m_pCurrentInput->saveCopyTo) + ".dat";
			remove(toFile.c_str());

			CopyFileA(fromFile.c_str(), toFile.c_str(), TRUE);
		}

		if (m_pCurrentInput->IsRoomRestart())
			room_restart();

		if (m_pCurrentInput->IsRoomGoto())
		{
			if (m_pCurrentInput->roomGoto != -1)
				room_goto(m_pCurrentInput->roomGoto);
		}

		if (m_pCurrentInput->IsSetAbility())
		{
			yygsForManager.Set("PlayerAbility");
			variable_global_get(pOutArray, 0x00, 0x01, 0x02, &yygsForManager);
			pOutArray->CastForRawAccess()[m_pCurrentInput->abilityIndex] = m_pCurrentInput->abilityValue;
		}


		if (m_pCurrentInput->IsLeft())
		{
			HoldKey('A', 0x01);
		}

		if (m_pCurrentInput->IsRight())
		{
			HoldKey('D', 0x01);
		}

		if (m_pCurrentInput->IsUp())
		{
			HoldKey('W', 0x01);
		}

		if (m_pCurrentInput->IsDown())
		{
			HoldKey('S', 0x01);
		}

		if (m_pCurrentInput->IsUpPress())
			PressKey('W', 0x01);
		if (m_pCurrentInput->IsDownPress())
			PressKey('S', 0x01);
		if (m_pCurrentInput->IsLeftPress())
			PressKey('A', 0x01);
		if (m_pCurrentInput->IsRightPress())
			PressKey('D', 0x01);

		if (m_pCurrentInput->IsDodge())
		{
			PressKey('Z', 0x01);
		}

		if (m_pCurrentInput->IsMenuProceed())
		{
			PressKey(VK_RETURN, 0x01);
		}

		if (m_pCurrentInput->IsMap())
		{
			PressKey(VK_TAB, 0x01);
		}

		if (m_pCurrentInput->IsUnMap())
		{
			PressKey(VK_TAB, 0x01);
			ReleaseKey(VK_TAB, 0x01);
		}

		if (m_pCurrentInput->IsUnJump())
		{
			PressKey(0x20, 0x01);
			//HoldKey(0x20, 0x00);
			ReleaseKey(0x20, 0x01);
		}

		if (m_pCurrentInput->IsUnAttack())
		{
			PressKey('X', 0x01);
			ReleaseKey('X', 0x01);
		}

		if (m_pCurrentInput->IsUnMenu())
		{
			PressKey(VK_RETURN, 0x01);
			ReleaseKey(VK_RETURN, 0x01);
		}
		
		if (m_pCurrentInput->IsAttackPrimary())
		{
			// check previous input, see if it also was attack

			if(this->m_pCurrentInput->m_Done == 0)
			{
				if (prev != nullptr)
				{
					// Previous wasn't attack primary, we can press it again.
					if (!prev->IsAttackPrimary())
					{
						PressKey('X', 0x01);
					}

				}
				else
				{
					PressKey('X', 0x01);
				}
	
			}

			if (m_pCurrentInput->m_Done < this->m_pCurrentInput->m_Frames)
			{
				HoldKey('X', 0x01);
			}

			else if (next != nullptr)
			{
				if (next->IsAttackPrimary())
					HoldKey('X', 0x01);
			}
		}

		// Handle holding \ pressing of jump
		if (m_pCurrentInput->IsJump())
		{

			if (m_pCurrentInput->m_Done == 0 || (this->m_InputIndex == 0 && m_pCurrentInput->m_Done == 1))
			{
				if (prev != nullptr)
				{
					// Previous wasn't jump, we can press it again.
					if (!prev->IsJump())
						PressKey(0x20, 0x01);
				}
				else
				{
					PressKey(0x20, 0x01);
				}

			}

			if (m_pCurrentInput->m_Done < this->m_pCurrentInput->m_Frames || (this->m_InputIndex == 0 && (m_pCurrentInput->m_Done <= this->m_pCurrentInput->m_Frames)))
			{
				ReleaseKey(0x20, 0x00);
			}
			else if (next != nullptr)
			{
				if (next->IsJump())
					ReleaseKey(0x20, 0x00);
			}


		}

		// Handle release of jump
		if (!m_pCurrentInput->IsJump() && prev != nullptr)
		{

			if (prev->IsJump())
				ReleaseKey(0x20, 0x01);
		}

		// Handle release of attack
		if (!m_pCurrentInput->IsAttackPrimary() && prev != nullptr)
		{
			if (prev->IsAttackPrimary())
				ReleaseKey('X', 0x01);
		}
	}
	return;
}

void PlaybackManager::FormatManagerString()
{
	memset(&this->m_szCurrentManagerState[0], 0, 2000);
	std::string playerInfo = "";

	auto pPlayerInst = GetPlayerRoomObjectInstance();

	VariablePointer* on_ground = NULL;
	VariablePointer* xspeed = NULL;
	VariablePointer* yspeed = NULL;

	if (onground_index == 0) onground_index = GetInstanceVariableIDFromName("on_ground");
	if (xspeed_index == 0) xspeed_index = GetInstanceVariableIDFromName("XSPEED"); 
	if (yspeed_index == 0) yspeed_index = GetInstanceVariableIDFromName("YSPEED");

	if (pPlayerInst) {
		on_ground = get_variable_by_index(onground_index, pPlayerInst->m_pInstProps);
		xspeed = get_variable_by_index(xspeed_index, pPlayerInst->m_pInstProps);
		yspeed = get_variable_by_index(yspeed_index, pPlayerInst->m_pInstProps);

		playerInfo += "\nPosition: " + std::to_string(pPlayerInst->m_fX) + ", " + std::to_string(pPlayerInst->m_fY);
		playerInfo += "\nSpeed: " + std::to_string(*xspeed->m_Value) + ", " + std::to_string(*yspeed->m_Value);//std::to_string(GetGlobalVariableAsDouble(0x54C)) + ", " + std::to_string(GetGlobalVariableAsDouble(0x198));
		playerInfo += "\nGrounded: " + std::to_string(*on_ground->m_Value);
	}

	playerInfo += "\nRoomNo: " + std::to_string(*(unsigned long*)(roomindex_addr));

	sprintf(this->m_szCurrentManagerState, "[%s]-Ln: %u (%u / %u) - [%s]\n(Cur: %u / Total: %u)%s\nInputIndex:%u",
		this->m_pCurrentInput->m_szFromFile,
		(m_pCurrentInput->m_bMultiLevelFile) ? this->m_pCurrentInput->m_nInternalLineNo : this->m_pCurrentInput->m_LineNo,
		this->m_pCurrentInput->m_Done, this->m_pCurrentInput->m_Frames,
		this->m_pCurrentInput->ToString().c_str(), this->m_CurrentFrame, this->m_nTotalFrameCount, playerInfo.c_str(),
		this->m_InputIndex);

}

unsigned long PlaybackManager::GetLastReadSeed()
{
	return this->m_nLastReadSeed;
}

InputRecord * PlaybackManager::GetCurrentInput()
{
	return this->m_pCurrentInput;
}

