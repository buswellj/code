/*
 * kattach (kernel attach)
 * Copyright (c) 2009-2010 Carbon Mountain LLC.
 * All Rights Reserved.
 *
 * John Buswell <buswellj@carbonmountain.com>
 * version 0.6.0.0
 *
 * LICENSE: GPL v2
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 *
 * To contact Carbon Mountain LLC please visit http://www.carbonmountain.com
 *
 */

/* begin definitions */
/* Uncomment this to print sys commands instead of executing them */
/* #define KATTACH_DEBUG		1 */

/* Uncomment the line below and define KATTACHIANA in the Makefile for full services support */
/* #define KATTACH_IANA			1 */

/* Uncomment this to print extra debug information when executing commands */
/* #define KATTACH_DEBUG_VERBOSE	1 */

/* kattach release information */
#define KATTACH_VERSION			"0.6.1.0"
#define KATTACH_ARCH			"x86-64"
#define KATTACH_RELEASE			"fcs"
#define KATTACH_COPYRIGHT		"Copyright (c) 2009 - 2010 Carbon Mountain LLC."
#define KATTACH_URL			"http://www.carbonmountain.com"

/* kattach result codes */
#define RC_OK				1
#define RC_FAIL				0

/* kattach environment */
#define KATTACH_CONSOLE			"/dev/console"
#define KATTACH_SHELL			"/kaos/core/busybox/bin/ash"
#define KATTACH_BINPATH			"/kaos/core/busybox/bin/"
#define KATTACH_SBINPATH		"/kaos/core/busybox/sbin/"
#define KATTACH_SSHPATH			"/kaos/core/ssh/bin/"
#define KATTACH_SSHSPATH		"/kaos/core/ssh/sbin/"
#define KATTACH_PROCPATH		"/proc"
#define KATTACH_CFGPATH			"/kaos/cfg"
#define KATTACH_DEVPATH			"/dev"
#define KATTACH_DEVSHMPATH		"/dev/shm"
#define KATTACH_DEVPTSPATH		"/dev/pts"
#define KATTACH_DEVNETPATH		"/dev/net"
#define KATTACH_DEVMISCPATH		"/dev/misc"
#define KATTACH_SSHCFGPATH		"/kaos/cfg/ssh/"
#define KATTACH_SSHKEYPATH		"/kaos/cfg/ssh/keys/"
#define KATTACH_PSSHKEYPATH		"/boot/.dbk/"
#define KATTACH_VARRUNPATH		"/var/run"
#define KATTACH_VARDBPATH		"/var/db"
#define KATTACH_VARLOGPATH		"/var/log"
#define KATTACH_VARLASTLOGPATH		"/var/log/lastlog"
#define KATTACH_SYSPATH			"/sys"
#define KATTACH_SYSNETPATH		"/class/net/"
#define KATTACH_APPQUEUEPATH		"/appq"
#define KATTACH_APPQUEUE_HVIMGPATH	"/appq/hvimg/"				/* Where Hypervisor Images are stored */
#define KATTACH_APPQUEUE_SVCPATH	"/appq/svc/"				/* Where additional Hypervisor Services are Installed */
#define KATTACH_APPQUEUE_SVCCFGPATH	"/appq/cfg/svc/"			/* Where additional Hypervisor Service Configs are located */
#define KATTACH_APPQUEUE_CFGPATH	"/appq/cfg/"				/* Where VM configuration images go */
#define KATTACH_APPQUEUE_IMGPATH	"/appq/images/"				/* Where App Module Images are stored */
#define KATTACH_APPQUEUE_IMGIMPATH	"/appq/images/import/"			/* Where VM images are imported */
#define KATTACH_APPQUEUE_VMDISKS	"/appq/vmdisks/"			/* Where Virtual Machine Disk Images are stored */
#define KATTACH_APPQUEUE_RAWDISKS	"/appq/vmdisks/raw/"			/* Where RAW disk images are stored temporarily */
#define KATTACH_APPQUEUE_RAWMOUNT	"/appq/vmdisks/raw/mnt/"		/* Where RAW disk images are stored temporarily */
#define KATTACH_APPQUEUE_AQIPATH	"/appq/vmdisks/aqi/"			/* Where we temporarily mount disk images */
#define KATTACH_APPQUEUE_VKLPATH	"/appq/vmdisks/raw/mnt/vkaos_launch"	/* vKaOS Launch Control */
#define KATTACH_APPQUEUE_VDIDISKS	"/appq/vmdisks/vdi/"			/* Where VDI disks are stored */
#define KATTACH_APPQUEUE_LOCALDISKS	"/appq/vmdisks/local/"			/* Where imported vmdisks reside */
#define KATTACH_APPQUEUE_BINPATH	"/appq/bin/"
#define KATTACH_APPQUEUE_DBPATH		"/appq/db/"				/* Where the databases are stored */
#define KATTACH_APPQUEUE_HVPATH		"/appq/hv"				/* Hypervisor Tools Path -- symlinked to /kaos/hv */
#define KATTACH_APPQUEUE_DHCPPATH	"/kaos/hv/dhcpd/sbin/"			/* Where DHCPD is located */
#define KATTACH_APPQUEUE_VDIPATH	"/appq/vdi/"				/* Main VDI path */
#define KATTACH_APPQUEUE_VDIMEDIA	"media/"				/* where install media is installed */
#define KATTACH_APPQUEUE_APPMOD		"/appq/am/"				/* main App Module directory */
#define KATTACH_APPQUEUE_APPMODCFG	"/appq/am/cfg/"
#define KATTACH_APPQUEUE_MGRPATH	"/mgr/appmgr.aq"
#define KATTACH_APPQUEUE_VENDOR_CM	"cm"
#define KATTACH_APPQUEUE_VENDOR_LOCAL	"local"
#define KATTACH_DHCPCPATH               "/kaos/core/dhcpcd/sbin/"               /* Where DHCPCD (client) is located */
#define KATTACH_HVPATH			"/kaos/hv/kvm/bin"			/* Where KVM is located */
#define KATTACH_ROOTHOME		"/root"
#define KATTACH_BOOTPATH		"/boot"
#define KATTACH_PANICPATH		"/proc/sys/kernel/panic"
#define KATTACH_APPQUEUE_CLI		"/kaos/core/aq/appqueue"
#define KATTACH_APPQUEUE_CLI_II         "/kaos/core/cmfs/aq/appqueue"
#define KATTACH_SQFSPATH                "/kaos/core/squashfs/bin/"
#define KATTACH_CMFSPATH                "/kaos/core/cmfs/"
#define KATTACH_CMFIPATH                "/kaos/core/cmfi/"


