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
	atobase_addr = (unsigned long)GetModuleHandleA("Ato.exe");

	murmurhash32_addr = (atobase_addr+ATO_MURMURHASH32_RVA);
	varnames_list_addr = (atobase_addr + ATO_VARIABLE_NAMESLIST_RVA);
	eventloop_addr = (atobase_addr + ATO_EVENTLOOP_RVA);
	yoyoupdate_addr = (atobase_addr + ATO_YOYOUPDATE_RVA);
	keyholdptr_addr = (atobase_addr + ATO_KEYHOLDPTR_RVA);
	keyreleasedptr_addr = (atobase_addr + ATO_KEYRELEASEDPTR_RVA);
	keypressedptr_addr = (atobase_addr + ATO_KEYPRESSEDPTR_RVA);
	currentroomptr_addr = (atobase_addr + ATO_CURRENTROOMPTR_RVA);
	drawenabled_addr = (atobase_addr + ATO_DRAWENABLED_RVA);
	yoyooldrandom_addr = (atobase_addr + ATO_YOYOOLDRANDOM_RVA);
	roomrestart_addr = (atobase_addr + ATO_ROOMRESTART_RVA);
	roomgoto_addr = (atobase_addr + ATO_ROOMGOTO_RVA);
	globalobject_addr = (atobase_addr + ATO_GLOBAL_OBJECT_RVA);
	instancenameslist_addr = (atobase_addr + ATO_INSTANCENAMESLIST_RVA);
	roomindex_addr = (atobase_addr + ATO_ROOMINDEX_RVA);

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

// dwEcx = 0x
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