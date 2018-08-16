#!/bin/sh
#
#
#        +--------------------------------------------------+
#        |                                                  |
#        | Project  : Ion Linux (tm) Project Build Script   |
#        | Developer: John Buswell <johnb@ionlinux.com>     |
#        | Created  : December 12th 2003                    |
#        |                                                  |
#        +--------------------------------------------------+
#        |                                                  |
#        | Updates:                                         |
#        |                                                  |
#        |    13-Dec-2003 - Added base projects             |
#        |                                                  |
#        +--------------------------------------------------+
#        |                                                  |
#        | This script is designed to generate project      |
#        | builds based off either Ion Linux (tm) Core or   |
#        | Ion Linux (tm) Tachyon. This build system has    |
#        | been designed to permit easy transfer of         |
#        | features between projects.                       |
#        |                                                  |
#        | All projects are prefixed with ION_PRJ_ and all  |
#        | features are prefixed with ION_FEA_. This build  |
#        | system is designed to allow us to generate       |
#        | custom builds for specific customers without a   |
#        | lot of extra work. For customer builds, we use a |
#        | special prefix ION_CCB_ (customer custom build). |
#        |                                                  |
#        | The build system also supports OEM builds.       |
#        | OEM builds are prefixed with ION_OEM_<ID>_.      |
#        | <ID> is the OEM identifier, this allows us to    |
#        | have multiple builds per OEM, and keep things a  |
#        | little organized.                                |
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
#
#
# Parameters:
# ------------
#
# ./build_proj.sh <PROJECT> <BASE> <BASE_VER> <ARCH>
#
# $1 = <PROJECT>  = PROJECT (eg. ION_PRJ_XXXX)
# $2 = <BASE>     = ION_CORE or ION_TACHYON
# $3 = <BASE_VER> = Release Base Version
# $4 = <BASE_VER> = Maintenance Base Version
# $5 = <BASE_VER> = Patch Base Version
# $6 = <ARCH>     = Hardware Architecture (i586, i686, x86-64)
# 
# Version Information:
# ---------------------
#
# Spliced Networks LLC uses a standard version numbering system
# across all projects. This system is similar to what is used in
# open source projects.
#
# The version system consists of three groups of numbers:
#
#		release.maintenance.patch
#
# The release number is only incremented for major software releases.
# Major software releases will contain new features, performance
# enhancements and should be different enough to justify customers paying
# for an upgrade :)
#
# The maintenance number is increased after a certain number of patch
# releases have been made. A maintenance increment indicates to the
# customer that a minimum of X patch releases have been made to the
# product, and that the maintenance release has passed certain standard
# QA testing and performance processes. 
#
# The patch release is incremented on a regular basis, indicating fixes,
# software upgrades and security patches. New features should not go into
# patch releases.
#
# New Features:
# --------------
#
# New features can be placed into the build system at any time,
# features maybe placed into the build system because we need something
# internally, or we may place features into the build system to meet a
# customer need or remove a competitors advantage.
#
# Generally new features should ONLY go into release or maintenance builds,
# NEVER NEVER NEVER put new features into a patch release. You can put fixes
# for a new feature into a patch release, but the first time a new feature is
# added it should always go into a maintenance release.
#

clear

ION_SCR_VER=1.0.0

# ION = Base release location
# ION_SRC = Base source location
# ION_BLD = Build location
# ION_FEA = Feature source location
# ION_LOG = Full log file path
# ION_LGF = Log filename
# ION_PRL = Project Release Location

case "$2" in 
 core|CORE|ion_core|ion-core|ION_CORE|ION-CORE)
  ION=/ion/release/$3.$4/$5
  ION_SRC=/ion/src/$3.$4/$5
  ION_BLD=/ion/bld/$3.$4/$1/$6/$5
  ION_FEA=/ion/src/feature
  ION_LGF=ion-$1_$6-$3.$4.$5.log
  ION_LOG=/ion/release/logs/$ION_LGF
  ION_PRL=/ion/release/$3.$4/$1/$6/$5
 ;;

 tachyon|TACHYON|ion_tachyon|ion-tachyon|ION_TACHYON|ION-TACHYON)
  ION=/ion/tachyon/release/$3.$4/$5
  ION_SRC=/ion/tachyon/src/$3.$4/$5
  ION_BLD=/ion/tachyon/bld/$3.$4/$1/$6/$5
  ION_FEA=/ion/tachyon/src/feature
  ION_LGF=ion-$1_$6-$3.$4.$5.log
  ION_LOG=/ion/tachyon/release/logs/$ION_LGF
  ION_PRL=/ion/tachyon/release/$3.$4/$1/$6/$5
 ;;
esac

ION_PROJECT=$1
ION_BASE=$2
ION_VER_REL=$3
ION_VER_MAINT=$4
ION_VER_PATCH=$5
ION_TARGET=$6
ION_PRJ_VER=$ION_VER_REL.$ION_VER_MAINT.$ION_VER_PATCH

echo ""
echo "Ion Linux Project Build Script v$ION_SCR_VER"
echo "Copyright (c) 2002-2003 Spliced Networks LLC"
echo ""
echo "Building $ION_PROJECT from $ION_BASE release ($ION_PRJ_VER)"
echo ""
echo ""
echo "Release build location        $ION"
echo "Feature source directory      $ION_FEA"
echo "Base source directory         $ION_SRC"
echo "Build directory               $ION_BLD"
echo "Project release location      $ION_PRL"
echo ""
echo "Log file                      $ION_LGF"
echo ""

CLI_BUILD_ARGS=""
WUI_BUILD_ARGS=""

# WARNING: ALL FEATURES MUST BE LISTED below and set to 0

ION_FEA_FOO=0

case $ION_PROJECT in

	ION_PRJ_ICB)
		;;

	ION_PRJ_DNS_APPL)
		;;

	ION_PRJ_LDAP_APPL)
		;;

	ION_PRJ_SMTP_APPL)
		;;

	ION_PRJ_SMTP_FW)
		;;

	ION_PRJ_GLB)
		;;

	ION_PRJ_WIRELESS_HOTSPOT)
		;;

	ION_PRJ_WIRELESS_BWM)
		;;

	ION_PRJ_PRV_JBFW)
		ION_FEA_FOO=1
		;;


	*)
		echo ""
		echo "********** FATAL: UNKNOWN PROJECT **********"
		echo ""
		echo "$ION_PROJECT is unknown"
		echo ""
		exit 3
		;;

esac

# now we first need to copy over the base to our release area

mkdir -p $ION_PRL $ION_LOG $ION_BLD
cp -a $ION $ION_PRL

# here we need to generate the build args for the CLI and WebUI (WUI)
# the following routines need to go into the particular feature directory
# copy over the source and append the build script for the feature onto
# the end of the chroot build script in the copied release area.
		
 if [ $ION_FEA_FOO -eq 1 ]; then
    # DO STUFF HERE FOR FEATURE
    ION_FEA_FOO=
    export ION_FEA_FOO
 fi
