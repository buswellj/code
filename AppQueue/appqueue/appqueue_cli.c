/*
 * AppQueue
 * Copyright (c) 2009 - 2010 Carbon Mountain LLC.
 * All Rights Reserved.
 *
 * John Buswell <buswellj@carbonmountain.com>
 * version 0.6.1
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
#include <termios.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/utsname.h>
#include <sys/statvfs.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <curl/curl.h>
#include <curl/types.h>
#include <curl/easy.h>
#include "kattach_types.h"
#include "kattach_shm.h"
#include "kattach_shm_globals.h"
#include "appqueue.h"

void
appqueue_cli_getuptime(void)
{
	FILE *stream;
	char uptime_str[32];

	stream = fopen(APPQUEUE_PROC_UPTIME,"r");
	if (stream == NULL)
		return;

	fgets(uptime_str,12,stream);	
	fclose(stream);

	appqueue_uptime = strtol(uptime_str,NULL,10);
	return;
}

void
appqueue_cli_getinfo(void)
{
	FILE *stream;
	char *buf;
	int j = 0;
	int i = 0;
	int x = 0;
	char c = '\0';
	char foo[1024];
	char flag[24];

	appqueue_proc = 0;

	stream = fopen(APPQUEUE_PROC_CPUINFO,"r");
	if (stream == NULL) 
		return;

	buf = (char *) malloc(sizeof(foo));

	while (!feof(stream)) {
		c = (char) fgetc(stream);
		if (c == '\n') {
			if (!strncmp(buf,"flags",5)) {
				x = 0;
				for (i = 12; i <= j; i++) {
					if (buf[i] == ' ') {
						flag[x] = '\0';
						if (!strncmp(flag,"vmx",3)) {
							appqueue_virt = 1;
						} else if (!strncmp(flag,"svm",3)) {
							appqueue_virt = 2;
						} else if (!strncmp(flag,"hypervisor",strlen("hypervisor"))) {
							appqueue_virt = 3;
						}
						x = 0;
					} else {
						flag[x] = buf[i];
						x++;
					}
				}
			} else if (!strncmp(buf,"processor",strlen("processor"))) {
				appqueue_proc++;
			} else if (!strncmp(buf,"model name",10)) {
				x = 0;
				for (i = 13; i <= j; i++) {
					appqueue_cpu[x] = buf[i];
					x++;
				}
			}
			memset(buf,0,strlen(foo));
			j = 0;
		} else {
			buf[j] = c;
			j++;
		}
	}
	fclose(stream);
	free(buf);
	return;
}

void
appqueue_cli_askq(char *askprompt, int do_echo, char *cmdline)
{
	char z;
	int x = 0, y = 0, w = 0;

	if (do_echo == 0x7) {
		w = 1;
		do_echo = 0;
	}

	printf("%s", askprompt);

	while (!x) {
		z = appqueue_cli_getch(do_echo);
		if (z == '\n') {
			cmdline[y] = '\0';
			return;
		} else {
			if ((do_echo == 0x6) || (w == 1)) {
				cmdline[y] = z;
			} else {
				cmdline[y] = tolower(z);
			}
			y++;
		}
	}
	cmdline[y] = '\0';

	return;
}

int
appqueue_cli_getch(int do_echo)
{
	struct termios savedt, newt;
	int ch;

	if (!do_echo) {
		tcgetattr(STDIN_FILENO, &savedt);
		newt = savedt;
		newt.c_lflag &= ~(ICANON|ECHO|IGNBRK);
		tcsetattr(STDIN_FILENO,TCSANOW, &newt);
	} else {
		tcgetattr(STDIN_FILENO, &savedt);
		newt = savedt;
		newt.c_lflag &= ~(IGNBRK);
		tcsetattr(STDIN_FILENO,TCSANOW, &newt);
	}
	
	ch = getchar();

/*
	if (do_echo) {
		if ((ch != 127) && (ch != 27)) {
			printf("%c",ch);
		}
	}
*/

	tcsetattr(STDIN_FILENO,TCSANOW, &savedt);

	return ch;
}

u8
appqueue_cli_auth(void)
{
	u8 nopass = 1, newat = 1, cnt = 0, pwdat = 0, authok = 0;
	char c = '\0';
	char cmdpass[64];
	char encpass[140];

	memset(cmdpass,0,sizeof(cmdpass));

	while (nopass) {
		if (cnt >= 64) newat = 1;
		if (newat) {
			if (pwdat >= 3) {
				printf("\nToo many password attempts. Goodbye!\n\n");
				nopass = 0;
				break;
			}
			printf("\nPassword: ");
			newat = 0;
			cnt = 0;
			pwdat++;
		}
		c = appqueue_cli_getch(0);

		if (c == '\n') {
			if (strlen(cmdpass) == 0) {
				cnt = 0;
				newat = 1;
				memset(cmdpass,0,sizeof(cmdpass));
				continue;
			}
			memset(encpass,0,strlen(encpass));
			sprintf(encpass,"%s",appqueue_cli_chkpass(cmdpass,kattach_cfg_shm->clipass));
			if (strlen(kattach_cfg_shm->clipass) == 0) {
				if ((strlen(kattach_cfg_shm->clipass) == 0) && ((!strncmp(cmdpass,"admin",5)) && (strlen(cmdpass) == 5))) {
					authok = 1;
					nopass = 0;
					appqueue_cli_user_auth = APPQUEUE_CLI_AUTH_ADMIN;
					memset(encpass,0,sizeof(cmdpass));
					memset(cmdpass,0,sizeof(cmdpass));
					break;
				} else {
					cnt = 0;
					newat = 1;
					memset(cmdpass,0,sizeof(cmdpass));
					continue;
				}
			} else if (!strncmp(encpass,kattach_cfg_shm->clipass,strlen(encpass)) && (strlen(encpass) == (strlen(kattach_cfg_shm->clipass)))) {
				authok = 1;
				nopass = 0;
				appqueue_cli_user_auth = APPQUEUE_CLI_AUTH_ADMIN;
				memset(encpass,0,sizeof(cmdpass));
				memset(cmdpass,0,sizeof(cmdpass));
				break;
			} else {
				cnt = 0;
				newat = 1;
				memset(cmdpass,0,sizeof(cmdpass));
				continue;
			}
		}

		cmdpass[cnt] = c;
		cnt++;
	}

	return(authok);
}
void
appqueue_cli(void)
{
	long unsigned int aq_upd = 0, aq_uph = 0, aq_upm = 0, aq_ups = 0, aq_diff = 0, aq_ut = 0;
	u8 authok = 0, authtry = 0;
	time_t aq_now;

	appqueue_po = 0;			/* display prompt only */
	appqueue_exit = 0;			/* exit cli */
	appqueue_virt = 0;			/* virtualization mode not known yet */

	if (getuid() != APPQUEUE_CLI_AUTH_UID) {
		printf("\n");
		printf("This user is not authorized to run this application.\n\n");
		return;
	}

	appqueue_shm_init();                            /* initialize shared memory */

	if (kattach_cfg_shm->hostname[0] != '\0') {
		sprintf(appqueue_prompt,"[%s | ", kattach_cfg_shm->hostname);
	} else {
		sprintf(appqueue_prompt,"[%s | ","kaos-new");
	}

	appqueue_cli_init();

	printf("\n");
	printf("AppQueue version %s\n",APPQUEUE_VERSION);
	printf("%s\n",APPQUEUE_COPYRIGHT);
	printf("%s\n\n",APPQUEUE_LICENSE);

	while (!appqueue_exit) {
		authok = appqueue_cli_auth();
		if (authok != 0) {
			authtry = 0;
			appqueue_cli_getinfo();
			time(&aq_now);
			printf("\n\nSystem Information @ %s\n",ctime(&aq_now));
			printf("%s\n",appqueue_cpu);
			printf("%u processor cores detected\n",appqueue_proc);
			if (appqueue_virt == 1) {
				printf("Intel(R) Hardware Virtualization detected.\n\n");
			} else if (appqueue_virt == 2) {
				printf("AMD Hardware Virtualization detected.\n\n");
			} else if (appqueue_virt == 3) {
				printf("WARNING: Unsupported Nested Hypervisor(s) detected.\n\n");
			} else {
				printf("WARNING: Hardware Virtualization NOT detected. Check BIOS.\n\n");
			}
			appqueue_cli_getuptime();
			aq_upd = (long unsigned int) appqueue_uptime / 86400;
			aq_diff = appqueue_uptime - (aq_upd * 86400);
			aq_uph = (long unsigned int) aq_diff / 3600;
			aq_ut = aq_diff - (aq_uph * 3600);
			aq_upm = (long unsigned int) aq_ut / 60;
			aq_ups = aq_ut - (aq_upm * 60);
			printf("Hypervisor is up %lu days, %lu hours, %lu minutes and %lu seconds\n\n",aq_upd,aq_uph,aq_upm,aq_ups);
			printf("Visit %s for further information.\n\n",APPQUEUE_LINK);
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAIN;
			appqueue_cli_loop();
			authok = 0;
		} else if (authtry > 2) {
			appqueue_shm_close();
			exit(0);
		} else {
			authtry++;
		}			
	}
	return;
}