/* compulsory linux devices symlinks */
#define KATTACH_DEVLINK_FD		"/dev/fd"
#define KATTACH_DEVLINK_FD_TGT		"/proc/self/fd"
#define KATTACH_DEVLINK_STDIN		"/dev/stdin"
#define KATTACH_DEVLINK_STDIN_TGT	"/proc/self/fd/0"
#define KATTACH_DEVLINK_STDOUT		"/dev/stdout"
#define KATTACH_DEVLINK_STDOUT_TGT	"/proc/self/fd/1"
#define KATTACH_DEVLINK_STDERR		"/dev/stderr"
#define KATTACH_DEVLINK_STDERR_TGT	"/proc/self/fd/2"

/* kattach configuration files */
#define KATTACH_CONF_DHCPD		"dhcpd.conf"
#define KATTACH_CONF_NSSWITCH		"nsswitch.conf"
#define KATTACH_CONF_GROUP		"group"
#define KATTACH_CONF_PASSWD		"passwd"
#define KATTACH_CONF_SHADOW		"shadow"
#define KATTACH_CONF_RESOLV		"resolv.conf"
#define KATTACH_CONF_RELEASE		"kaos.release"
#define KATTACH_CONF_EXTLINUX		"extlinux.conf"
#define KATTACH_CONF_SHELLS		"shells"
#define KATTACH_CONF_HOSTS		"hosts"
#define KATTACH_CONF_ISSUE		"issue"
#define KATTACH_CONF_SERVICES		"services"
#define KATTACH_CONF_PROTOCOLS		"protocols"
#define KATTACH_CONF_DHCPD_LEASES	"/kaos/cfg/var/db/dhcpd.leases"
#define KATTACH_CONF_DHCPD_PID		"/kaos/cfg/var/run/dhcpd.pid"

