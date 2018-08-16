/*
 * AppQueue
 * Copyright (c) 2009 - 2010 Carbon Mountain LLC.
 * All Rights Reserved.
 *
 * John Buswell <buswellj@carbonmountain.com>
 * version 0.6.2.0
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
/* definitions */

/* AppQueue release information */
#define APPQUEUE_VERSION		"0.6.2.0"
#define APPQUEUE_ARCH			"x86 / x64"
#define APPQUEUE_RELEASE		"dev"
#define APPQUEUE_COPYRIGHT		"Copyright (c) 2009 - 2010 Carbon Mountain LLC."
#define APPQUEUE_LICENSE		"This program may be freely redistributed under the terms of the GNU GPLv2"
#define APPQUEUE_LINK			"http://www.carbonmountain.com"
#define APPQUEUE_CLI_LINES		54

/* result codes */
#define RC_FAIL				0
#define RC_OK				1
#define RC_MISSING			2

/* Commands */
#define APPQUEUE_CMD_UNKNOWN		0
#define APPQUEUE_CMD_INSTALL		1
#define APPQUEUE_CMD_UNINSTALL		2
#define APPQUEUE_CMD_VM			3

#define APPQUEUE_CMD_CREATEVM		16
#define APPQUEUE_CMD_DEPLOYVM		17
#define APPQUEUE_CMD_LISTVM		18
#define APPQUEUE_CMD_STARTVM		19
#define APPQUEUE_CMD_STOPVM		20
#define APPQUEUE_CMD_KILLVM		21
#define APPQUEUE_CMD_DELETEVM		22

/* Strings */
#define APPQUEUE_STR_INSTALL		"install"
#define APPQUEUE_STR_UNINSTALL		"uninstall"
#define APPQUEUE_STR_CREATEVM		"createvm"
#define APPQUEUE_STR_DEPLOYVM		"deployvm"
#define APPQUEUE_STR_LISTVM		"listvm"
#define APPQUEUE_STR_STARTVM		"startvm"
#define APPQUEUE_STR_STOPVM		"stopvm"
#define APPQUEUE_STR_KILLVM		"killvm"
#define APPQUEUE_STR_DELETEVM		"deletevm"

/* exec */
#define APPQUEUE_EXEC_QEMUIMG		"/appq/bin/qemu-img"
#define APPQUEUE_SHELL			"/bin/ash"
#define APPQUEUE_FDISK			"/sbin/fdisk"

/* paths */
#define APPQUEUE_PATH			"/appq/"
#define APPQUEUE_PATH_HVIMAGE		"/appq/hvimg/"
#define APPQUEUE_PATH_IMAGES		"/appq/images/"
#define APPQUEUE_PATH_VMIMPORT		"/appq/images/vmimport/"
#define APPQUEUE_PATH_HYPERVISOR	"/appq/bin/"
#define APPQUEUE_PATH_VKAOS		"/appq/vkaos/"
#define APPQUEUE_PROC_CPUINFO           "/proc/cpuinfo"
#define APPQUEUE_PROC_UPTIME            "/proc/uptime"
#define APPQUEUE_PROC_UUID		"/proc/sys/kernel/random/uuid"

/* app modules */
#define APPQUEUE_AM_PATH		"/appq/am/"
#define APPQUEUE_AM_CM			"cm/"						/* default CM vendor ID */
#define APPQUEUE_AM_IMPORT		"import/"					/* vendor id for local imports */
#define APPQUEUE_AM_CLI			"cli/climod.aq"
#define APPQUEUE_AM_MGR			"mgr/appmgr.aq"

/* paths for groups are /app/am/<group>/ */
#define APPQUEUE_AM_GROUP		"cfg/group.sqfs"
#define APPQUEUE_AM_RAW			"cfg/raw/"

/* limits */
#define APPQUEUE_MAX_LEN_NAME		64
#define APPQUEUE_MAX_LEN_PARAM		32
#define APPQUEUE_MAX_LEN_URL		256
#define APPQUEUE_MAX_LEN_CONTENT	6407
#define APPQUEUE_MIN_LEN_CONTENT	4407
#define APPQUEUE_MAX_VLAN		4094

/* url */
#define APPQUEUE_HTTP			"http://"
#define APPQUEUE_HTTPS			"https://"
#define APPQUEUE_URL_VENDOR_CM		".cm"
#define APPQUEUE_URL_CLI		".cli"
#define APPQUEUE_URL_MGR		".mgr"
#define APPQUEUE_URL_DOMAIN_E		".appqueue.org"
#define APPQUEUE_URL_DOMAIN_D		".appqueue.net"
#define APPQUEUE_URL_DOMAIN_S		".appqueue.com"
#define APPQUEUE_UA			"AppQueue/x64"

/* trees */
#define APPQUEUE_TREE_EDGE		0
#define APPQUEUE_TREE_DEV		1
#define APPQUEUE_TREE_SUPPORTED		2

/* vendors */
#define APPQUEUE_VID_CM			0


/* misc */
#define APPQUEUE_DIV_MB			1048576
#define APPQUEUE_VKAOS_KERNEL		"vkImage"
#define APPQUEUE_PERM_GROUP              (S_IRWXU | S_IRGRP | S_IXGRP | S_IWGRP)


