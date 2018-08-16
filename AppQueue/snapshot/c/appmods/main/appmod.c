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
#include <sys/types.h>
#include "appmod.h"
#include "appmod_shared.h"

int main(int argc, char **argv)
{
	if (argc != 2) {
		printf("\n\nAppQueue CLI module requires a single argument\n\n");
		exit(1);
	} else if (getuid() != APPMOD_CLI_AUTH_UID) {
		printf("\n");
		printf("This user is not authorized to run this application.\n\n");
		exit(1);
	} else {
		appmod_shm_init();                      				/* initialize shared memory interface to appmgr */
		appmod_test_shm->status |= APPMOD_STATUS_CLI_ENTER;
		sprintf(appmod_test_shm->cfggrp,"%s",argv[1]);				/* copy the group name over */
		sprintf(appmod_prompt,"[%s | ",argv[1]);
		appmod_cli_user_auth = APPMOD_CLI_AUTH_ADMIN;				/* FIXME: security / only ADMINs can launch modules from AppQueue */
		appmod_cli();
		appmod_test_shm->status |= APPMOD_STATUS_CLI_EXIT;
		appmod_shm_close();
	}
	exit(1);
}