void
appqueue_cli_loop(void)
{
	u8 aqli = 1, cnt = 0, newcmd = 0, cpass = 0, fnd = 0;
	u16 cli_func = 0;
	char c = '\0';
	char cmdcli[64];

	appqueue_cli_caller = (void *) appqueue_xmenu[appqueue_cli_current_menu];
	appqueue_cli_caller();

	while (aqli) {
		if (appqueue_exit) return;
		if (cnt >= 64) newcmd = 1;
		if (newcmd) {
			appqueue_cli_caller = (void *) appqueue_xmenu[appqueue_cli_current_menu];
			appqueue_cli_caller();
			newcmd = 0;
			cnt = 0;
			memset(cmdcli,0,sizeof(cmdcli));
			continue;
		}
		c = tolower(appqueue_cli_getch(1));
		if (c == '\n') {
			switch (appqueue_cli_current_menu) {
				case APPQUEUE_CLI_FP_MAIN:
					for (cpass = 0; cpass < appqueue_cli_menu_main.index; cpass++) {
						if (strlen(appqueue_cli_menu_main.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_main.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_main.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_main.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_main.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_INFO) ||
							    (cli_func == APPQUEUE_CLI_FP_APPS) ||
							    (cli_func == APPQUEUE_CLI_FP_VM) ||
							    (cli_func == APPQUEUE_CLI_FP_NET) ||
							    (cli_func == APPQUEUE_CLI_FP_STORAGE) ||
							    (cli_func == APPQUEUE_CLI_FP_SYS) ||
							    (cli_func == APPQUEUE_CLI_FP_MAINT) ||
							    (cli_func == APPQUEUE_CLI_FP_BOOT)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_INFO:
					for (cpass = 0; cpass < appqueue_cli_menu_info.index; cpass++) {
						if (strlen(appqueue_cli_menu_info.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_info.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_info.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_info.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_info.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_INFO) ||
								(cli_func == APPQUEUE_CLI_FP_INFO_L2) ||
								(cli_func == APPQUEUE_CLI_FP_INFO_L3) ||
								(cli_func == APPQUEUE_CLI_FP_INFO_L4) ||
								(cli_func == APPQUEUE_CLI_FP_INFO_VM)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_INFO_L2:
					for (cpass = 0; cpass < appqueue_cli_menu_info_l2.index; cpass++) {
						if (strlen(appqueue_cli_menu_info_l2.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_info_l2.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_info_l2.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_info_l2.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_info_l2.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_INFO) || (cli_func == APPQUEUE_CLI_FP_INFO_L2)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_INFO_L3:
					for (cpass = 0; cpass < appqueue_cli_menu_info_l3.index; cpass++) {
						if (strlen(appqueue_cli_menu_info_l3.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_info_l3.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_info_l3.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_info_l3.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_info_l3.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_INFO) || (cli_func == APPQUEUE_CLI_FP_INFO_L3)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_INFO_L4:
					for (cpass = 0; cpass < appqueue_cli_menu_info_l4.index; cpass++) {
						if (strlen(appqueue_cli_menu_info_l4.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_info_l4.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_info_l4.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_info_l4.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_info_l4.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_INFO) || (cli_func == APPQUEUE_CLI_FP_INFO_L4)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_INFO_VM:
					for (cpass = 0; cpass < appqueue_cli_menu_info_vm.index; cpass++) {
						if (strlen(appqueue_cli_menu_info_vm.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_info_vm.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_info_vm.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_info_vm.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_info_vm.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_INFO) || (cli_func == APPQUEUE_CLI_FP_INFO_VM)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;


				case APPQUEUE_CLI_FP_APPS:
					for (cpass = 0; cpass < appqueue_cli_menu_apps.index; cpass++) {
						if (strlen(appqueue_cli_menu_apps.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_apps.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_apps.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_apps.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_apps.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_APPS) || (cli_func == APPQUEUE_CLI_FP_APPS_CONFIG)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_APPS_CONFIG:
					for (cpass = 0; cpass < appqueue_cli_menu_apps_config.index; cpass++) {
						if (strlen(appqueue_cli_menu_apps_config.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_apps_config.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_apps_config.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_apps_config.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_apps_config.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_APPS) || (cli_func == APPQUEUE_CLI_FP_APPS_CONFIG)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_VM:
					for (cpass = 0; cpass < appqueue_cli_menu_vm.index; cpass++) {
						if (strlen(appqueue_cli_menu_vm.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_vm.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_vm.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_vm.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_vm.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_VM) ||
								(cli_func == APPQUEUE_CLI_FP_VM_VDI) ||
							 	(cli_func == APPQUEUE_CLI_FP_VM_VSRV)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_VM_VDI:
					for (cpass = 0; cpass < appqueue_cli_menu_vm_vdi.index; cpass++) {
						if (strlen(appqueue_cli_menu_vm_vdi.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_vm_vdi.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_vm_vdi.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_vm_vdi.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_vm_vdi.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_VM) ||
							 	(cli_func == APPQUEUE_CLI_FP_VM_VDI)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_VM_VSRV:
					for (cpass = 0; cpass < appqueue_cli_menu_vm_vsrv.index; cpass++) {
						if (strlen(appqueue_cli_menu_vm_vsrv.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_vm_vsrv.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_vm_vsrv.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_vm_vsrv.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_vm_vsrv.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_VM) ||
							 	(cli_func == APPQUEUE_CLI_FP_VM_VSRV)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;


				case APPQUEUE_CLI_FP_NET:
					for (cpass = 0; cpass < appqueue_cli_menu_net.index; cpass++) {
						if (strlen(appqueue_cli_menu_net.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_net.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_net.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_net.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_net.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_NET) || 
								(cli_func == APPQUEUE_CLI_FP_NET_VPORTS) ||
								(cli_func == APPQUEUE_CLI_FP_NET_VNS) ||
								(cli_func == APPQUEUE_CLI_FP_NET_FW) ||
								(cli_func == APPQUEUE_CLI_FP_NET_NETIF) ||
								(cli_func == APPQUEUE_CLI_FP_NET_VLAN)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_NET_NETIF:
					for (cpass = 0; cpass < appqueue_cli_menu_net_netif.index; cpass++) {
						if (strlen(appqueue_cli_menu_net_netif.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_net_netif.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_net_netif.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_net_netif.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_net_netif.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_NET) || (cli_func == APPQUEUE_CLI_FP_NET_NETIF)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_NET_VNS:
					for (cpass = 0; cpass < appqueue_cli_menu_net_vns.index; cpass++) {
						if (strlen(appqueue_cli_menu_net_vns.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_net_vns.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_net_vns.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_net_vns.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_net_vns.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_NET) || (cli_func == APPQUEUE_CLI_FP_NET_VNS)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_NET_VPORTS:
					for (cpass = 0; cpass < appqueue_cli_menu_net_vports.index; cpass++) {
						if (strlen(appqueue_cli_menu_net_vports.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_net_vports.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_net_vports.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_net_vports.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_net_vports.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_NET) || (cli_func == APPQUEUE_CLI_FP_NET_VPORTS)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_NET_FW:
					for (cpass = 0; cpass < appqueue_cli_menu_net_fw.index; cpass++) {
						if (strlen(appqueue_cli_menu_net_fw.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_net_fw.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_net_fw.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_net_fw.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_net_fw.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_NET) || 
								(cli_func == APPQUEUE_CLI_FP_NET_FW_FILTER) ||
								(cli_func == APPQUEUE_CLI_FP_NET_FW_MANGLE) ||
								(cli_func == APPQUEUE_CLI_FP_NET_FW_NAT) ||
								(cli_func == APPQUEUE_CLI_FP_NET_FW_ZONES) ||
								(cli_func == APPQUEUE_CLI_FP_NET_FW_APPS) ||
								(cli_func == APPQUEUE_CLI_FP_NET_FW)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_NET_FW_FILTER:
					for (cpass = 0; cpass < appqueue_cli_menu_net_fw_filter.index; cpass++) {
						if (strlen(appqueue_cli_menu_net_fw_filter.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_net_fw_filter.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_net_fw_filter.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_net_fw_filter.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_net_fw_filter.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_NET_FW) || (cli_func == APPQUEUE_CLI_FP_NET_FW_FILTER)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_NET_FW_MANGLE:
					for (cpass = 0; cpass < appqueue_cli_menu_net_fw_mangle.index; cpass++) {
						if (strlen(appqueue_cli_menu_net_fw_mangle.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_net_fw_mangle.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_net_fw_mangle.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_net_fw_mangle.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_net_fw_mangle.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_NET_FW) || (cli_func == APPQUEUE_CLI_FP_NET_FW_MANGLE)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_NET_FW_NAT:
					for (cpass = 0; cpass < appqueue_cli_menu_net_fw_nat.index; cpass++) {
						if (strlen(appqueue_cli_menu_net_fw_nat.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_net_fw_nat.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_net_fw_nat.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_net_fw_nat.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_net_fw_nat.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_NET_FW) || (cli_func == APPQUEUE_CLI_FP_NET_FW_NAT)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_NET_FW_ZONES:
					for (cpass = 0; cpass < appqueue_cli_menu_net_fw_zones.index; cpass++) {
						if (strlen(appqueue_cli_menu_net_fw_zones.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_net_fw_zones.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_net_fw_zones.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_net_fw_zones.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_net_fw_zones.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_NET_FW) || (cli_func == APPQUEUE_CLI_FP_NET_FW_ZONES)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_NET_FW_APPS:
					for (cpass = 0; cpass < appqueue_cli_menu_net_fw_apps.index; cpass++) {
						if (strlen(appqueue_cli_menu_net_fw_apps.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_net_fw_apps.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_net_fw_apps.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_net_fw_apps.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_net_fw_apps.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_NET_FW) || (cli_func == APPQUEUE_CLI_FP_NET_FW_APPS)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;


				case APPQUEUE_CLI_FP_NET_VLAN:
					for (cpass = 0; cpass < appqueue_cli_menu_net_vlan.index; cpass++) {
						if (strlen(appqueue_cli_menu_net_vlan.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_net_vlan.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_net_vlan.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_net_vlan.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_net_vlan.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_NET) || (cli_func == APPQUEUE_CLI_FP_NET_VLAN)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_STORAGE:
					for (cpass = 0; cpass < appqueue_cli_menu_storage.index; cpass++) {
						if (strlen(appqueue_cli_menu_storage.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_storage.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_storage.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_storage.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_storage.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_STORAGE) ||
								(cli_func == APPQUEUE_CLI_FP_STORAGE_LOCAL) ||
								(cli_func == APPQUEUE_CLI_FP_STORAGE_ISCSI) ||
								(cli_func == APPQUEUE_CLI_FP_STORAGE_RAID) ||
								(cli_func == APPQUEUE_CLI_FP_STORAGE_VSTORAGE) ||
								(cli_func == APPQUEUE_CLI_FP_STORAGE_VDISK) ||
								(cli_func == APPQUEUE_CLI_FP_STORAGE_VMEDIA) ||
								(cli_func == APPQUEUE_CLI_FP_STORAGE_GLUSTER)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_STORAGE_LOCAL:
					for (cpass = 0; cpass < appqueue_cli_menu_storage_local.index; cpass++) {
						if (strlen(appqueue_cli_menu_storage_local.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_storage_local.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_storage_local.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_storage_local.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_storage_local.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_STORAGE) ||
								(cli_func == APPQUEUE_CLI_FP_STORAGE_LOCAL)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_STORAGE_ISCSI:
					for (cpass = 0; cpass < appqueue_cli_menu_storage_iscsi.index; cpass++) {
						if (strlen(appqueue_cli_menu_storage_iscsi.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_storage_iscsi.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_storage_iscsi.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_storage_iscsi.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_storage_iscsi.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_STORAGE) ||
								(cli_func == APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS) ||
								(cli_func == APPQUEUE_CLI_FP_STORAGE_ISCSI)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS:
					for (cpass = 0; cpass < appqueue_cli_menu_storage_iscsi_isns.index; cpass++) {
						if (strlen(appqueue_cli_menu_storage_iscsi_isns.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_storage_iscsi_isns.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_storage_iscsi_isns.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_storage_iscsi_isns.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_storage_iscsi_isns.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_STORAGE_ISCSI) ||
								(cli_func == APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;


				case APPQUEUE_CLI_FP_STORAGE_GLUSTER:
					for (cpass = 0; cpass < appqueue_cli_menu_storage_gluster.index; cpass++) {
						if (strlen(appqueue_cli_menu_storage_gluster.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_storage_gluster.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_storage_gluster.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_storage_gluster.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_storage_gluster.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_STORAGE) ||
								(cli_func == APPQUEUE_CLI_FP_STORAGE_GLUSTER)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_STORAGE_RAID:
					for (cpass = 0; cpass < appqueue_cli_menu_storage_raid.index; cpass++) {
						if (strlen(appqueue_cli_menu_storage_raid.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_storage_raid.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_storage_raid.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_storage_raid.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_storage_raid.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_STORAGE) ||
								(cli_func == APPQUEUE_CLI_FP_STORAGE_RAID)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_STORAGE_VSTORAGE:
					for (cpass = 0; cpass < appqueue_cli_menu_storage_vstorage.index; cpass++) {
						if (strlen(appqueue_cli_menu_storage_vstorage.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_storage_vstorage.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_storage_vstorage.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_storage_vstorage.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_storage_vstorage.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_STORAGE) ||
								(cli_func == APPQUEUE_CLI_FP_STORAGE_VSTORAGE)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_STORAGE_VDISK:
					for (cpass = 0; cpass < appqueue_cli_menu_storage_vdisk.index; cpass++) {
						if (strlen(appqueue_cli_menu_storage_vdisk.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_storage_vdisk.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_storage_vdisk.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_storage_vdisk.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_storage_vdisk.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_STORAGE) ||
								(cli_func == APPQUEUE_CLI_FP_STORAGE_VDISK)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_STORAGE_VMEDIA:
					for (cpass = 0; cpass < appqueue_cli_menu_storage_vmedia.index; cpass++) {
						if (strlen(appqueue_cli_menu_storage_vmedia.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_storage_vmedia.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_storage_vmedia.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_storage_vmedia.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_storage_vmedia.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_STORAGE) ||
								(cli_func == APPQUEUE_CLI_FP_STORAGE_VMEDIA)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_SYS:
					for (cpass = 0; cpass < appqueue_cli_menu_sys.index; cpass++) {
						if (strlen(appqueue_cli_menu_sys.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_sys.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_sys.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_sys.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_sys.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_SYS) ||
								(cli_func == APPQUEUE_CLI_FP_SYS_CLOCK) ||
								(cli_func == APPQUEUE_CLI_FP_SYS_DISK) ||
								(cli_func == APPQUEUE_CLI_FP_SYS_AUTH)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_SYS_CLOCK:
					for (cpass = 0; cpass < appqueue_cli_menu_sys_clock.index; cpass++) {
						if (strlen(appqueue_cli_menu_sys_clock.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_sys_clock.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_sys_clock.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_sys_clock.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_sys_clock.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_SYS) || (cli_func == APPQUEUE_CLI_FP_SYS_CLOCK)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_SYS_AUTH:
					for (cpass = 0; cpass < appqueue_cli_menu_sys_auth.index; cpass++) {
						if (strlen(appqueue_cli_menu_sys_auth.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_sys_auth.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_sys_auth.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_sys_auth.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_sys_auth.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_SYS) || (cli_func == APPQUEUE_CLI_FP_SYS_AUTH)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_SYS_DISK:
					for (cpass = 0; cpass < appqueue_cli_menu_sys_disk.index; cpass++) {
						if (strlen(appqueue_cli_menu_sys_disk.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_sys_disk.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_sys_disk.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_sys_disk.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_sys_disk.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if ((cli_func == APPQUEUE_CLI_FP_SYS) || (cli_func == APPQUEUE_CLI_FP_SYS_DISK)) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_MAINT:
					for (cpass = 0; cpass < appqueue_cli_menu_maint.index; cpass++) {
						if (strlen(appqueue_cli_menu_maint.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_maint.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_maint.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_maint.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else if ((appqueue_cli_user_auth < APPQUEUE_CLI_AUTH_DEV) &&
							((!strncmp(appqueue_cli_menu_maint.climenu[cpass].menu_cmd,"dbappq",strlen(appqueue_cli_menu_maint.climenu[cpass].menu_cmd))) ||
							(!strncmp(appqueue_cli_menu_maint.climenu[cpass].menu_cmd,"dbkaos",strlen(appqueue_cli_menu_maint.climenu[cpass].menu_cmd))) ||
							(!strncmp(appqueue_cli_menu_maint.climenu[cpass].menu_cmd,"dbvmsess",strlen(appqueue_cli_menu_maint.climenu[cpass].menu_cmd))))) {
								continue;
						} else {
							cli_func = appqueue_cli_menu_maint.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if (cli_func == APPQUEUE_CLI_FP_MAINT) {
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
						appqueue_po = 1;
						newcmd = 1;
					}
					cnt = 0;
					break;

				case APPQUEUE_CLI_FP_BOOT:
					for (cpass = 0; cpass < appqueue_cli_menu_boot.index; cpass++) {
						if (strlen(appqueue_cli_menu_boot.climenu[cpass].menu_cmd) != strlen(cmdcli)) continue;
						if (strncmp(cmdcli,appqueue_cli_menu_boot.climenu[cpass].menu_cmd,strlen(appqueue_cli_menu_boot.climenu[cpass].menu_cmd))) {
							continue;
						} else if (appqueue_cli_user_auth < appqueue_cli_menu_boot.climenu[cpass].menu_perms) {
							fnd = 1;
							break;
						} else {
							cli_func = appqueue_cli_menu_boot.climenu[cpass].menu_func;
							appqueue_cli_caller = (void *) appqueue_xmenu[cli_func];
							appqueue_cli_caller();
							if (cli_func == APPQUEUE_CLI_FP_BOOT) {
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
						appqueue_po = 1;
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
					if ((strstr(cmdcli,"/info/vm") != NULL) || (strstr(cmdcli,"\\info\\vm") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_VM;
					} else if ((strstr(cmdcli,"/info/l4") != NULL) || (strstr(cmdcli,"\\info\\l4") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_L4;
					} else if ((strstr(cmdcli,"/info/l3") != NULL) || (strstr(cmdcli,"\\info\\l3") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_L3;
					} else if ((strstr(cmdcli,"/info/l2") != NULL) || (strstr(cmdcli,"\\info\\l2") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_L2;
					} else if ((strstr(cmdcli,"/apps/config") != NULL) || (strstr(cmdcli,"\\apps\\config") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS_CONFIG;
					} else if ((strstr(cmdcli,"/vm/vserver") != NULL) || (strstr(cmdcli,"\\vm\\vserver") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM_VSRV;
					} else if ((strstr(cmdcli,"/vm/vdi") != NULL) || (strstr(cmdcli,"\\vm\\vdi") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM_VDI;
					} else if ((strstr(cmdcli,"/net/fw/filter") != NULL) || (strstr(cmdcli,"\\net\\fw\\filter") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
					} else if ((strstr(cmdcli,"/net/fw/mangle") != NULL) || (strstr(cmdcli,"\\net\\fw\\mangle") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
					} else if ((strstr(cmdcli,"/net/fw/zones") != NULL) || (strstr(cmdcli,"\\net\\fw\\zones") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_ZONES;
					} else if ((strstr(cmdcli,"/net/fw/apps") != NULL) || (strstr(cmdcli,"\\net\\fw\\apps") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
					} else if ((strstr(cmdcli,"/net/fw/nat") != NULL) || (strstr(cmdcli,"\\net\\fw\\nat") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
					} else if ((strstr(cmdcli,"/net/fw") != NULL) || (strstr(cmdcli,"\\net\\fw") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW;
					} else if ((strstr(cmdcli,"/net/vlan") != NULL) || (strstr(cmdcli,"\\net\\vlan") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VLAN;
					} else if ((strstr(cmdcli,"/net/vports") != NULL) || (strstr(cmdcli,"\\net\\vports") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VPORTS;
					} else if ((strstr(cmdcli,"/net/netif") != NULL) || (strstr(cmdcli,"\\net\\netif") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_NETIF;
					} else if ((strstr(cmdcli,"/net/vns") != NULL) || (strstr(cmdcli,"\\net\\vns") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
					} else if ((strstr(cmdcli,"/storage/iscsi/isns") != NULL) || (strstr(cmdcli,"\\storage\\iscsi\\isns") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS;
					} else if ((strstr(cmdcli,"/storage/vstorage") != NULL) || (strstr(cmdcli,"\\storage\\vstorage") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VSTORAGE;
					} else if ((strstr(cmdcli,"/storage/gluster") != NULL) || (strstr(cmdcli,"\\storage\\gluster") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_GLUSTER;
					} else if ((strstr(cmdcli,"/storage/vmedia") != NULL) || (strstr(cmdcli,"\\storage\\vmedia") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VMEDIA;
					} else if ((strstr(cmdcli,"/storage/local") != NULL) || (strstr(cmdcli,"\\storage\\local") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_LOCAL;
					} else if ((strstr(cmdcli,"/storage/raid") != NULL) || (strstr(cmdcli,"\\storage\\raid") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_RAID;
					} else if ((strstr(cmdcli,"/storage/vdisk") != NULL) || (strstr(cmdcli,"\\storage\\vdisk") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_VDISK;
					} else if ((strstr(cmdcli,"/storage/iscsi") != NULL) || (strstr(cmdcli,"\\storage\\iscsi") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_ISCSI;
					} else if ((strstr(cmdcli,"/sys/clock") != NULL) || (strstr(cmdcli,"\\sys\\clock") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_CLOCK;
					} else if ((strstr(cmdcli,"/sys/auth") != NULL) || (strstr(cmdcli,"\\sys\\auth") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_AUTH;
					} else if ((strstr(cmdcli,"/sys/disk") != NULL) || (strstr(cmdcli,"\\sys\\disk") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_DISK;
					} else if ((strstr(cmdcli,"/info") != NULL) || (strstr(cmdcli,"\\info") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO;
					} else if ((strstr(cmdcli,"/apps") != NULL) || (strstr(cmdcli,"\\apps") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS;
					} else if ((strstr(cmdcli,"/vm") != NULL) || (strstr(cmdcli,"\\vm") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
					} else if ((strstr(cmdcli,"/net") != NULL) || (strstr(cmdcli,"\\net") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET;
					} else if ((strstr(cmdcli,"/sys") != NULL) || (strstr(cmdcli,"\\sys") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS;
					} else if ((strstr(cmdcli,"/maint") != NULL) || (strstr(cmdcli,"\\maint") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAINT;
					} else if ((strstr(cmdcli,"/boot") != NULL) || (strstr(cmdcli,"\\boot") != NULL)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_BOOT;
					} else {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAIN;
					}
					cnt = 0;
					newcmd = 1;
				} else if ((!strncmp(cmdcli,"/",1)) || (!strncmp(cmdcli,"\\",1))) {
					newcmd = 1;
					cnt = 0;
					appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAIN;
				} else if (!strncmp(cmdcli,"..",2)) {
					switch (appqueue_cli_current_menu) {
						case APPQUEUE_CLI_FP_BOOT:
						case APPQUEUE_CLI_FP_MAINT:
						case APPQUEUE_CLI_FP_SYS:
						case APPQUEUE_CLI_FP_NET:
						case APPQUEUE_CLI_FP_VM:
						case APPQUEUE_CLI_FP_APPS:
						case APPQUEUE_CLI_FP_INFO:
						case APPQUEUE_CLI_FP_STORAGE:
							appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAIN;
							cnt = 0;
							newcmd = 1;
							break;

						case APPQUEUE_CLI_FP_NET_FW_ZONES:
						case APPQUEUE_CLI_FP_NET_FW_APPS:
						case APPQUEUE_CLI_FP_NET_FW_NAT:
						case APPQUEUE_CLI_FP_NET_FW_FILTER:
						case APPQUEUE_CLI_FP_NET_FW_MANGLE:
							appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW;
							cnt = 0;
							newcmd = 1;
							break;

						case APPQUEUE_CLI_FP_NET_NETIF:
						case APPQUEUE_CLI_FP_NET_VNS:
						case APPQUEUE_CLI_FP_NET_FW:
						case APPQUEUE_CLI_FP_NET_VLAN:
						case APPQUEUE_CLI_FP_NET_VPORTS:
							appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET;
							cnt = 0;
							newcmd = 1;
							break;

						case APPQUEUE_CLI_FP_INFO_L2:
						case APPQUEUE_CLI_FP_INFO_L3:
						case APPQUEUE_CLI_FP_INFO_L4:
						case APPQUEUE_CLI_FP_INFO_VM:
							appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO;
							cnt = 0;
							newcmd = 1;
							break;

						case APPQUEUE_CLI_FP_APPS_CONFIG:
							appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS;
							cnt = 0;
							newcmd = 1;
							break;

						case APPQUEUE_CLI_FP_VM_VDI:
						case APPQUEUE_CLI_FP_VM_VSRV:
							appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
							cnt = 0;
							newcmd = 1;
							break;

						case APPQUEUE_CLI_FP_SYS_CLOCK:
						case APPQUEUE_CLI_FP_SYS_AUTH:
						case APPQUEUE_CLI_FP_SYS_DISK:
							appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS;
							cnt = 0;
							newcmd = 1;
							break;

						case APPQUEUE_CLI_FP_STORAGE_LOCAL:
						case APPQUEUE_CLI_FP_STORAGE_ISCSI:
						case APPQUEUE_CLI_FP_STORAGE_GLUSTER:
						case APPQUEUE_CLI_FP_STORAGE_RAID:
						case APPQUEUE_CLI_FP_STORAGE_VDISK:
						case APPQUEUE_CLI_FP_STORAGE_VSTORAGE:
						case APPQUEUE_CLI_FP_STORAGE_VMEDIA:
							appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE;
							cnt = 0;
							newcmd = 1;
							break;


						case APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS:
							appqueue_cli_current_menu = APPQUEUE_CLI_FP_STORAGE_ISCSI;
							cnt = 0;
							newcmd = 1;
							break;

						case APPQUEUE_CLI_FP_MAIN:
						default:
							newcmd = 1;
							cnt = 0;
							break;

					}
				} else if (!strncmp(cmdcli,"exit",4)) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAIN;
						return;
				} else {
					newcmd = 1;
					cnt = 0;
				}
			}
		} else if (c == 27) { 				/* this is an escape sequence */
		} else if (c == 127) {				/* this is a backspace sequence */
			/*
			if (cnt == 0) continue;
			cnt -= 1;
			cmdcli[cnt] = '\0';
			if (appqueue_cli_current_menu == APPQUEUE_CLI_FP_MAIN) {
				printf("\n%sHypervisor]# %s",appqueue_prompt,cmdcli);
			}
			*/
		} else {					/* FIXME: we should check for arrow keys */
			cmdcli[cnt] = c;
			cnt++;
		}
	}

}

void
appqueue_cli_init_main(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_main.index;

	sprintf(appqueue_cli_menu_main.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_main.climenu[index].menu_desc,"%s",cli_menu_desc);
	if ((cli_menu_func == APPQUEUE_CLI_FP_INFO) || (cli_menu_func == APPQUEUE_CLI_FP_EXIT)) {
		appqueue_cli_menu_main.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_USER;
	} else {
		appqueue_cli_menu_main.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	}
	appqueue_cli_menu_main.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_main.index = index;
	return;
}

void
appqueue_cli_init_info(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_info.index;

	sprintf(appqueue_cli_menu_info.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_info.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_info.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_info.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_info.index = index;
	return;
}

void
appqueue_cli_init_info_l2(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_info_l2.index;

	sprintf(appqueue_cli_menu_info_l2.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_info_l2.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_info_l2.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_USER;
	appqueue_cli_menu_info_l2.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_info_l2.index = index;
	return;
}

void
appqueue_cli_init_info_l3(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_info_l3.index;

	sprintf(appqueue_cli_menu_info_l3.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_info_l3.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_info_l3.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_USER;
	appqueue_cli_menu_info_l3.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_info_l3.index = index;
	return;
}

void
appqueue_cli_init_info_l4(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_info_l4.index;

	sprintf(appqueue_cli_menu_info_l4.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_info_l4.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_info_l4.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_USER;
	appqueue_cli_menu_info_l4.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_info_l4.index = index;
	return;
}

void
appqueue_cli_init_info_vm(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_info_vm.index;

	sprintf(appqueue_cli_menu_info_vm.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_info_vm.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_info_vm.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_USER;
	appqueue_cli_menu_info_vm.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_info_vm.index = index;
	return;
}

void
appqueue_cli_init_apps(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_apps.index;

	sprintf(appqueue_cli_menu_apps.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_apps.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_apps.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_apps.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_apps.index = index;
	return;
}

void
appqueue_cli_init_apps_config(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_apps_config.index;

	sprintf(appqueue_cli_menu_apps_config.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_apps_config.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_apps_config.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_apps_config.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_apps_config.index = index;
	return;
}

void
appqueue_cli_init_vm(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_vm.index;

	sprintf(appqueue_cli_menu_vm.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_vm.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_vm.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_vm.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_vm.index = index;
	return;
}

void
appqueue_cli_init_vm_vdi(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_vm_vdi.index;

	sprintf(appqueue_cli_menu_vm_vdi.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_vm_vdi.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_vm_vdi.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_vm_vdi.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_vm_vdi.index = index;
	return;
}

void
appqueue_cli_init_vm_vsrv(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_vm_vsrv.index;

	sprintf(appqueue_cli_menu_vm_vsrv.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_vm_vsrv.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_vm_vsrv.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_vm_vsrv.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_vm_vsrv.index = index;
	return;
}

void
appqueue_cli_init_net(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_net.index;

	sprintf(appqueue_cli_menu_net.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_net.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_net.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_net.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_net.index = index;
	return;
}

void
appqueue_cli_init_net_netif(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_net_netif.index;

	sprintf(appqueue_cli_menu_net_netif.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_net_netif.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_net_netif.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_net_netif.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_net_netif.index = index;
	return;
}

void
appqueue_cli_init_net_vns(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_net_vns.index;

	sprintf(appqueue_cli_menu_net_vns.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_net_vns.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_net_vns.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_net_vns.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_net_vns.index = index;
	return;
}

void
appqueue_cli_init_net_fw(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_net_fw.index;

	sprintf(appqueue_cli_menu_net_fw.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_net_fw.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_net_fw.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_net_fw.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_net_fw.index = index;
	return;
}

void
appqueue_cli_init_net_fw_zones(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_net_fw_zones.index;

	sprintf(appqueue_cli_menu_net_fw_zones.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_net_fw_zones.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_net_fw_zones.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_net_fw_zones.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_net_fw_zones.index = index;
	return;
}

void
appqueue_cli_init_net_fw_apps(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_net_fw_apps.index;

	sprintf(appqueue_cli_menu_net_fw_apps.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_net_fw_apps.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_net_fw_apps.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_net_fw_apps.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_net_fw_apps.index = index;
	return;
}

void
appqueue_cli_init_net_fw_filter(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_net_fw_filter.index;

	sprintf(appqueue_cli_menu_net_fw_filter.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_net_fw_filter.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_net_fw_filter.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_net_fw_filter.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_net_fw_filter.index = index;
	return;
}

void
appqueue_cli_init_net_fw_nat(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_net_fw_nat.index;

	sprintf(appqueue_cli_menu_net_fw_nat.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_net_fw_nat.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_net_fw_nat.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_net_fw_nat.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_net_fw_nat.index = index;
	return;
}

void
appqueue_cli_init_net_fw_mangle(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_net_fw_mangle.index;

	sprintf(appqueue_cli_menu_net_fw_mangle.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_net_fw_mangle.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_net_fw_mangle.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_net_fw_mangle.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_net_fw_mangle.index = index;
	return;
}



void
appqueue_cli_init_net_vlan(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_net_vlan.index;

	sprintf(appqueue_cli_menu_net_vlan.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_net_vlan.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_net_vlan.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_net_vlan.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_net_vlan.index = index;
	return;
}

void
appqueue_cli_init_net_vports(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_net_vports.index;

	sprintf(appqueue_cli_menu_net_vports.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_net_vports.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_net_vports.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_net_vports.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_net_vports.index = index;
	return;
}

void
appqueue_cli_init_storage(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_storage.index;

	sprintf(appqueue_cli_menu_storage.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_storage.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_storage.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_storage.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_storage.index = index;
	return;
}

void
appqueue_cli_init_storage_local(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_storage_local.index;

	sprintf(appqueue_cli_menu_storage_local.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_storage_local.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_storage_local.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_storage_local.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_storage_local.index = index;
	return;
}

void
appqueue_cli_init_storage_iscsi(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_storage_iscsi.index;

	sprintf(appqueue_cli_menu_storage_iscsi.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_storage_iscsi.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_storage_iscsi.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_storage_iscsi.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_storage_iscsi.index = index;
	return;
}

void
appqueue_cli_init_storage_iscsi_isns(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_storage_iscsi_isns.index;

	sprintf(appqueue_cli_menu_storage_iscsi_isns.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_storage_iscsi_isns.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_storage_iscsi_isns.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_storage_iscsi_isns.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_storage_iscsi_isns.index = index;
	return;
}

void
appqueue_cli_init_storage_gluster(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_storage_gluster.index;

	sprintf(appqueue_cli_menu_storage_gluster.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_storage_gluster.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_storage_gluster.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_storage_gluster.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_storage_gluster.index = index;
	return;
}

void
appqueue_cli_init_storage_raid(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_storage_raid.index;

	sprintf(appqueue_cli_menu_storage_raid.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_storage_raid.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_storage_raid.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_storage_raid.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_storage_raid.index = index;
	return;
}

void
appqueue_cli_init_storage_vstorage(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_storage_vstorage.index;

	sprintf(appqueue_cli_menu_storage_vstorage.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_storage_vstorage.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_storage_vstorage.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_storage_vstorage.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_storage_vstorage.index = index;
	return;
}

void
appqueue_cli_init_storage_vdisk(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_storage_vdisk.index;

	sprintf(appqueue_cli_menu_storage_vdisk.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_storage_vdisk.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_storage_vdisk.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_storage_vdisk.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_storage_vdisk.index = index;
	return;
}

void
appqueue_cli_init_storage_vmedia(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_storage_vmedia.index;

	sprintf(appqueue_cli_menu_storage_vmedia.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_storage_vmedia.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_storage_vmedia.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_storage_vmedia.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_storage_vmedia.index = index;
	return;
}


void
appqueue_cli_init_sys(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_sys.index;

	sprintf(appqueue_cli_menu_sys.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_sys.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_sys.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_sys.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_sys.index = index;
	return;
}

void
appqueue_cli_init_sys_clock(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_sys_clock.index;

	sprintf(appqueue_cli_menu_sys_clock.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_sys_clock.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_sys_clock.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_sys_clock.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_sys_clock.index = index;
	return;
}

void
appqueue_cli_init_sys_auth(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_sys_auth.index;

	sprintf(appqueue_cli_menu_sys_auth.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_sys_auth.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_sys_auth.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_sys_auth.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_sys_auth.index = index;
	return;
}

void
appqueue_cli_init_sys_disk(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_sys_disk.index;

	sprintf(appqueue_cli_menu_sys_disk.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_sys_disk.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_sys_disk.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_sys_disk.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_sys_disk.index = index;
	return;
}

void
appqueue_cli_init_maint(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_maint.index;

	sprintf(appqueue_cli_menu_maint.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_maint.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_maint.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_maint.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_maint.index = index;
	return;
}

void
appqueue_cli_init_boot(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func)
{
	u8 index = appqueue_cli_menu_boot.index;

	sprintf(appqueue_cli_menu_boot.climenu[index].menu_cmd,"%s",cli_menu_cmd);
	sprintf(appqueue_cli_menu_boot.climenu[index].menu_desc,"%s",cli_menu_desc);
	appqueue_cli_menu_boot.climenu[index].menu_perms = APPQUEUE_CLI_AUTH_ADMIN;
	appqueue_cli_menu_boot.climenu[index].menu_func = cli_menu_func;

	index++;
	appqueue_cli_menu_boot.index = index;
	return;
}

void
appqueue_cli_init(void)
{
	appqueue_cli_menu_t *mPtr;

	mPtr = &appqueue_cli_menu_main;
	memset(mPtr,0,sizeof(appqueue_cli_menu_main));
	mPtr = &appqueue_cli_menu_info;
	memset(mPtr,0,sizeof(appqueue_cli_menu_info));
	mPtr = &appqueue_cli_menu_info_l2;
	memset(mPtr,0,sizeof(appqueue_cli_menu_info_l2));
	mPtr = &appqueue_cli_menu_info_l3;
	memset(mPtr,0,sizeof(appqueue_cli_menu_info_l3));
	mPtr = &appqueue_cli_menu_info_l4;
	memset(mPtr,0,sizeof(appqueue_cli_menu_info_l4));
	mPtr = &appqueue_cli_menu_info_vm;
	memset(mPtr,0,sizeof(appqueue_cli_menu_info_vm));
	mPtr = &appqueue_cli_menu_apps;
	memset(mPtr,0,sizeof(appqueue_cli_menu_apps));
	mPtr = &appqueue_cli_menu_apps_config;
	memset(mPtr,0,sizeof(appqueue_cli_menu_apps_config));
	mPtr = &appqueue_cli_menu_vm;
	memset(mPtr,0,sizeof(appqueue_cli_menu_vm));
	mPtr = &appqueue_cli_menu_vm_vdi;
	memset(mPtr,0,sizeof(appqueue_cli_menu_vm_vdi));
	mPtr = &appqueue_cli_menu_vm_vsrv;
	memset(mPtr,0,sizeof(appqueue_cli_menu_vm_vsrv));
	mPtr = &appqueue_cli_menu_net;
	memset(mPtr,0,sizeof(appqueue_cli_menu_net));
	mPtr = &appqueue_cli_menu_net_vlan;
	memset(mPtr,0,sizeof(appqueue_cli_menu_net_vlan));
	mPtr = &appqueue_cli_menu_net_netif;
	memset(mPtr,0,sizeof(appqueue_cli_menu_net_netif));
	mPtr = &appqueue_cli_menu_net_vns;
	memset(mPtr,0,sizeof(appqueue_cli_menu_net_vns));
	mPtr = &appqueue_cli_menu_net_fw;
	memset(mPtr,0,sizeof(appqueue_cli_menu_net_fw));
	mPtr = &appqueue_cli_menu_net_fw_zones;
	memset(mPtr,0,sizeof(appqueue_cli_menu_net_fw_zones));
	mPtr = &appqueue_cli_menu_net_fw_apps;
	memset(mPtr,0,sizeof(appqueue_cli_menu_net_fw_apps));
	mPtr = &appqueue_cli_menu_net_fw_mangle;
	memset(mPtr,0,sizeof(appqueue_cli_menu_net_fw_mangle));
	mPtr = &appqueue_cli_menu_net_fw_filter;
	memset(mPtr,0,sizeof(appqueue_cli_menu_net_fw_filter));
	mPtr = &appqueue_cli_menu_net_fw_nat;
	memset(mPtr,0,sizeof(appqueue_cli_menu_net_fw_nat));
	mPtr = &appqueue_cli_menu_net_vports;
	memset(mPtr,0,sizeof(appqueue_cli_menu_net_vports));
	mPtr = &appqueue_cli_menu_storage;
	memset(mPtr,0,sizeof(appqueue_cli_menu_storage));
	mPtr = &appqueue_cli_menu_storage_local;
	memset(mPtr,0,sizeof(appqueue_cli_menu_storage_local));
	mPtr = &appqueue_cli_menu_storage_iscsi;
	memset(mPtr,0,sizeof(appqueue_cli_menu_storage_iscsi));
	mPtr = &appqueue_cli_menu_storage_iscsi_isns;
	memset(mPtr,0,sizeof(appqueue_cli_menu_storage_iscsi_isns));
	mPtr = &appqueue_cli_menu_storage_gluster;
	memset(mPtr,0,sizeof(appqueue_cli_menu_storage_gluster));
	mPtr = &appqueue_cli_menu_storage_raid;
	memset(mPtr,0,sizeof(appqueue_cli_menu_storage_raid));
	mPtr = &appqueue_cli_menu_storage_vstorage;
	memset(mPtr,0,sizeof(appqueue_cli_menu_storage_vstorage));
	mPtr = &appqueue_cli_menu_storage_vdisk;
	memset(mPtr,0,sizeof(appqueue_cli_menu_storage_vdisk));
	mPtr = &appqueue_cli_menu_storage_vmedia;
	memset(mPtr,0,sizeof(appqueue_cli_menu_storage_vmedia));
	mPtr = &appqueue_cli_menu_sys;
	memset(mPtr,0,sizeof(appqueue_cli_menu_sys));
	mPtr = &appqueue_cli_menu_sys_clock;
	memset(mPtr,0,sizeof(appqueue_cli_menu_sys_clock));
	mPtr = &appqueue_cli_menu_sys_auth;
	memset(mPtr,0,sizeof(appqueue_cli_menu_sys_auth));
	mPtr = &appqueue_cli_menu_sys_disk;
	memset(mPtr,0,sizeof(appqueue_cli_menu_sys_disk));
	mPtr = &appqueue_cli_menu_maint;
	memset(mPtr,0,sizeof(appqueue_cli_menu_maint));
	mPtr = &appqueue_cli_menu_boot;
	memset(mPtr,0,sizeof(appqueue_cli_menu_boot));

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAIN;					/* initialize to the main menu */
	appqueue_xmenu[APPQUEUE_CLI_FP_MAIN] = appqueue_cli_mf_main;				/* main menu is a special case */

	/* set menu titles */
	sprintf(appqueue_cli_menu_main.cli_menu_title,"[Hypervisor Menu]\n");
	sprintf(appqueue_cli_menu_info.cli_menu_title,"[Information Menu]\n");
	sprintf(appqueue_cli_menu_info_l2.cli_menu_title,"[Layer 2 Info Menu]\n");
	sprintf(appqueue_cli_menu_info_l3.cli_menu_title,"[Layer 3 Info Menu]\n");
	sprintf(appqueue_cli_menu_info_l4.cli_menu_title,"[Layer 4 Info Menu]\n");
	sprintf(appqueue_cli_menu_info_vm.cli_menu_title,"[VM Info Menu]\n");
	sprintf(appqueue_cli_menu_apps.cli_menu_title,"[App Module Menu]\n");
	sprintf(appqueue_cli_menu_apps_config.cli_menu_title,"[App Configuration Menu]\n");
	sprintf(appqueue_cli_menu_vm.cli_menu_title,"[Virtual Machine Menu]\n");
	sprintf(appqueue_cli_menu_vm_vdi.cli_menu_title,"[VDI Menu]\n");
	sprintf(appqueue_cli_menu_vm_vsrv.cli_menu_title,"[Virtual Server Menu]\n");
	sprintf(appqueue_cli_menu_net.cli_menu_title,"[Networking Menu]\n");
	sprintf(appqueue_cli_menu_net_vlan.cli_menu_title,"[VLAN Menu]\n");
	sprintf(appqueue_cli_menu_net_vports.cli_menu_title,"[Virtual Ports Menu]\n");
	sprintf(appqueue_cli_menu_net_netif.cli_menu_title,"[Network Interface Menu]\n");
	sprintf(appqueue_cli_menu_net_vns.cli_menu_title,"[Virtual Network Services Menu]\n");
	sprintf(appqueue_cli_menu_net_fw.cli_menu_title,"[Firewall Menu]\n");
	sprintf(appqueue_cli_menu_net_fw_zones.cli_menu_title,"[Firewall Zones Menu]\n");
	sprintf(appqueue_cli_menu_net_fw_apps.cli_menu_title,"[Firewall Apps Menu]\n");
	sprintf(appqueue_cli_menu_net_fw_filter.cli_menu_title,"[Packet Filtering Menu]\n");
	sprintf(appqueue_cli_menu_net_fw_mangle.cli_menu_title,"[Packing Mangling Menu]\n");
	sprintf(appqueue_cli_menu_net_fw_nat.cli_menu_title,"[Network Address Translation Menu]\n");
	sprintf(appqueue_cli_menu_storage.cli_menu_title,"[Storage Menu]\n");
	sprintf(appqueue_cli_menu_storage_local.cli_menu_title,"[Local Storage Menu]\n");
	sprintf(appqueue_cli_menu_storage_iscsi.cli_menu_title,"[iSCSI Menu]\n");
	sprintf(appqueue_cli_menu_storage_iscsi_isns.cli_menu_title,"[iSCSI Name Service Menu]\n");
	sprintf(appqueue_cli_menu_storage_gluster.cli_menu_title,"[GlusterFS Menu]\n");
	sprintf(appqueue_cli_menu_storage_vstorage.cli_menu_title,"[Virtual Storage Menu]\n");
	sprintf(appqueue_cli_menu_storage_vdisk.cli_menu_title,"[Virtual Disk Menu]\n");
	sprintf(appqueue_cli_menu_storage_vmedia.cli_menu_title,"[Virtual Media Menu]\n");
	sprintf(appqueue_cli_menu_sys.cli_menu_title,"[System Menu]\n");
	sprintf(appqueue_cli_menu_sys_clock.cli_menu_title,"[System Clock Menu]\n");
	sprintf(appqueue_cli_menu_sys_auth.cli_menu_title,"[System Authentication Menu]\n");
	sprintf(appqueue_cli_menu_sys_disk.cli_menu_title,"[Local Storage Menu]\n");
	sprintf(appqueue_cli_menu_maint.cli_menu_title,"[Maintenance Menu]\n");
	sprintf(appqueue_cli_menu_boot.cli_menu_title,"[Boot Menu]\n");


	/* main menu */
	appqueue_cli_init_main("info","Information Menu", APPQUEUE_CLI_FP_INFO);
	appqueue_xmenu[APPQUEUE_CLI_FP_INFO] = appqueue_cli_mf_info;

	appqueue_cli_init_main("apps","App Module Menu", APPQUEUE_CLI_FP_APPS);
	appqueue_xmenu[APPQUEUE_CLI_FP_APPS] = appqueue_cli_mf_apps;

	appqueue_cli_init_main("vm","Virtual Machine Menu", APPQUEUE_CLI_FP_VM);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM] = appqueue_cli_mf_vm;

	appqueue_cli_init_main("net","Networking Menu", APPQUEUE_CLI_FP_NET);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET] = appqueue_cli_mf_net;

	appqueue_cli_init_main("storage","Storage Menu", APPQUEUE_CLI_FP_STORAGE);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE] = appqueue_cli_mf_storage;

	appqueue_cli_init_main("sys","System Menu", APPQUEUE_CLI_FP_SYS);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS] = appqueue_cli_mf_sys;

	appqueue_cli_init_main("setup","System Setup", APPQUEUE_CLI_FP_SETUP);
	appqueue_xmenu[APPQUEUE_CLI_FP_SETUP] = appqueue_cli_mf_setup;

	appqueue_cli_init_main("maint","Maintenance Menu", APPQUEUE_CLI_FP_MAINT);
	appqueue_xmenu[APPQUEUE_CLI_FP_MAINT] = appqueue_cli_mf_maint;

	appqueue_cli_init_main("boot","Boot Menu", APPQUEUE_CLI_FP_BOOT);
	appqueue_xmenu[APPQUEUE_CLI_FP_BOOT] = appqueue_cli_mf_boot;

	appqueue_cli_init_main("exit","Exit", APPQUEUE_CLI_FP_EXIT);
	appqueue_xmenu[APPQUEUE_CLI_FP_EXIT] = appqueue_cli_mf_exit;

	/* info menu */
	appqueue_cli_init_info("sys","System Information", APPQUEUE_CLI_FP_INFO_SYS);
	appqueue_xmenu[APPQUEUE_CLI_FP_INFO_SYS] = appqueue_cli_mf_info_sys;

	appqueue_cli_init_info("link","Physical Port Status", APPQUEUE_CLI_FP_INFO_LINK);
	appqueue_xmenu[APPQUEUE_CLI_FP_INFO_LINK] = appqueue_cli_mf_info_link;

	appqueue_cli_init_info("l2","Layer 2 Network Information", APPQUEUE_CLI_FP_INFO_L2);
	appqueue_xmenu[APPQUEUE_CLI_FP_INFO_L2] = appqueue_cli_mf_info_l2;

	appqueue_cli_init_info("l3","Layer 3 Network Information", APPQUEUE_CLI_FP_INFO_L3);
	appqueue_xmenu[APPQUEUE_CLI_FP_INFO_L3] = appqueue_cli_mf_info_l3;

	appqueue_cli_init_info("l4","Layer 4 Network Information", APPQUEUE_CLI_FP_INFO_L4);
	appqueue_xmenu[APPQUEUE_CLI_FP_INFO_L4] = appqueue_cli_mf_info_l4;

	appqueue_cli_init_info("vm","VM Information", APPQUEUE_CLI_FP_INFO_VM);
	appqueue_xmenu[APPQUEUE_CLI_FP_INFO_VM] = appqueue_cli_mf_info_vm;

	/* info:l2 menu */
	appqueue_cli_init_info_l2("vlan","VLAN Information", APPQUEUE_CLI_FP_INFO_L2_VLAN);
	appqueue_xmenu[APPQUEUE_CLI_FP_INFO_L2_VLAN] = appqueue_cli_mf_info_l2_vlan;

	appqueue_cli_init_info_l2("vbridge","Virtual Bridge Information", APPQUEUE_CLI_FP_INFO_L2_VBRIDGE);
	appqueue_xmenu[APPQUEUE_CLI_FP_INFO_L2_VBRIDGE] = appqueue_cli_mf_info_l2_vbridge;

	appqueue_cli_init_info_l2("vmac","Virtual MAC Information", APPQUEUE_CLI_FP_INFO_L2_VMAC);
	appqueue_xmenu[APPQUEUE_CLI_FP_INFO_L2_VMAC] = appqueue_cli_mf_info_l2_vmac;

	appqueue_cli_init_info_l2("vmports","Virtual Port Information", APPQUEUE_CLI_FP_INFO_L2_VMPORTS);
	appqueue_xmenu[APPQUEUE_CLI_FP_INFO_L2_VMPORTS] = appqueue_cli_mf_info_l2_vmports;

	/* info:l3 menu */
	appqueue_cli_init_info_l3("ip","IP Information", APPQUEUE_CLI_FP_INFO_L3_IP);
	appqueue_xmenu[APPQUEUE_CLI_FP_INFO_L3_IP] = appqueue_cli_mf_info_l3_ip;

	appqueue_cli_init_info_l3("route","IP Route Information", APPQUEUE_CLI_FP_INFO_L3_ROUTE);
	appqueue_xmenu[APPQUEUE_CLI_FP_INFO_L3_ROUTE] = appqueue_cli_mf_info_l3_route;

	appqueue_cli_init_info_l3("arp","Hypervisor ARP Information", APPQUEUE_CLI_FP_INFO_L3_ARP);
	appqueue_xmenu[APPQUEUE_CLI_FP_INFO_L3_ARP] = appqueue_cli_mf_info_l3_arp;

	/* info:l4 menu */
	appqueue_cli_init_info_l4("fw","Firewall Information", APPQUEUE_CLI_FP_INFO_L4_FW);
	appqueue_xmenu[APPQUEUE_CLI_FP_INFO_L4_FW] = appqueue_cli_mf_info_l4_fw;

	appqueue_cli_init_info_l4("vsip","Virtual Service Information", APPQUEUE_CLI_FP_INFO_L4_VSIP);
	appqueue_xmenu[APPQUEUE_CLI_FP_INFO_L4_VSIP] = appqueue_cli_mf_info_l4_vsip;

	/* info:vm menu */
	appqueue_cli_init_info_vm("fw","Firewall Information", APPQUEUE_CLI_FP_INFO_VM_FW);
	appqueue_xmenu[APPQUEUE_CLI_FP_INFO_VM_FW] = appqueue_cli_mf_info_vm_fw;

	appqueue_cli_init_info_vm("priority","VM Priority Information", APPQUEUE_CLI_FP_INFO_VM_PRIORITY);
	appqueue_xmenu[APPQUEUE_CLI_FP_INFO_VM_PRIORITY] = appqueue_cli_mf_info_vm_priority;

	/* apps menu */
	appqueue_cli_init_apps("list","Display installed App Modules", APPQUEUE_CLI_FP_APPS_LIST);
	appqueue_xmenu[APPQUEUE_CLI_FP_APPS_LIST] = appqueue_cli_mf_apps_list;

	appqueue_cli_init_apps("avail","Display available App Modules", APPQUEUE_CLI_FP_APPS_AVAIL);
	appqueue_xmenu[APPQUEUE_CLI_FP_APPS_AVAIL] = appqueue_cli_mf_apps_avail;

	appqueue_cli_init_apps("install","Install an App Module", APPQUEUE_CLI_FP_APPS_INSTALL);
	appqueue_xmenu[APPQUEUE_CLI_FP_APPS_INSTALL] = appqueue_cli_mf_apps_install;

	appqueue_cli_init_apps("update","Update App Module(s)", APPQUEUE_CLI_FP_APPS_UPDATE);
	appqueue_xmenu[APPQUEUE_CLI_FP_APPS_UPDATE] = appqueue_cli_mf_apps_update;

	appqueue_cli_init_apps("import","Import a local App Module", APPQUEUE_CLI_FP_APPS_IMPORT);
	appqueue_xmenu[APPQUEUE_CLI_FP_APPS_IMPORT] = appqueue_cli_mf_apps_import;

	appqueue_cli_init_apps("remove","Remove an App Modules", APPQUEUE_CLI_FP_APPS_REMOVE);
	appqueue_xmenu[APPQUEUE_CLI_FP_APPS_REMOVE] = appqueue_cli_mf_apps_remove;

	appqueue_cli_init_apps("config","Configure App Modules", APPQUEUE_CLI_FP_APPS_CONFIG);
	appqueue_xmenu[APPQUEUE_CLI_FP_APPS_CONFIG] = appqueue_cli_mf_apps_config;

	appqueue_cli_init_apps("showvm","Show VM App Images", APPQUEUE_CLI_FP_APPS_SHOWVM);
	appqueue_xmenu[APPQUEUE_CLI_FP_APPS_SHOWVM] = appqueue_cli_mf_apps_showvm;

	appqueue_cli_init_apps("createvm","Create a VM App Image", APPQUEUE_CLI_FP_APPS_CREATEVM);
	appqueue_xmenu[APPQUEUE_CLI_FP_APPS_CREATEVM] = appqueue_cli_mf_apps_createvm;

	appqueue_cli_init_apps("deletevm","Delete a VM App Image", APPQUEUE_CLI_FP_APPS_DELETEVM);
	appqueue_xmenu[APPQUEUE_CLI_FP_APPS_DELETEVM] = appqueue_cli_mf_apps_deletevm;

	/* apps:config menu */

	appqueue_cli_init_apps_config("list","List Configuration Groups", APPQUEUE_CLI_FP_APPS_CONFIG_LIST);
	appqueue_xmenu[APPQUEUE_CLI_FP_APPS_CONFIG_LIST] = appqueue_cli_mf_apps_config_list;

	appqueue_cli_init_apps_config("create","Create a Configuration Group", APPQUEUE_CLI_FP_APPS_CONFIG_CREATE);
	appqueue_xmenu[APPQUEUE_CLI_FP_APPS_CONFIG_CREATE] = appqueue_cli_mf_apps_config_create;

	appqueue_cli_init_apps_config("remove","Remove a Configuration Group", APPQUEUE_CLI_FP_APPS_CONFIG_REMOVE);
	appqueue_xmenu[APPQUEUE_CLI_FP_APPS_CONFIG_REMOVE] = appqueue_cli_mf_apps_config_remove;

	appqueue_cli_init_apps_config("app","App Configuration", APPQUEUE_CLI_FP_APPS_CONFIG_APP);
	appqueue_xmenu[APPQUEUE_CLI_FP_APPS_CONFIG_APP] = appqueue_cli_mf_apps_config_app;


	/* vm menu */
	appqueue_cli_init_vm("list","Display Virtual Machines", APPQUEUE_CLI_FP_VM_LIST);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_LIST] = appqueue_cli_mf_vm_list;
	
	appqueue_cli_init_vm("deploy","Deploy Virtual Machine(s)", APPQUEUE_CLI_FP_VM_DEPLOY);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_DEPLOY] = appqueue_cli_mf_vm_deploy;
	
	appqueue_cli_init_vm("start","Start a Virtual Machine", APPQUEUE_CLI_FP_VM_START);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_START] = appqueue_cli_mf_vm_start;
	
	appqueue_cli_init_vm("stop","Stop a Virtual Machine", APPQUEUE_CLI_FP_VM_STOP);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_STOP] = appqueue_cli_mf_vm_stop;
	
	appqueue_cli_init_vm("restart","Restart a Virtual Machine", APPQUEUE_CLI_FP_VM_RESTART);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_RESTART] = appqueue_cli_mf_vm_restart;
	
	appqueue_cli_init_vm("killall","Stop Multiple Virtual Machines", APPQUEUE_CLI_FP_VM_KILLALL);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_KILLALL] = appqueue_cli_mf_vm_killall;

	appqueue_cli_init_vm("startall","Start Multiple Virtual Machines", APPQUEUE_CLI_FP_VM_STARTALL);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_STARTALL] = appqueue_cli_mf_vm_startall;
	
	appqueue_cli_init_vm("remove","Remove a Virtual Machine", APPQUEUE_CLI_FP_VM_REMOVE);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_REMOVE] = appqueue_cli_mf_vm_remove;

	appqueue_cli_init_vm("import","Import a Virtual Machine", APPQUEUE_CLI_FP_VM_IMPORT);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_IMPORT] = appqueue_cli_mf_vm_import;

	appqueue_cli_init_vm("priority","Virtual Machine Priority", APPQUEUE_CLI_FP_VM_PRIORITY);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_PRIORITY] = appqueue_cli_mf_vm_priority;

	appqueue_cli_init_vm("vdi","Virtual Desktop Infrastructure", APPQUEUE_CLI_FP_VM_VDI);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_VDI] = appqueue_cli_mf_vm_vdi;

	appqueue_cli_init_vm("vserver","Virtual Server Menu", APPQUEUE_CLI_FP_VM_VSRV);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_VSRV] = appqueue_cli_mf_vm_vsrv;

	/* vm: vdi menu */
	appqueue_cli_init_vm_vdi("list","List VDI Information", APPQUEUE_CLI_FP_VM_VDI_LIST);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_VDI_LIST] = appqueue_cli_mf_vm_vdi_list;

	appqueue_cli_init_vm_vdi("install","Install VDI from Media", APPQUEUE_CLI_FP_VM_VDI_INSTALL);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_VDI_INSTALL] = appqueue_cli_mf_vm_vdi_install;

	appqueue_cli_init_vm_vdi("deploy","Deploy installed VDI", APPQUEUE_CLI_FP_VM_VDI_DEPLOY);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_VDI_DEPLOY] = appqueue_cli_mf_vm_vdi_deploy;

	appqueue_cli_init_vm_vdi("remove","Remove VDI Image", APPQUEUE_CLI_FP_VM_VDI_REMOVE);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_VDI_REMOVE] = appqueue_cli_mf_vm_vdi_remove;


	/* vm: vsrv menu */
	appqueue_cli_init_vm_vsrv("list","List Virtual Servers", APPQUEUE_CLI_FP_VM_VSRV_LIST);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_VSRV_LIST] = appqueue_cli_mf_vm_vsrv_list;

	appqueue_cli_init_vm_vsrv("install","Install Virtual Server from Media", APPQUEUE_CLI_FP_VM_VSRV_INSTALL);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_VSRV_INSTALL] = appqueue_cli_mf_vm_vsrv_install;

	appqueue_cli_init_vm_vsrv("deploy","Deploy installed Virtual Server", APPQUEUE_CLI_FP_VM_VSRV_DEPLOY);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_VSRV_DEPLOY] = appqueue_cli_mf_vm_vsrv_deploy;

	appqueue_cli_init_vm_vsrv("remove","Remove Virtual Server Image", APPQUEUE_CLI_FP_VM_VSRV_REMOVE);
	appqueue_xmenu[APPQUEUE_CLI_FP_VM_VSRV_REMOVE] = appqueue_cli_mf_vm_vsrv_remove;

	/* net menu */	
	appqueue_cli_init_net("vlan","Manage VLANs", APPQUEUE_CLI_FP_NET_VLAN);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_VLAN] = appqueue_cli_mf_net_vlan;

	appqueue_cli_init_net("dhcp","Display Hypervisor DHCP Information", APPQUEUE_CLI_FP_NET_DHCP);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_DHCP] = appqueue_cli_mf_net_dhcp;

	appqueue_cli_init_net("netif","Manage Network Interfaces", APPQUEUE_CLI_FP_NET_NETIF);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_NETIF] = appqueue_cli_mf_net_netif;

	appqueue_cli_init_net("fw","Manage Firewall", APPQUEUE_CLI_FP_NET_FW);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW] = appqueue_cli_mf_net_fw;

	appqueue_cli_init_net("vns","Manage Virtual Network Services", APPQUEUE_CLI_FP_NET_VNS);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_VNS] = appqueue_cli_mf_net_vns;

	appqueue_cli_init_net("vports","Manage Virtual Ports", APPQUEUE_CLI_FP_NET_VPORTS);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_VPORTS] = appqueue_cli_mf_net_vports;

	/* net netif menu */
	appqueue_cli_init_net_netif("addif","Add Interface", APPQUEUE_CLI_FP_NET_NETIF_ADDIF);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_NETIF_ADDIF] = appqueue_cli_mf_net_netif_addif;

	appqueue_cli_init_net_netif("editif","Edit Interface", APPQUEUE_CLI_FP_NET_NETIF_EDITIF);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_NETIF_EDITIF] = appqueue_cli_mf_net_netif_editif;

	appqueue_cli_init_net_netif("rmif","Remove Interface", APPQUEUE_CLI_FP_NET_NETIF_RMIF);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_NETIF_RMIF] = appqueue_cli_mf_net_netif_rmif;

	appqueue_cli_init_net_netif("ifmgr","Manage Interfaces", APPQUEUE_CLI_FP_NET_NETIF_IFMGR);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_NETIF_IFMGR] = appqueue_cli_mf_net_netif_ifmgr;


	/* net vlan menu */
	appqueue_cli_init_net_vlan("add","Create a new VLAN", APPQUEUE_CLI_FP_NET_VLAN_ADD);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_VLAN_ADD] = appqueue_cli_mf_net_vlan_add;

	appqueue_cli_init_net_vlan("edit","Edit a VLAN", APPQUEUE_CLI_FP_NET_VLAN_EDIT);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_VLAN_EDIT] = appqueue_cli_mf_net_vlan_edit;

	appqueue_cli_init_net_vlan("delete","Delete a VLAN", APPQUEUE_CLI_FP_NET_VLAN_DELETE);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_VLAN_DELETE] = appqueue_cli_mf_net_vlan_delete;

	appqueue_cli_init_net_vlan("list","List VLANs", APPQUEUE_CLI_FP_NET_VLAN_LIST);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_VLAN_LIST] = appqueue_cli_mf_net_vlan_list;

	/* net vns menu */
	appqueue_cli_init_net_vns("addvsip","Add Virtual Service IP", APPQUEUE_CLI_FP_NET_VNS_ADDVSIP);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_VNS_ADDVSIP] = appqueue_cli_mf_net_vns_addvsip;

	appqueue_cli_init_net_vns("rmvsip","Remove Virtual Service IP", APPQUEUE_CLI_FP_NET_VNS_RMVSIP);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_VNS_RMVSIP] = appqueue_cli_mf_net_vns_rmvsip;

	appqueue_cli_init_net_vns("addvsp","Add Virtual Service Port", APPQUEUE_CLI_FP_NET_VNS_ADDVSP);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_VNS_ADDVSP] = appqueue_cli_mf_net_vns_addvsp;

	appqueue_cli_init_net_vns("rmvsp","Remove Virtual Service Port", APPQUEUE_CLI_FP_NET_VNS_RMVSP);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_VNS_RMVSP] = appqueue_cli_mf_net_vns_rmvsp;

	appqueue_cli_init_net_vns("rlimit","Rate Limiting", APPQUEUE_CLI_FP_NET_VNS_RATELIMIT);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_VNS_RATELIMIT] = appqueue_cli_mf_net_vns_ratelimit;

	appqueue_cli_init_net_vns("state","Stateful Processing", APPQUEUE_CLI_FP_NET_VNS_STATE);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_VNS_STATE] = appqueue_cli_mf_net_vns_state;


	/* net fw menu */
	appqueue_cli_init_net_fw("zones","Firewall Zones", APPQUEUE_CLI_FP_NET_FW_ZONES);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_ZONES] = appqueue_cli_mf_net_fw_zones;

	appqueue_cli_init_net_fw("apps","Firewall Applications", APPQUEUE_CLI_FP_NET_FW_APPS);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_APPS] = appqueue_cli_mf_net_fw_apps;

	appqueue_cli_init_net_fw("filter","Packet Filtering", APPQUEUE_CLI_FP_NET_FW_FILTER);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_FILTER] = appqueue_cli_mf_net_fw_filter;

	appqueue_cli_init_net_fw("mangle","Packet Mangling", APPQUEUE_CLI_FP_NET_FW_MANGLE);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_MANGLE] = appqueue_cli_mf_net_fw_mangle;

	appqueue_cli_init_net_fw("nat","Network Address Translation", APPQUEUE_CLI_FP_NET_FW_NAT);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_NAT] = appqueue_cli_mf_net_fw_nat;

	/* net fw zones */
	appqueue_cli_init_net_fw_zones("list","List Firewall Zones", APPQUEUE_CLI_FP_NET_FW_ZONES_LIST);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_ZONES_LIST] = appqueue_cli_mf_net_fw_zones_list;

	appqueue_cli_init_net_fw_zones("create","Create a new Zone", APPQUEUE_CLI_FP_NET_FW_ZONES_CREATE);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_ZONES_CREATE] = appqueue_cli_mf_net_fw_zones_create;

	appqueue_cli_init_net_fw_zones("edit","Edit an existing Zone", APPQUEUE_CLI_FP_NET_FW_ZONES_EDIT);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_ZONES_EDIT] = appqueue_cli_mf_net_fw_zones_edit;

	appqueue_cli_init_net_fw_zones("delete","Delete an existing Zone", APPQUEUE_CLI_FP_NET_FW_ZONES_DELETE);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_ZONES_DELETE] = appqueue_cli_mf_net_fw_zones_delete;

	/* net fw apps */
	appqueue_cli_init_net_fw_apps("list","List Application Profiles", APPQUEUE_CLI_FP_NET_FW_APPS_LIST);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_APPS_LIST] = appqueue_cli_mf_net_fw_apps_list;

	appqueue_cli_init_net_fw_apps("create","Create a new App Profile", APPQUEUE_CLI_FP_NET_FW_APPS_CREATE);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_APPS_CREATE] = appqueue_cli_mf_net_fw_apps_create;

	appqueue_cli_init_net_fw_apps("edit","Edit an existing App Profile", APPQUEUE_CLI_FP_NET_FW_APPS_EDIT);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_APPS_EDIT] = appqueue_cli_mf_net_fw_apps_edit;

	appqueue_cli_init_net_fw_apps("delete","Delete an App Profile", APPQUEUE_CLI_FP_NET_FW_APPS_DELETE);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_APPS_DELETE] = appqueue_cli_mf_net_fw_apps_delete;

	/* net fw filter menu */
	appqueue_cli_init_net_fw_filter("list","Display Packet Filters", APPQUEUE_CLI_FP_NET_FW_FILTER_LIST);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_FILTER_LIST] = appqueue_cli_mf_net_fw_filter_list;

	appqueue_cli_init_net_fw_filter("add","Add a Packet Filter", APPQUEUE_CLI_FP_NET_FW_FILTER_ADD);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_FILTER_ADD] = appqueue_cli_mf_net_fw_filter_add;

	appqueue_cli_init_net_fw_filter("del","Delete a Packet Filter", APPQUEUE_CLI_FP_NET_FW_FILTER_DEL);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_FILTER_DEL] = appqueue_cli_mf_net_fw_filter_del;

	appqueue_cli_init_net_fw_filter("edit","Edit Packet Filter(s)", APPQUEUE_CLI_FP_NET_FW_FILTER_EDIT);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_FILTER_EDIT] = appqueue_cli_mf_net_fw_filter_edit;

	appqueue_cli_init_net_fw_filter("reverse","Reverse Filter Logic", APPQUEUE_CLI_FP_NET_FW_FILTER_REVERSE);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_FILTER_REVERSE] = appqueue_cli_mf_net_fw_filter_reverse;

	appqueue_cli_init_net_fw_filter("log","Toggle Logging on a Filter", APPQUEUE_CLI_FP_NET_FW_FILTER_LOG);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_FILTER_LOG] = appqueue_cli_mf_net_fw_filter_log;

	/* net fw mangle menu */
	appqueue_cli_init_net_fw_mangle("list","Display Mangling Rules", APPQUEUE_CLI_FP_NET_FW_MANGLE_LIST);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_MANGLE_LIST] = appqueue_cli_mf_net_fw_mangle_list;

	appqueue_cli_init_net_fw_mangle("add","Add a Packet Rule", APPQUEUE_CLI_FP_NET_FW_MANGLE_ADD);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_MANGLE_ADD] = appqueue_cli_mf_net_fw_mangle_add;

	appqueue_cli_init_net_fw_mangle("del","Delete a Packet Rule", APPQUEUE_CLI_FP_NET_FW_MANGLE_DEL);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_MANGLE_DEL] = appqueue_cli_mf_net_fw_mangle_del;

	appqueue_cli_init_net_fw_mangle("edit","Edit Packet Rule(s)", APPQUEUE_CLI_FP_NET_FW_MANGLE_EDIT);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_MANGLE_EDIT] = appqueue_cli_mf_net_fw_mangle_edit;

	appqueue_cli_init_net_fw_mangle("reverse","Reverse Rule Logic", APPQUEUE_CLI_FP_NET_FW_MANGLE_REVERSE);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_MANGLE_REVERSE] = appqueue_cli_mf_net_fw_mangle_reverse;

	appqueue_cli_init_net_fw_mangle("log","Toggle Logging on a Rule", APPQUEUE_CLI_FP_NET_FW_MANGLE_LOG);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_MANGLE_LOG] = appqueue_cli_mf_net_fw_mangle_log;

	/* net fw nat menu */
	appqueue_cli_init_net_fw_nat("list","Display NAT Rules", APPQUEUE_CLI_FP_NET_FW_NAT_LIST);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_NAT_LIST] = appqueue_cli_mf_net_fw_nat_list;

	appqueue_cli_init_net_fw_nat("add","Add a NAT Rule", APPQUEUE_CLI_FP_NET_FW_NAT_ADD);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_NAT_ADD] = appqueue_cli_mf_net_fw_nat_add;

	appqueue_cli_init_net_fw_nat("del","Delete a NAT Rule", APPQUEUE_CLI_FP_NET_FW_NAT_DEL);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_NAT_DEL] = appqueue_cli_mf_net_fw_nat_del;

	appqueue_cli_init_net_fw_nat("edit","Edit NAT Rule(s)", APPQUEUE_CLI_FP_NET_FW_NAT_EDIT);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_NAT_EDIT] = appqueue_cli_mf_net_fw_nat_edit;

	appqueue_cli_init_net_fw_nat("reverse","Reverse Rule Logic", APPQUEUE_CLI_FP_NET_FW_NAT_REVERSE);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_NAT_REVERSE] = appqueue_cli_mf_net_fw_nat_reverse;

	appqueue_cli_init_net_fw_nat("log","Toggle Logging on a Rule", APPQUEUE_CLI_FP_NET_FW_NAT_LOG);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_FW_NAT_LOG] = appqueue_cli_mf_net_fw_nat_log;

	/* net: vports menu */
	appqueue_cli_init_net_vports("list","List Virtual Ports", APPQUEUE_CLI_FP_NET_VPORTS_LIST);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_VPORTS_LIST] = appqueue_cli_mf_net_vports_list;

	appqueue_cli_init_net_vports("add","Create a new Virtual Port", APPQUEUE_CLI_FP_NET_VPORTS_ADD);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_VPORTS_ADD] = appqueue_cli_mf_net_vports_add;

	appqueue_cli_init_net_vports("del","Delete a Virtual Port", APPQUEUE_CLI_FP_NET_VPORTS_DEL);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_VPORTS_DEL] = appqueue_cli_mf_net_vports_del;

	appqueue_cli_init_net_vports("assign","Assign a Virtual Port", APPQUEUE_CLI_FP_NET_VPORTS_ASSIGN);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_VPORTS_ASSIGN] = appqueue_cli_mf_net_vports_assign;

	appqueue_cli_init_net_vports("unassign","Unassign a Virtual Port", APPQUEUE_CLI_FP_NET_VPORTS_UNASSIGN);
	appqueue_xmenu[APPQUEUE_CLI_FP_NET_VPORTS_UNASSIGN] = appqueue_cli_mf_net_vports_unassign;

	/* sys menu */
	appqueue_cli_init_sys("hostname","Set Hypervisor hostname", APPQUEUE_CLI_FP_SYS_HOSTNAME);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_HOSTNAME] = appqueue_cli_mf_sys_hostname;

	appqueue_cli_init_sys("domain","Set Hypervisor domain", APPQUEUE_CLI_FP_SYS_DOMAIN);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_DOMAIN] = appqueue_cli_mf_sys_domain;

	appqueue_cli_init_sys("dns","Set DNS servers", APPQUEUE_CLI_FP_SYS_DNS);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_DNS] = appqueue_cli_mf_sys_dns;

	appqueue_cli_init_sys("disk","Hypervisor Local Storage", APPQUEUE_CLI_FP_SYS_DISK);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_DISK] = appqueue_cli_mf_sys_disk;

	appqueue_cli_init_sys("clock","Hypervisor Clock Settings", APPQUEUE_CLI_FP_SYS_CLOCK);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_CLOCK] = appqueue_cli_mf_sys_clock;

	appqueue_cli_init_sys("auth","Hypervisor Authentication Settings", APPQUEUE_CLI_FP_SYS_AUTH);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_AUTH] = appqueue_cli_mf_sys_auth;

	/* storage menu */
	appqueue_cli_init_storage("local","Manage Local Storage", APPQUEUE_CLI_FP_STORAGE_LOCAL);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_LOCAL] = appqueue_cli_mf_storage_local;

	appqueue_cli_init_storage("iscsi","Manage iSCSI Storage", APPQUEUE_CLI_FP_STORAGE_ISCSI);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_ISCSI] = appqueue_cli_mf_storage_iscsi;

	appqueue_cli_init_storage("gluster","Manage GlusterFS Storage", APPQUEUE_CLI_FP_STORAGE_GLUSTER);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_GLUSTER] = appqueue_cli_mf_storage_gluster;

	appqueue_cli_init_storage("raid","Manage Software RAID", APPQUEUE_CLI_FP_STORAGE_RAID);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_RAID] = appqueue_cli_mf_storage_raid;

	appqueue_cli_init_storage("vstorage","Virtual Storage Management", APPQUEUE_CLI_FP_STORAGE_VSTORAGE);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VSTORAGE] = appqueue_cli_mf_storage_vstorage;

	appqueue_cli_init_storage("vdisk","Virtual Disk Management", APPQUEUE_CLI_FP_STORAGE_VDISK);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VDISK] = appqueue_cli_mf_storage_vdisk;

	appqueue_cli_init_storage("vmedia","Manage Virtual Installation Media", APPQUEUE_CLI_FP_STORAGE_VMEDIA);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VMEDIA] = appqueue_cli_mf_storage_vmedia;

	/* storage: local menu */
	appqueue_cli_init_storage_local("list","List available disks", APPQUEUE_CLI_FP_STORAGE_LOCAL_LIST);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_LOCAL_LIST] = appqueue_cli_mf_storage_local_list;

	/* storage: iscsi menu */
	appqueue_cli_init_storage_iscsi("enable","Enable iSCSI initiator", APPQUEUE_CLI_FP_STORAGE_ISCSI_ENABLE);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_ISCSI_ENABLE] = appqueue_cli_mf_storage_iscsi_enable;

	appqueue_cli_init_storage_iscsi("isns","iSCSI name service menu", APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS] = appqueue_cli_mf_storage_iscsi_isns;

	appqueue_cli_init_storage_iscsi("showtgt","List configured iSCSI targets", APPQUEUE_CLI_FP_STORAGE_ISCSI_SHOWTGT);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_ISCSI_SHOWTGT] = appqueue_cli_mf_storage_iscsi_showtgt;

	appqueue_cli_init_storage_iscsi("discover","Discover storage devices", APPQUEUE_CLI_FP_STORAGE_ISCSI_DISCOVERY);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_ISCSI_DISCOVERY] = appqueue_cli_mf_storage_iscsi_discovery;

	appqueue_cli_init_storage_iscsi("isnsdisc","iSNS Discovery", APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNSDISC);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNSDISC] = appqueue_cli_mf_storage_iscsi_isnsdisc;

	appqueue_cli_init_storage_iscsi("addtgt","Add iSCSI target", APPQUEUE_CLI_FP_STORAGE_ISCSI_ADDTGT);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_ISCSI_ADDTGT] = appqueue_cli_mf_storage_iscsi_addtgt;

	appqueue_cli_init_storage_iscsi("deltgt","Remove iSCSI target", APPQUEUE_CLI_FP_STORAGE_ISCSI_DELTGT);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_ISCSI_DELTGT] = appqueue_cli_mf_storage_iscsi_deltgt;

	/* storage: iscsi: isns menu */
	appqueue_cli_init_storage_iscsi_isns("list","List iSNS services", APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS_LIST);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS_LIST] = appqueue_cli_mf_storage_iscsi_isns_list;

	appqueue_cli_init_storage_iscsi_isns("add","Add iSNS server", APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS_ADD);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS_ADD] = appqueue_cli_mf_storage_iscsi_isns_add;

	appqueue_cli_init_storage_iscsi_isns("edit","Edit iSNS server", APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS_EDIT);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS_EDIT] = appqueue_cli_mf_storage_iscsi_isns_edit;

	appqueue_cli_init_storage_iscsi_isns("del","Delete iSNS server", APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS_DEL);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_ISCSI_ISNS_DEL] = appqueue_cli_mf_storage_iscsi_isns_del;

	/* storage: gluster menu */
	appqueue_cli_init_storage_gluster("list","List GlusterFS Servers", APPQUEUE_CLI_FP_STORAGE_GLUSTER_LIST);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_GLUSTER_LIST] = appqueue_cli_mf_storage_gluster_list;

	appqueue_cli_init_storage_gluster("add","Add GlusterFS Server", APPQUEUE_CLI_FP_STORAGE_GLUSTER_ADD);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_GLUSTER_ADD] = appqueue_cli_mf_storage_gluster_add;

	appqueue_cli_init_storage_gluster("edit","Edit GlusterFS Servers", APPQUEUE_CLI_FP_STORAGE_GLUSTER_EDIT);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_GLUSTER_EDIT] = appqueue_cli_mf_storage_gluster_edit;

	appqueue_cli_init_storage_gluster("del","Delete GlusterFS Server", APPQUEUE_CLI_FP_STORAGE_GLUSTER_DEL);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_GLUSTER_DEL] = appqueue_cli_mf_storage_gluster_del;


	/* storage: raid menu */
	appqueue_cli_init_storage_raid("list","List RAID devices", APPQUEUE_CLI_FP_STORAGE_RAID_LIST);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_RAID_LIST] = appqueue_cli_mf_storage_raid_list;

	appqueue_cli_init_storage_raid("create","Create a RAID array", APPQUEUE_CLI_FP_STORAGE_RAID_CREATE);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_RAID_CREATE] = appqueue_cli_mf_storage_raid_create;

	appqueue_cli_init_storage_raid("delete","Delete a RAID array", APPQUEUE_CLI_FP_STORAGE_RAID_DELETE);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_RAID_DELETE] = appqueue_cli_mf_storage_raid_delete;

	appqueue_cli_init_storage_raid("recover","Recover a RAID array", APPQUEUE_CLI_FP_STORAGE_RAID_RECOVER);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_RAID_RECOVER] = appqueue_cli_mf_storage_raid_recover;


	/* storage: vstorage menu */
	appqueue_cli_init_storage_vstorage("list","List Virtual Storage Containers", APPQUEUE_CLI_FP_STORAGE_VSTORAGE_LIST);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VSTORAGE_LIST] = appqueue_cli_mf_storage_vstorage_list;

	appqueue_cli_init_storage_vstorage("avail","Display available Storage systems", APPQUEUE_CLI_FP_STORAGE_VSTORAGE_AVAIL);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VSTORAGE_AVAIL] = appqueue_cli_mf_storage_vstorage_avail;

	appqueue_cli_init_storage_vstorage("create","Create a Virtual Storage Container", APPQUEUE_CLI_FP_STORAGE_VSTORAGE_CREATE);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VSTORAGE_CREATE] = appqueue_cli_mf_storage_vstorage_create;

	appqueue_cli_init_storage_vstorage("delete","Delete a Virtual Storage Container", APPQUEUE_CLI_FP_STORAGE_VSTORAGE_DELETE);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VSTORAGE_DELETE] = appqueue_cli_mf_storage_vstorage_delete;

	appqueue_cli_init_storage_vstorage("snapadd","Create a Snapshot Container", APPQUEUE_CLI_FP_STORAGE_VSTORAGE_SNADD);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VSTORAGE_SNADD] = appqueue_cli_mf_storage_vstorage_snapadd;

	appqueue_cli_init_storage_vstorage("snapdel","Delete a Snapshot Container", APPQUEUE_CLI_FP_STORAGE_VSTORAGE_SNDEL);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VSTORAGE_SNDEL] = appqueue_cli_mf_storage_vstorage_snapdel;

	appqueue_cli_init_storage_vstorage("assign","Assign a Virtual Storage Container", APPQUEUE_CLI_FP_STORAGE_VSTORAGE_ASSIGN);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VSTORAGE_ASSIGN] = appqueue_cli_mf_storage_vstorage_assign;

	/* storage: vdisk menu */
	appqueue_cli_init_storage_vdisk("list","List Virtual Disks", APPQUEUE_CLI_FP_STORAGE_VDISK_LIST);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VDISK_LIST] = appqueue_cli_mf_storage_vdisk_list;

	appqueue_cli_init_storage_vdisk("usage","Show Virtual Container Usage", APPQUEUE_CLI_FP_STORAGE_VDISK_USAGE);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VDISK_USAGE] = appqueue_cli_mf_storage_vdisk_usage;

	appqueue_cli_init_storage_vdisk("create","Create a new Virtual Disk", APPQUEUE_CLI_FP_STORAGE_VDISK_CREATE);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VDISK_CREATE] = appqueue_cli_mf_storage_vdisk_create;

	appqueue_cli_init_storage_vdisk("delete","Delete a Virtual Disk", APPQUEUE_CLI_FP_STORAGE_VDISK_DELETE);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VDISK_DELETE] = appqueue_cli_mf_storage_vdisk_delete;

	appqueue_cli_init_storage_vdisk("clone","Clone an existing Virtual Disk", APPQUEUE_CLI_FP_STORAGE_VDISK_CLONE);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VDISK_CLONE] = appqueue_cli_mf_storage_vdisk_clone;

	appqueue_cli_init_storage_vdisk("map","Map a Virtual Disk", APPQUEUE_CLI_FP_STORAGE_VDISK_MAP);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VDISK_MAP] = appqueue_cli_mf_storage_vdisk_map;

	/* storage: vmedia menu */
	appqueue_cli_init_storage_vmedia("list","List Virtual Media", APPQUEUE_CLI_FP_STORAGE_VMEDIA_LIST);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VMEDIA_LIST] = appqueue_cli_mf_storage_vmedia_list;

	appqueue_cli_init_storage_vmedia("add","Add Media to Virtual Library", APPQUEUE_CLI_FP_STORAGE_VMEDIA_ADD);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VMEDIA_ADD] = appqueue_cli_mf_storage_vmedia_add;

	appqueue_cli_init_storage_vmedia("del","Remove Media from Virtual Library", APPQUEUE_CLI_FP_STORAGE_VMEDIA_DEL);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VMEDIA_DEL] = appqueue_cli_mf_storage_vmedia_del;

	appqueue_cli_init_storage_vmedia("license","Change License Information", APPQUEUE_CLI_FP_STORAGE_VMEDIA_LICENSE);
	appqueue_xmenu[APPQUEUE_CLI_FP_STORAGE_VMEDIA_LICENSE] = appqueue_cli_mf_storage_vmedia_license;


	/* sys disk menu */
	appqueue_cli_init_sys_disk("bootdisk","Set Boot Partition", APPQUEUE_CLI_FP_SYS_DISK_BOOTDISK);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_DISK_BOOTDISK] = appqueue_cli_mf_sys_disk_bootdisk;

	appqueue_cli_init_sys_disk("swapdisk","Set Swap Partition", APPQUEUE_CLI_FP_SYS_DISK_SWAPDISK);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_DISK_SWAPDISK] = appqueue_cli_mf_sys_disk_swapdisk;

	appqueue_cli_init_sys_disk("appqdisk","Set AppQueue Partition", APPQUEUE_CLI_FP_SYS_DISK_APPQDISK);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_DISK_APPQDISK] = appqueue_cli_mf_sys_disk_appqdisk;

	appqueue_cli_init_sys_disk("datadisk","Set Data Partition", APPQUEUE_CLI_FP_SYS_DISK_DATADISK);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_DISK_DATADISK] = appqueue_cli_mf_sys_disk_datadisk;

	appqueue_cli_init_sys_disk("fdisk","Edit Disk Partitions", APPQUEUE_CLI_FP_SYS_DISK_FDISK);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_DISK_FDISK] = appqueue_cli_mf_sys_disk_fdisk;

	/* sys clock menu */
	appqueue_cli_init_sys_clock("ntp1","Primary NTP Server", APPQUEUE_CLI_FP_SYS_CLOCK_NTPA);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_CLOCK_NTPA] = appqueue_cli_mf_sys_clock_ntpa;

	appqueue_cli_init_sys_clock("ntp2","Secondary NTP Server", APPQUEUE_CLI_FP_SYS_CLOCK_NTPB);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_CLOCK_NTPB] = appqueue_cli_mf_sys_clock_ntpb;

	appqueue_cli_init_sys_clock("ntp3","Tertiary NTP Server", APPQUEUE_CLI_FP_SYS_CLOCK_NTPC);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_CLOCK_NTPC] = appqueue_cli_mf_sys_clock_ntpc;

	appqueue_cli_init_sys_clock("ntpint","NTP Polling Interval", APPQUEUE_CLI_FP_SYS_CLOCK_NTPINT);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_CLOCK_NTPINT] = appqueue_cli_mf_sys_clock_ntpint;

	/* sys auth menu */
	appqueue_cli_init_sys_auth("aquser","Set aqcli username", APPQUEUE_CLI_FP_SYS_AUTH_AQUSER);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_AUTH_AQUSER] = appqueue_cli_mf_sys_auth_aquser;

	appqueue_cli_init_sys_auth("aqpass","Set aqcli password", APPQUEUE_CLI_FP_SYS_AUTH_AQPASS);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_AUTH_AQPASS] = appqueue_cli_mf_sys_auth_aqpass;

	appqueue_cli_init_sys_auth("clipass","Set CLI password", APPQUEUE_CLI_FP_SYS_AUTH_CLIPASS);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_AUTH_CLIPASS] = appqueue_cli_mf_sys_auth_clipass;

	appqueue_cli_init_sys_auth("root","Enable root access", APPQUEUE_CLI_FP_SYS_AUTH_ROOT);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_AUTH_ROOT] = appqueue_cli_mf_sys_auth_root;

	appqueue_cli_init_sys_auth("rootpass","Set root password", APPQUEUE_CLI_FP_SYS_AUTH_ROOTPASS);
	appqueue_xmenu[APPQUEUE_CLI_FP_SYS_AUTH_ROOTPASS] = appqueue_cli_mf_sys_auth_rootpass;


	/* maint menu */
	appqueue_cli_init_maint("shell","Launch Linux Shell", APPQUEUE_CLI_FP_MAINT_SHELL);
	appqueue_xmenu[APPQUEUE_CLI_FP_MAINT_SHELL] = appqueue_cli_mf_maint_shell;

	appqueue_cli_init_maint("tsdump","Tech Support Dump", APPQUEUE_CLI_FP_MAINT_TSDUMP);
	appqueue_xmenu[APPQUEUE_CLI_FP_MAINT_TSDUMP] = appqueue_cli_mf_maint_tsdump;

	appqueue_cli_init_maint("dbappq","AppQueue Database", APPQUEUE_CLI_FP_MAINT_DBAPPQ);
	appqueue_xmenu[APPQUEUE_CLI_FP_MAINT_DBAPPQ] = appqueue_cli_mf_maint_dbappq;

	appqueue_cli_init_maint("dbkaos","Hypervisor Database", APPQUEUE_CLI_FP_MAINT_DBKAOS);
	appqueue_xmenu[APPQUEUE_CLI_FP_MAINT_DBKAOS] = appqueue_cli_mf_maint_dbkaos;

	appqueue_cli_init_maint("dbvmsess","VM Session Database", APPQUEUE_CLI_FP_MAINT_DBVMSESS);
	appqueue_xmenu[APPQUEUE_CLI_FP_MAINT_DBVMSESS] = appqueue_cli_mf_maint_dbvmsess;

	/* boot menu */
	appqueue_cli_init_boot("status","System Boot Status", APPQUEUE_CLI_FP_BOOT_STATUS);
	appqueue_xmenu[APPQUEUE_CLI_FP_BOOT_STATUS] = appqueue_cli_mf_boot_status;

	appqueue_cli_init_boot("upgrade","Check for Hypervisor Upgrades", APPQUEUE_CLI_FP_BOOT_UPGRADE);
	appqueue_xmenu[APPQUEUE_CLI_FP_BOOT_UPGRADE] = appqueue_cli_mf_boot_upgrade;

	appqueue_cli_init_boot("reboot","Reboot Hypervisor", APPQUEUE_CLI_FP_BOOT_REBOOT);
	appqueue_xmenu[APPQUEUE_CLI_FP_BOOT_REBOOT] = appqueue_cli_mf_boot_reboot;

	appqueue_cli_init_boot("gtimg","Get Hypervisor Image", APPQUEUE_CLI_FP_BOOT_GTIMG);
	appqueue_xmenu[APPQUEUE_CLI_FP_BOOT_GTIMG] = appqueue_cli_mf_boot_gtimg;

	appqueue_cli_init_boot("factory","Factory Reset Hypervisor", APPQUEUE_CLI_FP_BOOT_FACTORY);
	appqueue_xmenu[APPQUEUE_CLI_FP_BOOT_FACTORY] = appqueue_cli_mf_boot_factory;

}

void
appqueue_cli_mf_main(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_main.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_main.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_main.climenu[i].menu_cmd,appqueue_cli_menu_main.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sHypervisor]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAIN;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_info(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_info.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_info.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_info.climenu[i].menu_cmd,appqueue_cli_menu_info.climenu[i].menu_desc);
		}	
	}

	printf("\n\n%sInfo]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_apps(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_apps.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_apps.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_apps.climenu[i].menu_cmd,appqueue_cli_menu_apps.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sApp Modules]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_vm(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_vm.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_vm.index; i++) {
			printf("    %s  \t - %s\n",appqueue_cli_menu_vm.climenu[i].menu_cmd,appqueue_cli_menu_vm.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sVM]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_net(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_net.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_net.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_net.climenu[i].menu_cmd,appqueue_cli_menu_net.climenu[i].menu_desc);
		}
	}	

	printf("\n\n%sNetworking]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_sys(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_sys.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_sys.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_sys.climenu[i].menu_cmd,appqueue_cli_menu_sys.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sSystem]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_setup(void)
{
	char cliask[255];
	char clians[255];
	u16 index = 0;
	u8 y = 0, f = 0, m = 0, r = 0;

	printf("\n [System Setup]");
	printf("\n----------------");
	printf("\n\n");

	printf("The setup procedure will configure local storage and basic networking.\n");
	printf("The storage device(s) must be partitioned using the fdisk command in\n");
	printf("the system menu before setup.\n\n");
	printf("After setup is completed. Use the gtimg command in the boot menu to\n");
	printf("install the Hypervisor kernel to the local boot disk.\n\n");
	printf("Visit %s if you need further assistance.\n",APPQUEUE_LINK);

	printf("\nNew installations must say no to recovery question below.\n\n");

	sprintf(cliask,"Do you wish to recover an existing install (y/n)? ");
	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'n') {
			r = 0;
			y++;
			break;
		} else if (clians[0] == 'y') {
			r = 1;
			y++;
			break;
		}
	}
	y = 0;

	printf("\n### WARNING ###\n");
	if (!r) {
		printf("The setup procedure will *destroy* all existing data on the storage device selected.\n");
		printf("Please select a different storage device or backup the data before proceeding.\n\n");
	} else {
		printf("The recovery procedure may *fail*. Please backup the data before proceeding.\n\n");
	}

	sprintf(cliask,"Do you wish to proceed with setup (y/n)? ");
	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'n') {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAIN;
			return;
		} else if (clians[0] == 'y') {
			y++;
			break;
		}
	}
	y = 0;

	appqueue_cli_mf_sys_hostname();
	appqueue_cli_mf_sys_domain();

	/* DNS */
	if (kattach_cfg_shm->dns != 0) {
		kattach_cfg_shm->dns_ip[0] = kattach_cfg_shm->dns;
	}

	appqueue_cli_mf_sys_dns();
	appqueue_cli_mf_sys_disk_bootdisk();
	appqueue_cli_mf_sys_disk_swapdisk();
	appqueue_cli_mf_sys_disk_appqdisk();
	appqueue_cli_mf_sys_disk_datadisk();

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_mf_net_netif_addif();
		sprintf(cliask,"\nDo you wish to configure another interface (y/n)? ");
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'n') {
			y++;
			break;
		}
	}

	y = 0;
	sprintf(cliask,"Select the primary network interface (eg. eth0): ");

	appqueue_cli_lock_cfg();

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		for (index = 0; index < kattach_netdev_shm->index; index++) {
			if ((kattach_netdev_shm->pif[index].mtu == 0) || (kattach_netdev_shm->pif[index].status == KATTACH_LINK_STATUS_DELETED)) continue;
			if (!strncmp(clians,kattach_netdev_shm->pif[index].devname,strlen(clians))) {
				f = 1;
				kattach_cfg_shm->ip = kattach_netdev_shm->pif[index].ip;
				kattach_cfg_shm->gw = kattach_netdev_shm->pif[index].gw;
				kattach_cfg_shm->slash = kattach_netdev_shm->pif[index].mask;
				sprintf(kattach_cfg_shm->netdev,"%s",kattach_netdev_shm->pif[index].devname);
				if (kattach_cfg_shm->ip == 0) kattach_cfg_shm->dhcp = 1;
				for (m = 0; m <= 5; m++) {
					kattach_cfg_shm->mac[m] = kattach_netdev_shm->pif[index].mac[m];
				}
				kattach_cfg_shm->dns = kattach_cfg_shm->dns_ip[0];
				y++;
				break;
			}
		}
		if (strlen(clians) == 0) {
			y++;
			break;
		}
	}

	y = 0;
	sprintf(cliask,"\n\nIs this setup correct (y/n)? ");
	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'n') {
			y++;
			printf("\nSetup aborted. Please re-run setup again.\n\n");
			appqueue_cli_unlock_cfg();
			break;
		} else if (clians[0] == 'y') {
			if (r) {
				kattach_cfg_shm->mode = 0xf1;					/* recovery mode */
			} else {
				kattach_cfg_shm->mode = 1;					/* change modes */
			}
			kattach_appqueue->status = CM_MSG_ST_NONE;
			appqueue_cli_upd_cfg();

			printf("\n\n");
			printf("Setup Progress: \n\nPlease wait...\n\n");
			m = 0;
			while (!m) {
				if (kattach_appqueue->status == (kattach_appqueue->status | CM_MSG_ST_RECOVERY)) {
					kattach_appqueue->status ^= CM_MSG_ST_RECOVERY;
					printf(" [-] Hypervisor entering recovery mode...\n");
				}
				if (kattach_appqueue->status == (kattach_appqueue->status | CM_MSG_ST_INIT)) {
					kattach_appqueue->status ^= CM_MSG_ST_INIT;
					printf(" [-] Re-initializing Hypervisor...\n");
				}
				if (kattach_appqueue->status == (kattach_appqueue->status | CM_MSG_ST_BOOTFS)) {
					kattach_appqueue->status ^= CM_MSG_ST_BOOTFS;
					printf(" [*] Creating Boot Device Filesystem...\n");
				}
				if (kattach_appqueue->status == (kattach_appqueue->status | CM_MSG_ST_BOOTLOADER)) {
					kattach_appqueue->status ^= CM_MSG_ST_BOOTLOADER;
					printf(" [*] Installing Hypervisor Boot Loader...\n");
				}
				if (kattach_appqueue->status == (kattach_appqueue->status | CM_MSG_ST_MBR)) {
					kattach_appqueue->status ^= CM_MSG_ST_MBR;
					printf(" [*] Writing Master Boot Record...\n");
				}
				if (kattach_appqueue->status == (kattach_appqueue->status | CM_MSG_ST_SSHKEYS)) {
					kattach_appqueue->status ^= CM_MSG_ST_SSHKEYS;
					printf(" [*] Generating SSH keys...\n");
				}
				if (kattach_appqueue->status == (kattach_appqueue->status | CM_MSG_ST_SSHD)) {
					kattach_appqueue->status ^= CM_MSG_ST_SSHD;
					printf(" [!] Restarting SSHd...\n");
				}
				if (kattach_appqueue->status == (kattach_appqueue->status | CM_MSG_ST_AQFS)) {
					kattach_appqueue->status ^= CM_MSG_ST_AQFS;
					printf(" [*] Creating AppQueue Filesystem...\n");
				}
				if (kattach_appqueue->status == (kattach_appqueue->status | CM_MSG_ST_INITAQFS)) {
					kattach_appqueue->status ^= CM_MSG_ST_INITAQFS;
					printf(" [*] Initializing AppQueue Filesystem...\n");
				}
				if (kattach_appqueue->status == (kattach_appqueue->status | CM_MSG_ST_CONTENT)) {
					kattach_appqueue->status ^= CM_MSG_ST_CONTENT;
					printf(" [*] Creating Content Filesystem...\n");
				}
				if (kattach_appqueue->status == (kattach_appqueue->status | CM_MSG_ST_INITVM)) {
					kattach_appqueue->status ^= CM_MSG_ST_INITVM;
					printf(" [*] Initializing Virtual Memory...\n");
				}
				if (kattach_appqueue->status == (kattach_appqueue->status | CM_MSG_ST_INITSQL)) {
					kattach_appqueue->status ^= CM_MSG_ST_INITSQL;
					printf(" [!] Initializing SQL databases...\n");
				}
				if (kattach_appqueue->status == (kattach_appqueue->status | CM_MSG_ST_VSWITCH)) {
					kattach_appqueue->status ^= CM_MSG_ST_VSWITCH;
					printf(" [#] Initializing Virtual Switching Fabric...\n");
				}
				if (kattach_appqueue->status == (kattach_appqueue->status | CM_MSG_ST_FIREWALL)) {
					kattach_appqueue->status ^= CM_MSG_ST_FIREWALL;
					printf(" [#] Enabling Firewalls...\n");
				}
				if (kattach_appqueue->status == (kattach_appqueue->status | CM_MSG_ST_VIRT)) {
					kattach_appqueue->status ^= CM_MSG_ST_VIRT;
					printf(" [+] Initializing Virtualization Systems...\n");
				}
				if (kattach_appqueue->status == (kattach_appqueue->status | CM_MSG_ST_DONE)) {
					kattach_appqueue->status ^= CM_MSG_ST_DONE;
					printf(" [-] Installation Complete...\n");
					kattach_appqueue->status = 0;
					m++;
					break;
				}
			}
			y++;
			break;
		}
	}
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAIN;
	return;
}

