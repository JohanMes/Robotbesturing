#ifndef RESOURCE_INCLUDED
#define RESOURCE_INCLUDED

#include <windows.h>
#include <stdio.h>

#include "resource.h"
#include "city.h"
#include "list.h"
#include "map.h"
#include "road.h"
#include "mario.h"

#define IDI_MAINICON 1

#define ID_LOG      1001
#define ID_MARIO    1002
#define ID_BAUD     1003
#define ID_UPDATE   1004
#define ID_FOCUS    1005
#define ID_RESET    1006
#define ID_TURN     1007
#define ID_CITYFROM 1009
#define ID_FIND     1010
#define ID_CITY1TO  1011
#define ID_CITY2TO  1012
#define ID_CITY3TO  1013

#define CITYNAMEMAXLEN 100

// Win32-troep
extern HANDLE ComHandle;
extern HWND Logveld;
extern HWND MarioText;
extern HWND TurnText;
extern HWND hwnd;
extern HWND Baud;
extern HWND BaudText;
extern HWND Com;
extern HWND ComText;
extern HWND CityFrom;
extern HWND CityFromText;
extern HWND City1To;
extern HWND City1ToText;
extern HWND City2To;
extern HWND City2ToText;
extern HWND City3To;
extern HWND City3ToText;
extern HWND Update;
extern HWND Focus;
extern WNDPROC OldFocusProc;
extern HFONT logfont;
extern HFONT staticfont;
extern HFONT interfacefont;
extern HBRUSH transparentbrush;
extern COLORREF tkey;
extern HANDLE ComReader;

extern BOOL mario;
extern BOOL leftarrow;
extern BOOL rightarrow;
extern BOOL uparrow;
extern BOOL downarrow;
extern BOOL connected;
extern BOOL glassenabled;
extern BOOL recalcroute;

extern List* map;

extern City* current;
extern City* next;
extern City* currentto;

extern City* from;
extern City* to1;
extern City* to2;
extern City* to3;

void StdOut(const char* format,...);
void StdErr(const char* format,...);

unsigned char CharArrayToSignal(const char* signalin);
void SignalToCharArray(unsigned char signalin,char* chararrayout);

void UpdateSignal();

void UpdateMapMine();

void ResetMap();
void FindShortestRoute(City* from,City* to);

DWORD ReadByte(unsigned char* byte);
BOOL WriteByte(const unsigned char byte);

#endif
