#pragma once

#include <string>
#include <algorithm>
#include <sstream>
#include <vector>
#include <limits>

enum EInputState
{
	DEFAULT_NONE    = 0x0,
	LEFT            = 1 << 0,
	RIGHT           = 1 << 1,
	UP              = 1 << 2,
	DOWN            = 1 << 3,
	JUMP            = 1 << 4,
	DODGE           = 1 << 5,
	ATTACK_PRIMARY  = 1 << 6,
	ATTACK_MAGIC    = 1 << 7,
	MENU_PROCEED    = 1 << 8,
	MAP             = 1 << 9,
	LEFTP           = 1 << 10,
	RIGHTP          = 1 << 11,
	UPP             = 1 << 12,
	DOWNP           = 1 << 13,
	UNATTACK        = 1 << 14,
	UNMENU          = 1 << 15,
	UNMAP           = 1 << 16,
	SLOWDOWN        = 1 << 20,
	CLEARSAVE       = 1 << 21,
	COPYSAVE        = 1 << 22,
	DELETESAVE      = 1 << 23,
	ROOM_RESTART    = 1 << 24,
	ROOM_GOTO       = 1 << 25,
	SETABILITY      = 1 << 26,
	UNJUMP          = 1 << 27,
	DRNG_FIX        = 1 << 28
};

struct InputRecord
{
	EInputState m_InputState;

	int m_Frames;

	int m_TotalFrames;

	int m_Done;

	unsigned int m_LineNo;

	unsigned int m_nInternalLineNo;

	unsigned long m_nSeed;

	bool m_bMultiLevelFile;

	char m_szFromFile[56];

	bool HasPos;

	bool HasSpeedX;
	bool HasSpeedY;

	float xPos;
	float yPos;
	double xSpeed;
	double ySpeed;

	unsigned long saveToDelete;
	unsigned long saveCopyFrom;
	unsigned long saveCopyTo;

	unsigned long roomGoto;
	unsigned long abilityIndex;
	double abilityValue;
	bool DRNGF;

	bool fixDashTimer;

	std::string ToString()
	{
		std::string result = "";


		if (this->IsLeft())
			result += ",Left";

		if (this->IsLeftPress())
			result += ",LeftP";

		if (this->IsRight())
			result += ",Right";

		if (this->IsRightPress())
			result += ",RightP";

		if (this->IsUp())
			result += ",Up";

		if (this->IsUpPress())
			result += "UpP";

		if (this->IsDown())
			result += ",Down";

		if (this->IsDownPress())
			result += ",DownP";

		if (this->IsJump())
			result += ",Jump";

		if (this->IsDodge())
			result += ",Dodge";

		if (this->IsAttackPrimary())
			result += ",AttackPrimary";

		if (this->IsAttackMagic())
			result += ",AttackMagic";
		
		if (this->IsMenuProceed())
			result += ",Proceed";

		if (this->IsMap())
			result += ",Map";

		return result;
	}

	bool HasFlag(EInputState state, EInputState which)
	{
		return (state & which) == which;
	}

	bool IsLeft()
	{
		return this->HasFlag(this->m_InputState, EInputState::LEFT);
	}

	bool IsLeftPress()
	{
		return this->HasFlag(this->m_InputState, EInputState::LEFTP);
	}

	bool IsRight()
	{
		return this->HasFlag(this->m_InputState, EInputState::RIGHT);
	}

	bool IsRightPress()
	{
		return this->HasFlag(this->m_InputState, EInputState::RIGHTP);
	}

	bool IsUp()
	{
		return this->HasFlag(this->m_InputState, EInputState::UP);
	}

	bool IsUpPress()
	{
		return this->HasFlag(this->m_InputState, EInputState::UPP);
	}

	bool IsDown()
	{
		return this->HasFlag(this->m_InputState, EInputState::DOWN);
	}

	bool IsDownPress()
	{
		return this->HasFlag(this->m_InputState, EInputState::DOWNP);
	}

	bool IsJump()
	{
		return this->HasFlag(this->m_InputState, EInputState::JUMP);
	}

	bool IsDodge()
	{
		return this->HasFlag(this->m_InputState, EInputState::DODGE);
	}

	bool IsAttackPrimary()
	{
		return this->HasFlag(this->m_InputState, EInputState::ATTACK_PRIMARY);
	}

	bool IsAttackMagic()
	{
		return this->HasFlag(this->m_InputState, EInputState::ATTACK_MAGIC);
	}
	