/* locks */
#define APPQUEUE_LCK_DEV		0x00
#define APPQUEUE_LCK_CFG		0x01
#define APPQUEUE_LCK_INST		0x02
#define APPQUEUE_LCK_VMST		0x03
#define APPQUEUE_LCK_VMPORTS		0x04
#define APPQUEUE_LCK_VBRIDGE		0x05
#define APPQUEUE_LCK_VMIMAGES		0x06
#define APPQUEUE_LCK_APPMODULES		0x07
#define APPQUEUE_LCK_REBOOT		0x08
#define APPQUEUE_LCK_NETDEV		0x09

/* vlan definitions */
#define KATTACH_NET_VLAN_NOTSET         0
#define KATTACH_NET_VLAN_LOCAL          1
#define KATTACH_NET_VLAN_8021Q          2
#define KATTACH_NET_VLAN_ROUTED         3
#define KATTACH_NET_VLAN_NAT            4

/* cli menu defines */
#define APPQUEUE_MENU_MAX_CMDLEN	16
#define APPQUEUE_MENU_MAX_DESCLEN	64
#define APPQUEUE_MENU_MAX_ITEMS		16
#define APPQUEUE_MENU_BAR		"-----------------------------------------------------------------"
#define APPQUEUE_MENU_BARXL		"-------------------------------------------------------------------------------------------"

/* cli cli permissions */
//#define APPQUEUE_CLI_AUTH_UID		500				/* this is in here for development leave it commented out */
#define APPQUEUE_CLI_AUTH_UID		0x64
#define APPQUEUE_CLI_AUTH_USER		0x00
#define APPQUEUE_CLI_AUTH_ADMIN		0xf0
#define APPQUEUE_CLI_AUTH_DEV		0xf7

