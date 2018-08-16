CC      = gcc
CFLAGS  = -Wall -D_GNU_SOURCE -fPIC -Iinclude/
LDFLAGS = -Wl,--rpath=/lib -L/kaos/core/sqlite/lib -lsqlite3 -lrt

PROGS	= appmgr

all:		$(PROGS)
		strip --strip-all $(PROGS)
		mv $(PROGS) appmgr.aq

appmgr:		appmgr.o appmgr_shm.o appmgr_sql.o

appmgr_shm.o:	appmgr_shm.c
		$(CC) -c $(CFLAGS) appmgr_shm.c

appmgr_sql.o:	appmgr_sql.c
		$(CC) -c $(CFLAGS) appmgr_sql.c

appmgr.o:	appmgr.c
		$(CC) -c $(CFLAGS) appmgr.c

clean:
		rm -rf *.o *.bak
		rm -rf $(PROGS)
		rm -rf appmgr.aq
