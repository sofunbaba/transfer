#ifndef _LOG_H_
#define _LOG_H_

#include "glib.h"

#define LOG_PARSE_VA_ARGS \
    int size = 0; \
    char *p = NULL; \
    va_list ap; \
 \
    /* Determine required size */ \
 \
    va_start(ap, fmt); \
    size = vsnprintf(p, size, fmt, ap); \
    va_end(ap); \
 \
    size++;             /* For '\0' */ \
    char buff[size]; \
    memset(buff, 0, size); \
    p = buff; \
 \
    va_start(ap, fmt); \
    size = vsnprintf(p, size, fmt, ap); \
    va_end(ap);


#define LOG_TYPES_NAME_MAX 50

//print error msg and exit.
#define log_error_handler(level, fmt, ...) do{log_error(level, "[%s:%d]"fmt, __func__, __LINE__, ##__VA_ARGS__); g_error_free(error); error=NULL;}while(0)

//only print warning msg.
#define log_warning_handler(level, fmt, ...) do{log_warning(level, fmt, ##__VA_ARGS__); g_error_free(error); error=NULL;}while(0)

typedef enum {
    LOG_INIT,
    LOG_ACCEPT,
    LOG_ROUTER,
    LOG_SHOW_TIME,
    LOG_EVENT_POOL,
    LOG_PROCESS,
    LOG_TRANSLATE,
    LOG_MAX,
}LOG_TYPE;



gboolean is_log_type_enabled(LOG_TYPE level);
void log_disable(LOG_TYPE level);
void log_enable(LOG_TYPE level);
void log_error(LOG_TYPE level, const char *fmt, ...);
gboolean log_init(GError **error);
void log_message(LOG_TYPE level, const char *fmt, ...);
void log_warning(LOG_TYPE level, const char *fmt, ...);
char *log2name(LOG_TYPE type);
guint32 log_get_type_max();



#endif

