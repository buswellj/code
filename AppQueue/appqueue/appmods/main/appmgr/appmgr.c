/*
 * AppQueue Module Framework
 * Copyright (c) 2009 - 2010 Carbon Mountain LLC.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "sqlite3.h"
#include "appmgr.h"
#include "appmod_shared.h"

int main(int argc, char **argv)
{
	appmod_test_t *tPtr = &appmod_test;

	memset(tPtr,0,sizeof(appmod_test_t));
	appmgr_shm_init();
	appmgr_sql_init();
	appmgr_loop();

	appmgr_shm_close();

	exit(1);
}

void
appmgr_loop(void)
{
	appmod_test_t *tPtr = &appmod_test;
	u8 y = 0;
	unsigned int sleepy = 500000;
	char cfggrp[64];

	while (!y) {
		if (appmod_test_shm->status == (appmod_test_shm->status | APPMOD_STATUS_CLI_ENTER)) {
			/* new group */
			if (appmod_test_shm->cfggrp[0] != '\0') {
				sprintf(cfggrp,"%s",appmod_test_shm->cfggrp);
				appmgr_sql_read(cfggrp);				/* see if there is a database table for this */
				appmod_test_shm->status ^= APPMOD_STATUS_CLI_ENTER;
			}
		}
		if (appmod_test_shm->status == (appmod_test_shm->status | APPMOD_STATUS_CLI_EXIT)) {
			if (appmod_test_shm->cfggrp[0] != '\0') {
				sprintf(cfggrp,"%s",appmod_test_shm->cfggrp);

				/* write shm to memory */
				appmgr_shm_sync();
		
				/* write memory to database */
				appmgr_sql_write(cfggrp);

				/* generate config to /appq/am/cfg/<group>/raw/ */
				appmgr_cfg_write();

				/* nuke memory structure */
				memset(tPtr,0,sizeof(appmod_test_t));

				appmod_test_shm->status ^= APPMOD_STATUS_CLI_EXIT;
				
				/* close / open shm */
				appmgr_shm_close();
				appmgr_shm_init();

			}
		}
		if (appmod_test_shm->status == (appmod_test_shm->status | APPMOD_STATUS_CLI_UPD)) {
			/* the CLI has updated the configuration */
			if (appmod_test_shm->cfggrp[0] != '\0') {
				appmgr_shm_sync();
			}
			printf("\nThe config option is set to %lu\n",appmod_test_shm->config_option);
		}

		usleep(sleepy);
	}

	/* FIXME: need to gracefully shutdown properly via kattach_shm */
	return;
}

void
appmgr_cfg_write(void)
{
	FILE *stream;
	char cfgfile[255];
	int ret = 0;

	memset(cfgfile,0,sizeof(cfgfile));
	sprintf(cfgfile,"/appq/am/cfg/%s/",appmod_test.cfggrp);
	ret = mkdir(cfgfile,APPMGR_PERM);
	sprintf(cfgfile,"/appq/am/cfg/%s/raw/",appmod_test.cfggrp);
	ret = mkdir(cfgfile,APPMGR_PERM);
	sprintf(cfgfile,"/appq/am/cfg/%s/raw/appm",appmod_test.cfggrp);
	ret = mkdir(cfgfile,APPMGR_PERM);
	sprintf(cfgfile,"/appq/am/cfg/%s/raw/appm/cfg",appmod_test.cfggrp);
	ret = mkdir(cfgfile,APPMGR_PERM);
	sprintf(cfgfile,"/appq/am/cfg/%s/raw/appm/cfg/test.conf",appmod_test.cfggrp);
//	sprintf(cfgfile,"./cfg");
	stream = fopen(cfgfile,"w");

	if (stream == (FILE *)0) {
		printf("\n [!] FATAL -- Unable to write %s\n",cfgfile);
	} else {
		fprintf(stream,"config_option = %lu",appmod_test.config_option);
		fclose(stream);
	}
	return;
}
