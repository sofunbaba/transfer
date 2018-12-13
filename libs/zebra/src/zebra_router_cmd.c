#include "zebra_router_cmd.h"
#include <zebra.h>
#include "vty.h"
#include "log.h"
#include "command.h"
#include "server.h"
#include "event_pool.h"

#define _ROUTER_STR "router add/del/print node cmd\n"

static gboolean router_table_print(void *key, void *val, void *user_data)
{
    int k = GPOINTER_TO_INT(key);
    struct router_node *node = (struct router_node *)val;

    struct vty *vty = (struct vty *)user_data;

    int fd = g_io_channel_unix_get_fd(node->fs->io);
    gdouble uptime = g_timer_elapsed(node->last_update, NULL);
    vty_out(vty, "Route key:%d, node->fs->io->fd:%d, name:%s, uptime:%.02fs%s", k, fd, node->user_name, uptime, VTY_NEWLINE);

    return TRUE;
}

DEFUN (config_router_table_print, 
        router_table_print_cmd, 
        "router print", 
        _ROUTER_STR
        "print the router table\n")
{
    g_hash_table_foreach(router_table, (GHFunc)router_table_print, vty);

    return CMD_SUCCESS;
}

DEFUN (config_router_table_clear, 
        router_table_clear_cmd, 
        "router clear", 
        _ROUTER_STR
        "clear the router table\n")
{
    guint len = 0, i =0;
    void **fds = g_hash_table_get_keys_as_array(router_table, &len);

    for(i=0; i<len; i++)
    {
        vty_out(vty, "fd len:%d, i:%d, fd:%d%s", len, i, GPOINTER_TO_INT(fds[i]), VTY_NEWLINE);
        event_pool_dettach(GPOINTER_TO_INT(fds[i]));
    }

    g_free(fds);

    return CMD_SUCCESS;
}

DEFUN (config_router_table_del, 
        router_table_del_cmd, 
        "router del KEY", 
        _ROUTER_STR
        "del the route by a key\n")
{
    if(argc == 1)
    {
        int key = strtol(argv[0], NULL, 10);
        event_pool_dettach(key);
    }

   return CMD_SUCCESS;
}

#define _LOG_STR "log enable/disable/print cmd\n"
DEFUN (config_log_enable, 
        config_log_enable_cmd, 
        "log enable LOG_TYPE", 
        _LOG_STR
        "enable a log type.\n")
{
    if(argc == 1)
    {
        guint32 i = 0;
        const char *type = argv[0];
        for(i=0; i<log_get_type_max(); i++)
            if(strncmp(type, log2name(i), LOG_TYPES_NAME_MAX) == 0)
            {
                log_enable(i);
                break;
            }

        if(i == log_get_type_max())
            vty_out(vty, "No such type:%s%s", type, VTY_NEWLINE);
    }

   return CMD_SUCCESS;
}

DEFUN (config_log_disable, 
        config_log_disable_cmd, 
        "log disable LOG_TYPE", 
        _LOG_STR
        "disable a log type.\n")
{
    if(argc == 1)
    {
        guint32 i = 0;
        const char *type = argv[0];
        for(i=0; i<log_get_type_max(); i++)
            if(strncmp(type, log2name(i), LOG_TYPES_NAME_MAX) == 0)
            {
                log_disable(i);
                break;
            }

        if(i == log_get_type_max())
            vty_out(vty, "No such type:%s%s", type, VTY_NEWLINE);
    }

   return CMD_SUCCESS;
}

DEFUN (config_log_show, 
        config_log_show_cmd, 
        "log show [TYPE]", 
        _LOG_STR
        "show all log type.\n"
        "show a log type")
{
    guint32 i = 0;

    if(argc == 1)
    {
        const char *type = argv[0];
        for(i=0; i<log_get_type_max(); i++)
            if(strncmp(type, log2name(i), LOG_TYPES_NAME_MAX) == 0)
            {
                vty_out(vty, "[%s]: %s%s", type, is_log_type_enabled(i)?"Enabled":"Disabled", VTY_NEWLINE);
                break;
            }

        if(i == log_get_type_max())
            vty_out(vty, "No such type:%s%s", type, VTY_NEWLINE);
        
    }
    else
    {
        for(i=0; i<log_get_type_max(); i++)
            vty_out(vty, "[%s]: %s%s", log2name(i), is_log_type_enabled(i)?"Enabled":"Disabled", VTY_NEWLINE);
    }

   return CMD_SUCCESS;
}

DEFUN (server_stop, server_stop_cmd, "server stop", "shutdown the server")
{
    server_exit();

    return CMD_SUCCESS;
}

DEFUN (select_language, select_language_cmd, 
                        "set language (CN|EN)", 
                        "set the system params\n"
                        "switch the language\n"
                        "- chinese\n"
                        "- english\n"
                        )
{
    LANG_TYPE lang;

    if(strncmp(argv[0], "CN", 2) == 0)
        lang = LANG_CN;
    else if(strncmp(argv[0], "EN", 2) == 0)
        lang = LANG_EN;
    else
        vty_out(vty, "Invalid language type%s", VTY_NEWLINE);

    translate_init(lang);


    return CMD_SUCCESS;
}

void router_cmd_init()
{
    install_element(CONFIG_NODE, &router_table_print_cmd);
    install_element(CONFIG_NODE, &router_table_clear_cmd);
    install_element(CONFIG_NODE, &router_table_del_cmd);
    install_element(CONFIG_NODE, &config_log_enable_cmd);
    install_element(CONFIG_NODE, &config_log_disable_cmd);
    install_element(CONFIG_NODE, &config_log_show_cmd);
    install_element(CONFIG_NODE, &server_stop_cmd);
    install_element(CONFIG_NODE, &select_language_cmd);
}


