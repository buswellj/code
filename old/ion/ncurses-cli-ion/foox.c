#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CVT_DEC 0
#define CVT_HEX	1
#define CVT_OCT 2

void num_to_string(int num_conv, int cvt_type, char *str_tmp);

void
num_to_string(int num_conv, int cvt_type, char *str_tmp)
{
    switch(cvt_type) {

	case CVT_OCT:
	    sprintf(str_tmp,"%o",num_conv);
	    break;

	case CVT_HEX:
	    sprintf(str_tmp,"%x",num_conv);
	    break;

	case CVT_DEC:
	default:
	    sprintf(str_tmp,"%d",num_conv);
	    break;
    }

}

/* this routine shows the use of the above function */

void
main (void)
{
    int x = 114473;
    char *c, *c_ptr;

    c_ptr = c;		/* set a pointer to the string */

    num_to_string(x,CVT_DEC, c_ptr);
    printf("\n\nNumber is %d converted to DECIMAL is %s\n\n",x,c);
    num_to_string(x,CVT_HEX, c_ptr);
    printf("Number is %d converted to HEX is %s\n\n",x,c);
    num_to_string(x,CVT_OCT, c_ptr);
    printf("Number is %d converted to OCT is %s\n\n",x,c);

    exit(0);
}
