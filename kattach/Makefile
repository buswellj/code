CC = gcc
ifdef KATTACH_IANA
CFLAGS = -Wall -D_GNU_SOURCE -DKATTACH_IANA -fPIC -Iinclude/
VCFLAGS = -Wall -D_GNU_SOURCE -DKATTACH_BLD_VKAOS -DKATTACH_IANA -fPIC -Iinclude/
else
CFLAGS = -Wall -D_GNU_SOURCE -fPIC -Iinclude/
VCFLAGS = -Wall -D_GNU_SOURCE -DKATTACH_BLD_VKAOS -fPIC -Iinclude/
endif
LDFLAGS = -Wl,--rpath=/lib -L/kaos/core/sqlite/lib -lsqlite3 -lm -lrt -lcrypt

PROGS = kattach
VPROGS = vkattach

ifdef KATTACHIANA
all:    $(PROGS) $(VPROGS)
	strip --strip-all $(PROGS) $(VPROGS)
else
all:    $(PROGS) $(VPROGS)
	strip --strip-all $(PROGS) $(VPROGS)
endif

ifdef KATTACHIANA
kattach: kattach.o kattach-api.o kattach-cfg.o kattach-sys.o kattach-net.o kattach-sql.o kattach-loop.o kattach-vm.o thedawn.o kattach-vkaos.o kattach-iana.o
else
kattach: kattach.o kattach-api.o kattach-cfg.o kattach-sys.o kattach-net.o kattach-sql.o kattach-loop.o kattach-vm.o thedawn.o kattach-vkaos.o
endif

kattach.o:      kattach.c include/kattach.h
		$(CC) -c $(CFLAGS) kattach.c

thedawn.o:      thedawn.c include/kattach.h
		$(CC) -c $(CFLAGS) thedawn.c

kattach-api.o:  kattach-api.c include/kattach.h
		$(CC) -c $(CFLAGS) kattach-api.c

kattach-cfg.o:  kattach-cfg.c include/kattach.h
		$(CC) -c $(CFLAGS) kattach-cfg.c

kattach-sys.o:  kattach-sys.c include/kattach.h
		$(CC) -c $(CFLAGS) kattach-sys.c

kattach-net.o:  kattach-net.c include/kattach.h
		$(CC) -c $(CFLAGS) kattach-net.c

kattach-sql.o:  kattach-sql.c include/kattach.h
		$(CC) -c $(CFLAGS) kattach-sql.c

kattach-loop.o: kattach-loop.c include/kattach.h
		$(CC) -c $(CFLAGS) kattach-loop.c

kattach-vm.o:   kattach-vm.c include/kattach.h
		$(CC) -c $(CFLAGS) kattach-vm.c

kattach-vkaos.o:   kattach-vkaos.c include/kattach.h
		$(CC) -c $(CFLAGS) kattach-vkaos.c

ifdef KATTACHIANA
kattach-iana.o:   kattach-iana.c include/kattach.h
		$(CC) -c $(CFLAGS) kattach-iana.c
endif

vkaos:	$(VPROGS)
	strip --strip-all $(VPROGS)

vkattach: vkattach.o vkattach-api.o vkattach-cfg.o vkattach-sys.o vkattach-net.o vthedawn.o vkattach-vkaos.o

vkattach.o:      kattach.c include/kattach.h
		$(CC) -c $(VCFLAGS) kattach.c -o vkattach.o

vthedawn.o:      thedawn.c include/kattach.h
		$(CC) -c $(VCFLAGS) thedawn.c -o vthedawn.o

vkattach-api.o:  kattach-api.c include/kattach.h
		$(CC) -c $(VCFLAGS) kattach-api.c -o vkattach-api.o

vkattach-cfg.o:  kattach-cfg.c include/kattach.h
		$(CC) -c $(VCFLAGS) kattach-cfg.c -o vkattach-cfg.o

vkattach-sys.o:  kattach-sys.c include/kattach.h
		$(CC) -c $(VCFLAGS) kattach-sys.c -o vkattach-sys.o

vkattach-net.o:  kattach-net.c include/kattach.h
		$(CC) -c $(VCFLAGS) kattach-net.c -o vkattach-net.o

vkattach-vkaos.o:   kattach-vkaos.c include/kattach.h
		$(CC) -c $(VCFLAGS) kattach-vkaos.c -o vkattach-vkaos.o

iana:
		wget http://www.iana.org/assignments/port-numbers -O port-numbers
		echo "#include <stdio.h>" > kattach-iana.c
		echo "#include <string.h>" >> kattach-iana.c
		echo "#include \"kattach_types.h\"" >> kattach-iana.c
		echo "#include \"kattach.h\"" >> kattach-iana.c
		echo "" >> kattach-iana.c
		echo "void" >> kattach-iana.c
		echo "kattach_iana_ports(void)" >> kattach-iana.c
		echo "{" >> kattach-iana.c
		echo "        FILE *stream;" >> kattach-iana.c
		echo "        char cfgfile[128];" >> kattach-iana.c
		echo "" >> kattach-iana.c
		echo "        memset(cfgfile,0,sizeof(cfgfile));" >> kattach-iana.c
		echo "        sprintf(cfgfile,\"%s/%s\",KATTACH_CFGPATH,KATTACH_CONF_SERVICES);" >> kattach-iana.c
		echo "        stream = fopen(cfgfile,\"w\");" >> kattach-iana.c
		echo "" >> kattach-iana.c
		echo "        if (stream == (FILE *)0) {" >> kattach-iana.c
		echo "                printf(\"\\n [!] FATAL -- Unable to write %s\\n\",cfgfile);" >> kattach-iana.c
		echo "        } else {" >> kattach-iana.c
		echo "" >> kattach-iana.c
		cat port-numbers | grep ^[a-z0-9] | more | grep \/ | sed 's/\/tcp/\/tcp%/g' | sed 's/\/udp/\/udp%/g' | sed 's/\/sctp/\/sctp%/g' | \
		sed 's/\/dccp/\/dccp%/g' | sed 's/\/tdp/\/tdp%/g' | grep -v "the Registered Ports," | cut -f 1 -d % | \
		sed 's/^/\t\tfprintf(stream,\"/g' | sed 's/$$/\\n");/g' >> kattach-iana.c
		echo "                fclose(stream);" >> kattach-iana.c
		echo "        }" >> kattach-iana.c
		echo "        return;" >> kattach-iana.c
		echo "}" >> kattach-iana.c
		rm -rf port-numbers

clean:
		rm -rf kattach-iana.c
		rm -rf *.o *.bak
		rm -rf $(PROGS) $(VPROGS)