/* cli function pointers */
#define APPQUEUE_CLI_FP_MAIN			0
#define APPQUEUE_CLI_FP_INFO			1
#define APPQUEUE_CLI_FP_APPS			2
#define APPQUEUE_CLI_FP_VM			3
#define APPQUEUE_CLI_FP_NET			4
#define APPQUEUE_CLI_FP_SYS			5
#define APPQUEUE_CLI_FP_SETUP			6
#define APPQUEUE_CLI_FP_MAINT			7
#define APPQUEUE_CLI_FP_BOOT			8
#define APPQUEUE_CLI_FP_EXIT			9
#define APPQUEUE_CLI_FP_INFO_SYS		10
#define APPQUEUE_CLI_FP_INFO_LINK		11
#define APPQUEUE_CLI_FP_INFO_L2			12
#define APPQUEUE_CLI_FP_INFO_L3			13
#define APPQUEUE_CLI_FP_INFO_L2_VLAN		14
#define APPQUEUE_CLI_FP_INFO_L2_VBRIDGE		15
#define APPQUEUE_CLI_FP_INFO_L2_VMAC		16
#define APPQUEUE_CLI_FP_INFO_L2_VMPORTS		17
#define APPQUEUE_CLI_FP_INFO_L3_IP		18
#define APPQUEUE_CLI_FP_INFO_L3_ROUTE		19
#define APPQUEUE_CLI_FP_INFO_L3_ARP		20
#define APPQUEUE_CLI_FP_APPS_LIST		21
#define APPQUEUE_CLI_FP_APPS_AVAIL		22
#define APPQUEUE_CLI_FP_APPS_INSTALL		23
#define APPQUEUE_CLI_FP_APPS_REMOVE		24
#define APPQUEUE_CLI_FP_APPS_SHOWVM		25
#define APPQUEUE_CLI_FP_APPS_CREATEVM		26
#define APPQUEUE_CLI_FP_APPS_DELETEVM		27
#define APPQUEUE_CLI_FP_VM_LIST			28
#define APPQUEUE_CLI_FP_VM_DEPLOY		29
#define APPQUEUE_CLI_FP_VM_START		30
#define APPQUEUE_CLI_FP_VM_STOP			31
#define APPQUEUE_CLI_FP_VM_RESTART		32
#define APPQUEUE_CLI_FP_VM_KILLALL		33
#define APPQUEUE_CLI_FP_VM_REMOVE		34
#define APPQUEUE_CLI_FP_NET_VLAN		35
#define APPQUEUE_CLI_FP_NET_DHCP		36
#define APPQUEUE_CLI_FP_NET_NETIF_ADDIF		37
#define APPQUEUE_CLI_FP_NET_NETIF_EDITIF	38
#define APPQUEUE_CLI_FP_NET_NETIF_RMIF		39
#define APPQUEUE_CLI_FP_NET_NETIF_IFMGR		40
#define APPQUEUE_CLI_FP_NET_VLAN_ADD		41
#define APPQUEUE_CLI_FP_NET_VLAN_EDIT		42
#define APPQUEUE_CLI_FP_NET_VLAN_LIST		43
#define APPQUEUE_CLI_FP_SYS_HOSTNAME		44
#define APPQUEUE_CLI_FP_SYS_DOMAIN		45
#define APPQUEUE_CLI_FP_SYS_DNS			46
#define APPQUEUE_CLI_FP_SYS_DISK_BOOTDISK	47
#define APPQUEUE_CLI_FP_SYS_DISK_SWAPDISK	48
#define APPQUEUE_CLI_FP_SYS_DISK_APPQDISK	49
#define APPQUEUE_CLI_FP_SYS_DISK_DATADISK	50
#define APPQUEUE_CLI_FP_MAINT_SHELL		51
#define APPQUEUE_CLI_FP_MAINT_DBAPPQ		52
#define APPQUEUE_CLI_FP_MAINT_DBKAOS		53
#define APPQUEUE_CLI_FP_MAINT_DBVMSESS		54
#define APPQUEUE_CLI_FP_BOOT_STATUS		55
#define APPQUEUE_CLI_FP_BOOT_UPGRADE		56
#define APPQUEUE_CLI_FP_BOOT_REBOOT		57
#define APPQUEUE_CLI_FP_NET_VLAN_DELETE		58
#define APPQUEUE_CLI_FP_BOOT_GTIMG		59
#define APPQUEUE_CLI_FP_SYS_DISK_FDISK		60
#define APPQUEUE_CLI_FP_MAINT_TSDUMP		61
#define APPQUEUE_CLI_FP_VM_STARTALL		62
#define APPQUEUE_CLI_FP_BOOT_FACTORY		63
#define APPQUEUE_CLI_FP_INFO_L4			64
#define APPQUEUE_CLI_FP_INFO_L4_FW		65
#define APPQUEUE_CLI_FP_INFO_L4_VSIP		66
#define APPQUEUE_CLI_FP_INFO_VM			67
#define APPQUEUE_CLI_FP_INFO_VM_PRIORITY	68
#define APPQUEUE_CLI_FP_INFO_VM_FW		69
#define APPQUEUE_CLI_FP_SYS_CLOCK		70
#define APPQUEUE_CLI_FP_SYS_CLOCK_NTPA		71
#define APPQUEUE_CLI_FP_SYS_CLOCK_NTPB		72
#define APPQUEUE_CLI_FP_SYS_CLOCK_NTPC		73
#define APPQUEUE_CLI_FP_SYS_CLOCK_NTPINT	74
#define APPQUEUE_CLI_FP_SYS_AUTH		75
#define APPQUEUE_CLI_FP_SYS_AUTH_AQPASS		76
#define APPQUEUE_CLI_FP_SYS_AUTH_CLIPASS	77
#define APPQUEUE_CLI_FP_SYS_AUTH_ROOT		78
#define APPQUEUE_CLI_FP_SYS_AUTH_ROOTPASS	79
#define APPQUEUE_CLI_FP_VM_IMPORT		80
#define APPQUEUE_CLI_FP_APPS_IMPORT		81
#define APPQUEUE_CLI_FP_APPS_UPDATE		82
#define APPQUEUE_CLI_FP_VM_PRIORITY		83
#define APPQUEUE_CLI_FP_NET_VNS			84
#define APPQUEUE_CLI_FP_NET_VNS_ADDVSIP		85
#define APPQUEUE_CLI_FP_NET_VNS_RMVSIP		86
#define APPQUEUE_CLI_FP_NET_VNS_ADDVSP		87
#define APPQUEUE_CLI_FP_NET_VNS_RMVSP		88
#define APPQUEUE_CLI_FP_NET_VNS_RATELIMIT	89
#define APPQUEUE_CLI_FP_NET_VNS_STATE		90
#define APPQUEUE_CLI_FP_NET_FW_FILTER		91
#define APPQUEUE_CLI_FP_NET_FW_NAT		92
#define APPQUEUE_CLI_FP_NET_FW_MANGLE		93
#define APPQUEUE_CLI_FP_NET_FW_FILTER_ADD	94
#define APPQUEUE_CLI_FP_NET_FW_FILTER_DEL	95
#define APPQUEUE_CLI_FP_NET_FW_FILTER_EDIT	96
#define APPQUEUE_CLI_FP_NET_FW_FILTER_LIST	97
#define APPQUEUE_CLI_FP_NET_FW_FILTER_REVERSE	98
#define APPQUEUE_CLI_FP_NET_FW_FILTER_LOG	99
#define APPQUEUE_CLI_FP_NET_FW_NAT_ADD		100
#define APPQUEUE_CLI_FP_NET_FW_NAT_DEL		101
#define APPQUEUE_CLI_FP_NET_FW_NAT_EDIT		102
#define APPQUEUE_CLI_FP_NET_FW_NAT_LIST		103
#define APPQUEUE_CLI_FP_NET_FW_NAT_REVERSE	104
#define APPQUEUE_CLI_FP_NET_FW_NAT_LOG		105
#define APPQUEUE_CLI_FP_NET_FW_MANGLE_ADD	106
#define APPQUEUE_CLI_FP_NET_FW_MANGLE_DEL	107
#define APPQUEUE_CLI_FP_NET_FW_MANGLE_EDIT	108
#define APPQUEUE_CLI_FP_NET_FW_MANGLE_LIST	109
#define APPQUEUE_CLI_FP_NET_FW_MANGLE_REVERSE	110
#define APPQUEUE_CLI_FP_NET_FW_MANGLE_LOG	111
#define APPQUEUE_CLI_FP_APPS_CONFIG		112
#define APPQUEUE_CLI_FP_APPS_CONFIG_LIST	113
#define APPQUEUE_CLI_FP_APPS_CONFIG_CREATE	114
#define APPQUEUE_CLI_FP_APPS_CONFIG_REMOVE	115
#define APPQUEUE_CLI_FP_APPS_CONFIG_APP		116
#define APPQUEUE_CLI_FP_SYS_AUTH_AQUSER		117
#define APPQUEUE_CLI_FP_NET_FW			118
#define APPQUEUE_CLI_FP_SYS_DISK		119
#define APPQUEUE_CLI_FP_NET_NETIF		120
#define APPQUEUE_CLI_FP_NET_FW_ZONES		121
#define APPQUEUE_CLI_FP_NET_FW_ZONES_LIST	122
#define APPQUEUE_CLI_FP_NET_FW_ZONES_CREATE	123
#define APPQUEUE_CLI_FP_NET_FW_ZONES_EDIT	124
#define APPQUEUE_CLI_FP_NET_FW_ZONES_DELETE	125
#define APPQUEUE_CLI_FP_NET_FW_APPS		126
#define APPQUEUE_CLI_FP_NET_FW_APPS_LIST	127
#define APPQUEUE_CLI_FP_NET_FW_APPS_CREATE	128
#define APPQUEUE_CLI_FP_NET_FW_APPS_EDIT	129
#define APPQUEUE_CLI_FP_NET_FW_APPS_DELETE	130
#define APPQUEUE_CLI_FP_STORAGE			131
#define APPQUEUE_CLI_FP_STORAGE_LOCAL		132
#define APPQUEUE_CLI_FP_STORAGE_ISCSI		133
#define APPQUEUE_CLI_FP_STORAGE_GLUSTER		134
#define APPQUEUE_CLI_FP_STORAGE_RAID		135
#define APPQUEUE_CLI_FP_STORAGE_VSTORAGE	136
#define APPQUEUE_CLI_FP_STORAGE_VDISK		137
#define APPQUEUE_CLI_FP_STORAGE_VMEDIA		138
#define APPQUEUE_CLI_FP_STORAGE_LOCAL_LIST	139
#define APPQUEUE_CLI_FP_STORAGE_ISCSI_ENABLE	140
#define APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS	141
#define APPQUEUE_CLI_FP_STORAGE_ISCSI_SHOWTGT	142
#define APPQUEUE_CLI_FP_STORAGE_ISCSI_DISCOVERY	143
#define APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNSDISC	144
#define APPQUEUE_CLI_FP_STORAGE_ISCSI_ADDTGT	145
#define APPQUEUE_CLI_FP_STORAGE_ISCSI_DELTGT	146
#define APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS_LIST	147
#define APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS_ADD	148
#define APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS_EDIT	149
#define APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS_DEL	150
#define APPQUEUE_CLI_FP_STORAGE_GLUSTER_LIST	151
#define APPQUEUE_CLI_FP_STORAGE_GLUSTER_ADD	152
#define APPQUEUE_CLI_FP_STORAGE_GLUSTER_EDIT	153
#define APPQUEUE_CLI_FP_STORAGE_GLUSTER_DEL	154
#define APPQUEUE_CLI_FP_STORAGE_RAID_LIST	155
#define APPQUEUE_CLI_FP_STORAGE_RAID_CREATE	156
#define APPQUEUE_CLI_FP_STORAGE_RAID_DELETE	157
#define APPQUEUE_CLI_FP_STORAGE_RAID_RECOVER	158
#define APPQUEUE_CLI_FP_STORAGE_VSTORAGE_LIST	159
#define APPQUEUE_CLI_FP_STORAGE_VSTORAGE_AVAIL	160
#define APPQUEUE_CLI_FP_STORAGE_VSTORAGE_CREATE	161
#define APPQUEUE_CLI_FP_STORAGE_VSTORAGE_DELETE	162
#define APPQUEUE_CLI_FP_STORAGE_VSTORAGE_SNADD	163
#define APPQUEUE_CLI_FP_STORAGE_VSTORAGE_SNDEL	164
#define APPQUEUE_CLI_FP_STORAGE_VSTORAGE_ASSIGN	165
#define APPQUEUE_CLI_FP_STORAGE_VDISK_LIST	166
#define APPQUEUE_CLI_FP_STORAGE_VDISK_USAGE	167
#define APPQUEUE_CLI_FP_STORAGE_VDISK_CREATE	168
#define APPQUEUE_CLI_FP_STORAGE_VDISK_DELETE	169
#define APPQUEUE_CLI_FP_STORAGE_VDISK_CLONE	170
#define APPQUEUE_CLI_FP_STORAGE_VDISK_MAP	171
#define APPQUEUE_CLI_FP_STORAGE_VMEDIA_LIST	172
#define APPQUEUE_CLI_FP_STORAGE_VMEDIA_ADD	173
#define APPQUEUE_CLI_FP_STORAGE_VMEDIA_DEL	174
#define APPQUEUE_CLI_FP_STORAGE_VMEDIA_LICENSE	175
#define APPQUEUE_CLI_FP_VM_VDI			176
#define APPQUEUE_CLI_FP_VM_VDI_LIST		177
#define APPQUEUE_CLI_FP_VM_VDI_INSTALL		178
#define APPQUEUE_CLI_FP_VM_VDI_DEPLOY		179
#define APPQUEUE_CLI_FP_VM_VDI_REMOVE		180
#define APPQUEUE_CLI_FP_VM_VSRV_LIST		181
#define APPQUEUE_CLI_FP_VM_VSRV_INSTALL		182
#define APPQUEUE_CLI_FP_VM_VSRV_DEPLOY		183
#define APPQUEUE_CLI_FP_VM_VSRV_REMOVE		184
#define APPQUEUE_CLI_FP_NET_VPORTS		185
#define APPQUEUE_CLI_FP_NET_VPORTS_LIST		186
#define APPQUEUE_CLI_FP_NET_VPORTS_ADD		187
#define APPQUEUE_CLI_FP_NET_VPORTS_DEL		188
#define APPQUEUE_CLI_FP_NET_VPORTS_ASSIGN	189
#define APPQUEUE_CLI_FP_NET_VPORTS_UNASSIGN	190
#define APPQUEUE_CLI_FP_VM_VSRV			191