void
appqueue_cli_mf_maint(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_maint.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_maint.index; i++) {
			if ((appqueue_cli_user_auth < APPQUEUE_CLI_AUTH_DEV) &&
			    ((!strncmp(appqueue_cli_menu_maint.climenu[i].menu_cmd,"dbappq",strlen(appqueue_cli_menu_maint.climenu[i].menu_cmd))) ||
			    (!strncmp(appqueue_cli_menu_maint.climenu[i].menu_cmd,"dbkaos",strlen(appqueue_cli_menu_maint.climenu[i].menu_cmd))) ||
			    (!strncmp(appqueue_cli_menu_maint.climenu[i].menu_cmd,"dbvmsess",strlen(appqueue_cli_menu_maint.climenu[i].menu_cmd))))) continue;
			printf("     %s  \t - %s\n",appqueue_cli_menu_maint.climenu[i].menu_cmd,appqueue_cli_menu_maint.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sMaintenance]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAINT;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_boot(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_boot.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_boot.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_boot.climenu[i].menu_cmd,appqueue_cli_menu_boot.climenu[i].menu_desc);
		}
	}	

	printf("\n\n%sBoot]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_BOOT;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_exit(void)
{
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAIN;
	appqueue_exit = 1;
	return;
}

void
appqueue_cli_mf_info_sys(void)
{
	struct statvfs testfs;
	long unsigned int aq_upd = 0, aq_uph = 0, aq_upm = 0, aq_ups = 0, aq_diff = 0, aq_ut = 0;
	unsigned long mfree = 0, mtotal = 0;
	float pcnt = 0, opcnt = 0;
	time_t aq_now;
	struct utsname aq_bootk;
	int i = 0, x = 0;

	uname(&aq_bootk);

	printf("\nKernel Attached Operating System (KaOS)\n");
	printf("version %s (%s)\n",APPQUEUE_VERSION,APPQUEUE_ARCH);

	printf("%s",APPQUEUE_MENU_BAR);

	appqueue_cli_getinfo();
        time(&aq_now);
        printf("\n\nSystem Information @ %s\n",ctime(&aq_now));
        printf("%s\n",appqueue_cpu);
        printf("%u processor cores detected\n",appqueue_proc);
	if (appqueue_virt == 1) {
		printf("Intel(R) Hardware Virtualization detected.\n\n");
	} else if (appqueue_virt == 2) {
		printf("AMD Hardware Virtualization detected.\n\n");
	} else if (appqueue_virt == 3) {
		printf("WARNING: Unsupported Nested Hypervisor(s) detected.\n\n");
	} else {
		printf("WARNING: Hardware Virtualization NOT detected. Check BIOS.\n\n");
	}
        appqueue_cli_getuptime();
        aq_upd = (long unsigned int) appqueue_uptime / 86400;
        aq_diff = appqueue_uptime - (aq_upd * 86400);
        aq_uph = (long unsigned int) aq_diff / 3600;
        aq_ut = aq_diff - (aq_uph * 3600);
        aq_upm = (long unsigned int) aq_ut / 60;
        aq_ups = aq_ut - (aq_upm * 60);
        printf("Hypervisor is up %lu days, %lu hours, %lu minutes and %lu seconds\n\n",aq_upd,aq_uph,aq_upm,aq_ups);
	if (kattach_cfg_shm->hostname[0] != '\0') {
		printf("Hypervisor is %s.%s.\n\n",kattach_cfg_shm->hostname,kattach_cfg_shm->domain);
	}
	if (kattach_install_shm->diskboot[0] != '\0') {
		printf(" Boot partition      :  /dev/%s\n",kattach_install_shm->diskboot);
	}
	if (kattach_install_shm->diskswap[0] != '\0') {
		printf(" Swap partition      :  /dev/%s\n",kattach_install_shm->diskswap);
	}
	if (kattach_install_shm->diskappq[0] != '\0') {
		x = statvfs(APPQUEUE_PATH,&testfs);
		mfree = ((testfs.f_bfree * testfs.f_bsize) / 1048576);
		mtotal = ((testfs.f_blocks * testfs.f_bsize) / 1048576);
		opcnt = testfs.f_blocks / 100;
		pcnt = (testfs.f_blocks - testfs.f_bfree) / opcnt;
		printf(" AppQueue partition  :  /dev/%s\t(%lu MB, %lu MB avail, %2.2f %% used)\n",kattach_install_shm->diskappq,mtotal,mfree,pcnt);
	}
	if (kattach_install_shm->diskdata[0] != '\0') {
		printf(" Content partition   :  /dev/%s\n",kattach_install_shm->diskdata);
	}
	printf("\n");
	for (i = 0; i <= 5; i++) {
		if (kattach_cfg_shm->dns_ip[i] == 0) continue;
		printf(" DNS Server %d: %lu.%lu.%lu.%lu\n",(i+1),((kattach_cfg_shm->dns_ip[i] >> 24) & 0xff),((kattach_cfg_shm->dns_ip[i] >> 16) & 0xff),
								 ((kattach_cfg_shm->dns_ip[i] >> 8) & 0xff), ((kattach_cfg_shm->dns_ip[i]) & 0xff));
	}
	printf("\n");
	printf(" %s version %s %s\n",aq_bootk.sysname,aq_bootk.release,aq_bootk.version);
	printf("\n");
        printf("Visit %s for further information.\n\n",APPQUEUE_LINK);
        appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO;
	return;
}

void
appqueue_cli_mf_info_link(void)
{
	u16 index = 0;
	char portspd[10];
	char portdup[5];
	char portphy[10];
	char portstat[10];

	printf("\n");
	printf("     Port       Speed    Duplex    LACP          Type       Status\n");
	printf("---------    --------    ------    ----    ----------    ---------\n");

	for (index = 0; index < kattach_netdev_shm->index; index++) {
		if ((kattach_netdev_shm->pif[index].status == KATTACH_LINK_STATUS_DELETED) || (kattach_netdev_shm->pif[index].mtu == 0) ||
			(kattach_netdev_shm->pif[index].pvid == 0)) continue;

		switch(kattach_netdev_shm->pif[index].status) {
			case KATTACH_LINK_STATUS_DISABLED:
				sprintf(portstat,"disabled");
				if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_PSUEDO) {
					sprintf(portspd,"Virtual");
					sprintf(portdup,"n/a");
					sprintf(portphy,"Virtual");					/* FIXME: this should parse psuedo and display type */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_INFINIBAND) {
					sprintf(portspd,"InfiBnd");
					sprintf(portdup,"n/a");
					sprintf(portphy,"InfiBnd");
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_USB) {
					sprintf(portspd,"USB");
					sprintf(portdup,"n/a");
					sprintf(portphy,"USB Host");
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_10GBE) {
					sprintf(portspd,"10GbE");
					sprintf(portdup,"auto");
					sprintf(portphy,"10000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_1GBE) {
					sprintf(portspd,"1GbE");
					sprintf(portdup,"auto");
					sprintf(portphy,"1000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_ETHERNET) {
					sprintf(portspd,"100 Mbps");
					sprintf(portdup,"auto");
					sprintf(portphy,"100BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else {
					sprintf(portspd,"Unknown");
					sprintf(portdup,"n/a");
					sprintf(portphy,"unknown");
				}
				break;

			case KATTACH_LINK_STATUS_DOWN:
				sprintf(portstat,"down");
				if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_PSUEDO) {
					sprintf(portspd,"Virtual");
					sprintf(portdup,"n/a");
					sprintf(portphy,"Virtual");					/* FIXME: this should parse psuedo and display type */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_INFINIBAND) {
					sprintf(portspd,"InfiBnd");
					sprintf(portdup,"n/a");
					sprintf(portphy,"InfiBnd");
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_USB) {
					sprintf(portspd,"USB");
					sprintf(portdup,"n/a");
					sprintf(portphy,"USB Host");
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_10GBE) {
					sprintf(portspd,"10GbE");
					sprintf(portdup,"auto");
					sprintf(portphy,"10000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_1GBE) {
					sprintf(portspd,"1GbE");
					sprintf(portdup,"auto");
					sprintf(portphy,"1000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_ETHERNET) {
					sprintf(portspd,"100 Mbps");
					sprintf(portdup,"auto");
					sprintf(portphy,"100BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else {
					sprintf(portspd,"Unknown");
					sprintf(portdup,"n/a");
					sprintf(portphy,"unknown");
				}
				break;

			case KATTACH_LINK_STATUS_LACP:
			case KATTACH_LINK_STATUS_LACP_NEW:
				sprintf(portstat,"lacp");
				sprintf(portdup,"auto");
				sprintf(portphy,"802.3ad");
				sprintf(portspd,"auto");
				break;


			case KATTACH_LINK_STATUS_UP:
				sprintf(portstat,"up");
				if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_PSUEDO) {
					sprintf(portspd,"Virtual");
					sprintf(portdup,"n/a");
					sprintf(portphy,"Virtual");					/* FIXME: this should parse psuedo and display type */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_INFINIBAND) {
					sprintf(portspd,"InfiBnd");
					sprintf(portdup,"n/a");
					sprintf(portphy,"InfiBnd");
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_USB) {
					sprintf(portspd,"USB");
					sprintf(portdup,"n/a");
					sprintf(portphy,"USB Host");
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_10GBE) {
					sprintf(portspd,"10GbE");
					sprintf(portdup,"auto");
					sprintf(portphy,"10000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_1GBE) {
					sprintf(portspd,"1GbE");
					sprintf(portdup,"auto");
					sprintf(portphy,"1000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_ETHERNET) {
					sprintf(portspd,"100 Mbps");
					sprintf(portdup,"auto");
					sprintf(portphy,"100BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else {
					sprintf(portspd,"Unknown");
					sprintf(portdup,"n/a");
					sprintf(portphy,"unknown");
				}
				break;

			case KATTACH_LINK_STATUS_UP_10000:
				sprintf(portstat,"up");
				sprintf(portspd,"10GbE");
				sprintf(portdup,"full");
				if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_10GBE) {
					sprintf(portphy,"10000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_1GBE) {
					sprintf(portphy,"1000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_ETHERNET) {
					sprintf(portphy,"100BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else {
					sprintf(portphy,"unknown");
				}
				break;

			case KATTACH_LINK_STATUS_UP_1000:
				sprintf(portstat,"up");
				sprintf(portspd,"1GbE");
				sprintf(portdup,"full");
				if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_10GBE) {
					sprintf(portphy,"10000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_1GBE) {
					sprintf(portphy,"1000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_ETHERNET) {
					sprintf(portphy,"100BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else {
					sprintf(portphy,"unknown");
				}
				break;

			case KATTACH_LINK_STATUS_UP_100:
				sprintf(portstat,"up");
				sprintf(portspd,"100 Mbps");
				sprintf(portdup,"full");
				if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_10GBE) {
					sprintf(portphy,"10000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_1GBE) {
					sprintf(portphy,"1000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_ETHERNET) {
					sprintf(portphy,"100BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else {
					sprintf(portphy,"unknown");
				}
				break;

			case KATTACH_LINK_STATUS_UP_10:
				sprintf(portstat,"up");
				sprintf(portspd,"10 Mbps");
				sprintf(portdup,"full");
				if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_10GBE) {
					sprintf(portphy,"10000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_1GBE) {
					sprintf(portphy,"1000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_ETHERNET) {
					sprintf(portphy,"100BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else {
					sprintf(portphy,"unknown");
				}
				break;

			case KATTACH_LINK_STATUS_UP_H10000:
				sprintf(portstat,"up");
				sprintf(portspd,"10GbE");
				sprintf(portdup,"half");
				if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_10GBE) {
					sprintf(portphy,"10000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_1GBE) {
					sprintf(portphy,"1000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_ETHERNET) {
					sprintf(portphy,"100BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else {
					sprintf(portphy,"unknown");
				}
				break;

			case KATTACH_LINK_STATUS_UP_H1000:
				sprintf(portstat,"up");
				sprintf(portspd,"1GbE");
				sprintf(portdup,"half");
				if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_10GBE) {
					sprintf(portphy,"10000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_1GBE) {
					sprintf(portphy,"1000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_ETHERNET) {
					sprintf(portphy,"100BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else {
					sprintf(portphy,"unknown");
				}
				break;

			case KATTACH_LINK_STATUS_UP_H100:
				sprintf(portstat,"up");
				sprintf(portspd,"100 Mbps");
				sprintf(portdup,"half");
				if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_10GBE) {
					sprintf(portphy,"10000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_1GBE) {
					sprintf(portphy,"1000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_ETHERNET) {
					sprintf(portphy,"100BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else {
					sprintf(portphy,"unknown");
				}
				break;

			case KATTACH_LINK_STATUS_UP_H10:
				sprintf(portstat,"up");
				sprintf(portspd,"10 Mbps");
				sprintf(portdup,"half");
				if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_10GBE) {
					sprintf(portphy,"10000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_1GBE) {
					sprintf(portphy,"1000BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else if (kattach_netdev_shm->pif[index].type == KATTACH_LINK_TYPE_ETHERNET) {
					sprintf(portphy,"100BaseT");					/* FIXME: actually go look at the device to get phytype */
				} else {
					sprintf(portphy,"unknown");
				}
				break;

			default:
				sprintf(portstat,"unknown");
				sprintf(portspd,"unknown");
				sprintf(portdup,"ukwn");
				sprintf(portphy,"Unknown");
				break;
		}

		printf(" %8s    %8s      %4s     %3s     %9s     %8s\n",kattach_netdev_shm->pif[index].devname,portspd,portdup,
										(kattach_netdev_shm->pif[index].pvid == 0x8023) ? "yes" : "no", portphy,portstat);
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO;
	return;

}

void
appqueue_cli_mf_info_l2(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_info_l2.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_info_l2.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_info_l2.climenu[i].menu_cmd,appqueue_cli_menu_info_l2.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sLayer 2 Info]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_L2;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_info_l3(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_info_l3.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_info_l3.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_info_l3.climenu[i].menu_cmd,appqueue_cli_menu_info_l3.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sLayer 3 Info]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_L3;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_info_l2_vlan(void)
{
	u32 index = 0;
	char vsub[20];

	printf("\n");
	printf("VLAN\t \t  Subnet\t CIDR \t Free \t Used \t Private \t External \n");
	printf("----\t----------------\t ---- \t ---- \t ---- \t ------- \t -------- \n\n");

	for (index = 0; index < kattach_vbridge_shm->index; index++) {
		if (kattach_vbridge_shm->vbridge[index].vlan == 0) continue;
		memset(vsub,0,sizeof(vsub));
		sprintf(vsub,"%lu.%lu.%lu.%lu",((kattach_vbridge_shm->vbridge[index].vsubnet >> 24) & 0xff),
							((kattach_vbridge_shm->vbridge[index].vsubnet >> 16) & 0xff),
							((kattach_vbridge_shm->vbridge[index].vsubnet >> 8) & 0xff),
							((kattach_vbridge_shm->vbridge[index].vsubnet) & 0xff));
		printf("%4u\t%16s  \t  /%-2u \t %4u \t %4u \t     %3s \t %8s \n",kattach_vbridge_shm->vbridge[index].vlan, vsub,
							kattach_vbridge_shm->vbridge[index].vmask,
							kattach_vbridge_shm->vbridge[index].vpfree,
							kattach_vbridge_shm->vbridge[index].vbruse,
							(kattach_vbridge_shm->vbridge[index].vbrlocal) ? "yes" : "no",
							(kattach_vbridge_shm->vbridge[index].vbrlocal) ? "n/a" : kattach_vbridge_shm->vbridge[index].vlanext);
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_L2;
	return;
}

void
appqueue_cli_mf_info_l2_vbridge(void)
{
	u32 index = 0;
	char vsub[20];
	char vbip[20];

	printf("\n");
	printf(" Bridge\t  VLAN\t \t  Subnet\tCIDR \t\tBridge IP \t\t  MAC\t \t External \n");
	printf("-------\t  ----\t----------------\t---- \t ---------------- \t -------------------- \t --------\n\n");

	for (index = 0; index < kattach_vbridge_shm->index; index++) {
		if (kattach_vbridge_shm->vbridge[index].vlan == 0) continue;
		memset(vsub,0,sizeof(vsub));
		sprintf(vsub,"%lu.%lu.%lu.%lu",((kattach_vbridge_shm->vbridge[index].vsubnet >> 24) & 0xff),
							((kattach_vbridge_shm->vbridge[index].vsubnet >> 16) & 0xff),
							((kattach_vbridge_shm->vbridge[index].vsubnet >> 8) & 0xff),
							((kattach_vbridge_shm->vbridge[index].vsubnet) & 0xff));
		sprintf(vbip,"%lu.%lu.%lu.%lu",((kattach_vbridge_shm->vbridge[index].vbrip >> 24) & 0xff),
							((kattach_vbridge_shm->vbridge[index].vbrip >> 16) & 0xff),
							((kattach_vbridge_shm->vbridge[index].vbrip >> 8) & 0xff),
							((kattach_vbridge_shm->vbridge[index].vbrip) & 0xff));

		printf("vbr%-4u\t  %4u\t%16s  \t /%-2u  \t %16s  \t   %02x:%02x:%02x:%02x:%02x:%02x\t %8s\n",
							kattach_vbridge_shm->vbridge[index].vlan,kattach_vbridge_shm->vbridge[index].vlan,
							vsub,kattach_vbridge_shm->vbridge[index].vmask,
							vbip, kattach_vbridge_shm->vbridge[index].bmac[0],
							kattach_vbridge_shm->vbridge[index].bmac[1], kattach_vbridge_shm->vbridge[index].bmac[2],
							kattach_vbridge_shm->vbridge[index].bmac[3], kattach_vbridge_shm->vbridge[index].bmac[4],
							kattach_vbridge_shm->vbridge[index].bmac[5], 
							(kattach_vbridge_shm->vbridge[index].vbrlocal) ? "n/a" : kattach_vbridge_shm->vbridge[index].vlanext);
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_L2;
	return;
}

void
appqueue_cli_mf_info_l2_vmac(void)
{
	u32 index = 0;
	char vip[20];
	u16 vbr = 0;

	printf("\n");
	printf("\tMAC  \t \t  VLAN\t \t IP \t\t    Device Type\n");
	printf("-------------------  \t  ----\t   --------------\t---------------\n");

	for (index = 0; index < kattach_vbridge_shm->index; index++) {
		if (kattach_vbridge_shm->vbridge[index].vlan == 0) continue;
		memset(vip,0,sizeof(vip));
		sprintf(vip,"%lu.%lu.%lu.%lu",((kattach_vbridge_shm->vbridge[index].vbrip >> 24) & 0xff),
							((kattach_vbridge_shm->vbridge[index].vbrip >> 16) & 0xff),
							((kattach_vbridge_shm->vbridge[index].vbrip >> 8) & 0xff),
							((kattach_vbridge_shm->vbridge[index].vbrip) & 0xff));
		printf(" %02x:%02x:%02x:%02x:%02x:%02x\t  %4u\t%16s  \t Virtual Bridge\n",kattach_vbridge_shm->vbridge[index].bmac[0],
							kattach_vbridge_shm->vbridge[index].bmac[1], kattach_vbridge_shm->vbridge[index].bmac[2],
							kattach_vbridge_shm->vbridge[index].bmac[3], kattach_vbridge_shm->vbridge[index].bmac[4],
							kattach_vbridge_shm->vbridge[index].bmac[5], kattach_vbridge_shm->vbridge[index].vlan, vip);
	}

	for (index = 0; index < kattach_vmports_shm->index; index++) {
		if (kattach_vmports_shm->vmports[index].vmpip == 0) continue;
		memset(vip,0,sizeof(vip));
		vbr = kattach_vmports_shm->vmports[index].vbridge; 
		sprintf(vip,"%lu.%lu.%lu.%lu",((kattach_vmports_shm->vmports[index].vmpip >> 24) & 0xff),
						((kattach_vmports_shm->vmports[index].vmpip >> 16) & 0xff),
						((kattach_vmports_shm->vmports[index].vmpip >> 8) & 0xff),
						((kattach_vmports_shm->vmports[index].vmpip) & 0xff));
		printf(" %02x:%02x:%02x:%02x:%02x:%02x\t  %4u\t%16s  \t Virtual Port  \n",kattach_vmports_shm->vmports[index].vmac[0],
							kattach_vmports_shm->vmports[index].vmac[1], kattach_vmports_shm->vmports[index].vmac[2],
							kattach_vmports_shm->vmports[index].vmac[3], kattach_vmports_shm->vmports[index].vmac[4],
							kattach_vmports_shm->vmports[index].vmac[5], kattach_vbridge_shm->vbridge[vbr].vlan, vip);
	}
	
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_L2;
	return;
}

void
appqueue_cli_mf_info_l2_vmports(void)
{

	u32 index = 0, vmst = 0;
	u16 vbr = 0;
	char vip[20];

	printf("\n");
	printf("  Port\t  VLAN\t \t IP \t\t\tMAC  \t \t    Status     Guest \n");
	printf("------\t  ----\t   --------------\t-------------------  \t ---------     -----\n");

	for (index = 0; index < kattach_vmports_shm->index; index++) {
		if (kattach_vmports_shm->vmports[index].vmpip == 0) continue;
		memset(vip,0,sizeof(vip));
		vbr = kattach_vmports_shm->vmports[index].vbridge;
		vmst = kattach_vmports_shm->vmports[index].vmst;
		sprintf(vip,"%lu.%lu.%lu.%lu",((kattach_vmports_shm->vmports[index].vmpip >> 24) & 0xff),
						((kattach_vmports_shm->vmports[index].vmpip >> 16) & 0xff),
						((kattach_vmports_shm->vmports[index].vmpip >> 8) & 0xff),
						((kattach_vmports_shm->vmports[index].vmpip) & 0xff));
		printf("  %4lu\t  %4u\t %16s  \t %02x:%02x:%02x:%02x:%02x:%02x\t  %s      %4lu\n", (index+1), kattach_vbridge_shm->vbridge[vbr].vlan,vip,
						kattach_vmports_shm->vmports[index].vmac[0], kattach_vmports_shm->vmports[index].vmac[1],
						kattach_vmports_shm->vmports[index].vmac[2], kattach_vmports_shm->vmports[index].vmac[3],
						kattach_vmports_shm->vmports[index].vmac[4], kattach_vmports_shm->vmports[index].vmac[5],
						((kattach_vmst_shm->vmsess[vmst].vmstatus == 0x01) || (kattach_vmst_shm->vmsess[vmst].vmstatus == 0x05))
						 ? "active" : "inactive" , (1 + vmst));
	}
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_L2;
	return;
}

void
appqueue_cli_mf_info_l3_ip(void)
{
	u16 index = 0;
	char ifip[20];
	char gwip[20];

	printf("\n");
	printf(" Interface          IP Address     CIDR             Gateway     VLAN     Status\n");
	printf("----------    ----------------    -----    ----------------    -----    -------\n");

	for (index = 0; index < kattach_netdev_shm->index; index++) {
		if ((kattach_netdev_shm->pif[index].status == KATTACH_LINK_STATUS_DELETED) || (kattach_netdev_shm->pif[index].mtu == 0)) continue;
		/* special dhcp case -- store real ip in gw field */
		if (kattach_netdev_shm->pif[index].ip == 0xfffefdfc) {
			sprintf(ifip,"%lu.%lu.%lu.%lu",(kattach_netdev_shm->pif[index].gw >> 24) & 0xff,(kattach_netdev_shm->pif[index].gw >> 16) & 0xff,
							(kattach_netdev_shm->pif[index].gw >> 8) & 0xff, (kattach_netdev_shm->pif[index].gw) & 0xff);
			sprintf(gwip,"0.0.0.0");
		} else {
			sprintf(ifip,"%lu.%lu.%lu.%lu",(kattach_netdev_shm->pif[index].ip >> 24) & 0xff,(kattach_netdev_shm->pif[index].ip >> 16) & 0xff,
							(kattach_netdev_shm->pif[index].ip >> 8) & 0xff, (kattach_netdev_shm->pif[index].ip) & 0xff);
			sprintf(gwip,"%lu.%lu.%lu.%lu",(kattach_netdev_shm->pif[index].gw >> 24) & 0xff,(kattach_netdev_shm->pif[index].gw >> 16) & 0xff,
							(kattach_netdev_shm->pif[index].gw >> 8) & 0xff, (kattach_netdev_shm->pif[index].gw) & 0xff);
		}
		printf("%10s    %16s      /%2u    %16s     %4u       %4s\n",kattach_netdev_shm->pif[index].devname,ifip,kattach_netdev_shm->pif[index].mask,
										gwip,kattach_netdev_shm->pif[index].pvid, 
										(kattach_netdev_shm->pif[index].status == KATTACH_LINK_STATUS_DOWN) ? "down" : 
										((kattach_netdev_shm->pif[index].status == KATTACH_LINK_STATUS_DISABLED) ? "disabled" :
										(((kattach_netdev_shm->pif[index].status == KATTACH_LINK_STATUS_LACP) ||
										  (kattach_netdev_shm->pif[index].status == KATTACH_LINK_STATUS_LACP_NEW)) ? "lacp" : "up")));
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_L3;
	return;
}

void
appqueue_cli_mf_info_l3_route(void)
{
	FILE *stream;
	char buf[255];
	char rif[64];
	char dip[20];
	char dmask[20];
	char dgw[20];

	u32 rdst = 0, rgw = 0, rmask = 0;
	u16 rfl = 0, rrefc = 0, ruse = 0, rmet = 0, rmtu = 0, rwin = 0, rirtt = 0;

	memset(rif,0,sizeof(rif));
	stream = fopen("/proc/net/route","r");

	printf("\n");
	printf("     Destination              Mask           Gateway   Flags   Metric   Interface\n");
	printf("----------------  ----------------  ----------------  ------  -------  ----------\n");

	if (stream == (FILE *)0) {
		printf("\n");
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_L3;
		return;
	}

	fgets(buf,255,stream);				/* grab the header */

	while (!feof(stream)) {
		memset(buf,0,sizeof(buf));
		fgets(buf,255,stream);
		if (strlen(buf) != 0) {
			sscanf(buf,"%s\t%08lx\t%08lx\t%u\t%u\t%u\t%x\t%08lx\t%u\t%u\t%u",rif,&rdst,&rgw,&rfl,&rrefc,&ruse,&rmet,&rmask,&rmtu,&rwin,&rirtt);

			sprintf(dip,"%lu.%lu.%lu.%lu",(rdst) & 0xff, (rdst >> 8) & 0xff, (rdst >> 16) & 0xff, (rdst >> 24) & 0xff);
			sprintf(dmask,"%lu.%lu.%lu.%lu",(rmask) & 0xff, (rmask >> 8) & 0xff, (rmask >> 16) & 0xff, (rmask >> 24) & 0xff);
			sprintf(dgw,"%lu.%lu.%lu.%lu",(rgw) & 0xff, (rgw >> 8) & 0xff, (rgw >> 16) & 0xff, (rgw >> 24) & 0xff);
		
			printf("%16s  %16s  %16s    %s%s%s%s   %6u  %10s\n",dip,dmask,dgw,(rfl == (rfl | 0x0001)) ? "U" : "-",(rfl == (rfl | 0x0002)) ? "G" : "-",
								(rfl == (rfl | 0x0004)) ? "H" : "-", (rfl == (rfl | 0x0010)) ? "D" : "-",rmet,rif);
		}
	}

	printf("\n");
	fclose(stream);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_L3;
	return;
}

void
appqueue_cli_mf_info_l3_arp(void)
{
	FILE *stream;
	char buf[255];

	memset(buf,0,sizeof(buf));
	stream = fopen("/proc/net/arp","r");

	printf("\n");
	printf("IP address       HW type     Flags       HW address            Mask     Device\n");
	printf("---------------  --------    ------      -----------------     -----    ----------\n");


	if (stream == (FILE *)0) {
		printf("\n");
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_L3;
		return;
	}

	while (!feof(stream)) {
		fgets(buf,255,stream);
		if ((buf[0] >= 0x30) && (buf[0] <= 0x39)) {
			printf("%s",buf);
		}
		memset(buf,0,sizeof(buf));
	}

	fclose(stream);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_L3;
	return;
	
		
}

void
appqueue_cli_mf_apps_list(void)
{
	u32 aindex = 0;
	char mytree[10];

	printf("\n");
	printf("           App Module     Tree          Vendor      Cfg    Deployed                 Version\n");
	printf("---------------------    -----    ------------    -----    --------    --------------------\n");

	for (aindex = 0; aindex < kattach_appmods_shm->index; aindex++) {
		if (kattach_appmods_shm->appmodules[aindex].state == CM_APP_M_STATE_DELETED) continue;
		if (kattach_appmods_shm->appmodules[aindex].name[0] == '\0') continue;
		if (kattach_appmods_shm->appmodules[aindex].srctree == APPQUEUE_TREE_EDGE) {
			sprintf(mytree,"edge");
		} else if (kattach_appmods_shm->appmodules[aindex].srctree == APPQUEUE_TREE_DEV) {
			sprintf(mytree," dev");
		} else if (kattach_appmods_shm->appmodules[aindex].srctree == APPQUEUE_TREE_SUPPORTED) {
			sprintf(mytree,"supp");
		} else {
			sprintf(mytree,"unkn");
		}
		printf(" %20s     %4s     %11s    %5lu       %5lu      %19s\n", kattach_appmods_shm->appmodules[aindex].name, mytree, "Carbon Mtn",
									kattach_appmods_shm->appmodules[aindex].config, 
									kattach_appmods_shm->appmodules[aindex].deployed,
									kattach_appmods_shm->appmodules[aindex].version);
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS;
	return;

}

void
appqueue_cli_mf_apps_avail(void)
{
	FILE *stream;
	char listpath[64];
	char buf[255];
	char appmod[64];
	char appdsc[255];
	u32 tstamp = 0;
	u8 status = 0, foo = 0;

	appqueue_get_applist();
	sprintf(listpath,"%s/list.e",APPQUEUE_PATH_IMAGES);
	stream = fopen(listpath,"r");

	if (stream == (FILE *)0) {
		printf("\n");
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS;
		return;
	}

	printf("\n");
	printf("           App Module        Version                             Description\n");
	printf("---------------------    -----------    ------------------------------------\n");
	printf("\n");

	while(!feof(stream)) {
		memset(buf,0,strlen(buf));
		fgets(buf,255,stream);
		if (strlen(buf) != 0) {
			sscanf(buf,"%s\t%lu\t%u\t%s\t%u",appmod,&tstamp,&status,appdsc,&foo);
			printf(" %20s     %10lu     %35s\n",appmod,tstamp,appdsc);
		}
	}

	printf("\n");
	fclose(stream);

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS;
	return;
}

void
appqueue_cli_mf_apps_install(void)
{
	int y = 0, x = 0, q = 0, m = 0;
	char c = '\0';
	u32 oindex = 0, aindex = 0;
	char appmod[128];
	char cliask[255];
	char clians[255];

	printf("\n");
	printf("Available App Queues: ");
	printf(" (c)ommunity ");
	printf(" (d)evelopment ");

	/* FIXME: supported tree */

	if (y == 1) {
		printf(" (s)upported ");
	}
	
	printf("\n\n");

	sprintf(cliask,"Select an AppQueue: ");

	while (y == 0) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'c') {
			q = APPQUEUE_TREE_EDGE;
			y++;
			break;
		} else if (clians[0] == 'd') {
			q = APPQUEUE_TREE_DEV;
			y++;
			break;
		} else if (clians[0] == 's') {
			if (y == 1) {
				q = APPQUEUE_TREE_SUPPORTED;
				y++;
				break;
			}
		}
	}
	y = 0;
	memset(appmod,0,sizeof(appmod));
	printf("\nEnter App Module to Install: ");

	while (!x) {
		c = tolower(appqueue_cli_getch(1));
		if (c == '\n') {
			if (strlen(appmod) <= 1) {
				if (!appqueue_options.index) return;
				x = 1;
				continue;
			}
			for (aindex = 0; aindex < kattach_appmods_shm->index; aindex++) {
				if (strncmp(appmod,kattach_appmods_shm->appmodules[aindex].name,strlen(appmod))) continue;
				if (q != kattach_appmods_shm->appmodules[aindex].srctree) continue;
				printf("\nApp Module %s is already installed.\n", appmod);
				m = 1;
				break;
			}
			if (!m) {
				oindex = appqueue_options.index;
				sprintf(appqueue_options.option[oindex].app_mod,"%s",appmod);
				if (q == APPQUEUE_TREE_SUPPORTED) {
					sprintf(appqueue_options.option[oindex].url,"%s%s%s%s",APPQUEUE_HTTP,appmod,APPQUEUE_URL_VENDOR_CM,APPQUEUE_URL_DOMAIN_S);
					sprintf(appqueue_options.option[oindex].cliurl,"%s%s%s%s%s",APPQUEUE_HTTP,appmod,APPQUEUE_URL_VENDOR_CM,APPQUEUE_URL_CLI,APPQUEUE_URL_DOMAIN_S);
					sprintf(appqueue_options.option[oindex].mgrurl,"%s%s%s%s%s",APPQUEUE_HTTP,appmod,APPQUEUE_URL_VENDOR_CM,APPQUEUE_URL_MGR,APPQUEUE_URL_DOMAIN_S);
				} else if (q == APPQUEUE_TREE_DEV) {
					sprintf(appqueue_options.option[oindex].url,"%s%s%s%s",APPQUEUE_HTTP,appmod,APPQUEUE_URL_VENDOR_CM,APPQUEUE_URL_DOMAIN_D);
					sprintf(appqueue_options.option[oindex].cliurl,"%s%s%s%s%s",APPQUEUE_HTTP,appmod,APPQUEUE_URL_VENDOR_CM,APPQUEUE_URL_CLI,APPQUEUE_URL_DOMAIN_D);
					sprintf(appqueue_options.option[oindex].mgrurl,"%s%s%s%s%s",APPQUEUE_HTTP,appmod,APPQUEUE_URL_VENDOR_CM,APPQUEUE_URL_MGR,APPQUEUE_URL_DOMAIN_D);
				} else {
					sprintf(appqueue_options.option[oindex].url,"%s%s%s%s",APPQUEUE_HTTP,appmod,APPQUEUE_URL_VENDOR_CM,APPQUEUE_URL_DOMAIN_E);
					sprintf(appqueue_options.option[oindex].cliurl,"%s%s%s%s%s",APPQUEUE_HTTP,appmod,APPQUEUE_URL_VENDOR_CM,APPQUEUE_URL_CLI,APPQUEUE_URL_DOMAIN_E);
					sprintf(appqueue_options.option[oindex].mgrurl,"%s%s%s%s%s",APPQUEUE_HTTP,appmod,APPQUEUE_URL_VENDOR_CM,APPQUEUE_URL_MGR,APPQUEUE_URL_DOMAIN_E);
				}
				appqueue_options.index++;
			}
			memset(appmod,0,sizeof(appmod));
			c = '\0';
			y = 0;
			m = 0;
			printf("\nEnter App Module to Install: ");
		} else {
			appmod[y] = c;
			y++;
		}
	}

	appqueue_install();

	for (oindex = 0; oindex <= appqueue_options.index; oindex++) {
		if (appqueue_options.option[oindex].valid == 0) continue;
		aindex = kattach_appmods_shm->index;
		kattach_appmods_shm->index++;
		sprintf(kattach_appmods_shm->appmodules[aindex].name,"%s",appqueue_options.option[oindex].app_mod);
		sprintf(kattach_appmods_shm->appmodules[aindex].url,"%s",appqueue_options.option[oindex].url);
		sprintf(kattach_appmods_shm->appmodules[aindex].filename,"%s.aqi",appqueue_options.option[oindex].app_mod);
		kattach_appmods_shm->appmodules[aindex].app_size = (u16) (appqueue_options.option[oindex].size * 4);		/* assume 25% compression with sqfs */
		kattach_appmods_shm->appmodules[aindex].vendor_id = KATTACH_VID_CM;
		kattach_appmods_shm->appmodules[aindex].state = CM_APP_M_STATE_NEW;
		memset(appqueue_options.option[oindex].app_mod,0,sizeof(appqueue_options.option[oindex].app_mod));
		memset(appqueue_options.option[oindex].url,0,sizeof(appqueue_options.option[oindex].url));
	}

	appqueue_options.index = 0;

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_MODULE)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_MODULE;
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS;
	return;
}

void
appqueue_cli_mf_apps_remove(void)
{
	int y = 0, x = 0;
	char c = '\0';
	u32 oindex = 0, aindex = 0;
	char appmod[128];

	memset(appmod,0,sizeof(appmod));
	printf("\nEnter App Module to Remove: ");

	while (!x) {
		c = tolower(appqueue_cli_getch(1));
		if (c == '\n') {
			if (strlen(appmod) <= 1) {
				if (!appqueue_options.index) return;
				x = 1;
				continue;
			}
			for (aindex = 0; aindex < kattach_appmods_shm->index; aindex++) {
				if (strlen(kattach_appmods_shm->appmodules[aindex].name) != strlen(appmod)) {
					continue;
				} else if (!strncmp(kattach_appmods_shm->appmodules[aindex].name,appmod,strlen(kattach_appmods_shm->appmodules[aindex].name))) {
					oindex = appqueue_options.index;
					sprintf(appqueue_options.option[oindex].app_mod,"%s",appmod);
					/* FIXME */
					sprintf(appqueue_options.option[oindex].url,"%s%s%s%s",APPQUEUE_HTTP,appmod,APPQUEUE_URL_VENDOR_CM,APPQUEUE_URL_DOMAIN_E);
					appqueue_options.index++;
					memset(appmod,0,sizeof(appmod));
					c = '\0';
					y = 0;
					printf("\nEnter App Module to Remove: ");
					break;
				}
			}
		} else {
			appmod[y] = c;
			y++;
		}
	}

	if (appqueue_options.index) {
		appqueue_uninstall();
	}

	for (oindex = 0; oindex <= appqueue_options.index; oindex++) {
		for (aindex = 0; aindex < kattach_appmods_shm->index; aindex++) {
			if (strlen(kattach_appmods_shm->appmodules[aindex].name) != strlen(appqueue_options.option[oindex].app_mod)) {
				continue;
			} else if (!strncmp(kattach_appmods_shm->appmodules[aindex].name,appqueue_options.option[oindex].app_mod,strlen(kattach_appmods_shm->appmodules[aindex].name))) {
				kattach_appmods_shm->appmodules[aindex].state = CM_APP_M_STATE_DELETED;
				memset(appqueue_options.option[oindex].app_mod,0,sizeof(appqueue_options.option[oindex].app_mod));
				memset(appqueue_options.option[oindex].url,0,sizeof(appqueue_options.option[oindex].url));
				/* FIXME: decode aqi file and populate structure properly */
			}
		}
	}

	appqueue_options.index = 0;

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_MODULE)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_MODULE;
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS;
	return;
}

void
appqueue_cli_mf_apps_showvm(void)
{
	u32 index = 0, appindex = 0, cfgindex = 0;
	u8 appi = 0;

	printf("\n");
	printf("            VM Image                                                         Modules\n");
	printf("--------------------    ------------------------------------------------------------\n");

	for (index = 0; index < kattach_vmimages_shm->index; index++) {
		if (!kattach_vmimages_shm->vmimage[index].active) continue;
		printf("%20s    ",kattach_vmimages_shm->vmimage[index].vminame);
		for (appi = 0; appi < kattach_vmimages_shm->vmimage[index].appi; appi++) {
			appindex = kattach_vmimages_shm->vmimage[index].appindex[appi];
			cfgindex = kattach_vmimages_shm->vmimage[index].cfggrp[appi];
			/* [272]: Added code to display cfggrp in showvm command */
			printf(" %s:%s ",kattach_appmods_shm->appmodules[appindex].name,
					 kattach_cfggrp_shm->cfggrp[cfgindex].name);
			/* FIXME: support multiline if we go past the allocated space */
		}
		printf("\n");
	}
	printf("\n");
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS;
	return;
}

void
appqueue_cli_mf_apps_createvm(void)
{
	u32 index = 0, vmindex = 0, aindex = 0;
	char cliask[255];
	char clians[255];
	u8 y = 0, d = 0, c = 0, z = 0;

	sprintf(cliask,"Enter the VM image name: ");

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) != 0) {
			for (index = 0; index < kattach_vmimages_shm->index; index++) {
				if (strlen(clians) != strlen(kattach_vmimages_shm->vmimage[index].vminame)) continue;
				if (strncmp(clians,kattach_vmimages_shm->vmimage[index].vminame,strlen(kattach_vmimages_shm->vmimage[index].vminame)) != 0) continue;
				d++;
			}
			if (!d) {
				y++;
				break;
			} else {
				d = 0;
			}
		} else {
			printf("\n");
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS;
			return;
		}
	}

	/* FIXME: clean this up. Use local variables instead of leaving lock open */

	appqueue_cli_lock_vmimages();

	index = kattach_vmimages_shm->index;
	kattach_vmimages_shm->index++;
	sprintf(kattach_vmimages_shm->vmimage[index].vminame,"%s",clians);

	y = 0;
	d = 0;
	vmindex = index;

	while (!y) {
		sprintf(cliask,"Add App Module (? to list): ");
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		
		if (strlen(clians) > 1) {
			for (index = 0; index < kattach_appmods_shm->index; index++) {
				if (strlen(clians) != strlen(kattach_appmods_shm->appmodules[index].name)) continue;
				if (!strncmp(clians,kattach_appmods_shm->appmodules[index].name,strlen(clians))) {
					if (!strncmp(clians,"vkaos",strlen(clians))) {
						printf("\nvKaOS added by default. Add App Module for vKaOS ignored.\n\n");
						break;
					}
					d = kattach_vmimages_shm->vmimage[vmindex].appi;
					kattach_vmimages_shm->vmimage[vmindex].appi++;
					kattach_vmimages_shm->vmimage[vmindex].appindex[d] = index;
					c++;
					z = 0;
					while(!z) {
						sprintf(cliask,"Config Group For App (? to list): ");
						memset(clians,0,sizeof(clians));
						appqueue_cli_askq(cliask,1,clians);
						if (clians[0] == '?') {
							printf("\n");
							for (aindex = 0; aindex < kattach_cfggrp_shm->index; aindex++) {
								if (kattach_cfggrp_shm->cfggrp[aindex].name[0] == '\0') continue;
								if (kattach_cfggrp_shm->cfggrp[aindex].appmidx != index) continue;
								printf("%s\n",kattach_cfggrp_shm->cfggrp[aindex].name);
							}
							printf("\n");
						} else if (strlen(clians) > 1) {
							for (aindex = 0; aindex < kattach_cfggrp_shm->index; aindex++) {
								if (kattach_cfggrp_shm->cfggrp[aindex].name[0] == '\0') continue;
								if (kattach_cfggrp_shm->cfggrp[aindex].appmidx != index) continue;
								if (strlen(clians) != strlen(kattach_cfggrp_shm->cfggrp[aindex].name)) continue;
								if (strncmp(clians,kattach_cfggrp_shm->cfggrp[aindex].name,strlen(clians))) continue;
								kattach_vmimages_shm->vmimage[vmindex].cfggrp[d] = aindex;
								z++;
								break;
							}
						} else if (strlen(clians) == 0) {
							if (c) {
								kattach_vmimages_shm->vmimage[vmindex].active = 1;
								kattach_vmimages_shm->vmimage[vmindex].changed = 1;
								appqueue_cli_upd_vmimages();
								appqueue_cli_setflags(APPQUEUE_LCK_APPMODULES);
							} else {
								memset(kattach_vmimages_shm->vmimage[index].vminame,0,sizeof(kattach_vmimages_shm->vmimage[index].vminame));
								kattach_vmimages_shm->index--;
								appqueue_cli_upd_vmimages();
							}
							appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS;
							return;
						}
					}
					break;
				}
			}
		} else if (clians[0] == '?') {
			appqueue_cli_mf_apps_list();
			printf("\n");
		} else {
			y++;
			break;
		}
	}

	if (c != 0) {
		kattach_vmimages_shm->vmimage[vmindex].active = 1;
		kattach_vmimages_shm->vmimage[vmindex].changed = 1;
		/* REMOVED 
		for (y = 0; y < kattach_vmimages_shm->vmimage[vmindex].appi; y++) {
                	index = kattach_vmimages_shm->vmimage[vmindex].appindex[y];
                	kattach_appmods_shm->appmodules[index].config++;
        	}
		*/

		appqueue_cli_upd_vmimages();
		appqueue_cli_setflags(APPQUEUE_LCK_APPMODULES);
	} else {
		memset(kattach_vmimages_shm->vmimage[index].vminame,0,sizeof(kattach_vmimages_shm->vmimage[index].vminame));
		kattach_vmimages_shm->index--;
		appqueue_cli_upd_vmimages();

	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS;
	return;
	

}

void
appqueue_cli_mf_apps_deletevm(void)
{
	u32 index = 0, xindex = 0;
	char cliask[255];
	char clians[255];
	u8 y = 0, d = 0, x = 0;

	sprintf(cliask,"Enter the VM image name to delete: ");

	appqueue_cli_lock_vmimages();

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) != 0) {
			for (index = 0; index < kattach_vmimages_shm->index; index++) {
				if (strlen(clians) != strlen(kattach_vmimages_shm->vmimage[index].vminame)) continue;
				if (strncmp(clians,kattach_vmimages_shm->vmimage[index].vminame,strlen(kattach_vmimages_shm->vmimage[index].vminame)) != 0) continue;
				d++;
				break;
			}
			if (d) {
				kattach_vmimages_shm->vmimage[index].active = 0;
				kattach_vmimages_shm->vmimage[index].changed = 1;
				for (x = 0; x < kattach_vmimages_shm->vmimage[index].appi; x++) {
                			xindex = kattach_vmimages_shm->vmimage[index].appindex[x];
                			kattach_appmods_shm->appmodules[xindex].config--;
        			}
			}
			y++;
			break;
		} else {
			printf("\n");
			appqueue_cli_upd_vmimages();
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS;
			return;
		}
	}

	if (!d) {
		printf(" [!] VM image %s not found \n\n",clians);
	} else {
		printf(" Deleted VM image %s.\n\n",clians);
	}

	appqueue_cli_upd_vmimages();
	appqueue_cli_setflags(APPQUEUE_LCK_APPMODULES);

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS;
	return;
	
}

void
appqueue_cli_mf_vm_list(void)
{
	u32 index = 0;
	u16 vport = 0, vlan = 0, vbridge = 0;
	char pip[20];
	char vmstatus[20];

	printf("\n");
	printf("  vPID               Name    vCPU      vMem       Status    VLAN    Port          Primary IP            VM App Image\n");
	printf("------    ---------------    ----    ------    ---------    ----    ----    ----------------    --------------------\n");

	for (index = 0; index < kattach_vmst_shm->index; index++) {
		if (kattach_vmst_shm->vmsess[index].vmstatus == 0x04) continue;
		vport = kattach_vmst_shm->vmsess[index].vmport[0];
		vbridge = kattach_vmports_shm->vmports[vport].vbridge;
		vlan = kattach_vbridge_shm->vbridge[vbridge].vlan;
		if (kattach_vmst_shm->vmsess[index].vmstatus == 0x01) {
			sprintf(vmstatus,"   active");
		} else if (kattach_vmst_shm->vmsess[index].vmstatus == 0x00) {
			sprintf(vmstatus," starting");
		} else if (kattach_vmst_shm->vmsess[index].vmstatus == 0x02) {
			sprintf(vmstatus,"  stopped");
		} else if (kattach_vmst_shm->vmsess[index].vmstatus == 0x03) {
			sprintf(vmstatus," disabled");
		} else if (kattach_vmst_shm->vmsess[index].vmstatus == 0x04) {
			continue;
		} else {
			sprintf(vmstatus,"  unknown");
		}

		sprintf(pip,"%lu.%lu.%lu.%lu", (kattach_vmports_shm->vmports[vport].vmpip >> 24) & 0xff,
						(kattach_vmports_shm->vmports[vport].vmpip >> 16) & 0xff,
						(kattach_vmports_shm->vmports[vport].vmpip >> 8) & 0xff,
						(kattach_vmports_shm->vmports[vport].vmpip) & 0xff);
		printf(" %5d     %14s     %3u    %6u    %s    %4u    %4u    %16s    %20s\n",kattach_vmst_shm->vmsess[index].vpid,
							kattach_vmst_shm->vmsess[index].vmname, kattach_vmst_shm->vmsess[index].vcpu,
							kattach_vmst_shm->vmsess[index].vmem, vmstatus, vlan, (vport + 1), pip,
							kattach_vmimages_shm->vmimage[kattach_vmst_shm->vmsess[index].vmimage].vminame);
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
	return;
}

void
appqueue_cli_mf_vm_deploy(void)
{
	u32 index = 0, vmindex = 0;
	u16 vlan = 0, vbridge = 0, vmem = 0, vcpu = 0;
	char cliask[255];
	char clians[255];
	u8 y = 0, len = 0, cnt = 0, f = 0;
	
	for (index = 0; index < kattach_appmods_shm->index; index++) {
		if (strncmp("vkaos",kattach_appmods_shm->appmodules[index].name,5)) continue;
		f = 1;
		break;
	}

	if (!f) {
		printf("\nPlease install the vkaos app module first!\n\n");
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
		return;
	}

	f = 0;
	index = 0;

	sprintf(cliask,"Enter Name of VM App Image to use (? list): ");

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		len = strlen(clians);
		if (len > 1) {
			for (index = 0; index < kattach_vmimages_shm->index; index++) {
				if (len != strlen(kattach_vmimages_shm->vmimage[index].vminame)) continue;
				vmindex = index;
				y++;
				break;
			}
		} else if (clians[0] == '?') {
			appqueue_cli_mf_apps_showvm();
			printf("\n");
		} else {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
			return;
		}
	}

	y = 0;
	sprintf(cliask,"Enter VLAN ID: ");

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		len = strlen(clians);
		if (!len) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
			return;
		} else if (len <= 4) {
			vlan = atoi(clians);
			if ((vlan) && (vlan <= 4094)) {
				for (index = 0; index < kattach_vbridge_shm->index; index++) {
					if (kattach_vbridge_shm->vbridge[index].vlan != vlan) continue;
					vbridge = index;
					f++;
					break;
				}
				if (f) {
					y++;
					break;
				} else {
					printf("\nVLAN %u does not exist.\n\n",vlan);
				}
			}
		} 
	}

	y = 0;
	sprintf(cliask,"Number of VMs to deploy: ");

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		len = strlen(clians);
		if (!len) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
			return;
		} else {
			cnt = atoi(clians);
			if ((cnt + kattach_vmst_shm->index) <= KATTACH_MAX_VMSESSIONS) {
				y++;
				break;
			}
		}
	}

	y = 0;
	sprintf(cliask,"Number of Virtual CPUs per VM: ");

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		len = strlen(clians);
		if (!len) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
			return;
		} else {
			vcpu = atoi(clians);
			/* FIXME: constrain this */
			y++;
			break;
		}
	}

	y = 0;
	sprintf(cliask,"Amount of Virtual Memory per VM (in MB): ");

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		len = strlen(clians);
		if (!len) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
			return;
		} else {
			vmem = atoi(clians);
			/* FIXME: constrain this */
			y++;
			break;
		}
	}

	appqueue_cli_lock_vmst();
	appqueue_cli_lock_vmports();

	for (y = 0; y < cnt; y++) {
		/* FIXME: check constraints, if full replace DELETED entries */
		/* create a new vmst */

		kattach_vmst_shm->vmsess[kattach_vmst_shm->index].vmstatus = 0x00;
		kattach_vmst_shm->vmsess[kattach_vmst_shm->index].vmoper = 0x06;
		kattach_vmst_shm->vmsess[kattach_vmst_shm->index].vmimage = vmindex;
		kattach_vmst_shm->vmsess[kattach_vmst_shm->index].vmport[0] = kattach_vmports_shm->index;
		kattach_vmst_shm->vmsess[kattach_vmst_shm->index].vmpidx++;
		kattach_vmst_shm->vmsess[kattach_vmst_shm->index].vcpu = vcpu;
		kattach_vmst_shm->vmsess[kattach_vmst_shm->index].vmem = vmem;
		kattach_vmst_shm->vmsess[kattach_vmst_shm->index].vmname[0] = '\0';

		/* create a new vmp */
		kattach_vmports_shm->vmports[kattach_vmports_shm->index].vmst = kattach_vmst_shm->index;
		kattach_vmports_shm->vmports[kattach_vmports_shm->index].vbridge = vbridge;

		kattach_vmst_shm->index++;
		kattach_vmports_shm->index++;
	}

	for (y = 0; y < kattach_vmimages_shm->vmimage[vmindex].appi; y++) {
		index = kattach_vmimages_shm->vmimage[vmindex].appindex[y];
		/* REMOVED
		kattach_appmods_shm->appmodules[index].deployed++;
		*/
	}

	appqueue_cli_upd_vmst();
	appqueue_cli_upd_vmports();
	appqueue_cli_setflags(APPQUEUE_LCK_APPMODULES);

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
	return;

}

void
appqueue_cli_mf_vm_start(void)
{
	u32 index = 0, findex = 0;
	char cliask[255];
	char clians[255];
	u8 y = 0, len = 0;

	sprintf(cliask,"Enter Name of VM to start: ");

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);

		len = strlen(clians);

		if (len) {
			for (index = 0; index < kattach_vmst_shm->index; index++) {
				if (len != strlen(kattach_vmst_shm->vmsess[index].vmname)) continue;
				if (strncmp(kattach_vmst_shm->vmsess[index].vmname,clians,len)) continue;
				
				findex = index;
				y++;
				break;
			}
		} else {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
			return;
		}
	}

	appqueue_cli_lock_vmst();
	kattach_vmst_shm->vmsess[findex].vmoper = 0x08;
	appqueue_cli_upd_vmst();
	
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
	return;
}

void
appqueue_cli_mf_vm_stop(void)
{
	u32 index = 0, findex = 0;
	char cliask[255];
	char clians[255];
	u8 y = 0, len = 0;

	sprintf(cliask,"Enter Name of VM to stop: ");

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);

		len = strlen(clians);

		if (len) {
			for (index = 0; index < kattach_vmst_shm->index; index++) {
				if (len != strlen(kattach_vmst_shm->vmsess[index].vmname)) continue;
				if (strncmp(kattach_vmst_shm->vmsess[index].vmname,clians,len)) continue;
				
				findex = index;
				y++;
				break;
			}
		} else {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
			return;
		}
	}

	appqueue_cli_lock_vmst();
	kattach_vmst_shm->vmsess[findex].vmoper = 0x07;
	appqueue_cli_upd_vmst();
	
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
	return;
}

void
appqueue_cli_mf_vm_restart(void)
{
	u32 index = 0, findex = 0;
	char cliask[255];
	char clians[255];
	u8 y = 0, len = 0;

	sprintf(cliask,"Enter Name of VM to restart: ");

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);

		len = strlen(clians);

		if (len) {
			for (index = 0; index < kattach_vmst_shm->index; index++) {
				if (len != strlen(kattach_vmst_shm->vmsess[index].vmname)) continue;
				if (strncmp(kattach_vmst_shm->vmsess[index].vmname,clians,len)) continue;
				
				findex = index;
				y++;
				break;
			}
		} else {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
			return;
		}
	}
	
	appqueue_cli_lock_vmst();
	kattach_vmst_shm->vmsess[findex].vmoper = 0x09;
	appqueue_cli_upd_vmst();	
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
	return;

}

void
appqueue_cli_mf_vm_killall(void)
{
	u32 index = 0, findex = 0;
	char cliask[255];
	char clians[255];
	u8 y = 0, len = 0, f = 0;

	sprintf(cliask,"Enter Name of VM Image to stop: ");

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);

		len = strlen(clians);

		if (len) {
			appqueue_cli_lock_vmst();
			for (index = 0; index < kattach_vmst_shm->index; index++) {
				if (len != strlen(kattach_vmimages_shm->vmimage[kattach_vmst_shm->vmsess[index].vmimage].vminame)) continue;
				if (strncmp(kattach_vmimages_shm->vmimage[kattach_vmst_shm->vmsess[index].vmimage].vminame,clians,len)) continue;
				findex = index;
				kattach_vmst_shm->vmsess[findex].vmoper = 0x0b;
				f++;
			}
			if (!f) {
				printf("VM App Image %s not found.\n",clians);
			}
			y++;
			break;
		} else {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
			return;
		}
	}
	
	appqueue_cli_upd_vmst();
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
	return;
}

void
appqueue_cli_mf_vm_startall(void)
{
	u32 index = 0, findex = 0;
	char cliask[255];
	char clians[255];
	u8 y = 0, len = 0, f = 0;

	sprintf(cliask,"Enter Name of VM Image to start: ");

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);

		len = strlen(clians);

		if (len) {
			appqueue_cli_lock_vmst();
			for (index = 0; index < kattach_vmst_shm->index; index++) {
				if (len != strlen(kattach_vmimages_shm->vmimage[kattach_vmst_shm->vmsess[index].vmimage].vminame)) continue;
				if (strncmp(kattach_vmimages_shm->vmimage[kattach_vmst_shm->vmsess[index].vmimage].vminame,clians,len)) continue;
				findex = index;
				kattach_vmst_shm->vmsess[findex].vmoper = 0x09;
				f++;
			}
			if (!f) {
				printf("VM App Image %s not found.\n",clians);
			}
			y++;
			break;
		} else {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
			return;
		}
	}
	
	appqueue_cli_upd_vmst();
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
	return;
}

