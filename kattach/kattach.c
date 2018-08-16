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
 * Source File :                kattach.c
 * Purpose     :                main program control
 * Callers     :                none - first point of execution
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/reboot.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <sys/swap.h>
#include <signal.h>
#include <linux/fs.h>
#include <linux/reboot.h>
#include <string.h>
#include "kattach_types.h"
#include "kattach.h"

static void kattach_wdog_zombie(int sig);

int main(int argc, char **argv)
{
	pid_t p = getpid();
	struct sigaction kspike;

	/* initialize console, enable proc and setup dev */
	kattach_thedawn();

	if (p != 1) {
		printf("\n\n [!] WARNING: kattach is not pid 1\n");
	}

	/* process kernel cmdline */
	kattach_kcmdline();

	/* initialize system */
	memset(&kspike,0,sizeof(kspike));
	kspike.sa_handler = kattach_wdog_zombie;

	if (sigaction(SIGCHLD, &kspike, 0)) {
		printf("\n\n [!] WARNING - Unable to change signal action.\n");
	}

	kattach_network();
	kattach_init_sys();
#if !defined(KATTACH_BLD_VKAOS)
	kattach_vm_init();
	kattach_vm_netdev_init();
#endif /* !defined(KATTACH_BLD_VKAOS) */
#if defined(KATTACH_BLD_VKAOS)
	kattach_vkaos_launch();
	kattach_vkaos_loop();
#else /* defined(KATTACH_BLD_VKAOS) */

	/* Guest or Hypervisor mode */
	if (kattach_cfg.mode == KATTACH_MODE_VKAOS) {
		kattach_vkaos_launch();
		kattach_vkaos_loop();
	} else if ((kattach_setup) && (kattach_dbactive == 111)) {
		kattach_sshboot();
		if (!kattach_recovery) {
			kattach_vm_launch();
		}
		kattach_sys_shm_sync();
		kattach_loop();
	} else {
		kattach_sys_shm_sync();
		kattach_loop();
	}
#endif /* defined(KATTACH_BLD_VKAOS) */

	return 0;
}

#if !defined(KATTACH_BLD_VKAOS)
void 
kattach_sshboot(void)
{
	char lcmd[255];
	char mntpt[64];
	int ret = 0, rc = 0;
	
	ret = mkdir(KATTACH_BOOTPATH, KATTACH_PERM);
	sprintf(mntpt,"/dev/%s",kattach_install.diskboot);
	ret = mount(mntpt,KATTACH_BOOTPATH,KATTACH_FS_EXT2, MS_RELATIME, "");
	kattach_sys_sshd();
	rc = usleep(KATTACH_TIMER_QEMU_SPINUP);
	sprintf(lcmd,"%s%s %s",KATTACH_BINPATH,KCMD_UMOUNT,mntpt);
	rc = usleep(KATTACH_TIMER_QEMU_SPINUP);
	kattach_sysexec(lcmd);

	return;
}
#endif /* !defined(KATTACH_BLD_VKAOS) */

void
kattach_reboot(void)
{
	int cmd = LINUX_REBOOT_CMD_RESTART;
	int res = 0;
#if !defined(KATTACH_BLD_VKAOS)
	char lcmd[255];
	char ucmd[64];
	int rc = 0;
#endif /* !defined(KATTACH_BLD_VKAOS) */

	/* FIXME: sync, close shm, files, db, kill all pids, umount disks, and then reboot */
	sync();
#if !defined(KATTACH_BLD_VKAOS)
	if (kattach_setup) {
		kattach_vm_fw_syncdb();				/* sync the fw info to the databases */
		sync();
		kattach_sql_dbclose();				/* close out sqlite databases */
		kattach_vm_shutdown();				/* shutdown all the VMs */
		rc = usleep(KATTACH_TIMER_QEMU_SPINUP);
		if (kattach_cfg.pid_dhcpd) {
			res = kill(kattach_cfg.pid_dhcpd, SIGKILL);
		}
		sync();
		rc = usleep(KATTACH_TIMER_QEMU_SPINUP);
		if (kattach_install.diskboot[0] != '\0') {
			sprintf(ucmd,"/dev/%s",kattach_install.diskboot);
			sprintf(lcmd,"%s%s %s",KATTACH_BINPATH,KCMD_UMOUNT,ucmd);
                        kattach_sysexec(lcmd);
		}
		rc = usleep(KATTACH_TIMER_QEMU_SPINUP);
		if (kattach_install.diskdata[0] != '\0') {
			sprintf(ucmd,"/dev/%s",kattach_install.diskdata);
			sprintf(lcmd,"%s%s %s",KATTACH_BINPATH,KCMD_UMOUNT,ucmd);
                        kattach_sysexec(lcmd);
		}
		rc = usleep(KATTACH_TIMER_QEMU_SPINUP);
		if (kattach_install.diskappq[0] != '\0') {
			sprintf(ucmd,"/dev/%s",kattach_install.diskappq);
			sprintf(lcmd,"%s%s %s",KATTACH_BINPATH,KCMD_UMOUNT,ucmd);
                        kattach_sysexec(lcmd);
		}
		if (kattach_install.diskswap[0] != '\0') {
			sprintf(ucmd,"/dev/%s",kattach_install.diskswap);
			res = swapoff(ucmd);
		}
	}
	kattach_thedawn_shm_close();			/* close out shm files */
#endif /* !defined(KATTACH_BLD_VKAOS) */
	res = reboot(cmd);

	return;						/* this should never get executed */
}

static void
kattach_wdog_zombie(int sig)
{
	while(waitpid(-1, NULL, WNOHANG) > 0) {
		/* do nothing */
	}

	return;
}
