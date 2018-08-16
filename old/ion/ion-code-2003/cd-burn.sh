#!/bin/bash
#

source /ion/installer/buildcd.rc

# A function to display usuage, less lines of code ;)
function_usage ()
{
 echo
 echo " Usage: cd-burn X Y Z S"
 echo "    ex: cd-burn 0 1 9 4"
 echo "        This will build $ION_ISO/ion-$X.$Y.$Z.iso at 4x"
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
  else
   if [ -z $4 ]; then
    echo; echo " Speed(S) variable is empty"
    function_usage
    exit
   fi
  fi
 fi
fi


echo " Burning $ION_ISO/ion-$X.$Y.$Z.iso to CD at $4x"
 cdrecord -v speed=$4 dev=0,0,0 -data $ION_ISO/ion-$X.$Y.$Z.iso
echo " [ CD Creation Complete ]"
