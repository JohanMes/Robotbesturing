#ifndef LIST_INCLUDED
#define LIST_INCLUDED

typedef struct list List;

struct list {
	void* data;
	List* next;
};

List* new_list(void* data);
void delete_list(List* list);
void list_append(List* list, void* data);

#endif
