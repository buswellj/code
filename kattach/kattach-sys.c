/*
 * kattach (kernel attach)
 * Copyright (c) 2009 Carbon Mountain LLC.
 * All Rights Reserved.
 *
 * John Buswell <buswellj@carbonmountain.com>
 * version 1.0
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
 * Source File :                kattach-sys.c
 * Purpose     :                system initialization
 * Callers     :                kattach.c -- main()
 *
 */

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mount.h>
#include <linux/fs.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "kattach_types.h"
#include "kattach.h"
#include "kattach_shm.h"
#include "kattach_shm_globals.h"

void
kattach_init_sys(void)
{
	int ret = 0;
#if !defined(KATTACH_BLD_VKAOS)
	char lcmd[255];
	char mntpt[64];
	char cmvend[64];
#endif /* !defined(KATTACH_BLD_VKAOS) */

	ret = mount(KATTACH_SYS, KATTACH_SYSPATH, KATTACH_SYSFS, MS_RELATIME, "");
	ret = mkdir(KATTACH_SSHCFGPATH, KATTACH_PERM);
	ret = mkdir(KATTACH_SSHKEYPATH, KATTACH_PERM_SECURE);

#if defined(KATTACH_BLD_VKAOS)
	kattach_init_vkaos();
#endif /* defined(KATTACH_BLD_VKAOS) */

	kattach_init_sshkey();
	kattach_init_sshd();

	printf("\n\n");

#if defined(KATTACH_BLD_VKAOS)
	printf("Virtual Kernel Attached Operating System (vKaOS)\n");
#else /* defined(KATTACH_BLD_VKAOS) */
	printf("Kernel Attached Operating System (KaOS)\n");
#endif /* defined(KATTACH_BLD_VKAOS) */

	printf("version %s\n",KATTACH_VERSION);
	printf("%s\n\n",KATTACH_COPYRIGHT);
	printf("%s\n\n",KATTACH_URL);

#if !defined(KATTACH_BLD_VKAOS)
	if ((kattach_cfg.mode != KATTACH_MODE_NOTCFG) && kattach_setup && (kattach_cfg.mode != KATTACH_MODE_VKAOS)) {
		if (kattach_cfg.storedev[0] != '\0') {
			sprintf(lcmd,"%s%s -p /dev/%s",KATTACH_SBINPATH,KCMD_FSCK,kattach_cfg.storedev);
			kattach_sysexec(lcmd);
		}
		ret = mkdir(KATTACH_APPQUEUEPATH, KATTACH_PERM_GROUP);
		sprintf(mntpt,"/dev/%s",kattach_cfg.storedev);
		ret = mount(mntpt,KATTACH_APPQUEUEPATH,KATTACH_FS_APPQUEUE, MS_RELATIME, KATTACH_FS_EXT4);
		ret = chown(KATTACH_APPQUEUEPATH, KATTACH_UID_ROOT, KATTACH_GID_APPQ);
		ret = chmod(KATTACH_APPQUEUEPATH, KATTACH_PERM_GROUP);
		/* we probably should check the result from the mount, but the rest do not matter, may fail if exists */
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
		kattach_sql_init();
		kattach_thedawn_genauth();
	}

	if (kattach_cfg.mode != KATTACH_MODE_VKAOS) {
		kattach_init_shm();
	}
#endif /* !defined(KATTACH_BLD_VKAOS) */

	return;

}

void
kattach_init_vkaos(void)
{
	int x = 0, ret = 0;
	char ctmp[32];

	/* generate devices for virtual disk device */

	kattach_thedawn_devlist_build("/dev/vda",253,0,KATTACH_PERM_BLK);
	for (x = 1; x <= 15; x++) {
		sprintf(ctmp,"/dev/vda%d",x);
		kattach_thedawn_devlist_build(ctmp,253,x,KATTACH_PERM_BLK);
	}

	kattach_thedawn_devlist_build("/dev/vdb",253,16,KATTACH_PERM_BLK);
	for (x = 17; x <= 31; x++) {
		sprintf(ctmp,"/dev/vdb%d",(x - 16));
		kattach_thedawn_devlist_build(ctmp,253,x,KATTACH_PERM_BLK);
	}

	kattach_thedawn_devlist_build("/dev/vdc",253,32,KATTACH_PERM_BLK);
	for (x = 33; x <= 47; x++) {
		sprintf(ctmp,"/dev/vdc%d",(x - 32));
		kattach_thedawn_devlist_build(ctmp,253,x,KATTACH_PERM_BLK);
	}

	ret = mkdir(KATTACH_VKAOS_APPPATH, KATTACH_PERM);
	sprintf(ctmp,"/dev/%s", KATTACH_VKAOS_APPDISK);
	/* FIXME: mount this read-only */
	ret = mount(ctmp,KATTACH_VKAOS_APPPATH,KATTACH_FS_APPQUEUE,MS_RELATIME,KATTACH_FS_EXT4);

	return;
}

void
kattach_init_sshkey(void)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s -t rsa -f %s%s",KATTACH_SSHPATH,KCMD_SSHKEYGEN,KATTACH_SSHKEYPATH,KATTACH_NET_RSAKEY);
	kattach_sysexec(lcmd);

	sprintf(lcmd,"%s%s -t dss -f %s%s",KATTACH_SSHPATH,KCMD_SSHKEYGEN,KATTACH_SSHKEYPATH,KATTACH_NET_DSSKEY);
	kattach_sysexec(lcmd);

	return;
}

void
kattach_init_sshd(void)
{
	char ecmd[64];
	char lcmd[255];

	sprintf(ecmd,"%s%s",KATTACH_SSHSPATH,KCMD_SSHD);
	sprintf(lcmd,"-F -r %s%s -d %s%s -I %u -p %d -E",KATTACH_SSHKEYPATH,KATTACH_NET_RSAKEY, KATTACH_SSHKEYPATH, KATTACH_NET_DSSKEY, KATTACH_NET_SSHTIMEOUT, KATTACH_SSHPORT);
	kattach_sshdpid = kattach_bkexec(ecmd, lcmd);

	return;
}

#if !defined(KATTACH_BLD_VKAOS)
void
kattach_sys_sshd(void)
{
	char ecmd[64];
	char lcmd[255];
	int ret = 0, xstat = 0, rc = 0, kwaitcnt = 0;
	struct stat k_procstatus;

	sprintf(ecmd,"%s/%u",KATTACH_PROCPATH, kattach_sshdpid);
	ret = kill(kattach_sshdpid, SIGKILL);

	while((xstat = stat(ecmd,&k_procstatus)) == 0) {
		kwaitcnt++;
		rc = usleep(KATTACH_TIMER_QEMU_SPINUP);
		/* FIXME: ejection seat error handling */
		if (kwaitcnt > 40) {
			printf("\n\n [!] WARNING Unable to kill sshd. \n\n");
			break;
		}
	}

	/* FIXME: make sshport configurable */

	sprintf(ecmd,"%s%s",KATTACH_SSHSPATH,KCMD_SSHD);
	sprintf(lcmd,"-F -r %s%s -d %s%s -I %u -p %d -E",KATTACH_PSSHKEYPATH,KATTACH_NET_RSAKEY, KATTACH_PSSHKEYPATH, KATTACH_NET_DSSKEY, KATTACH_NET_SSHTIMEOUT, KATTACH_SSHPORT);
	kattach_sshdpid = kattach_bkexec(ecmd, lcmd);

	return;	
}

void
kattach_init_shm(void)
{
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
		printf("\n [!] FATAL - Unable to map network interfaces shm.\n");
        }

	if ((kattach_vns_shm = mmap(0, sizeof(kattach_vns_t), (PROT_READ | PROT_WRITE), MAP_SHARED, kattach_fd_shm_vns, 0)) == MAP_FAILED) {
		printf("\n [!] FATAL - Unable to map virtual network services shm.\n");
        }

	if ((kattach_cfggrp_shm = mmap(0, sizeof(kattach_cfggrp_t), (PROT_READ | PROT_WRITE), MAP_SHARED, kattach_fd_shm_cfggrp, 0)) == MAP_FAILED) {
		printf("\n [!] FATAL - Unable to map App Config Group shm.\n");
        }

	if ((kattach_fw_shm = mmap(0, sizeof(kattach_fw_t), (PROT_READ | PROT_WRITE), MAP_SHARED, kattach_fd_shm_fw, 0)) == MAP_FAILED) {
		printf("\n [!] FATAL - Unable to map firewall shm.\n");
        }

	return;
}

void
kattach_sys_shm_sync_devices(void)
{
	u32 index = 0;

	/* devices_t */
	kattach_devices_shm->index = kattach_devices.index;
	for (index = 0; index <= kattach_devices.index; index++) {
		kattach_devices_shm->device[index].devtype = kattach_devices.device[index].devtype;
		kattach_devices_shm->device[index].major = kattach_devices.device[index].major;
		kattach_devices_shm->device[index].minor = kattach_devices.device[index].minor;
		kattach_devices_shm->device[index].res = kattach_devices.device[index].res;
		sprintf(kattach_devices_shm->device[index].devname,"%s",kattach_devices.device[index].devname);
	}

	return;

}

void
kattach_sys_shm_syncback_devices(void)
{
	u32 index = 0;

	/* devices_t */
	kattach_devices.index = kattach_devices_shm->index;
	for (index = 0; index <= kattach_devices_shm->index; index++) {
		kattach_devices.device[index].devtype = kattach_devices_shm->device[index].devtype;
		kattach_devices.device[index].major = kattach_devices_shm->device[index].major;
		kattach_devices.device[index].minor = kattach_devices_shm->device[index].minor;
		kattach_devices.device[index].res = kattach_devices_shm->device[index].res;
		sprintf(kattach_devices.device[index].devname,"%s",kattach_devices_shm->device[index].devname);
	}

	return;

}


void
kattach_sys_shm_sync_cfg(void)
{
	u32 index = 0;

	/* cfg_t */
	kattach_cfg_shm->ip = kattach_cfg.ip;
	kattach_cfg_shm->gw = kattach_cfg.gw;
	kattach_cfg_shm->dns = kattach_cfg.dns;
	kattach_cfg_shm->slash = kattach_cfg.slash;
	kattach_cfg_shm->mode = kattach_cfg.mode;
	kattach_cfg_shm->dhcp = kattach_cfg.dhcp;
	kattach_cfg_shm->pid_dhcpd = kattach_cfg.pid_dhcpd;
	kattach_cfg_shm->ntpint = kattach_cfg.ntpint;
	kattach_cfg_shm->root = kattach_cfg.root;
	/* note: we do not sync rootpass, rootpwck, aquser or aqpass */
	sprintf(kattach_cfg_shm->clipass,"%s",kattach_cfg.clipass);
	sprintf(kattach_cfg_shm->hostname,"%s",kattach_cfg.hostname);
	sprintf(kattach_cfg_shm->domain,"%s",kattach_cfg.domain);
	sprintf(kattach_cfg_shm->netdev,"%s",kattach_cfg.netdev);
	sprintf(kattach_cfg_shm->storedev,"%s",kattach_cfg.storedev);
	for (index = 0; index <= 5; index++) {
		kattach_cfg_shm->mac[index] = kattach_cfg.mac[index];
		kattach_cfg_shm->dns_ip[index] = kattach_cfg.dns_ip[index];
		if (index < 3) {
			kattach_cfg_shm->ntp_ip[index] = kattach_cfg.ntp_ip[index];
		}
	}

	return;
}

