/*
 * kattach (kernel attach)
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
 * Source File :                kattach-loop.c
 * Purpose     :                main system loop
 * Callers     :                kattach.c
 *
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/time.h>
#include <linux/fs.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include "kattach_types.h"
#include "kattach.h"
#include "kattach_shm.h"
#include "kattach_shm_globals.h"

void
kattach_loop(void)
{
	u16 index = 0;
	unsigned int sleepy = 250000;
	int x = 0, ret = 0, len = 0, y = 0, m = 0, recovery = 0;
	char cmvend[64];
	char ccmd[64];
	char lcmd[255];
	char mntpt[64];
	char mbrd[32];

	/* Display IP information on the console */
	if (!kattach_setup) {
		printf("\n\nHypervisor is configured as:\n\n");
		sprintf(lcmd,"%s%s addr show | %s%s inet | %s%s -v inet6",KATTACH_BINPATH,KCMD_IP,KATTACH_BINPATH,KCMD_GREP,KATTACH_BINPATH,KCMD_GREP);
		kattach_sysexec(lcmd);
		printf("\n\n");
	}

	/* Launch initial consoles */
	for (y = 0; y <= 3; y++) {
		sprintf(ccmd,"%s%s",KATTACH_SBINPATH,KCMD_GETTY);
		sprintf(lcmd,"-L 115200 tty%u",(1 + y));
		kattach_gettypid[y] = kattach_bkexec(ccmd,lcmd);
	}


	while (!kattach_setup) {
		kattach_loop_monitor();
		kattach_sys_shm_checksync();		/* check for shared mem changes from AppQueue */
		if (kattach_change_detected) {
			if ((kattach_setup == 0) && (kattach_dbactive == 0)) {
				if (kattach_cfg.mode != KATTACH_MODE_NOTCFG) {
					kattach_appqueue->status = CM_MSG_ST_NONE;
					if (kattach_cfg.mode == KATTACH_MODE_SETUPRECOVERY) {
						kattach_cfg.mode = 1;
						recovery = 1;
						kattach_appqueue->status |= CM_MSG_ST_RECOVERY;
					} 
					if (recovery) {
						printf("\n\n [*] System entering recovery mode.\n");
					}
					/* mode change from first boot to production */
					printf("\n\n [*] System configured. Re-initializing system.\n");
					kattach_appqueue->status |= CM_MSG_ST_INIT;
					for (index = 0; index <= kattach_netdev.index; index++) {
						if (!strncmp(kattach_cfg.netdev,kattach_netdev.pif[index].devname,strlen(kattach_cfg.netdev))) {
							for (m = 0; m <= 5; m++) {
								kattach_cfg.mac[m] = kattach_netdev.pif[index].mac[m];
							}
							break;
						}
					}
					if (kattach_cfg.dns == 0) {
						kattach_cfg.dns = kattach_cfg.dns_ip[0];
					} 
					if (kattach_install.diskboot[0] != '\0') {
						if (!recovery) {
							kattach_appqueue->status |= CM_MSG_ST_BOOTFS;
							sprintf(lcmd,"%s%s -q -L kaosboot -t %s -m0 /dev/%s",KATTACH_SBINPATH,KCMD_MKFS,KATTACH_FS_EXT2,kattach_install.diskboot);
							kattach_sysexec(lcmd);
						}
						kattach_appqueue->status |= CM_MSG_ST_BOOTLOADER;
						/* install extlinux */
						ret = mkdir(KATTACH_BOOTPATH, KATTACH_PERM);
						sprintf(mntpt,"/dev/%s",kattach_install.diskboot);
						ret = mount(mntpt,KATTACH_BOOTPATH,KATTACH_FS_EXT2, MS_RELATIME, "");
						sprintf(lcmd,"%s%s --install %s",KATTACH_SBINPATH,KCMD_EXTLINUX,KATTACH_BOOTPATH);
						kattach_sysexec(lcmd);
						len = strlen(kattach_install.diskboot);
						for (x = 0; x <= strlen(kattach_install.diskboot); x++) {
							if (kattach_install.diskboot[x] <= 0x39) break;
							mbrd[x] = kattach_install.diskboot[x];
						}
						mbrd[x] = '\0';
						x = 0;
						kattach_appqueue->status |= CM_MSG_ST_MBR;
						sprintf(lcmd,"%s%s %s/mbr.bin > /dev/%s",KATTACH_BINPATH,KCMD_CAT,KATTACH_CFGPATH,mbrd);
						kattach_sys_write_extlinux();
						kattach_appqueue->status |= CM_MSG_ST_SSHKEYS;
						ret = mkdir(KATTACH_PSSHKEYPATH, KATTACH_PERM_SECURE);
						sprintf(lcmd,"%s%s -a %s%s %s",KATTACH_BINPATH,KCMD_CP,KATTACH_SSHKEYPATH,KATTACH_NET_RSAKEY,KATTACH_PSSHKEYPATH);
						kattach_sysexec(lcmd);
						sprintf(lcmd,"%s%s -a %s%s %s",KATTACH_BINPATH,KCMD_CP,KATTACH_SSHKEYPATH,KATTACH_NET_DSSKEY,KATTACH_PSSHKEYPATH);
						kattach_sysexec(lcmd);
						sync();
						kattach_appqueue->status |= CM_MSG_ST_SSHD;
						kattach_sys_sshd();
						sprintf(lcmd,"%s%s %s",KATTACH_BINPATH,KCMD_UMOUNT,mntpt);
						kattach_sysexec(lcmd);
					}
					if (kattach_install.diskappq[0] != '\0') {
						if (!recovery) {
							kattach_appqueue->status |= CM_MSG_ST_AQFS;
							sprintf(lcmd,"%s%s -q -L kaosappq -t %s -m0 -j /dev/%s",KATTACH_SBINPATH,KCMD_MKFS,KATTACH_FS_EXT4,kattach_install.diskappq);
							kattach_sysexec(lcmd);
						}
						kattach_appqueue->status |= CM_MSG_ST_INITAQFS;
						sprintf(kattach_cfg.storedev,"%s",kattach_install.diskappq);
						ret = mkdir(KATTACH_APPQUEUEPATH, KATTACH_PERM_GROUP);
						sprintf(mntpt,"/dev/%s",kattach_cfg.storedev);
						ret = mount(mntpt,KATTACH_APPQUEUEPATH,KATTACH_FS_APPQUEUE, MS_RELATIME, KATTACH_FS_EXT4);
						ret = chown(KATTACH_APPQUEUEPATH, KATTACH_UID_ROOT, KATTACH_GID_APPQ);
						ret = chmod(KATTACH_APPQUEUEPATH, KATTACH_PERM_GROUP);
						ret = mkdir(KATTACH_APPQUEUE_IMGPATH, KATTACH_PERM_GROUP);
						ret = chown(KATTACH_APPQUEUE_IMGPATH, KATTACH_UID_ROOT, KATTACH_GID_APPQ);
						ret = chmod(KATTACH_APPQUEUE_IMGPATH, KATTACH_PERM_GROUP);
                				ret = mkdir(KATTACH_APPQUEUE_APPMOD, KATTACH_PERM_GROUP);
                				ret = chown(KATTACH_APPQUEUE_APPMOD, KATTACH_UID_ROOT, KATTACH_GID_APPQ);
                				ret = chmod(KATTACH_APPQUEUE_APPMOD, KATTACH_PERM_GROUP);
                				ret = mkdir(KATTACH_APPQUEUE_APPMODCFG, KATTACH_PERM_GROUP);
                				ret = chown(KATTACH_APPQUEUE_APPMODCFG, KATTACH_UID_ROOT, KATTACH_GID_APPQ);
                				ret = chmod(KATTACH_APPQUEUE_APPMODCFG, KATTACH_PERM_GROUP);
                				sprintf(cmvend,"%s%s",KATTACH_APPQUEUE_APPMOD,KATTACH_APPQUEUE_VENDOR_CM);
                				ret = mkdir(cmvend, KATTACH_PERM_GROUP);
                				ret = chown(cmvend, KATTACH_UID_ROOT, KATTACH_GID_APPQ);
                				ret = chmod(cmvend, KATTACH_PERM_GROUP);
                				sprintf(cmvend,"%s%s",KATTACH_APPQUEUE_APPMOD,KATTACH_APPQUEUE_VENDOR_LOCAL);
                				ret = mkdir(cmvend, KATTACH_PERM_GROUP);
                				ret = chown(cmvend, KATTACH_UID_ROOT, KATTACH_GID_APPQ);
                				ret = chmod(cmvend, KATTACH_PERM_GROUP);
						ret = mkdir(KATTACH_APPQUEUE_BINPATH, KATTACH_PERM);
						ret = mkdir(KATTACH_APPQUEUE_DBPATH, KATTACH_PERM_SECURE_GRX);
						ret = chown(KATTACH_APPQUEUE_DBPATH, KATTACH_UID_ROOT, KATTACH_GID_APPQ);
						ret = mkdir(KATTACH_APPQUEUE_CFGPATH, KATTACH_PERM);
						ret = mkdir(KATTACH_APPQUEUE_VMDISKS, KATTACH_PERM_SECURE);
						ret = mkdir(KATTACH_APPQUEUE_RAWDISKS, KATTACH_PERM_SECURE);
		                		ret = mkdir(KATTACH_APPQUEUE_HVIMGPATH, KATTACH_PERM_GROUP);
						ret = chown(KATTACH_APPQUEUE_HVIMGPATH, KATTACH_UID_ROOT, KATTACH_GID_APPQ);
						ret = chmod(KATTACH_APPQUEUE_HVIMGPATH, KATTACH_PERM_GROUP);
						ret = mkdir(KATTACH_VKAOS_KERNELPATH, KATTACH_PERM_GROUP);
                				ret = chown(KATTACH_VKAOS_KERNELPATH, KATTACH_UID_ROOT, KATTACH_GID_APPQ);
                				ret = chmod(KATTACH_VKAOS_KERNELPATH, KATTACH_PERM_GROUP);
                				ret = mkdir(KATTACH_APPQUEUE_SVCPATH, KATTACH_PERM);
                				ret = mkdir(KATTACH_APPQUEUE_SVCCFGPATH, KATTACH_PERM);
                				ret = mkdir(KATTACH_APPQUEUE_RAWMOUNT, KATTACH_PERM_SECURE);
                				ret = mkdir(KATTACH_APPQUEUE_AQIPATH, KATTACH_PERM_SECURE);
					}
					if (kattach_install.diskdata[0] != '\0') {
						if (!recovery) {
							kattach_appqueue->status |= CM_MSG_ST_CONTENT;
							sprintf(lcmd,"%s%s -q -L kaosdata -t %s -m0 -j /dev/%s",KATTACH_SBINPATH,KCMD_MKFS,KATTACH_FS_EXT4,kattach_install.diskdata);
							kattach_sysexec(lcmd);
						}
					}
					if (kattach_install.diskswap[0] != '\0') {
						kattach_appqueue->status |= CM_MSG_ST_INITVM;
						sprintf(lcmd,"%s%s /dev/%s",KATTACH_SBINPATH,KCMD_MKSWAP,kattach_install.diskswap);
						kattach_sysexec(lcmd);
						sprintf(lcmd,"%s%s /dev/%s",KATTACH_SBINPATH,KCMD_SWAPON,kattach_install.diskswap);
						kattach_sysexec(lcmd);
					}
					kattach_setup = 1;
					kattach_appqueue->status |= CM_MSG_ST_INITSQL;
					kattach_sql_init();
					if (!recovery) {
						/* FIXME: check for differences */
						kattach_sql_insert_config();
					}
					kattach_thedawn_genauth();
					kattach_appqueue->status |= CM_MSG_ST_VSWITCH;
					kattach_vm_netdev_update();
					kattach_appqueue->status |= CM_MSG_ST_FIREWALL;
					kattach_loop_fw_defaults();
					kattach_appqueue->status |= CM_MSG_ST_VIRT;
					kattach_vm_launch();
					if (recovery) {
						recovery = 0;
						printf("\n\n [*] Recovery Complete. Install Hypervisor via gtimg. \n");
					}
					kattach_appqueue->status |= CM_MSG_ST_DONE;
				}
			}
		}
		if (kattach_reboot_me != 0) {
			kattach_reboot();
		}
		usleep(sleepy);
	}


	while (x == 0) {
		kattach_sys_shm_checksync();		/* check for shared mem changes from AppQueue */
		if (kattach_change_detected) {
			if (kattach_change_detected == (kattach_change_detected | KATTACH_UPD_DEVICES)) {
				/* devices change is for adding devices, not supported by AppQueue yet */
				kattach_change_detected ^= KATTACH_UPD_DEVICES;
			}
			if (kattach_change_detected == (kattach_change_detected | KATTACH_UPD_CONFIG)) {
				if (kattach_cfg.mode == 0xf1) {
					kattach_cfg.mode = 0x01;
					kattach_sys_hvimage();
				} else if (kattach_cfg.mode == 0xfc) {
					kattach_cfg.mode = 0x02;
					kattach_sys_hvimage();
				} else if (kattach_cfg.mode == 0xef) {
					ret = mkdir(KATTACH_BOOTPATH, KATTACH_PERM);
					sprintf(mntpt,"/dev/%s",kattach_install.diskboot);
					ret = mount(mntpt,KATTACH_BOOTPATH,KATTACH_FS_EXT2, MS_RELATIME, "");
					kattach_sys_write_extlinux();
					sprintf(lcmd,"%s%s %s",KATTACH_BINPATH,KCMD_UMOUNT,mntpt);
					kattach_sysexec(lcmd);					
				}
				memset(lcmd,0,sizeof(lcmd));
				sprintf(lcmd,"%s%s %s",KATTACH_BINPATH,KCMD_HOSTNAME,kattach_cfg.hostname);
				kattach_sysexec(lcmd);
				kattach_sys_write_resolv();
				kattach_sql_update_config();
				kattach_change_detected ^= KATTACH_UPD_CONFIG;
			}
			if (kattach_change_detected == (kattach_change_detected | KATTACH_UPD_INSTALL)) {
				kattach_sql_update_config();
				/* FIXME: Add support for users changing the partitions in runtime */
				/*        Need to prepare new partitions, umount old, mount new, restart */
				kattach_change_detected ^= KATTACH_UPD_INSTALL;
			}
			if (kattach_change_detected == (kattach_change_detected | KATTACH_UPD_VBRIDGE)) {
				/* someone added, edited or deleted a VLAN */
				kattach_vm_update_vbridge();
				kattach_change_detected ^= KATTACH_UPD_VBRIDGE;
			}
			if (kattach_change_detected == (kattach_change_detected | KATTACH_UPD_VMST)) {
				kattach_vm_check_vmst();
				kattach_change_detected ^= KATTACH_UPD_VMST;
			}
			if (kattach_change_detected == (kattach_change_detected | KATTACH_UPD_VMPORTS)) {
				/* vmport control is not supported in AppQueue yet */
				kattach_change_detected ^= KATTACH_UPD_VMPORTS;
			}
			if (kattach_change_detected == (kattach_change_detected | KATTACH_UPD_VMIMAGES)) {
				kattach_vm_vmi_update();
				kattach_change_detected ^= KATTACH_UPD_VMIMAGES;
			}
			if (kattach_change_detected == (kattach_change_detected | KATTACH_UPD_APPMODULES)) {
				kattach_vm_appmods();
				kattach_change_detected ^= KATTACH_UPD_APPMODULES;
			}
			if (kattach_change_detected == (kattach_change_detected | KATTACH_UPD_NETDEV)) {
				kattach_vm_netdev_update();
				kattach_change_detected ^= KATTACH_UPD_NETDEV;
			}
			if (kattach_change_detected == (kattach_change_detected | KATTACH_UPD_VNS)) {
				kattach_vm_vns_update();
				kattach_change_detected ^= KATTACH_UPD_VNS;
			}
			if (kattach_change_detected == (kattach_change_detected | KATTACH_UPD_CFGGRP)) {
				kattach_vm_cfggrp_update();
				kattach_change_detected ^= KATTACH_UPD_CFGGRP;
			}
			if (kattach_change_detected == (kattach_change_detected | KATTACH_UPD_FW)) {
				kattach_vm_fw_update();
				kattach_change_detected ^= KATTACH_UPD_FW;
			}

			kattach_change_detected = 0;
		}
		kattach_loop_monitor();
		kattach_vm_monitor();
		if (kattach_reboot_me != 0) {
			kattach_reboot();
		}
		usleep(sleepy);
	}
	return;

}