#define APPQUEUE_CLI_FP_END			192			/* this is the last item (must be unique to avoid seg faulting) */
#define APPQUEUE_MENU_MAX_CLI			APPQUEUE_CLI_FP_END
	
/* types */

typedef struct {
	char menu_cmd[APPQUEUE_MENU_MAX_CMDLEN];			/* menu command */
	char menu_desc[APPQUEUE_MENU_MAX_DESCLEN];			/* menu description */
	u32 menu_func;							/* menu function index */
	u8 menu_perms;							/* menu item minimum permission level */
} appqueue_cli_menu_entry_t;

typedef struct {
	appqueue_cli_menu_entry_t climenu[APPQUEUE_MENU_MAX_ITEMS];
	char cli_menu_title[APPQUEUE_MENU_MAX_DESCLEN];
	u8 index;							/* menu index */
} appqueue_cli_menu_t;

typedef struct {
	char app_mod[APPQUEUE_MAX_LEN_NAME];
	char url[APPQUEUE_MAX_LEN_URL];
	char cliurl[APPQUEUE_MAX_LEN_URL];
	char mgrurl[APPQUEUE_MAX_LEN_URL];
	char stamp[11];
	u32 size;
	u32 aindex;
	u8 valid;
} appqueue_options_entry_t;

typedef struct {
	appqueue_options_entry_t option[APPQUEUE_MAX_LEN_PARAM];
	u32 index;
} appqueue_options_t;

