#ifndef MARIO_INCLUDED
#define MARIO_INCLUDED

#include <stdio.h>

#include "resource.h"
#include "city.h"
#include "list.h"
#include "map.h"
#include "road.h"

void MarioIgnoreUntilLine();
void MarioFlush();
void MarioNoSignalForward(unsigned int ms);
void MarioLeft();
void MarioRight();
void MarioDisable();
void MarioTurnAround(BOOL ismine);
void MarioStop();

void MarioUpdate(int olddir,int newdir,BOOL ismine);

#endif
