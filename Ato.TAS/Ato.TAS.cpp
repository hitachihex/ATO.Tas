// Ato.TAS.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <string>
#include "DebugTools.h"
#include "Addresses.h"
#include "Hooks.h"
#include "Ato.TAS.h"
#include "PlaybackManager.h"
#include <time.h>
#include <psapi.h>

// sprintf, etc
#pragma warning(disable : 4996)

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "Shlwapi.lib")

extern "C" void __declspec(dllexport) __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO* inRemoteInfo);


HOOK_TRACE_INFO EventLoopHookHandle = { NULL };
HOOK_TRACE_INFO YoyoUpdateHookHandle = { NULL };
HOOK_TRACE_INFO YoyoOldRandomHookHandle = { NULL };

bool g_bDrawRNGFix = false;

std::tr1::ranlux64_base_01  g_rEng;
std::tr1::normal_distribution<double> * g_pfloatDist = nullptr;

std::normal_distribution<double> * g_pUnifDist = nullptr;
std::mt19937 g_mt1997;

unsigned long g_dwConstantSeed = 0x10;

unsigned long g_ACLEntries[1] = { NULL };
DrawText_Internal draw_text = (DrawText_Internal)(ATO_DRAWTEXT_ADDRESS);
DrawText_Internal2 draw_text2 = (DrawText_Internal2)(ATO_DRAWTEXTREAL_ADDRESS);
DrawText_Internal3 draw_text3 = (DrawText_Internal3)(ATO_DRAWTEXT2_ADDRESS);
oYoyoOldRandom original_YoyoOldRandom = (oYoyoOldRandom)(0x0);
fnroom_restart room_restart = (fnroom_restart)(0x0);
fnroom_goto room_goto = (fnroom_goto)(0x0); 
fnVariable_Global_Get variable_global_get = (fnVariable_Global_Get)(0x00544B40);
fnVariable_Instance_Get variable_instance_get = (fnVariable_Instance_Get)(ATO_VARINSTANCEGET_ADDRESS);
GameMaker_TextStructure * g_pText = new GameMaker_TextStructure();
oMurmurHash32_fn murmurhash_32 = (oMurmurHash32_fn)(0x0);
unsigned long g_dwOldProt = 0;

unsigned int caGrace_Offsets[2] = { 0x660, 0x6E0 };
unsigned int sba_Offsets[2] = { 0x660, 0x600 };

