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
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include "sqlite3.h"
#include "appmgr.h"
#include "appmod_shared.h"

void
appmgr_shm_init(void)
{
	appmod_fd_shm_test = appmgr_shm_open(APPMOD_SHM_FILE_TEST);

	ftruncate(appmod_fd_shm_test, sizeof(appmod_test_t));

	if ((appmod_test_shm = mmap(0, sizeof(appmod_test_t), (PROT_READ | PROT_WRITE), MAP_SHARED, appmod_fd_shm_test, 0)) == MAP_FAILED) {
		printf("\n [!] FATAL - Unable to map shared memory!\n");
	}

	return;
}

int
appmgr_shm_open(char *appmod_shm_path)
{
	int appmod_shm_fd = 0, res = 0;

	if ((appmod_shm_fd = shm_open(appmod_shm_path, (O_CREAT | O_EXCL | O_RDWR), (S_IREAD | S_IWRITE))) > 0) {
		printf("\n [*] Opened - %s first \n", appmod_shm_path);
	} else if ((appmod_shm_fd = shm_open(appmod_shm_path, (O_CREAT | O_RDWR), (S_IREAD | S_IWRITE))) < 0) {
		printf("\n [!] Unexpected SHM Error - %s\n",appmod_shm_path);
		return 0;
	}

	res = fchown(appmod_shm_fd, APPMOD_UID_ROOT, APPMOD_GID_APPQ);
	res = fchmod(appmod_shm_fd, APPMOD_PERM_SHM);

	return appmod_shm_fd;
}

void
appmgr_shm_close(void)
{
	fsync(appmod_fd_shm_test);
	close(appmod_fd_shm_test);
	shm_unlink(APPMOD_SHM_FILE_TEST);
	return;
}

void
appmgr_shm_sync(void)
{
	u8 y = 0;
	u16 cnt = 0;

	while (!y) {
		if (appmod_test_shm->status == (appmod_test_shm->status | APPMOD_STATUS_CLI_LOCK)) {
			cnt++;
			if (cnt > 32767) {
				printf("\n [!] FATAL -- Shared Memory Locked by CLI after timeout\n");
				y++;
				break;
			}
		}
		if (appmod_test_shm->status == (appmod_test_shm->status | APPMOD_STATUS_CLI_UPD)) {
			sprintf(appmod_test.cfggrp,"%s",appmod_test_shm->cfggrp);
			appmod_test.config_option = appmod_test_shm->config_option;
			appmod_test_shm->status ^= APPMOD_STATUS_CLI_UPD;
			y++;
			break;
		} else {
			sprintf(appmod_test_shm->cfggrp,"%s",appmod_test.cfggrp);
			appmod_test_shm->config_option = appmod_test.config_option;
			if (appmod_test_shm->status != (appmod_test_shm->status | APPMOD_STATUS_MGR_UPD)) {
				appmod_test_shm->status |= APPMOD_STATUS_MGR_UPD;
			}
			y++;
			break;
		}
	}
	return;

}