void
kattach_sys_shm_syncback_cfg(void)
{
	u32 index = 0;

	/* cfg_t */
	kattach_cfg.ip = kattach_cfg_shm->ip;
	kattach_cfg.gw = kattach_cfg_shm->gw;
	kattach_cfg.dns = kattach_cfg_shm->dns;
	kattach_cfg.slash = kattach_cfg_shm->slash;
	kattach_cfg.mode = kattach_cfg_shm->mode;
	kattach_cfg.dhcp = kattach_cfg_shm->dhcp;
	kattach_cfg.pid_dhcpd = kattach_cfg_shm->pid_dhcpd;
	kattach_cfg.ntpint = kattach_cfg_shm->ntpint;
	sprintf(kattach_cfg.hostname,"%s",kattach_cfg_shm->hostname);
	sprintf(kattach_cfg.domain,"%s",kattach_cfg_shm->domain);
	sprintf(kattach_cfg.netdev,"%s",kattach_cfg_shm->netdev);
	sprintf(kattach_cfg.storedev,"%s",kattach_cfg_shm->storedev);
	for (index = 0; index <= 5; index++) {
		kattach_cfg.mac[index] = kattach_cfg_shm->mac[index];
		kattach_cfg.dns_ip[index] = kattach_cfg_shm->dns_ip[index];
		if (index < 3) {
			kattach_cfg.ntp_ip[index] = kattach_cfg_shm->ntp_ip[index];
		}
	}
	/* we only sync the following if they are not NULL */
	index = 0;
	if (kattach_cfg.root != kattach_cfg_shm->root) {
		kattach_cfg.root = kattach_cfg_shm->root;
		index++;
	}
	if (kattach_cfg_shm->aquser[0] != '\0') {
		sprintf(kattach_cfg.aquser,"%s",kattach_cfg_shm->aquser);
		memset(kattach_cfg_shm->aquser,0,strlen(kattach_cfg_shm->aquser));
		index++;
	}
	if (kattach_cfg_shm->aqpass[0] != '\0') {
		sprintf(kattach_cfg.aqpass,"%s",kattach_cfg_shm->aqpass);
		memset(kattach_cfg_shm->aqpass,0,strlen(kattach_cfg_shm->aqpass));
		index++;
	}
	if (kattach_cfg_shm->clipass[0] != '\0') {
		sprintf(kattach_cfg.clipass,"%s",kattach_cfg_shm->clipass);
	}
	if (kattach_cfg_shm->rootpass[0] != '\0') {
		/* root check */
		if (kattach_cfg_shm->rootpwck[0] != '\0') {
			/* 
			 * the shared mem rootpwck has the unencrypted password in it. use it via crypt to authenticate it as the current root password.
			 * the resulting encrypted password should be the same as whats stored as the encrypted root password in kattach_cfg.rootpass.
			 * this is stored in kattach_cfg.rootpwck, then compared. if it doesn't match, then the user provided the wrong current root and we throw a message.
			 */
			sprintf(kattach_cfg.rootpwck,"%s",kattach_thedawn_chkpass(kattach_cfg_shm->rootpwck,kattach_cfg.rootpass));
			if ((!strncmp(kattach_cfg.rootpass,kattach_cfg.rootpwck,strlen(kattach_cfg.rootpass))) && (strlen(kattach_cfg.rootpass) == strlen(kattach_cfg.rootpwck))) {
				/* current password matches what we have, proceed with changes */
				sprintf(kattach_cfg.rootpass,"%s",kattach_cfg_shm->rootpass);
				if (kattach_cfg.root) {
					index++;			/* only trigger an update if root is active */
				}
			} else if ((kattach_cfg.rootpass[0] == '\0') && (strlen(kattach_cfg_shm->rootpass) > 3)) {
				sprintf(kattach_cfg.rootpass,"%s",kattach_cfg_shm->rootpass);
				if (kattach_cfg.root) {
					index++;			/* only trigger an update if root is active */
				}
			} else {
				printf("\n [!] WARNING: Attempted root password change failed! \n");
			}
		}
		/* clean up */
		memset(kattach_cfg_shm->rootpass,0,strlen(kattach_cfg_shm->rootpass));
		memset(kattach_cfg_shm->rootpwck,0,strlen(kattach_cfg_shm->rootpwck));
		memset(kattach_cfg.rootpwck,0,strlen(kattach_cfg.rootpwck));
	}

	if (index) {
		/* something has triggered an update of the passwd and shadow files */
		kattach_thedawn_genauth();
	}

	return;
}

void
kattach_sys_shm_sync_install(void)
{
	/* install_t */
	sprintf(kattach_install_shm->diskboot,"%s",kattach_install.diskboot);
	sprintf(kattach_install_shm->diskswap,"%s",kattach_install.diskswap);
	sprintf(kattach_install_shm->diskappq,"%s",kattach_install.diskappq);
	sprintf(kattach_install_shm->diskdata,"%s",kattach_install.diskdata);
	return;
}

void
kattach_sys_shm_syncback_install(void)
{
	/* install_t */
	sprintf(kattach_install.diskboot,"%s",kattach_install_shm->diskboot);
	sprintf(kattach_install.diskswap,"%s",kattach_install_shm->diskswap);
	sprintf(kattach_install.diskappq,"%s",kattach_install_shm->diskappq);
	sprintf(kattach_install.diskdata,"%s",kattach_install_shm->diskdata);
	return;
}

void
kattach_sys_shm_sync_vmst(void)
{
	u32 index = 0, yindex = 0;

	/* vmst_t */
	kattach_vmst_shm->index = kattach_vmst.index;
	for (index = 0; index < kattach_vmst.index; index++) {
		kattach_vmst_shm->vmsess[index].vmimage = kattach_vmst.vmsess[index].vmimage;
		kattach_vmst_shm->vmsess[index].vmem = kattach_vmst.vmsess[index].vmem;
		kattach_vmst_shm->vmsess[index].vcpu = kattach_vmst.vmsess[index].vcpu;
		kattach_vmst_shm->vmsess[index].vmstatus = kattach_vmst.vmsess[index].vmstatus;
		kattach_vmst_shm->vmsess[index].vmoper = kattach_vmst.vmsess[index].vmoper;
		kattach_vmst_shm->vmsess[index].vmpidx = kattach_vmst.vmsess[index].vmpidx;
		kattach_vmst_shm->vmsess[index].vpid = kattach_vmst.vmsess[index].vpid;
		kattach_vmst_shm->vmsess[index].priority = kattach_vmst.vmsess[index].priority;
		sprintf(kattach_vmst_shm->vmsess[index].vmname,"%s",kattach_vmst.vmsess[index].vmname);
		for (yindex = 0; yindex <= KATTACH_MAX_VPORTS; yindex++) {
			kattach_vmst_shm->vmsess[index].vmport[yindex] = kattach_vmst.vmsess[index].vmport[yindex];
		}
	}
	return;

}

void
kattach_sys_shm_syncback_vmst(void)
{
	u32 index = 0, yindex = 0;

	/* vmst_t */
	kattach_vmst.index = kattach_vmst_shm->index;
	for (index = 0; index < kattach_vmst_shm->index; index++) {
		kattach_vmst.vmsess[index].vmimage = kattach_vmst_shm->vmsess[index].vmimage;
		kattach_vmst.vmsess[index].vmem = kattach_vmst_shm->vmsess[index].vmem;
		kattach_vmst.vmsess[index].vcpu = kattach_vmst_shm->vmsess[index].vcpu;
		kattach_vmst.vmsess[index].vmstatus = kattach_vmst_shm->vmsess[index].vmstatus;
		kattach_vmst.vmsess[index].vmoper = kattach_vmst_shm->vmsess[index].vmoper;
		kattach_vmst.vmsess[index].vmpidx = kattach_vmst_shm->vmsess[index].vmpidx;
		kattach_vmst.vmsess[index].vpid = kattach_vmst_shm->vmsess[index].vpid;
		if (kattach_vmst.vmsess[index].priority != kattach_vmst_shm->vmsess[index].priority) {
			kattach_vmst.vmsess[index].priority = kattach_vmst_shm->vmsess[index].priority; 
			/* new priority for this VM, so lets renice it and update the db */
			if (kattach_vmst.vmsess[index].vpid) {
				kattach_vm_priority_update(index);
			}					
		}
		sprintf(kattach_vmst.vmsess[index].vmname,"%s",kattach_vmst_shm->vmsess[index].vmname);
		for (yindex = 0; yindex <= KATTACH_MAX_VPORTS; yindex++) {
			kattach_vmst.vmsess[index].vmport[yindex] = kattach_vmst_shm->vmsess[index].vmport[yindex];
		}
	}
	return;

}

void
kattach_sys_shm_sync_vmp(void)
{
	u32 index = 0, yindex = 0;

	/* vmp_t */
	kattach_vmports_shm->index = kattach_vmports.index;
	for (index = 0; index < kattach_vmports.index; index++) {
		kattach_vmports_shm->vmports[index].vmst = kattach_vmports.vmports[index].vmst;
		kattach_vmports_shm->vmports[index].vmpip = kattach_vmports.vmports[index].vmpip;
		kattach_vmports_shm->vmports[index].vbridge = kattach_vmports.vmports[index].vbridge;
		for (yindex = 0; yindex <= 5; yindex++) {
			kattach_vmports_shm->vmports[index].vmac[yindex] = kattach_vmports.vmports[index].vmac[yindex];
		}
	}
	return;
}

void
kattach_sys_shm_syncback_vmp(void)
{
	u32 index = 0, yindex = 0;

	/* vmp_t */
	kattach_vmports.index = kattach_vmports_shm->index;
	for (index = 0; index < kattach_vmports_shm->index; index++) {
		kattach_vmports.vmports[index].vmst = kattach_vmports_shm->vmports[index].vmst;
		kattach_vmports.vmports[index].vmpip = kattach_vmports_shm->vmports[index].vmpip;
		kattach_vmports.vmports[index].vbridge = kattach_vmports_shm->vmports[index].vbridge;
		for (yindex = 0; yindex <= 5; yindex++) {
			kattach_vmports.vmports[index].vmac[yindex] = kattach_vmports_shm->vmports[index].vmac[yindex];
		}
	}
	return;
}

void
kattach_sys_shm_sync_vbr(void)
{
	u32 index = 0, yindex = 0;

	/* vbr_t */
	kattach_vbridge_shm->index = kattach_vbridge.index;
	for (index = 0; index < kattach_vbridge.index; index++) {
		kattach_vbridge_shm->vbridge[index].vsubnet = kattach_vbridge.vbridge[index].vsubnet;
		kattach_vbridge_shm->vbridge[index].vbrip = kattach_vbridge.vbridge[index].vbrip;
		kattach_vbridge_shm->vbridge[index].vlan = kattach_vbridge.vbridge[index].vlan;
		kattach_vbridge_shm->vbridge[index].vmask = kattach_vbridge.vbridge[index].vmask;
		kattach_vbridge_shm->vbridge[index].vpfree = kattach_vbridge.vbridge[index].vpfree;
		kattach_vbridge_shm->vbridge[index].vbruse = kattach_vbridge.vbridge[index].vbruse;
		kattach_vbridge_shm->vbridge[index].vbrlocal = kattach_vbridge.vbridge[index].vbrlocal;
		kattach_vbridge_shm->vbridge[index].state = kattach_vbridge.vbridge[index].state;
		sprintf(kattach_vbridge_shm->vbridge[index].vlanext,"%s", kattach_vbridge.vbridge[index].vlanext);
		for (yindex = 0; yindex <= 5; yindex++) {
			kattach_vbridge_shm->vbridge[index].bmac[yindex] = kattach_vbridge.vbridge[index].bmac[yindex];
		}
	}
	return;
}

void
kattach_sys_shm_syncback_vbr(void)
{
	u32 index = 0, yindex = 0;

	/* vbr_t */
	kattach_vbridge.index = kattach_vbridge_shm->index;
	for (index = 0; index < kattach_vbridge_shm->index; index++) {
		kattach_vbridge.vbridge[index].vsubnet = kattach_vbridge_shm->vbridge[index].vsubnet;
		kattach_vbridge.vbridge[index].vbrip = kattach_vbridge_shm->vbridge[index].vbrip;
		kattach_vbridge.vbridge[index].vlan = kattach_vbridge_shm->vbridge[index].vlan;
		kattach_vbridge.vbridge[index].vmask = kattach_vbridge_shm->vbridge[index].vmask;
		kattach_vbridge.vbridge[index].vpfree = kattach_vbridge_shm->vbridge[index].vpfree;
		kattach_vbridge.vbridge[index].vbruse = kattach_vbridge_shm->vbridge[index].vbruse;
		kattach_vbridge.vbridge[index].vbrlocal = kattach_vbridge_shm->vbridge[index].vbrlocal;
		kattach_vbridge.vbridge[index].state = kattach_vbridge_shm->vbridge[index].state;
		sprintf(kattach_vbridge.vbridge[index].vlanext,"%s",kattach_vbridge_shm->vbridge[index].vlanext);
		for (yindex = 0; yindex <= 5; yindex++) {
			kattach_vbridge.vbridge[index].bmac[yindex] = kattach_vbridge_shm->vbridge[index].bmac[yindex];
		}
	}
	return;
}

