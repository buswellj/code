/*
 * AppQueue
 * Copyright (c) 2009 - 2010 Carbon Mountain LLC.
 * All Rights Reserved.
 *
 * John Buswell <buswellj@carbonmountain.com>
 * version 0.6.1
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
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include "kattach_types.h"
#include "kattach_shm.h"
#include "kattach_shm_globals.h"
#include "appqueue.h"

void 
appqueue_cli_mf_storage(void)
{
        u8 i = 0;

        if (!appqueue_po) {
                printf("\n\n\n");
                printf("%s",appqueue_cli_menu_storage.cli_menu_title);
                printf("%s\n\n",APPQUEUE_MENU_BAR);

                for (i = 0; i < appqueue_cli_menu_storage.index; i++) {
                        printf("     %s  \t - %s\n",appqueue_cli_menu_storage.climenu[i].menu_cmd,appqueue_cli_menu_storage.climenu[i].menu_desc);
                }
        }

        printf("\n\n%sStorage]# ",appqueue_prompt);

        appqueue_po = 0;
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE;
	return;
}

void 
appqueue_cli_mf_storage_local(void)
{
        u8 i = 0;

        if (!appqueue_po) {
                printf("\n\n\n");
                printf("%s",appqueue_cli_menu_storage_local.cli_menu_title);
                printf("%s\n\n",APPQUEUE_MENU_BAR);

                for (i = 0; i < appqueue_cli_menu_storage_local.index; i++) {
                        printf("     %s  \t - %s\n",appqueue_cli_menu_storage_local.climenu[i].menu_cmd,appqueue_cli_menu_storage_local.climenu[i].menu_desc);
                }
        }

        printf("\n\n%sLocal Storage]# ",appqueue_prompt);

        appqueue_po = 0;
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_LOCAL;
	return;
}

void 
appqueue_cli_mf_storage_local_list(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_LOCAL;
	return;
}

void 
appqueue_cli_mf_storage_iscsi(void)
{
        u8 i = 0;

        if (!appqueue_po) {
                printf("\n\n\n");
                printf("%s",appqueue_cli_menu_storage_iscsi.cli_menu_title);
                printf("%s\n\n",APPQUEUE_MENU_BAR);

                for (i = 0; i < appqueue_cli_menu_storage_iscsi.index; i++) {
                        printf("     %s  \t - %s\n",appqueue_cli_menu_storage_iscsi.climenu[i].menu_cmd,appqueue_cli_menu_storage_iscsi.climenu[i].menu_desc);
                }
        }

        printf("\n\n%siSCSI]# ",appqueue_prompt);
        appqueue_po = 0;
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_ISCSI;
	return;
}

void
appqueue_cli_mf_storage_iscsi_isns(void)
{
        u8 i = 0;

        if (!appqueue_po) {
                printf("\n\n\n");
                printf("%s",appqueue_cli_menu_storage_iscsi_isns.cli_menu_title);
                printf("%s\n\n",APPQUEUE_MENU_BAR);

                for (i = 0; i < appqueue_cli_menu_storage_iscsi_isns.index; i++) {
                        printf("     %s  \t - %s\n",appqueue_cli_menu_storage_iscsi_isns.climenu[i].menu_cmd,appqueue_cli_menu_storage_iscsi_isns.climenu[i].menu_desc);
                }
        }

        printf("\n\n%siSNS]# ",appqueue_prompt);

        appqueue_po = 0;
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS;
	return;
}

void 
appqueue_cli_mf_storage_iscsi_isns_list(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS;
	return;
}

void 
appqueue_cli_mf_storage_iscsi_isns_add(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS;
	return;
}

void 
appqueue_cli_mf_storage_iscsi_isns_edit(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS;
	return;
}

void 
appqueue_cli_mf_storage_iscsi_isns_del(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS;
	return;
}

void 
appqueue_cli_mf_storage_iscsi_enable(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_ISCSI;
	return;
}

void 
appqueue_cli_mf_storage_iscsi_showtgt(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_ISCSI;
	return;
}

void 
appqueue_cli_mf_storage_iscsi_discovery(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_ISCSI;
	return;
}

void 
appqueue_cli_mf_storage_iscsi_isnsdisc(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_ISCSI;
	return;
}

void 
appqueue_cli_mf_storage_iscsi_addtgt(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_ISCSI;
	return;
}

void 
appqueue_cli_mf_storage_iscsi_deltgt(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_ISCSI;
	return;
}

void 
appqueue_cli_mf_storage_gluster(void)
{
        u8 i = 0;

        if (!appqueue_po) {
                printf("\n\n\n");
                printf("%s",appqueue_cli_menu_storage_gluster.cli_menu_title);
                printf("%s\n\n",APPQUEUE_MENU_BAR);

                for (i = 0; i < appqueue_cli_menu_storage_gluster.index; i++) {
                        printf("     %s  \t - %s\n",appqueue_cli_menu_storage_gluster.climenu[i].menu_cmd,appqueue_cli_menu_storage_gluster.climenu[i].menu_desc);
                }
        }

        printf("\n\n%sGlusterFS]# ",appqueue_prompt);

        appqueue_po = 0;
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_GLUSTER;
	return;
}

void 
appqueue_cli_mf_storage_gluster_list(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_GLUSTER;
	return;
}

void 
appqueue_cli_mf_storage_gluster_add(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_GLUSTER;
	return;
}

void 
appqueue_cli_mf_storage_gluster_edit(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_GLUSTER;
	return;
}

void 
appqueue_cli_mf_storage_gluster_del(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_GLUSTER;
	return;
}

void 
appqueue_cli_mf_storage_raid(void)
{
        u8 i = 0;

        if (!appqueue_po) {
                printf("\n\n\n");
                printf("%s",appqueue_cli_menu_storage_raid.cli_menu_title);
                printf("%s\n\n",APPQUEUE_MENU_BAR);

                for (i = 0; i < appqueue_cli_menu_storage_raid.index; i++) {
                        printf("     %s  \t - %s\n",appqueue_cli_menu_storage_raid.climenu[i].menu_cmd,appqueue_cli_menu_storage_raid.climenu[i].menu_desc);
                }
        }

        printf("\n\n%sRAID]# ",appqueue_prompt);
        appqueue_po = 0;
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_RAID;
	return;
}

void 
appqueue_cli_mf_storage_raid_list(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_RAID;
	return;
}

void 
appqueue_cli_mf_storage_raid_create(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_RAID;
	return;
}

void 
appqueue_cli_mf_storage_raid_delete(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_RAID;
	return;
}

void 
appqueue_cli_mf_storage_raid_recover(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_RAID;
	return;
}

void 
appqueue_cli_mf_storage_vstorage(void)
{
        u8 i = 0;

        if (!appqueue_po) {
                printf("\n\n\n");
                printf("%s",appqueue_cli_menu_storage_vstorage.cli_menu_title);
                printf("%s\n\n",APPQUEUE_MENU_BAR);

                for (i = 0; i < appqueue_cli_menu_storage_vstorage.index; i++) {
                        printf("     %s  \t - %s\n",appqueue_cli_menu_storage_vstorage.climenu[i].menu_cmd,appqueue_cli_menu_storage_vstorage.climenu[i].menu_desc);
                }
        }

        printf("\n\n%sVirtual Storage]# ",appqueue_prompt);
        appqueue_po = 0;

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VSTORAGE;
	return;
}

void 
appqueue_cli_mf_storage_vstorage_list(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VSTORAGE;
	return;
}

void 
appqueue_cli_mf_storage_vstorage_avail(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VSTORAGE;
	return;
}

void 
appqueue_cli_mf_storage_vstorage_create(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VSTORAGE;
	return;
}

void 
appqueue_cli_mf_storage_vstorage_delete(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VSTORAGE;
	return;
}

void 
appqueue_cli_mf_storage_vstorage_snapadd(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VSTORAGE;
	return;
}

void 
appqueue_cli_mf_storage_vstorage_snapdel(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VSTORAGE;
	return;
}

void 
appqueue_cli_mf_storage_vstorage_assign(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VSTORAGE;
	return;
}

void 
appqueue_cli_mf_storage_vdisk(void)
{
        u8 i = 0;

        if (!appqueue_po) {
                printf("\n\n\n");
                printf("%s",appqueue_cli_menu_storage_vdisk.cli_menu_title);
                printf("%s\n\n",APPQUEUE_MENU_BAR);

                for (i = 0; i < appqueue_cli_menu_storage_vdisk.index; i++) {
                        printf("     %s  \t - %s\n",appqueue_cli_menu_storage_vdisk.climenu[i].menu_cmd,appqueue_cli_menu_storage_vdisk.climenu[i].menu_desc);
                }
        }

        printf("\n\n%sVirtual Disks]# ",appqueue_prompt);

        appqueue_po = 0;
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VDISK;
	return;
}


void 
appqueue_cli_mf_storage_vdisk_list(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VDISK;
	return;
}

void 
appqueue_cli_mf_storage_vdisk_usage(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VDISK;
	return;
}

void 
appqueue_cli_mf_storage_vdisk_create(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VDISK;
	return;
}

void 
appqueue_cli_mf_storage_vdisk_delete(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VDISK;
	return;
}

void 
appqueue_cli_mf_storage_vdisk_clone(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VDISK;
	return;
}

void 
appqueue_cli_mf_storage_vdisk_map(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VDISK;
	return;
}

void 
appqueue_cli_mf_storage_vmedia(void)
{
        u8 i = 0;

        if (!appqueue_po) {
                printf("\n\n\n");
                printf("%s",appqueue_cli_menu_storage_vmedia.cli_menu_title);
                printf("%s\n\n",APPQUEUE_MENU_BAR);

                for (i = 0; i < appqueue_cli_menu_storage_vmedia.index; i++) {
                        printf("     %s  \t - %s\n",appqueue_cli_menu_storage_vmedia.climenu[i].menu_cmd,appqueue_cli_menu_storage_vmedia.climenu[i].menu_desc);
                }
        }

        printf("\n\n%sVirtual Media]# ",appqueue_prompt);
        appqueue_po = 0;
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VMEDIA;
	return;
}

void 
appqueue_cli_mf_storage_vmedia_list(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VMEDIA;
	return;
}

void 
appqueue_cli_mf_storage_vmedia_add(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VMEDIA;
	return;
}

void 
appqueue_cli_mf_storage_vmedia_del(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VMEDIA;
	return;
}

void 
appqueue_cli_mf_storage_vmedia_license(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VMEDIA;
	return;
}