void
appqueue_cli_mf_vm_remove(void)
{
	u32 index = 0, findex = 0, vmindex = 0;
	char cliask[255];
	char clians[255];
	u8 y = 0, len = 0;

	sprintf(cliask,"Enter Name of VM to remove: ");

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);

		len = strlen(clians);

		if (len) {
			for (index = 0; index < kattach_vmst_shm->index; index++) {
				if (len != strlen(kattach_vmst_shm->vmsess[index].vmname)) continue;
				if (strncmp(kattach_vmst_shm->vmsess[index].vmname,clians,len)) continue;
				
				findex = index;
				y++;
				break;
			}
		} else {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
			return;
		}
	}
	
	appqueue_cli_lock_vmst();

	kattach_vmst_shm->vmsess[findex].vmoper = 0x0a;
	vmindex = kattach_vmst_shm->vmsess[findex].vmimage;

	for (y = 0; y < kattach_vmimages_shm->vmimage[vmindex].appi; y++) {
		index = kattach_vmimages_shm->vmimage[vmindex].appindex[y];
		kattach_appmods_shm->appmodules[index].deployed--;
	}

	appqueue_cli_upd_vmst();
	appqueue_cli_setflags(APPQUEUE_LCK_APPMODULES);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
	return;

}

void
appqueue_cli_mf_vm_vdi(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_vm_vdi.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_vm_vdi.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_vm_vdi.climenu[i].menu_cmd,appqueue_cli_menu_vm_vdi.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sVDI]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM_VDI;
	appqueue_po = 0;
	return;


}