/* function prototypes */

/* appqueue_install.c */
void appqueue_install(void);
void appqueue_uninstall(void);

/* appqueue_cli.c */
void appqueue_cli(void);
int appqueue_cli_getch(int do_echo);
void appqueue_cli_loop(void); 
u8 appqueue_cli_auth(void);
void appqueue_cli_askq(char *askprompt, int do_echo, char *cmdline);
void appqueue_cli_init_main(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_info(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_info_l2(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_info_l3(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_info_l4(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_info_vm(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_apps(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_apps_config(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_vm(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_vm_vdi(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_vm_vsrv(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_net(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_net_netif(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_net_vns(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_net_fw(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_net_fw_filter(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_net_fw_nat(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_net_fw_mangle(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_net_vlan(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_net_vports(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_storage(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_storage_local(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_storage_iscsi(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_storage_iscsi_isns(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_storage_gluster(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_storage_raid(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_storage_vstorage(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_storage_vdisk(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_storage_vmedia(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_maint(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_boot(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_sys(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_sys_disk(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_sys_auth(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init_sys_clock(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appqueue_cli_init(void);
void appqueue_cli_mf_main(void);
void appqueue_cli_mf_info(void);
void appqueue_cli_mf_apps(void);
void appqueue_cli_mf_vm(void);
void appqueue_cli_mf_net(void);
void appqueue_cli_mf_sys(void);
void appqueue_cli_mf_setup(void);
void appqueue_cli_mf_maint(void);
void appqueue_cli_mf_boot(void);
void appqueue_cli_mf_exit(void);
void appqueue_cli_mf_info_sys(void);
void appqueue_cli_mf_info_link(void);
void appqueue_cli_mf_info_l2(void);
void appqueue_cli_mf_info_l3(void);
void appqueue_cli_mf_info_l2_vlan(void);
void appqueue_cli_mf_info_l2_vbridge(void);
void appqueue_cli_mf_info_l2_vmac(void);
void appqueue_cli_mf_info_l2_vmports(void);
void appqueue_cli_mf_info_l3_ip(void);
void appqueue_cli_mf_info_l3_route(void);
void appqueue_cli_mf_info_l3_arp(void);
void appqueue_cli_mf_info_l4(void);
void appqueue_cli_mf_info_l4_fw(void);
void appqueue_cli_mf_info_l4_vsip(void);
void appqueue_cli_mf_info_vm(void);
void appqueue_cli_mf_info_vm_fw(void);
void appqueue_cli_mf_info_vm_priority(void);
void appqueue_cli_mf_apps_list(void);
void appqueue_cli_mf_apps_avail(void);
void appqueue_cli_mf_apps_install(void);
void appqueue_cli_mf_apps_remove(void);
void appqueue_cli_mf_apps_showvm(void);
void appqueue_cli_mf_apps_createvm(void);
void appqueue_cli_mf_apps_deletevm(void);
void appqueue_cli_mf_apps_import(void);
void appqueue_cli_mf_apps_update(void);
void appqueue_cli_mf_apps_config(void);
void appqueue_cli_mf_apps_config_list(void);
void appqueue_cli_mf_apps_config_create(void);
void appqueue_cli_mf_apps_config_remove(void);
void appqueue_cli_mf_apps_config_app(void);
void appqueue_cli_mf_vm_list(void);
void appqueue_cli_mf_vm_deploy(void);
void appqueue_cli_mf_vm_start(void);
void appqueue_cli_mf_vm_stop(void);
void appqueue_cli_mf_vm_restart(void);
void appqueue_cli_mf_vm_killall(void);
void appqueue_cli_mf_vm_startall(void);
void appqueue_cli_mf_vm_remove(void);
void appqueue_cli_mf_vm_priority(void);
void appqueue_cli_mf_vm_import(void);
void appqueue_cli_mf_vm_vdi(void);
void appqueue_cli_mf_vm_vdi_list(void);
void appqueue_cli_mf_vm_vdi_install(void);
void appqueue_cli_mf_vm_vdi_deploy(void);
void appqueue_cli_mf_vm_vdi_remove(void);
void appqueue_cli_mf_vm_vsrv(void);
void appqueue_cli_mf_vm_vsrv_list(void);
void appqueue_cli_mf_vm_vsrv_install(void);
void appqueue_cli_mf_vm_vsrv_deploy(void);
void appqueue_cli_mf_vm_vsrv_remove(void);
void appqueue_cli_mf_net_vlan(void);
void appqueue_cli_mf_net_dhcp(void);
void appqueue_cli_mf_net_netif(void);
void appqueue_cli_mf_net_netif_addif(void);
void appqueue_cli_mf_net_netif_editif(void);
void appqueue_cli_mf_net_netif_rmif(void);
void appqueue_cli_mf_net_netif_ifmgr(void);
void appqueue_cli_mf_net_vlan_add(void);
void appqueue_cli_mf_net_vlan_edit(void);
void appqueue_cli_mf_net_vlan_delete(void);
void appqueue_cli_mf_net_vlan_list(void);
void appqueue_cli_mf_net_vns(void);
void appqueue_cli_mf_net_vns_addvsip(void);
void appqueue_cli_mf_net_vns_rmvsip(void);
void appqueue_cli_mf_net_vns_addvsp(void);
void appqueue_cli_mf_net_vns_rmvsp(void);
void appqueue_cli_mf_net_vns_ratelimit(void);
void appqueue_cli_mf_net_vns_state(void);
void appqueue_cli_mf_net_fw(void);
void appqueue_cli_mf_net_fw_zones(void);
void appqueue_cli_mf_net_fw_zones_list(void);
void appqueue_cli_mf_net_fw_zones_create(void);
void appqueue_cli_mf_net_fw_zones_edit(void);
void appqueue_cli_mf_net_fw_zones_delete(void);
void appqueue_cli_mf_net_fw_apps(void);
void appqueue_cli_mf_net_fw_apps_list(void);
void appqueue_cli_mf_net_fw_apps_create(void);
void appqueue_cli_mf_net_fw_apps_edit(void);
void appqueue_cli_mf_net_fw_apps_delete(void);
void appqueue_cli_mf_net_fw_nat(void);
void appqueue_cli_mf_net_fw_nat_add(void);
void appqueue_cli_mf_net_fw_nat_del(void);
void appqueue_cli_mf_net_fw_nat_edit(void);
void appqueue_cli_mf_net_fw_nat_list(void);
void appqueue_cli_mf_net_fw_nat_reverse(void);
void appqueue_cli_mf_net_fw_nat_log(void);
void appqueue_cli_mf_net_fw_filter(void);
void appqueue_cli_mf_net_fw_filter_add(void);
void appqueue_cli_mf_net_fw_filter_del(void);
void appqueue_cli_mf_net_fw_filter_edit(void);
void appqueue_cli_mf_net_fw_filter_list(void);
void appqueue_cli_mf_net_fw_filter_reverse(void);
void appqueue_cli_mf_net_fw_filter_log(void);
void appqueue_cli_mf_net_fw_mangle(void);
void appqueue_cli_mf_net_fw_mangle_add(void);
void appqueue_cli_mf_net_fw_mangle_del(void);
void appqueue_cli_mf_net_fw_mangle_edit(void);
void appqueue_cli_mf_net_fw_mangle_list(void);
void appqueue_cli_mf_net_fw_mangle_reverse(void);
void appqueue_cli_mf_net_fw_mangle_log(void);
void appqueue_cli_mf_net_vports(void);
void appqueue_cli_mf_net_vports_list(void);
void appqueue_cli_mf_net_vports_add(void);
void appqueue_cli_mf_net_vports_del(void);
void appqueue_cli_mf_net_vports_assign(void);
void appqueue_cli_mf_net_vports_unassign(void);
void appqueue_cli_mf_storage(void);
void appqueue_cli_mf_storage_local(void);
void appqueue_cli_mf_storage_local_list(void);
void appqueue_cli_mf_storage_iscsi(void);
void appqueue_cli_mf_storage_iscsi_isns(void);
void appqueue_cli_mf_storage_iscsi_isns_list(void);
void appqueue_cli_mf_storage_iscsi_isns_add(void);
void appqueue_cli_mf_storage_iscsi_isns_edit(void);
void appqueue_cli_mf_storage_iscsi_isns_del(void);
void appqueue_cli_mf_storage_iscsi_enable(void);
void appqueue_cli_mf_storage_iscsi_showtgt(void);
void appqueue_cli_mf_storage_iscsi_discovery(void);
void appqueue_cli_mf_storage_iscsi_isnsdisc(void);
void appqueue_cli_mf_storage_iscsi_addtgt(void);
void appqueue_cli_mf_storage_iscsi_deltgt(void);
void appqueue_cli_mf_storage_gluster(void);
void appqueue_cli_mf_storage_gluster_list(void);
void appqueue_cli_mf_storage_gluster_add(void);
void appqueue_cli_mf_storage_gluster_edit(void);
void appqueue_cli_mf_storage_gluster_del(void);
void appqueue_cli_mf_storage_raid(void);
void appqueue_cli_mf_storage_raid_list(void);
void appqueue_cli_mf_storage_raid_create(void);
void appqueue_cli_mf_storage_raid_delete(void);
void appqueue_cli_mf_storage_raid_recover(void);
void appqueue_cli_mf_storage_vstorage(void);
void appqueue_cli_mf_storage_vstorage_list(void);
void appqueue_cli_mf_storage_vstorage_avail(void);
void appqueue_cli_mf_storage_vstorage_create(void);
void appqueue_cli_mf_storage_vstorage_delete(void);
void appqueue_cli_mf_storage_vstorage_snapadd(void);
void appqueue_cli_mf_storage_vstorage_snapdel(void);
void appqueue_cli_mf_storage_vstorage_assign(void);
void appqueue_cli_mf_storage_vdisk(void);
void appqueue_cli_mf_storage_vdisk_list(void);
void appqueue_cli_mf_storage_vdisk_usage(void);
void appqueue_cli_mf_storage_vdisk_create(void);
void appqueue_cli_mf_storage_vdisk_delete(void);
void appqueue_cli_mf_storage_vdisk_clone(void);
void appqueue_cli_mf_storage_vdisk_map(void);
void appqueue_cli_mf_storage_vmedia(void);
void appqueue_cli_mf_storage_vmedia_list(void);
void appqueue_cli_mf_storage_vmedia_add(void);
void appqueue_cli_mf_storage_vmedia_del(void);
void appqueue_cli_mf_storage_vmedia_license(void);
void appqueue_cli_mf_sys_hostname(void);
void appqueue_cli_mf_sys_domain(void);
void appqueue_cli_mf_sys_dns(void);
void appqueue_cli_mf_sys_disk(void);
void appqueue_cli_mf_sys_disk_bootdisk(void);
void appqueue_cli_mf_sys_disk_swapdisk(void);
void appqueue_cli_mf_sys_disk_appqdisk(void);
void appqueue_cli_mf_sys_disk_datadisk(void);
void appqueue_cli_mf_sys_disk_fdisk(void);
void appqueue_cli_mf_sys_clock(void);
void appqueue_cli_mf_sys_clock_ntpa(void);
void appqueue_cli_mf_sys_clock_ntpb(void);
void appqueue_cli_mf_sys_clock_ntpc(void);
void appqueue_cli_mf_sys_clock_ntpint(void);
void appqueue_cli_mf_sys_auth(void);
void appqueue_cli_mf_sys_auth_aquser(void);
void appqueue_cli_mf_sys_auth_aqpass(void);
void appqueue_cli_mf_sys_auth_clipass(void);
void appqueue_cli_mf_sys_auth_root(void);
void appqueue_cli_mf_sys_auth_rootpass(void);
void appqueue_cli_mf_maint_shell(void);
void appqueue_cli_mf_maint_dbappq(void);
void appqueue_cli_mf_maint_dbkaos(void);
void appqueue_cli_mf_maint_dbvmsess(void);
void appqueue_cli_mf_maint_tsdump(void);
void appqueue_cli_mf_boot_status(void);
void appqueue_cli_mf_boot_upgrade(void);
void appqueue_cli_mf_boot_reboot(void);
void appqueue_cli_mf_boot_gtimg(void);
void appqueue_cli_mf_boot_factory(void);
void appqueue_cli_getinfo(void);
void appqueue_shm_init(void);
int appqueue_shm_open(char *kattach_shm_path); 
void appqueue_shm_close(void);
u32 appqueue_cli_parseip(char *dotted);
void appqueue_cli_setflags(u8 flag);
u8 appqueue_cli_exec(char *cmd);
void appqueue_cli_upd_dev(void);
void appqueue_cli_upd_cfg(void);
void appqueue_cli_upd_inst(void);
void appqueue_cli_upd_vmst(void);
void appqueue_cli_upd_vmports(void);
void appqueue_cli_upd_vbridge(void);
void appqueue_cli_upd_vmimages(void);
void appqueue_cli_upd_netdev(void);
void appqueue_cli_upd_vns(void);
void appqueue_cli_upd_cfggrp(void);
void appqueue_cli_upd_fw(void);
void appqueue_cli_lock_dev(void);
void appqueue_cli_lock_cfg(void);
void appqueue_cli_lock_inst(void);
void appqueue_cli_lock_vmst(void);
void appqueue_cli_lock_vmports(void);
void appqueue_cli_lock_vbridge(void);
void appqueue_cli_lock_vmimages(void);
void appqueue_cli_lock_netdev(void);
void appqueue_cli_lock_vns(void);
void appqueue_cli_lock_cfggrp(void);
void appqueue_cli_lock_fw(void);
void appqueue_cli_unlock_cfg(void);
u8 appqueue_install_gethv(char *clians, u8 slot);
u32 appqueue_cli_ntp_parseip(char *dotted, u8 ntp);
void appqueue_cli_getruuid(void);
unsigned long appqueue_cli_hash(char *str);
char *appqueue_cli_genpass(char *dfpass);
char *appqueue_cli_chkpass(char *dfpass, char *salty);
u8 appqueue_cli_mod(char *cmd, char *kargv);
void appqueue_get_applist(void);

/* globals */
char appqueue_cmd[255];
char appqueue_prompt[64];
char appqueue_cpu[256];
char appqueue_ruuid[40];                                 	/* string used to hold a random UUID */
u8 appqueue_proc;
u8 appqueue_cli_current_menu;
u8 appqueue_cli_user_auth;
u8 appqueue_exit;
u8 appqueue_virt;
u8 appqueue_po;
u16 appqueue_uptime;
appqueue_options_t appqueue_options;
appqueue_cli_menu_t appqueue_cli_menu_main;
appqueue_cli_menu_t appqueue_cli_menu_info;
appqueue_cli_menu_t appqueue_cli_menu_info_l2;
appqueue_cli_menu_t appqueue_cli_menu_info_l3;
appqueue_cli_menu_t appqueue_cli_menu_info_l4;
appqueue_cli_menu_t appqueue_cli_menu_info_vm;
appqueue_cli_menu_t appqueue_cli_menu_apps;
appqueue_cli_menu_t appqueue_cli_menu_apps_config;
appqueue_cli_menu_t appqueue_cli_menu_vm;
appqueue_cli_menu_t appqueue_cli_menu_vm_vdi;
appqueue_cli_menu_t appqueue_cli_menu_vm_vsrv;
appqueue_cli_menu_t appqueue_cli_menu_net;
appqueue_cli_menu_t appqueue_cli_menu_net_netif;
appqueue_cli_menu_t appqueue_cli_menu_net_vns;
appqueue_cli_menu_t appqueue_cli_menu_net_fw;
appqueue_cli_menu_t appqueue_cli_menu_net_fw_zones;
appqueue_cli_menu_t appqueue_cli_menu_net_fw_apps;
appqueue_cli_menu_t appqueue_cli_menu_net_fw_filter;
appqueue_cli_menu_t appqueue_cli_menu_net_fw_nat;
appqueue_cli_menu_t appqueue_cli_menu_net_fw_mangle;
appqueue_cli_menu_t appqueue_cli_menu_net_vlan;
appqueue_cli_menu_t appqueue_cli_menu_net_vports;
appqueue_cli_menu_t appqueue_cli_menu_storage;
appqueue_cli_menu_t appqueue_cli_menu_storage_local;
appqueue_cli_menu_t appqueue_cli_menu_storage_iscsi;
appqueue_cli_menu_t appqueue_cli_menu_storage_iscsi_isns;
appqueue_cli_menu_t appqueue_cli_menu_storage_gluster;
appqueue_cli_menu_t appqueue_cli_menu_storage_raid;
appqueue_cli_menu_t appqueue_cli_menu_storage_vstorage;
appqueue_cli_menu_t appqueue_cli_menu_storage_vdisk;
appqueue_cli_menu_t appqueue_cli_menu_storage_vmedia;
appqueue_cli_menu_t appqueue_cli_menu_sys;
appqueue_cli_menu_t appqueue_cli_menu_sys_clock;
appqueue_cli_menu_t appqueue_cli_menu_sys_auth;
appqueue_cli_menu_t appqueue_cli_menu_sys_disk;
appqueue_cli_menu_t appqueue_cli_menu_maint;
appqueue_cli_menu_t appqueue_cli_menu_boot;

void (*appqueue_xmenu[APPQUEUE_MENU_MAX_CLI]) (void);
void (*appqueue_cli_caller) (void);
