#include "../CNM/cnm.h"

typedef struct {
	nm_node_t* node;

	int x;
	int y;
} object_t;


static void object_process(nm_node_t* p_node){
	object_t* l_object = (object_t*)nm_node_get_owner(p_node);

	if (l_object->x == 5) return;

	(l_object->x)++;
	(l_object->y)++;
}

static void object_render(nm_node_t* p_node){
	object_t* l_object = (object_t*)nm_node_get_owner(p_node);

	printf("X : %d | Y : %d \n", l_object->x, l_object->y);
}


int main(int argc, char** argv){
	// Test the node

	object_t obj;

	obj.x = 0;
	obj.y = 0;

	obj.node = nm_node_create(&obj, NULL, NULL, NULL, NULL, object_process, object_render);

	for (int i = 0; i < 5; i++){
		nm_node_process(obj.node);
		nm_node_render(obj.node);	
	}
	
	// Test the child node

	// Test signaling

	// Test node machine
}