void __stdcall NativeInjectionEntryPoint(REMOTE_ENTRY_INFO* inRemoteInfo)
{
	void* base = GetModuleHandleA("Ato.exe");
	atobase_addr = (unsigned long)base;
	
	MODULEINFO modinfo = { 0 };
	GetModuleInformation(GetCurrentProcess(), (HMODULE)base, &modinfo, sizeof(modinfo));
	auto addr = FindPattern(base, modinfo.SizeOfImage, "\xA1\x00\x00\x00\x00\x85\xC0\x0F\x84\x00\x00\x00\x00\x50\xE8\x00\x00\x00\x00\x83\xC4\x04\x85\xC0\x0F\x85\x00\x00\x00\x00\xFF\x35\x00\x00\x00\x00\x6A\x01\x6A\x04\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xFF\x35\x00\x00\x00\x00\x6A\x01\x68\x04\x10\x00\x00", "x????xxxx????xx????xxxxxxx????xx????xxxxx????x????xx????xxxxx??");

	if (addr) {
		DebugOutput("Addr is %08X", addr);

		unsigned long temp = (unsigned long)(addr);

		// +0x81, start of push keyholdptr_addr
        // +0x98, start of push keyreleaseptr_addr
        // +0xAF, start of push keypressptr_addr
		unsigned long kh_offs = (temp + 0x82);
		unsigned long kr_offs = (temp + 0x99);
		unsigned long kp_offs = (temp + 0xB0);

		keyholdptr_addr = *(unsigned long*)(kh_offs);
		keyreleasedptr_addr = *(unsigned long*)(kr_offs);
		keypressedptr_addr = *(unsigned long*)(kp_offs);
	}

	addr = FindPattern(base, modinfo.SizeOfImage, "\x8B\x44\x24\x04\x8B\x54\x24\x0C\x56\x57\x8B\x7C\x24\x10\x8B\xCF\xC1\xE9\x02\x8D\x34\x88", "xxxxxxxxxxxxxxxxxxxxxx");

	if (addr) {
		DebugOutput("MurmurHash32 addr is %08X", addr);
		murmurhash32_addr = (unsigned long)(addr);
	}

	addr = FindPattern(base, modinfo.SizeOfImage, "\xA1\x00\x00\x00\x000\x8B\x04\xB0\xEB\x00\x33\xC0\x85\xC0\xBD\x00\x00\x00\x00", "x????xxxx?xxxxx????");

	if (addr) {
		
		unsigned long temp = (unsigned long)(addr);
		unsigned long vn_offs = (temp + 0x1);

		DebugOutput("VarNamesList addr is %08X", *(unsigned long*)(vn_offs));
		varnames_list_addr = *(unsigned long*)(vn_offs);
	}

	addr = FindPattern(base, modinfo.SizeOfImage, "\x83\xEC\x0C\x00\x00\x00\x00\x00\x00\x00\x53\x57\x0F\x84\x00\x00\x00\x00\x56\x8B\x74\x24\x24\x00\x00\x00\x00\x00\x00\x00\x8B\x46\x08\x85\xC0", "xxx???????xxxx????xxxxx???????xxxxx");

	if (addr) {
		DebugOutput("EventLoop addr is %08X", addr);
		eventloop_addr = (unsigned long)(addr);
	}

	addr = FindPattern(base, modinfo.SizeOfImage, "\x83\xEC\x08\x56\xE8\x00\x00\x00\x00\x8B\xF0\x8B\xCA\x2B\x35\x00\x00\x00\x00", "xxxxx????xxxxxx????");

	if (addr) {
		DebugOutput("YoyoUpdate addr is %08X", addr);
		yoyoupdate_addr = (unsigned long)(addr);

		unsigned long de_offs = (yoyoupdate_addr + 0x306);
		drawenabled_addr = *(unsigned long*)(de_offs);

		DebugOutput("DrawEnabled addr is %08X", drawenabled_addr);
	}

	addr = FindPattern(base, modinfo.SizeOfImage, "\x83\xC4\x08\xD9\x5E\x44\x3B\x3D\x00\x00\x00\x00", "xxxxxxxx????");

	if (addr) {
		
		unsigned long temp = (unsigned long)(addr);
		unsigned long crp_offs = (temp + 0x08);

		DebugOutput("CurrentRoomPointer address is %08X", *(unsigned long*)(crp_offs));
		currentroomptr_addr = *(unsigned long*)(crp_offs);
	}

	addr = FindPattern(base, modinfo.SizeOfImage, "\x51\x56\x8B\x35\x00\x00\x00\x00\x57\x8B\x0C\xB5\x00\x00\x00\x00\x8D\x46\xFD", "xxxx????xxxx????xxx");

	if (addr) {
		DebugOutput("YoyoOldRandom addr is %08X", addr);

		yoyooldrandom_addr = (unsigned long)(addr);
	}

	addr = FindPattern(base, modinfo.SizeOfImage, "\x6A\x00\x6A\x02\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x6A\x00\x6A\x02\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x6A\x00\x6A\x02\x68\x00\x00\x00\x00\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x6A\x00\x6A\x03", "x?xxx????x????x????x?xxx????x????x????x?xxx????x????x????x?xx");

	if (addr) {
		unsigned long temp = (unsigned long)(addr);
		unsigned long rr_offs = (temp + 0x69F);

		roomrestart_addr = *(unsigned long*)(rr_offs);

		DebugOutput("RoomRestartAddr is %08X", roomrestart_addr);
	}

	addr = FindPattern(base, modinfo.SizeOfImage, "\x8B\x44\x24\x04\x50\xA3\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x83\xC4\x04\x84\xC0\x75\x00\x6A\x01\xFF\x35\x00\x00\x00\x00\x68\x00\x00\x00\x00", "xxxxxx??????????????x????xxxxxx?xxxx????x????");

	if (addr) {
		DebugOutput("RoomGoto addr is %08X", addr);
		roomgoto_addr = (unsigned long)(addr);
	}

	addr = FindPattern(base, modinfo.SizeOfImage, "\xFF\x35\x00\x00\x00\x00\xE8\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x83\xC4\x04\x83\x79\x30\x00", "xx????x????xx????xxxxxxx");

	if (addr) {
		unsigned long temp = (unsigned long)(addr);
		unsigned long go_offs = (temp + 0x2);

		DebugOutput("GlobalObject addr is %08X", *(unsigned long*)(go_offs));
		globalobject_addr = *(unsigned long*)(go_offs);
	}


	addr = FindPattern(base, modinfo.SizeOfImage, "\xC7\x46\x10\x00\x00\x00\x00\xC7\x06\x80\x00\x00\x00\xE8\x00\x00\x00\x00\x89\x35\x00\x00\x00\x00\x5E\xC3", "xxxxxxxxxxxxxx????xx????xx");

	if (addr) {
		unsigned long temp = (unsigned long)(addr);
		unsigned long inl_offs = (temp + 0x14);

		DebugOutput("InstanceNamesList addr is %08X", *(unsigned long*)(inl_offs));
		instancenameslist_addr = *(unsigned long*)(inl_offs);
	}

	addr = FindPattern(base, modinfo.SizeOfImage, "\xE8\x00\x00\x00\x00\x8B\x0D\x00\x00\x00\x00\x3B\xC8\x75\x00\x6A\x01\x68\x00\x00\x00\x00\xE8\x00\x00\x00\x00\xC7\x05\x00\x00\x00\x00\x70\xFE\xFF\xFF", "x????xx????xxx?xxx????x????xx????xxxx");

	if (addr) {
		unsigned long temp = (unsigned long)(addr);
		unsigned long ria_offs = (temp + 0x07);

		DebugOutput("RoomIndex addr is %08X", *(unsigned long*)(ria_offs));
		roomindex_addr = *(unsigned long*)(ria_offs);
	}

	// Assume 1.01 if we didn't find them.
	if(!murmurhash32_addr) murmurhash32_addr = (atobase_addr+ATO_MURMURHASH32_RVA);
	if(!varnames_list_addr) varnames_list_addr = (atobase_addr + ATO_VARIABLE_NAMESLIST_RVA);
	if(!eventloop_addr) eventloop_addr = (atobase_addr + ATO_EVENTLOOP_RVA);
	if(!yoyoupdate_addr) yoyoupdate_addr = (atobase_addr + ATO_YOYOUPDATE_RVA);
	if(!keyholdptr_addr) keyholdptr_addr = (atobase_addr + ATO_KEYHOLDPTR_RVA);
	if(!keyreleasedptr_addr) keyreleasedptr_addr = (atobase_addr + ATO_KEYRELEASEDPTR_RVA);
	if(!keypressedptr_addr) keypressedptr_addr = (atobase_addr + ATO_KEYPRESSEDPTR_RVA);
	if(!currentroomptr_addr)currentroomptr_addr = (atobase_addr + ATO_CURRENTROOMPTR_RVA);
	if(!drawenabled_addr) drawenabled_addr = (atobase_addr + ATO_DRAWENABLED_RVA);
	if(!yoyooldrandom_addr) yoyooldrandom_addr = (atobase_addr + ATO_YOYOOLDRANDOM_RVA);
	if(!roomrestart_addr) roomrestart_addr = (atobase_addr + ATO_ROOMRESTART_RVA);
	if(!roomgoto_addr) roomgoto_addr = (atobase_addr + ATO_ROOMGOTO_RVA);
	if(!globalobject_addr) globalobject_addr = (atobase_addr + ATO_GLOBAL_OBJECT_RVA);
	if(!instancenameslist_addr) instancenameslist_addr = (atobase_addr + ATO_INSTANCENAMESLIST_RVA);
	if(!roomindex_addr) roomindex_addr = (atobase_addr + ATO_ROOMINDEX_RVA);

	*(unsigned long*)(&original_EventLoop) = eventloop_addr;
	*(unsigned long*)(&original_YoyoUpdate) = yoyoupdate_addr;
	*(unsigned long*)(&original_YoyoOldRandom) = yoyooldrandom_addr;
	*(unsigned long*)(&murmurhash_32) = murmurhash32_addr;
	*(unsigned long*)(&room_restart) = roomrestart_addr;
	*(unsigned long*)(&room_goto) = roomgoto_addr;

	NTSTATUS result = AddHook((void*)eventloop_addr, EventLoop_Hook, NULL, &EventLoopHookHandle);

	if (FAILED(result))
	{
		std::wstring err(RtlGetLastErrorString());
		DebugOutputW(err.c_str());
	}
	else
	{
		DebugOutput("EventLoop hook installed.");
		//SetupEventCallbacks();
	}

	ExclusiveHook(&EventLoopHookHandle);

	result = AddHook((void*)yoyooldrandom_addr, YoYoOldRandom_Hook, NULL, &YoyoOldRandomHookHandle);

	if (FAILED(result))
	{
		std::wstring err(RtlGetLastErrorString());
		DebugOutputW(err.c_str());
	}
	else
	{
		DebugOutput("YoyoOldRandom hook installed.");
	}

	ExclusiveHook(&YoyoOldRandomHookHandle);

	result = AddHook((void*)yoyoupdate_addr, YoYoUpdate_Hook, NULL, &YoyoUpdateHookHandle);

	if (FAILED(result))
	{
		std::wstring err(RtlGetLastErrorString());
		DebugOutputW(err.c_str());
	}
	else
	{
		DebugOutput("YoyoUpdate hook installed.");
	}

	ExclusiveHook(&YoyoUpdateHookHandle);

	g_pPlaybackMgr = new PlaybackManager("Ato.rec");

	// random for GFX
	g_rEng.seed((unsigned int)time(NULL));
	g_pfloatDist = new  std::tr1::normal_distribution<double>(LONG_MIN, LONG_MAX);
	g_pfloatDist->reset();

	// For seeded random of enemies\bosses\ other crap
	g_mt1997.seed(1337);
	g_pUnifDist = new std::normal_distribution<double>(LONG_MIN, LONG_MAX);
	g_pUnifDist->reset();

	DumpPointersForExternalOSD();
	ReadConfig();
	
}