/* AppQueue defines */
#define KATTACH_FS_SQUASHFS		"squashfs"
#define KATTACH_FS_APPQUEUE		"ext4"
#define KATTACH_FS_EXT3			"ext2"
#define KATTACH_FS_EXT2			"ext2"
#define KATTACH_FS_SWAP			"swap"


/* kattach filesystem mount points */
#define KATTACH_PROC			"proc"
#define KATTACH_DEVPTS			"devpts"
#define KATTACH_DEVSHM			"shm"
#define KATTACH_TMPFS			"tmpfs"
#define KATTACH_PROCMOUNT		"-vt"
#define KATTACH_SYS			"sys"
#define KATTACH_SYSFS			"sysfs"

/* kattach permissions */
#define KATTACH_PERM			(S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#define KATTACH_PERM_SECURE		(S_IRWXU)
#define KATTACH_PERM_SECURE_RO		(S_IRUSR)
#define KATTACH_PERM_SECURE_GRD		(S_IRWXU | S_IRGRP)
#define KATTACH_PERM_SECURE_GRX		(S_IRWXU | S_IRGRP | S_IXGRP)
#define KATTACH_PERM_GROUP		(S_IRWXU | S_IRGRP | S_IXGRP | S_IWGRP)
#define KATTACH_PERM_PROC		(S_IRUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#define KATTACH_PERM_DEV		(S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#define KATTACH_PERM_SHM		(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)

#define KATTACH_PERM_BLK		(S_IFBLK | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define KATTACH_PERM_W_BLK		(S_IFBLK | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)
#define KATTACH_PERM_CHR		(S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define KATTACH_PERM_W_CHR		(S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

#define KATTACH_UID_ROOT		0
#define KATTACH_GID_APPQ		77

/* kattach fs defaults */
#define KATTACH_FS_DEFAULT		"defaults"
#define KATTACH_FS_DEVPTS		"mode=600"
#define KATTACH_FS_EXT4			"barrier=1,data=ordered"

/* kattach management */
#define KATTACH_SSHPORT			1289
#define KATTACH_PANICTIMER		5
#define KATTACH_RESERVED_LOOPBACK_CMFS	"/dev/loop0"
#define KATTACH_RESERVED_LOOPBACK	"/dev/loop1"
#define KATTACH_RESERVED_LOOPBACK_AQI	"/dev/loop2"

/* kattach commands */
#define KCMD_IPT			"/kaos/core/iptables/sbin/iptables"
#define KCMD_CLEAR			"clear"
#define KCMD_MKDIR			"mkdir"
#define KCMD_MOUNT			"mount"
#define KCMD_MKNOD			"mknod"
#define KCMD_MKSWAP			"mkswap"
#define KCMD_IP				"ip"
#define KCMD_DHCPC			"dhcpcd"
#define KCMD_ECHO			"echo"
#define KCMD_SSHKEYGEN			"dropbearkey"
#define KCMD_SSHD			"dropbear"
#define KCMD_GETTY			"getty"
#define KCMD_BRCTL			"brctl"
#define KCMD_VCONFIG			"vconfig"
#define KCMD_QEMU			"qemu-system-x86_64"
#define KCMD_QEMUIMG			"qemu-img"
#define KCMD_QEMUIO			"qemu-io"
#define KCMD_QEMUNBD			"qemu-nbd"
#define KCMD_DHCPD			"dhcpd"
#define KCMD_HOSTNAME			"hostname"
#define KCMD_DD				"dd"
#define KCMD_LOSETUP			"losetup"
#define KCMD_MKFS			"mkfs.ext4"
#define KCMD_CP				"cp"
#define KCMD_SWAPON			"swapon"
#define KCMD_SWAPOFF			"swapoff"
#define KCMD_EXTLINUX			"extlinux"
#define KCMD_CAT			"cat"
#define KCMD_MKSQUASHFS                 "mksquashfs"
#define KCMD_RM                         "rm"
#define KCMD_MV                         "mv"
#define KCMD_UMOUNT			"umount"
#define KCMD_FSCK			"e2fsck"
#define KCMD_GREP			"grep"
#define KCMD_KILL			"kill"
#define KCMD_NTP			"rdate"
#define KCMD_HWCLOCK			"hwclock"
#define KCMD_RENICE			"renice"


