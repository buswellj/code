#!/bin/bash
#
#        +--------------------------------------------------+
#        |                                                  |
#        | Project  : Ion Linux chroot test script          |
#        | Developer: John Buswell <johnb@ionlinux.com>     |
#        | Created  : August 6th 2003                       |
#        |                                                  |
#        +--------------------------------------------------+
#        |                                                  |
#        | Updates  :                                       |
#        |                                                  |
#        |                                                  |
#        +--------------------------------------------------+
#        |                                                  |
#        | Copyright (c) 2002-2003 Spliced Networks         |
#        | All Rights Reserved.                             |
#        |                                                  |
#        | This script is property of Spliced Networks.     |
#        | This script may not be copied or distributed.    |
#        |                                                  |
#        +--------------------------------------------------+
#


        chroot /ion/release/$1.$2/$3 /usr/bin/env -i \
         HOME=/root TERM=$TERM PS1='\u:\w\$ ' \
         PATH=/bin:/usr/bin:/sbin:/usr/sbin \
         /bin/bash --login

