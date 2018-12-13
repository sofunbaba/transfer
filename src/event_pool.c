#include "event_pool.h"
#include "glib.h"
#include "log.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/prctl.h>
#include "router.h"

#define EVENT_INTERNAL_INCREF(ep) __sync_add_and_fetch(&ep->len, 1)
#define EVENT_INTERNAL_DECREF(ep) __sync_sub_and_fetch(&ep->len, 1)
#define EVENT_INTERNAL_RELEASE(ep) __sync_lock_release(&ep->len)

static void *main_event(void *arg)
{
    struct event_pool *pool = arg;
    GMainContext *context = g_main_context_new();
    GMainLoop *loop = g_main_loop_new(context, false);

    GThread *thr = g_thread_self();
    struct event_loop *evloop = g_malloc0(sizeof(struct event_loop));
    evloop->loop = loop;
    evloop->pool = pool;
    EVENT_INTERNAL_RELEASE(evloop);
    g_hash_table_insert(pool->thr_table, thr, evloop);

    g_main_loop_run(loop);

    g_main_context_unref(context);
    g_main_loop_unref(loop);
    g_hash_table_remove(pool->thr_table, thr);
    
    char tname[strlen(pool->name+sizeof(guint32)*8)];
    prctl(PR_GET_NAME, tname);
    log_message(LOG_EVENT_POOL, "EventPool [%s] exit!", tname);
}

static gboolean thr_table_cb(void *key, void *val, void *user_data)
{
    struct fd_source *fs = user_data;
    struct event_loop *ep = val;
    struct event_pool *pool = ep->pool;

    log_message(LOG_EVENT_POOL, "ep->len:%d. max_event:%d", ep->len, pool->max_event);
    if(ep->len < pool->max_event)
    {
        GMainContext *context = g_main_loop_get_context(ep->loop);
        g_source_attach(fs->src, context);
        fs->ep = ep;
        struct router_node *node = router_node_new(fs);

        int fd = g_io_channel_unix_get_fd(fs->io);
        router_add(GINT_TO_POINTER(fd), node);
        EVENT_INTERNAL_INCREF(ep);

        log_message(LOG_EVENT_POOL, "Add a fs :%d", fd);

        return true;
    }

    return false;
}

static void event_pool_fd_source_destory(struct fd_source *fs)
{
    g_return_if_fail(fs != NULL);

    g_source_destroy(fs->src);
    g_source_unref(fs->src);
    g_io_channel_unref(fs->io);
    g_free(fs);
}

void event_pool_dettach(int fd)
{
   struct router_node *node = router_find_node_by_key(GINT_TO_POINTER(fd));
   if(node)
   {
       log_message(LOG_EVENT_POOL, "fd:%d dettached ep->len:%d", fd, node->fs->ep->len);

       EVENT_INTERNAL_DECREF(node->fs->ep);
       event_pool_fd_source_destory(node->fs);

       register_table_remove(node->user_name);
   }

   g_hash_table_remove(router_table, GINT_TO_POINTER(fd));
}

void event_pool_attach(struct event_pool *pool, int fd)
{
    struct fd_source *fs = g_malloc0(sizeof(struct fd_source));
    fs->io = g_io_channel_unix_new(fd);
    g_io_channel_set_close_on_unref(fs->io, true);
    fs->src = g_io_create_watch(fs->io, G_IO_IN|G_IO_ERR);
    g_source_set_callback(fs->src, pool->event_cb, pool->user_data, NULL);

    if(g_hash_table_find(pool->thr_table, thr_table_cb, fs) == NULL)
    {
        event_pool_fd_source_destory(fs);
        log_message(LOG_EVENT_POOL, "attach fd:%d failed!", fd);
    }
}

static void event_pool_hash_table_key_destory(void *key)
{
    g_thread_unref(key);
}

static void event_pool_hash_table_val_destory(void *val)
{
    g_free(val);
}

struct event_pool *event_pool_new(char *name, GSourceFunc event_cb, void *data, guint32 max_event, guint32 max_thread)
{
    struct event_pool *thr = g_malloc0(sizeof(struct event_pool));

    // log_enable(LOG_EVENT_POOL);

    strncpy(thr->name, name, sizeof(thr->name));
    thr->max_event    = max_event;
    thr->max_thread   = max_thread;
    thr->thr_table    = g_hash_table_new_full(NULL, NULL, 
                        event_pool_hash_table_key_destory, 
                        event_pool_hash_table_val_destory);
    thr->user_data    = data;
    thr->event_cb     = event_cb;

    guint32 i = 0;
    for(i=0; i< max_thread; i++)
    {
        char thr_name[strlen(name)+sizeof(guint32)*8];
        memset(thr_name, 0, sizeof(thr_name));
        snprintf(thr_name, sizeof(thr_name), "%s-%d", name, i);
        g_thread_new(thr_name, main_event, thr);
    }

    return thr;
}