/* kattach modes */
#define KATTACH_MODE_UNKNOWN		0x00
#define KATTACH_MODE_KAOS		0x01		/* KaOS platform */
#define KATTACH_MODE_RECOVERY		0x08		/* KaOS recovery mode -- boots to setup but does not activate vm */
#define KATTACH_MODE_VKAOS		0x09		/* vKaOS platform */
#define KATTACH_MODE_SETUPRECOVERY	0xf1		/* KaOS setup recovery */
#define KATTACH_MODE_RESERVED		0xfd		/* reserved mode */
#define KATTACH_MODE_AUTOINSTALL	0xfe		/* automatic installation */
#define KATTACH_MODE_NOTCFG		0xff		/* first time boot */

/* kattach networking */
#define KATTACH_NET_MAXSLASH		31
#define KATTACH_NET_MINSLASH		1
#define KATTACH_NET_RESOLVCONF		"/etc/resolv.conf"
#define KATTACH_NET_RSAKEY		"rsa.key"
#define KATTACH_NET_DSSKEY		"dss.key"
#define KATTACH_NET_SCANIP		0x7FFEED7F
#define KATTACH_NET_SCANSLASH		24
#define KATTACH_NET_MACPREFIX		"00:1e:6c"
#define KATTACH_NET_MAGIC               "2656d6d612b69616e1"
#define KATTACH_NET_SSHTIMEOUT		600		/* sshd idle timeout in seconds */
#define KATTACH_NET_HASDHCPIP		0xfffefdfc	/* special ip value to designate dhcp */

#define KATTACH_NET_VLAN_NOTSET		0
#define KATTACH_NET_VLAN_LOCAL		1
#define KATTACH_NET_VLAN_8021Q		2
#define KATTACH_NET_VLAN_ROUTED		3
#define KATTACH_NET_VLAN_NAT		4

#define KATTACH_VBR_STATE_NEW		0x00
#define KATTACH_VBR_STATE_ACTIVE	0x01
#define KATTACH_VBR_STATE_MODIFIED	0x02
#define KATTACH_VBR_STATE_DELETED	0x03
#define KATTACH_VBR_STATE_DISABLED	0x04
#define KATTACH_VBR_STATE_STP_FORWARD	0x05
#define KATTACH_VBR_STATE_STP_BLOCKING	0x06
#define KATTACH_VBR_STATE_INACTIVE	0x07
#define KATTACH_VBR_STATE_EMPTY		0xff

