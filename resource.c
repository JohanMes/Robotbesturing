#include <windows.h>
#include <stdio.h>

#include "resource.h"
#include "city.h"
#include "list.h"
#include "map.h"
#include "road.h"
#include "mario.h"

// Win32-troep
HANDLE ComHandle = NULL;
HWND Logveld = NULL;
HWND MarioText = NULL;
HWND TurnText = NULL;
HWND hwnd = NULL;
HWND Baud = NULL;
HWND BaudText = NULL;
HWND Com = NULL;
HWND ComText = NULL;
HWND CityFrom = NULL;
HWND CityFromText = NULL;
HWND City1To = NULL;
HWND City1ToText = NULL;
HWND City2To = NULL;
HWND City2ToText = NULL;
HWND City3To = NULL;
HWND City3ToText = NULL;
HWND Update = NULL;
HWND Focus = NULL;
WNDPROC OldFocusProc = NULL;
HFONT logfont = NULL;
HFONT staticfont = NULL;
HFONT interfacefont = NULL;
HBRUSH transparentbrush = NULL;
COLORREF tkey = 0;
HANDLE ComReader = NULL;

BOOL mario = FALSE;
BOOL leftarrow = FALSE;
BOOL rightarrow = FALSE;
BOOL uparrow = FALSE;
BOOL downarrow = FALSE;
BOOL connected = FALSE;
BOOL glassenabled = FALSE;
BOOL recalcroute = TRUE;

List* map = NULL;

City* current = NULL;
City* next = NULL;
City* currentto = NULL;

City* from = NULL;
City* to1 = NULL;
City* to2 = NULL;
City* to3 = NULL;
