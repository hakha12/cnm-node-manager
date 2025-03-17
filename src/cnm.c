#include "../CNM/cnm.h"

struct node_t {  

    void* _owner;

    // Parent node pointer

    struct node_t* _parent;

    // Child node container

    struct node_t* _child[NM_CHILD_NODE];

    // Internal method that were automatically called

    nm_node_method_t _in_init;
    nm_node_method_t _in_destroy;
    nm_node_method_t _in_awake;
    nm_node_method_t _in_sleep;
    nm_node_method_t _in_process;
    nm_node_method_t _in_render;

    // External method set by user

    nm_node_method_t _ext_init;
    nm_node_method_t _ext_destroy;
    nm_node_method_t _ext_awake;
    nm_node_method_t _ext_sleep;
    nm_node_method_t _ext_process;
    nm_node_method_t _ext_render;

    // Signal and callback struct

    struct {
        char _key[NM_CHAR_NAME];

        struct {
            char _key[NM_CHAR_NAME];
            struct node_t* _node;
            nm_node_method_t _method;
            bool _in_use;
        } _callback_hash[NM_CALLBACK_HASH];

        bool _in_use;

    } _signal_hash[NM_SIGNAL_HASH];


};

struct node_machine_t {
	// Pointer to the root node

	struct node_t* _root;

	// Container of the node state

	struct node_t* _node_states[NM_NODE_STATE];

	// Pointer to the current state

	struct node_t* _current_state;
};

// Local function declaration

static void _node_in_init(struct node_t* p_node);
static void _node_in_destroy(struct node_t* p_node);
static void _node_in_awake(struct node_t* p_node);
static void _node_in_sleep(struct node_t* p_node);
static void _node_in_process(struct node_t* p_node);
static void _node_in_render(struct node_t* p_node);

static void _node_machine_in_process(struct node_t* p_node);
static void _node_machine_in_render(struct node_t* p_node);

static uint64_t _get_hash_key(const char* p_key);


// Function Definition

nm_node_t* nm_node_create(void* p_owner, nm_node_method_t p_init, nm_node_method_t p_destroy, nm_node_method_t p_awake, nm_node_method_t p_sleep, nm_node_method_t p_process, nm_node_method_t p_render){
    struct node_t* l_node = (struct node_t*)malloc(sizeof(struct node_t));

    l_node->_owner = p_owner;

    l_node->_parent = NULL;

    memset(l_node->_child, 0, sizeof(l_node->_child));

    l_node->_ext_init = p_init;
    l_node->_ext_destroy = p_destroy;
    l_node->_ext_awake = p_awake;
    l_node->_ext_sleep = p_sleep;
    l_node->_ext_process = p_process;
    l_node->_ext_render = p_render;

    l_node->_in_init = _node_in_init;
    l_node->_in_destroy = _node_in_destroy;
    l_node->_in_awake = _node_in_awake;
    l_node->_in_sleep = _node_in_sleep;
    l_node->_in_process = _node_in_process;
    l_node->_in_render = _node_in_render;

    l_node->_in_init(l_node);

    memset(l_node->_signal_hash, 0, sizeof(l_node->_signal_hash));

    return l_node;
}

void nm_node_destroy(nm_node_t* p_node){
    free(p_node);
}

void nm_node_process(nm_node_t* p_node){
    p_node->_in_process(p_node);
}

void nm_node_render(nm_node_t* p_node){
    p_node->_in_render(p_node);
}

void* nm_node_get_owner(nm_node_t* p_node){
    if (!p_node->_owner){
        return NULL;
    }

    return p_node->_owner;
}

void* nm_node_get_parent_owner(nm_node_t* p_node){
    void* l_owner = nm_node_get_owner(p_node->_parent);

    return l_owner;
}

void* nm_node_get_child_owner(nm_node_t* p_node, uint16_t p_index){
    if (p_index >= NM_CHILD_NODE){
        return NULL;
    }

    void* l_owner = nm_node_get_owner(p_node->_child[p_index]);

    return l_owner;
}

bool nm_node_add_child(nm_node_t* p_parent, nm_node_t* p_child, uint16_t p_index){
    if (p_index >= NM_CHILD_NODE){
        return false;
    }   

    if (p_parent->_child[p_index]){
        return false;
    }

    p_parent->_child[p_index] = p_child;
    
    p_child[p_index]._parent = p_parent;

    return true;
}

void nm_node_remove_child(nm_node_t* p_parent, uint16_t p_index){
    if (p_index >= NM_CHILD_NODE){
        return;
    }
    
    if (p_parent->_child[p_index]){
        return;
    }

    p_parent->_child[p_index] = NULL;
}

