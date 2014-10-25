#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "resource.h"
#include "city.h"
#include "list.h"
#include "road.h"
#include "mario.h"

void delete_map(List* map) {
	List* map_copy = map;
	while(map_copy != NULL) {
		City* city = (City*)map_copy->data;

		delete_city(city);

		map_copy = map_copy->next;
	}
	delete_list(map);
}

List* create_map(FILE* data_file) {
	
	srand(GetTickCount());
	
	int numcities = 0;
	int numroads = 0;

	int x = 0;
	int y = 0;

	List* map = NULL;

	fscanf(data_file,"%d",&numcities);
	for(int i = 0;i < numcities;i++) {
		char city_name[CITYNAMEMAXLEN + 1];
		City* city = NULL;
		
		fscanf(data_file,"%s %d %d",city_name,&x,&y);

		if(find_city(map, city_name) != NULL) {
			StdErr("City %s already on map\r\n",city_name);
		}

		city = new_city(strdup(city_name),x,y);

		if(map == NULL) {
			map = new_list(city);
		} else {
			list_append(map, city);
		}
	}
	
	fscanf(data_file,"%d",&numroads);
	for(int i = 0;i < numroads;i++) {
		
		// Dump voor fscanf
		char fromname[CITYNAMEMAXLEN + 1] = "";
		char toname[CITYNAMEMAXLEN + 1] = "";
		
		// uiteindelijke data
		City* from = NULL;
		City* to = NULL;
		Road* roadfrom = NULL;
		Road* roadto = NULL;
		
		fscanf(data_file,"%s %s",fromname,toname);
		from = find_city(map,fromname);
		to = find_city(map,toname);
		
		if(from && to) {
			roadfrom = new_road(from,to);
			if(from->roads == NULL) {
				from->roads = new_list(roadfrom);
			} else {
				list_append(from->roads,roadfrom);
			}
			
			roadto = new_road(to,from);
			if(to->roads == NULL) {
				to->roads = new_list(roadto);
			} else {
				list_append(to->roads,roadto);
			}
		}
	}
	return map;
}
