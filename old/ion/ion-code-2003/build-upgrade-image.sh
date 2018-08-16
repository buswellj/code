#!/bin/bash
#
# Ion Linux Upgrade Image Build Script 
# version 1.0
#
# Copyright (C) 2002-2003 Spliced Networks
#
# Authors:      John Buswell    (johnb@splicednetworks.com)
#
# Created:      2003-11-26      Initial Ion upgrade build script
#
#
# parameters:
#
# $1 = new image version
# $2 = path to old image version

rsync -avvz --delete root@localhost::$1 $2
