#ifndef _ROUTER_MAIN_H_
#define _ROUTER_MAIN_H_

#include <stdbool.h>
#include <glib.h>
#include <gmodule.h>
#include <unistd.h>
#include "event_pool.h"

#define ROUTER_USER_NAME_MAX 100

struct router_node {
    GTimer *last_update;
    struct fd_source *fs;
    char user_name[ROUTER_USER_NAME_MAX];
    gboolean active;
};


void *router_main(void *arg);


void *router_node_new(void *fs);
gboolean router_add(void *key, void *node);
void *router_find_node_by_key(void *key);

gboolean register_table_key_exist(void *key);
void *register_table_find_by_key(void *key);
gboolean register_table_add(void *key, void *val);
void register_table_remove(void *key);
void **register_table_get_keys_as_array(guint *len);





extern GHashTable *router_table;



#endif

