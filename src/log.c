#include "log.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

static char log_types_name[LOG_MAX][LOG_TYPES_NAME_MAX+1];

#define log_name_add(type) do{snprintf(log_types_name[type], LOG_TYPES_NAME_MAX, "%s", #type);}while(0)

static guint32 g_log_level = 0;
void log_enable(LOG_TYPE level)
{
    g_log_level |= ((guint32)1<<level);
}

void log_disable(LOG_TYPE level)
{
    g_log_level &= ~((guint32)1<<level);
}

void log_message(LOG_TYPE level, const char *fmt, ...)
{
    if(is_log_type_enabled(level))
    {
        LOG_PARSE_VA_ARGS
        // write log to file.
        if(is_log_type_enabled(LOG_SHOW_TIME))
            g_message("[%s]:%s", log_types_name[level], buff);
        else
            printf("[%s]:%s\r\n", log_types_name[level], buff);
    }
}

void log_warning(LOG_TYPE level, const char *fmt, ...)
{
    if(is_log_type_enabled(level))
    {
        LOG_PARSE_VA_ARGS
        // write log to file.
        if(is_log_type_enabled(LOG_SHOW_TIME))
            g_warning("[%s]:%s", log_types_name[level], buff);
        else
            printf("[%s]:%s\r\n", log_types_name[level], buff);
    }
}

void log_error(LOG_TYPE level, const char *fmt, ...)
{
    if(is_log_type_enabled(level))
    {
        LOG_PARSE_VA_ARGS
        // write log to file.
        if(is_log_type_enabled(LOG_SHOW_TIME))
            g_error("[%s]:%s", log_types_name[level], buff);
        else
        {
            printf("[%s]:%s\r\n", log_types_name[level], buff);
            exit(errno);
        }
    }
}

gboolean log_init(GError **error)
{
    log_name_add(LOG_INIT);
    log_name_add(LOG_ACCEPT);
    log_name_add(LOG_ROUTER);
    log_name_add(LOG_SHOW_TIME);
    log_name_add(LOG_EVENT_POOL);
    log_name_add(LOG_PROCESS);
    log_name_add(LOG_TRANSLATE);

    guint64 log_level = 0x00;

    log_enable(LOG_INIT);

    if(error && *error)
        return false;

    g_log_level |= log_level;

    return true;
}

gboolean is_log_type_enabled(LOG_TYPE level)
{
    return (g_log_level & (guint32)1<<level);
}

char *log2name(LOG_TYPE type)
{
    return log_types_name[type];
}

guint32 log_get_type_max()
{
    return LOG_MAX;
}

