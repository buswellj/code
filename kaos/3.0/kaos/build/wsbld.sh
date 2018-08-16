#!/bin/bash
#
# Open Kernel Attached Operating System (OpenKaOS)
# Platform Build System version 3.0.0
#
# Copyright (c) 2009-2014 Opaque Systems LLC
#
# script : wsbld.sh
# purpose: create a new user workspace
#
#

PBHERE=`pwd`
cd ~
PBHOME=`pwd`
cd $PBHERE
PBVER="1.0.0"
PBUSER=`whoami`
PBNOW=`date +%s`
PBTAG="kaos-$PBNOW"
PBWS="$PBHOME/kaos-ws/$PBTAG"

export PBHERE PBHOME PBVER PBUSER PBTAG PBWS

echo ""
echo "KaOS Platform Build System, version $PBVER"
echo "Copyright (c) 2009-2014 Opaque Systems LLC"
echo ""
echo ""
echo "Building workspace for $PBUSER in $PBWS"
echo ""
echo "Build tag is $PBTAG"
echo ""

mkdir -p $PBWS
echo "The active workspace is now $PBWS"
echo $PBTAG > $PBHOME/kaos-ws/.current
echo ""

