#include "stdafx.h"
#include "EasyHookUtils.h"

NTSTATUS AddHook(void* pInEntryPoint, void * pHookProc, void * pCallback, TRACED_HOOK_HANDLE traceRecord)
{
	return LhInstallHook(pInEntryPoint, pHookProc, pCallback, traceRecord);
}


NTSTATUS MakeHookExclusive(unsigned long * pThreadListArray, unsigned long dwThreadCount, TRACED_HOOK_HANDLE traceRecord)
{
	return LhSetExclusiveACL(pThreadListArray, dwThreadCount, traceRecord);
}