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

/* shared memory files */
#define KATTACH_SHM_FILE_Q              "/kaos_q.shm"
#define KATTACH_SHM_FILE_DEV            "/kaos_dev.shm"
#define KATTACH_SHM_FILE_CFG            "/kaos_cfg.shm"
#define KATTACH_SHM_FILE_INSTALL        "/kaos_install.shm"
#define KATTACH_SHM_FILE_VMST           "/kaos_vmst.shm"
#define KATTACH_SHM_FILE_VMPORTS        "/kaos_vmports.shm"
#define KATTACH_SHM_FILE_VBRIDGE        "/kaos_vbridge.shm"
#define KATTACH_SHM_FILE_VMIMAGES       "/kaos_vmimages.shm"
#define KATTACH_SHM_FILE_APPMODS	"/kaos_appmods.shm"
#define KATTACH_SHM_FILE_NETDEV		"/kaos_netdev.shm"
#define KATTACH_SHM_FILE_VNS		"/kaos_vns.shm"
#define KATTACH_SHM_FILE_CFGGRP		"/kaos_cfggrp.shm"
#define KATTACH_SHM_FILE_FW		"/kaos_fw.shm"

/* CM messaging mask */
#define CM_MSG_Q_FREE                   0x01
#define CM_MSG_Q_KATTACH_LOCK           0x02
#define CM_MSG_Q_KATTACH_UPDATED        0x04
#define CM_MSG_Q_APPQUEUE_LOCK          0x08
#define CM_MSG_Q_APPQUEUE_UPDATED       0x10
#define CM_MSG_Q_APPQUEUE_REBOOT	0x20
#define CM_MSG_Q_APPQUEUE_MODULE	0x40
#define CM_MSG_Q_KATTACH_MODULE		0x80

/* CM status messaging mask */
#define CM_MSG_ST_NONE			0x0000
#define CM_MSG_ST_RECOVERY		0x0001
#define CM_MSG_ST_INIT			0x0002
#define CM_MSG_ST_BOOTFS		0x0004
#define CM_MSG_ST_BOOTLOADER		0x0008
#define CM_MSG_ST_MBR			0x0010
#define CM_MSG_ST_SSHKEYS		0x0020
#define CM_MSG_ST_SSHD			0x0040
#define CM_MSG_ST_AQFS			0x0080
#define CM_MSG_ST_INITAQFS		0x0100
#define CM_MSG_ST_CONTENT		0x0200
#define CM_MSG_ST_INITVM		0x0400
#define CM_MSG_ST_INITSQL		0x0800
#define CM_MSG_ST_VSWITCH		0x1000
#define CM_MSG_ST_FIREWALL		0x2000
#define CM_MSG_ST_VIRT			0x4000
#define CM_MSG_ST_DONE			0x8000

/* App Modules State */
#define CM_APP_M_STATE_UNCHANGED	0x00
#define CM_APP_M_STATE_NEW		0x01
#define CM_APP_M_STATE_MODIFIED		0x02
#define CM_APP_M_STATE_DELETED		0x03

/* global file descriptors */
int kattach_fd_shm_q;                                   /* CM messaging queue */
int kattach_fd_shm_dev;                                 /* device tree */
int kattach_fd_shm_cfg;                                 /* config */
int kattach_fd_shm_install;                             /* installation */
int kattach_fd_shm_vmst;                                /* vmsession table */
int kattach_fd_shm_vmports;                             /* virtual port table */
int kattach_fd_shm_vbridge;                             /* virtual bridge information */
int kattach_fd_shm_vmimages;                            /* vmimages */
int kattach_fd_shm_appmods;				/* app modules */
int kattach_fd_shm_netdev;				/* netdev */
int kattach_fd_shm_vns;					/* virtual network services */
int kattach_fd_shm_cfggrp;				/* app module configuration groups */
int kattach_fd_shm_fw;					/* firewall configuration */