nm_node_t* nm_node_get_parent(nm_node_t* p_node){
    if (!p_node->_parent){
        return NULL;
    }

    return p_node->_parent;
}

nm_node_t* nm_node_get_child(nm_node_t* p_node, uint16_t p_index){
    if (p_index >= NM_CHILD_NODE){
        return NULL;
    }

    if (!p_node->_child[p_index]){
        return NULL;
    }

    return p_node->_child[p_index];
}

bool nm_node_add_signal(nm_node_t* p_node, const char* p_signal_name){
    if (strlen(p_signal_name) > NM_CHAR_NAME){
        return false;
    }

    uint64_t l_key = _get_hash_key(p_signal_name);

    while (p_node->_signal_hash[l_key]._in_use){
        if (strncmp(p_node->_signal_hash[l_key]._key, p_signal_name, NM_CHAR_NAME) == 0){
            return true;
        }

        l_key = (l_key + 1) % NM_SIGNAL_HASH;
    }

    strncpy(p_node->_signal_hash[l_key]._key, p_signal_name, NM_CHAR_NAME - 1);
    p_node->_signal_hash[l_key]._key[NM_CHAR_NAME - 1] = '\0';

    memset(p_node->_signal_hash[l_key]._callback_hash, 0, sizeof(p_node->_signal_hash[l_key]._callback_hash));

    p_node->_signal_hash[l_key]._in_use = true;

    return true;
}

bool nm_node_connect_signal(nm_node_t* p_node, const char* p_signal_name, const char* p_callback_name, nm_node_t* p_callback_node, nm_node_method_t p_callback_method){
    if (strlen(p_signal_name) > NM_CHAR_NAME || strlen(p_callback_name) > NM_CHAR_NAME){
        return false;
    }

    uint64_t l_signal_key = _get_hash_key(p_signal_name);
    bool l_signal_found = false;

    for (int i = 0; i < NM_SIGNAL_HASH; i++){
        if (!p_node->_signal_hash[l_signal_key]._in_use){
            return false;
        }

        if (strncmp(p_node->_signal_hash[l_signal_key]._key, p_signal_name, NM_CHAR_NAME) == 0){
            l_signal_found = true;

            break;
        }

        l_signal_key = (l_signal_key + 1) % NM_SIGNAL_HASH;
    }

    if (!l_signal_found){
        return false;
    }

    uint64_t l_callback_key = _get_hash_key(p_callback_name);

    while (p_node->_signal_hash[l_signal_key]._callback_hash[l_callback_key]._in_use){
        if (strncmp(p_node->_signal_hash[l_signal_key]._callback_hash[l_callback_key]._key, p_callback_name, NM_CHAR_NAME) == 0){
            p_node->_signal_hash[l_signal_key]._callback_hash[l_callback_key]._node = p_callback_node;
            p_node->_signal_hash[l_signal_key]._callback_hash[l_callback_key]._method = p_callback_method;

            return true;
        }

        l_callback_key = (l_callback_key + 1) % NM_CALLBACK_HASH;
    }

    strncpy(p_node->_signal_hash[l_signal_key]._callback_hash[l_callback_key]._key, p_callback_name, NM_CHAR_NAME - 1);
    p_node->_signal_hash[l_signal_key]._callback_hash[l_callback_key]._key[NM_CHAR_NAME - 1] = '\0';

    p_node->_signal_hash[l_signal_key]._callback_hash[l_callback_key]._node = p_callback_node;
    p_node->_signal_hash[l_signal_key]._callback_hash[l_callback_key]._method = p_callback_method;

    p_node->_signal_hash[l_signal_key]._callback_hash[l_callback_key]._in_use = true;

    return true;
}

void nm_node_emit_signal(nm_node_t* p_node, const char* p_signal_name){
    if (strlen(p_signal_name) > NM_CHAR_NAME ){
        return;
    }

    uint64_t l_signal_key = _get_hash_key(p_signal_name);
    bool l_signal_found = false;

    for (int i = 0; i < NM_SIGNAL_HASH; i++){
        if (!p_node->_signal_hash[l_signal_key]._in_use){
            return;
        }

        if (strncmp(p_node->_signal_hash[l_signal_key]._key, p_signal_name, NM_CHAR_NAME) == 0){
            l_signal_found = true;

            break;
        }

        l_signal_key = (l_signal_key + 1) % NM_SIGNAL_HASH;
    }

    if (!l_signal_found){
        return;
    }

    for (int i = 0; i < NM_CALLBACK_HASH; i++){
        if (!p_node->_signal_hash[l_signal_key]._callback_hash[i]._in_use) continue;

        p_node->_signal_hash[l_signal_key]._callback_hash[i]._method(p_node->_signal_hash[l_signal_key]._callback_hash[i]._node);
    }
}

