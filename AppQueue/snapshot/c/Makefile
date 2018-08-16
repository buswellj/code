CC      = gcc
CFLAGS  = -Wall -D_GNU_SOURCE -fPIC -Iinclude/ -I/kaos/core/curl/include
LDFLAGS = -Wl,--rpath=/lib -L/kaos/core/curl/lib -lcurl -lcrypt

PROGS   = appqueue

all:            $(PROGS)
		strip --strip-all $(PROGS)

appqueue:       appqueue.o appqueue_install.o appqueue_cli.o appqueue_cli_storage.o appqueue_cli_fw.o appqueue_shm.o

appqueue.o:     appqueue.c include/appqueue.h
		$(CC) -c $(CFLAGS) appqueue.c

appqueue_install.o:     appqueue_install.c include/appqueue.h
			$(CC) -c $(CFLAGS) appqueue_install.c

appqueue_cli.o:         appqueue_cli.c include/appqueue.h
			$(CC) -c $(CFLAGS) appqueue_cli.c

appqueue_cli_storage.o: appqueue_cli_storage.c include/appqueue.h
			$(CC) -c $(CFLAGS) appqueue_cli_storage.c

appqueue_cli_fw.o:      appqueue_cli_fw.c include/appqueue.h
			$(CC) -c $(CFLAGS) appqueue_cli_fw.c

appqueue_shm.o:         appqueue_shm.c include/appqueue.h
			$(CC) -c $(CFLAGS) appqueue_shm.c

clean:
			rm -rf *.o *.bak
			rm -rf $(PROGS)
