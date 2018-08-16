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
appmod_cli_init(void)
{
	appmod_cli_menu_t *mPtr;

	mPtr = &appmod_cli_menu_main;
	memset(mPtr,0,sizeof(appmod_cli_menu_main));
	mPtr = &appmod_cli_menu_test;
	memset(mPtr,0,sizeof(appmod_cli_menu_test));

	appmod_cli_current_menu = APPMOD_CLI_FP_MAIN;				/* initialize CLI to the main menu */
	appmod_xmenu[APPMOD_CLI_FP_MAIN] = appmod_cli_mf_main;			/* main menu is a special case */

	/* set menu titles */
	sprintf(appmod_cli_menu_main.cli_menu_title,"[Main Menu]\n");

	/* main menu */
	appmod_cli_init_main("test","Test Menu", APPMOD_CLI_FP_TEST);
	appmod_xmenu[APPMOD_CLI_FP_TEST] = appmod_cli_mf_test;

	appmod_cli_init_main("exit","Exit", APPMOD_CLI_FP_EXIT);
	appmod_xmenu[APPMOD_CLI_FP_EXIT] = appmod_cli_mf_exit;

	/* test menu */
	appmod_cli_init_test("hello","Hello Command", APPMOD_CLI_FP_TEST_HELLO);
	appmod_xmenu[APPMOD_CLI_FP_TEST_HELLO] = appmod_cli_mf_test_hello;


	return;
}

void
appmod_cli_init_main(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
        u8 index = appmod_cli_menu_main.index;

        sprintf(appmod_cli_menu_main.climenu[index].menu_cmd,"%s",cli_menu_cmd);
        sprintf(appmod_cli_menu_main.climenu[index].menu_desc,"%s",cli_menu_desc);
        appmod_cli_menu_main.climenu[index].menu_perms = APPMOD_CLI_AUTH_ADMIN;
        appmod_cli_menu_main.climenu[index].menu_func = cli_menu_func;

        index++;
        appmod_cli_menu_main.index = index;
        return;
}

void
appmod_cli_init_test(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
        u8 index = appmod_cli_menu_test.index;

        sprintf(appmod_cli_menu_test.climenu[index].menu_cmd,"%s",cli_menu_cmd);
        sprintf(appmod_cli_menu_test.climenu[index].menu_desc,"%s",cli_menu_desc);
        appmod_cli_menu_test.climenu[index].menu_perms = APPMOD_CLI_AUTH_ADMIN;
        appmod_cli_menu_test.climenu[index].menu_func = cli_menu_func;

        index++;
        appmod_cli_menu_test.index = index;
        return;
}

