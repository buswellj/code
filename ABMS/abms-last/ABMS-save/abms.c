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
 *
 * What is ABMS?
 * --------------
 *
 * ABMS provides a management layer in early user space. It replaces the init
 * process and handles the image based system used in AppOS. It is designed to
 * provide a mechanism of being always "managable" regardless of how messed up
 * the system becomes.
 *
 *
 * ABMS Quickstart
 * -----------------
 *
 *  +--------------------+
 *  |        BIOS        |
 *  +--------------------+
 *  |     Boot Loader    |      <--- Grub, Lilo, etc.
 *  +--------------------+
 *  |     Linux Kernel   |      <--- Kernel 
 *  +--------------------+
 *  |        ABMS        |      <--- We setup the initramfs, network etc.
 *  +--------------------+
 *  |        init        |      <--- we act as an init replacement
 *  +--------------------+
 *  |      AppStacks     |      <--- we load, configure and launch appliance stacks
 *  +--------------------+
 *
 */

#include <stdio.h>
#include <unistd.h>
#include "abms.h"

int main(int argc, char **argv, char **envp)
{
	u8 rc = 0;

	abms_cfg.debug = 0;
	abms_cfg.sshport = ABMS_SSHPORT;

	/* open console */
	abms_disp_init();

	/* init */
	abms_init();

	/* parse cmdline */
	abms_cmdline();

	/* XXX: check and lock grsec sysctl */

	/* setup network */
	rc = abms_net();

	if (rc == RC_FAIL) {
	    /* network setup failed, need to determine why and recover */
	}

	abms_net_sshd();

	/* XXX: packetflex support - protect sshd */

	if (abms_deployed) {
//	    abms_load_stacks();
	}

	/* watchdog mode */
	abms_init_loop();

	/* we never reach this point */
	return 0;
}

void abms_init_loop(void)
{

	char lcmd[255];

        sprintf(lcmd,"%s%s /dev/tty%d c 4 %d",ABMS_BINPATH,ACMD_MKNOD,ABMS_SECTTYV,ABMS_SECTTYV);
        abms_system(lcmd);

        sprintf(lcmd,"%s%s og-r /dev/%s",ABMS_BINPATH,ACMD_CHMOD,ABMS_SECTTY);
        abms_system(lcmd);

	abms_loop_getty();

        printf("\n\n");
        printf("Spliced Networks (R) Appliance Operating System Software\n");
        printf("AppOS (R) Boot Management System, Version %s\n", ABMS_VERSION);
        printf("%s\n\n",SN_COPYRIGHT);

	printf("\nAppliance active, please use management console.\n\n");

	if (abms_cfg.debug) {
            printf("mode      = %u\n", abms_cfg.mode);
            printf("ip        = %lu.%lu.%lu.%lu\t %llx\n",(u16) ((abms_cfg.ip >> 24) & 0xff), (u16) ((abms_cfg.ip >> 16) & 0xff), (u16) ((abms_cfg.ip >> 8) & 0xff), (u16) ((abms_cfg.ip) & 0xff), abms_cfg.ip);
            printf("slash     = %lu\n",abms_cfg.slash);
            printf("gw        = %lu.%lu.%lu.%lu\t %llx\n",(u16) ((abms_cfg.gw >> 24) & 0xff), (u16) ((abms_cfg.gw >> 16) & 0xff), (u16) ((abms_cfg.gw >> 8) & 0xff), (u16) ((abms_cfg.gw) & 0xff), abms_cfg.gw);
            printf("dns       = %lu.%lu.%lu.%lu\t %llx\n",(u16) ((abms_cfg.dns >> 24) & 0xff), (u16) ((abms_cfg.dns >> 16) & 0xff), (u16) ((abms_cfg.dns >> 8) & 0xff), (u16) ((abms_cfg.dns) & 0xff), abms_cfg.dns);
            printf("ipab      = %lu.%lu.%lu.%lu\t %llx\n",(u16) ((abms_cfg.ipab >> 24) & 0xff), (u16) ((abms_cfg.ipab >> 16) & 0xff), (u16) ((abms_cfg.ipab >> 8) & 0xff), (u16) ((abms_cfg.ipab) & 0xff), abms_cfg.ipab);
            printf("mac       = %02x:%02x:%02x:%02x:%02x:%02x\n",abms_cfg.mac[0],abms_cfg.mac[1],abms_cfg.mac[2],abms_cfg.mac[3],abms_cfg.mac[4],abms_cfg.mac[5]);
            printf("flash     = /dev/%s\n",abms_cfg.flashdev);
            printf("slot      = %u\n",abms_cfg.slot);
            printf("sshport   = %lu\n",abms_cfg.sshport);
            printf("debug     = %u\n\n",abms_cfg.debug);

	    printf("Real time debug mode enabled \n\n");

	    sprintf(lcmd,"exec %s%s",ABMS_BINPATH,ACMD_SH);
	    abms_system(lcmd);
	}

	while (1) {
		sleep(10);
	}

	abms_init_loop();	/* evil recursion */
}