void
appqueue_cli_mf_vm_vdi_list(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM_VDI;
	return;
}

void
appqueue_cli_mf_vm_vdi_install(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM_VDI;
	return;
}

void
appqueue_cli_mf_vm_vdi_deploy(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM_VDI;
	return;
}

void
appqueue_cli_mf_vm_vdi_remove(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM_VDI;
	return;
}

void
appqueue_cli_mf_vm_vsrv(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_vm_vsrv.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_vm_vsrv.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_vm_vsrv.climenu[i].menu_cmd,appqueue_cli_menu_vm_vsrv.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sVirtual Server]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM_VSRV;
	appqueue_po = 0;
	return;


}

void
appqueue_cli_mf_vm_vsrv_list(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM_VSRV;
	return;
}

void
appqueue_cli_mf_vm_vsrv_install(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM_VSRV;
	return;
}

void
appqueue_cli_mf_vm_vsrv_deploy(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM_VSRV;
	return;
}

void
appqueue_cli_mf_vm_vsrv_remove(void)
{

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM_VSRV;
	return;
}



void
appqueue_cli_mf_net_vlan(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_net_vlan.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_net_vlan.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_net_vlan.climenu[i].menu_cmd,appqueue_cli_menu_net_vlan.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sVLAN]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VLAN;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_net_vports(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_net_vports.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_net_vports.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_net_vports.climenu[i].menu_cmd,appqueue_cli_menu_net_vports.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sVirtual Ports]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VPORTS;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_net_vports_list(void)
{
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VPORTS;
	return;
}

void
appqueue_cli_mf_net_vports_add(void)
{
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VPORTS;
	return;
}

void
appqueue_cli_mf_net_vports_del(void)
{
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VPORTS;
	return;
}

void
appqueue_cli_mf_net_vports_assign(void)
{
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VPORTS;
	return;
}

void
appqueue_cli_mf_net_vports_unassign(void)
{
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VPORTS;
	return;
}

void
appqueue_cli_mf_net_dhcp(void)
{
	u32 index = 0, vmpip = 0;
	u16 vbridge = 0, vlan = 0, vmask = 0;
	u8 pindex = 0;
	char vmip[20];

	printf("\n");
	printf("   Virtual Host    VLAN          VMAC                 Assigned IP    CIDR       Status\n");
	printf("---------------    ----    -----------------    -----------------    ----    ---------\n");

	for (index = 0; index < kattach_vmst_shm->index; index++) {
		if (kattach_vmst_shm->vmsess[index].vmstatus == 0x04) continue;					/* deleted */
		for (pindex = 0; pindex < kattach_vmst_shm->vmsess[index].vmpidx; pindex++) {
			vmpip = kattach_vmports_shm->vmports[kattach_vmst_shm->vmsess[index].vmport[pindex]].vmpip;
			vbridge = kattach_vmports_shm->vmports[kattach_vmst_shm->vmsess[index].vmport[pindex]].vbridge;
			vlan = kattach_vbridge_shm->vbridge[vbridge].vlan;
			vmask = kattach_vbridge_shm->vbridge[vbridge].vmask;
			sprintf(vmip,"%lu.%lu.%lu.%lu",(vmpip >> 24) & 0xff, (vmpip >> 16) & 0xff, (vmpip >> 8) & 0xff, (vmpip) & 0xff);
			printf(" %14s    %4u    %02x:%02x:%02x:%02x:%02x:%02x     %16s     /%2u     %s\n", kattach_vmst_shm->vmsess[index].vmname,
							vlan, kattach_vmports_shm->vmports[kattach_vmst_shm->vmsess[index].vmport[pindex]].vmac[0],
							kattach_vmports_shm->vmports[kattach_vmst_shm->vmsess[index].vmport[pindex]].vmac[1],
							kattach_vmports_shm->vmports[kattach_vmst_shm->vmsess[index].vmport[pindex]].vmac[2],
							kattach_vmports_shm->vmports[kattach_vmst_shm->vmsess[index].vmport[pindex]].vmac[3],
							kattach_vmports_shm->vmports[kattach_vmst_shm->vmsess[index].vmport[pindex]].vmac[4],
							kattach_vmports_shm->vmports[kattach_vmst_shm->vmsess[index].vmport[pindex]].vmac[5],
							vmip, vmask, ((kattach_vmst_shm->vmsess[index].vmstatus == 0x01) ||
									(kattach_vmst_shm->vmsess[index].vmstatus == 0x05)) ? "active" : "inactive");
		}
	}

        appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET;
        return;	
}

void
appqueue_cli_mf_net_netif_addif(void)
{
	u32 sip = 0, gwip = 0;
	u16 index = 0, mask = 0, vlan = 0, lindex = 0, lacpf = 0, mtu = 0;
	char cliask[255];
	char clians[255];
	u8 y = 0, lacp = 0;

	sprintf(cliask,"\nEnter a network interface to add (? to list): ");
	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == '?') {
			printf("\nAvailable Interfaces: \n\n");
			for (index = 0; index < kattach_netdev_shm->index; index++) {
				if ((kattach_netdev_shm->pif[index].mtu) || (kattach_netdev_shm->pif[index].pvid == 0x8023) 
					|| !(strlen(kattach_netdev_shm->pif[index].devname))) continue;
				printf("\t%s\t(%02x:%02x:%02x:%02x:%02x:%02x)\n",kattach_netdev_shm->pif[index].devname, kattach_netdev_shm->pif[index].mac[0],
							kattach_netdev_shm->pif[index].mac[1], kattach_netdev_shm->pif[index].mac[2],
							kattach_netdev_shm->pif[index].mac[3], kattach_netdev_shm->pif[index].mac[4],
							kattach_netdev_shm->pif[index].mac[5]);
			}
		} else if (strlen(clians) == 0) {
		        appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_NETIF;
		        return;
		} else {
			for (index = 0; index < kattach_netdev_shm->index; index++) {
				if ((!strncmp(kattach_netdev_shm->pif[index].devname,clians,strlen(clians))) && 
					(kattach_netdev_shm->pif[index].mtu == 0)) {
					y++;
					break;
				}
			}
		}
		
	}

	sprintf(cliask,"IPv4 address: ");
	y = 0;
	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		sip = appqueue_cli_parseip(clians);
		if (sip != 0) {
			y++;
		} else {
			memset(clians,0,sizeof(clians));
		}
	}
	memset(cliask,0,sizeof(cliask));
	memset(clians,0,sizeof(clians));
	y = 0;
	sprintf(cliask,"CIDR mask: ");
	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if ((strlen(clians) > 2)) {
			mask = appqueue_cli_parseip(clians);		/* FIXME: need to properly parse non-CIDR netmask */
		} else {
			mask = atoi(clians);
		}
		if (mask != 0) {
			y++;
		} else {
			memset(clians,0,sizeof(clians));
		}
	}

	sprintf(cliask,"Default Gateway IP: ");
	y = 0;
	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		gwip = appqueue_cli_parseip(clians);
		if (gwip != 0) {
			y++;
		} else {
			memset(clians,0,sizeof(clians));
		}
	}

	sprintf(cliask,"Default VLAN ID: ");
	y = 0;
	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		vlan = (u16) atoi(clians);
		if ((vlan > 0) && (vlan <= 4094)) {
			y++;
			break;
		}
	}

	sprintf(cliask,"Interface MTU (eg. 1500): ");
	y = 0;
	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		mtu = (u32) atol(clians);
		if (mtu > 0) {
			y++;
			break;
		}
	}

	sprintf(cliask,"Enable 802.3ad (LACP) Link Aggregration (y/n): ");
	y = 0;
	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'y') {
			lacp = 1;
			y++;
			break;
		} else if (clians[0] == 'n') {
			lacp = 0;
			y++;
			break;
		}
	}

	appqueue_cli_lock_netdev();

	if (lacp) {
		/* 802.3ad Link Aggregration connection */
		/* kattach uses this to create the bonding device */
		/* kattach automagically enslaves all interfaces with the same pvid */
		/* 802.1Q over an 802.3ad link is handled the same way we handled it for eth0 */

		for (lindex = 0; lindex < kattach_netdev_shm->index; lindex++) {
			if ((kattach_netdev_shm->pif[index].type != KATTACH_LINK_TYPE_PSUEDO) && (kattach_netdev_shm->pif[index].psuedo != KATTACH_LINK_STATUS_LACP_NEW) &&
				(kattach_netdev_shm->pif[index].pvid != vlan)) continue;
			lacpf = 1 + lindex;
			break;
		}

		kattach_netdev_shm->pif[index].ip = 0;
		kattach_netdev_shm->pif[index].mask = 0;
		kattach_netdev_shm->pif[index].gw = vlan;						/* tag the real pvid */
		kattach_netdev_shm->pif[index].pvid = 0x8023;						/* flag the pvid to 802.3ad status */
		kattach_netdev_shm->pif[index].mtu = mtu;
		kattach_netdev_shm->pif[index].status = KATTACH_LINK_STATUS_LACP_NEW;			/* lacp-new status */
		if (lacpf == 0) {
			lindex = kattach_netdev_shm->index;
			kattach_netdev_shm->index++;
		} else {
			lindex = lacpf - 1;								/* existing lacp entry */
		}
		kattach_netdev_shm->pif[index].lacpidx = lindex;
		index = lindex;										/* set this so we pickup the real values on our 802.3ad link */
		kattach_netdev_shm->pif[index].type = KATTACH_LINK_TYPE_PSUEDO;				/* psuedo device  */
		kattach_netdev_shm->pif[index].psuedo = KATTACH_LINK_STATUS_LACP_NEW;			/* 802.3ad */
		sprintf(kattach_netdev_shm->pif[index].devname,"lacp%u",vlan);				/* lacp device name is lacpX where X is the vlan ID */
	} else {
		kattach_netdev_shm->pif[index].status = KATTACH_LINK_STATUS_DOWN;
		kattach_netdev_shm->pif[index].type = KATTACH_LINK_TYPE_UNKNOWN;
	}

	if (lacpf) {
		printf("\n [!] WARNING - Existing LACP configuration found for VLAN %u.\n",vlan);
		if (kattach_netdev_shm->pif[index].mtu != mtu) 
			printf(" [-] WARNING - New MTU = %u. Previous MTU = %u\n",mtu,kattach_netdev_shm->pif[index].mtu);


		if (kattach_netdev_shm->pif[index].ip != sip)
			printf(" [-] WARNING - New IP = %lu.%lu.%lu.%lu. Previous IP = %lu.%lu.%lu.%lu.\n", (sip >> 24) & 0xff, (sip >> 16) & 0xff, (sip >> 8) & 0xff,
							(sip) & 0xff, (kattach_netdev_shm->pif[index].ip >> 24) & 0xff, (kattach_netdev_shm->pif[index].ip >> 16) & 0xff,
							(kattach_netdev_shm->pif[index].ip >> 8) & 0xff, (kattach_netdev_shm->pif[index].ip) & 0xff);

		if (kattach_netdev_shm->pif[index].gw != gwip)
			printf(" [-] WARNING - New GW = %lu.%lu.%lu.%lu. Previous GW = %lu.%lu.%lu.%lu.\n", (gwip >> 24) & 0xff, (gwip >> 16) & 0xff, (gwip >> 8) & 0xff,
							(gwip) & 0xff, (kattach_netdev_shm->pif[index].gw >> 24) & 0xff, (kattach_netdev_shm->pif[index].gw >> 16) & 0xff,
							(kattach_netdev_shm->pif[index].gw >> 8) & 0xff, (kattach_netdev_shm->pif[index].gw) & 0xff);

		if (kattach_netdev_shm->pif[index].mask != mask)
			printf(" [-] WARNING - New CIDR Mask = /%u. Previous CIDR Mask = /%u \n", mask, kattach_netdev_shm->pif[index].mask); 

	}


	kattach_netdev_shm->pif[index].mtu = mtu;
	kattach_netdev_shm->pif[index].pvid = vlan;
	kattach_netdev_shm->pif[index].gw = gwip;
	kattach_netdev_shm->pif[index].ip = sip;
	kattach_netdev_shm->pif[index].mask = mask;

	appqueue_cli_upd_netdev();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_NETIF;
	return;
}

