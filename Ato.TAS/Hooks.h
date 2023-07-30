#pragma once

#include "Addresses.h"

typedef bool(__cdecl * oEventLoop)(void*, void*, void*, void*, void*);
typedef void(__cdecl * oYoyoUpdate)();

extern oEventLoop original_EventLoop;
extern oYoyoUpdate original_YoyoUpdate;

extern void __cdecl YoYoUpdate_Hook();
extern bool __cdecl EventLoop_Hook(void *, void*, void*, void*, void*);