/* kattach vm status */
#define KATTACH_VM_STATUS_NEW		0x00		/* new VM waiting deployment */
#define KATTACH_VM_STATUS_RUNNING	0x01		/* Running VM */
#define KATTACH_VM_STATUS_STOPPED	0x02		/* Stopped VM */
#define KATTACH_VM_STATUS_DISABLED	0x03		/* Disabled VM */
#define KATTACH_VM_STATUS_DELETED	0x04		/* Deleted VM - pending removal */
#define KATTACH_VM_STATUS_STARTUP	0x05		/* Starting VM */
#define KATTACH_VM_STATUS_OP_DEPLOY	0x06		/* AppQueue set operational command to deploy */
#define KATTACH_VM_STATUS_OP_STOP	0x07		/* AppQueue set operational command to stop */
#define KATTACH_VM_STATUS_OP_START	0x08		/* AppQueue set operational command to start */
#define KATTACH_VM_STATUS_OP_RESTART	0x09		/* AppQueue set operational command to restart */
#define KATTACH_VM_STATUS_OP_REMOVE	0x0a		/* AppQueue set operational command to remove */
#define KATTACH_VM_STATUS_OP_GRPKILL	0x0b		/* AppQueue set operational command to group kill */
#define KATTACH_VM_STATUS_END		0x0b		/* marker */
#define KATTACH_VM_STATUS_UNKNOWN	0xff		/* Unknown status */

/* kattach timers */
#define KATTACH_TIMER_QEMU_SPINUP	500000		/* wait time for QEMU to spin up */

/* vkaos defines */
#define KATTACH_VKAOS_KERNELPATH	"/appq/vkaos/"
#define KATTACH_VKAOS_KERNEL		"vkImage"
#define KATTACH_VDISKEXT		"kvd"
#define KATTACH_VDISKCFG		"cfi"
#define KATTACH_VKAOS_APPDISK		"vda"
#define KATTACH_VKAOS_APPPATH		"/kaos/apps"
#define KATTACH_VKAOS_LAUNCH		"vkaos_launch"

/* proc stuff */
#define KATTACH_PROC_BOOTID		"/proc/sys/kernel/random/boot_id"
#define KATTACH_PROC_UUID		"/proc/sys/kernel/random/uuid"

/* update stuff */
#define KATTACH_UPD_DEVICES		0x001
#define KATTACH_UPD_CONFIG		0x002
#define KATTACH_UPD_INSTALL		0x004
#define KATTACH_UPD_VMST		0x008
#define KATTACH_UPD_VMPORTS		0x010
#define KATTACH_UPD_VBRIDGE		0x020
#define KATTACH_UPD_VMIMAGES		0x040
#define KATTACH_UPD_APPMODULES		0x080
#define KATTACH_UPD_NETDEV		0x100
#define KATTACH_UPD_VNS			0x200
#define KATTACH_UPD_CFGGRP		0x400
#define KATTACH_UPD_FW			0x800


