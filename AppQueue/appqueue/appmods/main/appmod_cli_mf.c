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
#include "appmod.h"
#include "appmod_shared.h"

void
appmod_cli_mf_main(void)
{
	u8 i = 0;

	if (!appmod_po) {
                printf("\n\n\n");
                printf("%s",appmod_cli_menu_main.cli_menu_title);
                printf("%s\n\n",APPMOD_MENU_BAR);

                for (i = 0; i < appmod_cli_menu_main.index; i++) {
                        printf("     %s  \t - %s\n",appmod_cli_menu_main.climenu[i].menu_cmd,appmod_cli_menu_main.climenu[i].menu_desc);
                }
        }

        printf("\n\n%sApp Config]# ",appmod_prompt);
        appmod_cli_current_menu = APPMOD_CLI_FP_MAIN;
        appmod_po = 0;
        return;
}

void
appmod_cli_mf_test(void)
{
	u8 i = 0;

	if (!appmod_po) {
                printf("\n\n\n");
                printf("%s",appmod_cli_menu_test.cli_menu_title);
                printf("%s\n\n",APPMOD_MENU_BAR);

                for (i = 0; i < appmod_cli_menu_test.index; i++) {
                        printf("     %s  \t - %s\n",appmod_cli_menu_test.climenu[i].menu_cmd,appmod_cli_menu_test.climenu[i].menu_desc);
                }
        }

        printf("\n\n%sTest]# ",appmod_prompt);
        appmod_cli_current_menu = APPMOD_CLI_FP_TEST;
        appmod_po = 0;
        return;
}

void
appmod_cli_mf_exit(void)
{
	appmod_cli_current_menu = APPMOD_CLI_FP_MAIN;
	appmod_exit = 1;
	return;
}

void
appmod_cli_mf_test_hello(void)
{
	printf("\n\nHello this is the test command the value is %lu\n",appmod_test_shm->config_option);
	appmod_test_shm->config_option++;
	if (appmod_test_shm->status != (appmod_test_shm->status | APPMOD_STATUS_CLI_UPD)) {
		appmod_test_shm->status |= APPMOD_STATUS_CLI_UPD;
	}
	printf("\nThe new value is %lu\n",appmod_test_shm->config_option);

	appmod_cli_current_menu = APPMOD_CLI_FP_TEST;
	return;
}
