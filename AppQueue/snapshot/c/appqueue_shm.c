/*
 * AppQueue
 * Copyright (c) 2009 - 2010 Carbon Mountain LLC.
 * All Rights Reserved.
 *
 * John Buswell <buswellj@carbonmountain.com>
 * version 0.6.0
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

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "kattach_types.h"
#include "appqueue.h"
#include "kattach_shm.h"
#include "kattach_shm_globals.h"

void
appqueue_shm_init(void)
{
	kattach_fd_shm_q = appqueue_shm_open(KATTACH_SHM_FILE_Q);
        kattach_fd_shm_dev = appqueue_shm_open(KATTACH_SHM_FILE_DEV);
        kattach_fd_shm_cfg = appqueue_shm_open(KATTACH_SHM_FILE_CFG);
        kattach_fd_shm_install = appqueue_shm_open(KATTACH_SHM_FILE_INSTALL);
        kattach_fd_shm_vmst = appqueue_shm_open(KATTACH_SHM_FILE_VMST);
        kattach_fd_shm_vmports = appqueue_shm_open(KATTACH_SHM_FILE_VMPORTS);
        kattach_fd_shm_vbridge = appqueue_shm_open(KATTACH_SHM_FILE_VBRIDGE);
        kattach_fd_shm_vmimages = appqueue_shm_open(KATTACH_SHM_FILE_VMIMAGES);
	kattach_fd_shm_appmods = appqueue_shm_open(KATTACH_SHM_FILE_APPMODS);
	kattach_fd_shm_netdev = appqueue_shm_open(KATTACH_SHM_FILE_NETDEV);
	kattach_fd_shm_vns = appqueue_shm_open(KATTACH_SHM_FILE_VNS);
	kattach_fd_shm_cfggrp = appqueue_shm_open(KATTACH_SHM_FILE_CFGGRP);
	kattach_fd_shm_fw = appqueue_shm_open(KATTACH_SHM_FILE_FW);

        ftruncate(kattach_fd_shm_q, sizeof(cm_ak_ping_pong_t));
        ftruncate(kattach_fd_shm_dev, sizeof(kattach_dev_t));
        ftruncate(kattach_fd_shm_cfg, sizeof(kattach_cfg_t)); 
        ftruncate(kattach_fd_shm_install, sizeof(kattach_install_t));
        ftruncate(kattach_fd_shm_vmst, sizeof(kattach_vmst_t));
        ftruncate(kattach_fd_shm_vmports, sizeof(kattach_vmp_t));
        ftruncate(kattach_fd_shm_vbridge, sizeof(kattach_vbr_t));
        ftruncate(kattach_fd_shm_vmimages, sizeof(kattach_vmi_t));
	ftruncate(kattach_fd_shm_appmods, sizeof(kattach_am_t));
	ftruncate(kattach_fd_shm_netdev, sizeof(kattach_netdev_t));
	ftruncate(kattach_fd_shm_vns, sizeof(kattach_vns_t));
	ftruncate(kattach_fd_shm_cfggrp, sizeof(kattach_cfggrp_t));
	ftruncate(kattach_fd_shm_fw, sizeof(kattach_fw_t));

        if ((kattach_appqueue = mmap(0, sizeof(cm_ak_ping_pong_t), (PROT_READ | PROT_WRITE), MAP_SHARED, kattach_fd_shm_q, 0)) == MAP_FAILED) {
                printf("\n [!] FATAL - Unable to map messaging shm.\n");
        }

        if ((kattach_devices_shm = mmap(0, sizeof(kattach_dev_t), (PROT_READ | PROT_WRITE), MAP_SHARED, kattach_fd_shm_dev, 0)) == MAP_FAILED) {
                printf("\n [!] FATAL - Unable to map devices shm.\n");
        }

        if ((kattach_cfg_shm = mmap(0, sizeof(kattach_cfg_t), (PROT_READ | PROT_WRITE), MAP_SHARED, kattach_fd_shm_cfg, 0)) == MAP_FAILED) {
                printf("\n [!] FATAL - Unable to map config shm.\n");
        }

        if ((kattach_install_shm = mmap(0, sizeof(kattach_install_t), (PROT_READ | PROT_WRITE), MAP_SHARED, kattach_fd_shm_install, 0)) == MAP_FAILED) {
                printf("\n [!] FATAL - Unable to map installation shm.\n");
        }

        if ((kattach_vmst_shm = mmap(0, sizeof(kattach_vmst_t), (PROT_READ | PROT_WRITE), MAP_SHARED, kattach_fd_shm_vmst, 0)) == MAP_FAILED) {
                printf("\n [!] FATAL - Unable to map vmst shm.\n");
        }

        if ((kattach_vmports_shm = mmap(0, sizeof(kattach_vmp_t), (PROT_READ | PROT_WRITE), MAP_SHARED, kattach_fd_shm_vmports, 0)) == MAP_FAILED) {
                printf("\n [!] FATAL - Unable to map virtual ports shm.\n");
        }

        if ((kattach_vbridge_shm = mmap(0, sizeof(kattach_vbr_t), (PROT_READ | PROT_WRITE), MAP_SHARED, kattach_fd_shm_vbridge, 0)) == MAP_FAILED) {
                printf("\n [!] FATAL - Unable to map virtual bridge shm.\n");
        }

        if ((kattach_vmimages_shm = mmap(0, sizeof(kattach_vmi_t), (PROT_READ | PROT_WRITE), MAP_SHARED, kattach_fd_shm_vmimages, 0)) == MAP_FAILED) {
                printf("\n [!] FATAL - Unable to map vm images shm.\n");
        }

        if ((kattach_appmods_shm = mmap(0, sizeof(kattach_am_t), (PROT_READ | PROT_WRITE), MAP_SHARED, kattach_fd_shm_appmods, 0)) == MAP_FAILED) {
                printf("\n [!] FATAL - Unable to map app modules shm.\n");
        }

        if ((kattach_netdev_shm = mmap(0, sizeof(kattach_netdev_t), (PROT_READ | PROT_WRITE), MAP_SHARED, kattach_fd_shm_netdev, 0)) == MAP_FAILED) {
                printf("\n [!] FATAL - Unable to map netdev shm.\n");
        }

        if ((kattach_vns_shm = mmap(0, sizeof(kattach_vns_t), (PROT_READ | PROT_WRITE), MAP_SHARED, kattach_fd_shm_vns, 0)) == MAP_FAILED) {
                printf("\n [!] FATAL - Unable to map vns shm.\n");
        }

        if ((kattach_cfggrp_shm = mmap(0, sizeof(kattach_cfggrp_t), (PROT_READ | PROT_WRITE), MAP_SHARED, kattach_fd_shm_cfggrp, 0)) == MAP_FAILED) {
                printf("\n [!] FATAL - Unable to map cfggrp shm.\n");
        }

        if ((kattach_fw_shm = mmap(0, sizeof(kattach_fw_t), (PROT_READ | PROT_WRITE), MAP_SHARED, kattach_fd_shm_fw, 0)) == MAP_FAILED) {
                printf("\n [!] FATAL - Unable to map fw shm.\n");
        }

}

int
appqueue_shm_open(char *kattach_shm_path)
{
	int kattach_shm_fd = 0;

	if ((kattach_shm_fd = shm_open(kattach_shm_path, (O_CREAT | O_EXCL | O_RDWR), (S_IREAD | S_IWRITE))) > 0) {
                printf("\n [*] Opened - %s first \n",kattach_shm_path);
	} else if ((kattach_shm_fd = shm_open(kattach_shm_path, (O_CREAT | O_RDWR), (S_IREAD | S_IWRITE))) < 0) {
                printf("\n [!] Unexpected SHM Error - %s\n",kattach_shm_path);
                return 0;
        }

        return kattach_shm_fd;
}

void
appqueue_shm_close(void)
{
        /* move this to shutdown file */
        fsync(kattach_fd_shm_q);
        fsync(kattach_fd_shm_dev);  
        fsync(kattach_fd_shm_cfg);
        fsync(kattach_fd_shm_install);
        fsync(kattach_fd_shm_vmst);
        fsync(kattach_fd_shm_vmports);
        fsync(kattach_fd_shm_vbridge);
        fsync(kattach_fd_shm_vmimages);
        fsync(kattach_fd_shm_appmods);
        fsync(kattach_fd_shm_netdev);
	fsync(kattach_fd_shm_vns);
	fsync(kattach_fd_shm_cfggrp);
	fsync(kattach_fd_shm_fw);

        close(kattach_fd_shm_q);
        close(kattach_fd_shm_dev);
        close(kattach_fd_shm_cfg);
        close(kattach_fd_shm_install);
        close(kattach_fd_shm_vmst);
        close(kattach_fd_shm_vmports);
        close(kattach_fd_shm_vbridge);
        close(kattach_fd_shm_vmimages);
        close(kattach_fd_shm_appmods);
        close(kattach_fd_shm_netdev);
	close(kattach_fd_shm_vns);
	close(kattach_fd_shm_cfggrp);
	close(kattach_fd_shm_fw);
}