/* function prototypes */
size_t kattach_strlcpy(char *dst, const char *src, size_t size);
size_t kattach_strlcat(char *dst, const char *src, size_t size);
u8 kattach_sysexec(char *cmd);
u8 kattach_exec(char *cmd);
int kattach_bkexec(char *cmd, char *kargv);
int kattach_execbk(char *cmd, char *kargv);
unsigned long kattach_hash(char *str); 
void kattach_getbuuid(void);
void kattach_getruuid(void);
u8 kattach_dev_init(void);
void kattach_devadd_block(char *newdev,u16 major, u16 minor);
void kattach_devadd_char(char *newdev,u16 major, u16 minor);
void kattach_thedawn(void);
void kattach_thedawn_console(void);
void kattach_thedawn_generate_dev(void);
void kattach_thedawn_init_devlist(void);
void kattach_thedawn_devlist_build(char *devname, int major, int minor, u16 perm); 
void kattach_thedawn_devlist_add(void);
void kattach_thedawn_shm(void);
int kattach_thedawn_shm_open(char *kattach_shm_path);
void kattach_thedawn_shm_close(void);
void kattach_kcmdline(void);
u8 kattach_parse_cmdline(void);
u8 kattach_net_chkmac(char *devname);
void kattach_net_findmac(void);
char *kattach_net_parseip(u32 iptp);
u8 kattach_net_chkmask(u16 slash);
void kattach_net_linkdown(char *devname);
void kattach_net_linkup(char *devname);
void kattach_net_dhcp(char *devname);
void kattach_net_dhcpscan(void);
void kattach_net_vkaos(void);
void kattach_net_loop(void);
void kattach_net_setup(void);
void kattach_net_dns_add(u32 ip);
void kattach_net_rte_add_default(u32 ip);
void kattach_net_rte_del_default(u32 ip);
void kattach_net_ip_add(u32 ip, u16 slash, char *netdev);
void kattach_net_ip_del(u32 ip, u16 slash, char *netdev);
void kattach_network(void);
void kattach_init_vkaos(void);
void kattach_init_sshkey(void);
void kattach_init_sshd(void);
void kattach_sql_init(void);
void kattach_loop(void);
void kattach_init_sys(void);
void kattach_vm_launch(void);
void kattach_vm_init(void);
void kattach_vm_build_vlans(void);
void kattach_vm_launch_all(void);
void kattach_vm_bridge_up(u16 index);
void kattach_vm_create_bridge(u16 vlan);
void kattach_vm_apply_fw(void);
void kattach_vm_ldapd(void);
void kattach_vm_dhcpd(void);
void kattach_net_genmac(void);
u32 kattach_net_mask(u16 slash);
u32 kattach_net_bcast(u32 ip, u16 slash);
u8 kattach_vm_dupmac(void);
u32 kattach_net_netaddr(u32 ip, u16 slash);
u32 kattach_net_ip_assign(u32 subnet, u16 slash);
u8 kattach_vm_dupip(u32 genip);
void kattach_init_shm(void);
void kattach_sys_shm_sync(void);
void kattach_sys_shm_sync_devices(void);
void kattach_sys_shm_sync_cfg(void);
void kattach_sys_shm_sync_install(void);
void kattach_sys_shm_sync_vmst(void);
void kattach_sys_shm_sync_vmp(void);
void kattach_sys_shm_sync_vbr(void);
void kattach_sys_shm_sync_vmi(void);
void kattach_sys_shm_syncback_devices(void);
void kattach_sys_shm_syncback_cfg(void);
void kattach_sys_shm_syncback_install(void); 
void kattach_sys_shm_syncback_vmst(void);
void kattach_sys_shm_syncback_vmp(void);
void kattach_sys_shm_syncback_vbr(void);
void kattach_sys_shm_syncback(void);
void kattach_sys_shm_syncback_appmods(void);
void kattach_sys_shm_sync_appmods(void);
u8 kattach_sys_shm_setsync_netdev(void);
u8 kattach_sys_shm_setsync_appmods(void);
u8 kattach_sys_shm_setsync_vmst(void);
u8 kattach_sys_shm_setsync_vmp(void);
u8 kattach_sys_shm_setsync_vbr(void);
u8 kattach_sys_shm_setsync_vmi(void);
void kattach_sys_shm_checksync(void);
void kattach_sys_shm_checklocks(void);
void kattach_loop_monitor(void);
void kattach_vm_monitor(void);
void kattach_vm_start(u32 index);
void kattach_reboot(void);
void kattach_vm_update_vbridge(void);
void kattach_vm_new_net(u32 index);
void kattach_vm_new(void);
void kattach_vm_bridge_down(u16 index);
char *kattach_thedawn_genpass(char *dfpass);
void kattach_thedawn_cfg_write(void);
void kattach_sys_write_resolv(void);
void kattach_sql_update_config(void);
void kattach_vm_check_vmst(void);
void kattach_vm_bradd(u32 index);
void kattach_vm_bridge_addif(u16 vmport, u16 vlan);
void kattach_vm_bridge_delif(u16 vmport, u16 vlan);
void kattach_vm_appmods(void);
void kattach_sql_dbclose(void);
void kattach_vm_vmi_update(void);
void kattach_vkaos_loop_monitor(void);
void kattach_vkaos_loop(void);
void kattach_vkaos_launch(void);
void kattach_vm_new_cfgdisk(u32 index);
void kattach_sys_shm_syncback_netdev(void);
void kattach_sys_shm_sync_netdev(void);
void kattach_sys_shm_syncback_vns(void);
void kattach_sys_shm_sync_vns(void);
void kattach_sys_shm_syncback_cfggrp(void);
void kattach_sys_shm_sync_cfggrp(void);
void kattach_sys_shm_syncback_fw(void);
void kattach_sys_shm_sync_fw(void);
void kattach_vm_netdev_init(void);
void kattach_vm_netdev_update(void);
void kattach_sys_write_extlinux(void);
void kattach_sql_clear_netdev(void);
void kattach_netdev_checklink(void);
void kattach_sql_insert_config(void);
void kattach_sys_hvimage(void);
void kattach_thedawn_cmfs(void);
u8 kattach_net_getmac(char *devname);
void kattach_sys_debug_akflags(u8 fmt, u8 dbgtag);
void kattach_vm_shutdown(void);
void kattach_iana_ports(void);
void kattach_sshboot(void);
void kattach_sys_sshd(void);
char *kattach_thedawn_chkpass(char *dfpass, char *encpass);
void kattach_thedawn_genauth(void);
void kattach_vm_apply_vns(void);
void kattach_vm_launch_mgr(void);
void kattach_vm_vns_update(void);
void kattach_vm_cfggrp_update(void);
void kattach_vm_fw_update(void);
void kattach_vm_priority_update(u32 index);
void kattach_vm_fw_update_filter(void);
void kattach_vm_fw_update_nat(void);
void kattach_vm_fw_update_mangle(void);
void kattach_vm_fw_update_zones(void);
void kattach_vm_fw_update_apps(void);
void kattach_vm_db_fw(void);
void kattach_vm_fw_syncdb_zones(void);
void kattach_vm_fw_syncdb_apps(void);
void kattach_vm_fw_syncdb_fwmain(void);
void kattach_vm_fw_syncdb_filter(void);
void kattach_vm_fw_syncdb_nat(void);
void kattach_vm_fw_syncdb_mangle(void);
void kattach_vm_fw_syncdb(void);
void kattach_loop_fw_defaults(void);



