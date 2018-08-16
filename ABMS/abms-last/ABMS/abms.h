/*
 * AppOS Boot Management System (ABMS)
 * Copyright (c) 2002-2008 Spliced Networks LLC.
 * All Rights Reserved.
 *
 * Author: John Buswell <buswellj@splicednetworks.com>
 * Release: 4.0.0
 *
 * LICENSE: GPL v3
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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
 * To contact Spliced Networks LLC:
 * 
 * Spliced Networks LLC
 * 4820 Fisher Road
 * Athens, OH 45701
 * USA
 * 
 * Tel: (408) 416-3832
 * email: support@splicednetworks.com
 *
 * http://www.splicednetworks.com
 *
 * 03-May-2005: [Release 1.1.7]
 * 29-Sep-2005: [Release 1.1.8]
 * 04-Nov-2005: [Release 1.1.9]
 * 01-Mar-2006: [Release 2.0.0]
 * 05-Sep-2006: [Release 3.0.0]
 *
 * Changed name from Linux Image Management Boot System (LIMBS) to ABMS for 4.0
 *
 * 23-Mar-2008: [Release 4.0.0rc1]
 * 27-Apr-2008: [Release 4.0.0rc2]
 * 01-Jun-2008: [Release 4.0.0rc3]
 * 11-Jun-2008: [Release 4.0.0rc4]
 * 22-Jun-2008: [Release 4.0.0rc5]
 * 24-Jun-2008: [Release 4.0.0]
 *
 */

/* enabling debug will print commands instead of system */
#define ABMS_DEBUG		1

/* enabling verbose debug will add extra output during exec */
/* #define ABMS_DEBUG_VERBOSE	1 */
/* #define ABMS_DEBUG_ANNOYING 1 */

/* version information */
#define ABMS_VERSION           "4.0.0"
#define ABMS_ARCH              "x86"
#define ABMS_RELEASE           "stable"
#define SN_COPYRIGHT            "Copyright (c) 2002-2008 Spliced Networks LLC"

/* basic definitions */
#define ABMS_PATH		"/app/os/sys/"
#define ABMS_BINPATH		"/app/os/sys/bin/"
#define ABMS_IMAGEDIR		"/app/os/images/"
#define ABMS_IMAGE		"abms.sqfs"
#define ABMS_SSHPORT		1103
#define ABMS_KEY		"/app/os/sys/sshd/keys/rsa.key"
#define ABMS_SSHD		"/app/os/sys/sshd/sbin/dropbear"
#define ABMS_KEYGEN		"/app/os/sys/sshd/bin/dropbearkey"
#define ABMS_CMDLINE		"/proc/cmdline"
#define ABMS_CONSOLE		"/dev/console"
#define ABMS_SHELL		"/app/os/sys/bin/ash"
#define RC_OK			1
#define RC_FAIL			0
#define ABMS_SECTTY		"tty8"
#define ABMS_SECTTYV		8
#define ABMS_ISSUENET		"/etc/issue.net"
#define ABMS_LOOPBACK		"127.0.0.1/8"
#define ABMS_LOOPDEV		"lo"
#define ABMS_LINKLOCAL          "169.254.6.66"
#define ABMS_LLSLASH            16
#define ABMS_LLADD              0x0
#define ABMS_LLDEL              0x1
#define ABMS_BLOCKDEV		0xb
#define ABMS_CHARDEV		0xc
#define ABMS_ASCSTART		0x61
#define ABMS_ASCEND		0x7a
#define ABMS_FLASHFS		"ext3"
#define ABMS_FLASHDST		"/flash"
#define ABMS_FLASHSLOT		"slot"
#define ABMS_FLASHCFG		".snrc"
#define ABMS_FLAUTH		".auth"
#define ABMS_FLPASS		"passwd"
#define ABMS_FLSHADOW		"shadow"
#define ABMS_FLGROUP		"group"
#define ABMS_MAX_IPBIND		32
#define ABMS_MAX_PHY		16

/* emfs */
#define ABMS_EMFS_DST		"/app/os/emfs"
#define ABMS_EMFS_IMAGE		"emfs.sqfs"
#define ABMS_EMFS_COMBINE	"/app/os/esys"


/* IP parsing */
#define ABMS_MAXSLASH		31				/* used for calculating subnets, do not use 32 */
#define ABMS_MINSLASH		1

