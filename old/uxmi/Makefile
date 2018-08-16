CC=gcc
OBJECTS=uxmi.o uxmi-halt.o uxmi-maint.o uxmi-multiuser.o uxmi-reboot.o \
	uxmi-sysinit.o uxmi-upgrade.o uxmi-cfg.o
INCLUDES=includes/
CFLAGS=-Wall -s -g -ansi -I$(INCLUDES)

all: clean uxmi

uxmi: $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o uxmi

%.o : %.c
	$(CC) $(CFLAGS) -c $<

clean:

	rm -f uxmi *.o