void
kattach_sys_shm_sync_appmods(void)
{
	u32 index = 0;

	kattach_appmods_shm->index = kattach_appmods.index;
	for (index = 0; index < kattach_appmods.index; index++) {
		kattach_appmods_shm->appmodules[index].deployed = kattach_appmods.appmodules[index].deployed;
		kattach_appmods_shm->appmodules[index].config = kattach_appmods.appmodules[index].config;
		kattach_appmods_shm->appmodules[index].vendor_id = kattach_appmods.appmodules[index].vendor_id;
		kattach_appmods_shm->appmodules[index].revision = kattach_appmods.appmodules[index].revision;
		kattach_appmods_shm->appmodules[index].srctree = kattach_appmods.appmodules[index].srctree;
		kattach_appmods_shm->appmodules[index].license = kattach_appmods.appmodules[index].license;
		kattach_appmods_shm->appmodules[index].latest = kattach_appmods.appmodules[index].latest;
		kattach_appmods_shm->appmodules[index].state = kattach_appmods.appmodules[index].state;
		kattach_appmods_shm->appmodules[index].app_size = kattach_appmods.appmodules[index].app_size;
		kattach_appmods_shm->appmodules[index].mgrpid = kattach_appmods.appmodules[index].mgrpid;
		sprintf(kattach_appmods_shm->appmodules[index].filename,"%s",kattach_appmods.appmodules[index].filename);
		sprintf(kattach_appmods_shm->appmodules[index].buildinfo,"%s",kattach_appmods.appmodules[index].buildinfo);
		sprintf(kattach_appmods_shm->appmodules[index].url,"%s",kattach_appmods.appmodules[index].url);
		sprintf(kattach_appmods_shm->appmodules[index].name,"%s",kattach_appmods.appmodules[index].name);
		sprintf(kattach_appmods_shm->appmodules[index].version,"%s",kattach_appmods.appmodules[index].version);
		sprintf(kattach_appmods_shm->appmodules[index].release,"%s",kattach_appmods.appmodules[index].release);
		sprintf(kattach_appmods_shm->appmodules[index].chksum,"%s",kattach_appmods.appmodules[index].chksum);
	}
	return;

}

void
kattach_sys_shm_syncback_appmods(void)
{
	u32 index = 0;

	kattach_appmods.index = kattach_appmods_shm->index;
	for (index = 0; index < kattach_appmods_shm->index; index++) {
		kattach_appmods.appmodules[index].deployed = kattach_appmods_shm->appmodules[index].deployed;
		kattach_appmods.appmodules[index].config = kattach_appmods_shm->appmodules[index].config;
		kattach_appmods.appmodules[index].vendor_id = kattach_appmods_shm->appmodules[index].vendor_id;
		kattach_appmods.appmodules[index].revision = kattach_appmods_shm->appmodules[index].revision;
		kattach_appmods.appmodules[index].srctree = kattach_appmods_shm->appmodules[index].srctree;
		kattach_appmods.appmodules[index].license = kattach_appmods_shm->appmodules[index].license;
		kattach_appmods.appmodules[index].latest = kattach_appmods_shm->appmodules[index].latest;
		kattach_appmods.appmodules[index].state = kattach_appmods_shm->appmodules[index].state;
		kattach_appmods.appmodules[index].app_size = kattach_appmods_shm->appmodules[index].app_size;
		kattach_appmods.appmodules[index].mgrpid = kattach_appmods_shm->appmodules[index].mgrpid;
		sprintf(kattach_appmods.appmodules[index].filename,"%s",kattach_appmods_shm->appmodules[index].filename);
		sprintf(kattach_appmods.appmodules[index].buildinfo,"%s",kattach_appmods_shm->appmodules[index].buildinfo);
		sprintf(kattach_appmods.appmodules[index].url,"%s",kattach_appmods_shm->appmodules[index].url);
		sprintf(kattach_appmods.appmodules[index].name,"%s",kattach_appmods_shm->appmodules[index].name);
		sprintf(kattach_appmods.appmodules[index].version,"%s",kattach_appmods_shm->appmodules[index].version);
		sprintf(kattach_appmods.appmodules[index].release,"%s",kattach_appmods_shm->appmodules[index].release);
		sprintf(kattach_appmods.appmodules[index].chksum,"%s",kattach_appmods_shm->appmodules[index].chksum);
	}
	return;

}

void
kattach_sys_shm_sync_vmi(void)
{
	u32 index = 0;
	u8 aindex = 0;

	/* vmi_t */
	kattach_vmimages_shm->index = kattach_vmimages.index;
	for (index = 0; index < kattach_vmimages.index; index++) {
		kattach_vmimages_shm->vmimage[index].active = kattach_vmimages.vmimage[index].active;
		kattach_vmimages_shm->vmimage[index].appi = kattach_vmimages.vmimage[index].appi;
		kattach_vmimages_shm->vmimage[index].changed = kattach_vmimages.vmimage[index].changed;
		kattach_vmimages_shm->vmimage[index].import = kattach_vmimages.vmimage[index].import;
		sprintf(kattach_vmimages_shm->vmimage[index].vminame,"%s",kattach_vmimages.vmimage[index].vminame);
		for (aindex = 0; aindex < kattach_vmimages.vmimage[index].appi; aindex++) {
			kattach_vmimages_shm->vmimage[index].appindex[aindex] = kattach_vmimages.vmimage[index].appindex[aindex];
			kattach_vmimages_shm->vmimage[index].cfggrp[aindex] = kattach_vmimages.vmimage[index].cfggrp[aindex];
		}
	}
	return;
}

void
kattach_sys_shm_syncback_vmi(void)
{
	u32 index = 0;
	u8 aindex = 0;

	/* vmi_t */
	kattach_vmimages.index = kattach_vmimages_shm->index;
	for (index = 0; index < kattach_vmimages_shm->index; index++) {
		kattach_vmimages.vmimage[index].active = kattach_vmimages_shm->vmimage[index].active;
		kattach_vmimages.vmimage[index].appi = kattach_vmimages_shm->vmimage[index].appi;
		kattach_vmimages.vmimage[index].changed = kattach_vmimages_shm->vmimage[index].changed;
		kattach_vmimages.vmimage[index].import = kattach_vmimages_shm->vmimage[index].import;
		sprintf(kattach_vmimages.vmimage[index].vminame,"%s",kattach_vmimages_shm->vmimage[index].vminame);
		for (aindex = 0; aindex < kattach_vmimages_shm->vmimage[index].appi; aindex++) {
			kattach_vmimages.vmimage[index].appindex[aindex] = kattach_vmimages_shm->vmimage[index].appindex[aindex];
			kattach_vmimages.vmimage[index].cfggrp[aindex] = kattach_vmimages_shm->vmimage[index].cfggrp[aindex];
		}
	}
	return;
}


void
kattach_sys_shm_sync_netdev(void)
{
	u32 index = 0;
	u8 mmac = 0;

	/* netdev_t */
	kattach_netdev_shm->index = kattach_netdev.index;
	for (index = 0; index < kattach_netdev.index; index++) {
		kattach_netdev_shm->pif[index].status = kattach_netdev.pif[index].status;
		kattach_netdev_shm->pif[index].psuedo = kattach_netdev.pif[index].psuedo;
		kattach_netdev_shm->pif[index].type = kattach_netdev.pif[index].type;
		kattach_netdev_shm->pif[index].mtu = kattach_netdev.pif[index].mtu;
		kattach_netdev_shm->pif[index].lacpidx = kattach_netdev.pif[index].lacpidx;
		kattach_netdev_shm->pif[index].pvid = kattach_netdev.pif[index].pvid;
		kattach_netdev_shm->pif[index].mask = kattach_netdev.pif[index].mask;
		kattach_netdev_shm->pif[index].gw = kattach_netdev.pif[index].gw;
		kattach_netdev_shm->pif[index].ip = kattach_netdev.pif[index].ip;
		kattach_netdev_shm->pif[index].vns = kattach_netdev.pif[index].vns;
		kattach_netdev_shm->pif[index].spare = kattach_netdev.pif[index].spare;
		for (mmac = 0; mmac <= 5; mmac++) {
			kattach_netdev_shm->pif[index].mac[mmac] = kattach_netdev.pif[index].mac[mmac];
		}
		sprintf(kattach_netdev_shm->pif[index].devname,"%s",kattach_netdev.pif[index].devname);
	}
	return;
}

void
kattach_sys_shm_syncback_netdev(void)
{
	u32 index = 0;
	u8 mmac = 0;

	/* netdev_t */
	kattach_netdev.index = kattach_netdev_shm->index;
	for (index = 0; index < kattach_netdev_shm->index; index++) {

		kattach_netdev.pif[index].status = kattach_netdev_shm->pif[index].status;
		kattach_netdev.pif[index].psuedo = kattach_netdev_shm->pif[index].psuedo;
		kattach_netdev.pif[index].type = kattach_netdev_shm->pif[index].type;
		kattach_netdev.pif[index].mtu = kattach_netdev_shm->pif[index].mtu;
		kattach_netdev.pif[index].lacpidx = kattach_netdev_shm->pif[index].lacpidx;
		kattach_netdev.pif[index].pvid = kattach_netdev_shm->pif[index].pvid;
		kattach_netdev.pif[index].mask = kattach_netdev_shm->pif[index].mask;
		kattach_netdev.pif[index].gw = kattach_netdev_shm->pif[index].gw;
		kattach_netdev.pif[index].ip = kattach_netdev_shm->pif[index].ip;
		kattach_netdev.pif[index].vns = kattach_netdev_shm->pif[index].vns;
		kattach_netdev.pif[index].spare = kattach_netdev_shm->pif[index].spare;
		for (mmac = 0; mmac <= 5; mmac++) {
			kattach_netdev.pif[index].mac[mmac] = kattach_netdev_shm->pif[index].mac[mmac];
		}
		sprintf(kattach_netdev.pif[index].devname,"%s",kattach_netdev_shm->pif[index].devname);
	}
	return;
}

void
kattach_sys_shm_sync_vns(void)
{
	u32 index = 0, yindex = 0;

	/* vns_t */
	kattach_vns_shm->index = kattach_vns.index;
	for (index = 0; index < kattach_vns.index; index++) {
		kattach_vns_shm->vns[index].vsip = kattach_vns.vns[index].vsip;
		kattach_vns_shm->vns[index].vsmsk = kattach_vns.vns[index].vsmsk;
		kattach_vns_shm->vns[index].enabled = kattach_vns.vns[index].enabled;
		kattach_vns_shm->vns[index].mstate = kattach_vns.vns[index].mstate;
		kattach_vns_shm->vns[index].vspindex = kattach_vns.vns[index].vspindex;
		kattach_vns_shm->vns[index].netifidx = kattach_vns.vns[index].netifidx;
		for (yindex = 0; yindex <= KATTACH_MAX_VSP; yindex++) {
			kattach_vns_shm->vns[index].vsp[yindex].rate_in = kattach_vns.vns[index].vsp[yindex].rate_in;
			kattach_vns_shm->vns[index].vsp[yindex].rate_out = kattach_vns.vns[index].vsp[yindex].rate_out;
			kattach_vns_shm->vns[index].vsp[yindex].vsport = kattach_vns.vns[index].vsp[yindex].vsport;
			kattach_vns_shm->vns[index].vsp[yindex].vmsport = kattach_vns.vns[index].vsp[yindex].vmsport;
			kattach_vns_shm->vns[index].vsp[yindex].vmport = kattach_vns.vns[index].vsp[yindex].vmport;
			kattach_vns_shm->vns[index].vsp[yindex].time_in = kattach_vns.vns[index].vsp[yindex].time_in;
			kattach_vns_shm->vns[index].vsp[yindex].time_out = kattach_vns.vns[index].vsp[yindex].time_out;
			kattach_vns_shm->vns[index].vsp[yindex].sproto = kattach_vns.vns[index].vsp[yindex].sproto;
			kattach_vns_shm->vns[index].vsp[yindex].enabled = kattach_vns.vns[index].vsp[yindex].enabled;
			kattach_vns_shm->vns[index].vsp[yindex].spare = kattach_vns.vns[index].vsp[yindex].spare;
		}
	}
	return;

}

void
kattach_sys_shm_syncback_vns(void)
{
	u32 index = 0, yindex = 0;

	/* vns_t */
	kattach_vns.index = kattach_vns_shm->index;
	for (index = 0; index < kattach_vns.index; index++) {
		kattach_vns.vns[index].vsip = kattach_vns_shm->vns[index].vsip;
		kattach_vns.vns[index].vsmsk = kattach_vns_shm->vns[index].vsmsk;
		kattach_vns.vns[index].enabled = kattach_vns_shm->vns[index].enabled;
		kattach_vns.vns[index].mstate = kattach_vns_shm->vns[index].mstate;
		kattach_vns.vns[index].vspindex = kattach_vns_shm->vns[index].vspindex;
		kattach_vns.vns[index].netifidx = kattach_vns_shm->vns[index].netifidx;

		for (yindex = 0; yindex <= KATTACH_MAX_VSP; yindex++) {
			kattach_vns.vns[index].vsp[yindex].rate_in = kattach_vns_shm->vns[index].vsp[yindex].rate_in;
			kattach_vns.vns[index].vsp[yindex].rate_out = kattach_vns_shm->vns[index].vsp[yindex].rate_out;
			kattach_vns.vns[index].vsp[yindex].vsport = kattach_vns_shm->vns[index].vsp[yindex].vsport;
			kattach_vns.vns[index].vsp[yindex].vmsport = kattach_vns_shm->vns[index].vsp[yindex].vmsport;
			kattach_vns.vns[index].vsp[yindex].vmport = kattach_vns_shm->vns[index].vsp[yindex].vmport;
			kattach_vns.vns[index].vsp[yindex].time_in = kattach_vns_shm->vns[index].vsp[yindex].time_in;
			kattach_vns.vns[index].vsp[yindex].time_out = kattach_vns_shm->vns[index].vsp[yindex].time_out;
			kattach_vns.vns[index].vsp[yindex].sproto = kattach_vns_shm->vns[index].vsp[yindex].sproto;
			kattach_vns.vns[index].vsp[yindex].enabled = kattach_vns_shm->vns[index].vsp[yindex].enabled;
			kattach_vns.vns[index].vsp[yindex].spare = kattach_vns_shm->vns[index].vsp[yindex].spare;
		}
	}
	return;

}

