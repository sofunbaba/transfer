#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <glib/glib.h>
#include <glib-unix.h>
#include "libzebra.h"
#include "event_pool.h"
#include "server.h"
#include "router.h"
#include "process.h"
#include "log.h"

#define SERVER_PORT 12345
#define SERVER_CLINET_QUE_MAX 100

static GList *g_loop_list = NULL;
static GList *g_thread_list = NULL;

GMainLoop *m_main_loop_new(GMainContext *context)
{
    GMainLoop *loop = g_main_loop_new(context, false);
    g_loop_list = g_list_prepend(g_loop_list, loop);

    return loop;
}

static void loop_list_exit(void *data)
{
    GMainLoop *loop = data;
    g_main_loop_quit(loop);
}

GThread *m_thread_new(const gchar *name, GThreadFunc func, gpointer data)
{
    GThread *thr = g_thread_new(name, func, data);
    g_thread_list = g_list_prepend(g_thread_list, thr);

    return thr;
}

static void thread_list_exit(void *data)
{
    GThread *thr = data;
    g_thread_join(thr);
    g_thread_unref(thr);
}

void server_exit(void)
{
    raise(SIGINT);
}

static int sigint_cb(gpointer user_data)
{
    GMainLoop *loop = user_data;

    g_list_free_full(g_loop_list, loop_list_exit);
    g_list_free_full(g_thread_list, thread_list_exit);

    g_main_loop_quit(loop);

    return G_SOURCE_CONTINUE;
}

static int server_accept_cb(GIOChannel *io, GIOCondition cond, void *user_data)
{
    int server_fd = g_io_channel_unix_get_fd(io);
    struct event_pool *pool = user_data;

    int client_fd = 0;
    struct sockaddr_in client_sock;

    bzero(&client_sock, sizeof(client_sock));

    socklen_t client_sock_len = sizeof(client_sock);
    client_fd = accept(server_fd, (struct sockaddr *)&client_sock, &client_sock_len);
    if(client_fd < 0)
    {
        log_warning(LOG_INIT, "accept:%s", strerror(errno));
        return G_SOURCE_CONTINUE;
    }

    char buff[INET_ADDRSTRLEN+1];
    memset(buff, 0, sizeof(buff));

    const char *client_ip = inet_ntop(client_sock.sin_family, &client_sock.sin_addr, buff, sizeof(buff));
    if(client_ip == NULL)
    {
        log_warning(LOG_INIT, "inet_ntop:%s", strerror(errno));
        close(client_fd);
        return G_SOURCE_CONTINUE;
    }

    log_message(LOG_ACCEPT, "Accept client_fd:%d, ip:%s\r\n", client_fd, client_ip);

    event_pool_attach(pool, client_fd);

    return G_SOURCE_CONTINUE;
}

static gboolean event_pool_cb(GIOChannel *io, GIOCondition con, void *user_data)
{
    char *str = NULL;
    gsize len = 0, term_pos = 0;

    GIOStatus st = g_io_channel_read_line(io, &str, &len, &term_pos, NULL);    
    switch(st)
    {
        case G_IO_STATUS_NORMAL:
            str[term_pos] = '\0';
            process_dispatcher(io, str);
            g_free(str);
        case G_IO_STATUS_AGAIN:
            return G_SOURCE_CONTINUE;
        default:
            {
                int fd = g_io_channel_unix_get_fd(io);
                log_message(LOG_EVENT_POOL, "Recv error fd:%d, dettached!", fd);
                event_pool_dettach(fd);
                return G_SOURCE_REMOVE;
            }
    }
}

gboolean translate_init(LANG_TYPE type)
{
    char filename[200], path[200], lang[200];

    memset(filename, 0, sizeof(filename));
    memset(path, 0, sizeof(path));
    memset(lang, 0, sizeof(lang));

    char *str = NULL;
    if(type == LANG_CN)
        str = "zh_CN";
    else if(type == LANG_EN)
        str = "en_US";
    else
        return false;

    snprintf(lang, sizeof(lang), "%s.utf8", str);
    if(setlocale(LC_ALL, lang) == NULL)
        return false;

    snprintf(filename, sizeof(filename), "%s_%s", PACKAGE, str);
    snprintf(path, sizeof(path), "%s/po", getenv("PWD"));
    if(bindtextdomain(filename, path) == NULL)
        return false;

    if(textdomain(filename) == NULL)
        return false;

    return true;
}

int main(int argc, char *argv[])
{
    char buff[200];

    memset(buff, 0, sizeof(buff));

    if(log_init(NULL) == false)
        log_warning(LOG_INIT, "Log init failed!");

    /* init the language table */
    if(translate_init(LANG_EN) == false)
        log_message(LOG_INIT, "Failed to init translate");

    struct event_pool *pool = event_pool_new("event thr", (GSourceFunc)event_pool_cb, NULL, 100, 2);

    /* router thr */
    m_thread_new("router", router_main, NULL);

    /* zebra thr */
    struct zebra_param param = {"ZEBRA", "admin", "admin", 2000};
    GThread *zebra_thr = g_thread_new("zebra", zebra_main, &param);
    log_message(LOG_INIT, "Zebra start with port %d ...", param.port);

    GMainContext *context = g_main_context_new();
    GMainLoop    *loop    = g_main_loop_new(context, false);

    /* watch the SIGINT signal. */
    GSource *sigint_src = g_unix_signal_source_new(SIGINT);
    g_source_set_callback(sigint_src, sigint_cb, loop, NULL);
    g_source_attach(sigint_src, context);

    /* server */
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    int val = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0)
        log_error(LOG_INIT, "setsockopt:%s", strerror_r(errno, buff, sizeof(buff)));

    struct sockaddr_in serv_sock;
    bzero(&serv_sock, sizeof(struct sockaddr_in));
    serv_sock.sin_family = AF_INET;
    serv_sock.sin_addr.s_addr = INADDR_ANY;
    serv_sock.sin_port = htons(SERVER_PORT);

    if(bind(server_fd, (struct sockaddr *)&serv_sock, sizeof(struct sockaddr)) < 0)
        log_error(LOG_INIT, "bind:%s", strerror_r(errno, buff, sizeof(buff)));

    if(listen(server_fd, SERVER_CLINET_QUE_MAX) < 0)
        log_error(LOG_INIT, "listen:%s", strerror_r(errno, buff, sizeof(buff)));

    GIOChannel *server_io = g_io_channel_unix_new(server_fd);
    g_io_channel_set_close_on_unref(server_io, true);

    GSource *server_src = g_io_create_watch(server_io, G_IO_IN);
    g_source_set_callback(server_src, (GSourceFunc)server_accept_cb, pool, NULL);
    g_source_attach(server_src, context);

    /* running ... */
    g_main_loop_run(loop);

    g_io_channel_unref(server_io);
    g_source_unref(server_src);
    g_source_unref(sigint_src);
    g_main_loop_unref(loop);
    g_main_context_unref(context);

    log_message(LOG_INIT, "Shutdown ...");

    return 0;
}