nm_node_machine_t* nm_node_machine_create(){
	struct node_machine_t* l_machine = (struct node_machine_t*)malloc(sizeof(struct node_machine_t));

	l_machine->_root = nm_node_create(l_machine, NULL, NULL, NULL, NULL, _node_machine_in_process, _node_machine_in_render);

	memset(l_machine->_node_states, 0, sizeof(l_machine->_node_states));

	l_machine->_current_state = NULL;

	return l_machine;
}

void nm_node_machine_destroy(nm_node_machine_t* p_machine){
	free(p_machine);
}

void nm_node_machine_process(nm_node_machine_t* p_machine){
	p_machine->_root->_in_process(p_machine->_root);
}

void nm_node_machine_render(nm_node_machine_t* p_machine){
	p_machine->_root->_in_render(p_machine->_root);
}

nm_node_t* nm_node_machine_get_root(nm_node_machine_t* p_machine){
	return p_machine->_root;
}

// Local function definition

static void _node_in_init(struct node_t* p_node){
    if (p_node->_ext_init){
        p_node->_ext_init(p_node);
    }

    for (int i = 0; i < NM_CHILD_NODE; i++){
        if (!p_node->_child[i] || !p_node->_child[i]->_in_init) continue;

        p_node->_child[i]->_in_init(p_node->_child[i]);
    }
}

static void _node_in_destroy(struct node_t* p_node){
    if (p_node->_ext_destroy){
        p_node->_ext_destroy(p_node);
    }

    for (int i = 0; i < NM_CHILD_NODE; i++){
        if (!p_node->_child[i] || !p_node->_child[i]->_in_destroy) continue;

        p_node->_child[i]->_in_destroy(p_node->_child[i]);
    }
}

static void _node_in_awake(struct node_t* p_node){
    if (p_node->_ext_awake){
        p_node->_ext_awake(p_node);
    }

    for (int i = 0; i < NM_CHILD_NODE; i++){
        if (!p_node->_child[i] || !p_node->_child[i]->_in_awake) continue;

        p_node->_child[i]->_in_awake(p_node->_child[i]);
    }
}

static void _node_in_sleep(struct node_t* p_node){
    if (p_node->_ext_sleep){
        p_node->_ext_sleep(p_node);
    }

    for (int i = 0; i < NM_CHILD_NODE; i++){
        if (!p_node->_child[i] || !p_node->_child[i]->_in_sleep) continue;

        p_node->_child[i]->_in_sleep(p_node->_child[i]);
    }
}

static void _node_in_process(struct node_t* p_node){
    if (p_node->_ext_process){
        p_node->_ext_process(p_node);
    }

    for (int i = 0; i < NM_CHILD_NODE; i++){
        if (!p_node->_child[i] || !p_node->_child[i]->_in_process) continue;

        p_node->_child[i]->_in_process(p_node->_child[i]);
    }
}

static void _node_in_render(struct node_t* p_node){
    if (p_node->_ext_render){
        p_node->_ext_render(p_node);
    }

    for (int i = 0; i < NM_CHILD_NODE; i++){
        if (!p_node->_child[i] || !p_node->_child[i]->_in_render) continue;

        p_node->_child[i]->_in_render(p_node->_child[i]);
    }
}

static void _node_machine_in_process(struct node_t* p_node){
	struct node_machine_t* l_machine = (struct node_machine_t*)nm_node_get_owner(p_node);

	if (l_machine->_current_state){
		l_machine->_current_state->_in_process(l_machine->_current_state);
	}
}

static void _node_machine_in_render(struct node_t* p_node){
	struct node_machine_t* l_machine = (struct node_machine_t*)nm_node_get_owner(p_node);

	if (l_machine->_current_state){
		l_machine->_current_state->_in_render(l_machine->_current_state);
	}
}

static uint64_t _get_hash_key(const char* p_key){
    #define FNV_OFFSET 14695981039346656037ULL
    #define FNV_PRIME  1099511628211ULL

    uint64_t l_hash = FNV_OFFSET;

    for (int i = 0; i < NM_CHAR_NAME && p_key[i] != '\0'; i++){
        l_hash ^= (uint8_t)p_key[i];
        l_hash *= FNV_PRIME;
    }

    return l_hash % NM_SIGNAL_HASH;
}