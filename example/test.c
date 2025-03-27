#include "../CNM/cnm.h"

typedef struct {
	nm_node_t* node;

	int x;
	int y;
} object_t;


static void object_process(nm_node_t* p_node){
	object_t* l_object = (object_t*)nm_node_get_owner(p_node);

	if (l_object->x == 5) return;

	nm_node_emit_signal(p_node, "test signal");

	(l_object->x)++;
	(l_object->y)++;
}

static void object_render(nm_node_t* p_node){
	object_t* l_object = (object_t*)nm_node_get_owner(p_node);

	printf("X : %d | Y : %d \n", l_object->x, l_object->y);
}

static void signal_callback(nm_node_t* p_node){
	object_t* l_object = (object_t*)nm_node_get_owner(p_node);

	printf("Callback from node with Y : %d\n", l_object->y);
}


int main(int argc, char** argv){
	// Test the node

	object_t obj;

	obj.x = 0;
	obj.y = 0;

	obj.node = nm_node_create(&obj, NULL, NULL, NULL, NULL, object_process, object_render);

	object_t obj_callback;

	printf("%lu\n", sizeof(nm_node_t*));

	obj_callback.x = 10;
	obj_callback.y = 15;
	obj_callback.node = nm_node_create(&obj_callback, NULL, NULL, NULL, NULL, NULL, NULL);

	nm_node_add_signal(obj.node, "test signal");
	nm_node_connect_signal(obj.node, "test signal", "test callback", obj_callback.node, signal_callback);

	for (int i = 0; i < 5; i++){
		nm_node_process(obj.node);
		nm_node_render(obj.node);	
	}
	
	// Test the child node

	// Test signaling

	// Test node machine
}