GMLRoom * GetCurrentRoomPointer()
{
	return (GMLRoom*)(*(unsigned long*)currentroomptr_addr);
}


GMLRoomObjectInstance* GetGlobalObjectInstance()
{
	return (GMLRoomObjectInstance*)(*(unsigned long*)globalobject_addr);
}

GMLRoomObjectInstance * GetPlayerRoomObjectInstance()
{
	GMLRoom * pRoom = GetCurrentRoomPointer();
	unsigned int i = 0;
	if (!pRoom)
		return nullptr;

	if (!pRoom->m_pYoyoList)
	{
		if (!pRoom->m_ObjListCount)
			return nullptr;
	}
	else
	{
		GMLRoomObjectInstance * pObjectInstance = nullptr;
		for (pObjectInstance = pRoom->m_pYoyoList->m_pObjectLList; pObjectInstance != nullptr; pObjectInstance = pObjectInstance->m_pNext)
		{
			if (!stricmp(pObjectInstance->m_pBindData->m_pszObjectName, "oPlayer"))
				return (pObjectInstance);
		}
	}

	return nullptr;
}

void DumpCurrentRoomInstanceIds()
{
	GMLRoom * pRoom = GetCurrentRoomPointer();
	unsigned int i = 0;

	if (!pRoom)
		return;

	if (!pRoom->m_pYoyoList)
	{
		if (pRoom->m_ObjListCount == 0)
			return;
	}
	else
	{
		GMLRoomObjectInstance * pObjectInstance = nullptr;
		for (pObjectInstance = pRoom->m_pYoyoList->m_pObjectLList; pObjectInstance != nullptr; pObjectInstance = pObjectInstance->m_pNext)
		{
			DebugOutput("Object at index %u instance id is binded as: %s",i, pObjectInstance->m_pBindData->m_pszObjectName);
			i++;
		}
	}
}

