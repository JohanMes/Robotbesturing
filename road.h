#ifndef ROAD_INCLUDED
#define ROAD_INCLUDED

#include <windows.h>

#include "city.h"
#include "list.h"

typedef struct {
	City* origin;
	City* destination;
	BOOL disabled; // deze gaat richting een mijn
} Road;

Road* new_road(City* origin,City* destination);
void delete_road(Road* road);
void delete_roads(List* roads);

#endif