void
appqueue_cli_mf_net_netif_editif(void)
{
	u32 sip = 0, gwip = 0;
	u16 index = 0, mask = 0, vlan = 0, lindex = 0, lacpf = 0, mtu = 0;
	char cliask[255];
	char clians[255];
	u8 y = 0, lacp = 0;

	sprintf(cliask,"\nEnter a network interface to edit (? to list): ");
	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == '?') {
			printf("\nAvailable Interfaces: \n\n");
			for (index = 0; index < kattach_netdev_shm->index; index++) {
				if ((kattach_netdev_shm->pif[index].mtu == 0) || (kattach_netdev_shm->pif[index].pvid == 0) 
					|| (strlen(kattach_netdev_shm->pif[index].devname) == 0)) continue;
				printf("\t%s\t(%02x:%02x:%02x:%02x:%02x:%02x)\n",kattach_netdev_shm->pif[index].devname, kattach_netdev_shm->pif[index].mac[0],
							kattach_netdev_shm->pif[index].mac[1], kattach_netdev_shm->pif[index].mac[2],
							kattach_netdev_shm->pif[index].mac[3], kattach_netdev_shm->pif[index].mac[4],
							kattach_netdev_shm->pif[index].mac[5]);
			}
		} else if (strlen(clians) == 0) {
		        appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_NETIF;
		        return;
		} else {
			for (index = 0; index < kattach_netdev_shm->index; index++) {
				if ((!strncmp(kattach_netdev_shm->pif[index].devname,clians,strlen(clians))) && 
					(kattach_netdev_shm->pif[index].mtu != 0)) {
					y++;
					break;
				}
			}
		}
		
	}

	sprintf(cliask,"IPv4 address [%lu.%lu.%lu.%lu]: ",(kattach_netdev_shm->pif[index].ip >> 24) & 0xff, (kattach_netdev_shm->pif[index].ip >> 16) & 0xff,
								(kattach_netdev_shm->pif[index].ip >> 8) & 0xff, (kattach_netdev_shm->pif[index].ip) & 0xff);
	y = 0;
	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			sip = kattach_netdev_shm->pif[index].ip;
			y++;
			break;
		}
		sip = appqueue_cli_parseip(clians);
		if (sip != 0) {
			y++;
		} else {
			memset(clians,0,sizeof(clians));
		}
	}
	memset(cliask,0,sizeof(cliask));
	memset(clians,0,sizeof(clians));
	y = 0;
	sprintf(cliask,"CIDR mask [/%u]: ",kattach_netdev_shm->pif[index].mask);
	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			mask = kattach_netdev_shm->pif[index].mask;
			y++;
			break;
		}
		if ((strlen(clians) > 2)) {
			mask = appqueue_cli_parseip(clians);		/* FIXME: need to properly parse non-CIDR netmask */
		} else {
			mask = atoi(clians);
		}
		if (mask != 0) {
			y++;
		} else {
			memset(clians,0,sizeof(clians));
		}
	}

	sprintf(cliask,"Default Gateway IP [%lu.%lu.%lu.%lu]: ",(kattach_netdev_shm->pif[index].gw >> 24) & 0xff, (kattach_netdev_shm->pif[index].gw >> 16) & 0xff,
                                                                (kattach_netdev_shm->pif[index].gw >> 8) & 0xff, (kattach_netdev_shm->pif[index].gw) & 0xff);
	y = 0;
	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			gwip = kattach_netdev_shm->pif[index].gw;
			y++;
			break;
		}
		gwip = appqueue_cli_parseip(clians);
		if (gwip != 0) {
			y++;
		} else {
			memset(clians,0,sizeof(clians));
		}
	}

	sprintf(cliask,"Default VLAN ID [%u]: ",kattach_netdev_shm->pif[index].pvid);
	y = 0;
	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			vlan = kattach_netdev_shm->pif[index].pvid;
			y++;
			break;
		}
		vlan = (u16) atoi(clians);
		if ((vlan > 0) && (vlan <= 4094)) {
			y++;
			break;
		}
	}

	sprintf(cliask,"Interface MTU [%u]: ",kattach_netdev_shm->pif[index].mtu);
	y = 0;
	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			mtu = kattach_netdev_shm->pif[index].mtu;
			y++;
			break;
		}
		mtu = (u32) atol(clians);
		if (mtu > 0) {
			y++;
			break;
		}
	}

	sprintf(cliask,"Enable 802.3ad (LACP) Link Aggregration (y/n): ");
	y = 0;
	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'y') {
			lacp = 1;
			y++;
			break;
		} else if (clians[0] == 'n') {
			lacp = 0;
			y++;
			break;
		}
	}

	appqueue_cli_lock_netdev();

	if (lacp) {
		/* 802.3ad Link Aggregration connection */
		/* kattach uses this to create the bonding device */
		/* kattach automagically enslaves all interfaces with the same pvid */
		/* 802.1Q over an 802.3ad link is handled the same way we handled it for eth0 */

		for (lindex = 0; lindex < kattach_netdev_shm->index; lindex++) {
			if ((kattach_netdev_shm->pif[index].type != KATTACH_LINK_TYPE_PSUEDO) && (kattach_netdev_shm->pif[index].psuedo != KATTACH_LINK_STATUS_LACP_NEW) &&
				(kattach_netdev_shm->pif[index].pvid != vlan)) continue;
			lacpf = 1 + lindex;
			break;
		}

		kattach_netdev_shm->pif[index].ip = 0;
		kattach_netdev_shm->pif[index].mask = 0;
		kattach_netdev_shm->pif[index].gw = vlan;						/* tag the real pvid */
		kattach_netdev_shm->pif[index].pvid = 0x8023;						/* flag the pvid to 802.3ad status */
		kattach_netdev_shm->pif[index].mtu = mtu;
		kattach_netdev_shm->pif[index].status = KATTACH_LINK_STATUS_LACP_NEW;			/* lacp-new status */
		if (lacpf == 0) {
			lindex = kattach_netdev_shm->index;
			kattach_netdev_shm->index++;
		} else {
			lindex = lacpf - 1;								/* existing lacp entry */
		}
		kattach_netdev_shm->pif[index].lacpidx = lindex;
		index = lindex;										/* set this so we pickup the real values on our 802.3ad link */
		kattach_netdev_shm->pif[index].type = KATTACH_LINK_TYPE_PSUEDO;				/* psuedo device  */
		kattach_netdev_shm->pif[index].psuedo = KATTACH_LINK_STATUS_LACP_NEW;			/* 802.3ad */
		sprintf(kattach_netdev_shm->pif[index].devname,"lacp%u",vlan);
	}

	if (lacpf) {
		printf("\n [!] WARNING - Existing LACP configuration found for VLAN %u.\n",vlan);
		if (kattach_netdev_shm->pif[index].mtu != mtu) 
			printf(" [-] WARNING - New MTU = %u. Previous MTU = %u\n",mtu,kattach_netdev_shm->pif[index].mtu);


		if (kattach_netdev_shm->pif[index].ip != sip)
			printf(" [-] WARNING - New IP = %lu.%lu.%lu.%lu. Previous IP = %lu.%lu.%lu.%lu.\n", (sip >> 24) & 0xff, (sip >> 16) & 0xff, (sip >> 8) & 0xff,
							(sip) & 0xff, (kattach_netdev_shm->pif[index].ip >> 24) & 0xff, (kattach_netdev_shm->pif[index].ip >> 16) & 0xff,
							(kattach_netdev_shm->pif[index].ip >> 8) & 0xff, (kattach_netdev_shm->pif[index].ip) & 0xff);

		if (kattach_netdev_shm->pif[index].gw != gwip)
			printf(" [-] WARNING - New GW = %lu.%lu.%lu.%lu. Previous GW = %lu.%lu.%lu.%lu.\n", (gwip >> 24) & 0xff, (gwip >> 16) & 0xff, (gwip >> 8) & 0xff,
							(gwip) & 0xff, (kattach_netdev_shm->pif[index].gw >> 24) & 0xff, (kattach_netdev_shm->pif[index].gw >> 16) & 0xff,
							(kattach_netdev_shm->pif[index].gw >> 8) & 0xff, (kattach_netdev_shm->pif[index].gw) & 0xff);

		if (kattach_netdev_shm->pif[index].mask != mask)
			printf(" [-] WARNING - New CIDR Mask = /%u. Previous CIDR Mask = /%u \n", mask, kattach_netdev_shm->pif[index].mask); 

	}


	kattach_netdev_shm->pif[index].mtu = mtu;
	kattach_netdev_shm->pif[index].pvid = vlan;
	kattach_netdev_shm->pif[index].gw = gwip;
	kattach_netdev_shm->pif[index].ip = sip;	
	kattach_netdev_shm->pif[index].mask = mask;

	appqueue_cli_upd_netdev();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_NETIF;
	return;
}

void
appqueue_cli_mf_net_netif_rmif(void)
{
	u16 index;
	u8 y = 0;
	char cliask[255];
	char clians[255];

        sprintf(cliask,"\nEnter a network interface to remove (? to list): ");
        while (!y) {
                memset(clians,0,sizeof(clians));
                appqueue_cli_askq(cliask,1,clians);
                if (clians[0] == '?') {
                        printf("\nAvailable Interfaces: \n\n");
                        for (index = 0; index < kattach_netdev_shm->index; index++) {  
                                if ((kattach_netdev_shm->pif[index].mtu == 0) || (kattach_netdev_shm->pif[index].pvid == 0)
                                        || (strlen(kattach_netdev_shm->pif[index].devname) == 0)) continue;
                                printf("\t%s\t(%02x:%02x:%02x:%02x:%02x:%02x)\n",kattach_netdev_shm->pif[index].devname, kattach_netdev_shm->pif[index].mac[0],
                                                        kattach_netdev_shm->pif[index].mac[1], kattach_netdev_shm->pif[index].mac[2],
                                                        kattach_netdev_shm->pif[index].mac[3], kattach_netdev_shm->pif[index].mac[4],
                                                        kattach_netdev_shm->pif[index].mac[5]);
                        }
                } else if (strlen(clians) == 0) {
                        appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_NETIF;
                        return;
                } else {
                        for (index = 0; index < kattach_netdev_shm->index; index++) {
                                if ((!strncmp(kattach_netdev_shm->pif[index].devname,clians,strlen(clians))) &&
                                        (kattach_netdev_shm->pif[index].mtu != 0)) {
                                        y++;
                                        break;
                                }
                        }
                }

        }

	
	appqueue_cli_lock_netdev();
	/* delete interface configuration, set status to delete, zero mtu and pvid */
	kattach_netdev_shm->pif[index].mtu = 0;
	kattach_netdev_shm->pif[index].pvid = 0;
	kattach_netdev_shm->pif[index].status = KATTACH_LINK_STATUS_DELETED;
	appqueue_cli_upd_netdev();
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_NETIF;
	return;
}

void
appqueue_cli_mf_net_netif_ifmgr(void)
{
	printf("\n\nNot implemented yet.\n\n");

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_NETIF;
	return;
}

void
appqueue_cli_mf_net_vlan_add(void)
{
	u32 sip = 0, bip = 0, lckcnt = 0;
	u16 mask = 0, vlan = 0, bindex = 0;
	char cliask[255];
	char clians[255];
	char vlanext[255];
	u8 fnd = 0, y = 0, vlocal = 0;

	sprintf(cliask,"\nAdd VLAN (1-%u): ",APPQUEUE_MAX_VLAN);
	while (!vlan) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if ((atoi(clians) > 0) && (atoi(clians) <= APPQUEUE_MAX_VLAN)) {
			vlan = atoi(clians);
		}
	}
	printf("\nChecking for VLAN %u ....", vlan);
	for (bindex = 0; bindex < kattach_vbridge_shm->index; bindex++) {
		if (kattach_vbridge_shm->vbridge[bindex].vlan != vlan) continue;
		fnd++;
		break;
	}
	if (fnd) {
		printf(" *found*\n\n");
		printf("VLAN %u is already configured. Please use the edit vlan command.\n\n",vlan);
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VLAN;
		return;
	} else {
		printf(" *not found*\n");
	}

	printf("\nConfiguring VLAN %u:\n",vlan);
	memset(cliask,0,sizeof(cliask));
	memset(clians,0,sizeof(clians));
	sprintf(cliask,"\nVLAN mode (v)irtual, 802.1(q), (r)outed or (n)at: ");

	while (!y) {
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'v') {
			vlocal = KATTACH_NET_VLAN_LOCAL;		/* VLAN is private to the hypervisor */
			y++;		
		} else if (clians[0] == 'q') {
			vlocal = KATTACH_NET_VLAN_8021Q;		/* VLAN is trunked */
			y++;
		} else if (clians[0] == 'r') {
			vlocal = KATTACH_NET_VLAN_ROUTED;		/* VLAN is routed */
			y++;
		} else if (clians[0] == 'n') {
			vlocal = KATTACH_NET_VLAN_NAT;			/* VLAN is NAT'd */
			y++;
		} else {
			memset(clians,0,sizeof(clians));
		}
	}

	if (vlocal != KATTACH_NET_VLAN_LOCAL) {
		memset(cliask,0,sizeof(cliask));
		/* FIXME: add ? to list */
		sprintf(cliask,"\nEnter the external port for this VLAN (eg. eth0): ");
		y = 0;
		while (!y) {
			appqueue_cli_askq(cliask,1,clians);
			/* FIXME: check port */
			sprintf(vlanext,"%s",clians);
			y++;
		}
	}

	memset(cliask,0,sizeof(cliask));
	memset(clians,0,sizeof(clians));
	y = 0;
	sprintf(cliask,"\nIPv4 subnet to associate with VLAN %u (network address): ",vlan);
	while (!y) {
		appqueue_cli_askq(cliask,1,clians);
		sip = appqueue_cli_parseip(clians);
		if (sip != 0) {
			y++;
		} else {
			memset(clians,0,sizeof(clians));
		}
	}

	memset(cliask,0,sizeof(cliask));
	memset(clians,0,sizeof(clians));
	y = 0;
	sprintf(cliask,"\nVLAN CIDR mask: ");
	while (!y) {
		appqueue_cli_askq(cliask,1,clians);
		if ((strlen(clians) > 2)) {
			mask = appqueue_cli_parseip(clians);		/* FIXME: need to properly parse non-CIDR netmask */
		} else {
			mask = atoi(clians);
		}
		if (mask != 0) {
			y++;
		} else {
			memset(clians,0,sizeof(clians));
		}
	}

	memset(cliask,0,sizeof(cliask));
	memset(clians,0,sizeof(clians));
	y = 0;
	if (vlocal == KATTACH_NET_VLAN_LOCAL) {
		sprintf(cliask,"\nHypervisor VLAN IP address: ");
	} else if (vlocal == KATTACH_NET_VLAN_NAT) {
		sprintf(cliask,"\nPublic NAT IP subnet: ");
	} else {
		sprintf(cliask,"\nExternal Gateway IP address: ");
	}
	while (!y) {
		appqueue_cli_askq(cliask,1,clians);
		bip = appqueue_cli_parseip(clians);
		if (bip != 0) {
			y++;
		} else {
			memset(clians,0,sizeof(clians));
		}
	}

	while (kattach_appqueue->ka_vbridge == (kattach_appqueue->ka_vbridge | CM_MSG_Q_KATTACH_LOCK)) {
		if (lckcnt >= 32767) {
			printf("\n\n [!] FATAL - Unable to obtain lock on VLAN configuration. Try again later.\n");
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VLAN;
			return;
		}
		lckcnt++;
	}

	if (kattach_appqueue->ka_vbridge == (kattach_appqueue->ka_vbridge | CM_MSG_Q_KATTACH_UPDATED)) {
		kattach_appqueue->ka_vbridge ^= CM_MSG_Q_KATTACH_UPDATED;
	}

	kattach_appqueue->ka_vbridge |= CM_MSG_Q_APPQUEUE_LOCK;
	bindex = kattach_vbridge_shm->index;
	kattach_vbridge_shm->index++;
	kattach_vbridge_shm->vbridge[bindex].vlan = vlan;
	kattach_vbridge_shm->vbridge[bindex].vsubnet = sip;
	kattach_vbridge_shm->vbridge[bindex].vbrip = bip;
	kattach_vbridge_shm->vbridge[bindex].vmask = mask;
	kattach_vbridge_shm->vbridge[bindex].vpfree = 0;			/* this is set by kattach */
	kattach_vbridge_shm->vbridge[bindex].vbruse = 0;			/* this is set by kattach */
	kattach_vbridge_shm->vbridge[bindex].state = 0;				/* this is set by kattach */
	kattach_vbridge_shm->vbridge[bindex].vbrlocal = vlocal;
	if (vlocal != KATTACH_NET_VLAN_LOCAL) {
		sprintf(kattach_vbridge_shm->vbridge[bindex].vlanext,"%s",vlanext);
	} else {
		kattach_vbridge_shm->vbridge[bindex].vlanext[0] = '\0';
	}
	kattach_appqueue->ka_vbridge |= CM_MSG_Q_APPQUEUE_UPDATED;
	kattach_appqueue->ka_vbridge ^= CM_MSG_Q_APPQUEUE_LOCK;

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VLAN;
        return;

}

void
appqueue_cli_mf_net_vlan_edit(void)
{
	u32 sip = 0, bip = 0, lckcnt = 0;
	u16 mask = 0, vlan = 0, bindex = 0;
	char cliask[255];
	char clians[255];
	char vlanext[255];
	char iptmp[20];
	u8 fnd = 0, y = 0, vlocal = 0;

	sprintf(cliask,"\nEdit VLAN (1-%u): ",APPQUEUE_MAX_VLAN);
	while (!vlan) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if ((atoi(clians) > 0) && (atoi(clians) <= APPQUEUE_MAX_VLAN)) {
			vlan = atoi(clians);
		}
	}
	printf("\nChecking for VLAN %u ....", vlan);
	for (bindex = 0; bindex < kattach_vbridge_shm->index; bindex++) {
		if (kattach_vbridge_shm->vbridge[bindex].vlan == vlan) {
			fnd = 1;
			break;
		}
	}
	if (!fnd) {
		printf(" *not found*\n\n");
		printf("VLAN %u is not configured. Please use the add vlan command.\n\n",vlan);
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VLAN;
		return;
	} else {
		printf(" *found*\n");
	}

	printf("\nEditing VLAN %u:\n",vlan);
	memset(cliask,0,sizeof(cliask));
	memset(clians,0,sizeof(clians));
	/* FIXME: display previous setting here */
	sprintf(cliask,"\nVLAN mode (v)irtual, 802.1(q), (r)outed or (n)at: ");

	while (!y) {
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			vlocal = kattach_vbridge_shm->vbridge[bindex].vbrlocal;
			y++;
		} else if (clians[0] == 'v') {
			vlocal = KATTACH_NET_VLAN_LOCAL;		/* VLAN is private to the hypervisor */
			y++;		
		} else if (clians[0] == 'q') {
			vlocal = KATTACH_NET_VLAN_8021Q;		/* VLAN is trunked */
			y++;
		} else if (clians[0] == 'r') {
			vlocal = KATTACH_NET_VLAN_ROUTED;		/* VLAN is routed */
			y++;
		} else if (clians[0] == 'n') {
			vlocal = KATTACH_NET_VLAN_NAT;			/* VLAN is NAT'd */
			y++;
		} else {
			memset(clians,0,sizeof(clians));
		}
	}

	if (vlocal != KATTACH_NET_VLAN_LOCAL) {
		memset(cliask,0,sizeof(cliask));
		sprintf(cliask,"\nEnter the external port for this VLAN (eg. eth0) [%s]: ",kattach_vbridge_shm->vbridge[bindex].vlanext);
		y = 0;
		while (!y) {
			appqueue_cli_askq(cliask,1,clians);
			if (strlen(clians) == 0) {
				sprintf(vlanext,"%s",kattach_vbridge_shm->vbridge[bindex].vlanext);
				y++;
			} else {
				/* FIXME: check port */
				sprintf(vlanext,"%s",clians);
				y++;
			}
		}
	}

	memset(cliask,0,sizeof(cliask));
	memset(clians,0,sizeof(clians));
	y = 0;
	sprintf(iptmp,"%lu.%lu.%lu.%lu",((kattach_vbridge_shm->vbridge[bindex].vsubnet >> 24) & 0xff),
					((kattach_vbridge_shm->vbridge[bindex].vsubnet >> 16) & 0xff),
					((kattach_vbridge_shm->vbridge[bindex].vsubnet >> 8) & 0xff),
					((kattach_vbridge_shm->vbridge[bindex].vsubnet) & 0xff));
	sprintf(cliask,"\nIPv4 subnet to associate with VLAN %u (network address) [%s]: ",vlan,iptmp);
	while (!y) {
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			sip = kattach_vbridge_shm->vbridge[bindex].vsubnet;
			y++;
		} else {
			sip = appqueue_cli_parseip(clians);
			if (sip != 0) {
				y++;
			} else {
				memset(clians,0,sizeof(clians));
			}
		}
	}

	memset(cliask,0,sizeof(cliask));
	memset(clians,0,sizeof(clians));
	y = 0;
	sprintf(cliask,"\nVLAN CIDR mask [%u]: ",kattach_vbridge_shm->vbridge[bindex].vmask);
	while (!y) {
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			mask = kattach_vbridge_shm->vbridge[bindex].vmask;
			y++;
		} else {
			if ((strlen(clians) > 2)) {
				mask = appqueue_cli_parseip(clians);		/* FIXME: need to properly parse non-CIDR netmask */
			} else {
				mask = atoi(clians);
			}
			if (mask != 0) {
				y++;
			} else {
				memset(clians,0,sizeof(clians));
			}
		}
	}

	memset(cliask,0,sizeof(cliask));
	memset(clians,0,sizeof(clians));
	y = 0;
	sprintf(iptmp,"%lu.%lu.%lu.%lu",((kattach_vbridge_shm->vbridge[bindex].vbrip >> 24) & 0xff),
					((kattach_vbridge_shm->vbridge[bindex].vbrip >> 16) & 0xff),
					((kattach_vbridge_shm->vbridge[bindex].vbrip >> 8) & 0xff),
					((kattach_vbridge_shm->vbridge[bindex].vbrip) & 0xff));
        if (vlocal == KATTACH_NET_VLAN_LOCAL) {
                sprintf(cliask,"\nHypervisor VLAN IP address [%s]: ",iptmp);
        } else if (vlocal == KATTACH_NET_VLAN_NAT) {
                sprintf(cliask,"\nPublic NAT IP subnet [%s]: ",iptmp);
        } else {
                sprintf(cliask,"\nExternal Gateway IP address [%s]: ",iptmp);
        }
	while (!y) {
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			bip = kattach_vbridge_shm->vbridge[bindex].vbrip;
			y++;
		} else {
			bip = appqueue_cli_parseip(clians);
			if (bip != 0) {
				y++;
			} else {
				memset(clians,0,sizeof(clians));
			}
		}
	}

	while (kattach_appqueue->ka_vbridge == (kattach_appqueue->ka_vbridge | CM_MSG_Q_KATTACH_LOCK)) {
		if (lckcnt >= 32767) {
			printf("\n\n [!] FATAL - Unable to obtain lock on VLAN configuration. Try again later.\n");
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VLAN;
			return;
		}
		lckcnt++;
	}

	if (kattach_appqueue->ka_vbridge == (kattach_appqueue->ka_vbridge | CM_MSG_Q_KATTACH_UPDATED)) {
		kattach_appqueue->ka_vbridge ^= CM_MSG_Q_KATTACH_UPDATED;
	}

	kattach_appqueue->ka_vbridge |= CM_MSG_Q_APPQUEUE_LOCK;
	kattach_vbridge_shm->vbridge[bindex].vlan = vlan;
	kattach_vbridge_shm->vbridge[bindex].vsubnet = sip;
	kattach_vbridge_shm->vbridge[bindex].vbrip = bip;
	kattach_vbridge_shm->vbridge[bindex].vmask = mask;
	kattach_vbridge_shm->vbridge[bindex].vpfree = 0;			/* this is set by kattach */
	kattach_vbridge_shm->vbridge[bindex].vbruse = 0;			/* this is set by kattach */
	kattach_vbridge_shm->vbridge[bindex].state = 0;				/* this is set by kattach */
	kattach_vbridge_shm->vbridge[bindex].vbrlocal = vlocal;
	if (vlocal != KATTACH_NET_VLAN_LOCAL) {
		sprintf(kattach_vbridge_shm->vbridge[bindex].vlanext,"%s",vlanext);
	} else {
		kattach_vbridge_shm->vbridge[bindex].vlanext[0] = '\0';
	}
	kattach_appqueue->ka_vbridge |= CM_MSG_Q_APPQUEUE_UPDATED;
	kattach_appqueue->ka_vbridge ^= CM_MSG_Q_APPQUEUE_LOCK;

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VLAN;
        return;

}

void
appqueue_cli_mf_net_vlan_delete(void)
{
	u16 vlan = 0, bindex = 0;
	char cliask[255];
	char clians[255];
	u8 fnd = 0;

	sprintf(cliask,"\nDelete VLAN (1-%u): ",APPQUEUE_MAX_VLAN);
	while (!vlan) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if ((atoi(clians) > 0) && (atoi(clians) <= APPQUEUE_MAX_VLAN)) {
			vlan = atoi(clians);
		}
	}
	printf("\nChecking for VLAN %u ....", vlan);
	for (bindex = 0; bindex < kattach_vbridge_shm->index; bindex++) {
		if (kattach_vbridge_shm->vbridge[bindex].vlan == vlan) {
			fnd = 1;
			break;
		}
	}
	if (!fnd) {
		printf(" *not found*\n\n");
		printf("VLAN %u is not configured.\n\n",vlan);
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VLAN;
		return;
	} else {
		printf(" *found*\n");
	}

	appqueue_cli_lock_vbridge();
	printf("\nDeleting VLAN %u:\n",vlan);
	kattach_vbridge_shm->vbridge[bindex].state = 0x03;
	kattach_vbridge_shm->vbridge[bindex].vsubnet = 0;
	kattach_vbridge_shm->vbridge[bindex].vbrip = 0;
	kattach_vbridge_shm->vbridge[bindex].vlan = 0;
	kattach_vbridge_shm->vbridge[bindex].vmask = 0;
	appqueue_cli_upd_vbridge();
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VLAN;
	return;
}

void
appqueue_cli_mf_net_vlan_list(void)
{
	u32 index = 0;
	char vsub[20];

	printf("\n");
	printf("VLAN\t \t  Subnet\t CIDR \t Free \t Used \t    Type \t External \n");
	printf("----\t----------------\t ---- \t ---- \t ---- \t ------- \t -------- \n\n");

	for (index = 0; index < kattach_vbridge_shm->index; index++) {
		if (kattach_vbridge_shm->vbridge[index].vlan == 0) continue;
		memset(vsub,0,sizeof(vsub));
		sprintf(vsub,"%lu.%lu.%lu.%lu",((kattach_vbridge_shm->vbridge[index].vsubnet >> 24) & 0xff),
							((kattach_vbridge_shm->vbridge[index].vsubnet >> 16) & 0xff),
							((kattach_vbridge_shm->vbridge[index].vsubnet >> 8) & 0xff),
							((kattach_vbridge_shm->vbridge[index].vsubnet) & 0xff));
		printf("%4u\t%16s  \t  /%-2u \t %4u \t %4u \t %7s \t %8s \n",kattach_vbridge_shm->vbridge[index].vlan, vsub,
							kattach_vbridge_shm->vbridge[index].vmask,
							kattach_vbridge_shm->vbridge[index].vpfree,
							kattach_vbridge_shm->vbridge[index].vbruse,
							(kattach_vbridge_shm->vbridge[index].vbrlocal == KATTACH_NET_VLAN_LOCAL) ? "private" :
							(kattach_vbridge_shm->vbridge[index].vbrlocal == KATTACH_NET_VLAN_8021Q) ? " 802.1q" :
							(kattach_vbridge_shm->vbridge[index].vbrlocal == KATTACH_NET_VLAN_ROUTED) ? " routed" :
							(kattach_vbridge_shm->vbridge[index].vbrlocal == KATTACH_NET_VLAN_NAT) ? "    nat" : "    n/a",
							(kattach_vbridge_shm->vbridge[index].vbrlocal == KATTACH_NET_VLAN_LOCAL) ? "n/a" : kattach_vbridge_shm->vbridge[index].vlanext);
	}
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VLAN;
	return;
}

void
appqueue_cli_mf_sys_hostname(void)
{
	char cliask[255];
	char clians[255];
	int y = 0;

	sprintf(cliask,"\nEnter Hypervisor hostname: ");

	appqueue_cli_lock_cfg();

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) <= sizeof(kattach_cfg_shm->hostname)) {
			sprintf(kattach_cfg_shm->hostname,"%s",clians);
			y++;
		}
	}

	if (kattach_cfg_shm->hostname[0] != '\0') {
		sprintf(appqueue_prompt,"[%s | ", kattach_cfg_shm->hostname);
	} else {
		sprintf(appqueue_prompt,"[%s | ","kaos-new");
	}

	appqueue_cli_upd_cfg();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS;
	return;
}

void
appqueue_cli_mf_sys_domain(void)
{
	char cliask[255];
	char clians[255];
	int y = 0;

	sprintf(cliask,"\nEnter default domainname: ");

	appqueue_cli_lock_cfg();

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) <= sizeof(kattach_cfg_shm->domain)) {
			sprintf(kattach_cfg_shm->domain,"%s",clians);
			y++;
		}
	}

	appqueue_cli_upd_cfg();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS;
	return;
}

void
appqueue_cli_mf_sys_dns(void)
{
	u32 namesrv = 0;
	char cliask[255];
	char clians[255];
	int y = 0, d = 0, x = 0;

	appqueue_cli_lock_cfg();

	for (x = 0; x <= 5; x++) {
		if (kattach_cfg_shm->dns_ip[d] == 0) {
			sprintf(cliask,"\nDNS Server %d IP: ",(d+1));
		} else {
			sprintf(cliask,"\nDNS Server %d IP [%lu.%lu.%lu.%lu]: ", (d+1),((kattach_cfg_shm->dns_ip[d] >> 24) & 0xff),
										((kattach_cfg_shm->dns_ip[d] >> 16) & 0xff),
										((kattach_cfg_shm->dns_ip[d] >> 8) & 0xff),
										((kattach_cfg_shm->dns_ip[d]) & 0xff));
		}
		while (!y) {
			memset(clians,0,sizeof(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (strlen(clians) == 0) {
				y++;
			} else {
				namesrv = appqueue_cli_parseip(clians);
				if (namesrv) {
					kattach_cfg_shm->dns_ip[d] = namesrv;
					y++;
				}
			}
		}
		d++;
		y = 0;
	}

	appqueue_cli_upd_cfg();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS;
	return;
}

void
appqueue_cli_mf_sys_disk_bootdisk(void)
{
	char cliask[255];
	char clians[255];
	int y = 0;

	sprintf(cliask,"\nHypervisor boot device (without /dev): ");

	appqueue_cli_lock_inst();

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) <= sizeof(kattach_install_shm->diskboot)) {
			sprintf(kattach_install_shm->diskboot,"%s",clians);
			y++;
		}
	}

	appqueue_cli_upd_inst();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_DISK;
	return;

}

void
appqueue_cli_mf_sys_disk_swapdisk(void)
{
	char cliask[255];
	char clians[255];
	int y = 0;

	sprintf(cliask,"\nHypervisor swap device (without /dev): ");

	appqueue_cli_lock_inst();

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) <= sizeof(kattach_install_shm->diskswap)) {
			sprintf(kattach_install_shm->diskswap,"%s",clians);
			y++;
		}
	}

	appqueue_cli_upd_inst();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_DISK;
	return;

}

void
appqueue_cli_mf_sys_disk_appqdisk(void)
{
	char cliask[255];
	char clians[255];
	int y = 0;

	sprintf(cliask,"\nAppQueue storage device (without /dev): ");

	appqueue_cli_lock_inst();

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) <= sizeof(kattach_install_shm->diskappq)) {
			sprintf(kattach_install_shm->diskappq,"%s",clians);
			y++;
		}
	}

	appqueue_cli_upd_inst();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_DISK;
	return;

}

void
appqueue_cli_mf_sys_disk_datadisk(void)
{
	char cliask[255];
	char clians[255];
	int y = 0;

	sprintf(cliask,"\nContent storage device (without /dev): ");

	appqueue_cli_lock_inst();

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) <= sizeof(kattach_install_shm->diskdata)) {
			sprintf(kattach_install_shm->diskdata,"%s",clians);
			y++;
		}
	}

	appqueue_cli_upd_inst();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_DISK;
	return;
}

void
appqueue_cli_mf_sys_disk_fdisk(void)
{
	char cliask[255];
	char clians[255];
	int y = 0;
	u8 rc = 0;

	sprintf(cliask,"\nEnter storage device to partition (without /dev): ");

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			y++;
			break;
		} else {
			sprintf(cliask,"%s /dev/%s",APPQUEUE_FDISK,clians);
			rc = appqueue_cli_exec(cliask);
			y++;
			break;
		}
	}
	printf("\n");

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_DISK;
	return;
}