void
kattach_loop_monitor(void)
{
	FILE *stream;
	char dpid[8];
	char *pidval;
	char procpath[32];
	char ccmd[64];
	char lcmd[255];
	char dhcpdconf[255];
	struct stat k_procstatus;
	int x = 0, y = 0, res = 0, z = 0;
	struct timeval looptime;
	time_t looputime;

	/* FIXME: do we need a time check here to reduce the number of times we check this stuff?? */

	if (kattach_setup) {
		/* NTP sync */
		if (kattach_cfg.ntpint != 0) {
			if (kattach_ntppid != 0) {
				sprintf(procpath,"%s/%d",KATTACH_PROCPATH,kattach_ntppid);
				x = stat(procpath,&k_procstatus);
				if (x != 0) {
					/* ntp client has completed */
					kattach_ntppid = 0;
					sprintf(ccmd,"%s%s",KATTACH_SBINPATH,KCMD_HWCLOCK);
					sprintf(lcmd,"-w");
					z = kattach_bkexec(ccmd,lcmd);
				}
			} else {
				/* FIXME: we will move this outside the check when we have other things that need precision timing */
				gettimeofday(&looptime, NULL);				/* FIXME: this maybe excessive / expensive in the loop, look and see if we need to limit it */
				looputime = looptime.tv_sec;				/* unix time */
				if (kattach_ntpnext == 0) {
					/* first time */
					if (kattach_cfg.ntp_ip[0] != 0) {
						kattach_ntpnext = 1;
					} else if (kattach_cfg.ntp_ip[1] != 0) {
						kattach_ntpnext = 2;
					} else if (kattach_cfg.ntp_ip[2] != 0) {
						kattach_ntpnext = 3;
					} else {
						/* none of the NTP servers are configured, so flag it */
						kattach_ntpnext = 0xff;
						printf("\n [!] WARNING: No usable NTP servers \n");
					}
				}
				if ((kattach_ntptime == 0) || (kattach_ntptime <= (u32) looputime)) {
					if (kattach_ntpnext != 0xff) {
						sprintf(ccmd,"%s%s",KATTACH_SBINPATH,KCMD_NTP);
						sprintf(lcmd,"-c -n %lu.%lu.%lu.%lu",((kattach_cfg.ntp_ip[(kattach_ntpnext - 1)] >> 24) & 0xff),((kattach_cfg.ntp_ip[(kattach_ntpnext - 1)] >> 16) & 0xff),
											((kattach_cfg.ntp_ip[(kattach_ntpnext - 1)] >> 8) & 0xff),((kattach_cfg.ntp_ip[(kattach_ntpnext - 1)]) & 0xff));
						kattach_ntppid = kattach_bkexec(ccmd,lcmd);
					} else {
						kattach_ntpnext = 0;
					}
					kattach_ntptime = (kattach_cfg.ntpint * 60) + ((u32) looputime);
				}
			}
		}
		if (kattach_cfg.pid_dhcpd != 0) {
			sprintf(dhcpdconf,"%s%s",KATTACH_APPQUEUE_SVCCFGPATH,KATTACH_CONF_DHCPD);
			sprintf(procpath,"%s/%d",KATTACH_PROCPATH,kattach_cfg.pid_dhcpd);
			x = stat(procpath,&k_procstatus);
	
			if (x != 0) {
				/* dhcpd is not running on this pid */
				stream = fopen(KATTACH_CONF_DHCPD_PID,"r");
				if (stream == (FILE *) 0) {
					/* dhcpd is not running, restart it! */
					res = kill(kattach_cfg.pid_dhcpd, SIGKILL);
					sprintf(ccmd,"%s%s",KATTACH_APPQUEUE_DHCPPATH,KCMD_DHCPD);
					sprintf(lcmd,"-cf %s",dhcpdconf);
					kattach_cfg.pid_dhcpd = kattach_bkexec(ccmd,lcmd);
				} else {
					pidval = fgets(dpid,8,stream);
					if (pidval == NULL) {
						res = remove(KATTACH_CONF_DHCPD_PID);
						res = kill(kattach_cfg.pid_dhcpd, SIGKILL);
						sprintf(ccmd,"%s%s",KATTACH_APPQUEUE_DHCPPATH,KCMD_DHCPD);
						sprintf(lcmd,"-cf %s",dhcpdconf);
						kattach_cfg.pid_dhcpd = kattach_bkexec(ccmd,lcmd);
					} else {
						kattach_cfg.pid_dhcpd = atoi(pidval);
						sprintf(procpath,"%s/%d",KATTACH_PROCPATH,kattach_cfg.pid_dhcpd);
						x = stat(procpath,&k_procstatus);

						if (x != 0) {
							sprintf(ccmd,"%s%s",KATTACH_APPQUEUE_DHCPPATH,KCMD_DHCPD);
							sprintf(lcmd,"-cf %s",dhcpdconf);
							kattach_cfg.pid_dhcpd = kattach_bkexec(ccmd,lcmd);
						}
					}
					fclose(stream);
				}
			}
		}
	}

	for (y = 0; y <= 3; y++) {
		memset(procpath,0,sizeof(procpath));
		memset(lcmd,0,sizeof(lcmd));

		sprintf(procpath,"%s/%d",KATTACH_PROCPATH,kattach_gettypid[y]);
		x = stat(procpath,&k_procstatus);

		if (x != 0) {
			/* getty is not running, restart it! */
			sprintf(ccmd,"%s%s",KATTACH_SBINPATH,KCMD_GETTY);
			sprintf(lcmd,"-L 115200 tty%u",(1+y));
			kattach_gettypid[y] = kattach_bkexec(ccmd,lcmd);
		}
	}

	/* FIXME: Add dropbear to monitoring */

	/* monitor link status */
	kattach_netdev_checklink();

	return;
}