/* commands */
#define ACMD_CLEAR		"clear"
#define ACMD_MKDIR		"mkdir"
#define ACMD_MOUNT		"mount"
#define ACMD_UMOUNT		"umount"
#define ACMD_MKNOD		"mknod"
#define ACMD_CHMOD		"chmod"
#define ACMD_ECHO		"echo"
#define ACMD_RM			"rm"
#define ACMD_MKSQUASHFS		"mksquashfs"
#define ACMD_IP			"ip"
#define ACMD_GENPASS		"sngp.sh"
#define ACMD_DHCP		"dhcpcd"
#define ACMD_SH			"ash"
#define ACMD_GETTY		"getty"
#define ACMD_LOGIN		"login"
#define ACMD_CP			"cp"
#define ACMD_RMDIR		"rmdir"
#define ACMD_LOSETUP		"losetup"
#define ACMD_LN			"ln"

/* modes */
#define ABMS_MODE_UNKNOWN			0x00
#define ABMS_MODE_MANAGED			0x01		/* physical mgmt network, dhcp and IP Abacus */
#define ABMS_MODE_REMOTE			0x02		/* remote device, VPN, static IP and IP Abacus (local) */
#define ABMS_MODE_STANDALONE			0x03		/* stand alone server, deploy via CLI manually, static IP or DHCP */
#define ABMS_MODE_NETDEV			0x04		/* network device - router, switch, has multiple interfaces, WAN */
#define ABMS_MODE_AEROSWARM			0x05		/* device is an Open Access Point Device */
#define ABMS_MODE_AEROSWARM_MANAGED		0x06		/* device is an Open Access Point managed by IP Abacus */
#define ABMS_MODE_VM				0x07		/* vm guest  */
#define ABMS_MODE_VM_MANAGED			0x08		/* managed vm guest */

#define ABMS_MODE_DEPLOYED			0x64
#define ABMS_MODE_DEPLOYED_MANAGED		0x65
#define ABMS_MODE_DEPLOYED_REMOTE		0x66
#define ABMS_MODE_DEPLOYED_STANDALONE		0x67
#define ABMS_MODE_DEPLOYED_NETDEV		0x68
#define ABMS_MODE_DEPLOYED_AEROSWARM		0x69
#define ABMS_MODE_DEPLOYED_AEROSWARM_MANAGED	0x70
#define ABMS_MODE_DEPLOYED_VM			0x71
#define ABMS_MODE_DEPLOYED_VM_MANAGED		0x72

#define ABMS_MODE_RESERVED			0xc8
#define ABMS_MODE_NOTCFG			0xff

/* stack stuff */
#define ABMS_STACK_IMAGES		"/app/stack/images/"
#define ABMS_STACK_FLASHIMAGE		"/flash/stacks/"
#define ABMS_STACK_PRODUCTION		"/app/stack/production/"
#define ABMS_STACK_CHROOTSTACK		"/app/stack/"
#define ABMS_STACK_CHROOTCOMMON 	"/app/common/"
#define ABMS_STACK_CHROOTSUPPORT	"/app/support/"
#define ABMS_STACK_CHROOTCONFIG		"/config"
#define ABMS_STACK_FLASHCONFIG		"/flash/config/"

#define ABMS_STACK_STACK                "stack.sqfs"
#define ABMS_STACK_COMMON               "common.sqfs"
#define ABMS_STACK_SUPPORT              "support.sqfs"



/* note: modes determine how ABMS handles the network and system configuration.
 *       the mode space begins with 0x01 (1) and runs thru 0xff (255). The mode
 *	 value serves two purposes - first to tell ABMS is the device is deployed
 *	 in production, second to tell ABMS what type of deployment it is. A device
 *	 is typically not deployed by default. Each mode has two values (not-deployed,
 *	 ranging from 0x01 thru 0x63) and (deployed, ranging from 0x64 thru 0xc7).
 *
 *	 To get the deployed value you simply add decimal 100 (0x64) to the non-deployed value.
 *	 The values from 0xc8 thru 0xfe (200 thru 254) are reserved for special use.
 */

/* structures */
typedef unsigned u8;
typedef unsigned long u16;
typedef unsigned long long u32;

