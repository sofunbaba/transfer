#include "process.h"
#include "router.h"
#include "log.h"
#include "jansson.h"
#include <errno.h>

#define PROCESS_RET_MSG_MAX 200

static gsize process_write(int fd, char *fmt, ...)
{
    LOG_PARSE_VA_ARGS
    return write(fd, buff, size);
}

static void process_send_return(int fd, int id, char *err_msg)
{
    json_t *root = json_object();

    json_t *cmd_t = json_integer(PROCESS_CMD_RET);
    json_object_set(root, "cmd", cmd_t);

    json_t *id_t = json_integer(id);
    json_object_set(root, "id", id_t);

    json_t *error_t = NULL;
    if(err_msg == NULL)
        error_t = json_null();
    else
        error_t = json_string(err_msg);
    json_object_set(root, "error", error_t);

    char *str = json_dumps(root, JSON_ENCODE_ANY);

    if(process_write(fd,"%s\r\n", str) < 0)
    {
        char *ret = g_alloca(PROCESS_RET_MSG_MAX);
        strerror_r(errno, ret, PROCESS_RET_MSG_MAX);
        log_message(LOG_PROCESS, "[%s:%d]:%s", __func__, __LINE__, ret);
    }

    g_free(str);
    json_decref(cmd_t);
    json_decref(id_t);
    json_decref(error_t);
    json_decref(root);
}

static void process_cmd_register(struct router_node *node, int id, json_t *param_t)
{
    g_return_if_fail(param_t != NULL);
    g_return_if_fail(node != NULL);

    int fd = g_io_channel_unix_get_fd(node->fs->io);

    if(node->active == true)
        return process_send_return(fd, id, _("Have been registe."));

    json_t *name_t = json_array_get(param_t, 0);
    if(name_t == NULL)
        return process_send_return(fd, id, _("Invalid user name."));

    const char *name = json_string_value(name_t);

    if(register_table_key_exist((void*)name))
        return process_send_return(fd, id, _("The user name has been registed."));

    log_message(LOG_PROCESS, "Register fd:%d, name:%s", fd, name);

    strncpy(node->user_name, name, ROUTER_USER_NAME_MAX);
    node->active = true;
    register_table_add(node->user_name, GINT_TO_POINTER(fd));

    return process_send_return(fd, id, NULL);
}

static void process_cmd_send(struct router_node *node, int id, json_t *param_t)
{
    g_return_if_fail(param_t != NULL);

    int fd = g_io_channel_unix_get_fd(node->fs->io);

    if(node->active == false)
        return process_send_return(fd, id, _("Have not been registed."));

    json_t *dest_name_t = json_array_get(param_t, 0);
    const char *dest_name = json_string_value(dest_name_t);

    int dest_fd = GPOINTER_TO_INT(register_table_find_by_key((void*)dest_name));
    log_message(LOG_PROCESS, "send to %d", dest_fd);
    if(dest_fd == 0)
        return process_send_return(fd, id, _("The target user is not online."));

    json_t *msg_t = json_array_get(param_t, 1);
    if(msg_t == NULL)
        return process_send_return(fd, id, _("Invalid message."));
    const char *msg = json_string_value(msg_t);

    /* build json msg */
    json_t *root = json_object();

    json_t *cmd_t = json_integer(PROCESS_CMD_SEND);
    json_object_set(root, "cmd", cmd_t);

    json_t *from_t = json_string(node->user_name);
    json_object_set(root, "from", from_t);

    json_t *array_t = json_array();
    msg_t = json_string(msg);
    json_array_append(array_t, msg_t);
    json_object_set(root, "params", array_t);

    char *str = json_dumps(root, JSON_ENCODE_ANY);

    char *ret = NULL;
    if(process_write(dest_fd,"%s\r\n", str) < 0)
    {
        ret = g_alloca(PROCESS_RET_MSG_MAX);
        strerror_r(errno, ret, PROCESS_RET_MSG_MAX);
        log_message(LOG_PROCESS, "[%s:%d]:%s", __func__, __LINE__, ret);
        strncpy(ret, _("Failed to send."), PROCESS_RET_MSG_MAX);
    }

    g_free(str);
    json_array_clear(array_t);
    json_object_clear(root);

    json_decref(cmd_t);
    json_decref(from_t);
    json_decref(msg_t);
    json_decref(array_t);
    json_decref(root);

    return process_send_return(fd, id, ret);
}

static void process_cmd_list(struct router_node *node, int id, json_t *param_t)
{
    int fd = g_io_channel_unix_get_fd(node->fs->io);

    if(node->active == false)
        return process_send_return(fd, id, _("Have not been registed."));

    guint online_len = 0, i = 0;
    char **online_list = (char **)register_table_get_keys_as_array(&online_len);
    log_message(LOG_PROCESS, "online_len:%d", online_len);

    /* build json msg */
    json_t *root = json_object();

    json_t *cmd_t = json_integer(PROCESS_CMD_LIST);
    json_object_set(root, "cmd", cmd_t);

    json_t *array_t = json_array();
    json_t *online_t[online_len];
    for(i=0; i < online_len; i++)
    {
        log_message(LOG_PROCESS, "[%d]-> %s", i, online_list[i]);
        online_t[i] = json_string(online_list[i]);
        json_array_append(array_t, online_t[i]);
    }

    json_t *error_t = json_null();
    json_object_set(root, "error", error_t);

    json_object_set(root, "params", array_t);

    char *str = json_dumps(root, JSON_ENCODE_ANY);

    log_message(LOG_PROCESS, "list str:%s", str);

    int dest_fd = g_io_channel_unix_get_fd(node->fs->io);
    if(process_write(dest_fd,"%s\r\n", str) < 0)
    {
        char *ret = g_alloca(PROCESS_RET_MSG_MAX);
        strerror_r(errno, ret, PROCESS_RET_MSG_MAX);
        log_message(LOG_PROCESS, "[%s:%d]:%s", __func__, __LINE__, ret);
    }

    g_free(online_list);
    g_free(str);
    json_array_clear(array_t);
    json_object_clear(root);

    json_decref(cmd_t);
    for(i=0; i < online_len; i++)
        json_decref(online_t[i]);
        
    json_decref(error_t);
    json_decref(array_t);
    json_decref(root);
}

void process_dispatcher(GIOChannel *io, char *data)
{
    // log_enable(LOG_PROCESS);

    log_message(LOG_PROCESS, "Recv:%s", data);

    int fd = g_io_channel_unix_get_fd(io);

    struct router_node *node = router_find_node_by_key(GINT_TO_POINTER(fd));
    if(node == NULL)
        return;

    json_t *root = NULL;
    json_error_t jerr;

    root = json_loads(data, 0, &jerr);
    if(root == NULL)
    {
        log_message(LOG_PROCESS, "dispatcher json loads failed:%s, %d", 
                jerr.text, jerr.line);
        return;
    }

    json_t *id_t = json_object_get(root, "id");
    if(id_t == NULL)
        goto _OUT;

    int id = json_integer_value(id_t);

    json_t *param_t = json_object_get(root, "params");

    json_t *cmd_t = json_object_get(root, "cmd");
    int cmd = json_integer_value(cmd_t);

    switch(cmd)
    {
        case PROCESS_CMD_REGISTER:
            process_cmd_register(node, id, param_t);
            break;
        case PROCESS_CMD_SEND:
            process_cmd_send(node, id, param_t);
            break;
        case PROCESS_CMD_LIST:
            process_cmd_list(node, id, param_t);
            break;
    }

_OUT:
    json_decref(root);
}
