#ifndef _CNM_H_
#define _CNM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#ifndef NM_CHILD_NODE
    #define NM_CHILD_NODE 16
#endif

#ifndef NM_CHAR_NAME
    #define NM_CHAR_NAME 64
#endif

#ifndef NM_SIGNAL_HASH
    #define NM_SIGNAL_HASH 67
#endif

#ifndef NM_CALLBACK_HASH
    #define NM_CALLBACK_HASH NM_SIGNAL_HASH
#endif

#ifndef NM_NODE_STATE
    #define NM_NODE_STATE 16
#endif


typedef struct node_t nm_node_t;

typedef void (*nm_node_method_t)(nm_node_t*);

typedef struct node_machine_t nm_node_machine_t;

nm_node_t* nm_node_create(void* p_owner, nm_node_method_t p_init, nm_node_method_t p_destroy, nm_node_method_t p_awake, nm_node_method_t p_sleep, nm_node_method_t p_process, nm_node_method_t p_render);
void nm_node_destroy(nm_node_t* p_node);

void nm_node_process(nm_node_t* p_node);
void nm_node_render(nm_node_t* p_node);

void* nm_node_get_owner(nm_node_t* p_node);
void* nm_node_get_parent_owner(nm_node_t* p_node);
void* nm_node_get_child_owner(nm_node_t* p_node, uint16_t p_index);

bool nm_node_add_child(nm_node_t* p_parent, nm_node_t* p_child, uint16_t p_index);
void nm_node_remove_child(nm_node_t* p_parent, uint16_t p_index);

nm_node_t* nm_node_get_parent(nm_node_t* p_node);
nm_node_t* nm_node_get_child(nm_node_t* p_node, uint16_t p_index);

bool nm_node_add_signal(nm_node_t* p_node, const char* p_signal_name);
bool nm_node_connect_signal(nm_node_t* p_node, const char* p_signal_name, const char* p_callback_name, nm_node_t* p_callback_node, nm_node_method_t p_callback_method);
void nm_node_emit_signal(nm_node_t* p_node, const char* p_signal_name);
void nm_node_remove_signal(nm_node_t* p_node, const char* p_signal_name);

nm_node_machine_t* nm_node_machine_create();
void nm_node_machine_destroy(nm_node_machine_t* p_machine);

void nm_node_machine_process(nm_node_machine_t* p_machine);
void nm_node_machine_render(nm_node_machine_t* p_machine);

nm_node_t* nm_node_machine_get_core(nm_node_machine_t* p_machine);

bool nm_node_machine_add_node(nm_node_machine_t* p_machine, nm_node_t* p_node, int16_t p_type);
void nm_node_machine_remove_node(nm_node_machine_t* p_machine, int16_t p_type);

void nm_node_machine_set_active_node(nm_node_machine_t* p_machine, int16_t p_type);

#ifdef __cplusplus
}
#endif

#endif