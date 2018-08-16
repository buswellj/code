#!/bin/sh
#

echo "Formating the CDRW in 0,0,0 ... "
 cdrecord -v -speed 12 -blank=fast -dev 0,0,0
echo " [ done ]"
