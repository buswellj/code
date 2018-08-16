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
 * Source File :                kattach-vkaos.c
 * Purpose     :                vKaOS specific code
 * Callers     :                kattach.c - main()
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "kattach_types.h"
#include "kattach.h"

void
kattach_vkaos_launch(void)
{
	char lcmd[255];

	/* FIXME: this is hacked together for 0.6.0. KaOS Hypervisor
	 *	  needs to write /kaos/apps/vkaos.map which contains
	 *	  [appname]
	 *	   start = start_cmd
	 *	   stop  = stop_cmd
	 *	   pid   = pid_file (for monitoring)
	 *	   tcp   = tcp_ports (for monitoring)
	 *	   udp	 = udp_ports (for monitoring)
	 */

	/* KaOS Hypervisor assembles the disk image in /dev/vda 
	 * kattach_init_vkaos() already has mounted it in /kaos/apps
	 * KaOS Hypervisor has tied together start commands into
	 * /kaos/apps/vkaos_launch.sh. 
	 */

	sprintf(lcmd,"%s/%s",KATTACH_VKAOS_APPPATH,KATTACH_VKAOS_LAUNCH);
	kattach_sysexec(lcmd);
	return;

}

void
kattach_vkaos_loop(void)
{
	int x = 0;
	char lcmd[255];
	char ccmd[64];
	unsigned int sleepy = 250000;


	sprintf(ccmd,"%s%s",KATTACH_SBINPATH,KCMD_GETTY);
	sprintf(lcmd,"-L 115200 tty1");
	kattach_gettypid[0] = kattach_bkexec(ccmd,lcmd);

	while (x == 0) {
		kattach_vkaos_loop_monitor();
		/* kattach_vkaos_app_monitor(); */	/* FIXME: add this in 0.6.1 */
		if (kattach_reboot_me != 0) {
			kattach_reboot();
		}
		usleep(sleepy);
	}
	return;

}

void
kattach_vkaos_loop_monitor(void)
{
	char procpath[32];
	char ccmd[64];
	char lcmd[255];
	struct stat k_procstatus;
	int x = 0, z = 0;
	struct timeval looptime;
	time_t looputime;

	memset(procpath,0,sizeof(procpath));
	memset(lcmd,0,sizeof(lcmd));

	sprintf(procpath,"%s/%d",KATTACH_PROCPATH,kattach_gettypid[0]);
	x = stat(procpath,&k_procstatus);

	if (x < 0) {
		/* getty is not running, restart it! */
		sprintf(ccmd,"%s%s",KATTACH_SBINPATH,KCMD_GETTY);
		sprintf(lcmd,"-L 115200 tty1");
		kattach_gettypid[0] = kattach_bkexec(ccmd,lcmd);
	}

	gettimeofday(&looptime, NULL);
	looputime = looptime.tv_sec;
	if (looputime > kattach_ntptime) {
		kattach_ntptime = looputime + 600;				/* sync every 10 minutes */
		sprintf(ccmd,"%s%s",KATTACH_SBINPATH,KCMD_HWCLOCK);
		sprintf(lcmd,"-s");
		z = kattach_bkexec(ccmd,lcmd);
	}

	/* FIXME: Add dropbear to monitoring */
	return;
}
