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
#include <ctype.h>
#include "appmod.h"
#include "appmod_shared.h"

void
appmod_cli(void)
{
	appmod_po = 0;				/* display prompt only */
	appmod_exit = 0;			/* exit cli */

	appmod_cli_init();			/* initialize the cli */

	printf("\n");
	printf("AppQueue Module Framework version %s\n",APPQUEUE_VERSION);
	printf("%s\n",APPQUEUE_COPYRIGHT);
	printf("%s\n\n",APPQUEUE_LICENSE);

	while (!appmod_exit) {
		appmod_cli_current_menu = APPMOD_CLI_FP_MAIN;
		appmod_cli_loop();
	}

	return;
}

void
appmod_cli_loop(void)
{
	u8 aqli = 1, cnt = 0, newcmd = 0, cpass = 0, fnd = 0;
	u16 cli_func = 0;
	char c = '\0';
	char cmdcli[64];

	appmod_cli_caller = (void *) appmod_xmenu[appmod_cli_current_menu];
	appmod_cli_caller();
	memset(cmdcli,0,strlen(cmdcli));

	while (aqli) {
		if (appmod_exit) return;
		if (cnt >= 64) newcmd = 1;
		if (newcmd) {
			appmod_cli_caller = (void *) appmod_xmenu[appmod_cli_current_menu];
			appmod_cli_caller();
			newcmd = 0;
			cnt = 0;
			memset(cmdcli,0,sizeof(cmdcli));
			continue;
		}
		c = tolower(appmod_cli_getch(1));
		if (c == '\n') {
//			printf("\nDEBUG: %s %d\n\n",cmdcli,strlen(cmdcli));
			switch (appmod_cli_current_menu) {
				case APPMOD_CLI_FP_MAIN:
					for (cpass = 0; cpass < appmod_cli_menu_main.index; cpass++) {
                                                if (strlen(appmod_cli_menu_main.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
                                                if (strncmp(cmdcli,appmod_cli_menu_main.climenu[cpass].menu_cmd,strlen(appmod_cli_menu_main.climenu[cpass].menu_cmd))) {
                                                        continue;
                                                } else if (appmod_cli_user_auth < appmod_cli_menu_main.climenu[cpass].menu_perms) {
                                                        fnd = 1;
                                                        break;
                                                } else {
                                                        cli_func = appmod_cli_menu_main.climenu[cpass].menu_func;
                                                        appmod_cli_caller = (void *) appmod_xmenu[cli_func];
                                                        appmod_cli_caller();
                                                        if ((cli_func == APPMOD_CLI_FP_TEST) ||
                                                            (cli_func == APPMOD_CLI_FP_TEST) ||
                                                            (cli_func == APPMOD_CLI_FP_TEST)) {
                                                                fnd = 2;
                                                        } else {
                                                                fnd = 1;
                                                        }
                                                        break;
                                                }
                                        }
                                        if (fnd == 2) {
                                                newcmd = 0;
                                        } else if (fnd == 1) {
                                                appmod_po = 1;
                                                newcmd = 1;
                                        }
                                        cnt = 0;

				break;

				case APPMOD_CLI_FP_TEST:
					for (cpass = 0; cpass < appmod_cli_menu_test.index; cpass++) {
                                                if (strlen(appmod_cli_menu_test.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
                                                if (strncmp(cmdcli,appmod_cli_menu_test.climenu[cpass].menu_cmd,strlen(appmod_cli_menu_test.climenu[cpass].menu_cmd))) {
                                                        continue;
                                                } else if (appmod_cli_user_auth < appmod_cli_menu_test.climenu[cpass].menu_perms) {
                                                        fnd = 1;
                                                        break;
                                                } else {
                                                        cli_func = appmod_cli_menu_test.climenu[cpass].menu_func;
                                                        appmod_cli_caller = (void *) appmod_xmenu[cli_func];
                                                        appmod_cli_caller();
                                                        if ((cli_func == APPMOD_CLI_FP_TEST) ||
                                                            (cli_func == APPMOD_CLI_FP_TEST) ||
                                                            (cli_func == APPMOD_CLI_FP_TEST)) {
                                                                fnd = 2;
                                                        } else {
                                                                fnd = 1;
                                                        }
                                                        break;
                                                }
                                        }
                                        if (fnd == 2) {
                                                newcmd = 0;
                                        } else if (fnd == 1) {
                                                appmod_po = 1;
                                                newcmd = 1;
                                        }
                                        cnt = 0;

				break;

				default:
					/* redisplay the menu */
					newcmd = 1;
					cnt = 0;
					fnd = 1;
					break;
			}

			if (fnd > 0) {
				fnd = 0;
				memset(cmdcli,0,sizeof(cmdcli));
			} else {
				/* special system commands go here */
				if ((strlen(cmdcli) > 1) && ((!strncmp(cmdcli,"/",1)) || (!strncmp(cmdcli,"\\",1)))) {
					/* FIXME: This could be done better -- longer strings at the top */
					if ((strstr(cmdcli,"/test") != NULL) || (strstr(cmdcli,"\\test") != NULL)) {
						appmod_cli_current_menu = APPMOD_CLI_FP_TEST;
					} else {
						appmod_cli_current_menu = APPMOD_CLI_FP_MAIN;
					}
					cnt = 0;
					newcmd = 1;
				} else if ((!strncmp(cmdcli,"/",1)) || (!strncmp(cmdcli,"\\",1))) {
					newcmd = 1;
					cnt = 0;
					appmod_cli_current_menu = APPMOD_CLI_FP_MAIN;
				} else if (!strncmp(cmdcli,"..",2)) {
					switch (appmod_cli_current_menu) {
						case APPMOD_CLI_FP_TEST:
							appmod_cli_current_menu = APPMOD_CLI_FP_MAIN;
							cnt = 0;
							newcmd = 1;
							break;

						case APPMOD_CLI_FP_MAIN:
						default:
							newcmd = 1;
							cnt = 0;
							break;
					}
				} else if (!strncmp(cmdcli,"exit",4)) {
					appmod_cli_current_menu = APPMOD_CLI_FP_MAIN;
					return;
				} else {
					newcmd = 1;
					cnt = 0;
				}
			}
		} else {
			cmdcli[cnt] = c;
			cnt++;
		}
	}
	return;
}