void
kattach_sys_shm_sync_cfggrp(void)
{
	u32 index = 0;

	kattach_cfggrp_shm->index = kattach_cfggrp.index;
	for (index = 0; index < KATTACH_MAX_CFGGRP; index++) {
		kattach_cfggrp_shm->cfggrp[index].appmidx = kattach_cfggrp.cfggrp[index].appmidx;
		sprintf(kattach_cfggrp_shm->cfggrp[index].name,"%s",kattach_cfggrp.cfggrp[index].name);
	}

	return;
}


void
kattach_sys_shm_syncback_cfggrp(void)
{
	u32 index = 0;

	kattach_cfggrp.index = kattach_cfggrp_shm->index;
	for (index = 0; index < KATTACH_MAX_CFGGRP; index++) {
		kattach_cfggrp.cfggrp[index].appmidx = kattach_cfggrp_shm->cfggrp[index].appmidx;
		sprintf(kattach_cfggrp.cfggrp[index].name,"%s",kattach_cfggrp_shm->cfggrp[index].name);
	}

	return;
}

void
kattach_sys_shm_sync_fw(void)
{
	u32 index = 0;
	u8 pindex = 0, y = 0, l = 0;
	kattach_fw_chain_t *chain, *t_chain;
	kattach_fw_n_chain_t *nchain, *t_nchain;
	kattach_fw_m_chain_t *mchain, *t_mchain;

	/* app_t */
	kattach_fw_shm->apps.index = kattach_fw.apps.index;
	for (index = 0; index < KATTACH_MAX_FW_APPS; index++) {
		kattach_fw_shm->apps.app[index].pindex = kattach_fw.apps.app[index].pindex;
		kattach_fw_shm->apps.app[index].statemask = kattach_fw.apps.app[index].statemask;
		if (kattach_fw.apps.app[index].name[0] != '\0') {
			sprintf(kattach_fw_shm->apps.app[index].name,"%s",kattach_fw.apps.app[index].name);
		} else {
			kattach_fw_shm->apps.app[index].name[0] = '\0';
		}
		for (pindex = 0; pindex < KATTACH_MAX_FW_APP_PORTS; pindex++) {
			kattach_fw_shm->apps.app[index].port[pindex].direction = kattach_fw.apps.app[index].port[pindex].direction;
			kattach_fw_shm->apps.app[index].port[pindex].protmask = kattach_fw.apps.app[index].port[pindex].protmask;
			kattach_fw_shm->apps.app[index].port[pindex].port[0] = kattach_fw.apps.app[index].port[pindex].port[0];
			kattach_fw_shm->apps.app[index].port[pindex].port[1] = kattach_fw.apps.app[index].port[pindex].port[1];
			kattach_fw_shm->apps.app[index].port[pindex].tcp_flags.all = kattach_fw.apps.app[index].port[pindex].tcp_flags.all;
			kattach_fw_shm->apps.app[index].port[pindex].tcp_flags.none = kattach_fw.apps.app[index].port[pindex].tcp_flags.none;
			kattach_fw_shm->apps.app[index].port[pindex].tcp_flags.syn = kattach_fw.apps.app[index].port[pindex].tcp_flags.syn;
			kattach_fw_shm->apps.app[index].port[pindex].tcp_flags.ack = kattach_fw.apps.app[index].port[pindex].tcp_flags.ack;
			kattach_fw_shm->apps.app[index].port[pindex].tcp_flags.fin = kattach_fw.apps.app[index].port[pindex].tcp_flags.fin;
			kattach_fw_shm->apps.app[index].port[pindex].tcp_flags.reset = kattach_fw.apps.app[index].port[pindex].tcp_flags.reset;
			kattach_fw_shm->apps.app[index].port[pindex].tcp_flags.push = kattach_fw.apps.app[index].port[pindex].tcp_flags.push;
			kattach_fw_shm->apps.app[index].port[pindex].tcp_flags.urgent = kattach_fw.apps.app[index].port[pindex].tcp_flags.urgent;
		}
	}
	
	/* zone_t */
	kattach_fw_shm->zones.index = kattach_fw.zones.index;
	for (index = 0; index < KATTACH_MAX_FW_ZONES; index++) {
		kattach_fw_shm->zones.zone[index].vlan = kattach_fw.zones.zone[index].vlan;
		kattach_fw_shm->zones.zone[index].nindex = kattach_fw.zones.zone[index].nindex;
		sprintf(kattach_fw_shm->zones.zone[index].name,"%s",kattach_fw.zones.zone[index].name);
		for (pindex = 0; pindex < KATTACH_MAX_FW_ZNODES; pindex++) {
			kattach_fw_shm->zones.zone[index].node[pindex].ip = kattach_fw.zones.zone[index].node[pindex].ip;
			kattach_fw_shm->zones.zone[index].node[pindex].mask = kattach_fw.zones.zone[index].node[pindex].mask;
		}
	}

	/* filter_t */
	while (!y) {
		if (l == 0) {
			chain = &kattach_fw_shm->filter.input;
			t_chain = &kattach_fw.filter.input;
		} else if (l == 1) {
			chain = &kattach_fw_shm->filter.output;
			t_chain = &kattach_fw.filter.output;
		} else if (l == 2) {
			chain = &kattach_fw_shm->filter.forward;
			t_chain = &kattach_fw.filter.forward;
		} else {
			y++;
			break;
		}
		chain->index = t_chain->index;
		chain->hindex = t_chain->hindex;
		chain->eindex = t_chain->eindex;
		for (index = 0; index < KATTACH_MAX_FW; index++) {
			chain->filter[index].pindex = t_chain->filter[index].pindex;
			chain->filter[index].nindex = t_chain->filter[index].nindex;
			chain->filter[index].szindex = t_chain->filter[index].szindex;
			chain->filter[index].dzindex = t_chain->filter[index].dzindex;
			chain->filter[index].appindex = t_chain->filter[index].appindex;
			chain->filter[index].rlimitpkt = t_chain->filter[index].rlimitpkt;
			chain->filter[index].rlimitint = t_chain->filter[index].rlimitint;
			chain->filter[index].action = t_chain->filter[index].action;
			chain->filter[index].ttl[0] = t_chain->filter[index].ttl[0];
			chain->filter[index].ttl[1] = t_chain->filter[index].ttl[1];
			chain->filter[index].tos[0] = t_chain->filter[index].tos[0];
			chain->filter[index].tos[1] = t_chain->filter[index].tos[1];
			chain->filter[index].enabled = t_chain->filter[index].enabled;
			chain->filter[index].type = t_chain->filter[index].type;
			chain->filter[index].rejectwith = t_chain->filter[index].rejectwith;
			chain->filter[index].reverse = t_chain->filter[index].reverse;
			chain->filter[index].logging = t_chain->filter[index].logging;
			sprintf(chain->filter[index].logprefix,"%s",t_chain->filter[index].logprefix);
		}
		l++;
	}
	y = 0;
	l = 0;

	/* nat_t */
	while (!y) {
		if (l == 0) {
			nchain = &kattach_fw_shm->nat.prerouting;
			t_nchain = &kattach_fw.nat.prerouting;
		} else if (l == 1) {
			nchain = &kattach_fw_shm->nat.postrouting;
			t_nchain = &kattach_fw.nat.postrouting;
		} else if (l == 2) {
			nchain = &kattach_fw_shm->nat.output;
			t_nchain = &kattach_fw.nat.output;
		} else {
			y++;
			break;
		}
		nchain->index = t_nchain->index;
		nchain->hindex = t_nchain->hindex;
		nchain->eindex = t_nchain->eindex;
		for (index = 0; index < KATTACH_MAX_FW; index++) {
			nchain->filter[index].pindex = t_nchain->filter[index].pindex;
			nchain->filter[index].nindex = t_nchain->filter[index].nindex;
			nchain->filter[index].szindex = t_nchain->filter[index].szindex;
			nchain->filter[index].dzindex = t_nchain->filter[index].dzindex;
			nchain->filter[index].nzindex = t_nchain->filter[index].nzindex;
			nchain->filter[index].appindex = t_nchain->filter[index].appindex;
			nchain->filter[index].nappindex = t_nchain->filter[index].nappindex;
			nchain->filter[index].rlimitpkt = t_nchain->filter[index].rlimitpkt;
			nchain->filter[index].rlimitint = t_nchain->filter[index].rlimitint;
			nchain->filter[index].action = t_nchain->filter[index].action;
			nchain->filter[index].ttl[0] = t_nchain->filter[index].ttl[0];
			nchain->filter[index].ttl[1] = t_nchain->filter[index].ttl[1];
			nchain->filter[index].tos[0] = t_nchain->filter[index].tos[0];
			nchain->filter[index].tos[1] = t_nchain->filter[index].tos[1];
			nchain->filter[index].enabled = t_nchain->filter[index].enabled;
			nchain->filter[index].type = t_nchain->filter[index].type;
			nchain->filter[index].rejectwith = t_nchain->filter[index].rejectwith;
			nchain->filter[index].reverse = t_nchain->filter[index].reverse;
			nchain->filter[index].logging = t_nchain->filter[index].logging;
			sprintf(nchain->filter[index].logprefix,"%s",t_nchain->filter[index].logprefix);
		}
		l++;
	}

	/* mangle_t */
	while (!y) {
		if (l == 0) {
			mchain = &kattach_fw_shm->mangle.input;
			t_mchain = &kattach_fw.mangle.input;
		} else if (l == 1) {
			mchain = &kattach_fw_shm->mangle.output;
			t_mchain = &kattach_fw.mangle.output;
		} else if (l == 2) {
			mchain = &kattach_fw_shm->mangle.forward;
			t_mchain = &kattach_fw.mangle.forward;
		} else if (l == 3) {
			mchain = &kattach_fw_shm->mangle.prerouting;
			t_mchain = &kattach_fw.mangle.prerouting;
		} else if (l == 4) {
			mchain = &kattach_fw_shm->mangle.postrouting;
			t_mchain = &kattach_fw.mangle.postrouting;
		} else {
			y++;
			break;
		}
		mchain->index = t_mchain->index;
		mchain->hindex = t_mchain->hindex;
		mchain->eindex = t_mchain->eindex;
		for (index = 0; index < KATTACH_MAX_FW; index++) {
			mchain->filter[index].pindex = t_mchain->filter[index].pindex;
			mchain->filter[index].nindex = t_mchain->filter[index].nindex;
			mchain->filter[index].szindex = t_mchain->filter[index].szindex;
			mchain->filter[index].dzindex = t_mchain->filter[index].dzindex;
			mchain->filter[index].appindex = t_mchain->filter[index].appindex;
			mchain->filter[index].rlimitpkt = t_mchain->filter[index].rlimitpkt;
			mchain->filter[index].rlimitint = t_mchain->filter[index].rlimitint;
			mchain->filter[index].mark = t_mchain->filter[index].mark;
			mchain->filter[index].action = t_mchain->filter[index].action;
			mchain->filter[index].ttl[0] = t_mchain->filter[index].ttl[0];
			mchain->filter[index].ttl[1] = t_mchain->filter[index].ttl[1];
			mchain->filter[index].tos[0] = t_mchain->filter[index].tos[0];
			mchain->filter[index].tos[1] = t_mchain->filter[index].tos[1];
			mchain->filter[index].enabled = t_mchain->filter[index].enabled;
			mchain->filter[index].type = t_mchain->filter[index].type;
			mchain->filter[index].rejectwith = t_mchain->filter[index].rejectwith;
			mchain->filter[index].reverse = t_mchain->filter[index].reverse;
			mchain->filter[index].logging = t_mchain->filter[index].logging;
			sprintf(mchain->filter[index].logprefix,"%s",t_mchain->filter[index].logprefix);
		}
		l++;
	}
	kattach_fw_shm->fw_update = kattach_fw.fw_update;
	return;
}

