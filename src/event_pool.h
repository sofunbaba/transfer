#ifndef _EVENT_POOL_H_
#define _EVENT_POOL_H_

#include "glib.h"

#define EVENT_POOL_NAME_MAX 200

struct event_pool {
    char name[EVENT_POOL_NAME_MAX];
    guint32 max_thread;
    guint32 max_event;
    GHashTable *thr_table;
    void *user_data;
    GSourceFunc event_cb;
};

struct event_loop{
    guint32 len;
    GMainLoop *loop;
    struct event_pool *pool;
};

struct fd_source {
    GIOChannel *io;
    GSource *src;
    struct event_loop *ep;
};



struct event_pool *event_pool_new(char *name, GSourceFunc event_cb, void *data, guint32 max_event, guint32 max_thread);
void event_pool_attach(struct event_pool *pool, int fd);
void event_pool_dettach(int fd);









#endif