/*
// *(ulong*)(0x8B2780 or 0x8B27F8) + 0x2C, + 0x10, +0x198 = GLobal Y Speed Pointer
		// *(ulong*)(0x8B2780 or 0x8B27F8) + 0x2C, + 0x10, +0x54C = Global X Speed Pointer*/
__declspec(noinline) double GetGlobalVariableAsDouble(unsigned int index)
{
	unsigned long table = *(unsigned long*)(globalobject_addr);
	if (!table)
		return 0.0f;

	table = *(unsigned long*)(table + 0x2C);
	table = *(unsigned long*)(table + 0x10);
	table = *(unsigned long*)(table + index);
	return *(double*)(table);
}

double * GetGlobalVariableAsDoublePtr(unsigned int index)
{
	unsigned long table = *(unsigned long*)(globalobject_addr);
	if (!table)
		return nullptr;

	table = *(unsigned long*)(table + 0x2C);
	table = *(unsigned long*)(table + 0x10);
	table = *(unsigned long*)(table + index);

	return (double*)(table);
}


double * GetGlobalVariableAsDoublePtrWithOffsets(unsigned int * offsets, unsigned int count)
{
	unsigned long table = *(unsigned long*)(globalobject_addr);
	if (!table)
		return nullptr;

	table = *(unsigned long*)(table + 0x2C);
	table = *(unsigned long*)(table + 0x10);

	unsigned int i = 0;
	for (i = 0; i < count - 1; i++)
		table = *(unsigned long*)(table + offsets[i]);

	return (double*)((unsigned long)table + offsets[i]);
}

