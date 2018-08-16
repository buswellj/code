/*
 * AppQueue Module Framework
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

#define APPMOD_STATUS_CLI_LOCK		0x01
#define APPMOD_STATUS_CLI_UPD		0x02
#define APPMOD_STATUS_MGR_LOCK		0x04
#define APPMOD_STATUS_MGR_UPD		0x08
#define APPMOD_STATUS_CLI_EXIT		0x10
#define APPMOD_STATUS_CLI_ENTER		0x20

#define APPMOD_SHM_FILE_TEST		"/am_cm_test.shm"

/* file descriptors */
int appmod_fd_shm_test;			

/* structure */

typedef struct {
	char cfggrp[64];					/* required config group name */
	u8 status;						/* required status mask */
	u32 config_option;					/* example config option */
} appmod_test_t;


/* shared memory structures */
appmod_test_t *appmod_test_shm;

/* local memory structures */
appmod_test_t appmod_test;

/* functions */
void appmod_shm_init(void);
int appmod_shm_open(char *appmod_shm_path);
void appmod_shm_close(void);