void
kattach_sys_shm_syncback_fw(void)
{
	u32 index = 0;
	u8 pindex = 0, y = 0, l = 0;
	kattach_fw_chain_t *chain, *t_chain;
	kattach_fw_n_chain_t *nchain, *t_nchain;
	kattach_fw_m_chain_t *mchain, *t_mchain;

	/* app_t */
	kattach_fw.apps.index = kattach_fw_shm->apps.index;
	for (index = 0; index < KATTACH_MAX_FW_APPS; index++) {
		kattach_fw.apps.app[index].pindex = kattach_fw_shm->apps.app[index].pindex;
		kattach_fw.apps.app[index].statemask = kattach_fw_shm->apps.app[index].statemask;
		if (kattach_fw_shm->apps.app[index].name[0] != '\0') {
			sprintf(kattach_fw.apps.app[index].name,"%s",kattach_fw_shm->apps.app[index].name);
		} else {
			kattach_fw.apps.app[index].name[0] = '\0';
		}
		for (pindex = 0; pindex < KATTACH_MAX_FW_APP_PORTS; pindex++) {
			kattach_fw.apps.app[index].port[pindex].direction = kattach_fw_shm->apps.app[index].port[pindex].direction;
			kattach_fw.apps.app[index].port[pindex].protmask = kattach_fw_shm->apps.app[index].port[pindex].protmask;
			kattach_fw.apps.app[index].port[pindex].port[0] = kattach_fw_shm->apps.app[index].port[pindex].port[0];
			kattach_fw.apps.app[index].port[pindex].port[1] = kattach_fw_shm->apps.app[index].port[pindex].port[1];
			kattach_fw.apps.app[index].port[pindex].tcp_flags.all = kattach_fw_shm->apps.app[index].port[pindex].tcp_flags.all;
			kattach_fw.apps.app[index].port[pindex].tcp_flags.none = kattach_fw_shm->apps.app[index].port[pindex].tcp_flags.none;
			kattach_fw.apps.app[index].port[pindex].tcp_flags.syn = kattach_fw_shm->apps.app[index].port[pindex].tcp_flags.syn;
			kattach_fw.apps.app[index].port[pindex].tcp_flags.ack = kattach_fw_shm->apps.app[index].port[pindex].tcp_flags.ack;
			kattach_fw.apps.app[index].port[pindex].tcp_flags.fin = kattach_fw_shm->apps.app[index].port[pindex].tcp_flags.fin;
			kattach_fw.apps.app[index].port[pindex].tcp_flags.reset = kattach_fw_shm->apps.app[index].port[pindex].tcp_flags.reset;
			kattach_fw.apps.app[index].port[pindex].tcp_flags.push = kattach_fw_shm->apps.app[index].port[pindex].tcp_flags.push;
			kattach_fw.apps.app[index].port[pindex].tcp_flags.urgent = kattach_fw_shm->apps.app[index].port[pindex].tcp_flags.urgent;
		}
	}

	/* zone_t */
	kattach_fw.zones.index = kattach_fw_shm->zones.index;
	for (index = 0; index < KATTACH_MAX_FW_ZONES; index++) {
		kattach_fw.zones.zone[index].vlan = kattach_fw_shm->zones.zone[index].vlan;
		kattach_fw.zones.zone[index].nindex = kattach_fw_shm->zones.zone[index].nindex;
		sprintf(kattach_fw.zones.zone[index].name,"%s",kattach_fw_shm->zones.zone[index].name);
		for (pindex = 0; pindex < KATTACH_MAX_FW_ZNODES; pindex++) {
			kattach_fw.zones.zone[index].node[pindex].ip = kattach_fw_shm->zones.zone[index].node[pindex].ip;
			kattach_fw.zones.zone[index].node[pindex].mask = kattach_fw_shm->zones.zone[index].node[pindex].mask;
		}
	}

	/* filter_t */
	while (!y) {
		if (l == 0) {
			t_chain = &kattach_fw_shm->filter.input;
			chain = &kattach_fw.filter.input;
		} else if (l == 1) {
			t_chain = &kattach_fw_shm->filter.output;
			chain = &kattach_fw.filter.output;
		} else if (l == 2) {
			t_chain = &kattach_fw_shm->filter.forward;
			chain = &kattach_fw.filter.forward;
		} else {
			y++;
			break;
		}
		chain->index = t_chain->index;
		chain->hindex = t_chain->hindex;
		chain->eindex = t_chain->eindex;
		for (index = 0; index < KATTACH_MAX_FW; index++) {
			chain->filter[index].pindex = t_chain->filter[index].pindex;
			chain->filter[index].nindex = t_chain->filter[index].nindex;
			chain->filter[index].szindex = t_chain->filter[index].szindex;
			chain->filter[index].dzindex = t_chain->filter[index].dzindex;
			chain->filter[index].appindex = t_chain->filter[index].appindex;
			chain->filter[index].rlimitpkt = t_chain->filter[index].rlimitpkt;
			chain->filter[index].rlimitint = t_chain->filter[index].rlimitint;
			chain->filter[index].action = t_chain->filter[index].action;
			chain->filter[index].ttl[0] = t_chain->filter[index].ttl[0];
			chain->filter[index].ttl[1] = t_chain->filter[index].ttl[1];
			chain->filter[index].tos[0] = t_chain->filter[index].tos[0];
			chain->filter[index].tos[1] = t_chain->filter[index].tos[1];
			chain->filter[index].enabled = t_chain->filter[index].enabled;
			chain->filter[index].type = t_chain->filter[index].type;
			chain->filter[index].rejectwith = t_chain->filter[index].rejectwith;
			chain->filter[index].reverse = t_chain->filter[index].reverse;
			chain->filter[index].logging = t_chain->filter[index].logging;
			sprintf(chain->filter[index].logprefix,"%s",t_chain->filter[index].logprefix);
		}
		l++;
	}
	y = 0;
	l = 0;

	/* nat_t */
	while (!y) {
		if (l == 0) {
			t_nchain = &kattach_fw_shm->nat.prerouting;
			nchain = &kattach_fw.nat.prerouting;
		} else if (l == 1) {
			t_nchain = &kattach_fw_shm->nat.postrouting;
			nchain = &kattach_fw.nat.postrouting;
		} else if (l == 2) {
			t_nchain = &kattach_fw_shm->nat.output;
			nchain = &kattach_fw.nat.output;
		} else {
			y++;
			break;
		}
		nchain->index = t_nchain->index;
		nchain->hindex = t_nchain->hindex;
		nchain->eindex = t_nchain->eindex;
		for (index = 0; index < KATTACH_MAX_FW; index++) {
			nchain->filter[index].pindex = t_nchain->filter[index].pindex;
			nchain->filter[index].nindex = t_nchain->filter[index].nindex;
			nchain->filter[index].szindex = t_nchain->filter[index].szindex;
			nchain->filter[index].dzindex = t_nchain->filter[index].dzindex;
			nchain->filter[index].nzindex = t_nchain->filter[index].nzindex;
			nchain->filter[index].appindex = t_nchain->filter[index].appindex;
			nchain->filter[index].nappindex = t_nchain->filter[index].nappindex;
			nchain->filter[index].rlimitpkt = t_nchain->filter[index].rlimitpkt;
			nchain->filter[index].rlimitint = t_nchain->filter[index].rlimitint;
			nchain->filter[index].action = t_nchain->filter[index].action;
			nchain->filter[index].ttl[0] = t_nchain->filter[index].ttl[0];
			nchain->filter[index].ttl[1] = t_nchain->filter[index].ttl[1];
			nchain->filter[index].tos[0] = t_nchain->filter[index].tos[0];
			nchain->filter[index].tos[1] = t_nchain->filter[index].tos[1];
			nchain->filter[index].enabled = t_nchain->filter[index].enabled;
			nchain->filter[index].type = t_nchain->filter[index].type;
			nchain->filter[index].rejectwith = t_nchain->filter[index].rejectwith;
			nchain->filter[index].reverse = t_nchain->filter[index].reverse;
			nchain->filter[index].logging = t_nchain->filter[index].logging;
			sprintf(nchain->filter[index].logprefix,"%s",t_nchain->filter[index].logprefix);
		}
		l++;
	}

	/* mangle_t */
	while (!y) {
		if (l == 0) {
			t_mchain = &kattach_fw_shm->mangle.input;
			mchain = &kattach_fw.mangle.input;
		} else if (l == 1) {
			t_mchain = &kattach_fw_shm->mangle.output;
			mchain = &kattach_fw.mangle.output;
		} else if (l == 2) {
			t_mchain = &kattach_fw_shm->mangle.forward;
			mchain = &kattach_fw.mangle.forward;
		} else if (l == 3) {
			t_mchain = &kattach_fw_shm->mangle.prerouting;
			mchain = &kattach_fw.mangle.prerouting;
		} else if (l == 4) {
			t_mchain = &kattach_fw_shm->mangle.postrouting;
			mchain = &kattach_fw.mangle.postrouting;
		} else {
			y++;
			break;
		}
		mchain->index = t_mchain->index;
		mchain->hindex = t_mchain->hindex;
		mchain->eindex = t_mchain->eindex;
		for (index = 0; index < KATTACH_MAX_FW; index++) {
			mchain->filter[index].pindex = t_mchain->filter[index].pindex;
			mchain->filter[index].nindex = t_mchain->filter[index].nindex;
			mchain->filter[index].szindex = t_mchain->filter[index].szindex;
			mchain->filter[index].dzindex = t_mchain->filter[index].dzindex;
			mchain->filter[index].appindex = t_mchain->filter[index].appindex;
			mchain->filter[index].rlimitpkt = t_mchain->filter[index].rlimitpkt;
			mchain->filter[index].rlimitint = t_mchain->filter[index].rlimitint;
			mchain->filter[index].mark = t_mchain->filter[index].mark;
			mchain->filter[index].action = t_mchain->filter[index].action;
			mchain->filter[index].ttl[0] = t_mchain->filter[index].ttl[0];
			mchain->filter[index].ttl[1] = t_mchain->filter[index].ttl[1];
			mchain->filter[index].tos[0] = t_mchain->filter[index].tos[0];
			mchain->filter[index].tos[1] = t_mchain->filter[index].tos[1];
			mchain->filter[index].enabled = t_mchain->filter[index].enabled;
			mchain->filter[index].type = t_mchain->filter[index].type;
			mchain->filter[index].rejectwith = t_mchain->filter[index].rejectwith;
			mchain->filter[index].reverse = t_mchain->filter[index].reverse;
			mchain->filter[index].logging = t_mchain->filter[index].logging;
			sprintf(mchain->filter[index].logprefix,"%s",t_mchain->filter[index].logprefix);
		}
		l++;
	}
	kattach_fw.fw_update = kattach_fw_shm->fw_update;
}


void
kattach_sys_shm_sync(void)
{
	kattach_sys_shm_sync_devices();
	kattach_sys_shm_sync_cfg();
	kattach_sys_shm_sync_install();
	kattach_sys_shm_sync_vmst();
	kattach_sys_shm_sync_vmp();
	kattach_sys_shm_sync_vbr();
	kattach_sys_shm_sync_vmi();
	kattach_sys_shm_sync_appmods();
	kattach_sys_shm_sync_netdev();
	kattach_sys_shm_sync_vns();
	kattach_sys_shm_sync_cfggrp();
	kattach_sys_shm_sync_fw();

	/* cm queue */
	kattach_appqueue->ka_dev = CM_MSG_Q_FREE;
	kattach_appqueue->ka_cfg = CM_MSG_Q_FREE;
	kattach_appqueue->ka_inst = CM_MSG_Q_FREE;
	kattach_appqueue->ka_vmst = CM_MSG_Q_FREE;
	kattach_appqueue->ka_vmports = CM_MSG_Q_FREE;
	kattach_appqueue->ka_vbridge = CM_MSG_Q_FREE;
	kattach_appqueue->ka_vmimages = CM_MSG_Q_FREE;
	kattach_appqueue->ka_netdev = CM_MSG_Q_FREE;
	kattach_appqueue->ka_vns = CM_MSG_Q_FREE;
	kattach_appqueue->ka_cfggrp = CM_MSG_Q_FREE;
	kattach_appqueue->ka_fw = CM_MSG_Q_FREE;
	kattach_appqueue->ak_update = CM_MSG_Q_FREE;

	return;
}

void
kattach_sys_shm_syncback(void)
{
	kattach_sys_shm_syncback_devices();
	kattach_sys_shm_syncback_cfg();
	kattach_sys_shm_syncback_install();
	kattach_sys_shm_syncback_vmst();
	kattach_sys_shm_syncback_vmp();
	kattach_sys_shm_syncback_vbr();
	kattach_sys_shm_syncback_vmi();
	kattach_sys_shm_syncback_appmods();
	kattach_sys_shm_syncback_netdev();
	kattach_sys_shm_syncback_vns();
	kattach_sys_shm_syncback_cfggrp();
	kattach_sys_shm_syncback_fw();

	/* cm queue */
	kattach_appqueue->ka_dev = CM_MSG_Q_FREE;
	kattach_appqueue->ka_cfg = CM_MSG_Q_FREE;
	kattach_appqueue->ka_inst = CM_MSG_Q_FREE;
	kattach_appqueue->ka_vmst = CM_MSG_Q_FREE;
	kattach_appqueue->ka_vmports = CM_MSG_Q_FREE;
	kattach_appqueue->ka_vbridge = CM_MSG_Q_FREE;
	kattach_appqueue->ka_vmimages = CM_MSG_Q_FREE;
	kattach_appqueue->ka_netdev = CM_MSG_Q_FREE;
	kattach_appqueue->ka_vns = CM_MSG_Q_FREE;
	kattach_appqueue->ka_cfggrp = CM_MSG_Q_FREE;
	kattach_appqueue->ka_fw = CM_MSG_Q_FREE;
	kattach_appqueue->ak_update = CM_MSG_Q_FREE;

	return;
}

