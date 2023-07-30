#pragma once

#include "stdafx.h"
#include <random>
#include "EasyHookUtils.h"
#include "GMEvent.h"
#include "Globals.h"

#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
#define ExclusiveHookWithCount(a,b) MakeHookExclusive(g_ACLEntries, a, b)
#define ExclusiveHook(b) MakeHookExclusive(g_ACLEntries, 1, b)

extern unsigned long g_ACLEntries[1];

extern HOOK_TRACE_INFO EventLoopHookHandle;
extern HOOK_TRACE_INFO YoyoUpdateHookHandle;	

extern HOOK_TRACE_INFO YoyoOldRandomHookHandle;
extern unsigned long __cdecl YoYoOldRandom_Hook();

extern std::ranlux64_base_01  g_rEng;
extern std::normal_distribution<double> * g_pFloatDist;

extern std::normal_distribution<double> * g_pUnifDist;
extern std::mt19937 g_mt1997;

extern bool g_bDrawRNGFix;

extern unsigned long g_dwConstantSeed;

typedef void(__cdecl * DrawText_Internal)(/*float, float, const char**/GameMaker_TextStructure*);
extern DrawText_Internal draw_text;

typedef void(__cdecl * DrawText_Internal2)(const char*, unsigned long, screenPos*, unsigned long, const char*, unsigned long, unsigned long);
extern DrawText_Internal2 draw_text2;

typedef void(__cdecl * DrawText_Internal3)(float, float, const char*);
extern DrawText_Internal3 draw_text3;

typedef unsigned long(__cdecl * oYoyoOldRandom)();
extern oYoyoOldRandom original_YoyoOldRandom;

typedef unsigned long(__cdecl* oMurmurHash32_fn)(const char* pcszString, unsigned int length, unsigned int seed);
extern oMurmurHash32_fn murmurhash_32;
// YYGSString is currently at 0x14, needs to be at 0x24?
typedef void(__cdecl * fnVariable_Global_Get)(void*, unsigned long a, unsigned long b, unsigned long c, YYGString*);
extern fnVariable_Global_Get variable_global_get;

typedef void(__cdecl * fnVariable_Instance_Get)(void*, unsigned long a, unsigned long b, unsigned long c, /*YYGArgumentList*/YYGNumberAndStringArgument*);
extern fnVariable_Instance_Get variable_instance_get;

typedef void(__cdecl * fnroom_restart)();
extern fnroom_restart room_restart;

typedef void(__cdecl * fnroom_goto)(unsigned long);
extern fnroom_goto room_goto;

extern GameMaker_TextStructure * g_pText;

extern void MyDrawText(float, float, const char*);

extern GMLRoomObjectInstance * GetPlayerRoomObjectInstance();
extern double GetGlobalVariableAsDouble(unsigned int);
extern double * GetGlobalVariableAsDoublePtr(unsigned int);
extern double * GetGlobalVariableAsDoublePtrWithOffsets(unsigned int *, unsigned int);
extern void DumpCurrentRoomInstanceIds();
extern void DumpPointersForExternalOSD();
extern GMLRoom * GetCurrentRoomPointer();
extern void SetDrawEnabled(bool);
void dump_variable_names(GMLRoomObjectInstance*, const char*);
unsigned long GetVariableIDFromHashedName(const char*, InstanceVariableProperties*);

unsigned long GetInstanceVariableIDFromName(const char*);
unsigned long GetGlobalVariableIndexByName(const char*, unsigned long);
VariablePointer* get_variable_by_index(unsigned long index, InstanceVariableProperties* pProps);

extern GMLRoomObjectInstance* GetGlobalObjectInstance();

extern unsigned int caGrace_Offsets[2];
extern unsigned int sba_Offsets[2];


#define ATOV11 1