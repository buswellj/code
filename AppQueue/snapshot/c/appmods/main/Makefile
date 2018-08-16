CC      = gcc
CFLAGS  = -Wall -D_GNU_SOURCE -fPIC -Iinclude/
LDFLAGS = -Wl,--rpath=/lib -lrt

PROGS	= appmod

all:		$(PROGS)
		strip --strip-all $(PROGS)
		mv $(PROGS) climod.aq

appmod:		appmod.o appmod_cli_init.o appmod_cli.o appmod_cli_mf.o appmod_cli_api.o appmod_cli_shm.o

appmod.o:	appmod.c
		$(CC) -c $(CFLAGS) appmod.c

appmod_cli_shm.o:	appmod_cli_shm.c
			$(CC) -c $(CFLAGS) appmod_cli_shm.c

appmod_cli_api.o:  appmod_cli_api.c
			$(CC) -c $(CFLAGS) appmod_cli_api.c

appmod_cli_mf.o:   appmod_cli_mf.c
			$(CC) -c $(CFLAGS) appmod_cli_mf.c

appmod_cli_init.o: appmod_cli_init.c
			$(CC) -c $(CFLAGS) appmod_cli_init.c

appmod_cli.o:	appmod_cli.c
		$(CC) -c $(CFLAGS) appmod_cli.c

clean:
		rm -rf *.o *.bak
		rm -rf $(PROGS)
		rm -rf climod.aq