u8
kattach_sys_shm_setsync_vmst(void)
{
	u32 kattach_syncwaiting = 0;

	while (kattach_appqueue->ka_vmst == (kattach_appqueue->ka_vmst | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_syncwaiting++;
		printf("\n\n [!] FATAL - Waited too long for VM Session Table Lock (%lu)\n", kattach_syncwaiting);
		if (kattach_syncwaiting >= 65535) return (RC_FAIL);
	}

	if (kattach_appqueue->ka_vmst == (kattach_appqueue->ka_vmst | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_sys_shm_syncback_vmst();
		kattach_appqueue->ka_vmst ^= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	kattach_appqueue->ka_vmst |= CM_MSG_Q_KATTACH_LOCK;
	kattach_sys_shm_sync_vmst();
	kattach_appqueue->ka_vmst |= CM_MSG_Q_KATTACH_UPDATED;
	kattach_appqueue->ak_update |= CM_MSG_Q_KATTACH_UPDATED;
	kattach_appqueue->ka_vmst ^= CM_MSG_Q_KATTACH_LOCK;

	return (RC_OK);
}


u8
kattach_sys_shm_setsync_vmp(void)
{
	u32 kattach_syncwaiting = 0;

	while (kattach_appqueue->ka_vmports == (kattach_appqueue->ka_vmports | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_syncwaiting++;
		printf("\n\n [!] FATAL - Waited too long for Virtual Ports Table Lock (%lu)\n", kattach_syncwaiting);
		if (kattach_syncwaiting >= 65535) return (RC_FAIL);
	}

	if (kattach_appqueue->ka_vmports == (kattach_appqueue->ka_vmports | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_sys_shm_syncback_vmp();
		kattach_appqueue->ka_vmports ^= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	kattach_appqueue->ka_vmports |= CM_MSG_Q_KATTACH_LOCK;
	kattach_sys_shm_sync_vmp();
	kattach_appqueue->ka_vmports |= CM_MSG_Q_KATTACH_UPDATED;
	kattach_appqueue->ak_update |= CM_MSG_Q_KATTACH_UPDATED;
	kattach_appqueue->ka_vmports ^= CM_MSG_Q_KATTACH_LOCK;
	return (RC_OK);
}

u8
kattach_sys_shm_setsync_vbr(void)
{
	u32 kattach_syncwaiting = 0;

	while (kattach_appqueue->ka_vbridge == (kattach_appqueue->ka_vbridge | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_syncwaiting++;
		printf("\n\n [!] FATAL - Waited too long for Virtual Bridge Table Lock (%lu)\n", kattach_syncwaiting);
		if (kattach_syncwaiting >= 65535) return (RC_FAIL);
	}

	if (kattach_appqueue->ka_vbridge == (kattach_appqueue->ka_vbridge | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_sys_shm_syncback_vbr();
		kattach_appqueue->ka_vbridge ^= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	kattach_appqueue->ka_vbridge |= CM_MSG_Q_KATTACH_LOCK;
	kattach_sys_shm_sync_vbr();
	kattach_appqueue->ka_vbridge |= CM_MSG_Q_KATTACH_UPDATED;
	kattach_appqueue->ak_update |= CM_MSG_Q_KATTACH_UPDATED;
	kattach_appqueue->ka_vbridge ^= CM_MSG_Q_KATTACH_LOCK;
	return (RC_OK);
}

u8
kattach_sys_shm_setsync_vmi(void)
{
	u32 kattach_syncwaiting = 0;

	while (kattach_appqueue->ka_vmimages == (kattach_appqueue->ka_vmimages | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_syncwaiting++;
		printf("\n\n [!] FATAL - Waited too long for VM Image Table Lock (%lu)\n", kattach_syncwaiting);
		if (kattach_syncwaiting >= 65535) return (RC_FAIL);
	}

	if (kattach_appqueue->ka_vmimages == (kattach_appqueue->ka_vmimages | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_sys_shm_syncback_vmi();
		kattach_appqueue->ka_vmimages ^= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	kattach_appqueue->ka_vmimages |= CM_MSG_Q_KATTACH_LOCK;
	kattach_sys_shm_sync_vmi();
	kattach_appqueue->ka_vmimages |= CM_MSG_Q_KATTACH_UPDATED;
	kattach_appqueue->ak_update |= CM_MSG_Q_KATTACH_UPDATED;
	kattach_appqueue->ka_vmimages ^= CM_MSG_Q_KATTACH_LOCK;
	return (RC_OK);
}

u8
kattach_sys_shm_setsync_netdev(void)
{
	u32 kattach_syncwaiting = 0;

	while (kattach_appqueue->ka_netdev == (kattach_appqueue->ka_netdev | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_syncwaiting++;
		printf("\n\n [!] FATAL - Waited too long for Net Devices Table Lock (%lu)\n", kattach_syncwaiting);
		if (kattach_syncwaiting >= 65535) return (RC_FAIL);
	}

	if (kattach_appqueue->ka_netdev == (kattach_appqueue->ka_netdev | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_sys_shm_syncback_netdev();
		kattach_appqueue->ka_netdev ^= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	kattach_appqueue->ka_netdev |= CM_MSG_Q_KATTACH_LOCK;
	kattach_sys_shm_sync_netdev();
	kattach_appqueue->ka_netdev |= CM_MSG_Q_KATTACH_UPDATED;
	kattach_appqueue->ak_update |= CM_MSG_Q_KATTACH_UPDATED;
	kattach_appqueue->ka_netdev ^= CM_MSG_Q_KATTACH_LOCK;
	return (RC_OK);
}

u8
kattach_sys_shm_setsync_appmods(void)
{
	if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_MODULE)) {
		kattach_sys_shm_syncback_appmods();
		kattach_appqueue->ak_update ^= CM_MSG_Q_APPQUEUE_MODULE;
	}

	kattach_appqueue->ak_update |= CM_MSG_Q_KATTACH_MODULE;
	kattach_sys_shm_sync_appmods();
	return (RC_OK);
}

u8
kattach_sys_shm_setsync_vns(void)
{
	u32 kattach_syncwaiting = 0;

	while (kattach_appqueue->ka_vns == (kattach_appqueue->ka_vns | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_syncwaiting++;
		printf("\n\n [!] FATAL - Waited too long for VNS Table Lock (%lu)\n", kattach_syncwaiting);
		if (kattach_syncwaiting >= 65535) return (RC_FAIL);
	}

	if (kattach_appqueue->ka_vns == (kattach_appqueue->ka_vns | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_sys_shm_syncback_vns();
		kattach_appqueue->ka_vns ^= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	kattach_appqueue->ka_vns |= CM_MSG_Q_KATTACH_LOCK;
	kattach_sys_shm_sync_vns();
	kattach_appqueue->ka_vns |= CM_MSG_Q_KATTACH_UPDATED;
	kattach_appqueue->ak_update |= CM_MSG_Q_KATTACH_UPDATED;
	kattach_appqueue->ka_vns ^= CM_MSG_Q_KATTACH_LOCK;
	return (RC_OK);
}

u8
kattach_sys_shm_setsync_cfggrp(void)
{
	u32 kattach_syncwaiting = 0;

	while (kattach_appqueue->ka_cfggrp == (kattach_appqueue->ka_cfggrp | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_syncwaiting++;
		printf("\n\n [!] FATAL - Waited too long for App Config Table Lock (%lu)\n", kattach_syncwaiting);
		if (kattach_syncwaiting >= 65535) return (RC_FAIL);
	}

	if (kattach_appqueue->ka_cfggrp == (kattach_appqueue->ka_cfggrp | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_sys_shm_syncback_cfggrp();
		kattach_appqueue->ka_cfggrp ^= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	kattach_appqueue->ka_cfggrp |= CM_MSG_Q_KATTACH_LOCK;
	kattach_sys_shm_sync_cfggrp();
	kattach_appqueue->ka_cfggrp |= CM_MSG_Q_KATTACH_UPDATED;
	kattach_appqueue->ak_update |= CM_MSG_Q_KATTACH_UPDATED;
	kattach_appqueue->ka_cfggrp ^= CM_MSG_Q_KATTACH_LOCK;
	return (RC_OK);
}

u8
kattach_sys_shm_setsync_fw(void)
{
	u32 kattach_syncwaiting = 0;

	while (kattach_appqueue->ka_fw == (kattach_appqueue->ka_fw | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_syncwaiting++;
		printf("\n\n [!] FATAL - Waited too long for FW Table Lock (%lu)\n", kattach_syncwaiting);
		if (kattach_syncwaiting >= 65535) return (RC_FAIL);
	}

	if (kattach_appqueue->ka_fw == (kattach_appqueue->ka_fw | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_sys_shm_syncback_fw();
		kattach_appqueue->ka_fw ^= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	kattach_appqueue->ka_fw |= CM_MSG_Q_KATTACH_LOCK;
	kattach_sys_shm_sync_fw();
	kattach_appqueue->ka_fw |= CM_MSG_Q_KATTACH_UPDATED;
	kattach_appqueue->ak_update |= CM_MSG_Q_KATTACH_UPDATED;
	kattach_appqueue->ka_fw ^= CM_MSG_Q_KATTACH_LOCK;
	return (RC_OK);
}


void
kattach_sys_shm_checksync(void)
{
	if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_UPDATED))  {
		if (kattach_appqueue->ka_dev == (kattach_appqueue->ka_dev | CM_MSG_Q_APPQUEUE_UPDATED)) {
			/* note: we do not need to check for kattach updates, already pushed to shm */
			kattach_sys_shm_syncback_devices();
			kattach_appqueue->ka_dev ^= CM_MSG_Q_APPQUEUE_UPDATED;
			if (kattach_change_detected != (kattach_change_detected | KATTACH_UPD_DEVICES)) {
				kattach_change_detected |= KATTACH_UPD_DEVICES;
			}
		}
		if (kattach_appqueue->ka_netdev == (kattach_appqueue->ka_netdev | CM_MSG_Q_APPQUEUE_UPDATED)) {
			kattach_sys_shm_syncback_netdev();
			kattach_appqueue->ka_netdev ^= CM_MSG_Q_APPQUEUE_UPDATED;
			if (kattach_change_detected != (kattach_change_detected | KATTACH_UPD_NETDEV)) {
				kattach_change_detected |= KATTACH_UPD_NETDEV;
			}
		}
		if (kattach_appqueue->ka_cfg == (kattach_appqueue->ka_cfg | CM_MSG_Q_APPQUEUE_UPDATED)) {
			kattach_sys_shm_syncback_cfg();
			kattach_appqueue->ka_cfg ^= CM_MSG_Q_APPQUEUE_UPDATED;
			if (kattach_change_detected != (kattach_change_detected | KATTACH_UPD_CONFIG)) {
				kattach_change_detected |= KATTACH_UPD_CONFIG;
			}
		}
		if (kattach_appqueue->ka_inst == (kattach_appqueue->ka_inst | CM_MSG_Q_APPQUEUE_UPDATED)) {
			kattach_sys_shm_syncback_install();
			kattach_appqueue->ka_inst ^= CM_MSG_Q_APPQUEUE_UPDATED;
			if (kattach_change_detected != (kattach_change_detected | KATTACH_UPD_INSTALL)) {
				kattach_change_detected |= KATTACH_UPD_INSTALL;
			}
		}
		if (kattach_appqueue->ka_vmst == (kattach_appqueue->ka_vmst | CM_MSG_Q_APPQUEUE_UPDATED)) {
			kattach_sys_shm_syncback_vmst();
			kattach_appqueue->ka_vmst ^= CM_MSG_Q_APPQUEUE_UPDATED;
			if (kattach_change_detected != (kattach_change_detected | KATTACH_UPD_VMST)) {
				kattach_change_detected |= KATTACH_UPD_VMST;
			}
		}
		if (kattach_appqueue->ka_vmports == (kattach_appqueue->ka_vmports | CM_MSG_Q_APPQUEUE_UPDATED)) {
			kattach_sys_shm_syncback_vmp();
			kattach_appqueue->ka_vmports ^= CM_MSG_Q_APPQUEUE_UPDATED;
			if (kattach_change_detected != (kattach_change_detected | KATTACH_UPD_VMPORTS)) {
				kattach_change_detected |= KATTACH_UPD_VMPORTS;
			}
		}
		if (kattach_appqueue->ka_vbridge == (kattach_appqueue->ka_vbridge | CM_MSG_Q_APPQUEUE_UPDATED)) {
			kattach_sys_shm_syncback_vbr();
			kattach_appqueue->ka_vbridge ^= CM_MSG_Q_APPQUEUE_UPDATED;
			if (kattach_change_detected != (kattach_change_detected | KATTACH_UPD_VBRIDGE)) {
				kattach_change_detected |= KATTACH_UPD_VBRIDGE;
			}
		}
		if (kattach_appqueue->ka_vmimages == (kattach_appqueue->ka_vmimages | CM_MSG_Q_APPQUEUE_UPDATED)) {
			kattach_sys_shm_syncback_vmi();
			kattach_appqueue->ka_vmimages ^= CM_MSG_Q_APPQUEUE_UPDATED;
			if (kattach_change_detected != (kattach_change_detected | KATTACH_UPD_VMIMAGES)) {
				kattach_change_detected |= KATTACH_UPD_VMIMAGES;
			}
		}
		if (kattach_appqueue->ka_vns == (kattach_appqueue->ka_vns | CM_MSG_Q_APPQUEUE_UPDATED)) {
			kattach_sys_shm_syncback_vns();
			kattach_appqueue->ka_vns ^= CM_MSG_Q_APPQUEUE_UPDATED;
			if (kattach_change_detected != (kattach_change_detected | KATTACH_UPD_VNS)) {
				kattach_change_detected |= KATTACH_UPD_VNS;
			}
		}
		if (kattach_appqueue->ka_cfggrp == (kattach_appqueue->ka_cfggrp | CM_MSG_Q_APPQUEUE_UPDATED)) {
			kattach_sys_shm_syncback_cfggrp();
			kattach_appqueue->ka_cfggrp ^= CM_MSG_Q_APPQUEUE_UPDATED;
			if (kattach_change_detected != (kattach_change_detected | KATTACH_UPD_CFGGRP)) {
				kattach_change_detected |= KATTACH_UPD_CFGGRP;
			}
		}
		if (kattach_appqueue->ka_fw == (kattach_appqueue->ka_fw | CM_MSG_Q_APPQUEUE_UPDATED)) {
			kattach_sys_shm_syncback_fw();
			kattach_appqueue->ka_fw ^= CM_MSG_Q_APPQUEUE_UPDATED;
			if (kattach_change_detected != (kattach_change_detected | KATTACH_UPD_FW)) {
				kattach_change_detected |= KATTACH_UPD_FW;
			}
		}
		if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_MODULE)) {
			kattach_sys_shm_syncback_appmods();
			kattach_appqueue->ak_update ^= CM_MSG_Q_APPQUEUE_MODULE;
			if (kattach_change_detected != (kattach_change_detected | KATTACH_UPD_APPMODULES)) {
				kattach_change_detected |= KATTACH_UPD_APPMODULES;
			}
		}
		kattach_appqueue->ak_update ^= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	kattach_sys_shm_checklocks();

	if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_REBOOT)) {
		kattach_reboot_me = 1;
		kattach_appqueue->ak_update ^= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	return;
}

void
kattach_sys_shm_checklocks(void)
{
	u8 fnd_lock = 0;

	/* Check if any of the bitmasks are locked by AppQueue */
	if (kattach_appqueue->ka_dev == (kattach_appqueue->ka_dev | CM_MSG_Q_APPQUEUE_LOCK)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_netdev == (kattach_appqueue->ka_netdev | CM_MSG_Q_APPQUEUE_LOCK)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_cfg == (kattach_appqueue->ka_cfg | CM_MSG_Q_APPQUEUE_LOCK)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_inst == (kattach_appqueue->ka_inst | CM_MSG_Q_APPQUEUE_LOCK)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_vmst == (kattach_appqueue->ka_vmst | CM_MSG_Q_APPQUEUE_LOCK)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_vmports == (kattach_appqueue->ka_vmports | CM_MSG_Q_APPQUEUE_LOCK)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_vbridge == (kattach_appqueue->ka_vbridge | CM_MSG_Q_APPQUEUE_LOCK)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_vmimages == (kattach_appqueue->ka_vmimages | CM_MSG_Q_APPQUEUE_LOCK)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_vns == (kattach_appqueue->ka_vns | CM_MSG_Q_APPQUEUE_LOCK)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_cfggrp == (kattach_appqueue->ka_cfggrp | CM_MSG_Q_APPQUEUE_LOCK)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_fw == (kattach_appqueue->ka_fw | CM_MSG_Q_APPQUEUE_LOCK)) {
		fnd_lock++;
	}

	if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		if (!fnd_lock) {
			kattach_appqueue->ak_update ^= CM_MSG_Q_APPQUEUE_LOCK;		/* nothing locked, clear the lock */
		}
	} else {
		if (fnd_lock) {
			kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_LOCK;		/* stuff is locked, set the lock properly */
		}
	}

	fnd_lock = 0;

	/* Check if any of the bitmasks are updated by AppQueue */
	if (kattach_appqueue->ka_dev == (kattach_appqueue->ka_dev | CM_MSG_Q_APPQUEUE_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_netdev == (kattach_appqueue->ka_netdev | CM_MSG_Q_APPQUEUE_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_cfg == (kattach_appqueue->ka_cfg | CM_MSG_Q_APPQUEUE_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_inst == (kattach_appqueue->ka_inst | CM_MSG_Q_APPQUEUE_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_vmst == (kattach_appqueue->ka_vmst | CM_MSG_Q_APPQUEUE_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_vmports == (kattach_appqueue->ka_vmports | CM_MSG_Q_APPQUEUE_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_vbridge == (kattach_appqueue->ka_vbridge | CM_MSG_Q_APPQUEUE_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_vmimages == (kattach_appqueue->ka_vmimages | CM_MSG_Q_APPQUEUE_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_vns == (kattach_appqueue->ka_vns | CM_MSG_Q_APPQUEUE_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_cfggrp == (kattach_appqueue->ka_cfggrp | CM_MSG_Q_APPQUEUE_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_fw == (kattach_appqueue->ka_fw | CM_MSG_Q_APPQUEUE_UPDATED)) {
		fnd_lock++;
	}

	if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_UPDATED)) {
		if (!fnd_lock) {
			kattach_appqueue->ak_update ^= CM_MSG_Q_APPQUEUE_UPDATED;	/* nothing updated, clear the update flag */
		}
	} else {
		if (fnd_lock) {
			kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_UPDATED;	/* stuff is updated, set the flag properly */
		}
	}

	fnd_lock = 0;

	/* Check to make sure we don't have the kattach lock set by mistake */
	if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_KATTACH_LOCK)) {
		/* we don't have it locked */
		kattach_appqueue->ak_update ^= CM_MSG_Q_KATTACH_LOCK;
	}
	if (kattach_appqueue->ka_dev == (kattach_appqueue->ka_dev | CM_MSG_Q_KATTACH_LOCK)) {
		kattach_appqueue->ka_dev ^= CM_MSG_Q_KATTACH_LOCK;
	}
	if (kattach_appqueue->ka_netdev == (kattach_appqueue->ka_netdev | CM_MSG_Q_KATTACH_LOCK)) {
		kattach_appqueue->ka_netdev ^= CM_MSG_Q_KATTACH_LOCK;
	}
	if (kattach_appqueue->ka_cfg == (kattach_appqueue->ka_cfg | CM_MSG_Q_KATTACH_LOCK)) {
		kattach_appqueue->ka_cfg ^= CM_MSG_Q_KATTACH_LOCK;
	}
	if (kattach_appqueue->ka_inst == (kattach_appqueue->ka_inst | CM_MSG_Q_KATTACH_LOCK)) {
		kattach_appqueue->ka_inst ^= CM_MSG_Q_KATTACH_LOCK;
	}
	if (kattach_appqueue->ka_vmst == (kattach_appqueue->ka_vmst | CM_MSG_Q_KATTACH_LOCK)) {
		kattach_appqueue->ka_vmst ^= CM_MSG_Q_KATTACH_LOCK;
	}
	if (kattach_appqueue->ka_vmports == (kattach_appqueue->ka_vmports | CM_MSG_Q_KATTACH_LOCK)) {
		kattach_appqueue->ka_vmports ^= CM_MSG_Q_KATTACH_LOCK;
	}
	if (kattach_appqueue->ka_vbridge == (kattach_appqueue->ka_vbridge | CM_MSG_Q_KATTACH_LOCK)) {
		kattach_appqueue->ka_vbridge ^= CM_MSG_Q_KATTACH_LOCK;
	}
	if (kattach_appqueue->ka_vmimages == (kattach_appqueue->ka_vmimages | CM_MSG_Q_KATTACH_LOCK)) {
		kattach_appqueue->ka_vmimages ^= CM_MSG_Q_KATTACH_LOCK;
	}
	if (kattach_appqueue->ka_vns == (kattach_appqueue->ka_vns | CM_MSG_Q_KATTACH_LOCK)) {
		kattach_appqueue->ka_vns ^= CM_MSG_Q_KATTACH_LOCK;
	}
	if (kattach_appqueue->ka_cfggrp == (kattach_appqueue->ka_cfggrp | CM_MSG_Q_KATTACH_LOCK)) {
		kattach_appqueue->ka_cfggrp ^= CM_MSG_Q_KATTACH_LOCK;
	}
	if (kattach_appqueue->ka_fw == (kattach_appqueue->ka_fw | CM_MSG_Q_KATTACH_LOCK)) {
		kattach_appqueue->ka_fw ^= CM_MSG_Q_KATTACH_LOCK;
	}

	/* Check if anything is updated by kattach */
	if (kattach_appqueue->ka_dev == (kattach_appqueue->ka_dev | CM_MSG_Q_KATTACH_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_netdev == (kattach_appqueue->ka_netdev | CM_MSG_Q_KATTACH_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_cfg == (kattach_appqueue->ka_cfg | CM_MSG_Q_KATTACH_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_inst == (kattach_appqueue->ka_inst | CM_MSG_Q_KATTACH_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_vmst == (kattach_appqueue->ka_vmst | CM_MSG_Q_KATTACH_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_vmports == (kattach_appqueue->ka_vmports | CM_MSG_Q_KATTACH_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_vbridge == (kattach_appqueue->ka_vbridge | CM_MSG_Q_KATTACH_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_vmimages == (kattach_appqueue->ka_vmimages | CM_MSG_Q_KATTACH_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_vns == (kattach_appqueue->ka_vns | CM_MSG_Q_KATTACH_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_cfggrp == (kattach_appqueue->ka_cfggrp | CM_MSG_Q_KATTACH_UPDATED)) {
		fnd_lock++;
	}
	if (kattach_appqueue->ka_fw == (kattach_appqueue->ka_fw | CM_MSG_Q_KATTACH_UPDATED)) {
		fnd_lock++;
	}

	if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_KATTACH_UPDATED)) {
		if (!fnd_lock) {
			kattach_appqueue->ak_update ^= CM_MSG_Q_KATTACH_UPDATED;	/* nothing is updated, reset the flag */
		}
	} else {
		if (fnd_lock) {
			kattach_appqueue->ak_update |= CM_MSG_Q_KATTACH_UPDATED;	/* stuff is updated, set the flag properly */
		}
	}

	return;
}
#endif /* !defined(KATTACH_BLD_VKAOS) */

void
kattach_sys_write_resolv(void)
{
	FILE *stream;
	char cfgfile[255];
	int i = 0;

	memset(cfgfile,0,sizeof(cfgfile));
	sprintf(cfgfile,"%s/%s",KATTACH_CFGPATH,KATTACH_CONF_RESOLV);
	stream = fopen(cfgfile,"w");

	if (stream == (FILE *)0) {
		printf("\n [!] FATAL -- Unable to write %s\n",cfgfile);
	} else {
		for (i = 0; i <= 5; i++) {
			if (kattach_cfg.dns_ip[i] == 0) continue;
			fprintf(stream,"nameserver %s\n",kattach_net_parseip(kattach_cfg.dns_ip[i]));
		}
		fclose(stream);
	}
	return;
}

#if !defined(KATTACH_BLD_VKAOS)
void
kattach_sys_write_extlinux(void)
{
	FILE *stream;
	char cfgfile[64];
	u8 mode = 0;

	memset(cfgfile,0,sizeof(cfgfile));
	sprintf(cfgfile,"%s/%s",KATTACH_BOOTPATH,KATTACH_CONF_EXTLINUX);
	stream = fopen(cfgfile,"w");

	sprintf(kattach_cfg.storedev,"%s",kattach_install.diskappq);

	if (stream == (FILE *)0) {
		printf("\n [!] FATAL -- Unable to write %s\n",cfgfile);
	} else {
		/* FIXME: add extra slots in here */
		if (kattach_cfg.mode == 0x01) {
			fprintf(stream,"default hvslot01\n");
			mode = 1;
		} else if (kattach_cfg.mode == 0x02) {					/* secondary slots are slot_id + 10, so slot 2 is 0x0c */
			fprintf(stream,"default hvslot02\n");
			mode = 2;
		} else if (kattach_cfg.mode == 0xef) {
			fprintf(stream,"default factory\n");
			mode = 1;
			kattach_cfg.mode = 0x01;
		} else {
			/* this is a default so we don't become unbootable */
			fprintf(stream,"default hvslot01\n");
		}	
		fprintf(stream,"timeout 5\n");
		fprintf(stream,"prompt 1\n");
		fprintf(stream,"label hvslot01\n");
		fprintf(stream," kernel hvImage.001\n");
		if (kattach_cfg.dhcp == 0) {
			mode = 1;
			fprintf(stream," append kaos=%u:%s:%02x%02x%02x%02x%02x%02x:%lu.%lu.%lu.%lu:%u:%lu.%lu.%lu.%lu:%lu.%lu.%lu.%lu\n",
											mode,kattach_cfg.storedev,kattach_cfg.mac[0],kattach_cfg.mac[1],
											kattach_cfg.mac[2],kattach_cfg.mac[3],kattach_cfg.mac[4],kattach_cfg.mac[5],
											(kattach_cfg.ip >> 24) & 0xff,(kattach_cfg.ip >> 16) & 0xff,
											(kattach_cfg.ip >> 8) & 0xff,(kattach_cfg.ip) & 0xff,kattach_cfg.slash,
											(kattach_cfg.gw >> 24) & 0xff,(kattach_cfg.gw >> 16) & 0xff,
											(kattach_cfg.gw >> 8) & 0xff,(kattach_cfg.gw) & 0xff,
											(kattach_cfg.dns >> 24) & 0xff,(kattach_cfg.dns >> 16) & 0xff,
											(kattach_cfg.dns >> 8) & 0xff,(kattach_cfg.dns) & 0xff);
		} else {
			mode = 1;
			fprintf(stream," append kaos=%u:%s:%02x%02x%02x%02x%02x%02x:0\n",mode,kattach_cfg.storedev,kattach_cfg.mac[0],kattach_cfg.mac[1],
											kattach_cfg.mac[2],kattach_cfg.mac[3],kattach_cfg.mac[4],kattach_cfg.mac[5]);
		}
		fprintf(stream,"label hvslot02\n");
		fprintf(stream," kernel hvImage.002\n");
		if (kattach_cfg.dhcp == 0) {
			mode = 2;
			fprintf(stream," append kaos=%u:%s:%02x%02x%02x%02x%02x%02x:%lu.%lu.%lu.%lu:%u:%lu.%lu.%lu.%lu:%lu.%lu.%lu.%lu\n",
											mode,kattach_cfg.storedev,kattach_cfg.mac[0],kattach_cfg.mac[1],
											kattach_cfg.mac[2],kattach_cfg.mac[3],kattach_cfg.mac[4],kattach_cfg.mac[5],
											(kattach_cfg.ip >> 24) & 0xff,(kattach_cfg.ip >> 16) & 0xff,
											(kattach_cfg.ip >> 8) & 0xff,(kattach_cfg.ip) & 0xff,kattach_cfg.slash,
											(kattach_cfg.gw >> 24) & 0xff,(kattach_cfg.gw >> 16) & 0xff,
											(kattach_cfg.gw >> 8) & 0xff,(kattach_cfg.gw) & 0xff,
											(kattach_cfg.dns >> 24) & 0xff,(kattach_cfg.dns >> 16) & 0xff,
											(kattach_cfg.dns >> 8) & 0xff,(kattach_cfg.dns) & 0xff);
		} else {
			mode = 2;
			fprintf(stream," append kaos=%u:%s:%02x%02x%02x%02x%02x%02x:0\n",mode,kattach_cfg.storedev,kattach_cfg.mac[0],kattach_cfg.mac[1],
											kattach_cfg.mac[2],kattach_cfg.mac[3],kattach_cfg.mac[4],kattach_cfg.mac[5]);
		}
		fprintf(stream,"label recovery2\n");
		fprintf(stream," kernel hvImage.002\n");
		if (kattach_cfg.dhcp == 0) {
			mode = 8;
			fprintf(stream," append kaos=%u:%s:%02x%02x%02x%02x%02x%02x:%lu.%lu.%lu.%lu:%u:%lu.%lu.%lu.%lu:%lu.%lu.%lu.%lu\n",
											mode,kattach_cfg.storedev,kattach_cfg.mac[0],kattach_cfg.mac[1],
											kattach_cfg.mac[2],kattach_cfg.mac[3],kattach_cfg.mac[4],kattach_cfg.mac[5],
											(kattach_cfg.ip >> 24) & 0xff,(kattach_cfg.ip >> 16) & 0xff,
											(kattach_cfg.ip >> 8) & 0xff,(kattach_cfg.ip) & 0xff,kattach_cfg.slash,
											(kattach_cfg.gw >> 24) & 0xff,(kattach_cfg.gw >> 16) & 0xff,
											(kattach_cfg.gw >> 8) & 0xff,(kattach_cfg.gw) & 0xff,
											(kattach_cfg.dns >> 24) & 0xff,(kattach_cfg.dns >> 16) & 0xff,
											(kattach_cfg.dns >> 8) & 0xff,(kattach_cfg.dns) & 0xff);
		} else {
			mode = 8;
			fprintf(stream," append kaos=%u:%s:%02x%02x%02x%02x%02x%02x:0\n",mode,kattach_cfg.storedev,kattach_cfg.mac[0],kattach_cfg.mac[1],
											kattach_cfg.mac[2],kattach_cfg.mac[3],kattach_cfg.mac[4],kattach_cfg.mac[5]);
		}
		fprintf(stream,"label recovery\n");
		fprintf(stream," kernel hvImage.001\n");
		if (kattach_cfg.dhcp == 0) {
			mode = 8;
			fprintf(stream," append kaos=%u:%s:%02x%02x%02x%02x%02x%02x:%lu.%lu.%lu.%lu:%u:%lu.%lu.%lu.%lu:%lu.%lu.%lu.%lu\n",
											mode,kattach_cfg.storedev,kattach_cfg.mac[0],kattach_cfg.mac[1],
											kattach_cfg.mac[2],kattach_cfg.mac[3],kattach_cfg.mac[4],kattach_cfg.mac[5],
											(kattach_cfg.ip >> 24) & 0xff,(kattach_cfg.ip >> 16) & 0xff,
											(kattach_cfg.ip >> 8) & 0xff,(kattach_cfg.ip) & 0xff,kattach_cfg.slash,
											(kattach_cfg.gw >> 24) & 0xff,(kattach_cfg.gw >> 16) & 0xff,
											(kattach_cfg.gw >> 8) & 0xff,(kattach_cfg.gw) & 0xff,
											(kattach_cfg.dns >> 24) & 0xff,(kattach_cfg.dns >> 16) & 0xff,
											(kattach_cfg.dns >> 8) & 0xff,(kattach_cfg.dns) & 0xff);
		} else {
			mode = 8;
			fprintf(stream," append kaos=%u:%s:%02x%02x%02x%02x%02x%02x:0\n",mode,kattach_cfg.storedev,kattach_cfg.mac[0],kattach_cfg.mac[1],
											kattach_cfg.mac[2],kattach_cfg.mac[3],kattach_cfg.mac[4],kattach_cfg.mac[5]);
		}
		fprintf(stream,"label factory\n");
		fprintf(stream," kernel hvImage.001\n");
		fprintf(stream,"label factory2\n");
		fprintf(stream," kernel hvImage.002\n");
		fclose(stream);

	}
	return;
}

void
kattach_sys_hvimage(void)
{
	int x = 0, ret = 0;
	struct stat k_imgstatus;
	char lcmd[255];
	char hvpath[128];
	char dspath[128];
	u8 mode = 0;

	if (kattach_cfg.mode == 0x01) mode = 0x01;
	if (kattach_cfg.mode == 0x02) mode = 0x02;

	if (kattach_install.diskboot[0] == '\0') return;				/* santity check */

	/* check that hvImage.00x, where x is the slot id, is downloaded */
	sprintf(dspath,"%shvImage.00%u",KATTACH_APPQUEUE_HVIMGPATH,mode);
	x = stat(dspath,&k_imgstatus);
	if (x < 0) {
		return;									/* not found */
	}
	/* next check to see if /boot/extlinux.sys is there */
	sprintf(hvpath,"%s/extlinux.sys",KATTACH_BOOTPATH);
	x = stat(hvpath,&k_imgstatus);

	if (x < 0) {
		/* get /boot ready */
		ret = mkdir(KATTACH_BOOTPATH,KATTACH_PERM);
		sprintf(hvpath,"/dev/%s",kattach_install.diskboot);
		ret = mount(hvpath,KATTACH_BOOTPATH,KATTACH_FS_EXT2, MS_RELATIME, "");
	}
	/* check to see if an existing image is there */
	sprintf(hvpath,"%s/hvImage.00%u",KATTACH_BOOTPATH,mode);
	x = stat(hvpath, &k_imgstatus);

	if (x < 0) {
		/* no existing image, just move the file over */
		sprintf(lcmd,"%s%s %s %s",KATTACH_BINPATH,KCMD_MV,dspath,hvpath);
		kattach_sysexec(lcmd);
	} else {
		/* file exists, rename it using random UUID */
		kattach_getruuid();
		memset(dspath,0,sizeof(dspath));
		sprintf(dspath,"%s/hvImage.%s",KATTACH_BOOTPATH,kattach_ruuid);
		sprintf(lcmd,"%s%s %s %s",KATTACH_BINPATH,KCMD_MV,hvpath,dspath);
		kattach_sysexec(lcmd);
		memset(dspath,0,strlen(dspath));
		/* now move over the new image */
		sprintf(dspath,"%shvImage.00%u",KATTACH_APPQUEUE_HVIMGPATH,mode);
		sprintf(lcmd,"%s%s %s %s",KATTACH_BINPATH,KCMD_MV,dspath,hvpath);
		kattach_sysexec(lcmd);
	}
	kattach_sys_write_extlinux();								/* update the default boot image */
	sprintf(hvpath,"%s%s /dev/%s",KATTACH_BINPATH,KCMD_UMOUNT,kattach_install.diskboot);
	kattach_sysexec(hvpath);								/* clean up */

	printf("\n [*] Installed new Hypervisor kernel image to slot %u.\n", mode);

	return;
}

void
kattach_sys_debug_akflags(u8 fmt, u8 dbgtag)
{
	FILE *stream;
        char *logfile = "/kaos/cfg/var/log/flg.log";

        stream = fopen(logfile,"a");
        if (stream == (FILE *)0) {
                printf("\n [#] ERROR: Unable to write log file %s.\n", logfile);
                return;
        }

        if (fmt) {
                fprintf(stream,"\n## Shared Memory %04u #######\n",dbgtag);
                fprintf(stream,"       ka_dev      =  %04x\n",kattach_appqueue->ka_dev);
                fprintf(stream,"       ka_cfg      =  %04x\n",kattach_appqueue->ka_cfg);
                fprintf(stream,"       ka_inst     =  %04x\n",kattach_appqueue->ka_inst);
                fprintf(stream,"       ka_vmst     =  %04x\n",kattach_appqueue->ka_vmst);
                fprintf(stream,"       ka_vbridge  =  %04x\n",kattach_appqueue->ka_vbridge);
                fprintf(stream,"       ka_vmimages =  %04x\n",kattach_appqueue->ka_vmimages);
                fprintf(stream,"       ka_netdev   =  %04x\n",kattach_appqueue->ka_netdev);
                fprintf(stream,"       ka_vns      =  %04x\n",kattach_appqueue->ka_vns);
                fprintf(stream,"       ka_cfggrp   =  %04x\n",kattach_appqueue->ka_cfggrp);
                fprintf(stream,"       ka_fw       =  %04x\n",kattach_appqueue->ka_fw);
                fprintf(stream,"       ak_update   =  %04x\n",kattach_appqueue->ak_update);
                fprintf(stream,"###############################\n\n");
        } else {
                fprintf(stream,"\n [:] shm_dbg-%u: %03x | %03x | %03x | %03x | %03x | %03x | %03x | %03x | %03x | %03x | %03x\n", dbgtag,
                        kattach_appqueue->ka_dev,kattach_appqueue->ka_cfg,kattach_appqueue->ka_inst,kattach_appqueue->ka_vmst,
                        kattach_appqueue->ka_vbridge, kattach_appqueue->ka_vmimages,kattach_appqueue->ka_netdev,kattach_appqueue->ka_vns, kattach_appqueue->ka_cfggrp, kattach_appqueue->ka_fw, kattach_appqueue->ak_update);
        }
	fclose(stream);

        return;
}
#endif /* !defined(KATTACH_BLD_VKAOS) */
