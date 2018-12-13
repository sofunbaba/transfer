#include "router.h"
#include "server.h"
#include "log.h"

#define ROUTER_TABLE_DEAD_TIME 60
#define ROUTER_TABLE_SCAN_TIME 5

GHashTable *router_table = NULL;
static GHashTable *register_table = NULL;

static void router_key_destory_func(void *data)
{
}

static void router_val_destory_func(void *data)
{
    struct router_node *node = (struct router_node *)data;

    g_timer_destroy(node->last_update);
    g_free(node);
}

void *router_node_new(void *fs)
{
    struct router_node *node = g_malloc0(sizeof(struct router_node));

    node->fs = fs;
    node->active = false;
    node->last_update = g_timer_new();
}

gboolean router_add(void *key, void *node)
{
    if(g_hash_table_contains(router_table, key))
    {
        log_warning(LOG_ROUTER, "The key has existed:%s!", key);
        return false;
    }

    g_hash_table_insert(router_table, key, node);
}

void *router_find_node_by_key(void *key)
{
    return g_hash_table_lookup(router_table, key);
}

static void router_table_dead_cb(void *val, void *user_data)
{
    struct router_node *node = val;

    gdouble elapsed_sec = g_timer_elapsed(node->last_update, NULL);
    if(elapsed_sec > ROUTER_TABLE_DEAD_TIME && (node->active == false))
    {
        int fd = g_io_channel_unix_get_fd(node->fs->io);
        event_pool_dettach(fd);
    }
}

static gboolean timeout_cb(void *data)
{
    log_message(LOG_ROUTER, "Router table timeout ...");


    guint len = 0, i = 0;
    GList *val_list = g_hash_table_get_values(router_table);

    g_list_foreach(val_list, router_table_dead_cb, NULL);

    g_list_free(val_list);

    return G_SOURCE_CONTINUE;
}

gboolean register_table_add(void *key, void *val)
{
    return g_hash_table_insert(register_table, key, val);
}

void *register_table_find_by_key(void *key)
{
    return g_hash_table_lookup(register_table, key);
}

gboolean register_table_key_exist(void *key)
{
    return g_hash_table_contains(register_table, key);
}

void register_table_remove(void *key)
{
    g_hash_table_remove(register_table, key);
}

void **register_table_get_keys_as_array(guint *len)
{
    return g_hash_table_get_keys_as_array(register_table, len);
}

void *router_main(void *arg)
{
    // log_enable(LOG_ROUTER);
    log_message(LOG_INIT, "Start router thread ...");

    router_table = g_hash_table_new_full(NULL, NULL, 
                                         router_key_destory_func, 
                                         router_val_destory_func);

    register_table = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, NULL);

    GMainContext *context = g_main_context_new();
    GMainLoop    *loop    = m_main_loop_new(context);

    GSource *timeout_src = g_timeout_source_new_seconds(ROUTER_TABLE_SCAN_TIME);
    g_source_set_callback(timeout_src, timeout_cb, NULL, NULL);
    g_source_attach(timeout_src, context);

    g_main_loop_run(loop);

    g_source_unref(timeout_src);
    g_main_loop_unref(loop);
    g_main_context_unref(context);

    log_message(LOG_ROUTER, "Router table thr exit!");

    return NULL;
}