void
appqueue_cli_mf_maint_shell(void)
{
	u8 rc = 0;

	printf("\n");
	printf("Entering Linux Shell.\n Type 'exit' to return to CLI.\n\n");

	rc = appqueue_cli_exec(APPQUEUE_SHELL);

	if (rc != RC_OK) {
		printf("\nUnable to execute shell - %s\n\n",APPQUEUE_SHELL);
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAINT;
	return;
}

void
appqueue_cli_mf_maint_tsdump(void)
{
	u16 index = 0;
	u32 vindex = 0;
	u8 vpindex = 0;

	printf("\n");
	printf("AppQueue Tech Support Dump - version %s\n",APPQUEUE_VERSION);
	printf("%s\n\n",APPQUEUE_MENU_BARXL);
	printf("Visit %s for further assistance\n\n",APPQUEUE_LINK);
	printf("%s\n\n",APPQUEUE_MENU_BARXL);
	printf("Statistics:\n\n");
	printf("         Devices : \t %5lu / %5u. \n", kattach_devices_shm->index, KATTACH_MAX_DEV);
	printf("     Net Devices : \t %5u / %5u. \n", kattach_netdev_shm->index, KATTACH_MAX_IFDEV);
	printf("     VM Sessions : \t %5lu / %5u. \n", kattach_vmst_shm->index, KATTACH_MAX_VMSESSIONS);
	printf("       VM Images : \t %5lu / %5u. \n", kattach_vmimages_shm->index, KATTACH_MAX_VMIMAGES);
	printf("     App Modules : \t %5lu / %5u. \n", kattach_appmods_shm->index, KATTACH_MAX_APPMODULES);
	printf(" App Config Grps : \t %5lu / %5u. \n", kattach_cfggrp_shm->index, KATTACH_MAX_CFGGRP);
	printf(" Virtual Bridges : \t %5u / %5u. \n", kattach_vbridge_shm->index, KATTACH_MAX_VBRIDGES);
	printf("   Virtual Ports : \t %5u / %5u. \n\n", kattach_vmports_shm->index, KATTACH_MAX_VMPORTS);

	printf("          Config : \t %s\n", (kattach_cfg_shm->mode <= 0x08) ? "hypervisor" : "not configured");
	printf("     State Table : \t %03x | %03x | %03x | %03x | %03x | %03x | %03x | %03x\n\n",
					kattach_appqueue->ka_dev,kattach_appqueue->ka_cfg,kattach_appqueue->ka_inst,kattach_appqueue->ka_vmst,
					kattach_appqueue->ka_vbridge, kattach_appqueue->ka_vmimages,kattach_appqueue->ka_netdev,kattach_appqueue->ak_update);
	printf("%s\n\n",APPQUEUE_MENU_BARXL);
	printf("Network Interface Table:\n\n");
	printf("idx               dev       ip       gw  m pvid   li    mtu   t   p   s               mac\n");	
	for (index = 0; index < kattach_netdev_shm->index; index++) {
		printf("%3u: %16s:%08lx:%08lx:%2u:%4u:%4u:%6u:%3u:%3u:%3u:%02x-%02x-%02x-%02x-%02x-%02x\n",index,
				kattach_netdev_shm->pif[index].devname, kattach_netdev_shm->pif[index].ip, kattach_netdev_shm->pif[index].gw,
				kattach_netdev_shm->pif[index].mask, kattach_netdev_shm->pif[index].pvid, kattach_netdev_shm->pif[index].lacpidx,
				kattach_netdev_shm->pif[index].mtu, kattach_netdev_shm->pif[index].type, kattach_netdev_shm->pif[index].psuedo,
				kattach_netdev_shm->pif[index].status,kattach_netdev_shm->pif[index].mac[0],kattach_netdev_shm->pif[index].mac[1],
				kattach_netdev_shm->pif[index].mac[2],kattach_netdev_shm->pif[index].mac[3],kattach_netdev_shm->pif[index].mac[4],
				kattach_netdev_shm->pif[index].mac[5]);
	}
	printf("%s\n\n",APPQUEUE_MENU_BARXL);
	printf("VM Session Table:\n\n");
	printf("  idx                   vm  st cpu   mem    pid  vo   vi  vp: ports\n");
	for (vindex = 0; vindex < kattach_vmst_shm->index; vindex++) { 
		printf("%4lu: %20s:%3x:%3u:%5u:%6u:%3u:%4lu:%3u: ",vindex,kattach_vmst_shm->vmsess[vindex].vmname,kattach_vmst_shm->vmsess[vindex].vmstatus,
								 kattach_vmst_shm->vmsess[vindex].vcpu,kattach_vmst_shm->vmsess[vindex].vmem,kattach_vmst_shm->vmsess[vindex].vpid,
								 kattach_vmst_shm->vmsess[vindex].vmoper, kattach_vmst_shm->vmsess[vindex].vmimage,
								 kattach_vmst_shm->vmsess[vindex].vmpidx);

		for (vpindex = 0; vpindex < kattach_vmst_shm->vmsess[vindex].vmpidx; vpindex++) {
			printf(" %2u ",kattach_vmst_shm->vmsess[vindex].vmport[vpindex]);
		}

		printf("\n");
	}

	printf("%s\n\n",APPQUEUE_MENU_BARXL);
	printf("VM Image Table:\n\n");
	printf("  idx                 name  ch  ac  ap  appidx:cfggrp\n");
	for (vindex = 0; vindex < kattach_vmimages_shm->index; vindex++) { 
		printf("%4lu: %20s:%3x:%3x:%3u: ", vindex, kattach_vmimages_shm->vmimage[vindex].vminame,kattach_vmimages_shm->vmimage[vindex].changed,
							 kattach_vmimages_shm->vmimage[vindex].active, kattach_vmimages_shm->vmimage[vindex].appi);
		for (vpindex = 0; vpindex < kattach_vmimages_shm->vmimage[vindex].appi; vpindex++) {
			printf(" %5lu:%5lu ",kattach_vmimages_shm->vmimage[vindex].appindex[vpindex], kattach_vmimages_shm->vmimage[vindex].cfggrp[vpindex]);
		}
		printf("\n");
	}

	printf("%s\n\n",APPQUEUE_MENU_BARXL);

	printf("Virtual Port Table:\n\n");
	printf("  idx       ip   vmst  vbr              vmac\n");
	for (index = 0; index < kattach_vmports_shm->index; index++) {
		printf("%4u:%8lx:%6lu:%4u:%02x-%02x-%02x-%02x-%02x-%02x\n", index, kattach_vmports_shm->vmports[index].vmpip, kattach_vmports_shm->vmports[index].vmst,
									kattach_vmports_shm->vmports[index].vbridge, kattach_vmports_shm->vmports[index].vmac[0],
									kattach_vmports_shm->vmports[index].vmac[1], kattach_vmports_shm->vmports[index].vmac[2],
									kattach_vmports_shm->vmports[index].vmac[3], kattach_vmports_shm->vmports[index].vmac[4],
									kattach_vmports_shm->vmports[index].vmac[5]);
	}
	printf("\n");
	printf("%s\n\n",APPQUEUE_MENU_BARXL);

	printf("App Module Table:\n\n");
	printf(" idx st tr lt cfgd dply         name\n");
	for (vindex = 0; vindex < kattach_appmods_shm->index; vindex++) {
		printf("%4lu:%2x:%2x:%2u:%4lu:%4lu:%12s\n",vindex,kattach_appmods_shm->appmodules[vindex].state,kattach_appmods_shm->appmodules[vindex].srctree,
							kattach_appmods_shm->appmodules[vindex].latest,kattach_appmods_shm->appmodules[vindex].config,
							kattach_appmods_shm->appmodules[vindex].deployed,kattach_appmods_shm->appmodules[vindex].name);
	}
	printf("\n");
	printf("%s\n\n",APPQUEUE_MENU_BARXL);

	

	/* FIXME: add remaining tables to tsdump */
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAINT;
	return;
}

void
appqueue_cli_mf_maint_dbappq(void)
{
	if (appqueue_cli_user_auth < APPQUEUE_CLI_AUTH_DEV) {
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAINT;
		return;
	}

	/* FIXME -- debug stuff */

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAINT;
	return;
}

void
appqueue_cli_mf_maint_dbkaos(void)
{
	if (appqueue_cli_user_auth < APPQUEUE_CLI_AUTH_DEV) {
		appqueue_po = 0;
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAINT;
		return;
	}

	/* FIXME -- debug stuff */

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAINT;
	return;
}

void
appqueue_cli_mf_maint_dbvmsess(void)
{
	if (appqueue_cli_user_auth < APPQUEUE_CLI_AUTH_DEV) {
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAINT;
		return;
	}

	/* FIXME -- debug stuff */

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_MAINT;
	return;
}

void
appqueue_cli_mf_boot_status(void)
{
	struct utsname aq_bootk;
	
	uname(&aq_bootk);

	printf("\n");
	if ((kattach_cfg_shm->mode != 0) && (kattach_cfg_shm->mode <= 0x02)) {
		printf("Hypervisor booting from slot %u.\n",kattach_cfg_shm->mode);
	} else if ((kattach_cfg_shm->mode <= 0x13) && (kattach_cfg_shm->mode >= 0x0c)) {
		printf("Hypervisor booting from slot %u.\n",(kattach_cfg_shm->mode - 10));
	} else {
		printf("Hypervisor booting from an external source.\n");
	}

	printf(" boot kernel: %s version %s\n",aq_bootk.sysname, aq_bootk.release);

	if (kattach_install_shm->diskboot[0] != '\0') {
		printf(" boot device: /dev/%s\n\n",kattach_install_shm->diskboot);
	}

	if (kattach_cfg_shm->dhcp) {
		printf("Hypervisor using DHCP on %s (%02x:%02x:%02x:%02x:%02x:%02x).\n",kattach_cfg_shm->netdev,kattach_cfg_shm->mac[0],kattach_cfg_shm->mac[1],
											kattach_cfg_shm->mac[2],kattach_cfg_shm->mac[3],kattach_cfg_shm->mac[4],
											kattach_cfg_shm->mac[5]);
	} else {
		printf("Hypervisor using %lu.%lu.%lu.%lu/%u on %s (%02x:%02x:%02x:%02x:%02x:%02x).\n",(kattach_cfg_shm->ip >> 24) & 0xff, (kattach_cfg_shm->ip >> 16) & 0xff,
											(kattach_cfg_shm->ip >> 8) & 0xff,(kattach_cfg_shm->ip) & 0xff,kattach_cfg_shm->slash,
											kattach_cfg_shm->netdev,kattach_cfg_shm->mac[0],kattach_cfg_shm->mac[1],
											kattach_cfg_shm->mac[2],kattach_cfg_shm->mac[3],kattach_cfg_shm->mac[4],
											kattach_cfg_shm->mac[5]);
		printf("Hypervisor using %lu.%lu.%lu.%lu as default gateway.\n",(kattach_cfg_shm->gw >> 24) & 0xff, (kattach_cfg_shm->gw >> 16) & 0xff,
                                                                                        (kattach_cfg_shm->gw >> 8) & 0xff,(kattach_cfg_shm->gw) & 0xff);
	}

	if (kattach_cfg_shm->pid_dhcpd == 0) {
		printf("\n\nPlease run 'setup' to perform initial configuration.\n");
		printf("Hypervisor **SHOULD** be rebooted to enter production mode.\n");
		printf("DHCPd is **NOT** running. Configure VLANs to start DHCPd.\n\n");
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_BOOT;
	return;

}

void
appqueue_cli_mf_boot_upgrade(void)
{
	/* this should query www.appqueue.info and compare to utsname.release */
}

void
appqueue_cli_mf_boot_reboot(void)
{
	char cliask[255];
	char clians[255];
	int y = 0;

	sprintf(cliask,"\nReboot Hypervisor (y/n): ");

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'y') {
			appqueue_cli_setflags(APPQUEUE_LCK_REBOOT);
			printf("\n\n [!] Hypervisor Reboot Initiated. Please wait. \n\n");
			appqueue_exit = 1;
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_BOOT;
			return;
		} else if (clians[0] == 'n') {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_BOOT;
			return;
		}
	}
}

void
appqueue_cli_mf_boot_gtimg(void)
{
	char cliask[255];
	char clians[255];
	int y = 0;
	u8 slot = 0, rc = 0, mode = 0;
	curl_version_info_data *aq_curlinfo;
	const char * const *aq_curl_proto;

	if (kattach_cfg_shm->mode != 0x0c) {
		slot = 2;
		mode = 1;
	} else {
	   	slot = 1;
		mode = 2;
	}

	aq_curlinfo = curl_version_info(CURLVERSION_NOW);

	printf("\n");
	printf("This Hypervisor is currently booting from slot %u.\n",mode);
	printf("Loading an invalid image onto the Hypervisor may render it unbootable!\n");
	printf("Make sure there is a good image in the alternate slot\n\n");
	sprintf(cliask,"Which image slot do you wish to update (1 or 2) : ");
	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			printf("\nAborted!\n\n");
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_BOOT;
			return;
		} else if ((clians[0] == '1') || (clians[0] == '2')) {
			slot = (u8) atoi(clians);
			y++;
			break;
		}
	}
	printf("\n\n");
	printf("Supported Protocols: ");
	for (aq_curl_proto=aq_curlinfo->protocols; *aq_curl_proto; ++aq_curl_proto) {
		printf("%s ",*aq_curl_proto);
	}
	printf("\n\n");
	sprintf(cliask,"Enter the URL to download hvImage.00%u: ",slot);
	y = 0;
	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,6,clians);
		if (strlen(clians) == 0) {
			printf("\nAborted!\n\n");
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_BOOT;
			return;
		} else {
			rc = appqueue_install_gethv(clians, slot);
			if (rc == RC_OK) {
				y++;
			} else {
				printf("\nUnable to install hvImage.00%u. Please try again.\n",slot);
			}
		}
	}

	appqueue_cli_lock_cfg();
	if (slot == 2) {
		kattach_cfg_shm->mode = 0xfc;
	} else {
		kattach_cfg_shm->mode = 0xf1;
	}
	appqueue_cli_upd_cfg();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_BOOT;
	return;
}

void
appqueue_cli_mf_boot_factory(void)
{
	char cliask[255];
	char clians[255];
	int y = 0;

	printf("\n\n");
	sprintf(cliask,"Confirm factory reset (y/n) ?");

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'n') {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_BOOT;
			return;
		} else if (clians[0] == 'y') {
			appqueue_cli_lock_cfg();
			kattach_cfg_shm->mode = 0xef;
			appqueue_cli_upd_cfg();
			y++;
			break;
		}
	}
	printf("\n\nReboot Hypervisor to complete factory reset!\n\n");

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_BOOT;
	return;
}


u32
appqueue_cli_parseip(char *dotted)
{
	unsigned int i = 0, y = 0, x = 24;
	char iptmp[20];
	char ch = '\0';
	unsigned long iptr = 0;

	/* special case for DHCP, if user enters 0 we use DHCP */
	if ((strlen(dotted) == 1) && (dotted[0] == '0')) {
		iptr = 0xfffefdfc;
		return(iptr);
	}

	if ((dotted[0] == '/') || (strlen(dotted) <= 3)) {
		if (dotted[0] == '/') {
			sprintf(iptmp,"%c%c",dotted[1],dotted[2]);
		} else {
			sprintf(iptmp,"%c%c",dotted[0],dotted[1]);
		}
		iptr = atol(iptmp);
		return(iptr);
	}

	while (i <= strlen(dotted)) {
		ch = dotted[i];
		if ((ch == '.') || i == strlen(dotted)) {
			iptmp[y] = '\0';
			iptr += ((atoi(iptmp) << x) & 0xffffffff);
			x -= 8;
			y = 0;
		} else {
			if (y <= strlen(iptmp)) {
				iptmp[y] = ch;
				y++;
			}
		}
		i++;
	}
	return(iptr);
}

void
appqueue_cli_setflags(u8 flag)
{

	switch(flag) {
		case APPQUEUE_LCK_DEV:
			if (kattach_appqueue->ka_dev != (kattach_appqueue->ka_dev | CM_MSG_Q_APPQUEUE_UPDATED)) {
				kattach_appqueue->ka_dev |= CM_MSG_Q_APPQUEUE_UPDATED;
			}
			break;

		case APPQUEUE_LCK_CFG:
			if (kattach_appqueue->ka_cfg != (kattach_appqueue->ka_cfg | CM_MSG_Q_APPQUEUE_UPDATED)) {
				kattach_appqueue->ka_cfg |= CM_MSG_Q_APPQUEUE_UPDATED;
			}
			break;

		case APPQUEUE_LCK_INST:
			if (kattach_appqueue->ka_inst != (kattach_appqueue->ka_inst | CM_MSG_Q_APPQUEUE_UPDATED)) {
				kattach_appqueue->ka_inst |= CM_MSG_Q_APPQUEUE_UPDATED;
			}
			break;

		case APPQUEUE_LCK_VMST:
			if (kattach_appqueue->ka_vmst != (kattach_appqueue->ka_vmst | CM_MSG_Q_APPQUEUE_UPDATED)) {
				kattach_appqueue->ka_vmst |= CM_MSG_Q_APPQUEUE_UPDATED;
			}
			break;

		case APPQUEUE_LCK_VMPORTS:
			if (kattach_appqueue->ka_vmports != (kattach_appqueue->ka_vmports | CM_MSG_Q_APPQUEUE_UPDATED)) {
				kattach_appqueue->ka_vmports |= CM_MSG_Q_APPQUEUE_UPDATED;
			}
			break;

		case APPQUEUE_LCK_VBRIDGE:
			if (kattach_appqueue->ka_vbridge != (kattach_appqueue->ka_vbridge | CM_MSG_Q_APPQUEUE_UPDATED)) {
				kattach_appqueue->ka_vbridge |= CM_MSG_Q_APPQUEUE_UPDATED;
			}
			break;

		case APPQUEUE_LCK_VMIMAGES:
			if (kattach_appqueue->ka_vmimages != (kattach_appqueue->ka_vmimages | CM_MSG_Q_APPQUEUE_UPDATED)) {
				kattach_appqueue->ka_vmimages |= CM_MSG_Q_APPQUEUE_UPDATED;
			}
			break;

		case APPQUEUE_LCK_APPMODULES:
			if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_MODULE)) {
				kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_MODULE;
			}
			break;

		case APPQUEUE_LCK_REBOOT:
			if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_REBOOT)) {
				kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_REBOOT;
			}
			break;

		default:
			return;
	}

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	return;
}

u8 
appqueue_cli_exec(char *cmd)
{  
        int status;                             /* child status */
        int fpid;                               /* forked process id */
        char pcmd[256];                         /* process command */
        char *pargv[4];                         /* process argv */

        pargv[0] = APPQUEUE_SHELL;              /* set the shell */
        pargv[1] = "-c";                        /* set the command option */

        sprintf(pcmd,"%s",cmd);  

        pargv[2] = pcmd;
        pargv[3] = NULL;

        fpid = fork();                          /* fork the command */

        if (fpid < 0) {
            return RC_FAIL;
        }

        if (fpid == 0) {
            execve(APPQUEUE_SHELL,pargv,NULL);
            exit(1);
        }

        while (wait(&status) != fpid);

        return RC_OK;           /* we should probably handle errors here */

}

u8 
appqueue_cli_mod(char *cmd, char *kargv)
{  
        int fpid;                               /* forked process id */
	int status;
        char *pargv[128];
        char *buf;
        char c = '\0';
        int i = 0, x = 0, n = 1;

        pargv[0] = cmd;
        buf = (char *) malloc(strlen(kargv)+1);
        for (i = 0; i <= strlen(kargv); i++) {
        	c = kargv[i];
                if ((c != ' ') && (c != '\0')) {
                	buf[x] = c;
                        x++;
                } else {
                        if (x == 0) continue;
                        buf[x] = '\0';
                        pargv[n] = (char *) malloc(strlen(buf)+1);
                        sprintf(pargv[n],"%s",buf);
                        n++;
                        x = 0;
                        memset(buf,0,sizeof(buf));
                }
	}
        free(buf);
        pargv[n] = NULL;

        fpid = fork();                          /* fork the command */

        if (fpid < 0) {
            printf("\n\n*** DEBUG *** Fork failed %s %s\n\n",cmd,kargv);
            return RC_FAIL;
        }

        if (fpid == 0) {
        	x = execve(cmd,pargv,NULL);
        	printf("\n\n*** DEBUG *** Exec failed %s %s\n\n",cmd,kargv);
        	for (i = 0; i <= n; i++) {
        		printf(" arg %u is %s\n",i,pargv[i]);
        	}
        	exit(1);
        }

        while (wait(&status) != fpid);

        return RC_OK;           /* we should probably handle errors here */
}