void DumpPointersForExternalOSD()
{
	FILE *fp = nullptr;

	fp = fopen("pointer_ref.txt", "w");

	if (!fp)
		return;

	unsigned long ** ptrPaused = (unsigned long**)&g_bPaused;
	unsigned long ** ptrManagerState = (unsigned long**)&g_pPlaybackMgr->m_szCurrentManagerState[0];

	fprintf(fp, "%lx,%lx", ptrManagerState, ptrPaused);

	fclose(fp);
}

void SetDrawEnabled(bool v)
{
	bool * p = (bool*)(drawenabled_addr);
	*p = v;
}

// Shut up c4244
#pragma warning(disable : 4244)
unsigned long __cdecl YoYoOldRandom_Hook()
{
	// we don't give a flippity flap about calling the original
	if (g_bDrawRNGFix)
	{
			float f = g_pfloatDist->operator()(g_rEng);
			g_dwConstantSeed = g_dwConstantSeed + (unsigned long)f;
			original_YoyoOldRandom();
			return g_dwConstantSeed;
	}


	if (g_pPlaybackMgr)
	{
		//if (g_pPlaybackMgr->GetLastReadSeed() != 16)
		//{
			//DebugOutput("Seed != Normal, seed = %u", g_pPlaybackMgr->GetLastReadSeed());
			double d = g_pUnifDist->operator()(g_mt1997);
			g_dwConstantSeed = (unsigned long)d;
			//original_YoyoOldRandom();
			return g_dwConstantSeed;

			//auto gen = std::bind(g_pUnifDist, std::ref(g_mt1997));

		//}
		//else
			//return (g_dwConstantSeed = 16);
	}


	return  original_YoyoOldRandom();
}

VariablePointer* get_variable_by_index(unsigned long index, InstanceVariableProperties* pProps)
{

	unsigned long i_hash_constant = (index * 0x61C8864F);

	unsigned long other_hash = (0x01 - i_hash_constant);

	other_hash &= 0x7FFFFFFF;

	unsigned long result = (0x1FF) & other_hash;

	return &pProps->m_pInternal->variables[result];
}

unsigned long GetInstanceVariableIDFromName(const char* pcszVariableName)
{    
	unsigned long mhash32 = murmurhash_32(pcszVariableName, strlen(pcszVariableName), 0x0);
	unsigned long ptr = *(unsigned long*)(instancenameslist_addr);

	unsigned long dwEdx = mhash32;
	unsigned long dwEsi = *(unsigned long*)(ptr + 0x8);
	unsigned long dwEax = *(unsigned long*)(ptr + 0x10);

	dwEdx &= 0x7FFFFFFF;
	dwEsi &= dwEdx;

	unsigned long dwEcx = dwEsi * 3;
	InternedVariableData* pVariableInternedData = (InternedVariableData*)(dwEax + dwEcx * 4);

	if (pVariableInternedData->m_NameHash == dwEdx)
		return pVariableInternedData->m_ID;

	// alright, we need to do more, the hashes weren't equal

	dwEdx = *(unsigned long*)(ptr + 0x8);
	dwEax = dwEdx; // save hash
	dwEcx = *(unsigned long*)(ptr); 
	dwEax &= pVariableInternedData->m_NameHash;

	dwEcx -= dwEax;
	dwEcx += dwEsi;
	dwEcx &= dwEdx;
	// ebp is a counter to comp esi, if we never find it once ebp>esi then it doesn't exist

	dwEax = *(unsigned long*)(ptr + 0x10);
	// esi is not a counter
	dwEsi++;
	dwEsi &= dwEdx;

	dwEcx = (dwEsi * 3);

	pVariableInternedData = (InternedVariableData*)(dwEax + dwEcx * 4);


	// need to come back and add the loop that is done to compare the hashes again later. for now this is ok
	return pVariableInternedData->m_ID;


}

unsigned long GetVariableIDFromHashedName(const char* pcszVariableName, InstanceVariableProperties * pProps)
{
	unsigned long hash = murmurhash_32(pcszVariableName, strlen(pcszVariableName), 0x0);

	unsigned long sign = hash & 0x7FFFFFFF;

	for (unsigned int i = 0; i <= 0x1FF; i++)
	{

		unsigned long var_id = pProps->m_pInternal->variables[i].m_Id;
		if (pProps->m_pInternal->variables[i].m_NameHash == sign)
			return var_id;
	}

	return -1;
}