	bool IsMenuProceed()
	{
		return this->HasFlag(this->m_InputState, EInputState::MENU_PROCEED);
	}

	bool IsMap()
	{
		return this->HasFlag(this->m_InputState, EInputState::MAP);
	}

	bool IsSlow()
	{
		return this->HasFlag(this->m_InputState, EInputState::SLOWDOWN);
	}

	bool IsClearSave()
	{
		return this->HasFlag(this->m_InputState, EInputState::CLEARSAVE);
	}

	bool IsDeleteSave()
	{
		return this->HasFlag(this->m_InputState, EInputState::DELETESAVE);
	}

	bool IsCopySave()
	{
		return this->HasFlag(this->m_InputState, EInputState::COPYSAVE);
	}

	bool IsRoomRestart()
	{
		return this->HasFlag(this->m_InputState, EInputState::ROOM_RESTART);
	}

	bool IsRoomGoto()
	{
		return this->HasFlag(this->m_InputState, EInputState::ROOM_GOTO);
	}

	bool IsSetAbility()
	{
		return this->HasFlag(this->m_InputState, EInputState::SETABILITY);
	}

	bool IsUnJump()
	{
		return this->HasFlag(this->m_InputState, EInputState::UNJUMP);
	}

	bool IsUnAttack()
	{
		return this->HasFlag(this->m_InputState, EInputState::UNATTACK);
	}

	bool IsUnMap()
	{
		return this->HasFlag(this->m_InputState, EInputState::UNMAP);
	}
	bool IsUnMenu()
	{
		return this->HasFlag(this->m_InputState, EInputState::UNMENU);
	}

	bool IsDRNGFix()
	{
		return this->HasFlag(this->m_InputState, EInputState::DRNG_FIX);
	}

