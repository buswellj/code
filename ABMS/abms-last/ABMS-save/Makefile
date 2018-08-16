CC      = gcc
CFLAGS  = -Wall -D_GNU_SOURCE -fPIC
LDFLAGS = -Wl
#LDFLAGS = -Wl,--rpath=/app/os/sys/lib

PROGS	= abms

all:		$(PROGS)

abms:		abms.o abms-api.o abms-disp.o abms-init.o abms-proc.o abms-net.o abms-loop.o

abms.o:		abms.c abms.h
		$(CC) -c $(CFLAGS) abms.c

abms-api.o:	abms-api.c abms.h
		$(CC) -c $(CFLAGS) abms-api.c

abms-disp.o:	abms-disp.c abms.h
		$(CC) -c $(CFLAGS) abms-disp.c

abms-init.o:	abms-init.c abms.h
		$(CC) -c $(CFLAGS) abms-init.c

abms-proc.o:	abms-proc.c abms.h
		$(CC) -c $(CFLAGS) abms-proc.c

abms-net.o:	abms-net.c abms.h
		$(CC) -c $(CFLAGS) abms-net.c

abms-loop.o:	abms-loop.c abms.h
		$(CC) -c $(CFLAGS) abms-loop.c

clean:
		rm -f *.o *.bak
		rm -f $(PROGS)


