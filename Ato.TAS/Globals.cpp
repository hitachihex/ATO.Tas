#include "stdafx.h"
#include "Globals.h"

unsigned long murmurhash32_addr = 0;
unsigned long varnames_list_addr = 0;
unsigned long eventloop_addr = 0;
unsigned long yoyoupdate_addr = 0;
unsigned long keyholdptr_addr = 0;
unsigned long keyreleasedptr_addr = 0;
unsigned long keypressedptr_addr = 0;
unsigned long currentroomptr_addr = 0;
unsigned long drawenabled_addr = 0;
unsigned long yoyooldrandom_addr = 0;
unsigned long roomrestart_addr = 0;
unsigned long roomgoto_addr = 0;
unsigned long globalobject_addr = 0;
unsigned long atobase_addr = 0;
unsigned long instancenameslist_addr = 0;
unsigned long roomindex_addr = 0;

unsigned long onground_index = 0;
unsigned long xspeed_index = 0;
unsigned long yspeed_index = 0;

int g_pause_key = 0x70;
int g_unpause_key = 0x69;
int g_playback_key = 0x72;
int g_defaultspeed_key = 0x6F;
int g_increasespeed_key = 0x6B;
int g_decreasespeed_key = 0x6D;
int g_framestep_key = 0xDD;