/* global variables */
u8 kattach_setup;					/* indicate setup detected */
u8 kattach_macfound;					/* indicate mac found on an interface */
u8 kattach_dbactive;					/* indicate if db were setup properly */
u8 kattach_genmac[6];					/* target for generated mac */
u8 kattach_reboot_me;					/* reboot detected */
u8 kattach_bootmode;					/* retain boot mode */
u8 kattach_recovery;					/* recovery mode */
u8 kattach_hasdhcpif;					/* dhcp interface counter */
u8 kattach_ntpnext;					/* next ntp server */
u32 kattach_ntptime;					/* next time */
u32 kattach_dbtmpindex;					/* temporary tmpindex for db */
u16 kattach_change_detected;				/* change detected */
kattach_dev_t kattach_devices;				/* kattach device table */
kattach_cfg_t kattach_cfg;				/* kattach configuration */
kattach_install_t kattach_install;			/* partition to install */
kattach_vmst_t kattach_vmst;				/* vm session table */
kattach_vmp_t kattach_vmports;				/* virtual ports */
kattach_vbr_t kattach_vbridge;				/* virtual bridges */
kattach_vmi_t kattach_vmimages;				/* virtual images */
kattach_am_t kattach_appmods;				/* app modules */
kattach_netdev_t kattach_netdev;			/* network interfaces */
kattach_vns_t kattach_vns;				/* virtual network services */
kattach_cfggrp_t kattach_cfggrp;			/* app configuration groups */
kattach_fw_t kattach_fw;				/* firewall */
char *kattach_netipp;					/* string used for ip parsing */
char kattach_buuid[40];					/* string used to hold the boot_id UUID */
char kattach_ruuid[40];					/* string used to hold a random UUID */
int kattach_gettypid[4];				/* getty pid */
int kattach_sshdpid;					/* sshd pid */
int kattach_ntppid;					/* ntp client pid */

/* global file descriptors */
int fd_con;						/* file descriptor for console */