/* proc cmdline 
 *
 * abms=[mode]:[ip]:[slash]:[gw]:[dns]:[target]:[mgmtmac]:[flashdev]:[slot]:[sshport]:[debug]:[failover]
 *
 *
 * example:
 *
 *	abms=3:10.20.30.40:24:10.20.30.1:10.20.30.40:192.168.75.22:001e6c770306:sda1:0:1103:0
 *
 * This would boot standalone mode, configure 10.20.30.40/24 as the IP.
 * The gateway is 10.20.30.1, the target IP Abacus box is 192.168.75.22.
 * It will use the interface with a mac of 00:1e:6c:77:03:06 as the management
 * port. It'll attempt to use /dev/sda1, image slot 0 for AppStacks. 
 *
 *
 * fields:
 *
 *       value:          type:                   description:
 *       ===================================================================================
 *       
 *	 [mode]		 unsigned		 1 = managed, 2 = remote, 3 = standalone (+100 for provisioned)
 *	 [ip]		 string			 IPv4 address to use, 0 = DHCP
 *	 [slash]	 string			 netmask in slash notation (eg. 24)
 *	 [gw]		 string			 IP address of gateway
 *	 [dns]		 string			 IP address of primary DNS server
 *	 [target]	 string			 IP address of IP Abacus box
 *	 [mgmtmac]	 string			 Mac of mgmt interface (00:1e:6c:77:03:06 -> 001e6c770306)
 *	 [flashdev]	 string			 flash storage device (any disk will work, use nodX-Y to create it)
 *	 [slot]		 unsigned		 image slot to use (0 = default, 666 = try all)
 *	 [sshport]	 unsigned		 port to use (default 1103)
 *	 [debug]	 unsigned		 real-time debugging (default - disabled) 0 = off, 1 = on
 *	 [failover]	 unsigned		 are we a backup for something (0 = default image, >= 1 backup, 0xff = factory)
 *
 */

typedef struct {
	u32 ip;
	u32 gw;
	u32 ipab;
	u32 dns;
	u16 slash;
	u8 mode;
	u8 slot;
	u8 mac[6];
	u16 sshport;
	u8 debug;
	u8 failover;
	char *flashdev;
} abms_cfg_t;

typedef struct {
	u32 ip;
	u16 slash;
} abms_ipbind_t;

typedef struct {
	char devname[16];
	u8 mac[6];
	u8 mgmt:1;
	u8 live:1;
	u8 spare:6;
	abms_ipbind_t ipbind[ABMS_MAX_IPBIND];
} abms_iftable_t;

typedef struct {
	u8 nxt_idx;
	u8 single;
	abms_iftable_t phy[ABMS_MAX_PHY];
} abms_netdev_t;

/* Prototypes */

u8 abms_system(char *cmd);
u8 abms_exec(const char *cmd);
u32 abms_parse_cmdline(void);
void abms_disp_init(void);
void abms_preinit(void);
void abms_init(void);
void ramfs_init(void);
void pass_init(void);
void sshd_init(void);
void sqfs_gen(void);
void ramfs_clean(void);
void sqfs_init(void);
void abms_net_findmac(void);
void abms_net_linkdown(char *devname);
void abms_net_linkup(char *devname);
void abms_net_remote(void);
void abms_net_default(void);
void abms_net_standalone(void);
void abms_net_d_default(void);
void abms_net_d_remote(void);
void abms_net_d_standalone(void);
void abms_net_dhcpscan(void);
void abms_net_setup(void);
u8 abms_net_chkmask(u16 slash);
char *abms_net_parseip(u32 iptp);
void abms_net_dhcp(char *devname);
void abms_init_loop(void);
u8 abms_net(void);
u8 abms_net_chkmac(char *devname);
void abms_cmdline(void);
void abms_net_loopback(void);
void abms_net_linklocal(char *devname, u8 action);
void abms_loop_getty(void);
void abms_net_sshd(void);
void abms_mount(char *src, char *fs, char *dst);
void abms_mountf(char *src, char *fs, char *dst, u8 fsrw);
void abms_mknod(char *dst, u8 type, u16 major, u16 minor);
void abms_mkdir(char *path, u8 p);
void abms_chmod(char *perm, char *path);
void abms_rm(char *dst);
void abms_flash_init(void);
void abms_map_init(void);
void abms_flash_emfs(void);
u8 abms_map_loopadd(u8 fs, u8 type, u8 crypto, char *src);
void abms_flash_combine(void);
void abms_map_dump(void);
void abms_losetup(char *loop, char *image);
void abms_mountshm(char *dst, u32 size);
void abms_symlink(char *src, char *dst);
void abms_flash_auth(void);
void abms_stack_init(void);
void abms_net_prod(void);

size_t abms_strlcpy(char *dst, const char *src, size_t size);
size_t abms_strlcat(char *dst, const char *src, size_t size);

/* Global Variables */

abms_cfg_t abms_cfg;
int fd_con;
char abms_mgmtdev[64];
char *abms_netipp;
u8 abms_deployed;
u8 abms_mgmtfound;
u8 abms_netup;
u8 abms_eloop;				/* loopback dev for live upgrade */
u8 abms_nostacks;
u32 abms_gettypid;
u32 abms_ramimage;			/* use ramdisk - split as needed, just u8 */
