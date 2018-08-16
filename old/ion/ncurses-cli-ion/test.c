#include <stdio.h>
#include <fcntl.h>
#include <errno.h>

#define ERROR(s)   {system("stty -raw"); fprintf(stderr,"%d-",errno); \
                    perror(s); exit(1);}

main(argc,argv)
int argc;
char *argv[];
{
        int fd, fdo, retval, count=0;
        char c;

        system("stty raw");
        fd = open("/dev/tty", O_RDONLY);
        if(fd < 0) 
                ERROR("open input");

        fdo = open(argv[1], O_CREAT|O_TRUNC|O_WRONLY, 00666);
        if(fdo < 0) 
                ERROR("open output");

        while( retval=read(fd, &c, 1) ) {  /* Read & write data until ^D */
                if(retval < 0) {
                        fprintf(stdout,"read %d bytes\n", count);
                        ERROR("read");
                        }
                if(c == 4) /* ^D */ break;
                count++;
                retval=write( fdo, &c, 1);
                if(retval < 0) {
                        fprintf(stdout,"read %d bytes\n",count);
                        ERROR("write");
                        }
                }
         system("stty -raw");
         fprintf(stderr,"read/wrote %d bytes\n", count);
}


