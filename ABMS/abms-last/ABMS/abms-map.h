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
 */

/* note: stacks are executed as follows - /app/stack/production/[stackname]-[stackver]/sbin/asl */

#define ABMS_NET_MGMTMAX	8
#define ABMS_NET_PRODIPMAX	16

#define ABMS_SDT_MAX		32
#define ABMS_DMT_MAX		32
#define ABMS_LOOP_MAX		64
#define ABMS_MD_MAX		16

#define ABMS_DMT_LMAX		32
#define ABMS_DMT_LOMAX		32
#define ABMS_DMT_MDMAX		32
#define ABMS_DMT_NFSMAX		32
#define ABMS_DMT_LVMMAX		32

/* filesystem types */
#define ABMS_FS_UNKNOWN		0x00
#define ABMS_FS_SQFS		0x01
#define ABMS_FS_EXT2		0x02
#define ABMS_FS_EXT3		0x03
#define ABMS_FS_JFS		0x04
#define ABMS_FS_XFS		0x05
#define ABMS_FS_NFS		0x06
#define ABMS_FS_GFS		0x07

/* loop types */
#define ABMS_LOOP_T_UNKNOWN	0x00
#define ABMS_LOOP_T_STACK	0x01
#define ABMS_LOOP_T_CFG		0x02
#define ABMS_LOOP_T_COMMON	0x03
#define ABMS_LOOP_T_ODMT	0x04
#define ABMS_LOOP_T_SYSTEM	0xff

/* storage types */

#define ABMS_STORAGE_LOCAL	0x00		/* local disk - ATA/IDE, SCSI, HW RAID */
#define ABMS_STORAGE_LOOP	0x01		/* loopback device -- must be in /flash/storage/ */
#define ABMS_STORAGE_MD		0x02		/* MD device - eg. SW RAID */
#define ABMS_STORAGE_LVM	0x03		/* LVM device */
#define ABMS_STORAGE_NFS	0x04		/* NFS */
#define ABMS_STORAGE_GFS	0x05		/* Red Hat GFS - XXX */
#define ABMS_STORAGE_ISCSI	0x06		/* iSCSI - XXX */

typedef struct {
	char dev[32];
} abms_md_devlist_t;


typedef struct {
	u32 uid;		/* stack runs with uid */
	u32 gid;		/* stack runs with gid */
} abms_sdt_user_t;

typedef struct {
	u32 ip;			/* monitor stack on this IP address */
	u32 tcp;		/* tcp */
	u32 udp;		/* udp */
} abms_sdt_net_t;

/* the lmt indexes point to entries in the abms_loop_t */

typedef struct {
	u8 master;
	u8 support;
	u8 stack;				/* stack mapping */
	u8 common;				/* common mapping */
	u8 cfg;					/* config mapping */
	u8 data;				/* data mapping */
} abms_sdt_lmt_t;				/* loopback mapping table */

typedef struct {
	char name[128];				/* name of app (eg. sn_dns) */
	char ver[64];				/* version string */
} abms_sdt_app_t;

typedef struct {
	char profile[128];			/* profile name */
	u32 size;				/* size in M */
	u16 dataidx;				/* index into data table */
	u16 dataperm;				/* stack specific permissions in chmod 744 style format */
	u8 pread;
	u8 pwrite;
	u8 pexec;
} abms_sdt_data_t;

typedef struct {
	u8 index;
	abms_sdt_user_t user;
	abms_sdt_net_t net;
	abms_sdt_lmt_t loopmap;
	abms_sdt_app_t app;
	abms_sdt_data_t data;
} abms_sdt_e_t;					/* sdt entry */

typedef struct {
	u8 nxt_idx;				/* next available index */
	abms_sdt_e_t stack[ABMS_SDT_MAX];
} abms_sdt_t;					/* stack deployment table */

typedef struct {
	char sdev[16];
	u8 pread:1;
	u8 pwrite:1;
	u8 pexec:1;
	u8 atime:1;
	u8 spare:4;
} abms_dmt_local_e_t;

typedef struct {
	u16 lmidx;				/* index into loop table */
	u8 pread:1;
	u8 pwrite:1;
	u8 pexec:1;
	u8 atime:1;
	u8 spare:4;
} abms_dmt_loop_e_t;

typedef struct {
	char sdev[16];				/* md device name */
	u8 numdev;				/* number of devices */
	abms_md_devlist_t devlist[ABMS_MD_MAX]; /* dev list */
} abms_dmt_md_e_t;

typedef struct {
	char vg[128];
	char id[128];
	char src[128];
	u8 pread:1;
	u8 pwrite:1;
	u8 pexec:1;
	u8 atime:1;
	u8 spare:4;
	u16 sindex;				/* source index - indexes into dmt */
} abms_dmt_lvm_e_t;

typedef struct {
	u32 nfsip;
	u32 rsize;
	u32 wsize;
	u8 pread:1;
	u8 pwrite:1;
	u8 pexec:1;
	u8 atime:1;
	u8 tcp:1;
	u8 udp:1;
	u8 hard:1;
	u8 lock:1;
	u8 ver;
} abms_dmt_nfs_e_t;

typedef struct {
	char name[128];				/* profile name */
	u8 type;				/* type of storage - local, loop */
	u8 fs;					/* filesystem type */
	u16 index;				/* index into table */
} abms_dmt_e_t;

typedef struct {
	u8 nxt_idx;
	abms_dmt_e_t index[ABMS_DMT_MAX];
	abms_dmt_local_e_t local[ABMS_DMT_LMAX];
	abms_dmt_loop_e_t loop[ABMS_DMT_LOMAX];
	abms_dmt_md_e_t md[ABMS_DMT_MDMAX];
	abms_dmt_lvm_e_t lvm[ABMS_DMT_LVMMAX];
	abms_dmt_nfs_e_t nfs[ABMS_DMT_NFSMAX];
} abms_dmt_t;					/* data mount table */

typedef struct {
	u8 id;					/* loopback device id */
	u8 fs;					/* loopback fs type */
	u8 type;				/* type of loopback */
	u8 crypto;				/* encrypted - XXX */
	char src[128];				/* filename */
} abms_loop_e_t;

typedef struct {
	u8 nxt_idx;				/* next available index */
	u8 fr_idx;				/* index into free_ooo */
	u8 free_ooo[ABMS_LOOP_MAX];		/* table of freed indexes -- note: free on removal only */
	abms_loop_e_t map[ABMS_LOOP_MAX];	/* mapping table */
} abms_loop_t;

typedef struct {
	u32 mgmtip;
	u16 slash;
} abms_mgmtnet_t;

typedef struct {
	u8 nxt_idx;
	abms_mgmtnet_t mnet[ABMS_NET_MGMTMAX];
} abms_mgmt_t;

typedef struct {
	u8 mac[6];
	u32 gwip;
	u16 slash;
	u16 ipcount;
	u32 iplist[ABMS_NET_PRODIPMAX];
} abms_prodip_e_t;

typedef struct {
	u8 nxt_idx;
	abms_prodip_e_t ipnet[ABMS_NET_PRODIPMAX];
} abms_prodip_t;
