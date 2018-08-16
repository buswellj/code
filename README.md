# code
generic test projects

### ABMS
ABMS is an init replacement to provide an appliance style management system for Linux platforms.
It was designed to work with AppOS to provide a system that could be self-contained and integrated into
the Linux kernel cpio image.



### AppOS-Linux
AppOS is a Hardened Linux Appliance Platform. This is the open sourced build system for it. It was last developed in 2008.
A redesign of the approach for Cloud platforms was done in 2009, the base for that code can be found here under KaOS.
But the latest version of that system can be found in the opaquesystems OpenKaOS repository. Both this, KaOS and OpenKaOS are
built from source code Linux platforms, with no basis from any pre-existing Linux distribution.



### AppQueue
AppQueue was a proof of concept platform that provided a management CLI for virtual appliance VMs that were run inside a
lightweight hypervisor platform based off KaOS and KVM. The code has some interesting examples of how to use shared memory under Linux.



### LIMBS
Linux Image Boot System is an init replacement for lightweight Linux appliances. This was the original proof of concept code that ABMS is built off.



### KaOS
Kernel Attached Operating System is a from source Linux build system that produces a chroot environment and subsequent cpio image that can be embedded
into the Linux kernel so that you have an all-in-one platform in a single bzImage file. This code was the original proof of concept for building an
embedded Linux platform within the kernel.



### kattach
Kattach is an init replacement that was designed for KaOS and was a from-scratch redesign / rewrite. It follows on from the lessons learned with
LIMBS and ABMS. It was updated to support an SQLite3 database and was a proof of concept code base.



### linux-os-build-old/
This contains some original build scripts that were proof of concept for building a from scratch Linux platform.


### o3-mag/
This contains o3 magazine website and old PDFs. o3 magazine was a popular open source FREE PDF publication that I produced and wrote articles for using
open source software.


### old/
This contains some really old projects and proof of concept code.

### router-cfg/
This contains lab and sample router configuration files for some Cisco, Acme Packet and Alteon configurations I have done in the past. These deployments no
longer exist.

### scripts/
Some old scripts for building / setup apache / dns etc.


###test-code/
Lots of simple C code and other examples. These were things I threw together to verify how code behaved before putting it into embedded firmware images.
This code is useful for seeing how things work / behave etc.

### tuxppc/
This is the really old TuxPPC project (Linux on the PowerPC) website.



