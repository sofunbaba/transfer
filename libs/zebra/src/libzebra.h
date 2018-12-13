#ifndef _ZEBRA_H_
#define _ZEBRA_H_


void *zebra_main(void *arg);

struct zebra_param {
    const char *host;
    const char *passwd;
    const char *en_passwd;
    unsigned int port;
};




























#endif