void dump_variable_names(GMLRoomObjectInstance* pObject, const char* filename) {

	FILE* fp = fopen(filename, "w");
	unsigned long var_count = pObject->m_pInstProps->m_nVariableCount;
	unsigned long var_name_list_ptr = *(unsigned long*)(varnames_list_addr);

	for (unsigned long i = 0; i <= 0x1FF; i++) {
		unsigned long var_id = pObject->m_pInstProps->m_pInternal->variables[i].m_Id;

		if (var_id == 0 || var_id > 0x19000) continue;
		unsigned long index_as_instance_id = (var_id - 0x186A0);
		if (index_as_instance_id < 0 || index_as_instance_id > 0x19000) continue;

		unsigned long dwEax = *(unsigned long*)(var_name_list_ptr + index_as_instance_id * 4);
		const char* var_name = (const char*)(dwEax);
		fprintf(fp, "VarName with id %08X and index %d is %s\n", var_id, i, var_name);
	}

	fclose(fp);
}

unsigned long GetGlobalVariableIndexByName(const char* pcszVariableName, unsigned long ptr)
{
	unsigned long list_ptr = *(unsigned long*)(ptr);

	for (unsigned int i = 0; i < 0x92B; i++) {
		unsigned long var_ptr = *(unsigned long*)(list_ptr + i * 4);
		if (!var_ptr) continue;
		const char* var_name = (const char*)(var_ptr);

		if (!stricmp(pcszVariableName, var_name))
		{
			return i;
		}
	}

	return -1;
}

void ReadConfig()
{
	FILE * fp = _fsopen("ato.tas.conf", "r", _SH_DENYNO);
	char buf[2048] = { 0 };
	if (!fp) {
		// file doesn't exist, create default first.

		fp = _fsopen("ato.tas.conf", "w", _SH_DENYNO);

		fprintf(fp, "pause=70\n");
		fprintf(fp, "unpause=69\n");
		fprintf(fp, "playback=72\n");
		fprintf(fp, "defaultspeed=6F\n");
		fprintf(fp, "increasespeed=6B\n");
		fprintf(fp, "decreasespeed=6D\n");
		fprintf(fp, "framestep=DD");
		fclose(fp);
	}
	else {
		int lc = 0;
		while (true)
		{
			if (fgets(buf, 2048, fp) == NULL)
				break;

			buf[strcspn(buf, "\n")] = 0;

			//DebugOutput("Read line from conf: %s", buf);

			std::string s = buf;

			size_t pause_offs = s.find("pause=");
			size_t unpause_offs = s.find("unpause=");
			size_t playback_offs = s.find("playback=");
			size_t defaultspeed_offs = s.find("defaultspeed=");
			size_t increasespeed_offs = s.find("increasespeed=");
			size_t decreasespeed_offs = s.find("decreasespeed=");
			size_t framestep_offs = s.find("framestep=");

			if (!lc) {
				if (pause_offs != std::string::npos) {
					s.erase(0, 6);
					DebugOutput("Pause hotkey=%s", s.c_str());
					g_pause_key = std::stoi(s, 0, 16);
				}
			}

			if (lc) {
				if (unpause_offs != std::string::npos) {
					s.erase(0, 8);
					DebugOutput("Unpause hotkey=%s", s.c_str());
					g_unpause_key = std::stoi(s, 0, 16);
				}
			}

			if (playback_offs != std::string::npos) {
				s.erase(0, 9);
				DebugOutput("Playback hotkey=%s", s.c_str());
				g_playback_key = std::stoi(s, 0, 16);
			}

			if (defaultspeed_offs != std::string::npos) 
			{
				s.erase(0, 13);
				DebugOutput("Defaultspeed hotkey=%s", s.c_str());
				g_defaultspeed_key = std::stoi(s, 0, 16);
			}

			if (increasespeed_offs != std::string::npos)
			{
				s.erase(0, 14);
				DebugOutput("Increasespeed hotkey=%s", s.c_str());
				g_increasespeed_key = std::stoi(s, 0, 16);
			}

			if (decreasespeed_offs != std::string::npos) 
			{
				s.erase(0, 14);
				DebugOutput("Decreasespeed hotkey=%s", s.c_str());
				g_decreasespeed_key = std::stoi(s, 0, 16);
			}

			if (framestep_offs != std::string::npos)
			{
				s.erase(0, 10);
				DebugOutput("Framestep hotkey=%s", s.c_str());
				g_framestep_key = std::stoi(s, 0, 16);
			}
			lc++;
		}

		fclose(fp);
	}
}