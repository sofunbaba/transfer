#ifndef _SERVER_H_
#define _SERVER_H_

#include "glib.h"
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <libintl.h>
#include <locale.h>

typedef enum {
    LANG_CN,
    LANG_EN
}LANG_TYPE;


#define _(STRING) gettext(STRING)

GMainLoop *m_main_loop_new(GMainContext *context);
void server_exit(void);
gboolean translate_init(LANG_TYPE lang);








#endif

