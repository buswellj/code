#!/bin/bash

# Ion Linux CD Build script
#
# Authors:	Frank Boyd	(fb@ionlinux.com)
#
# Created:	2003-08-20	Ion CD build script
#
# Modified:	2003-08-20	- 
#
# directory layout is as follows :
#
# /ion				master directory
# /ion/release			release directory
# /ion/release/X.Y		version X.Y build
# /ion/release/X.Y/Z		complete version of ION
# /ion/iso			iso directory
# /ion/installer		Ion Installer
#
#	X		Release (Major) version
#	Y		Maintenance (Minor) version
#	Z		Patch version

source /ion/installer/buildcd.rc

# A function to display usuage, less lines of code ;)
function_usage ()
{
 echo
 echo " Usage: buildcd.sh X Y Z"
 echo "	   ex: buildcd.sh 0 1 9"
 echo "        This will build distro.tar.gz based from"
 echo "        /ion/release/0.1/9"
 echo
}

# Check to make sure the cmdline has $1 $2 $3
if [ -z $X ]; then
 echo; echo " Major(X) variable is empty"
 function_usage
 exit
else
 if [ -z $Y ]; then
  echo; echo " Minor(Y) variable is empty"
  function_usage
  exit
 else
  if [ -z $Z ]; then
   echo; echo " Patch(Z) variable is empty"
   function_usage
   exit
  fi
 fi
fi

# Verify that /ion/release/X.Y/Z exists 
if [ ! -e $ION ]; then
 echo
 echo " Its not there!"
 echo " Please make sure $ION is there, it appears it isn't built yet!"
 echo
 exit
fi

# Run $ION_INSTALLER/scripts/build-all.sh
echo " Running the Ion Installer Build script on $ION ..."
$ION_INSTALLER/scripts/build-all.sh $X $Y $Z

# En, ION should be built now....  