void
appqueue_cli_lock_dev(void)
{
	kattach_appqueue->ka_dev |= CM_MSG_Q_APPQUEUE_LOCK;

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_lock_cfg(void)
{
	kattach_appqueue->ka_cfg |= CM_MSG_Q_APPQUEUE_LOCK;

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_unlock_cfg(void)
{
	if (kattach_appqueue->ka_cfg == (kattach_appqueue->ka_cfg | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ka_cfg ^= CM_MSG_Q_APPQUEUE_LOCK;
	}

	if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update ^= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_lock_inst(void)
{
	kattach_appqueue->ka_inst |= CM_MSG_Q_APPQUEUE_LOCK;

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_lock_vmst(void)
{
	kattach_appqueue->ka_vmst |= CM_MSG_Q_APPQUEUE_LOCK;

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_lock_vmports(void)
{
	kattach_appqueue->ka_vmports |= CM_MSG_Q_APPQUEUE_LOCK;

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_lock_vbridge(void)
{
	kattach_appqueue->ka_vbridge |= CM_MSG_Q_APPQUEUE_LOCK;

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_lock_vmimages(void)
{
	kattach_appqueue->ka_vmimages |= CM_MSG_Q_APPQUEUE_LOCK;

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_lock_netdev(void)
{
	kattach_appqueue->ka_netdev |= CM_MSG_Q_APPQUEUE_LOCK;

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_lock_vns(void)
{
	kattach_appqueue->ka_vns |= CM_MSG_Q_APPQUEUE_LOCK;

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_lock_cfggrp(void)
{
	kattach_appqueue->ka_cfggrp |= CM_MSG_Q_APPQUEUE_LOCK;

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_lock_fw(void)
{
	kattach_appqueue->ka_fw |= CM_MSG_Q_APPQUEUE_LOCK;

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}



void
appqueue_cli_upd_dev(void)
{
	if (kattach_appqueue->ka_dev != (kattach_appqueue->ka_dev | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ka_dev |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	/* remove locks */
	if (kattach_appqueue->ka_dev == (kattach_appqueue->ka_dev | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ka_dev ^= CM_MSG_Q_APPQUEUE_LOCK;
	}

	if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update ^= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_upd_cfg(void)
{
	if (kattach_appqueue->ka_cfg != (kattach_appqueue->ka_cfg | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ka_cfg |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	/* remove locks */
	if (kattach_appqueue->ka_cfg == (kattach_appqueue->ka_cfg | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ka_cfg ^= CM_MSG_Q_APPQUEUE_LOCK;
	}

	if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update ^= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_upd_inst(void)
{
	if (kattach_appqueue->ka_inst != (kattach_appqueue->ka_inst | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ka_inst |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	/* remove locks */
	if (kattach_appqueue->ka_inst == (kattach_appqueue->ka_inst | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ka_inst ^= CM_MSG_Q_APPQUEUE_LOCK;
	}

	if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update ^= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_upd_vmst(void)
{
	if (kattach_appqueue->ka_vmst != (kattach_appqueue->ka_vmst | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ka_vmst |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	/* remove locks */
	if (kattach_appqueue->ka_vmst == (kattach_appqueue->ka_vmst | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ka_vmst ^= CM_MSG_Q_APPQUEUE_LOCK;
	}

	if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update ^= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_upd_vmports(void)
{
	if (kattach_appqueue->ka_vmports != (kattach_appqueue->ka_vmports | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ka_vmports |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	/* remove locks */
	if (kattach_appqueue->ka_vmports == (kattach_appqueue->ka_vmports | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ka_vmports ^= CM_MSG_Q_APPQUEUE_LOCK;
	}

	if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update ^= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_upd_vbridge(void)
{
	if (kattach_appqueue->ka_vbridge != (kattach_appqueue->ka_vbridge | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ka_vbridge |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	/* remove locks */
	if (kattach_appqueue->ka_vbridge == (kattach_appqueue->ka_vbridge | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ka_vbridge ^= CM_MSG_Q_APPQUEUE_LOCK;
	}

	if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update ^= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_upd_vmimages(void)
{
	if (kattach_appqueue->ka_vmimages != (kattach_appqueue->ka_vmimages | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ka_vmimages |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	/* remove locks */
	if (kattach_appqueue->ka_vmimages == (kattach_appqueue->ka_vmimages | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ka_vmimages ^= CM_MSG_Q_APPQUEUE_LOCK;
	}

	if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update ^= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_upd_netdev(void)
{
	if (kattach_appqueue->ka_netdev != (kattach_appqueue->ka_netdev | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ka_netdev |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	/* remove locks */
	if (kattach_appqueue->ka_netdev == (kattach_appqueue->ka_netdev | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ka_netdev ^= CM_MSG_Q_APPQUEUE_LOCK;
	}

	if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update ^= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_upd_vns(void)
{
	if (kattach_appqueue->ka_vns != (kattach_appqueue->ka_vns | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ka_vns |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	/* remove locks */
	if (kattach_appqueue->ka_vns == (kattach_appqueue->ka_vns | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ka_vns ^= CM_MSG_Q_APPQUEUE_LOCK;
	}

	if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update ^= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_upd_cfggrp(void)
{
	if (kattach_appqueue->ka_cfggrp != (kattach_appqueue->ka_cfggrp | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ka_cfggrp |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	/* remove locks */
	if (kattach_appqueue->ka_cfggrp == (kattach_appqueue->ka_cfggrp | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ka_cfggrp ^= CM_MSG_Q_APPQUEUE_LOCK;
	}

	if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update ^= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}

void
appqueue_cli_upd_fw(void)
{
	if (kattach_appqueue->ka_fw != (kattach_appqueue->ka_fw | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ka_fw |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_UPDATED)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_UPDATED;
	}

	/* remove locks */
	if (kattach_appqueue->ka_fw == (kattach_appqueue->ka_fw | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ka_fw ^= CM_MSG_Q_APPQUEUE_LOCK;
	}

	if (kattach_appqueue->ak_update == (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_LOCK)) {
		kattach_appqueue->ak_update ^= CM_MSG_Q_APPQUEUE_LOCK;
	}
	return;
}


void
appqueue_cli_mf_info_l4(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_info_l4.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_info_l4.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_info_l4.climenu[i].menu_cmd,appqueue_cli_menu_info_l4.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sLayer 4 Info]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_L4;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_info_l4_fw(void)
{
	printf("\n\nFilter Table:\n");
	printf("---------------\n");
	appqueue_cli_mf_net_fw_filter_list();
	printf("\nPacket Mangling Table:\n");
	printf("------------------------\n");
	appqueue_cli_mf_net_fw_mangle_list();
	printf("\nNetwork Address Translation Table:\n");
	printf("------------------------------------\n");
	appqueue_cli_mf_net_fw_nat_list();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_L4;
	return;
}

void
appqueue_cli_mf_info_l4_vsip(void)
{
	u32 index = 0; 
	u16 vmp = 0;
	u8 vspindex = 0;
	char vip[20];

	printf("\n\n");
	printf("Virtual Service IP    CIDR    Net Device    VM Mapping\n");
	printf("------------------    ----    ----------    ----------------------------------------------------------\n");

	for (index = 0; index < kattach_vns_shm->index; index++) {
		if (kattach_vns_shm->vns[index].enabled == 0) continue;
		sprintf(vip,"%lu.%lu.%lu.%lu",((kattach_vns_shm->vns[index].vsip >> 24) & 0xff), ((kattach_vns_shm->vns[index].vsip >> 16) & 0xff), ((kattach_vns_shm->vns[index].vsip >> 8) & 0xff), 
						((kattach_vns_shm->vns[index].vsip) & 0xff));
		printf("%18s     /%2u    %10s    ",vip,kattach_vns_shm->vns[index].vsmsk,kattach_netdev_shm->pif[kattach_vns_shm->vns[index].netifidx].devname);
		for (vspindex = 0; vspindex < kattach_vns_shm->vns[index].vspindex; vspindex++) {
			if (vspindex != 0) {
				printf("                                            ");
			}
			vmp = kattach_vns_shm->vns[index].vsp[vspindex].vmport;
			sprintf(vip,"%lu.%lu.%lu.%lu",((kattach_vmports_shm->vmports[vmp].vmpip >> 24) & 0xff), ((kattach_vmports_shm->vmports[vmp].vmpip >> 16) & 0xff),((kattach_vmports_shm->vmports[vmp].vmpip >> 8) & 0xff),
							((kattach_vmports_shm->vmports[vmp].vmpip) & 0xff));
			printf("%3s %5u -> %18s:%5u, port %4u\n",(kattach_vns_shm->vns[index].vsp[vspindex].sproto == 0) ? "tcp" : "udp", kattach_vns_shm->vns[index].vsp[vspindex].vsport,
								vip,kattach_vns_shm->vns[index].vsp[vspindex].vmsport,kattach_vns_shm->vns[index].vsp[vspindex].vmport+1);
			/* FIXME: show VLAN information here too */
		}
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_L4;
	return;
}

void
appqueue_cli_mf_info_vm(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_info_vm.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_info_vm.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_info_vm.climenu[i].menu_cmd,appqueue_cli_menu_info_vm.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sVM Info]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_VM;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_info_vm_fw(void)
{
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_VM;
	return;
}

void
appqueue_cli_mf_info_vm_priority(void)
{
	u32 index = 0;
	u16 vport = 0, vlan = 0, vbridge = 0;
	char pip[20];
	char vmstatus[20];

	printf("\n");
	printf("Priority      vPID               Name    vCPU      vMem       Status    VLAN    Port          Primary IP            VM App Image\n");
	printf("--------    ------    ---------------    ----    ------    ---------    ----    ----    ----------------    --------------------\n");

	for (index = 0; index < kattach_vmst_shm->index; index++) {
		if (kattach_vmst_shm->vmsess[index].vmstatus == 0x04) continue;
		vport = kattach_vmst_shm->vmsess[index].vmport[0];
		vbridge = kattach_vmports_shm->vmports[vport].vbridge;
		vlan = kattach_vbridge_shm->vbridge[vbridge].vlan;
		if (kattach_vmst_shm->vmsess[index].vmstatus == 0x01) {
			sprintf(vmstatus,"   active");
		} else if (kattach_vmst_shm->vmsess[index].vmstatus == 0x00) {
			sprintf(vmstatus," starting");
		} else if (kattach_vmst_shm->vmsess[index].vmstatus == 0x02) {
			sprintf(vmstatus,"  stopped");
		} else if (kattach_vmst_shm->vmsess[index].vmstatus == 0x03) {
			sprintf(vmstatus," disabled");
		} else if (kattach_vmst_shm->vmsess[index].vmstatus == 0x04) {
			continue;
		} else {
			sprintf(vmstatus,"  unknown");
		}

		sprintf(pip,"%lu.%lu.%lu.%lu", (kattach_vmports_shm->vmports[vport].vmpip >> 24) & 0xff,
						(kattach_vmports_shm->vmports[vport].vmpip >> 16) & 0xff,
						(kattach_vmports_shm->vmports[vport].vmpip >> 8) & 0xff,
						(kattach_vmports_shm->vmports[vport].vmpip) & 0xff);
		printf("%3s (%2u)    %5d     %14s     %3u    %6u    %s    %4u    %4u    %16s    %20s\n", ((kattach_vmst_shm->vmsess[index].priority <= 13) ? " hi" : ((kattach_vmst_shm->vmsess[index].priority > 13) &&
							(kattach_vmst_shm->vmsess[index].priority < 26)) ? "med" : "low"),
							kattach_vmst_shm->vmsess[index].priority, kattach_vmst_shm->vmsess[index].vpid,
							kattach_vmst_shm->vmsess[index].vmname, kattach_vmst_shm->vmsess[index].vcpu,
							kattach_vmst_shm->vmsess[index].vmem, vmstatus, vlan, (vport + 1), pip,
							kattach_vmimages_shm->vmimage[kattach_vmst_shm->vmsess[index].vmimage].vminame);
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_INFO_VM;
	return;
}

void
appqueue_cli_mf_apps_import(void)
{
	printf("\nNot implemented\n");

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS;
	return;
}

void
appqueue_cli_mf_apps_update(void)
{

	int y = 0, x = 0, q = 0, m = 0;
	char c = '\0';
	u32 oindex = 0, aindex = 0;
	char appmod[128];
	char cliask[255];
	char clians[255];

	printf("\n");
	printf("Available App Queues: ");
	printf(" (c)ommunity ");
	printf(" (d)evelopment ");

	/* FIXME: supported tree */

	if (y == 1) {
		printf(" (s)upported ");
	}
	
	printf("\n\n");

	sprintf(cliask,"Select an AppQueue: ");

	while (y == 0) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'c') {
			q = APPQUEUE_TREE_EDGE;
			y++;
			break;
		} else if (clians[0] == 'd') {
			q = APPQUEUE_TREE_DEV;
			y++;
			break;
		} else if (clians[0] == 's') {
			if (y == 1) {
				q = APPQUEUE_TREE_SUPPORTED;
				y++;
				break;
			}
		}
	}
	y = 0;
	memset(appmod,0,sizeof(appmod));
	printf("\nEnter App Module to Update: ");

	while (!x) {
		c = tolower(appqueue_cli_getch(1));
		if (c == '\n') {
			if (strlen(appmod) <= 1) {
				if (!appqueue_options.index) return;
				x = 1;
				continue;
			}
			for (aindex = 0; aindex < kattach_appmods_shm->index; aindex++) {
				if (strncmp(appmod,kattach_appmods_shm->appmodules[aindex].name,strlen(appmod))) continue;
				if (q != kattach_appmods_shm->appmodules[aindex].srctree) continue;
				m = 1;
				break;
			}

			if (!m) {
				printf("\nApp Module %s is not installed.\n", appmod);
			} else {
				oindex = appqueue_options.index;
				sprintf(appqueue_options.option[oindex].app_mod,"%s",appmod);
				if (q == APPQUEUE_TREE_SUPPORTED) {
					sprintf(appqueue_options.option[oindex].url,"%s%s%s%s",APPQUEUE_HTTP,appmod,APPQUEUE_URL_VENDOR_CM,APPQUEUE_URL_DOMAIN_S);
					sprintf(appqueue_options.option[oindex].cliurl,"%s%s%s%s%s",APPQUEUE_HTTP,appmod,APPQUEUE_URL_VENDOR_CM,APPQUEUE_URL_CLI,APPQUEUE_URL_DOMAIN_S);
					sprintf(appqueue_options.option[oindex].mgrurl,"%s%s%s%s%s",APPQUEUE_HTTP,appmod,APPQUEUE_URL_VENDOR_CM,APPQUEUE_URL_MGR,APPQUEUE_URL_DOMAIN_S);
				} else if (q == APPQUEUE_TREE_DEV) {
					sprintf(appqueue_options.option[oindex].url,"%s%s%s%s",APPQUEUE_HTTP,appmod,APPQUEUE_URL_VENDOR_CM,APPQUEUE_URL_DOMAIN_D);
					sprintf(appqueue_options.option[oindex].cliurl,"%s%s%s%s%s",APPQUEUE_HTTP,appmod,APPQUEUE_URL_VENDOR_CM,APPQUEUE_URL_CLI,APPQUEUE_URL_DOMAIN_D);
					sprintf(appqueue_options.option[oindex].mgrurl,"%s%s%s%s%s",APPQUEUE_HTTP,appmod,APPQUEUE_URL_VENDOR_CM,APPQUEUE_URL_MGR,APPQUEUE_URL_DOMAIN_D);
				} else {
					sprintf(appqueue_options.option[oindex].url,"%s%s%s%s",APPQUEUE_HTTP,appmod,APPQUEUE_URL_VENDOR_CM,APPQUEUE_URL_DOMAIN_E);
					sprintf(appqueue_options.option[oindex].cliurl,"%s%s%s%s%s",APPQUEUE_HTTP,appmod,APPQUEUE_URL_VENDOR_CM,APPQUEUE_URL_CLI,APPQUEUE_URL_DOMAIN_E);
					sprintf(appqueue_options.option[oindex].mgrurl,"%s%s%s%s%s",APPQUEUE_HTTP,appmod,APPQUEUE_URL_VENDOR_CM,APPQUEUE_URL_MGR,APPQUEUE_URL_DOMAIN_E);
				}
				appqueue_options.index++;
			}
			memset(appmod,0,sizeof(appmod));
			c = '\0';
			y = 0;
			m = 0;
			printf("\nEnter App Module to Update: ");
		} else {
			appmod[y] = c;
			y++;
		}
	}

	appqueue_install();

	for (oindex = 0; oindex <= appqueue_options.index; oindex++) {
		if (appqueue_options.option[oindex].valid == 0) continue;
		aindex = appqueue_options.option[oindex].aindex;
		sprintf(kattach_appmods_shm->appmodules[aindex].name,"%s",appqueue_options.option[oindex].app_mod);
		sprintf(kattach_appmods_shm->appmodules[aindex].url,"%s",appqueue_options.option[oindex].url);
		sprintf(kattach_appmods_shm->appmodules[aindex].filename,"%s.aqi",appqueue_options.option[oindex].app_mod);
		kattach_appmods_shm->appmodules[aindex].vendor_id = KATTACH_VID_CM;
		kattach_appmods_shm->appmodules[aindex].state = CM_APP_M_STATE_NEW;
		memset(appqueue_options.option[oindex].app_mod,0,sizeof(appqueue_options.option[oindex].app_mod));
		memset(appqueue_options.option[oindex].url,0,sizeof(appqueue_options.option[oindex].url));
	}

	appqueue_options.index = 0;

	if (kattach_appqueue->ak_update != (kattach_appqueue->ak_update | CM_MSG_Q_APPQUEUE_MODULE)) {
		kattach_appqueue->ak_update |= CM_MSG_Q_APPQUEUE_MODULE;
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS;
	return;
}

void
appqueue_cli_mf_apps_config(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_apps_config.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_apps_config.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_apps_config.climenu[i].menu_cmd,appqueue_cli_menu_apps_config.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sApp Config]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS_CONFIG;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_apps_config_list(void)
{
	u32 index;

	printf("\n\n");
	printf("     Config Group               App Module\n");
	printf("-----------------    ---------------------\n");

	for (index = 0; index < kattach_cfggrp_shm->index; index++) {
		if (kattach_cfggrp_shm->cfggrp[index].name[0] == '\0') continue;
		printf(" %16s     %20s\n", kattach_cfggrp_shm->cfggrp[index].name, kattach_appmods_shm->appmodules[kattach_cfggrp_shm->cfggrp[index].appmidx].name);
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS_CONFIG;
	return;
}

void
appqueue_cli_mf_apps_config_create(void)
{
	char cliask[255];
	char clians[255];
	char name[64];
	u32 cfgidx = 0, index = 0, aindex = 0;
	u8 y = 0, d = 0, q = 0;

	sprintf(cliask,"New Config Group Name: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS_CONFIG;
			return;
		}
		d = 0;
		for (index = 0; index < kattach_cfggrp_shm->index; index++) {
			if (strlen(clians) != strlen(kattach_cfggrp_shm->cfggrp[index].name)) continue;
			if (strncmp(clians,kattach_cfggrp_shm->cfggrp[index].name,strlen(clians))) continue;
			d = 1;
			break;
		}
		if (!d) {
			y++;
			break;
		} else {
			printf("\nConfig Group Name must be unique. %s already exists.\n",clians);
		}
	}

	sprintf(name,"%s",clians);

	y = 0;
	d = 0;
	printf("\n");
	printf("Available App Queues: ");
	printf(" (c)ommunity ");
	printf(" (d)evelopment ");

	/* FIXME: supported tree */

	if (y == 1) {
		printf(" (s)upported ");
	}
	
	printf("\n\n");

	sprintf(cliask,"Select an AppQueue: ");

	while (y == 0) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'c') {
			q = APPQUEUE_TREE_EDGE;
			y++;
			break;
		} else if (clians[0] == 'd') {
			q = APPQUEUE_TREE_DEV;
			y++;
			break;
		} else if (clians[0] == 's') {
			if (y == 1) {
				q = APPQUEUE_TREE_SUPPORTED;
				y++;
				break;
			}
		}
	}
	y = 0;

	sprintf(cliask,"App Module to Configure (? to list): ");
	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS_CONFIG;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_apps_list();
		} else {
			/* [271]: Fix erroronous display of error message */
			for (aindex = 0; aindex < kattach_appmods_shm->index; aindex++) {
				if (strncmp(clians,kattach_appmods_shm->appmodules[aindex].name,strlen(clians))) continue;
				if (q != kattach_appmods_shm->appmodules[aindex].srctree) continue;
				d = 1;
				break;
			}
			if (d) {
				y++;
				break;
			} else {
				printf("\nApp Module %s is not installed.\n",clians);
			}
		}	
	}

	appqueue_cli_lock_cfggrp();
	cfgidx = kattach_cfggrp_shm->index;
	kattach_cfggrp_shm->index++;
	sprintf(kattach_cfggrp_shm->cfggrp[cfgidx].name,"%s",name);
	kattach_cfggrp_shm->cfggrp[cfgidx].appmidx = aindex;
	appqueue_cli_upd_cfggrp();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS_CONFIG;
	return;
}

void
appqueue_cli_mf_apps_config_remove(void)
{
	char cliask[255];
	char clians[255];
	u32 index = 0;
	u8 y = 0, d = 0;

	sprintf(cliask,"Delete Config Group: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS_CONFIG;
			return;
		}
		d = 0;
		for (index = 0; index < kattach_cfggrp_shm->index; index++) {
			if (strlen(clians) != strlen(kattach_cfggrp_shm->cfggrp[index].name)) continue;
			if (strncmp(clians,kattach_cfggrp_shm->cfggrp[index].name,strlen(clians))) continue;
			d = 1;
			break;
		}
		if (d) {
			y++;
			break;
		} else {
			printf("\nConfig Group %s does not exist.\n",clians);
		}
	}

	appqueue_cli_lock_cfggrp();
	kattach_cfggrp_shm->cfggrp[index].name[0] = '\0';
	appqueue_cli_upd_cfggrp();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS_CONFIG;
	return;
}

void
appqueue_cli_mf_apps_config_app(void)
{
	char cliask[255];
	char clians[255];
	u32 index = 0;
	u8 y = 0, d = 0;

	sprintf(cliask,"Configuration Group (? to list): ");
	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS_CONFIG;
			return;
		}
		d = 0;
		if (clians[0] == '?') {
			appqueue_cli_mf_apps_config_list();
		} else {
			for (index = 0; index < kattach_cfggrp_shm->index; index++) {
				if (strlen(clians) != strlen(kattach_cfggrp_shm->cfggrp[index].name)) continue;
				if (strncmp(clians,kattach_cfggrp_shm->cfggrp[index].name,strlen(clians))) continue;
				d = 1;
				break;
			}
		}
		if (d) {
			y++;
			break;
		}
	}

	memset(clians,0,strlen(clians));
	if (kattach_appmods_shm->appmodules[kattach_cfggrp_shm->cfggrp[index].appmidx].vendor_id == KATTACH_VID_LOCAL) { 
		sprintf(clians,"%s%s%s/%s",APPQUEUE_AM_PATH,APPQUEUE_AM_IMPORT,kattach_appmods_shm->appmodules[kattach_cfggrp_shm->cfggrp[index].appmidx].name,APPQUEUE_AM_CLI);
	} else if (kattach_appmods_shm->appmodules[kattach_cfggrp_shm->cfggrp[index].appmidx].vendor_id == KATTACH_VID_CM) {
		sprintf(clians,"%s%s%s/%s",APPQUEUE_AM_PATH,APPQUEUE_AM_CM,kattach_appmods_shm->appmodules[kattach_cfggrp_shm->cfggrp[index].appmidx].name,APPQUEUE_AM_CLI);		
	} else {
		sprintf(clians,"%s%s%s/%s",APPQUEUE_AM_PATH,APPQUEUE_AM_CM,kattach_appmods_shm->appmodules[kattach_cfggrp_shm->cfggrp[index].appmidx].name,APPQUEUE_AM_CLI);		
	}
	/* the following function will execute the extended CLI module for the select app module.
	 * from this point on, AppQueue has passed control of the current CLI to this module.
	 * AppQueue will regain control when the module has exited. AppQueue passes the group name
	 * into the module which is used to tell it which configuration it should be using.
	 */

	appqueue_cli_mod(clians,kattach_cfggrp_shm->cfggrp[index].name);

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_APPS_CONFIG;
	return;
}

void
appqueue_cli_mf_vm_priority(void)
{
	u32 index = 0, findex = 0;
	char cliask[255];
	char clians[255];
	u8 y = 0, len = 0, priority = 0;

	sprintf(cliask,"\nEnter Name of VM: ");

	while (!y) {
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);

		len = strlen(clians);

		if (len) {
			for (index = 0; index < kattach_vmst_shm->index; index++) {
				if (len != strlen(kattach_vmst_shm->vmsess[index].vmname)) continue;
				if (strncmp(kattach_vmst_shm->vmsess[index].vmname,clians,len)) continue;
				
				findex = index;
				y++;
				break;
			}
		} else {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
			return;
		}
	}

	y = 0;
	sprintf(cliask,"New Priority (0 - 39): ");
	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
			return;
		}
		priority = atoi(clians);
		if (priority < 40) {
			y++;
			break;
		}
	}

	appqueue_cli_lock_vmst();
	kattach_vmst_shm->vmsess[findex].priority = priority;
	appqueue_cli_upd_vmst();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
	return;
}

void
appqueue_cli_mf_vm_import(void)
{
	printf("\nNot implemented\n");
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_VM;
	return;
}

void
appqueue_cli_mf_net_netif(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_net_netif.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_net_netif.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_net_netif.climenu[i].menu_cmd,appqueue_cli_menu_net_netif.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sNetwork Interfaces]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_NETIF;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_net_vns(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_net_vns.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_net_vns.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_net_vns.climenu[i].menu_cmd,appqueue_cli_menu_net_vns.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sVirtual Network Services]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_net_vns_addvsip(void)
{
	char cliask[255];
	char clians[255];
	u32 index = 0, vnsindex = 0;
	u32 ip = 0;
	u8 cidr = 0, y = 0;

	sprintf(cliask,"\nNew Virtual Service IP: ");

	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
			return;
		}	
		ip = appqueue_cli_parseip(clians);
		if (ip != 0) {
			y++;
			break;
		}
	}

	y = 0;
	sprintf(cliask,"IP Mask (32 for single IP): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
			return;
		}	
		cidr = appqueue_cli_parseip(clians);
		if (cidr != 0) {
			y++;
			break;
		}
	}

	y = 0;
	sprintf(cliask,"Bind to Network Interface (? to list): ");
	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);	
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
			return;
		}	
		if (clians[0] == '?') {
			printf("\nAvailable Interfaces: \n\n");
			for (index = 0; index < kattach_netdev_shm->index; index++) {
				if ((kattach_netdev_shm->pif[index].mtu == 0) || (kattach_netdev_shm->pif[index].pvid == 0) 
					|| (strlen(kattach_netdev_shm->pif[index].devname) == 0)) continue;
				printf("\t%s\t(%02x:%02x:%02x:%02x:%02x:%02x)\n",kattach_netdev_shm->pif[index].devname, kattach_netdev_shm->pif[index].mac[0],
							kattach_netdev_shm->pif[index].mac[1], kattach_netdev_shm->pif[index].mac[2],
							kattach_netdev_shm->pif[index].mac[3], kattach_netdev_shm->pif[index].mac[4],
							kattach_netdev_shm->pif[index].mac[5]);
			}
		} else {
			for (index = 0; index < kattach_netdev_shm->index; index++) {
				if ((!strncmp(kattach_netdev_shm->pif[index].devname,clians,strlen(clians))) && 
					(kattach_netdev_shm->pif[index].mtu != 0)) {
					y++;
					break;
				}
			}
		}
	}

	appqueue_cli_lock_vns();
	vnsindex = kattach_vns_shm->index;
	kattach_vns_shm->index++;
	kattach_vns_shm->vns[vnsindex].mstate = 0;
	kattach_vns_shm->vns[vnsindex].netifidx = index;
	kattach_vns_shm->vns[vnsindex].vspindex = 0;
	kattach_vns_shm->vns[vnsindex].vsip = ip;
	kattach_vns_shm->vns[vnsindex].vsmsk = cidr;
	kattach_vns_shm->vns[vnsindex].enabled = 1;
	appqueue_cli_upd_vns();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
	return;
}

void
appqueue_cli_mf_net_vns_rmvsip(void)
{
	char cliask[255];
	char clians[255];
	u32 vnsindex = 0;
	u32 ip = 0;
	u8 cidr = 0, y = 0;

	sprintf(cliask,"\nDelete Virtual Service IP: ");

	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
			return;
		}	
		ip = appqueue_cli_parseip(clians);
		if (ip != 0) {
			y++;
			break;
		}
	}

	y = 0;
	sprintf(cliask,"with IP Mask (32 for single IP): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
			return;
		}	
		cidr = appqueue_cli_parseip(clians);
		if (cidr != 0) {
			y++;
			break;
		}
	}

	y = 0;

	for (vnsindex = 0; vnsindex < kattach_vns_shm->index; vnsindex++) {
		if (kattach_vns_shm->vns[vnsindex].enabled == 0) continue;
		if ((kattach_vns_shm->vns[vnsindex].vsip == ip) && (kattach_vns_shm->vns[vnsindex].vsmsk == cidr)) {
			y = 1;
			break;
		}
	}

	if (y) {
		appqueue_cli_lock_vns();
		kattach_vns_shm->vns[vnsindex].enabled = 0;			/* FIXME: properly delete this later */
		appqueue_cli_upd_vns();
	} else {
		printf("\nNo match found.\n");
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
	return;
}

void
appqueue_cli_mf_net_vns_addvsp(void)
{
	char cliask[255];
	char clians[255];
	u32 vip = 0, vnsindex;
	u16 vsport = 0, vmsport = 0, vport = 0;
	u8 y = 0, protocols = 0, vspindex = 0, n = 0, vmask = 0, d = 0;

	sprintf(cliask,"\nVirtual Service Port [0 - 65535]: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		vsport = atol(clians);
		if (vsport <= 65535) {
			y++;
			break;
		}
	}

	y = 0;
	sprintf(cliask,"Map to Virtual Guest Service Port [0 - 65535]: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		vmsport = atol(clians);
		if (vmsport <= 65535) {
			y++;
			break;
		}
	}

	y = 0;
	sprintf(cliask,"Protocol (t)cp or (u)dp: ");
	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 't') {
			protocols = 0;
			y++;
			break;
		} else if (clians[0] == 'u') {
			protocols = 1;
			y++;
			break;
		}
	}

	y = 0;
	sprintf(cliask,"Map to Virtual Port (? to list): ");
	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == '?') {
			appqueue_cli_mf_info_l2_vmports();
		} else {
			vport = atoi(clians);
			if (vport <= 65535) {
				y++;
				break;
			}
		}
	}
	/* FIXME: add code here to verify that vmsport corresponds to a service port on that vm */

	y = 0;
	while (!y) {
		sprintf(cliask,"\nAdd to Virtual Service IP (? to list): ");
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			y++;
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_info_l4_vsip();
		} else {
			vip = appqueue_cli_parseip(clians);
			n = 0;
			sprintf(cliask,"with CIDR mask: ");	
			while (!n) {
				memset(clians,0,strlen(clians));
				appqueue_cli_askq(cliask,1,clians);
				vmask = atoi(clians);
				for (vnsindex = 0; vnsindex < kattach_vns_shm->index; vnsindex++) {
					if (kattach_vns_shm->vns[vnsindex].enabled == 0) continue;
					if ((kattach_vns_shm->vns[vnsindex].vsip == vip) && (kattach_vns_shm->vns[vnsindex].vsmsk == vmask)) {
						/* check for duplicates */
						d = 0;
						for (vspindex = 0; vspindex < kattach_vns_shm->vns[vnsindex].vspindex; vspindex++) {
							if (kattach_vns_shm->vns[vnsindex].vsp[vspindex].vsport == vsport) {
								if (kattach_vns_shm->vns[vnsindex].vsp[vspindex].sproto == protocols) {
									d = 1;
								}
							}
						}
						if (kattach_vns_shm->vns[vnsindex].vspindex >= 32) {
							printf("\nMaximum Virtual Service Ports reached for this Virtual Service IP.\n");
							d = 1;
						}
						if (!d) {
							appqueue_cli_lock_vns();
							vspindex = kattach_vns_shm->vns[vnsindex].vspindex;
							kattach_vns_shm->vns[vnsindex].vsp[vspindex].vsport = vsport;
							kattach_vns_shm->vns[vnsindex].vsp[vspindex].vmsport = vmsport;
							kattach_vns_shm->vns[vnsindex].vsp[vspindex].vmport = (vport - 1);
							kattach_vns_shm->vns[vnsindex].vsp[vspindex].sproto = protocols;
							kattach_vns_shm->vns[vnsindex].vsp[vspindex].enabled = 1;
							kattach_vns_shm->vns[vnsindex].vsp[vspindex].rate_in = 0;
							kattach_vns_shm->vns[vnsindex].vsp[vspindex].rate_out = 0;
							kattach_vns_shm->vns[vnsindex].vsp[vspindex].time_in = 0;
							kattach_vns_shm->vns[vnsindex].vsp[vspindex].time_out = 0;
							kattach_vns_shm->vns[vnsindex].vspindex++;
							appqueue_cli_upd_vns();
							printf("\nAdded port %u.\n",vsport);
						}
						n++;
						break;
					}
				}
				n++;
			}	
		}			
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
	return;
}

void
appqueue_cli_mf_net_vns_rmvsp(void)
{
	char cliask[255];
	char clians[255];
	u32 vip = 0, vnsindex;
	u16 vsport = 0;
	u8 y = 0, protocols = 0, vspindex = 0, n = 0, vmask = 0, d = 0;

	sprintf(cliask,"\nDelete Virtual Service Port [0 - 65535]: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		vsport = atol(clians);
		if (vsport <= 65535) {
			y++;
			break;
		}
	}

	y = 0;
	sprintf(cliask,"Protocol (t)cp or (u)dp: ");
	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 't') {
			protocols = 0;
			y++;
			break;
		} else if (clians[0] == 'u') {
			protocols = 1;
			y++;
			break;
		}
	}

	y = 0;
	while (!y) {
		sprintf(cliask,"Delete from Virtual Service IP (? to list): ");
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			y++;
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_info_l4_vsip();
		} else {
			vip = appqueue_cli_parseip(clians);
			n = 0;
			sprintf(cliask,"with CIDR mask: ");	
			while (!n) {
				memset(clians,0,strlen(clians));
				appqueue_cli_askq(cliask,1,clians);
				vmask = atoi(clians);
				for (vnsindex = 0; vnsindex < kattach_vns_shm->index; vnsindex++) {
					if (kattach_vns_shm->vns[vnsindex].enabled == 0) continue;
					if ((kattach_vns_shm->vns[vnsindex].vsip == vip) && (kattach_vns_shm->vns[vnsindex].vsmsk == vmask)) {
						/* check for match */
						d = 0;
						for (vspindex = 0; vspindex < kattach_vns_shm->vns[vnsindex].vspindex; vspindex++) {
							if (kattach_vns_shm->vns[vnsindex].vsp[vspindex].vsport == vsport) {
								if (kattach_vns_shm->vns[vnsindex].vsp[vspindex].sproto == protocols) {
									d = 1;
									break;
								}
							}
						}
						if (d) {
							appqueue_cli_lock_vns();
							vspindex = kattach_vns_shm->vns[vnsindex].vspindex;
							kattach_vns_shm->vns[vnsindex].vsp[vspindex].vsport = 0;
							kattach_vns_shm->vns[vnsindex].vsp[vspindex].vmsport = 0;
							kattach_vns_shm->vns[vnsindex].vsp[vspindex].vmport = 0;
							kattach_vns_shm->vns[vnsindex].vsp[vspindex].sproto = 0;
							kattach_vns_shm->vns[vnsindex].vsp[vspindex].enabled = 0;
							kattach_vns_shm->vns[vnsindex].vsp[vspindex].rate_in = 0;
							kattach_vns_shm->vns[vnsindex].vsp[vspindex].rate_out = 0;
							kattach_vns_shm->vns[vnsindex].vsp[vspindex].time_in = 0;
							kattach_vns_shm->vns[vnsindex].vsp[vspindex].time_out = 0;
							kattach_vns_shm->vns[vnsindex].vspindex++;
							appqueue_cli_upd_vns();
							printf("\nPort removed.\n");
						} else {
							printf("\nNot Found.\n");
						}
						n++;
						break;
					}
				}
				n++;
			}	
		}			
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
	return;
}

void
appqueue_cli_mf_net_vns_ratelimit(void)
{
	printf("\n\nNot implemented\n\n");				/* FIXME: implement this */

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
	return;
}

void
appqueue_cli_mf_net_vns_state(void)
{
	char cliask[255];
	char clians[255];
	u8 y = 0, statemask = 0, cidr = 0;
	u32 ip = 0, vnsindex = 0;

	sprintf(cliask,"\nConfigure State for Virtual Service IP: ");

	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
			return;
		}	
		ip = appqueue_cli_parseip(clians);
		if (ip != 0) {
			y++;
			break;
		}
	}

	y = 0;
	sprintf(cliask,"with IP Mask (32 for single IP): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
			return;
		}	
		cidr = appqueue_cli_parseip(clians);
		if (cidr != 0) {
			y++;
			break;
		}
	}

	y = 0;
	sprintf(cliask,"\nMatch on NEW state (y/n) ? ");
	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
			return;
		}
		if (clians[0] == 'y') {
			statemask |= KATTACH_FW_STMASK_NEW;
			y++;
			break;
		} else if (clians[0] == 'n') {
			y++;
			break;
		}
	}

	y = 0;
	sprintf(cliask,"\nMatch on ESTABLISHED state (y/n) ? ");
	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
			return;
		}
		if (clians[0] == 'y') {
			statemask |= KATTACH_FW_STMASK_ESTABLISHED;
			y++;
			break;
		} else if (clians[0] == 'n') {
			y++;
			break;
		}
	}

	y = 0;
	sprintf(cliask,"\nMatch on RELATED state (y/n) ? ");
	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
			return;
		}
		if (clians[0] == 'y') {
			statemask |= KATTACH_FW_STMASK_RELATED;
			y++;
			break;
		} else if (clians[0] == 'n') {
			y++;
			break;
		}
	}

	y = 0;
	sprintf(cliask,"\nMatch on INVALID state (y/n) ? ");
	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
			return;
		}
		if (clians[0] == 'y') {
			statemask |= KATTACH_FW_STMASK_INVALID;
			y++;
			break;
		} else if (clians[0] == 'n') {
			y++;
			break;
		}
	}

	y = 0;
	for (vnsindex = 0; vnsindex < kattach_vns_shm->index; vnsindex++) {
		if (kattach_vns_shm->vns[vnsindex].enabled == 0) continue;
		if ((kattach_vns_shm->vns[vnsindex].vsip == ip) && (kattach_vns_shm->vns[vnsindex].vsmsk == cidr)) {
			y = 1;
			break;
		}
	}

	if (y) {
		appqueue_cli_lock_vns();
		kattach_vns_shm->vns[vnsindex].mstate = statemask;
		appqueue_cli_upd_vns();
	} else {
		printf("\nNo match found.\n");
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_VNS;
	return;
}

void
appqueue_cli_mf_sys_clock(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_sys_clock.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_sys_clock.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_sys_clock.climenu[i].menu_cmd,appqueue_cli_menu_sys_clock.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sSystem Clock]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_CLOCK;
	appqueue_po = 0;
	return;
}

u32
appqueue_cli_ntp_parseip(char *dotted, u8 ntp)
{
	struct hostent *ntph;
	u32 ntpip = 0;
	int x = 0;

	if ((strlen(dotted) >= 7) && ((dotted[0] >= 0x30) && (dotted[0] <= 0x39)) &&
		((dotted[1] == '.') || (dotted[2] == '.') || (dotted[3] == '.'))) {
		return(appqueue_cli_parseip(dotted));
	}

	ntph = gethostbyname(dotted);
	if (ntph != NULL) {
		ntpip = appqueue_cli_parseip(inet_ntoa(*(struct in_addr *)ntph->h_addr_list[0]));
		if (ntp == 0) {
			while ((ntpip == kattach_cfg_shm->ntp_ip[1]) || (ntpip == kattach_cfg_shm->ntp_ip[2])) {
				for (x = 1; ntph->h_addr_list[x] != NULL; x++) {
					ntpip = appqueue_cli_parseip(inet_ntoa(*(struct in_addr *)ntph->h_addr_list[x]));
				}
				break;
			}
		} else if (ntp == 1) {
			while ((ntpip == kattach_cfg_shm->ntp_ip[0]) || (ntpip == kattach_cfg_shm->ntp_ip[2])) {
				for (x = 1; ntph->h_addr_list[x] != NULL; x++) {
					ntpip = appqueue_cli_parseip(inet_ntoa(*(struct in_addr *)ntph->h_addr_list[x]));
				}
				break;
			}
		} else if (ntp == 2) {
			while ((ntpip == kattach_cfg_shm->ntp_ip[0]) || (ntpip == kattach_cfg_shm->ntp_ip[1])) {
				for (x = 1; ntph->h_addr_list[x] != NULL; x++) {
					ntpip = appqueue_cli_parseip(inet_ntoa(*(struct in_addr *)ntph->h_addr_list[x]));
				}
				break;
			}
		}
	} else {
		ntpip = 0;
	}
	return(ntpip);
}

void
appqueue_cli_mf_sys_clock_ntpa(void)
{
	char cliask[255];
	char clians[255];
	u32 ntpip = 0;
	u8 y = 0;

	if (kattach_cfg_shm->ntp_ip[0] == 0) {
		sprintf(cliask,"\nPrimary NTP Server: ");	
	} else {
		sprintf(cliask,"\nPrimary NTP Server [%lu.%lu.%lu.%lu]: ", ((kattach_cfg_shm->ntp_ip[0] >> 24) & 0xff),
				((kattach_cfg_shm->ntp_ip[0] >> 16) & 0xff), ((kattach_cfg_shm->ntp_ip[0] >> 8) & 0xff),
				((kattach_cfg_shm->ntp_ip[0]) & 0xff));
	}


	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_CLOCK;
			return;
		}
		ntpip = appqueue_cli_ntp_parseip(clians, 0);
		if (ntpip != 0) {
			y++;
			break;
		} else {
			printf("\nNTP server not found. Please try again.\n\n");
		}
	}

	appqueue_cli_lock_cfg();
	kattach_cfg_shm->ntp_ip[0] = ntpip;
	appqueue_cli_upd_cfg();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_CLOCK;
	return;
}

void
appqueue_cli_mf_sys_clock_ntpb(void)
{
	char cliask[255];
	char clians[255];
	u32 ntpip = 0;
	u8 y = 0;


	if (kattach_cfg_shm->ntp_ip[1] == 0) {
		sprintf(cliask,"\nSecondary NTP Server: ");	
	} else {
		sprintf(cliask,"\nSecondary NTP Server [%lu.%lu.%lu.%lu]: ", ((kattach_cfg_shm->ntp_ip[1] >> 24) & 0xff),
				((kattach_cfg_shm->ntp_ip[1] >> 16) & 0xff), ((kattach_cfg_shm->ntp_ip[1] >> 8) & 0xff),
				((kattach_cfg_shm->ntp_ip[1]) & 0xff));
	}

	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_CLOCK;
			return;
		}
		ntpip = appqueue_cli_ntp_parseip(clians, 1);
		if (ntpip != 0) {
			y++;
			break;
		} else {
			printf("\nNTP server not found. Please try again.\n\n");
		}
	}

	appqueue_cli_lock_cfg();
	kattach_cfg_shm->ntp_ip[1] = ntpip;
	appqueue_cli_upd_cfg();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_CLOCK;
	return;
}

void
appqueue_cli_mf_sys_clock_ntpc(void)
{
	char cliask[255];
	char clians[255];
	u32 ntpip = 0;
	u8 y = 0;

	if (kattach_cfg_shm->ntp_ip[2] == 0) {
		sprintf(cliask,"\nTertiary NTP Server: ");	
	} else {
		sprintf(cliask,"\nTertiary NTP Server [%lu.%lu.%lu.%lu]: ", ((kattach_cfg_shm->ntp_ip[2] >> 24) & 0xff),
				((kattach_cfg_shm->ntp_ip[2] >> 16) & 0xff), ((kattach_cfg_shm->ntp_ip[2] >> 8) & 0xff),
				((kattach_cfg_shm->ntp_ip[2]) & 0xff));
	}

	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_CLOCK;
			return;
		}
		ntpip = appqueue_cli_ntp_parseip(clians, 2);
		if (ntpip != 0) {
			y++;
			break;
		} else {
			printf("\nNTP server not found. Please try again.\n\n");
		}
	}

	appqueue_cli_lock_cfg();
	kattach_cfg_shm->ntp_ip[2] = ntpip;
	appqueue_cli_upd_cfg();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_CLOCK;
	return;
}

void
appqueue_cli_mf_sys_clock_ntpint(void)
{
	char cliask[255];
	char clians[255];
	u16 ntpint = 0, y = 0;

	printf("\n\n");
	sprintf(cliask,"NTP Polling Interval (minutes): ");

	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_CLOCK;
			return;
		}
		ntpint = atoi(clians);
		if (ntpint > 0) {
			y++;
			break;
		}
	}

	appqueue_cli_lock_cfg();
	kattach_cfg_shm->ntpint = ntpint;
	appqueue_cli_upd_cfg();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_CLOCK;
	return;
}


void
appqueue_cli_mf_sys_auth(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_sys_auth.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_sys_auth.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_sys_auth.climenu[i].menu_cmd,appqueue_cli_menu_sys_auth.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sSystem Auth]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_AUTH;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_sys_disk(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_sys_disk.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_sys_disk.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_sys_disk.climenu[i].menu_cmd,appqueue_cli_menu_sys_disk.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sLocal Storage]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_DISK;
	appqueue_po = 0;
	return;
}


void
appqueue_cli_mf_sys_auth_aqpass(void)
{
	char cliask[255];
	char clians[255];
	char fgo[140];
	char sgo[140];
	u8 y = 0;

	sprintf(cliask,"New password for CLI User: ");

	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,7,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_AUTH;
			return;
		}			
		sprintf(fgo,"%s",appqueue_cli_genpass(clians));
		if (fgo != NULL) {
			memset(clians,0,strlen(clians));
			sprintf(cliask,"\nRe-enter password: ");
			appqueue_cli_askq(cliask,7,clians);
			if (strlen(clians) == 0) {
				appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_AUTH;
				return;
			}			
			sprintf(sgo,"%s",appqueue_cli_chkpass(clians,fgo));
			if (sgo != NULL) {
				if (!strncmp(fgo,sgo,strlen(sgo))) {
					y++;
					break;
				} else {
					sprintf(cliask,"\nNew password for CLI User: ");
					printf("\nPasswords do not match!\n\n");
				}
			} else {
				sprintf(cliask,"\nNew password for CLI User: ");
			}
		}
		memset(fgo,0,strlen(fgo));
		memset(sgo,0,strlen(sgo));
	}

	appqueue_cli_lock_cfg();
	sprintf(kattach_cfg_shm->aqpass,"%s",sgo);
	appqueue_cli_upd_cfg();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_AUTH;
	return;
}


void
appqueue_cli_mf_sys_auth_clipass(void)
{
	char cliask[255];
	char clians[255];
	char fgo[140];
	char sgo[140];
	u8 y = 0;

	sprintf(cliask,"New CLI admin password: ");

	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,7,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_AUTH;
			return;
		}			
		sprintf(fgo,"%s",appqueue_cli_genpass(clians));
		if (fgo != NULL) {
			memset(clians,0,strlen(clians));
			sprintf(cliask,"\nRe-enter password: ");
			appqueue_cli_askq(cliask,7,clians);
			if (strlen(clians) == 0) {
				appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_AUTH;
				return;
			}			
			sprintf(sgo,"%s",appqueue_cli_chkpass(clians,fgo));
			if (sgo != NULL) {
				if (!strncmp(fgo,sgo,strlen(sgo))) {
					y++;
					break;
				} else {
					sprintf(cliask,"\nNew CLI admin password: ");
					printf("\nPasswords do not match!\n\n");
				}
			} else {
				sprintf(cliask,"\nNew CLI admin password: ");
			}
		}
		memset(fgo,0,strlen(fgo));
		memset(sgo,0,strlen(sgo));
	}

	appqueue_cli_lock_cfg();
	sprintf(kattach_cfg_shm->clipass,"%s",sgo);
	appqueue_cli_upd_cfg();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_AUTH;
	return;
}

void
appqueue_cli_mf_sys_auth_root(void)
{
	char cliask[255];
	char clians[255];
	u8 y = 0, x = 0;

	printf("\n\nHypervisor root access is currently: %s",((kattach_cfg_shm->root) ? "enabled" : "disabled"));

	if (kattach_cfg_shm->root) {
		sprintf(cliask,"\nDisable root access (y/n)? ");
		x = 1;
	} else {
		sprintf(cliask,"\nEnable root access (y/n)? ");
	}

	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);

		if (clians[0] == 'y') {
			y++;
			break;
		} else if (clians[0] == 'n') {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_AUTH;
			return;
		}
	}

	appqueue_cli_lock_cfg();
	if (x) {
		kattach_cfg_shm->root = 0;
	} else {
		kattach_cfg_shm->root = 1;
	}
	appqueue_cli_upd_cfg();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_AUTH;
	return;
}

void
appqueue_cli_mf_sys_auth_rootpass(void)
{
	char cliask[255];
	char clians[255];
	char opw[140];
	char fgo[140];
	char sgo[140];
	u8 y = 0, x = 0;

	sprintf(cliask,"\nCurrent root password: ");

	while (!y) {
		memset(opw,0,strlen(opw));
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,7,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_AUTH;
			return;
		}			
		sprintf(opw,"%s",clians);
		if (opw != NULL) {
			while (x == 0) {
				memset(clians,0,strlen(clians));
				sprintf(cliask,"\nNew root password: ");
				appqueue_cli_askq(cliask,7,clians);
				if (strlen(clians) == 0) {
					appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_AUTH;
					return;
				}			
				sprintf(fgo,"%s",appqueue_cli_genpass(clians));
				if (fgo != NULL) {
					memset(clians,0,strlen(clians));
					sprintf(cliask,"\nRe-enter password: ");
					appqueue_cli_askq(cliask,7,clians);
					if (strlen(clians) == 0) {
						appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_AUTH;
						return;
					}			
					sprintf(sgo,"%s",appqueue_cli_chkpass(clians,fgo));
					if (sgo != NULL) {
						if (!strncmp(fgo,sgo,strlen(sgo))) {
							y++;
							break;
						} else {
							sprintf(cliask,"New root password: ");
							printf("\nPasswords do not match!\n\n");
						}
					} else {
						sprintf(cliask,"\nNew root password: ");
					}
				} else {
					sprintf(cliask,"\nNew root password: ");
				}
				memset(fgo,0,strlen(fgo));
				memset(sgo,0,strlen(sgo));
			}
			y++;
			break;
		}
	}

	appqueue_cli_lock_cfg();
	sprintf(kattach_cfg_shm->rootpwck,"%s",opw);
	sprintf(kattach_cfg_shm->rootpass,"%s",sgo);
	appqueue_cli_upd_cfg();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_AUTH;
	return;
}

void
appqueue_cli_mf_sys_auth_aquser(void)
{
	char cliask[255];
	char clians[255];
	u8 y = 0;

	sprintf(cliask,"New CLI user name: ");

	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_AUTH;
			return;
		} else {
			y++;
			break;
		}
	}

	appqueue_cli_lock_cfg();
	sprintf(kattach_cfg_shm->aquser,"%s",clians);
	appqueue_cli_upd_cfg();
	printf("\nCLI user is now %s.\n\n",kattach_cfg_shm->aquser);

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_SYS_AUTH;
	return;
}

void
appqueue_cli_getruuid(void)
{
        FILE *stream;
        char *buf;
        int j = 0;
        char c = '\0';

        stream = fopen(APPQUEUE_PROC_UUID,"r");
        buf = (char *) malloc(sizeof(appqueue_ruuid));

        while (!feof(stream)) {
                c = (char) fgetc(stream);
                buf[j] = c;
                j++;
        }
        buf[j] = '\0';

        memset(appqueue_ruuid,0,sizeof(appqueue_ruuid));
        strncpy(appqueue_ruuid,buf,strlen(buf));
        appqueue_ruuid[36] = '\0';
        fclose(stream);
        free(buf);
        return;
}

char *
appqueue_cli_genpass(char *dfpass)
{
	unsigned long pwdhash = 0;
	unsigned int pwdtmp = 0;
	char *passwd;
	char salt[16];
	char salty[19];
	int i = 0, y = 0, z = 0, j = 0;

	if (strlen(dfpass) == 0) return(NULL);

	while (!z) {
		appqueue_cli_getruuid();
		pwdhash = appqueue_cli_hash(appqueue_ruuid);
		pwdhash ^= time(NULL);

		for (j = 0; j <= 36; j++) {
			if (i == 15) {
				z = 1;
				salt[i] = '\0';
				break;
			}
			pwdtmp = ((pwdhash >> j) & 0xff);
			y = pwdtmp;
			if (((y >= 0x2e) && (y <= 0x39)) ||
				((y >= 0x41) && (y <= 0x5a)) ||
				((y >= 0x61) && (y <= 0x7a))) {
				salt[i] = (char) y;
				i++;
			}
		}
	}
	sprintf(salty,"$6$%s",salt);
	passwd = crypt(dfpass,salty);
	return(passwd);
}

char *
appqueue_cli_chkpass(char *dfpass, char *encpass)
{
	char *passwd;

	if (strlen(dfpass) == 0) return(NULL);

	passwd = crypt(dfpass,encpass);
	return(passwd);
}

unsigned long 
appqueue_cli_hash(char *str)
{
        unsigned long hash = 5381;
        int c;

        while ((c = *str++))
                hash = ((hash << 5) + hash) + c;

        return hash;
}