	InputRecord(unsigned long frames, EInputState state)
	{
		this->fixDashTimer = false;
		this->DRNGF = false;
		this->abilityIndex = -1;
		this->abilityValue = -1.0;
		this->roomGoto = -1;
		this->saveCopyFrom = -1;
		this->saveCopyTo = -1;
		this->saveToDelete = -1;
		this->m_Frames = frames;
		this->m_InputState = state;
		this->m_nSeed = -1;
		this->HasPos = false;
		this->HasSpeedX = false;
		this->HasSpeedY = false;
	}

#pragma warning(disable : 4996)
	InputRecord(std::string line, unsigned int ln, const char *filename, unsigned int otherln)
	{
		this->fixDashTimer = false;
		this->DRNGF = false;
		this->abilityIndex = -1;
		this->abilityValue = -1.0;
		this->roomGoto = -1;
		this->saveCopyFrom = -1;
		this->saveCopyTo = -1;
		this->saveToDelete = -1;
		this->m_nInternalLineNo = 0;
		this->m_LineNo = ln;
		this->m_Done = 0;
		this->m_nSeed = -1;
		this->HasPos = false;
		this->HasSpeedX = false;
		this->HasSpeedY = false;
		this->xPos = 0.0f;
		this->yPos = 0.0f;
		this->xSpeed = 0.0f;
		this->ySpeed = 0.0f;

		strncpy(this->m_szFromFile, filename, sizeof(this->m_szFromFile) / sizeof(this->m_szFromFile[0]));
		this->m_bMultiLevelFile = false;
		if (strcmpi(filename, "Ato.rec"))
		{
			this->m_nInternalLineNo = 0;
			this->m_bMultiLevelFile = true;
		}

		this->m_nInternalLineNo = otherln;
		std::istringstream ss(line);
		std::string token;

		std::vector<std::string> tokens;
		auto delimited = line.find(',');

		while (std::getline(ss, token, ','))
			tokens.push_back(token);

		this->m_Frames = (delimited == std::string::npos) ? std::stoul(line) : std::stoul(tokens[0]);
		this->m_TotalFrames = this->m_Frames;

		unsigned int TempState = EInputState::DEFAULT_NONE;

		if (tokens.size() > 1 && delimited != std::string::npos)
		{
			for (unsigned int i = 1; i < tokens.size(); i++)
			{
				token = tokens[i];

				// Just continue again, we already handled cases where we needed to process integers.
				if (isdigit(tokens[i][0]))
					continue;

				auto negativelamb = [](char& ch) { ch = toupper((unsigned char)ch); };
				std::for_each(token.begin(), token.end(), negativelamb);

				if (token == "LEFT")
				{
					TempState |= EInputState::LEFT;
					continue;
				}
				else if (token == "RIGHT")
				{
					TempState |= EInputState::RIGHT;
					continue;
				}
				else if (token == "UP")
				{
					TempState |= EInputState::UP;
					continue;
				}
				else if (token == "DOWN")
				{
					TempState |= EInputState::DOWN;
					continue;
				}
				else if (token == "LEFTP")
				{
					TempState |= EInputState::LEFTP;
					continue;
				}
				else if (token == "RIGHTP")
				{
					TempState |= EInputState::RIGHTP;
					continue;
				}
				else if (token == "UPP")
				{
					TempState |= EInputState::UPP;
					continue;

				}
				else if (token == "DOWNP")
				{
					TempState |= EInputState::DOWNP;
					continue;
				}
				else if (token == "JUMP")
				{
					TempState |= EInputState::JUMP;
					continue;
				}
				else if (token == "UNJUMP")
				{
					TempState |= EInputState::UNJUMP;
					continue;
				}
				else if (token == "UNATTACK")
				{
					TempState |= EInputState::UNATTACK;
					continue;
				}
				else if (token == "UNMAP")
				{
					TempState |= EInputState::UNMAP;
					continue;
				}
				else if (token == "UNMENU")
				{
					TempState |= EInputState::UNMENU;
					continue;
				}
				else if (token == "DODGE")
				{
					TempState |= EInputState::DODGE;
					continue;
				}
				else if (token == "PATTACK" || token == "SLASH" || token == "ATTACK")
				{
					TempState |= EInputState::ATTACK_PRIMARY;
					continue;
				}
				else if (token == "MATTACK" || token == "MAGIC")
				{
					TempState |= EInputState::ATTACK_MAGIC;
					continue;
				}
				else if (token == "PROCEED" || token == "MENU")
				{
					TempState |= EInputState::MENU_PROCEED;
					continue;
				}
				else if (token == "MAP")
				{
					TempState |= EInputState::MAP;
					continue;
				}
				else if (token == "SLOW")
				{
					TempState |= EInputState::SLOWDOWN;
					continue;
				}
				else if (token == "RESTART")
				{
					TempState |= EInputState::ROOM_RESTART;
					continue;
				}
				else if (token == "DRNGFIX")
				{
					TempState |= EInputState::DRNG_FIX;
					this->DRNGF = true;
					continue;
				}
				else if (token == "FIXDASH")
				{
					this->fixDashTimer = true;
					continue;
				}
				else if (token == "GOTO")
				{
					TempState |= EInputState::ROOM_GOTO;
					this->roomGoto = std::stoul(tokens[i + 1].c_str(), nullptr);
					continue;
				}
				else if (token == "SETABILITY")
				{
					TempState |= EInputState::SETABILITY;
					// need to grab index and val
					this->abilityIndex = std::stoul(tokens[i + 1].c_str(), nullptr);
					this->abilityValue = std::stod(tokens[i + 2].c_str(), nullptr);

					continue;
				}
				else if (token == "CLEARSAVE")
				{
					TempState |= EInputState::CLEARSAVE;
					continue;
				}
				else if (token == "DELETESAVE")
				{
					TempState |= EInputState::DELETESAVE;
					this->saveToDelete = std::stoul(tokens[i + 1].c_str(), nullptr);
					continue;
				}
				else if (token == "COPYSAVE")
				{
					TempState |= EInputState::COPYSAVE;
					this->saveCopyFrom = std::stoul(tokens[i + 1].c_str(), nullptr);
					this->saveCopyTo = std::stoul(tokens[i + 2].c_str(), nullptr);
					continue;
				}
				else if (token == "SEED")
				{
					// tokens[i + 1]
					this->m_nSeed = std::stoul(tokens[i + 1].c_str(), nullptr);
					continue;
				}
				else if (token == "POS")
				{
					this->HasPos = true;
					this->xPos = std::strtof(tokens[i + 1].c_str(), nullptr);
					this->yPos = std::strtof(tokens[i + 2].c_str(), nullptr);
					continue;
				}
				else if (token == "SPEEDY")
				{
					this->HasSpeedY = true;
					this->ySpeed = std::strtof(tokens[i + 1].c_str(), nullptr);
					continue;
				}
				else if (token == "SPEEDX")
				{
					this->HasSpeedX = true;
					this->xSpeed = std::strtof(tokens[i + 1].c_str(), nullptr);
					continue;
				}
			
			}
		}

		this->m_InputState = (EInputState)(TempState);
	}

	InputRecord() {}
};