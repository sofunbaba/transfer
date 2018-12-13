bin_PROGRAMS=transfer client

JANSSON_CFLAG  = -I$(top_srcdir)/libs/jansson-2.11/src
JANSSON_LDFLAG = -L$(top_srcdir)/libs/jansson-2.11/src
JANSSON_LIB   = -ljansson

# CCAN_CFLAG  = -I$(top_srcdir)/libs/ccan
# CCAN_LDFLAG = -L$(top_srcdir)/libs/ccan
# CCAN_LIB    = -lccan

# LIBEVENT_CFLAG  = -I$(top_srcdir)/libs/libevent-2.1.8/include
# LIBEVENT_LDFLAG = -L$(top_srcdir)/libs/libevent-2.1.8
# LIBEVENT_LIB   = -levent

# LIBOPENSSL_CFLAG  = -I$(top_srcdir)/libs/openssl-1.1.1/include
# LIBOPENSSL_LDFLAG = -L$(top_srcdir)/libs/openssl-1.1.1/
# LIBOPENSSL_LIB   = -lssl -lcrypto

# ZLIB_CFLAG  = -I$(top_srcdir)/libs/zlib-1.2.11/
# ZLIB_LDFLAG = -L$(top_srcdir)/libs/zlib-1.2.11/
# ZLIB_LIB   = -lz

# NCURSES_CFLAG  = -I$(top_srcdir)/libs/ncurses-6.1/include
# NCURSES_LDFLAG = -L$(top_srcdir)/libs/ncurses-6.1/lib
# NCURSES_LIB   = -lncurses -lpanel

ZEBRA_CFLAG  = -I$(top_srcdir)/libs/zebra/src
ZEBRA_LDFLAG = -L$(top_srcdir)/libs/zebra
ZEBRA_LIB   = -lzebra

GLIB_CFLAG  = -I$(top_srcdir)/libs/glib-2.58.1/ -I$(top_srcdir)/libs/glib-2.58.1/glib -I$(top_srcdir)/libs/glib-2.58.1/gmodule
GLIB_LDFLAG = -L$(top_srcdir)/libs/glib-2.58.1/glib -L/$(top_srcdir)/libs/glib-2.58.1/gmodule
GLIB_LIB   = -lglib-2.0 -lgmodule-2.0

CFLAGS=-g -O0
LDFLAGS=
transfer_CFLAGS  = $(JANSSON_CFLAG)  $(CCAN_CFLAG)  $(LIBEVENT_CFLAG) $(LIBOPENSSL_CFLAG) $(ZLIB_CFLAG) $(NCURSES_CFLAG) $(ZEBRA_CFLAG) $(GLIB_CFLAG)
transfer_LDFLAGS = $(JANSSON_LDFLAG) $(CCAN_LDFLAG) $(LIBEVENT_LDFLAG) $(LIBOPENSSL_LDFLAG) $(ZLIB_LDFLAG) $(NCURSES_LDFLAG) -no-install $(ZEBRA_LDFLAG) $(GLIB_LDFLAG)
transfer_LDADD   = $(JANSSON_LIB)   $(CCAN_LIB)   $(LIBEVENT_LIB) $(LIBOPENSSL_LIB) $(ZLIB_LIB) $(NCURSES_LIB) $(ZEBRA_LIB) $(GLIB_LIB)

transfer_SOURCES = main.c router.c log.c event_pool.c process.c

client_CFLAGS  = $(transfer_CFLAGS)
client_LDFLAGS = $(transfer_LDFLAGS)
client_LDADD   = $(transfer_LDADD)

client_SOURCES = client_src/main.c

.PHONEY: tags

tags:
	ctags-exuberant -R *

