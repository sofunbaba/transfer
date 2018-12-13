noinst_LTLIBRARIES = libzebra.la

include_HEADERS = src/libzebra.h config.h\
				  lib/log.h lib/pqueue.h lib/sockopt.h lib/zassert.h lib/network.h lib/if.h\
				  lib/jhash.h lib/smux.h lib/filter.h lib/queue.h lib/privs.h lib/if_rmap.h\
				  lib/memory.h lib/vty.h lib/libospf.h lib/zclient.h lib/md5.h lib/hash.h\
				  lib/regex-gnu.h lib/stream.h lib/getopt.h lib/thread.h lib/sockunion.h lib/command.h\
				  lib/route_types.h lib/prefix.h lib/routemap.h lib/str.h lib/keychain.h lib/memtypes.h\
				  lib/checksum.h lib/buffer.h lib/plist.h lib/distribute.h lib/table.h lib/vector.h\
				  lib/sigevent.h lib/version.h lib/workqueue.h lib/zebra.h lib/linklist.h zebra/rib.h\
				  zebra/zserv.h fpm/fpm.h src/zebra_router_cmd.h

libzebra_la_SOURCES = lib/agentx.c lib/buffer.c lib/checksum.c lib/command.c lib/daemon.c lib/distribute.c lib/filter.c\
					  lib/getopt1.c lib/getopt.c lib/hash.c lib/if.c lib/if_rmap.c lib/jhash.c lib/keychain.c\
					  lib/linklist.c lib/log.c lib/md5.c lib/memory.c lib/memtypes.c lib/network.c lib/pid_output.c\
					  lib/plist.c lib/pqueue.c lib/prefix.c lib/privs.c lib/regex.c lib/routemap.c lib/sigevent.c\
					  lib/smux.c lib/snmp.c lib/sockopt.c lib/sockunion.c lib/str.c lib/stream.c lib/table.c lib/thread.c\
					  lib/vector.c lib/vty.c lib/workqueue.c lib/zclient.c\
					  src/main.c src/zebra_router_cmd.c

libzebra_la_CFLAGS = -I$(top_srcdir)/src -I$(top_srcdir)/libs/glib-2.58.1/glib 
libzebra_la_CFLAGS += -I -Ifpm -Ilib -Isrc -Izebra -DHAVE_CONFIG_H -DSYSCONFDIR=\"/etc/config/\"
libzebra_la_CFLAGS += -I$(top_srcdir)/libs/glib-2.58.1/gmodule -I$(top_srcdir)/libs/glib-2.58.1/

libzebra_la_LDFLAGS = -L$(top_srcdir)/libs/glib-2.58.1/glib -L$(top_srcdir)/libs/glib-2.58.1/gmodule
libzebra_la_LIBADD   = -lm -lpthread -ldl -lcrypt -lglib-2.0 -lgmodule-2.0

