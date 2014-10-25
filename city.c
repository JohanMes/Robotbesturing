#include <stdlib.h>
#include <string.h>

#include "resource.h"
#include "list.h"
#include "map.h"
#include "road.h"
#include "mario.h"

City* new_city(char* name,int x,int y) {
	City* city = (City*)malloc(sizeof(City));

	city->name = name;
	city->roads	= NULL;
	city->x = x;
	city->y = y;
	city->totaldistance = 0xFFFFFFFF;
	city->visited = FALSE;
	city->highlighted = FALSE;
	city->hasmine = FALSE;

	return city;
}

void delete_city(City* city) {
	if(city != NULL) {
		delete_roads(city->roads);	
		free(city->name);
		free(city);
	}
}

City* find_city(List* list_of_cities,const char* city_name) {
	while(list_of_cities != NULL) {
		City* city = (City*)list_of_cities->data;

		// If they're equal...
		if(!strcmp(city->name,city_name)) {
			return city;
		}

		list_of_cities = list_of_cities->next;
	}
	return NULL;
}