void
kattach_loop_fw_defaults(void)
{
	u32 index = 0, azindex = 0, outindex = 0, sshindex = 0, icmpindex = 0, icmpoutindex = 0, tftpindex = 0, ftpindex = 0;		/* FIXME: yeah this is thrown in here for 0.6.0 :) */
	kattach_fw_chain_t *chain;

	/* create some default zones */
	index = kattach_fw.zones.index;
	kattach_fw.zones.index++;

	sprintf(kattach_fw.zones.zone[index].name,"all");
	kattach_fw.zones.zone[index].vlan = 0;
	kattach_fw.zones.zone[index].nindex = 1;
	kattach_fw.zones.zone[index].node[0].ip = 0;			/* 0.0.0.0 */
	kattach_fw.zones.zone[index].node[0].mask = 0;			/* /0 */
	
	azindex = index;

	/* create some default apps */
	/* dst all - for outbound */
	index = kattach_fw.apps.index;
	kattach_fw.apps.index++;
	sprintf(kattach_fw.apps.app[index].name,"dst-all");
	kattach_fw.apps.app[index].statemask |= KATTACH_FW_STMASK_NEW;
	kattach_fw.apps.app[index].statemask |= KATTACH_FW_STMASK_ESTABLISHED;
	kattach_fw.apps.app[index].statemask |= KATTACH_FW_STMASK_RELATED;
	kattach_fw.apps.app[index].pindex = 1;
	kattach_fw.apps.app[index].port[0].direction = KATTACH_FW_DIR_DESTINATION;
	kattach_fw.apps.app[index].port[0].protmask |= KATTACH_FW_PROTOCOL_TCP;
	kattach_fw.apps.app[index].port[0].protmask |= KATTACH_FW_PROTOCOL_UDP;
	kattach_fw.apps.app[index].port[0].port[0] = 1;
	kattach_fw.apps.app[index].port[0].port[1] = 65535;
	outindex = index;

	/* ssh mgmt port */
	index = kattach_fw.apps.index;
	kattach_fw.apps.index++;
	sprintf(kattach_fw.apps.app[index].name,"mgmt-ssh");
	kattach_fw.apps.app[index].statemask |= KATTACH_FW_STMASK_NEW;
	kattach_fw.apps.app[index].statemask |= KATTACH_FW_STMASK_ESTABLISHED;
	kattach_fw.apps.app[index].statemask |= KATTACH_FW_STMASK_RELATED;
	kattach_fw.apps.app[index].pindex = 1;
	kattach_fw.apps.app[index].port[0].direction = KATTACH_FW_DIR_DESTINATION;
	kattach_fw.apps.app[index].port[0].protmask |= KATTACH_FW_PROTOCOL_TCP;
	kattach_fw.apps.app[index].port[0].port[0] = KATTACH_SSHPORT;
	kattach_fw.apps.app[index].port[0].port[1] = KATTACH_SSHPORT;
	sshindex = index;

	/* tftp related port */
	index = kattach_fw.apps.index;
	kattach_fw.apps.index++;
	sprintf(kattach_fw.apps.app[index].name,"mgmt-tftp");
	kattach_fw.apps.app[index].statemask |= KATTACH_FW_STMASK_NEW;
	kattach_fw.apps.app[index].statemask |= KATTACH_FW_STMASK_ESTABLISHED;
	kattach_fw.apps.app[index].statemask |= KATTACH_FW_STMASK_RELATED;
	kattach_fw.apps.app[index].pindex = 1;
	kattach_fw.apps.app[index].port[0].direction = KATTACH_FW_DIR_DESTINATION;
	kattach_fw.apps.app[index].port[0].protmask |= KATTACH_FW_PROTOCOL_UDP;
	kattach_fw.apps.app[index].port[0].port[0] = 69;
	kattach_fw.apps.app[index].port[0].port[1] = 69;
	tftpindex = index;

	/* ftp related port */
	index = kattach_fw.apps.index;
	kattach_fw.apps.index++;
	sprintf(kattach_fw.apps.app[index].name,"mgmt-ftp");
	kattach_fw.apps.app[index].statemask |= KATTACH_FW_STMASK_ESTABLISHED;
	kattach_fw.apps.app[index].statemask |= KATTACH_FW_STMASK_RELATED;
	kattach_fw.apps.app[index].pindex = 1;
	kattach_fw.apps.app[index].port[0].direction = KATTACH_FW_DIR_DESTINATION;
	kattach_fw.apps.app[index].port[0].protmask |= KATTACH_FW_PROTOCOL_TCP;
	kattach_fw.apps.app[index].port[0].port[0] = 20;
	kattach_fw.apps.app[index].port[0].port[1] = 21;
	ftpindex = index;

	/* icmp echo request - for inbound */
	index = kattach_fw.apps.index;
	kattach_fw.apps.index++;
	sprintf(kattach_fw.apps.app[index].name,"icmp-ping-in");
	kattach_fw.apps.app[index].statemask |= KATTACH_FW_STMASK_NEW;
	kattach_fw.apps.app[index].pindex = 1;
	kattach_fw.apps.app[index].port[0].direction = KATTACH_FW_DIR_DESTINATION;
	kattach_fw.apps.app[index].port[0].protmask |= KATTACH_FW_PROTOCOL_ICMP;
	kattach_fw.apps.app[index].port[0].port[0] = 8;
	kattach_fw.apps.app[index].port[0].port[1] = 8;
	icmpindex = index;

	/* icmp echo reply - for outbound */
	index = kattach_fw.apps.index;
	kattach_fw.apps.index++;
	sprintf(kattach_fw.apps.app[index].name,"icmp-ping-reply");
	kattach_fw.apps.app[index].statemask |= KATTACH_FW_STMASK_NEW;
	kattach_fw.apps.app[index].pindex = 1;
	kattach_fw.apps.app[index].port[0].direction = KATTACH_FW_DIR_DESTINATION;
	kattach_fw.apps.app[index].port[0].protmask |= KATTACH_FW_PROTOCOL_ICMP;
	kattach_fw.apps.app[index].port[0].port[0] = 0;
	kattach_fw.apps.app[index].port[0].port[1] = 0;
	icmpoutindex = index;

	/* this injects some default rules into the firewall */
	chain = &kattach_fw.filter.input;

	/* allow port 1289 */
	index = chain->index;
	if ((index == 0) && (chain->eindex == chain->hindex)) {
		/* this is the first rule */
		chain->eindex = index;
		chain->hindex = index;
		chain->filter[index].nindex = index;
		chain->filter[index].pindex = index;
	} else {
		/* goes after the last index */
		chain->filter[index].pindex = chain->eindex;
		chain->filter[index].nindex = index;
		chain->filter[chain->eindex].nindex = index;
		chain->eindex = index;
	}
	chain->index++;
	chain->filter[index].szindex = azindex;
	chain->filter[index].dzindex = azindex;
	chain->filter[index].appindex = sshindex;
	chain->filter[index].action = KATTACH_FW_ACTION_ALLOW;
	chain->filter[index].enabled = 1;
	chain->filter[index].type = KATTACH_FW_ORIG_SYSTEM;
	sprintf(chain->filter[index].logprefix,"kaos_mgmt_ssh: ");

	/* allow related ftp */
	index = chain->index;
	if ((index == 0) && (chain->eindex == chain->hindex)) {
		/* this is the first rule */
		chain->eindex = index;
		chain->hindex = index;
		chain->filter[index].nindex = index;
		chain->filter[index].pindex = index;
	} else {
		/* goes after the last index */
		chain->filter[index].pindex = chain->eindex;
		chain->filter[index].nindex = index;
		chain->filter[chain->eindex].nindex = index;
		chain->eindex = index;
	}
	chain->index++;
	chain->filter[index].szindex = azindex;
	chain->filter[index].dzindex = azindex;
	chain->filter[index].appindex = ftpindex;
	chain->filter[index].action = KATTACH_FW_ACTION_ALLOW;
	chain->filter[index].enabled = 1;
	chain->filter[index].type = KATTACH_FW_ORIG_SYSTEM;
	sprintf(chain->filter[index].logprefix,"kaos_mgmt_ftp: ");


	/* allow icmp echo request */
	index = chain->index;
	if ((index == 0) && (chain->eindex == chain->hindex)) {
		/* this is the first rule */
		chain->eindex = index;
		chain->hindex = index;
		chain->filter[index].nindex = index;
		chain->filter[index].pindex = index;
	} else {
		/* goes after the last index */
		chain->filter[index].pindex = chain->eindex;
		chain->filter[index].nindex = index;
		chain->filter[chain->eindex].nindex = index;
		chain->eindex = index;
	}
	chain->index++;
	chain->filter[index].szindex = azindex;
	chain->filter[index].dzindex = azindex;
	chain->filter[index].appindex = icmpindex;
	chain->filter[index].action = KATTACH_FW_ACTION_ALLOW;
	chain->filter[index].enabled = 1;
	chain->filter[index].type = KATTACH_FW_ORIG_SYSTEM;
	chain->filter[index].rlimitpkt = 0;
	chain->filter[index].rlimitint = 0;
	sprintf(chain->filter[index].logprefix,"kaos_mgmt_icmp: ");

	/* allow icmp echo reply */
	index = chain->index;
	if ((index == 0) && (chain->eindex == chain->hindex)) {
		/* this is the first rule */
		chain->eindex = index;
		chain->hindex = index;
		chain->filter[index].nindex = index;
		chain->filter[index].pindex = index;
	} else {
		/* goes after the last index */
		chain->filter[index].pindex = chain->eindex;
		chain->filter[index].nindex = index;
		chain->filter[chain->eindex].nindex = index;
		chain->eindex = index;
	}
	chain->index++;
	chain->filter[index].szindex = azindex;
	chain->filter[index].dzindex = azindex;
	chain->filter[index].appindex = icmpoutindex;
	chain->filter[index].action = KATTACH_FW_ACTION_ALLOW;
	chain->filter[index].enabled = 1;
	chain->filter[index].type = KATTACH_FW_ORIG_SYSTEM;
	chain->filter[index].rlimitpkt = 0;
	chain->filter[index].rlimitint = 0;
	sprintf(chain->filter[index].logprefix,"kaos_mgmt_icmp2: ");

	kattach_sys_shm_sync_fw();
	kattach_vm_fw_syncdb();
	return;
}

