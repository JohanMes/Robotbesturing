#ifndef CITY_INCLUDED
#define CITY_INCLUDED

typedef struct city City;

#include <windows.h>
#include "road.h"
#include "list.h"

struct city {
	char* name;
	List* roads;
	unsigned int x;
	unsigned int y;
	BOOL hasmine;
	BOOL highlighted;
	
	 // zooi voor findshortestroute
	BOOL visited;
	unsigned int totaldistance;
};

City* new_city(char* name,int x,int y);
void delete_city(City* city);
City* find_city(List* list_of_cities,const char* city_name);

#endif
