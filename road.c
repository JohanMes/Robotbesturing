#include <stdlib.h>

#include "resource.h"
#include "city.h"
#include "list.h"
#include "map.h"
#include "mario.h"

Road* new_road(City* origin, City* destination) {
	Road* road = (Road*)malloc(sizeof(Road));

	road->origin = origin;
	road->destination = destination;
	road->disabled = FALSE;

	return road;
}

void delete_road(Road* road) {
	if(road != NULL)
		free(road);
}

void delete_roads(List* roads) {
	List* roads_copy = roads;

	while(roads_copy) {
		delete_road((Road*)roads_copy->data);
		roads_copy = roads_copy->next;
	}
	delete_list(roads);
}
