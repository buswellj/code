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
 * 11-Jun-2008: [Release 4.0.0]
 *
 */

/* enabling debug will print commands instead of system */
#define ABMS_DEBUG		1

/* enabling verbose debug will add extra output during exec */
/* #define ABMS_DEBUG_VERBOSE	1 */

/* enabing real time debug will print verbose data during runtime, console shell, delayed exec */
/* #define ABMS_RT_DEBUG		1 */

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

/* IP parsing */
#define ABMS_MAXSLASH		31				/* used for calculating subnets, do not use 32 */
#define ABMS_MINSLASH		1

/* commands */
#define ACMD_CLEAR		"clear"
#define ACMD_MKDIR		"mkdir"
#define ACMD_MOUNT		"mount"
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
 * abms=[mode]:[ip]:[slash]:[gw]:[dns]:[target]:[mgmtmac]:[flashdev]:[slot]
 *
 *
 * example:
 *
 *	abms=3:10.20.30.40:24:10.20.30.1:192.168.75.22:001e6c770306:sda1:0:0
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
	char *flashdev;
} abms_cfg_t;

/* Prototypes */

u8 abms_system(char *cmd);
u8 abms_exec(const char *cmd);
u32 abms_parse_cmdline(void);
void abms_disp_init(void);
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

