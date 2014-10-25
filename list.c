#include <stdlib.h>

#include "resource.h"
#include "city.h"
#include "map.h"
#include "road.h"
#include "mario.h"

List* new_list(void* data) {
	List* list = (List*)malloc(sizeof(List));

	list->data = data;
	list->next = NULL;

	return list;
}

void list_append(List* list, void* data) {
	List* element = new_list(data);

	while(list->next) {
		list = list->next;	
	} 

	list->next = element;
}

void delete_list(List* list) {
	if(list != NULL)
		delete_list(list->next);
	free(list);
}
