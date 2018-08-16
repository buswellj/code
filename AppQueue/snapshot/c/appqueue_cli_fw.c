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
appqueue_cli_mf_net_fw(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_net_fw.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_net_fw.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_net_fw.climenu[i].menu_cmd,appqueue_cli_menu_net_fw.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sFirewall]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_net_fw_nat(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_net_fw_nat.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_net_fw_nat.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_net_fw_nat.climenu[i].menu_cmd,appqueue_cli_menu_net_fw_nat.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sNAT]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_net_fw_nat_add(void)
{
	char cliask[255];
	char clians[255];
	u32 iindex = 0, zindex = 0;
	u16 fw_update = 0;
	u8 y = 0, d = 0, f = 0, n = 0, a = 0;
	kattach_fw_n_entry_t fw;
	kattach_fw_n_chain_t *chain;

	sprintf(cliask,"Add Firewall Rule to pre(r)outing, (p)ostrouting or (o)utput: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'r') {
			f = 1;
		} else if (clians[0] == 'o') {
			f = 2;
		} else if (clians[0] == 'p') {
			f = 3;
		}
		if (f) {
			y++;
			break;
		}
	}
	y = 0;

	sprintf(cliask,"Index to Insert New Rule after: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
			return;
		}
		iindex = atol(clians);
		y++;
		break;	
	}
	d = 0;
	y = 0;
	sprintf(cliask,"Source Zone (? to list): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_net_fw_zones_list();
		} else {
			for (zindex = 0; zindex < kattach_fw_shm->zones.index; zindex++) {
				if (kattach_fw_shm->zones.zone[zindex].name[0] == '\0') continue;
				if ((strlen(clians)) != (strlen(kattach_fw_shm->zones.zone[zindex].name))) continue;
				if (!strncmp(clians,kattach_fw_shm->zones.zone[zindex].name,strlen(clians))) {
					d = 1;
					y++;
					break;
				}
			}
			if (d) {
				break;
			} else {
				printf("\nZone %s not found. Please check and retry\n",clians);
			}
		}
	}
	fw.szindex = zindex;
	d = 0;
	y = 0;
	sprintf(cliask,"Destination Zone (? to list): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_net_fw_zones_list();
		} else {
			for (zindex = 0; zindex < kattach_fw_shm->zones.index; zindex++) {
				if (kattach_fw_shm->zones.zone[zindex].name[0] == '\0') continue;
				if (strlen(clians) != strlen(kattach_fw_shm->zones.zone[zindex].name)) continue;
				if (!strncmp(clians,kattach_fw_shm->zones.zone[zindex].name,strlen(clians))) {
					d = 1;
					y++;
					break;
				}
			}
			if (d) {
				break;
			} else {
				printf("\nZone %s not found. Please check and retry\n",clians);
			}
		}
	}
	fw.dzindex = zindex;
	d = 0;
	y = 0;
	sprintf(cliask,"App Profile (? to list): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_net_fw_apps_list();
		} else {
			for (zindex = 0; zindex < kattach_fw_shm->apps.index; zindex++) {
				if (kattach_fw_shm->apps.app[zindex].name[0] == '\0') continue;
				if (strlen(clians) != strlen(kattach_fw_shm->apps.app[zindex].name)) continue;
				if (!strncmp(clians,kattach_fw_shm->apps.app[zindex].name,strlen(clians))) {
					d = 1;
					y++;
					break;
				}
			}
			if (d) {
				break;
			} else {
				printf("\nApp %s not found. Please check and retry\n",clians);
			}
		}
	}
	fw.appindex = zindex;
	y = 0;
	d = 0;
	while(!y) {
		sprintf(cliask,"Enable Rate Limiting (y/n) ? ");
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
			return;
		}
		if (clians[0] == 'y') {
			while (!d) {
				/* FIXME */
				sprintf(cliask,"Rate Limit Packets: ");
				appqueue_cli_askq(cliask,1,clians);
				fw.rlimitpkt = atoi(clians);
				sprintf(cliask,"Rate Limit Interval (seconds): ");
				appqueue_cli_askq(cliask,1,clians);
				fw.rlimitint = atoi(clians);
				d++;
			}
			y++;
			break;
		} else if (clians[0] == 'n') {
			fw.rlimitpkt = 0;
			fw.rlimitint = 0;
			y++;
			break;
		}
	}
	y = 0;
	/* action */
	if (f == 3) {
		sprintf(cliask,"Filter Action - (a)llow, (d)rop, (r)eject, (l)og, (s)ource nat, (m)asquerade, netma(p) : ");
	} else {
		sprintf(cliask,"Filter Action - (a)llow, (d)rop, (r)eject, (l)og, destination (n)at, r(e)direct, netma(p) : ");
	}
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
			return;
		}
		if (clians[0] == 'a') {
			fw.action = KATTACH_FW_ACTION_ALLOW;
			y++;
			break;
		} else if (clians[0] == 'd') {
			fw.action = KATTACH_FW_ACTION_DROP;
			y++;
			break;
		} else if (clians[0] == 'r') {
			fw.action = KATTACH_FW_ACTION_REJECT;
			/* FIXME: add capability to select REJECT with */
			y++;
			break;
		} else if (clians[0] == 'l') {
			fw.action = KATTACH_FW_ACTION_LOG;
			y++;
			break;
		} else if ((clians[0] == 's') && (f == 3)) {
			fw.action = KATTACH_FW_ACTION_SNAT;
			n = 1;
			a = 1;
			y++;
			break;
		} else if ((clians[0] == 'n') && (f == 1)) {
			fw.action = KATTACH_FW_ACTION_DNAT;
			n = 1;
			a = 1;
			y++;
			break;
		} else if ((clians[0] == 'm') && (f == 3)) {
			fw.action = KATTACH_FW_ACTION_MASQ;
			a = 1;
			y++;
			break;
		} else if ((clians[0] == 'r') && (f == 1)) {
			fw.action = KATTACH_FW_ACTION_REDIR;
			a = 1;
			y++;
			break;
		} else if ((clians[0] == 'p') && (f != 2)) {
			fw.action = KATTACH_FW_ACTION_NETMAP;
			n = 1;
			y++;
			break;
		}
	}

	d = 0;
	y = 0;
	if (n) {
		sprintf(cliask,"NAT Zone (? to list): ");
		while(!y) {
			memset(clians,0,strlen(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (strlen(clians) == 0) {
				appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
				return;
			}
			if (clians[0] == '?') {
				appqueue_cli_mf_net_fw_zones_list();
			} else {
				for (zindex = 0; zindex < kattach_fw_shm->zones.index; zindex++) {
					if (kattach_fw_shm->zones.zone[zindex].name[0] == '\0') continue;
					if (strlen(clians) != strlen(kattach_fw_shm->zones.zone[zindex].name)) continue;
					if (!strncmp(clians,kattach_fw_shm->zones.zone[zindex].name,strlen(clians))) {
						d = 1;
						y++;
						break;
					}
				}
				if (d) {
					break;
				} else {
					printf("\nZone %s not found. Please check and retry\n",clians);
				}
			}
		}
	}
	fw.nzindex = zindex;
	d = 0;
	y = 0;
	if (a) {
		sprintf(cliask,"NAT App Profile (? to list): ");
		while(!y) {
			memset(clians,0,strlen(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (strlen(clians) == 0) {
				appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
				return;
			}
			if (clians[0] == '?') {
				appqueue_cli_mf_net_fw_apps_list();
			} else {
				for (zindex = 0; zindex < kattach_fw_shm->apps.index; zindex++) {
					if (kattach_fw_shm->apps.app[zindex].name[0] == '\0') continue;
					if (strlen(clians) != strlen(kattach_fw_shm->apps.app[zindex].name)) continue;
					if (!strncmp(clians,kattach_fw_shm->apps.app[zindex].name,strlen(clians))) {
						d = 1;
						y++;
						break;
					}
				}
				if (d) {
					break;
				} else {
					printf("\nApp %s not found. Please check and retry\n",clians);
				}
			}
		}
	}
	fw.nappindex = zindex;

	if (f == 1) {
		chain = &kattach_fw_shm->nat.prerouting;
		fw_update = KATTACH_FW_CH_NAT_PREROUTING;
	} else if (f == 2) {
		chain = &kattach_fw_shm->nat.output;
		fw_update = KATTACH_FW_CH_NAT_OUTPUT;
	} else if (f == 3) {
		chain = &kattach_fw_shm->nat.postrouting;
		fw_update = KATTACH_FW_CH_NAT_POSTROUTING;
	} else {
		printf("\nERROR occured - chain unknown\n");
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
		return;
	}

	appqueue_cli_lock_fw();
	if ((chain->index == 0) && (chain->eindex == chain->hindex)) {
		/* this is the first rule */
		chain->eindex = chain->index;
		chain->hindex = chain->index;
		chain->filter[chain->index].nindex = chain->index;
		chain->filter[chain->index].pindex = chain->index;
	} else if ((iindex == chain->eindex) || (iindex >= chain->index)) {
		/* this puts the rule at the end of the chain */
		chain->filter[chain->eindex].nindex = chain->index;
		chain->filter[chain->index].pindex = chain->eindex;
		chain->filter[chain->index].nindex = chain->index;
		chain->eindex = chain->index;
	} else {
		/* this puts the rule after rule X */
		chain->filter[chain->index].pindex = iindex;
		chain->filter[chain->index].nindex = chain->filter[iindex].nindex;
		chain->filter[iindex].nindex = chain->index;
		chain->filter[chain->filter[chain->index].nindex].pindex = chain->index;
	}
	chain->filter[chain->index].reverse = 0;
	chain->filter[chain->index].logging = 0;
	chain->filter[chain->index].rejectwith = 0;
	chain->filter[chain->index].type = 0;
	chain->filter[chain->index].enabled = 1;
	chain->filter[chain->index].tos[0] = 0;
	chain->filter[chain->index].tos[1] = 0;
	chain->filter[chain->index].ttl[0] = 0;
	chain->filter[chain->index].ttl[1] = 0;
	chain->filter[chain->index].action = fw.action;
	chain->filter[chain->index].rlimitint = fw.rlimitint;
	chain->filter[chain->index].rlimitpkt = fw.rlimitpkt;
	chain->filter[chain->index].appindex = fw.appindex;
	chain->filter[chain->index].nappindex = fw.nappindex;
	chain->filter[chain->index].dzindex = fw.dzindex;
	chain->filter[chain->index].szindex = fw.szindex;
	chain->filter[chain->index].nzindex = fw.nzindex;
	chain->index++;
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
	return;
}

void
appqueue_cli_mf_net_fw_nat_del(void)
{
	char cliask[255];
	char clians[255];
	u32 iindex = 0;
	u16 fw_update = 0;
	u8 y = 0, f = 0;
	kattach_fw_n_chain_t *chain;

	sprintf(cliask,"Delete Firewall Rule from pre(r)outing, (p)ostrouting or (o)utput: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'r') {
			f = 1;
		} else if (clians[0] == 'o') {
			f = 2;
		} else if (clians[0] == 'p') {
			f = 3;
		}
		if (f) {
			y++;
			break;
		}
	}
	y = 0;

	sprintf(cliask,"Delete Rule Index: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
			return;
		}
		iindex = atol(clians);
		y++;
		break;	
	}

	if (f == 1) {
		chain = &kattach_fw_shm->nat.prerouting;
		fw_update = KATTACH_FW_CH_NAT_PREROUTING;
	} else if (f == 2) {
		chain = &kattach_fw_shm->nat.output;
		fw_update = KATTACH_FW_CH_NAT_OUTPUT;
	} else if (f == 3) {
		chain = &kattach_fw_shm->nat.postrouting;
		fw_update = KATTACH_FW_CH_NAT_POSTROUTING;
	} else {
		printf("\nERROR occured - chain unknown\n");
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
		return;
	}

	if ((chain->filter[iindex].nindex == 0) && (chain->filter[iindex].pindex == 0) && (chain->filter[iindex].enabled == 0)) {
		printf("\nFilter ID %lu is not configured.\n",iindex);
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
		return;
	}

	appqueue_cli_lock_fw();
	if (chain->hindex == iindex) {
		/* this filter is head of the chain */
		chain->hindex = chain->filter[iindex].nindex;
		chain->filter[chain->hindex].pindex = chain->hindex;
		if (chain->eindex == iindex) {
			chain->eindex = chain->filter[iindex].pindex;
			chain->filter[chain->eindex].nindex = chain->eindex;
			if (chain->index) {
				chain->index--;
			}
		}
	} else if (chain->eindex == iindex) {
		/* this filter is end of the chain */
		chain->eindex = chain->filter[iindex].pindex;
		chain->filter[chain->eindex].nindex = chain->eindex;
		if (chain->index) {
			chain->index--;
		}
	} else {
		/* this filter is between filters */
		chain->filter[chain->filter[iindex].pindex].nindex = chain->filter[iindex].nindex;
		chain->filter[chain->filter[iindex].nindex].pindex = chain->filter[iindex].pindex;
	}
	chain->filter[iindex].reverse = 0;
	chain->filter[iindex].logging = 0;
	chain->filter[iindex].rejectwith = 0;
	chain->filter[iindex].type = 0;
	chain->filter[iindex].enabled = 0;
	chain->filter[iindex].tos[0] = 0;
	chain->filter[iindex].tos[1] = 0;
	chain->filter[iindex].ttl[0] = 0;
	chain->filter[iindex].ttl[1] = 0;
	chain->filter[iindex].action = 0;
	chain->filter[iindex].rlimitint = 0;
	chain->filter[iindex].rlimitpkt = 0;
	chain->filter[iindex].appindex = 0;
	chain->filter[iindex].nappindex = 0;
	chain->filter[iindex].dzindex = 0;
	chain->filter[iindex].szindex = 0;
	chain->filter[iindex].nzindex = 0;
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
	return;
}

void
appqueue_cli_mf_net_fw_nat_edit(void)
{
	char cliask[255];
	char clians[255];
	u32 iindex = 0, zindex = 0;
	u16 fw_update = 0;
	u8 y = 0, d = 0, f = 0, n = 0, a = 0;
	kattach_fw_n_entry_t fw;
	kattach_fw_n_chain_t *chain;

	sprintf(cliask,"Add Firewall Rule to pre(r)outing, (p)ostrouting or (o)utput: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'r') {
			f = 1;
		} else if (clians[0] == 'o') {
			f = 2;
		} else if (clians[0] == 'p') {
			f = 3;
		}
		if (f) {
			y++;
			break;
		}
	}
	y = 0;

	if (f == 1) {
		chain = &kattach_fw_shm->nat.prerouting;
		fw_update = KATTACH_FW_CH_NAT_PREROUTING;
	} else if (f == 2) {
		chain = &kattach_fw_shm->nat.output;
		fw_update = KATTACH_FW_CH_NAT_OUTPUT;
	} else if (f == 3) {
		chain = &kattach_fw_shm->nat.postrouting;
		fw_update = KATTACH_FW_CH_NAT_POSTROUTING;
	} else {
		printf("\nERROR occured - chain unknown\n");
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
		return;
	}

	sprintf(cliask,"Edit NAT Rule: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
			return;
		}
		iindex = atol(clians);
		y++;
		break;	
	}

	if ((chain->filter[iindex].nindex == 0) && (chain->filter[iindex].pindex == 0) && (chain->filter[iindex].enabled == 0)) {
		printf("\nFilter ID %lu is not configured.\n",iindex);
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
		return;
	}

	d = 0;
	y = 0;
	sprintf(cliask,"Source Zone (? to list): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_net_fw_zones_list();
		} else {
			for (zindex = 0; zindex < kattach_fw_shm->zones.index; zindex++) {
				if (kattach_fw_shm->zones.zone[zindex].name[0] == '\0') continue;
				if ((strlen(clians)) != (strlen(kattach_fw_shm->zones.zone[zindex].name))) continue;
				if (!strncmp(clians,kattach_fw_shm->zones.zone[zindex].name,strlen(clians))) {
					d = 1;
					y++;
					break;
				}
			}
			if (d) {
				break;
			} else {
				printf("\nZone %s not found. Please check and retry\n",clians);
			}
		}
	}
	fw.szindex = zindex;
	d = 0;
	y = 0;
	sprintf(cliask,"Destination Zone (? to list): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_net_fw_zones_list();
		} else {
			for (zindex = 0; zindex < kattach_fw_shm->zones.index; zindex++) {
				if (kattach_fw_shm->zones.zone[zindex].name[0] == '\0') continue;
				if (strlen(clians) != strlen(kattach_fw_shm->zones.zone[zindex].name)) continue;
				if (!strncmp(clians,kattach_fw_shm->zones.zone[zindex].name,strlen(clians))) {
					d = 1;
					y++;
					break;
				}
			}
			if (d) {
				break;
			} else {
				printf("\nZone %s not found. Please check and retry\n",clians);
			}
		}
	}
	fw.dzindex = zindex;
	d = 0;
	y = 0;
	sprintf(cliask,"App Profile (? to list): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_net_fw_apps_list();
		} else {
			for (zindex = 0; zindex < kattach_fw_shm->apps.index; zindex++) {
				if (kattach_fw_shm->apps.app[zindex].name[0] == '\0') continue;
				if (strlen(clians) != strlen(kattach_fw_shm->apps.app[zindex].name)) continue;
				if (!strncmp(clians,kattach_fw_shm->apps.app[zindex].name,strlen(clians))) {
					d = 1;
					y++;
					break;
				}
			}
			if (d) {
				break;
			} else {
				printf("\nApp %s not found. Please check and retry\n",clians);
			}
		}
	}
	fw.appindex = zindex;
	y = 0;
	d = 0;
	while(!y) {
		sprintf(cliask,"Enable Rate Limiting (y/n) ? ");
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
			return;
		}
		if (clians[0] == 'y') {
			while (!d) {
				/* FIXME */
				sprintf(cliask,"Rate Limit Packets: ");
				appqueue_cli_askq(cliask,1,clians);
				fw.rlimitpkt = atoi(clians);
				sprintf(cliask,"Rate Limit Interval (seconds): ");
				appqueue_cli_askq(cliask,1,clians);
				fw.rlimitint = atoi(clians);
				d++;
			}
			y++;
			break;
		} else if (clians[0] == 'n') {
			fw.rlimitpkt = 0;
			fw.rlimitint = 0;
			y++;
			break;
		}
	}
	y = 0;
	/* action */
	if (f == 3) {
		sprintf(cliask,"Filter Action - (a)llow, (d)rop, (r)eject, (l)og, (s)ource nat, (m)asquerade, netma(p) : ");
	} else {
		sprintf(cliask,"Filter Action - (a)llow, (d)rop, (r)eject, (l)og, destination (n)at, r(e)direct, netma(p) : ");
	}
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
			return;
		}
		if (clians[0] == 'a') {
			fw.action = KATTACH_FW_ACTION_ALLOW;
			y++;
			break;
		} else if (clians[0] == 'd') {
			fw.action = KATTACH_FW_ACTION_DROP;
			y++;
			break;
		} else if (clians[0] == 'r') {
			fw.action = KATTACH_FW_ACTION_REJECT;
			/* FIXME: add capability to select REJECT with */
			y++;
			break;
		} else if (clians[0] == 'l') {
			fw.action = KATTACH_FW_ACTION_LOG;
			y++;
			break;
		} else if ((clians[0] == 's') && (f == 3)) {
			fw.action = KATTACH_FW_ACTION_SNAT;
			n = 1;
			a = 1;
			y++;
			break;
		} else if ((clians[0] == 'n') && (f == 1)) {
			fw.action = KATTACH_FW_ACTION_DNAT;
			n = 1;
			a = 1;
			y++;
			break;
		} else if ((clians[0] == 'm') && (f == 3)) {
			fw.action = KATTACH_FW_ACTION_MASQ;
			a = 1;
			y++;
			break;
		} else if ((clians[0] == 'r') && (f == 1)) {
			fw.action = KATTACH_FW_ACTION_REDIR;
			a = 1;
			y++;
			break;
		} else if ((clians[0] == 'p') && (f != 2)) {
			fw.action = KATTACH_FW_ACTION_NETMAP;
			n = 1;
			y++;
			break;
		}
	}

	d = 0;
	y = 0;
	if (n) {
		sprintf(cliask,"NAT Zone (? to list): ");
		while(!y) {
			memset(clians,0,strlen(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (strlen(clians) == 0) {
				appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
				return;
			}
			if (clians[0] == '?') {
				appqueue_cli_mf_net_fw_zones_list();
			} else {
				for (zindex = 0; zindex < kattach_fw_shm->zones.index; zindex++) {
					if (kattach_fw_shm->zones.zone[zindex].name[0] == '\0') continue;
					if (strlen(clians) != strlen(kattach_fw_shm->zones.zone[zindex].name)) continue;
					if (!strncmp(clians,kattach_fw_shm->zones.zone[zindex].name,strlen(clians))) {
						d = 1;
						y++;
						break;
					}
				}
				if (d) {
					break;
				} else {
					printf("\nZone %s not found. Please check and retry\n",clians);
				}
			}
		}
	}
	fw.nzindex = zindex;
	d = 0;
	y = 0;
	if (a) {
		sprintf(cliask,"NAT App Profile (? to list): ");
		while(!y) {
			memset(clians,0,strlen(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (strlen(clians) == 0) {
				appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
				return;
			}
			if (clians[0] == '?') {
				appqueue_cli_mf_net_fw_apps_list();
			} else {
				for (zindex = 0; zindex < kattach_fw_shm->apps.index; zindex++) {
					if (kattach_fw_shm->apps.app[zindex].name[0] == '\0') continue;
					if (strlen(clians) != strlen(kattach_fw_shm->apps.app[zindex].name)) continue;
					if (!strncmp(clians,kattach_fw_shm->apps.app[zindex].name,strlen(clians))) {
						d = 1;
						y++;
						break;
					}
				}
				if (d) {
					break;
				} else {
					printf("\nApp %s not found. Please check and retry\n",clians);
				}
			}
		}
	}
	fw.nappindex = zindex;

	appqueue_cli_lock_fw();
	chain->filter[iindex].reverse = 0;
	chain->filter[iindex].logging = 0;
	chain->filter[iindex].rejectwith = 0;
	chain->filter[iindex].type = 0;
	chain->filter[iindex].enabled = 1;
	chain->filter[iindex].tos[0] = 0;
	chain->filter[iindex].tos[1] = 0;
	chain->filter[iindex].ttl[0] = 0;
	chain->filter[iindex].ttl[1] = 0;
	chain->filter[iindex].action = fw.action;
	chain->filter[iindex].rlimitint = fw.rlimitint;
	chain->filter[iindex].rlimitpkt = fw.rlimitpkt;
	chain->filter[iindex].appindex = fw.appindex;
	chain->filter[iindex].nappindex = fw.nappindex;
	chain->filter[iindex].dzindex = fw.dzindex;
	chain->filter[iindex].szindex = fw.szindex;
	chain->filter[iindex].nzindex = fw.nzindex;
	chain->index++;
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
	return;
}

void
appqueue_cli_mf_net_fw_nat_list(void)
{
	char cliask[64];
	char clians[64];
	kattach_fw_n_chain_t *chain;
	u32 index;
	u8 y = 0, l = 0;

	chain = &kattach_fw_shm->nat.prerouting;
	printf("\n[PREROUTING firewall (policy: ACCEPT)]\n\n");
	index = chain->hindex;
	printf(" Index           Source Zone     Destination Zone     Application     Action    Flags              NAT Zone     Application    Rate Limiting\n");
	printf("------    ------------------    -----------------    ------------    -------    -----    ------------------    ------------    ----------------------------------\n");
	while (!y) {
		if ((chain->index == 0) && (chain->hindex == 0) && (chain->eindex == 0)) {
			y++;
			break;
		}
		l++;
		printf("%5lu:    %18s   %18s      %10s     %6s    %c:%c:%c    %18s      %10s    ", index, kattach_fw_shm->zones.zone[chain->filter[index].szindex].name, kattach_fw_shm->zones.zone[chain->filter[index].dzindex].name,
						kattach_fw_shm->apps.app[chain->filter[index].appindex].name, ((chain->filter[index].action == KATTACH_FW_ACTION_ALLOW) ? " allow" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_DROP) ? "  drop" : ((chain->filter[index].action == KATTACH_FW_ACTION_REJECT) ? "reject" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_LOG) ? "   log" : ((chain->filter[index].action == KATTACH_FW_ACTION_DNAT) ? "  dnat" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_REDIR) ? " redir" : ((chain->filter[index].action == KATTACH_FW_ACTION_NETMAP) ? "netmap" :
						"  none"))))))), ((chain->filter[index].reverse == 1) ? 'R' : '-'),
						((chain->filter[index].logging == 1) ? 'L' : '-'), ((chain->filter[index].enabled == 1) ? 'E' : '-'), kattach_fw_shm->zones.zone[chain->filter[index].nzindex].name,
						kattach_fw_shm->apps.app[chain->filter[index].nappindex].name);
		if (chain->filter[index].rlimitpkt) {
			if (chain->filter[index].rlimitint <= 59) {
				printf("%8lu pkts per second\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint <= 3599)) {
				printf("%8lu pkts per minute\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 3600) && (chain->filter[index].rlimitint <= 86399)) {
				printf("%8lu pkts per hour\n",chain->filter[index].rlimitpkt);
			} else {
				printf("%8lu pkts per day\n",chain->filter[index].rlimitpkt);
			}
		} else {
			printf("\n");
		}
		if (index != chain->eindex) {
			index = chain->filter[index].nindex;
		} else {
			y++;
			break;
		}
		if (l >= APPQUEUE_CLI_LINES) {
			sprintf(cliask,"\nPress Enter to continue or q to quit... ");
			appqueue_cli_askq(cliask,1,clians);
			if (clians[0] == 'q') {
				appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
				return;
			} else {
				l = 0;
				printf("\n");
			}
		}
	}	
	printf("\n\n");
	y = 0;
	chain = &kattach_fw_shm->nat.output;
	printf("\n[OUTPUT firewall (policy: ACCEPT)]\n\n");
	index = chain->hindex;
	printf(" Index           Source Zone     Destination Zone     Application     Action    Flags              NAT Zone     Application    Rate Limiting\n");
	printf("------    ------------------    -----------------    ------------    -------    -----    ------------------    ------------    ----------------------------------\n");
	while (!y) {
		if ((chain->index == 0) && (chain->hindex == 0) && (chain->eindex == 0)) {
			y++;
			break;
		}
		l++;
		printf("%5lu:    %18s   %18s      %10s     %6s    %c:%c:%c    %18s    %10s    ", index, kattach_fw_shm->zones.zone[chain->filter[index].szindex].name, kattach_fw_shm->zones.zone[chain->filter[index].dzindex].name,
						kattach_fw_shm->apps.app[chain->filter[index].appindex].name, ((chain->filter[index].action == KATTACH_FW_ACTION_ALLOW) ? " allow" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_DROP) ? "  drop" : ((chain->filter[index].action == KATTACH_FW_ACTION_REJECT) ? "reject" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_LOG) ? "   log" : ((chain->filter[index].action == KATTACH_FW_ACTION_DNAT) ? "  dnat" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_REDIR) ? " redir" : ((chain->filter[index].action == KATTACH_FW_ACTION_NETMAP) ? "netmap" :
						"  none"))))))), ((chain->filter[index].reverse == 1) ? 'R' : '-'),
						((chain->filter[index].logging == 1) ? 'L' : '-'), ((chain->filter[index].enabled == 1) ? 'E' : '-'), kattach_fw_shm->zones.zone[chain->filter[index].nzindex].name,
						kattach_fw_shm->apps.app[chain->filter[index].nappindex].name);
		if (chain->filter[index].rlimitpkt) {
			if (chain->filter[index].rlimitint <= 59) {
				printf("%8lu pkts per second\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint <= 3599)) {
				printf("%8lu pkts per minute\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 3600) && (chain->filter[index].rlimitint <= 86399)) {
				printf("%8lu pkts per hour\n",chain->filter[index].rlimitpkt);
			} else {
				printf("%8lu pkts per day\n",chain->filter[index].rlimitpkt);
			}
		} else {
			printf("\n");
		}
		if (index != chain->eindex) {
			index = chain->filter[index].nindex;
		} else {
			y++;
			break;
		}
		if (l >= APPQUEUE_CLI_LINES) {
			sprintf(cliask,"\nPress Enter to continue or q to quit... ");
			appqueue_cli_askq(cliask,1,clians);
			if (clians[0] == 'q') {
				appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
				return;
			} else {
				l = 0;
				printf("\n");
			}
		}
	}	
	printf("\n\n");
	y = 0;
	chain = &kattach_fw_shm->nat.postrouting;
	printf("\n[POSTROUTING firewall (policy: ACCEPT)]\n\n");
	index = chain->hindex;
	printf(" Index           Source Zone     Destination Zone     Application     Action    Flags              NAT Zone     Application    Rate Limiting\n");
	printf("------    ------------------    -----------------    ------------    -------    -----    ------------------    ------------    ----------------------------------\n");
	while (!y) {
		if ((chain->index == 0) && (chain->hindex == 0) && (chain->eindex == 0)) {
			y++;
			break;
		}
		l++;
		printf("%5lu:    %18s   %18s      %10s     %6s    %c:%c:%c    %18s    %10s    ", index, kattach_fw_shm->zones.zone[chain->filter[index].szindex].name, kattach_fw_shm->zones.zone[chain->filter[index].dzindex].name,
						kattach_fw_shm->apps.app[chain->filter[index].appindex].name, ((chain->filter[index].action == KATTACH_FW_ACTION_ALLOW) ? " allow" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_DROP) ? "  drop" : ((chain->filter[index].action == KATTACH_FW_ACTION_REJECT) ? "reject" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_LOG) ? "   log" : ((chain->filter[index].action == KATTACH_FW_ACTION_SNAT) ? "  snat" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_MASQ) ? "  masq" : ((chain->filter[index].action == KATTACH_FW_ACTION_NETMAP) ? "netmap" :
						"  none"))))))), ((chain->filter[index].reverse == 1) ? 'R' : '-'),
						((chain->filter[index].logging == 1) ? 'L' : '-'), ((chain->filter[index].enabled == 1) ? 'E' : '-'), kattach_fw_shm->zones.zone[chain->filter[index].nzindex].name,
						kattach_fw_shm->apps.app[chain->filter[index].nappindex].name);
		if (chain->filter[index].rlimitpkt) {
			if (chain->filter[index].rlimitint <= 59) {
				printf("%8lu pkts per second\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint <= 3599)) {
				printf("%8lu pkts per minute\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 3600) && (chain->filter[index].rlimitint <= 86399)) {
				printf("%8lu pkts per hour\n",chain->filter[index].rlimitpkt);
			} else {
				printf("%8lu pkts per day\n",chain->filter[index].rlimitpkt);
			}
		} else {
			printf("\n");
		}
		if (index != chain->eindex) {
			index = chain->filter[index].nindex;
		} else {
			y++;
			break;
		}
		if (l >= APPQUEUE_CLI_LINES) {
			sprintf(cliask,"\nPress Enter to continue or q to quit... ");
			appqueue_cli_askq(cliask,1,clians);
			if (clians[0] == 'q') {
				appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
				return;
			} else {
				l = 0;
				printf("\n");
			}
		}
	}	
	printf("\n\n");

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
	return;
}


void
appqueue_cli_mf_net_fw_nat_reverse(void)
{
	char cliask[255];
	char clians[255];
	u32 iindex = 0;
	u16 fw_update = 0;
	u8 y = 0, f = 0;
	kattach_fw_n_chain_t *chain;

	sprintf(cliask,"Reverse Firewall Rule in pre(r)outing, (o)utbound or (p)ostrouting: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'r') {
			f = 1;
		} else if (clians[0] == 'o') {
			f = 2;
		} else if (clians[0] == 'p') {
			f = 3;
		}
		if (f) {
			y++;
			break;
		}
	}
	y = 0;

	sprintf(cliask,"Reverse Rule Index: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
			return;
		}
		iindex = atol(clians);
		y++;
		break;	
	}

	if (f == 1) {
		chain = &kattach_fw_shm->nat.prerouting;
		fw_update = KATTACH_FW_CH_NAT_PREROUTING;
	} else if (f == 2) {
		chain = &kattach_fw_shm->nat.output;
		fw_update = KATTACH_FW_CH_NAT_OUTPUT;
	} else if (f == 3) {
		chain = &kattach_fw_shm->nat.postrouting;
		fw_update = KATTACH_FW_CH_NAT_POSTROUTING;
	} else {
		printf("\nERROR occured - chain unknown\n");
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
		return;
	}

	if ((chain->filter[iindex].nindex == 0) && (chain->filter[iindex].pindex == 0) && (chain->filter[iindex].enabled == 0)) {
		printf("\nFilter ID %lu is not configured.\n",iindex);
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
		return;
	}

	appqueue_cli_lock_fw();
	if (chain->filter[iindex].reverse == 0) {
		chain->filter[iindex].reverse = 1;
	} else {
		chain->filter[iindex].reverse = 0;
	}

	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
	return;
}


void
appqueue_cli_mf_net_fw_nat_log(void)
{
	char cliask[255];
	char clians[255];
	u32 iindex = 0;
	u16 fw_update = 0;
	u8 y = 0, f = 0;
	kattach_fw_n_chain_t *chain;

	sprintf(cliask,"Log Firewall Rule in pre(r)outing, (o)utbound or (p)ostrouting: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'r') {
			f = 1;
		} else if (clians[0] == 'o') {
			f = 2;
		} else if (clians[0] == 'p') {
			f = 3;
		}
		if (f) {
			y++;
			break;
		}
	}
	y = 0;

	sprintf(cliask,"Log Rule Index: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
			return;
		}
		iindex = atol(clians);
		y++;
		break;	
	}
	y = 0;
	sprintf(cliask,"Tag rule with: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) <= 32) {
			y++;
			break;
		}
	}		

	if (f == 1) {
		chain = &kattach_fw_shm->nat.prerouting;
		fw_update = KATTACH_FW_CH_NAT_PREROUTING;
	} else if (f == 2) {
		chain = &kattach_fw_shm->nat.output;
		fw_update = KATTACH_FW_CH_NAT_OUTPUT;
	} else if (f == 3) {
		chain = &kattach_fw_shm->nat.postrouting;
		fw_update = KATTACH_FW_CH_NAT_POSTROUTING;
	} else {
		printf("\nERROR occured - chain unknown\n");
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
		return;
	}

	if ((chain->filter[iindex].nindex == 0) && (chain->filter[iindex].pindex == 0) && (chain->filter[iindex].enabled == 0)) {
		printf("\nFilter ID %lu is not configured.\n",iindex);
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
		return;
	}

	appqueue_cli_lock_fw();
	if (chain->filter[iindex].logging == 0) {
		chain->filter[iindex].logging = 1;
	} else {
		chain->filter[iindex].logging = 0;
	}
	sprintf(chain->filter[iindex].logprefix,"%s",clians);
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_NAT;
	return;
}


void
appqueue_cli_mf_net_fw_filter(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_net_fw_filter.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_net_fw_filter.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_net_fw_filter.climenu[i].menu_cmd,appqueue_cli_menu_net_fw_filter.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sPkt Filtering]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
	appqueue_po = 0;
	return;
}
void
appqueue_cli_mf_net_fw_filter_add(void)
{
	char cliask[255];
	char clians[255];
	u32 iindex = 0, zindex = 0;
	u16 fw_update = 0;
	u8 y = 0, d = 0, f = 0;
	kattach_fw_entry_t fw;
	kattach_fw_chain_t *chain;

	sprintf(cliask,"Add Firewall Rule to (i)nbound, (o)utbound or (f)orward: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'i') {
			f = 1;
		} else if (clians[0] == 'o') {
			f = 2;
		} else if (clians[0] == 'f') {
			f = 3;
		}
		if (f) {
			y++;
			break;
		}
	}
	y = 0;

	sprintf(cliask,"Index to Insert New Rule after: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
			return;
		}
		iindex = atol(clians);
		y++;
		break;	
	}
	d = 0;
	y = 0;
	sprintf(cliask,"Source Zone (? to list): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_net_fw_zones_list();
		} else {
			for (zindex = 0; zindex < kattach_fw_shm->zones.index; zindex++) {
				if (kattach_fw_shm->zones.zone[zindex].name[0] == '\0') continue;
				if ((strlen(clians)) != (strlen(kattach_fw_shm->zones.zone[zindex].name))) continue;
				if (!strncmp(clians,kattach_fw_shm->zones.zone[zindex].name,strlen(clians))) {
					d = 1;
					y++;
					break;
				}
			}
			if (d) {
				break;
			} else {
				printf("\nZone %s not found. Please check and retry\n",clians);
			}
		}
	}
	fw.szindex = zindex;
	d = 0;
	y = 0;
	sprintf(cliask,"Destination Zone (? to list): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_net_fw_zones_list();
		} else {
			for (zindex = 0; zindex < kattach_fw_shm->zones.index; zindex++) {
				if (kattach_fw_shm->zones.zone[zindex].name[0] == '\0') continue;
				if (strlen(clians) != strlen(kattach_fw_shm->zones.zone[zindex].name)) continue;
				if (!strncmp(clians,kattach_fw_shm->zones.zone[zindex].name,strlen(clians))) {
					d = 1;
					y++;
					break;
				}
			}
			if (d) {
				break;
			} else {
				printf("\nZone %s not found. Please check and retry\n",clians);
			}
		}
	}
	fw.dzindex = zindex;
	d = 0;
	y = 0;
	sprintf(cliask,"App Profile (? to list): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_net_fw_apps_list();
		} else {
			for (zindex = 0; zindex < kattach_fw_shm->apps.index; zindex++) {
				if (kattach_fw_shm->apps.app[zindex].name[0] == '\0') continue;
				if (strlen(clians) != strlen(kattach_fw_shm->apps.app[zindex].name)) continue;
				if (!strncmp(clians,kattach_fw_shm->apps.app[zindex].name,strlen(clians))) {
					d = 1;
					y++;
					break;
				}
			}
			if (d) {
				break;
			} else {
				printf("\nApp %s not found. Please check and retry\n",clians);
			}
		}
	}
	fw.appindex = zindex;
	y = 0;
	d = 0;
	while(!y) {
		sprintf(cliask,"Enable Rate Limiting (y/n) ? ");
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
			return;
		}
		if (clians[0] == 'y') {
			while (!d) {
				/* FIXME */
				sprintf(cliask,"Rate Limit Packets: ");
				appqueue_cli_askq(cliask,1,clians);
				fw.rlimitpkt = atoi(clians);
				sprintf(cliask,"Rate Limit Interval (seconds): ");
				appqueue_cli_askq(cliask,1,clians);
				fw.rlimitint = atoi(clians);
				d++;
			}
			y++;
			break;
		} else if (clians[0] == 'n') {
			fw.rlimitpkt = 0;
			fw.rlimitint = 0;
			y++;
			break;
		}
	}
	y = 0;
	/* action */
	sprintf(cliask,"Filter Action - (a)llow, (d)rop, (r)eject, (l)og: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
			return;
		}
		if (clians[0] == 'a') {
			fw.action = KATTACH_FW_ACTION_ALLOW;
			y++;
			break;
		} else if (clians[0] == 'd') {
			fw.action = KATTACH_FW_ACTION_DROP;
			y++;
			break;
		} else if (clians[0] == 'r') {
			fw.action = KATTACH_FW_ACTION_REJECT;
			/* FIXME: add capability to select REJECT with */
			y++;
			break;
		} else if (clians[0] == 'l') {
			fw.action = KATTACH_FW_ACTION_LOG;
			y++;
			break;
		}
	}

	if (f == 1) {
		chain = &kattach_fw_shm->filter.input;
		fw_update = KATTACH_FW_CH_FILTER_INPUT;
	} else if (f == 2) {
		chain = &kattach_fw_shm->filter.output;
		fw_update = KATTACH_FW_CH_FILTER_OUTPUT;
	} else if (f == 3) {
		chain = &kattach_fw_shm->filter.forward;
		fw_update = KATTACH_FW_CH_FILTER_FORWARD;
	} else {
		printf("\nERROR occured - chain unknown\n");
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
		return;
	}

	appqueue_cli_lock_fw();
	if ((chain->index == 0) && (chain->eindex == chain->hindex)) {
		/* this is the first rule */
		chain->eindex = chain->index;
		chain->hindex = chain->index;
		chain->filter[chain->index].nindex = chain->index;
		chain->filter[chain->index].pindex = chain->index;
	} else if ((iindex == chain->eindex) || (iindex >= chain->index)) {
		/* this puts the rule at the end of the chain */
		chain->filter[chain->eindex].nindex = chain->index;
		chain->filter[chain->index].pindex = chain->eindex;
		chain->filter[chain->index].nindex = chain->index;
		chain->eindex = chain->index;
	} else {
		/* this puts the rule after rule X */
		chain->filter[chain->index].pindex = iindex;
		chain->filter[chain->index].nindex = chain->filter[iindex].nindex;
		chain->filter[iindex].nindex = chain->index;
		chain->filter[chain->filter[chain->index].nindex].pindex = chain->index;
	}
	chain->filter[chain->index].reverse = 0;
	chain->filter[chain->index].logging = 0;
	chain->filter[chain->index].rejectwith = 0;
	chain->filter[chain->index].type = 0;
	chain->filter[chain->index].enabled = 1;
	chain->filter[chain->index].tos[0] = 0;
	chain->filter[chain->index].tos[1] = 0;
	chain->filter[chain->index].ttl[0] = 0;
	chain->filter[chain->index].ttl[1] = 0;
	chain->filter[chain->index].action = fw.action;
	chain->filter[chain->index].rlimitint = fw.rlimitint;
	chain->filter[chain->index].rlimitpkt = fw.rlimitpkt;
	chain->filter[chain->index].appindex = fw.appindex;
	chain->filter[chain->index].dzindex = fw.dzindex;
	chain->filter[chain->index].szindex = fw.szindex;
	chain->index++;
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
	return;
}


void
appqueue_cli_mf_net_fw_filter_del(void)
{
	char cliask[255];
	char clians[255];
	u32 iindex = 0;
	u16 fw_update = 0;
	u8 y = 0, f = 0;
	kattach_fw_chain_t *chain;

	sprintf(cliask,"Delete Firewall Rule from (i)nbound, (o)utbound or (f)orward: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'i') {
			f = 1;
		} else if (clians[0] == 'o') {
			f = 2;
		} else if (clians[0] == 'f') {
			f = 3;
		}
		if (f) {
			y++;
			break;
		}
	}
	y = 0;

	sprintf(cliask,"Delete Rule Index: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
			return;
		}
		iindex = atol(clians);
		y++;
		break;	
	}

	if (f == 1) {
		chain = &kattach_fw_shm->filter.input;
		fw_update = KATTACH_FW_CH_FILTER_INPUT;
	} else if (f == 2) {
		chain = &kattach_fw_shm->filter.output;
		fw_update = KATTACH_FW_CH_FILTER_OUTPUT;
	} else if (f == 3) {
		chain = &kattach_fw_shm->filter.forward;
		fw_update = KATTACH_FW_CH_FILTER_FORWARD;
	} else {
		printf("\nERROR occured - chain unknown\n");
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
		return;
	}

	if ((chain->filter[iindex].nindex == 0) && (chain->filter[iindex].pindex == 0) && (chain->filter[iindex].enabled == 0)) {
		printf("\nFilter ID %lu is not configured.\n",iindex);
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
		return;
	}

	appqueue_cli_lock_fw();
	if (chain->hindex == iindex) {
		/* this filter is head of the chain */
		chain->hindex = chain->filter[iindex].nindex;
		chain->filter[chain->hindex].pindex = chain->hindex;
		if (chain->eindex == iindex) {
			chain->eindex = chain->filter[iindex].pindex;
			chain->filter[chain->eindex].nindex = chain->eindex;
			if (chain->index) {
				chain->index--;
			}
		}
	} else if (chain->eindex == iindex) {
		/* this filter is end of the chain */
		chain->eindex = chain->filter[iindex].pindex;
		chain->filter[chain->eindex].nindex = chain->eindex;
		if (chain->index) {
			chain->index--;
		}
	} else {
		/* this filter is between filters */
		chain->filter[chain->filter[iindex].pindex].nindex = chain->filter[iindex].nindex;
		chain->filter[chain->filter[iindex].nindex].pindex = chain->filter[iindex].pindex;
	}
	chain->filter[iindex].reverse = 0;
	chain->filter[iindex].logging = 0;
	chain->filter[iindex].rejectwith = 0;
	chain->filter[iindex].type = 0;
	chain->filter[iindex].enabled = 0;
	chain->filter[iindex].tos[0] = 0;
	chain->filter[iindex].tos[1] = 0;
	chain->filter[iindex].ttl[0] = 0;
	chain->filter[iindex].ttl[1] = 0;
	chain->filter[iindex].action = 0;
	chain->filter[iindex].rlimitint = 0;
	chain->filter[iindex].rlimitpkt = 0;
	chain->filter[iindex].appindex = 0;
	chain->filter[iindex].dzindex = 0;
	chain->filter[iindex].szindex = 0;
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
	return;
}


void
appqueue_cli_mf_net_fw_filter_edit(void)
{
	char cliask[255];
	char clians[255];
	u32 iindex = 0, zindex = 0;
	u16 fw_update = 0;
	u8 y = 0, d = 0, f = 0;
	kattach_fw_entry_t fw;
	kattach_fw_chain_t *chain;

	sprintf(cliask,"Edit Firewall Rule in (i)nbound, (o)utbound or (f)orward: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'i') {
			f = 1;
		} else if (clians[0] == 'o') {
			f = 2;
		} else if (clians[0] == 'f') {
			f = 3;
		}
		if (f) {
			y++;
			break;
		}
	}
	y = 0;

	if (f == 1) {
		chain = &kattach_fw_shm->filter.input;
		fw_update = KATTACH_FW_CH_FILTER_INPUT;
	} else if (f == 2) {
		chain = &kattach_fw_shm->filter.output;
		fw_update = KATTACH_FW_CH_FILTER_OUTPUT;
	} else if (f == 3) {
		chain = &kattach_fw_shm->filter.forward;
		fw_update = KATTACH_FW_CH_FILTER_FORWARD;
	} else {
		printf("\nERROR occured - chain unknown\n");
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
		return;
	}

	sprintf(cliask,"Edit Rule: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
			return;
		}
		iindex = atol(clians);
		y++;
		break;	
	}

	if ((chain->filter[iindex].nindex == 0) && (chain->filter[iindex].pindex == 0) && (chain->filter[iindex].enabled == 0)) {
		printf("\nFilter ID %lu is not configured.\n",iindex);
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
		return;
	}

	d = 0;
	y = 0;
	sprintf(cliask,"Source Zone (? to list): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_net_fw_zones_list();
		} else {
			for (zindex = 0; zindex < kattach_fw_shm->zones.index; zindex++) {
				if (kattach_fw_shm->zones.zone[zindex].name[0] == '\0') continue;
				if ((strlen(clians)) != (strlen(kattach_fw_shm->zones.zone[zindex].name))) continue;
				if (!strncmp(clians,kattach_fw_shm->zones.zone[zindex].name,strlen(clians))) {
					d = 1;
					y++;
					break;
				}
			}
			if (d) {
				break;
			} else {
				printf("\nZone %s not found. Please check and retry\n",clians);
			}
		}
	}
	fw.szindex = zindex;
	d = 0;
	y = 0;
	sprintf(cliask,"Destination Zone (? to list): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_net_fw_zones_list();
		} else {
			for (zindex = 0; zindex < kattach_fw_shm->zones.index; zindex++) {
				if (kattach_fw_shm->zones.zone[zindex].name[0] == '\0') continue;
				if (strlen(clians) != strlen(kattach_fw_shm->zones.zone[zindex].name)) continue;
				if (!strncmp(clians,kattach_fw_shm->zones.zone[zindex].name,strlen(clians))) {
					d = 1;
					y++;
					break;
				}
			}
			if (d) {
				break;
			} else {
				printf("\nZone %s not found. Please check and retry\n",clians);
			}
		}
	}
	fw.dzindex = zindex;
	d = 0;
	y = 0;
	sprintf(cliask,"App Profile (? to list): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_net_fw_apps_list();
		} else {
			for (zindex = 0; zindex < kattach_fw_shm->apps.index; zindex++) {
				if (kattach_fw_shm->apps.app[zindex].name[0] == '\0') continue;
				if (strlen(clians) != strlen(kattach_fw_shm->apps.app[zindex].name)) continue;
				if (!strncmp(clians,kattach_fw_shm->apps.app[zindex].name,strlen(clians))) {
					d = 1;
					y++;
					break;
				}
			}
			if (d) {
				break;
			} else {
				printf("\nZone %s not found. Please check and retry\n",clians);
			}
		}
	}
	fw.appindex = zindex;
	y = 0;
	d = 0;
	while(!y) {
		sprintf(cliask,"Enable Rate Limiting (y/n) ? ");
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
			return;
		}
		if (clians[0] == 'y') {
			while (!d) {
				/* FIXME */
				sprintf(cliask,"Rate Limit Packets: ");
				appqueue_cli_askq(cliask,1,clians);
				fw.rlimitpkt = atoi(clians);
				sprintf(cliask,"Rate Limit Interval (seconds): ");
				appqueue_cli_askq(cliask,1,clians);
				fw.rlimitint = atoi(clians);
				d++;
			}
			y++;
			break;
		} else if (clians[0] == 'n') {
			fw.rlimitpkt = 0;
			fw.rlimitint = 0;
			y++;
			break;
		}
	}
	y = 0;
	/* action */
	sprintf(cliask,"Filter Action - (a)llow, (d)rop, (r)eject, (l)og: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
			return;
		}
		if (clians[0] == 'a') {
			fw.action = KATTACH_FW_ACTION_ALLOW;
			y++;
			break;
		} else if (clians[0] == 'd') {
			fw.action = KATTACH_FW_ACTION_DROP;
			y++;
			break;
		} else if (clians[0] == 'r') {
			fw.action = KATTACH_FW_ACTION_REJECT;
			/* FIXME: add capability to select REJECT with */
			y++;
			break;
		} else if (clians[0] == 'l') {
			fw.action = KATTACH_FW_ACTION_LOG;
			y++;
			break;
		}
	}

	/* FIXME: add command to move filter */
	/* filter doesn't move on edit */

	appqueue_cli_lock_fw();
	chain->filter[iindex].reverse = 0;
	chain->filter[iindex].logging = 0;
	chain->filter[iindex].rejectwith = 0;
	chain->filter[iindex].type = 0;
	chain->filter[iindex].enabled = 1;
	chain->filter[iindex].tos[0] = 0;
	chain->filter[iindex].tos[1] = 0;
	chain->filter[iindex].ttl[0] = 0;
	chain->filter[iindex].ttl[1] = 0;
	chain->filter[iindex].action = fw.action;
	chain->filter[iindex].rlimitint = fw.rlimitint;
	chain->filter[iindex].rlimitpkt = fw.rlimitpkt;
	chain->filter[iindex].appindex = fw.appindex;
	chain->filter[iindex].dzindex = fw.dzindex;
	chain->filter[iindex].szindex = fw.szindex;
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
	return;
}

void
appqueue_cli_mf_net_fw_filter_list(void)
{
	char cliask[64];
	char clians[64];
	kattach_fw_chain_t *chain;
	u32 index;
	u8 y = 0, l = 0;

	chain = &kattach_fw_shm->filter.input;
	printf("\n[INPUT firewall (policy: DROP)]\n\n");
	index = chain->hindex;
	printf(" Index           Source Zone     Destination Zone               Application     Action    Flags    Rate Limiting\n");
	printf("------    ------------------    -----------------    ----------------------    -------    -----    ----------------------------------\n");
	while (!y) {
		if ((chain->index == 0) && (chain->hindex == 0) && (chain->eindex == 0)) {
			y++;
			break;
		}
		l++;
		printf("%5lu:    %18s   %18s      %20s     %6s    %c:%c:%c    ", index, kattach_fw_shm->zones.zone[chain->filter[index].szindex].name, kattach_fw_shm->zones.zone[chain->filter[index].dzindex].name,
						kattach_fw_shm->apps.app[chain->filter[index].appindex].name, ((chain->filter[index].action == KATTACH_FW_ACTION_ALLOW) ? " allow" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_DROP) ? "  drop" : ((chain->filter[index].action == KATTACH_FW_ACTION_REJECT) ? "reject" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_LOG) ? "   log" : "  none")))), ((chain->filter[index].reverse == 1) ? 'R' : '-'),
						((chain->filter[index].logging == 1) ? 'L' : '-'), ((chain->filter[index].enabled == 1) ? 'E' : '-'));
		if (chain->filter[index].rlimitpkt) {
			if (chain->filter[index].rlimitint <= 59) {
				printf("%8lu pkts per second\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint <= 3599)) {
				printf("%8lu pkts per minute\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 3600) && (chain->filter[index].rlimitint <= 86399)) {
				printf("%8lu pkts per hour\n",chain->filter[index].rlimitpkt);
			} else {
				printf("%8lu pkts per day\n",chain->filter[index].rlimitpkt);
			}
		} else {
			printf("\n");
		}
		if (index != chain->eindex) {
			index = chain->filter[index].nindex;
		} else {
			y++;
			break;
		}
		if (l >= APPQUEUE_CLI_LINES) {
			sprintf(cliask,"\nPress Enter to continue or q to quit... ");
			appqueue_cli_askq(cliask,1,clians);
			if (clians[0] == 'q') {
				appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
				return;
			} else {
				l = 0;
				printf("\n");
			}
		}
	}	
	printf("\n\n");

	chain = &kattach_fw_shm->filter.output;
	printf("\n[OUTPUT firewall (policy: DROP)]\n\n");
	index = chain->hindex;
	printf(" Index           Source Zone     Destination Zone               Application     Action    Flags    Rate Limiting\n");
	printf("------    ------------------    -----------------    ----------------------    -------    -----    ----------------------------------\n");
	y = 0;
	while (!y) {
		if ((chain->index == 0) && (chain->hindex == 0) && (chain->eindex == 0)) {
			y++;
			break;
		}
		l++;
		printf("%5lu:    %18s   %18s      %20s     %6s    %c:%c:%c    ", index, kattach_fw_shm->zones.zone[chain->filter[index].szindex].name, kattach_fw_shm->zones.zone[chain->filter[index].dzindex].name,
						kattach_fw_shm->apps.app[chain->filter[index].appindex].name, ((chain->filter[index].action == KATTACH_FW_ACTION_ALLOW) ? " allow" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_DROP) ? "  drop" : ((chain->filter[index].action == KATTACH_FW_ACTION_REJECT) ? "reject" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_LOG) ? "   log" : "  none")))), ((chain->filter[index].reverse == 1) ? 'R' : '-'),
						((chain->filter[index].logging == 1) ? 'L' : '-'), ((chain->filter[index].enabled == 1) ? 'E' : '-'));
		if (chain->filter[index].rlimitpkt) {
			if (chain->filter[index].rlimitint <= 59) {
				printf("%8lu pkts per second\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint <= 3599)) {
				printf("%8lu pkts per minute\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 3600) && (chain->filter[index].rlimitint <= 86399)) {
				printf("%8lu pkts per hour\n",chain->filter[index].rlimitpkt);
			} else {
				printf("%8lu pkts per day\n",chain->filter[index].rlimitpkt);
			}
		} else {
			printf("\n");
		}
		if (index != chain->eindex) {
			index = chain->filter[index].nindex;
		} else {
			y++;
			break;
		}
		if (l >= APPQUEUE_CLI_LINES) {
			sprintf(cliask,"\nPress Enter to continue or q to quit... ");
			appqueue_cli_askq(cliask,1,clians);
			if (clians[0] == 'q') {
				appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
				return;
			} else {
				l = 0;
				printf("\n");
			}
		}
	}	
	printf("\n\n");

	chain = &kattach_fw_shm->filter.forward;
	printf("\n[FORWARDING firewall (policy: DROP)]\n\n");
	index = chain->hindex;
	printf(" Index           Source Zone     Destination Zone               Application     Action    Flags    Rate Limiting\n");
	printf("------    ------------------    -----------------    ----------------------    -------    -----    ----------------------------------\n");
	y = 0;
	while (!y) {
		if ((chain->index == 0) && (chain->hindex == 0) && (chain->eindex == 0)) {
			y++;
			break;
		}
		l++;
		printf("%5lu:    %18s   %18s      %20s     %6s    %c:%c:%c    ", index, kattach_fw_shm->zones.zone[chain->filter[index].szindex].name, kattach_fw_shm->zones.zone[chain->filter[index].dzindex].name,
						kattach_fw_shm->apps.app[chain->filter[index].appindex].name, ((chain->filter[index].action == KATTACH_FW_ACTION_ALLOW) ? " allow" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_DROP) ? "  drop" : ((chain->filter[index].action == KATTACH_FW_ACTION_REJECT) ? "reject" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_LOG) ? "   log" : "  none")))), ((chain->filter[index].reverse == 1) ? 'R' : '-'),
						((chain->filter[index].logging == 1) ? 'L' : '-'), ((chain->filter[index].enabled == 1) ? 'E' : '-'));
		if (chain->filter[index].rlimitpkt) {
			if (chain->filter[index].rlimitint <= 59) {
				printf("%8lu pkts per second\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint <= 3599)) {
				printf("%8lu pkts per minute\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 3600) && (chain->filter[index].rlimitint <= 86399)) {
				printf("%8lu pkts per hour\n",chain->filter[index].rlimitpkt);
			} else {
				printf("%8lu pkts per day\n",chain->filter[index].rlimitpkt);
			}
		} else {
			printf("\n");
		}
		if (index != chain->eindex) {
			index = chain->filter[index].nindex;
		} else {
			y++;
			break;
		}
		if (l >= APPQUEUE_CLI_LINES) {
			sprintf(cliask,"\nPress Enter to continue or q to quit... ");
			appqueue_cli_askq(cliask,1,clians);
			if (clians[0] == 'q') {
				appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
				return;
			} else {
				l = 0;
				printf("\n");
			}
		}
	}	
	printf("\n\n");

	

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
	return;
}


void
appqueue_cli_mf_net_fw_filter_reverse(void)
{
	char cliask[255];
	char clians[255];
	u32 iindex = 0;
	u16 fw_update = 0;
	u8 y = 0, f = 0;
	kattach_fw_chain_t *chain;

	sprintf(cliask,"Reverse Firewall Rule in (i)nbound, (o)utbound or (f)orward: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'i') {
			f = 1;
		} else if (clians[0] == 'o') {
			f = 2;
		} else if (clians[0] == 'f') {
			f = 3;
		}
		if (f) {
			y++;
			break;
		}
	}
	y = 0;

	sprintf(cliask,"Reverse Rule Index: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
			return;
		}
		iindex = atol(clians);
		y++;
		break;	
	}

	if (f == 1) {
		chain = &kattach_fw_shm->filter.input;
		fw_update = KATTACH_FW_CH_FILTER_INPUT;
	} else if (f == 2) {
		chain = &kattach_fw_shm->filter.output;
		fw_update = KATTACH_FW_CH_FILTER_OUTPUT;
	} else if (f == 3) {
		chain = &kattach_fw_shm->filter.forward;
		fw_update = KATTACH_FW_CH_FILTER_FORWARD;
	} else {
		printf("\nERROR occured - chain unknown\n");
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
		return;
	}

	if ((chain->filter[iindex].nindex == 0) && (chain->filter[iindex].pindex == 0) && (chain->filter[iindex].enabled == 0)) {
		printf("\nFilter ID %lu is not configured.\n",iindex);
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
		return;
	}

	appqueue_cli_lock_fw();
	if (chain->filter[iindex].reverse == 0) {
		chain->filter[iindex].reverse = 1;
	} else {
		chain->filter[iindex].reverse = 0;
	}
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
	return;
}


void
appqueue_cli_mf_net_fw_filter_log(void)
{
	char cliask[255];
	char clians[255];
	u32 iindex = 0;
	u16 fw_update = 0;
	u8 y = 0, f = 0;
	kattach_fw_chain_t *chain;

	sprintf(cliask,"Log Firewall Rule in (i)nbound, (o)utbound or (f)orward: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'i') {
			f = 1;
		} else if (clians[0] == 'o') {
			f = 2;
		} else if (clians[0] == 'f') {
			f = 3;
		}
		if (f) {
			y++;
			break;
		}
	}
	y = 0;

	sprintf(cliask,"Log Rule Index: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
			return;
		}
		iindex = atol(clians);
		y++;
		break;	
	}
	y = 0;
	sprintf(cliask,"Tag rule with: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) <= 32) {
			y++;
			break;
		}
	}		

	if (f == 1) {
		chain = &kattach_fw_shm->filter.input;
		fw_update = KATTACH_FW_CH_FILTER_INPUT;
	} else if (f == 2) {
		chain = &kattach_fw_shm->filter.output;
		fw_update = KATTACH_FW_CH_FILTER_OUTPUT;
	} else if (f == 3) {
		chain = &kattach_fw_shm->filter.forward;
		fw_update = KATTACH_FW_CH_FILTER_FORWARD;
	} else {
		printf("\nERROR occured - chain unknown\n");
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
		return;
	}

	if ((chain->filter[iindex].nindex == 0) && (chain->filter[iindex].pindex == 0) && (chain->filter[iindex].enabled == 0)) {
		printf("\nFilter ID %lu is not configured.\n",iindex);
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
		return;
	}

	appqueue_cli_lock_fw();
	if (chain->filter[iindex].logging == 0) {
		chain->filter[iindex].logging = 1;
	} else {
		chain->filter[iindex].logging = 0;
	}
	sprintf(chain->filter[iindex].logprefix,"%s",clians);
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_FILTER;
	return;
}

void
appqueue_cli_mf_net_fw_mangle(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_net_fw_mangle.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_net_fw_mangle.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_net_fw_mangle.climenu[i].menu_cmd,appqueue_cli_menu_net_fw_mangle.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sPkt Mangling]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_net_fw_mangle_add(void)
{
	char cliask[255];
	char clians[255];
	u32 iindex = 0, zindex = 0;
	u16 fw_update = 0;
	u8 y = 0, d = 0, f = 0, m = 0;
	kattach_fw_m_entry_t fw;
	kattach_fw_m_chain_t *chain;

	/* set some optional stuff */
	fw.ttl[0] = 0;
	fw.ttl[1] = 0;
	fw.tos[0] = 0;
	fw.tos[1] = 0;
	fw.mark = 0;

	sprintf(cliask,"Add Firewall Rule to (i)nbound, (o)utbound, (f)orward, pre(r)outing, (p)ostrouting: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'i') {
			f = 1;
		} else if (clians[0] == 'o') {
			f = 2;
		} else if (clians[0] == 'f') {
			f = 3;
		} else if (clians[0] == 'r') {
			f = 4;
		} else if (clians[0] == 'p') {
			f = 5;
		}
		if (f) {
			y++;
			break;
		}
	}
	y = 0;

	sprintf(cliask,"Index to Insert New Rule after: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
			return;
		}
		iindex = atol(clians);
		y++;
		break;	
	}
	d = 0;
	y = 0;
	sprintf(cliask,"Source Zone (? to list): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_net_fw_zones_list();
		} else {
			for (zindex = 0; zindex < kattach_fw_shm->zones.index; zindex++) {
				if (kattach_fw_shm->zones.zone[zindex].name[0] == '\0') continue;
				if ((strlen(clians)) != (strlen(kattach_fw_shm->zones.zone[zindex].name))) continue;
				if (!strncmp(clians,kattach_fw_shm->zones.zone[zindex].name,strlen(clians))) {
					d = 1;
					y++;
					break;
				}
			}
			if (d) {
				break;
			} else {
				printf("\nZone %s not found. Please check and retry\n",clians);
			}
		}
	}
	fw.szindex = zindex;
	d = 0;
	y = 0;
	sprintf(cliask,"Destination Zone (? to list): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_net_fw_zones_list();
		} else {
			for (zindex = 0; zindex < kattach_fw_shm->zones.index; zindex++) {
				if (kattach_fw_shm->zones.zone[zindex].name[0] == '\0') continue;
				if (strlen(clians) != strlen(kattach_fw_shm->zones.zone[zindex].name)) continue;
				if (!strncmp(clians,kattach_fw_shm->zones.zone[zindex].name,strlen(clians))) {
					d = 1;
					y++;
					break;
				}
			}
			if (d) {
				break;
			} else {
				printf("\nZone %s not found. Please check and retry\n",clians);
			}
		}
	}
	fw.dzindex = zindex;
	d = 0;
	y = 0;
	sprintf(cliask,"App Profile (? to list): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_net_fw_apps_list();
		} else {
			for (zindex = 0; zindex < kattach_fw_shm->apps.index; zindex++) {
				if (kattach_fw_shm->apps.app[zindex].name[0] == '\0') continue;
				if (strlen(clians) != strlen(kattach_fw_shm->apps.app[zindex].name)) continue;
				if (!strncmp(clians,kattach_fw_shm->apps.app[zindex].name,strlen(clians))) {
					d = 1;
					y++;
					break;
				}
			}
			if (d) {
				break;
			} else {
				printf("\nApp %s not found. Please check and retry\n",clians);
			}
		}
	}
	fw.appindex = zindex;
	y = 0;
	d = 0;
	while(!y) {
		sprintf(cliask,"Enable Rate Limiting (y/n) ? ");
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
			return;
		}
		if (clians[0] == 'y') {
			while (!d) {
				/* FIXME */
				sprintf(cliask,"Rate Limit Packets: ");
				appqueue_cli_askq(cliask,1,clians);
				fw.rlimitpkt = atoi(clians);
				sprintf(cliask,"Rate Limit Interval (seconds): ");
				appqueue_cli_askq(cliask,1,clians);
				fw.rlimitint = atoi(clians);
				d++;
			}
			y++;
			break;
		} else if (clians[0] == 'n') {
			fw.rlimitpkt = 0;
			fw.rlimitint = 0;
			y++;
			break;
		}
	}
	y = 0;
	d = 0;
	/* action */
	sprintf(cliask,"Filter Action - (a)llow, (d)rop, (r)eject, (l)og, (m)ark, (t)tl, t(o)s: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
			return;
		}
		if (clians[0] == 'a') {
			fw.action = KATTACH_FW_ACTION_ALLOW;
			y++;
			break;
		} else if (clians[0] == 'd') {
			fw.action = KATTACH_FW_ACTION_DROP;
			y++;
			break;
		} else if (clians[0] == 'r') {
			fw.action = KATTACH_FW_ACTION_REJECT;
			/* FIXME: add capability to select REJECT with */
			y++;
			break;
		} else if (clians[0] == 'l') {
			fw.action = KATTACH_FW_ACTION_LOG;
			y++;
			break;
		} else if (clians[0] == 'm') {
			fw.action = KATTACH_FW_ACTION_MARK;
			d = 1;
			y++;
			break;
		} else if (clians[0] == 't') {
			fw.action = KATTACH_FW_ACTION_TTL;
			d = 2;
			y++;
			break;
		} else if (clians[0] == 'o') {
			fw.action = KATTACH_FW_ACTION_TOS;
			d = 3;
			y++;
			break;
		}
	}

	y = 0;
	if (d == 1) {
		sprintf(cliask,"\nPacket marking value: ");
		while(!y) {
			memset(clians,0,strlen(clians));
			appqueue_cli_askq(cliask,1,clians);
			fw.mark = atol(clians);
			if (fw.mark) {
				y++;
				break;
			}
		}
		y = 0;
		sprintf(cliask,"\nExtra Match on (t)tl, t(o)s, (n)one : ");
		while(!y) {
			memset(clians,0,strlen(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (clians[0] == 'n') {
				m = 0;
				y++;
				break;
			} else if (clians[0] == 't') {
				m = 1;
				y++;
				break;
			} else if (clians[0] == 'o') {
				m = 2;
				y++;
				break;
			}
		}
		y = 0;
		if (m == 1) {
			sprintf(cliask,"\nTTL value to match: ");
			while(!y) {
				memset(clians,0,strlen(clians));
				appqueue_cli_askq(cliask,1,clians);
				fw.ttl[1] = atol(clians);
				if (fw.ttl[1] <= 0xff) {
					fw.ttl[0] = 3;
					y++;
					break;
				}
			}
		} else if (m == 2) {
			sprintf(cliask,"\nTOS value to match: ");
			while(!y) {
				memset(clians,0,strlen(clians));
				appqueue_cli_askq(cliask,1,clians);
				fw.tos[1] = atol(clians);
				if (fw.tos[1] <= 0xff) {
					fw.tos[0] = 1;
					y++;
					break;
				}
			}
		}
	} else if (d == 2) {
		sprintf(cliask,"\nTTL (s)et, (d)ecrease, (i)ncrease : ");
		while(!y) {
			memset(clians,0,strlen(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (clians[0] == 's') {
				fw.ttl[0] = 0;
				sprintf(cliask,"\nSet TTL to (0 - 255) : ");
				y++;
				break;
			} else if (clians[0] == 'd') {
				fw.ttl[0] = 1;
				sprintf(cliask,"\nDecrease TTL by (0 - 255) : ");
				y++;
				break;
			} else if (clians[0] == 'i') {
				fw.ttl[0] = 2;
				sprintf(cliask,"\nIncrease TTL by (0 - 255) : ");
				y++;
				break;
			}
		}
		y = 0;
		while(!y) {
			memset(clians,0,strlen(clians));
			appqueue_cli_askq(cliask,1,clians);
			fw.ttl[1] = atol(clians);
			if (fw.ttl[1] <= 0xff) {
				y++;
				break;
			}
		}
	} else if (d == 3) {
		sprintf(cliask,"\nType of Service Value (0 - 255) : ");
		while(!y) {
			memset(clians,0,strlen(clians));
			appqueue_cli_askq(cliask,1,clians);
			fw.tos[1] = atol(clians);
			if (fw.tos[1] <= 0xff) {
				fw.tos[0] = 0;
				y++;
				break;
			}
		}
		
	}

	if (f == 1) {
		chain = &kattach_fw_shm->mangle.input;
		fw_update = KATTACH_FW_CH_MANGLE_INPUT;
	} else if (f == 2) {
		chain = &kattach_fw_shm->mangle.output;
		fw_update = KATTACH_FW_CH_MANGLE_OUTPUT;
	} else if (f == 3) {
		chain = &kattach_fw_shm->mangle.forward;
		fw_update = KATTACH_FW_CH_MANGLE_FORWARD;
	} else if (f == 4) {
		chain = &kattach_fw_shm->mangle.prerouting;
		fw_update = KATTACH_FW_CH_MANGLE_PREROUTING;
	} else if (f == 5) {
		chain = &kattach_fw_shm->mangle.postrouting;
		fw_update = KATTACH_FW_CH_MANGLE_POSTROUTING;
	} else {
		printf("\nERROR occured - chain unknown\n");
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
		return;
	}

	appqueue_cli_lock_fw();
	if ((chain->index == 0) && (chain->eindex == chain->hindex)) {
		/* this is the first rule */
		chain->eindex = chain->index;
		chain->hindex = chain->index;
		chain->filter[chain->index].nindex = chain->index;
		chain->filter[chain->index].pindex = chain->index;
	} else if ((iindex == chain->eindex) || (iindex >= chain->index)) {
		/* this puts the rule at the end of the chain */
		chain->filter[chain->eindex].nindex = chain->index;
		chain->filter[chain->index].pindex = chain->eindex;
		chain->filter[chain->index].nindex = chain->index;
		chain->eindex = chain->index;
	} else {
		/* this puts the rule after rule X */
		chain->filter[chain->index].pindex = iindex;
		chain->filter[chain->index].nindex = chain->filter[iindex].nindex;
		chain->filter[iindex].nindex = chain->index;
		chain->filter[chain->filter[chain->index].nindex].pindex = chain->index;
	}
	chain->filter[chain->index].reverse = 0;
	chain->filter[chain->index].logging = 0;
	chain->filter[chain->index].rejectwith = 0;
	chain->filter[chain->index].type = 0;
	chain->filter[chain->index].mark = fw.mark;
	chain->filter[chain->index].enabled = 1;
	chain->filter[chain->index].tos[0] = fw.tos[0];
	chain->filter[chain->index].tos[1] = fw.tos[1];
	chain->filter[chain->index].ttl[0] = fw.ttl[0];
	chain->filter[chain->index].ttl[1] = fw.ttl[1];
	chain->filter[chain->index].action = fw.action;
	chain->filter[chain->index].rlimitint = fw.rlimitint;
	chain->filter[chain->index].rlimitpkt = fw.rlimitpkt;
	chain->filter[chain->index].appindex = fw.appindex;
	chain->filter[chain->index].dzindex = fw.dzindex;
	chain->filter[chain->index].szindex = fw.szindex;
	chain->index++;
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
	return;
}


void
appqueue_cli_mf_net_fw_mangle_del(void)
{
	char cliask[255];
	char clians[255];
	u32 iindex = 0;
	u16 fw_update = 0;
	u8 y = 0, f = 0;
	kattach_fw_m_chain_t *chain;

	sprintf(cliask,"Delete Firewall Rule from (i)nbound, (o)utbound, (f)orward, pre(r)outing or (p)ostrouting: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'i') {
			f = 1;
		} else if (clians[0] == 'o') {
			f = 2;
		} else if (clians[0] == 'f') {
			f = 3;
		} else if (clians[0] == 'r') {
			f = 4;
		} else if (clians[0] == 'p') {
			f = 5;
		}
		if (f) {
			y++;
			break;
		}
	}
	y = 0;

	sprintf(cliask,"Delete Rule Index: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
			return;
		}
		iindex = atol(clians);
		y++;
		break;	
	}

	if (f == 1) {
		chain = &kattach_fw_shm->mangle.input;
		fw_update = KATTACH_FW_CH_MANGLE_INPUT;
	} else if (f == 2) {
		chain = &kattach_fw_shm->mangle.output;
		fw_update = KATTACH_FW_CH_MANGLE_OUTPUT;
	} else if (f == 3) {
		chain = &kattach_fw_shm->mangle.forward;
		fw_update = KATTACH_FW_CH_MANGLE_FORWARD;
	} else if (f == 4) {
		chain = &kattach_fw_shm->mangle.prerouting;
		fw_update = KATTACH_FW_CH_MANGLE_PREROUTING;
	} else if (f == 5) {
		chain = &kattach_fw_shm->mangle.postrouting;
		fw_update = KATTACH_FW_CH_MANGLE_POSTROUTING;
	} else {
		printf("\nERROR occured - chain unknown\n");
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
		return;
	}

	if ((chain->filter[iindex].nindex == 0) && (chain->filter[iindex].pindex == 0) && (chain->filter[iindex].enabled == 0)) {
		printf("\nFilter ID %lu is not configured.\n",iindex);
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
		return;
	}

	appqueue_cli_lock_fw();
	if (chain->hindex == iindex) {
		/* this filter is head of the chain */
		chain->hindex = chain->filter[iindex].nindex;
		chain->filter[chain->hindex].pindex = chain->hindex;
		if (chain->eindex == iindex) {
			chain->eindex = chain->filter[iindex].pindex;
			chain->filter[chain->eindex].nindex = chain->eindex;
			if (chain->index) {
				chain->index--;
			}
		}
	} else if (chain->eindex == iindex) {
		/* this filter is end of the chain */
		chain->eindex = chain->filter[iindex].pindex;
		chain->filter[chain->eindex].nindex = chain->eindex;
		if (chain->index) {
			chain->index--;
		}
	} else {
		/* this filter is between filters */
		chain->filter[chain->filter[iindex].pindex].nindex = chain->filter[iindex].nindex;
		chain->filter[chain->filter[iindex].nindex].pindex = chain->filter[iindex].pindex;
	}
	chain->filter[iindex].reverse = 0;
	chain->filter[iindex].logging = 0;
	chain->filter[iindex].rejectwith = 0;
	chain->filter[iindex].type = 0;
	chain->filter[iindex].enabled = 0;
	chain->filter[iindex].tos[0] = 0;
	chain->filter[iindex].tos[1] = 0;
	chain->filter[iindex].ttl[0] = 0;
	chain->filter[iindex].ttl[1] = 0;
	chain->filter[iindex].action = 0;
	chain->filter[iindex].rlimitint = 0;
	chain->filter[iindex].rlimitpkt = 0;
	chain->filter[iindex].appindex = 0;
	chain->filter[iindex].dzindex = 0;
	chain->filter[iindex].szindex = 0;
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
	return;
}

void
appqueue_cli_mf_net_fw_mangle_edit(void)
{
	char cliask[255];
	char clians[255];
	u32 iindex = 0, zindex = 0;
	u16 fw_update = 0;
	u8 y = 0, d = 0, f = 0, m = 0;
	kattach_fw_m_entry_t fw;
	kattach_fw_m_chain_t *chain;

	/* set some optional stuff */
	fw.ttl[0] = 0;
	fw.ttl[1] = 0;
	fw.tos[0] = 0;
	fw.tos[1] = 0;
	fw.mark = 0;

	sprintf(cliask,"Edit Firewall Rule in (i)nbound, (o)utbound, (f)orward, pre(r)outing, (p)ostrouting: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'i') {
			f = 1;
		} else if (clians[0] == 'o') {
			f = 2;
		} else if (clians[0] == 'f') {
			f = 3;
		} else if (clians[0] == 'r') {
			f = 4;
		} else if (clians[0] == 'p') {
			f = 5;
		}
		if (f) {
			y++;
			break;
		}
	}
	y = 0;

	if (f == 1) {
		chain = &kattach_fw_shm->mangle.input;
		fw_update = KATTACH_FW_CH_MANGLE_INPUT;
	} else if (f == 2) {
		chain = &kattach_fw_shm->mangle.output;
		fw_update = KATTACH_FW_CH_MANGLE_OUTPUT;
	} else if (f == 3) {
		chain = &kattach_fw_shm->mangle.forward;
		fw_update = KATTACH_FW_CH_MANGLE_FORWARD;
	} else if (f == 4) {
		chain = &kattach_fw_shm->mangle.prerouting;
		fw_update = KATTACH_FW_CH_MANGLE_PREROUTING;
	} else if (f == 5) {
		chain = &kattach_fw_shm->mangle.postrouting;
		fw_update = KATTACH_FW_CH_MANGLE_POSTROUTING;
	} else {
		printf("\nERROR occured - chain unknown\n");
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
		return;
	}

	sprintf(cliask,"Edit Rule: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
			return;
		}
		iindex = atol(clians);
		y++;
		break;	
	}

	if ((chain->filter[iindex].nindex == 0) && (chain->filter[iindex].pindex == 0) && (chain->filter[iindex].enabled == 0)) {
		printf("\nFilter ID %lu is not configured.\n",iindex);
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
		return;
	}

	d = 0;
	y = 0;
	sprintf(cliask,"Source Zone (? to list): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_net_fw_zones_list();
		} else {
			for (zindex = 0; zindex < kattach_fw_shm->zones.index; zindex++) {
				if (kattach_fw_shm->zones.zone[zindex].name[0] == '\0') continue;
				if ((strlen(clians)) != (strlen(kattach_fw_shm->zones.zone[zindex].name))) continue;
				if (!strncmp(clians,kattach_fw_shm->zones.zone[zindex].name,strlen(clians))) {
					d = 1;
					y++;
					break;
				}
			}
			if (d) {
				break;
			} else {
				printf("\nZone %s not found. Please check and retry\n",clians);
			}
		}
	}
	fw.szindex = zindex;
	d = 0;
	y = 0;
	sprintf(cliask,"Destination Zone (? to list): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_net_fw_zones_list();
		} else {
			for (zindex = 0; zindex < kattach_fw_shm->zones.index; zindex++) {
				if (kattach_fw_shm->zones.zone[zindex].name[0] == '\0') continue;
				if (strlen(clians) != strlen(kattach_fw_shm->zones.zone[zindex].name)) continue;
				if (!strncmp(clians,kattach_fw_shm->zones.zone[zindex].name,strlen(clians))) {
					d = 1;
					y++;
					break;
				}
			}
			if (d) {
				break;
			} else {
				printf("\nZone %s not found. Please check and retry\n",clians);
			}
		}
	}
	fw.dzindex = zindex;
	d = 0;
	y = 0;
	sprintf(cliask,"App Profile (? to list): ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
			return;
		}
		if (clians[0] == '?') {
			appqueue_cli_mf_net_fw_apps_list();
		} else {
			for (zindex = 0; zindex < kattach_fw_shm->apps.index; zindex++) {
				if (kattach_fw_shm->apps.app[zindex].name[0] == '\0') continue;
				if (strlen(clians) != strlen(kattach_fw_shm->apps.app[zindex].name)) continue;
				if (!strncmp(clians,kattach_fw_shm->apps.app[zindex].name,strlen(clians))) {
					d = 1;
					y++;
					break;
				}
			}
			if (d) {
				break;
			} else {
				printf("\nApp %s not found. Please check and retry\n",clians);
			}
		}
	}
	fw.appindex = zindex;
	y = 0;
	d = 0;
	while(!y) {
		sprintf(cliask,"Enable Rate Limiting (y/n) ? ");
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
			return;
		}
		if (clians[0] == 'y') {
			while (!d) {
				/* FIXME */
				sprintf(cliask,"Rate Limit Packets: ");
				appqueue_cli_askq(cliask,1,clians);
				fw.rlimitpkt = atoi(clians);
				sprintf(cliask,"Rate Limit Interval (seconds): ");
				appqueue_cli_askq(cliask,1,clians);
				fw.rlimitint = atoi(clians);
				d++;
			}
			y++;
			break;
		} else if (clians[0] == 'n') {
			fw.rlimitpkt = 0;
			fw.rlimitint = 0;
			y++;
			break;
		}
	}
	y = 0;
	d = 0;
	/* action */
	sprintf(cliask,"Filter Action - (a)llow, (d)rop, (r)eject, (l)og, (m)ark, (t)tl, t(o)s: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
			return;
		}
		if (clians[0] == 'a') {
			fw.action = KATTACH_FW_ACTION_ALLOW;
			y++;
			break;
		} else if (clians[0] == 'd') {
			fw.action = KATTACH_FW_ACTION_DROP;
			y++;
			break;
		} else if (clians[0] == 'r') {
			fw.action = KATTACH_FW_ACTION_REJECT;
			/* FIXME: add capability to select REJECT with */
			y++;
			break;
		} else if (clians[0] == 'l') {
			fw.action = KATTACH_FW_ACTION_LOG;
			y++;
			break;
		} else if (clians[0] == 'm') {
			fw.action = KATTACH_FW_ACTION_MARK;
			d = 1;
			y++;
			break;
		} else if (clians[0] == 't') {
			fw.action = KATTACH_FW_ACTION_TTL;
			d = 2;
			y++;
			break;
		} else if (clians[0] == 'o') {
			fw.action = KATTACH_FW_ACTION_TOS;
			d = 3;
			y++;
			break;
		}
	}

	y = 0;
	if (d == 1) {
		sprintf(cliask,"\nPacket marking value: ");
		while(!y) {
			memset(clians,0,strlen(clians));
			appqueue_cli_askq(cliask,1,clians);
			fw.mark = atol(clians);
			if (fw.mark) {
				y++;
				break;
			}
		}
		y = 0;
		sprintf(cliask,"\nExtra Match on (t)tl, t(o)s, (n)one : ");
		while(!y) {
			memset(clians,0,strlen(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (clians[0] == 'n') {
				m = 0;
				y++;
				break;
			} else if (clians[0] == 't') {
				m = 1;
				y++;
				break;
			} else if (clians[0] == 'o') {
				m = 2;
				y++;
				break;
			}
		}
		y = 0;
		if (m == 1) {
			sprintf(cliask,"\nTTL value to match: ");
			while(!y) {
				memset(clians,0,strlen(clians));
				appqueue_cli_askq(cliask,1,clians);
				fw.ttl[1] = atol(clians);
				if (fw.ttl[1] <= 0xff) {
					fw.ttl[0] = 3;
					y++;
					break;
				}
			}
		} else if (m == 2) {
			sprintf(cliask,"\nTOS value to match: ");
			while(!y) {
				memset(clians,0,strlen(clians));
				appqueue_cli_askq(cliask,1,clians);
				fw.tos[1] = atol(clians);
				if (fw.tos[1] <= 0xff) {
					fw.tos[0] = 1;
					y++;
					break;
				}
			}
		}
	} else if (d == 2) {
		sprintf(cliask,"\nTTL (s)et, (d)ecrease, (i)ncrease : ");
		while(!y) {
			memset(clians,0,strlen(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (clians[0] == 's') {
				fw.ttl[0] = 0;
				sprintf(cliask,"\nSet TTL to (0 - 255) : ");
				y++;
				break;
			} else if (clians[0] == 'd') {
				fw.ttl[0] = 1;
				sprintf(cliask,"\nDecrease TTL by (0 - 255) : ");
				y++;
				break;
			} else if (clians[0] == 'i') {
				fw.ttl[0] = 2;
				sprintf(cliask,"\nIncrease TTL by (0 - 255) : ");
				y++;
				break;
			}
		}
		y = 0;
		while(!y) {
			memset(clians,0,strlen(clians));
			appqueue_cli_askq(cliask,1,clians);
			fw.ttl[1] = atol(clians);
			if (fw.ttl[1] <= 0xff) {
				y++;
				break;
			}
		}
	} else if (d == 3) {
		sprintf(cliask,"\nType of Service Value (0 - 255) : ");
		while(!y) {
			memset(clians,0,strlen(clians));
			appqueue_cli_askq(cliask,1,clians);
			fw.tos[1] = atol(clians);
			if (fw.tos[1] <= 0xff) {
				fw.tos[0] = 0;
				y++;
				break;
			}
		}
		
	}

	appqueue_cli_lock_fw();
	chain->filter[iindex].reverse = 0;
	chain->filter[iindex].logging = 0;
	chain->filter[iindex].rejectwith = 0;
	chain->filter[iindex].type = 0;
	chain->filter[iindex].mark = fw.mark;
	chain->filter[iindex].enabled = 1;
	chain->filter[iindex].tos[0] = fw.tos[0];
	chain->filter[iindex].tos[1] = fw.tos[1];
	chain->filter[iindex].ttl[0] = fw.ttl[0];
	chain->filter[iindex].ttl[1] = fw.ttl[1];
	chain->filter[iindex].action = fw.action;
	chain->filter[iindex].rlimitint = fw.rlimitint;
	chain->filter[iindex].rlimitpkt = fw.rlimitpkt;
	chain->filter[iindex].appindex = fw.appindex;
	chain->filter[iindex].dzindex = fw.dzindex;
	chain->filter[iindex].szindex = fw.szindex;
	chain->index++;
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
	return;
}


void
appqueue_cli_mf_net_fw_mangle_list(void)
{
	char cliask[64];
	char clians[64];
	kattach_fw_m_chain_t *chain;
	u32 index;
	u8 y = 0, l = 0;

	chain = &kattach_fw_shm->mangle.input;
	printf("\n[INPUT firewall (policy: ACCEPT)]\n\n");
	index = chain->hindex;
	printf(" Index           Source Zone     Destination Zone     Application     Action    Flags      TTL        TOS    Rate Limiting\n");
	printf("------    ------------------    -----------------    ------------    -------    -----    -----    -------    ----------------------------------\n");
	while (!y) {
		if ((chain->index == 0) && (chain->hindex == 0) && (chain->eindex == 0)) {
			y++;
			break;
		}
		l++;
		printf("%5lu:    %18s   %18s      %10s     %6s    %c:%c:%c    ", index, kattach_fw_shm->zones.zone[chain->filter[index].szindex].name, kattach_fw_shm->zones.zone[chain->filter[index].dzindex].name,
						kattach_fw_shm->apps.app[chain->filter[index].appindex].name, ((chain->filter[index].action == KATTACH_FW_ACTION_ALLOW) ? " allow" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_DROP) ? "  drop" : ((chain->filter[index].action == KATTACH_FW_ACTION_REJECT) ? "reject" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_LOG) ? "   log" : ((chain->filter[index].action == KATTACH_FW_ACTION_MARK) ? "  mark" :
						((chain->filter[index].action == KATTACH_FW_ACTION_TOS) ? "   tos" : ((chain->filter[index].action == KATTACH_FW_ACTION_TTL) ? "   ttl" :
						"  none"))))))), ((chain->filter[index].reverse == 1) ? 'R' : '-'),
						((chain->filter[index].logging == 1) ? 'L' : '-'), ((chain->filter[index].enabled == 1) ? 'E' : '-'));
		if ((chain->filter[index].action == KATTACH_FW_ACTION_MARK) || (chain->filter[index].action == KATTACH_FW_ACTION_TTL)) {
			printf("%c:%3u    ",((chain->filter[index].ttl[0] == 0) ? 'S' : ((chain->filter[index].ttl[0] == 1) ? 'D' : ((chain->filter[index].ttl[0] == 2) ? 'I' :
						((chain->filter[index].ttl[0] == 3) ? 'm' : '?')))), chain->filter[index].ttl[1]);
		} else {
			printf("-----    ");
		}
		if ((chain->filter[index].action == KATTACH_FW_ACTION_MARK) || (chain->filter[index].action == KATTACH_FW_ACTION_TOS)) {
			printf(" %c:%5u    ",((chain->filter[index].tos[0] == 1) ? 'm' : 's'), chain->filter[index].tos[1]);
		} else {
			printf("-------    ");
		}
		if (chain->filter[index].rlimitpkt) {
			if (chain->filter[index].rlimitint <= 59) {
				printf("%8lu pkts per second\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint <= 3599)) {
				printf("%8lu pkts per minute\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 3600) && (chain->filter[index].rlimitint <= 86399)) {
				printf("%8lu pkts per hour\n",chain->filter[index].rlimitpkt);
			} else {
				printf("%8lu pkts per day\n",chain->filter[index].rlimitpkt);
			}
		} else {
			printf("\n");
		}
		if (index != chain->eindex) {
			index = chain->filter[index].nindex;
		} else {
			y++;
			break;
		}
		if (l >= APPQUEUE_CLI_LINES) {
			sprintf(cliask,"\nPress Enter to continue or q to quit... ");
			appqueue_cli_askq(cliask,1,clians);
			if (clians[0] == 'q') {
				appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
				return;
			} else {
				l = 0;
				printf("\n");
			}
		}
	}	
	printf("\n\n");

	y = 0;
	chain = &kattach_fw_shm->mangle.output;
	printf("\n[OUTPUT firewall (policy: ACCEPT)]\n\n");
	index = chain->hindex;
	printf(" Index           Source Zone     Destination Zone     Application     Action    Flags      TTL        TOS    Rate Limiting\n");
	printf("------    ------------------    -----------------    ------------    -------    -----    -----    -------    ----------------------------------\n");
	while (!y) {
		if ((chain->index == 0) && (chain->hindex == 0) && (chain->eindex == 0)) {
			y++;
			break;
		}
		l++;
		printf("%5lu:    %18s   %18s      %10s     %6s    %c:%c:%c    ", index, kattach_fw_shm->zones.zone[chain->filter[index].szindex].name, kattach_fw_shm->zones.zone[chain->filter[index].dzindex].name,
						kattach_fw_shm->apps.app[chain->filter[index].appindex].name, ((chain->filter[index].action == KATTACH_FW_ACTION_ALLOW) ? " allow" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_DROP) ? "  drop" : ((chain->filter[index].action == KATTACH_FW_ACTION_REJECT) ? "reject" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_LOG) ? "   log" : ((chain->filter[index].action == KATTACH_FW_ACTION_MARK) ? "  mark" :
						((chain->filter[index].action == KATTACH_FW_ACTION_TOS) ? "   tos" : ((chain->filter[index].action == KATTACH_FW_ACTION_TTL) ? "   ttl" :
						"  none"))))))), ((chain->filter[index].reverse == 1) ? 'R' : '-'),
						((chain->filter[index].logging == 1) ? 'L' : '-'), ((chain->filter[index].enabled == 1) ? 'E' : '-'));
		if ((chain->filter[index].action == KATTACH_FW_ACTION_MARK) || (chain->filter[index].action == KATTACH_FW_ACTION_TTL)) {
			printf("%c:%3u    ",((chain->filter[index].ttl[0] == 0) ? 'S' : ((chain->filter[index].ttl[0] == 1) ? 'D' : ((chain->filter[index].ttl[0] == 2) ? 'I' :
						((chain->filter[index].ttl[0] == 3) ? 'm' : '?')))), chain->filter[index].ttl[1]);
		} else {
			printf("-----    ");
		}
		if ((chain->filter[index].action == KATTACH_FW_ACTION_MARK) || (chain->filter[index].action == KATTACH_FW_ACTION_TOS)) {
			printf("%c:%5u    ",((chain->filter[index].tos[0] == 1) ? 'm' : 's'), chain->filter[index].tos[1]);
		} else {
			printf("-------    ");
		}
		if (chain->filter[index].rlimitpkt) {
			if (chain->filter[index].rlimitint <= 59) {
				printf("%8lu pkts per second\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint <= 3599)) {
				printf("%8lu pkts per minute\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 3600) && (chain->filter[index].rlimitint <= 86399)) {
				printf("%8lu pkts per hour\n",chain->filter[index].rlimitpkt);
			} else {
				printf("%8lu pkts per day\n",chain->filter[index].rlimitpkt);
			}
		} else {
			printf("\n");
		}
		if (index != chain->eindex) {
			index = chain->filter[index].nindex;
		} else {
			y++;
			break;
		}
		if (l >= APPQUEUE_CLI_LINES) {
			sprintf(cliask,"\nPress Enter to continue or q to quit... ");
			appqueue_cli_askq(cliask,1,clians);
			if (clians[0] == 'q') {
				appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
				return;
			} else {
				l = 0;
				printf("\n");
			}
		}
	}	
	printf("\n\n");

	y = 0;
	chain = &kattach_fw_shm->mangle.forward;
	printf("\n[FORWARDING firewall (policy: ACCEPT)]\n\n");
	index = chain->hindex;
	printf(" Index           Source Zone     Destination Zone     Application     Action    Flags      TTL        TOS    Rate Limiting\n");
	printf("------    ------------------    -----------------    ------------    -------    -----    -----    -------    ----------------------------------\n");
	while (!y) {
		if ((chain->index == 0) && (chain->hindex == 0) && (chain->eindex == 0)) {
			y++;
			break;
		}
		l++;
		printf("%5lu:    %18s   %18s      %10s     %6s    %c:%c:%c    ", index, kattach_fw_shm->zones.zone[chain->filter[index].szindex].name, kattach_fw_shm->zones.zone[chain->filter[index].dzindex].name,
						kattach_fw_shm->apps.app[chain->filter[index].appindex].name, ((chain->filter[index].action == KATTACH_FW_ACTION_ALLOW) ? " allow" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_DROP) ? "  drop" : ((chain->filter[index].action == KATTACH_FW_ACTION_REJECT) ? "reject" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_LOG) ? "   log" : ((chain->filter[index].action == KATTACH_FW_ACTION_MARK) ? "  mark" :
						((chain->filter[index].action == KATTACH_FW_ACTION_TOS) ? "   tos" : ((chain->filter[index].action == KATTACH_FW_ACTION_TTL) ? "   ttl" :
						"  none"))))))), ((chain->filter[index].reverse == 1) ? 'R' : '-'),
						((chain->filter[index].logging == 1) ? 'L' : '-'), ((chain->filter[index].enabled == 1) ? 'E' : '-'));
		if ((chain->filter[index].action == KATTACH_FW_ACTION_MARK) || (chain->filter[index].action == KATTACH_FW_ACTION_TTL)) {
			printf("%c:%3u    ",((chain->filter[index].ttl[0] == 0) ? 'S' : ((chain->filter[index].ttl[0] == 1) ? 'D' : ((chain->filter[index].ttl[0] == 2) ? 'I' :
						((chain->filter[index].ttl[0] == 3) ? 'm' : '?')))), chain->filter[index].ttl[1]);
		} else {
			printf("-----    ");
		}
		if ((chain->filter[index].action == KATTACH_FW_ACTION_MARK) || (chain->filter[index].action == KATTACH_FW_ACTION_TOS)) {
			printf("%c:%5u    ",((chain->filter[index].tos[0] == 1) ? 'm' : 's'), chain->filter[index].tos[1]);
		} else {
			printf("-------    ");
		}
		if (chain->filter[index].rlimitpkt) {
			if (chain->filter[index].rlimitint <= 59) {
				printf("%8lu pkts per second\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint <= 3599)) {
				printf("%8lu pkts per minute\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 3600) && (chain->filter[index].rlimitint <= 86399)) {
				printf("%8lu pkts per hour\n",chain->filter[index].rlimitpkt);
			} else {
				printf("%8lu pkts per day\n",chain->filter[index].rlimitpkt);
			}
		} else {
			printf("\n");
		}
		if (index != chain->eindex) {
			index = chain->filter[index].nindex;
		} else {
			y++;
			break;
		}
		if (l >= APPQUEUE_CLI_LINES) {
			sprintf(cliask,"\nPress Enter to continue or q to quit... ");
			appqueue_cli_askq(cliask,1,clians);
			if (clians[0] == 'q') {
				appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
				return;
			} else {
				l = 0;
				printf("\n");
			}
		}
	}	
	printf("\n\n");

	y = 0;
	chain = &kattach_fw_shm->mangle.prerouting;
	printf("\n[PREROUTING firewall (policy: ACCEPT)]\n\n");
	index = chain->hindex;
	printf(" Index           Source Zone     Destination Zone     Application     Action    Flags      TTL        TOS    Rate Limiting\n");
	printf("------    ------------------    -----------------    ------------    -------    -----    -----    -------    ----------------------------------\n");
	while (!y) {
		if ((chain->index == 0) && (chain->hindex == 0) && (chain->eindex == 0)) {
			y++;
			break;
		}
		l++;
		printf("%5lu:    %18s   %18s      %10s     %6s    %c:%c:%c    ", index, kattach_fw_shm->zones.zone[chain->filter[index].szindex].name, kattach_fw_shm->zones.zone[chain->filter[index].dzindex].name,
						kattach_fw_shm->apps.app[chain->filter[index].appindex].name, ((chain->filter[index].action == KATTACH_FW_ACTION_ALLOW) ? " allow" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_DROP) ? "  drop" : ((chain->filter[index].action == KATTACH_FW_ACTION_REJECT) ? "reject" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_LOG) ? "   log" : ((chain->filter[index].action == KATTACH_FW_ACTION_MARK) ? "  mark" :
						((chain->filter[index].action == KATTACH_FW_ACTION_TOS) ? "   tos" : ((chain->filter[index].action == KATTACH_FW_ACTION_TTL) ? "   ttl" :
						"  none"))))))), ((chain->filter[index].reverse == 1) ? 'R' : '-'),
						((chain->filter[index].logging == 1) ? 'L' : '-'), ((chain->filter[index].enabled == 1) ? 'E' : '-'));
		if ((chain->filter[index].action == KATTACH_FW_ACTION_MARK) || (chain->filter[index].action == KATTACH_FW_ACTION_TTL)) {
			printf("%c:%3u    ",((chain->filter[index].ttl[0] == 0) ? 'S' : ((chain->filter[index].ttl[0] == 1) ? 'D' : ((chain->filter[index].ttl[0] == 2) ? 'I' :
						((chain->filter[index].ttl[0] == 3) ? 'm' : '?')))), chain->filter[index].ttl[1]);
		} else {
			printf("-----    ");
		}
		if ((chain->filter[index].action == KATTACH_FW_ACTION_MARK) || (chain->filter[index].action == KATTACH_FW_ACTION_TOS)) {
			printf("%c:%5u    ",((chain->filter[index].tos[0] == 1) ? 'm' : 's'), chain->filter[index].tos[1]);
		} else {
			printf("-------    ");
		}
		if (chain->filter[index].rlimitpkt) {
			if (chain->filter[index].rlimitint <= 59) {
				printf("%8lu pkts per second\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint <= 3599)) {
				printf("%8lu pkts per minute\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 3600) && (chain->filter[index].rlimitint <= 86399)) {
				printf("%8lu pkts per hour\n",chain->filter[index].rlimitpkt);
			} else {
				printf("%8lu pkts per day\n",chain->filter[index].rlimitpkt);
			}
		} else {
			printf("\n");
		}
		if (index != chain->eindex) {
			index = chain->filter[index].nindex;
		} else {
			y++;
			break;
		}
		if (l >= APPQUEUE_CLI_LINES) {
			sprintf(cliask,"\nPress Enter to continue or q to quit... ");
			appqueue_cli_askq(cliask,1,clians);
			if (clians[0] == 'q') {
				appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
				return;
			} else {
				l = 0;
				printf("\n");
			}
		}
	}	
	printf("\n\n");

	y = 0;
	chain = &kattach_fw_shm->mangle.postrouting;
	printf("\n[POSTROUTING firewall (policy: ACCEPT)]\n\n");
	index = chain->hindex;
	printf(" Index           Source Zone     Destination Zone     Application     Action    Flags      TTL        TOS    Rate Limiting\n");
	printf("------    ------------------    -----------------    ------------    -------    -----    -----    -------    ----------------------------------\n");
	while (!y) {
		if ((chain->index == 0) && (chain->hindex == 0) && (chain->eindex == 0)) {
			y++;
			break;
		}
		l++;
		printf("%5lu:    %18s   %18s      %10s     %6s    %c:%c:%c    ", index, kattach_fw_shm->zones.zone[chain->filter[index].szindex].name, kattach_fw_shm->zones.zone[chain->filter[index].dzindex].name,
						kattach_fw_shm->apps.app[chain->filter[index].appindex].name, ((chain->filter[index].action == KATTACH_FW_ACTION_ALLOW) ? " allow" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_DROP) ? "  drop" : ((chain->filter[index].action == KATTACH_FW_ACTION_REJECT) ? "reject" : 
						((chain->filter[index].action == KATTACH_FW_ACTION_LOG) ? "   log" : ((chain->filter[index].action == KATTACH_FW_ACTION_MARK) ? "  mark" :
						((chain->filter[index].action == KATTACH_FW_ACTION_TOS) ? "   tos" : ((chain->filter[index].action == KATTACH_FW_ACTION_TTL) ? "   ttl" :
						"  none"))))))), ((chain->filter[index].reverse == 1) ? 'R' : '-'),
						((chain->filter[index].logging == 1) ? 'L' : '-'), ((chain->filter[index].enabled == 1) ? 'E' : '-'));
		if ((chain->filter[index].action == KATTACH_FW_ACTION_MARK) || (chain->filter[index].action == KATTACH_FW_ACTION_TTL)) {
			printf("%c:%3u    ",((chain->filter[index].ttl[0] == 0) ? 'S' : ((chain->filter[index].ttl[0] == 1) ? 'D' : ((chain->filter[index].ttl[0] == 2) ? 'I' :
						((chain->filter[index].ttl[0] == 3) ? 'm' : '?')))), chain->filter[index].ttl[1]);
		} else {
			printf("-----    ");
		}
		if ((chain->filter[index].action == KATTACH_FW_ACTION_MARK) || (chain->filter[index].action == KATTACH_FW_ACTION_TOS)) {
			printf("%c:%5u    ",((chain->filter[index].tos[0] == 1) ? 'm' : 's'), chain->filter[index].tos[1]);
		} else {
			printf("-------    ");
		}
		if (chain->filter[index].rlimitpkt) {
			if (chain->filter[index].rlimitint <= 59) {
				printf("%8lu pkts per second\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint <= 3599)) {
				printf("%8lu pkts per minute\n",chain->filter[index].rlimitpkt);
			} else if ((chain->filter[index].rlimitint >= 3600) && (chain->filter[index].rlimitint <= 86399)) {
				printf("%8lu pkts per hour\n",chain->filter[index].rlimitpkt);
			} else {
				printf("%8lu pkts per day\n",chain->filter[index].rlimitpkt);
			}
		} else {
			printf("\n");
		}
		if (index != chain->eindex) {
			index = chain->filter[index].nindex;
		} else {
			y++;
			break;
		}
		if (l >= APPQUEUE_CLI_LINES) {
			sprintf(cliask,"\nPress Enter to continue or q to quit... ");
			appqueue_cli_askq(cliask,1,clians);
			if (clians[0] == 'q') {
				appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
				return;
			} else {
				l = 0;
				printf("\n");
			}
		}
	}	
	printf("\n\n");


	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
	return;
}


void
appqueue_cli_mf_net_fw_mangle_reverse(void)
{
	char cliask[255];
	char clians[255];
	u32 iindex = 0;
	u16 fw_update = 0;
	u8 y = 0, f = 0;
	kattach_fw_m_chain_t *chain;

	sprintf(cliask,"Reverse Firewall Rule in (i)nbound, (o)utbound, (f)orward, pre(r)outing or (p)ostrouting: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'i') {
			f = 1;
		} else if (clians[0] == 'o') {
			f = 2;
		} else if (clians[0] == 'f') {
			f = 3;
		} else if (clians[0] == 'r') {
			f = 4;
		} else if (clians[0] == 'p') {
			f = 5;
		}
		if (f) {
			y++;
			break;
		}
	}
	y = 0;

	sprintf(cliask,"Reverse Rule Index: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
			return;
		}
		iindex = atol(clians);
		y++;
		break;	
	}

	if (f == 1) {
		chain = &kattach_fw_shm->mangle.input;
		fw_update = KATTACH_FW_CH_MANGLE_INPUT;
	} else if (f == 2) {
		chain = &kattach_fw_shm->mangle.output;
		fw_update = KATTACH_FW_CH_MANGLE_OUTPUT;
	} else if (f == 3) {
		chain = &kattach_fw_shm->mangle.forward;
		fw_update = KATTACH_FW_CH_MANGLE_FORWARD;
	} else if (f == 4) {
		chain = &kattach_fw_shm->mangle.prerouting;
		fw_update = KATTACH_FW_CH_MANGLE_PREROUTING;
	} else if (f == 5) {
		chain = &kattach_fw_shm->mangle.postrouting;
		fw_update = KATTACH_FW_CH_MANGLE_POSTROUTING;
	} else {
		printf("\nERROR occured - chain unknown\n");
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
		return;
	}

	if ((chain->filter[iindex].nindex == 0) && (chain->filter[iindex].pindex == 0) && (chain->filter[iindex].enabled == 0)) {
		printf("\nFilter ID %lu is not configured.\n",iindex);
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
		return;
	}

	appqueue_cli_lock_fw();
	if (chain->filter[iindex].reverse == 0) {
		chain->filter[iindex].reverse = 1;
	} else {
		chain->filter[iindex].reverse = 0;
	}
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
	return;
}


void
appqueue_cli_mf_net_fw_mangle_log(void)
{
	char cliask[255];
	char clians[255];
	u32 iindex = 0;
	u16 fw_update = 0;
	u8 y = 0, f = 0;
	kattach_fw_m_chain_t *chain;

	sprintf(cliask,"Reverse Firewall Rule in (i)nbound, (o)utbound, (f)orward, pre(r)outing or (p)ostrouting: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (clians[0] == 'i') {
			f = 1;
		} else if (clians[0] == 'o') {
			f = 2;
		} else if (clians[0] == 'f') {
			f = 3;
		} else if (clians[0] == 'r') {
			f = 4;
		} else if (clians[0] == 'p') {
			f = 5;
		}
		if (f) {
			y++;
			break;
		}
	}
	y = 0;

	sprintf(cliask,"Reverse Rule Index: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
			return;
		}
		iindex = atol(clians);
		y++;
		break;	
	}
	y = 0;
	sprintf(cliask,"Tag rule with: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) <= 32) {
			y++;
			break;
		}
	}		

	if (f == 1) {
		chain = &kattach_fw_shm->mangle.input;
		fw_update = KATTACH_FW_CH_MANGLE_INPUT;
	} else if (f == 2) {
		chain = &kattach_fw_shm->mangle.output;
		fw_update = KATTACH_FW_CH_MANGLE_OUTPUT;
	} else if (f == 3) {
		chain = &kattach_fw_shm->mangle.forward;
		fw_update = KATTACH_FW_CH_MANGLE_FORWARD;
	} else if (f == 4) {
		chain = &kattach_fw_shm->mangle.prerouting;
		fw_update = KATTACH_FW_CH_MANGLE_PREROUTING;
	} else if (f == 5) {
		chain = &kattach_fw_shm->mangle.postrouting;
		fw_update = KATTACH_FW_CH_MANGLE_POSTROUTING;
	} else {
		printf("\nERROR occured - chain unknown\n");
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
		return;
	}

	if ((chain->filter[iindex].nindex == 0) && (chain->filter[iindex].pindex == 0) && (chain->filter[iindex].enabled == 0)) {
		printf("\nFilter ID %lu is not configured.\n",iindex);
		appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
		return;
	}

	appqueue_cli_lock_fw();
	if (chain->filter[iindex].logging == 0) {
		chain->filter[iindex].logging = 1;
	} else {
		chain->filter[iindex].logging = 0;
	}
	sprintf(chain->filter[iindex].logprefix,"%s",clians);
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_MANGLE;
	return;
}

void
appqueue_cli_mf_net_fw_zones(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_net_fw_zones.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_net_fw_zones.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_net_fw_zones.climenu[i].menu_cmd,appqueue_cli_menu_net_fw_zones.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sFirewall Zones]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_ZONES;
	appqueue_po = 0;
	return;
}

void
appqueue_cli_mf_net_fw_apps(void)
{
	u8 i = 0;

	if (!appqueue_po) {
		printf("\n\n\n");
		printf("%s",appqueue_cli_menu_net_fw_apps.cli_menu_title);
		printf("%s\n\n",APPQUEUE_MENU_BAR);

		for (i = 0; i < appqueue_cli_menu_net_fw_apps.index; i++) {
			printf("     %s  \t - %s\n",appqueue_cli_menu_net_fw_apps.climenu[i].menu_cmd,appqueue_cli_menu_net_fw_apps.climenu[i].menu_desc);
		}
	}

	printf("\n\n%sFirewall Apps]# ",appqueue_prompt);
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
	appqueue_po = 0;
	return;
}

void appqueue_cli_mf_net_fw_zones_list(void)
{
	u32 zindex = 0;
	u8 nindex = 0;
	char sip[20];

	printf("\n Index              Name  VLAN  IP Networks\n");
	printf("------  ----------------  ----  --------------------------------------------------------------------------------------\n");

	for (zindex = 0; zindex < KATTACH_MAX_FW_ZONES; zindex++) {
		if (kattach_fw_shm->zones.zone[zindex].name[0] == '\0') continue;
		printf("%5lu:  %16s  %4u  ",zindex, kattach_fw_shm->zones.zone[zindex].name, kattach_fw_shm->zones.zone[zindex].vlan);
		for (nindex = 0; nindex < kattach_fw_shm->zones.zone[zindex].nindex; nindex++) {
			sprintf(sip,"%lu.%lu.%lu.%lu/%u ",((kattach_fw_shm->zones.zone[zindex].node[nindex].ip >> 24) & 0xff),((kattach_fw_shm->zones.zone[zindex].node[nindex].ip >> 16) & 0xff),
							((kattach_fw_shm->zones.zone[zindex].node[nindex].ip >> 8) & 0xff),((kattach_fw_shm->zones.zone[zindex].node[nindex].ip) & 0xff),
							kattach_fw_shm->zones.zone[zindex].node[nindex].mask);
			printf("%-19s",sip);
		}
		printf("\n");
	}

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_ZONES;
	return;
}

void appqueue_cli_mf_net_fw_zones_create(void)
{
	char cliask[255];
	char clians[255];
	char zname[32];
	u32 zindex = 0, sip[KATTACH_MAX_FW_ZNODES], mask[KATTACH_MAX_FW_ZNODES];
	u16 zvlan = 0, fw_update = KATTACH_FW_CH_ZONES;
	u8 y = 0, d = 0;

	sprintf(cliask,"\nZone Name: ");
	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_ZONES;
			return;
		}
		if (strlen(clians) > 32) {
			printf("\nZone name %s is too long. Must be less than 32 characters.\n",clians);
			continue;
		}
		for (zindex = 0; zindex < kattach_fw_shm->zones.index; zindex++) {
			if (kattach_fw_shm->zones.zone[zindex].name[0] == '\0') continue;
			if ((strlen(kattach_fw_shm->zones.zone[zindex].name)) != (strlen(clians))) continue;
			if (!strncmp(clians,kattach_fw_shm->zones.zone[zindex].name,strlen(clians))) {
				printf("\nZone %s already exists. Please edit or select a unique name.\n",clians);
				d = 1;
				break;
			}
		}
		if (!d) {
			sprintf(zname,"%s",clians);
			y++;
			break;
		} else {
			d = 0;
		}
	}
	y = 0;

	sprintf(cliask,"\nZone VLAN: ");
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_ZONES;
			return;
		}
		zvlan = atoi(clians);
		if ((zvlan > 0) && (zvlan <= APPQUEUE_MAX_VLAN)) {
			y++;
			break;
		}
	}

	/* ip / mask */
	y = 0;
	while (y < KATTACH_MAX_FW_ZNODES) {
		sprintf(cliask,"IPv4 address: ");
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			break;
		}
		sip[y] = appqueue_cli_parseip(clians);
		if ((sip[y] != 0) || (!strncmp(clians,"0.0.0.0",strlen(clians)))) {
			memset(cliask,0,sizeof(cliask));
			d = 0;
			while (!d) {
				memset(clians,0,sizeof(clians));
				sprintf(cliask,"CIDR mask: ");
				appqueue_cli_askq(cliask,1,clians);
				if ((strlen(clians) > 2)) {
					mask[y] = appqueue_cli_parseip(clians);		/* FIXME: need to properly parse non-CIDR netmask */
				} else {
					mask[y] = atoi(clians);
				}
				if ((mask[y] != 0) || (sip[y] == 0)) {
					d++;
					y++;
					break;
				}
			}
		} else {
			memset(clians,0,sizeof(clians));
		}
	}

	appqueue_cli_lock_fw();
	kattach_fw_shm->zones.zone[kattach_fw_shm->zones.index].vlan = zvlan;
	kattach_fw_shm->zones.zone[kattach_fw_shm->zones.index].nindex = y;
	sprintf(kattach_fw_shm->zones.zone[kattach_fw_shm->zones.index].name,"%s",zname);
	for (d = 0; d < y; d++) {
		kattach_fw_shm->zones.zone[kattach_fw_shm->zones.index].node[d].ip = sip[d];
		kattach_fw_shm->zones.zone[kattach_fw_shm->zones.index].node[d].mask = mask[d];
	}
	kattach_fw_shm->zones.index++;
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_ZONES;
	return;
}

void appqueue_cli_mf_net_fw_zones_edit(void)
{
	char cliask[255];
	char clians[255];
	char zname[32];
	u32 zindex = 0, sip[KATTACH_MAX_FW_ZNODES], mask[KATTACH_MAX_FW_ZNODES];
	u16 zvlan = 0, fw_update = KATTACH_FW_CH_ZONES;
	u8 y = 0, d = 0;

	sprintf(cliask,"\nZone Name: ");
	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_ZONES;
			return;
		}
		if (strlen(clians) > 32) {
			printf("\nZone name %s is too long. Must be less than 32 characters.\n",clians);
			continue;
		}
		for (zindex = 0; zindex < kattach_fw_shm->zones.index; zindex++) {
			if (kattach_fw_shm->zones.zone[zindex].name[0] == '\0') continue;
			if ((strlen(kattach_fw_shm->zones.zone[zindex].name)) != (strlen(clians))) continue;
			if (!strncmp(clians,kattach_fw_shm->zones.zone[zindex].name,strlen(clians))) {
				d = 1;
				break;
			}
		}
		if (!d) {
			printf("\nZone %s not found. Use create or check the zone name.\n",clians);
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_ZONES;
			return;
		} else {
			d = 0;
			y++;
			break;
		}
	}
	y = 0;
	sprintf(zname,"%s",clians);
	sprintf(cliask,"\nZone VLAN [%u]: ",kattach_fw_shm->zones.zone[zindex].vlan);
	while(!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			zvlan = kattach_fw_shm->zones.zone[zindex].vlan;
			y++;
			break;
		}
		zvlan = atoi(clians);
		if ((zvlan > 0) && (zvlan <= APPQUEUE_MAX_VLAN)) {
			y++;
			break;
		}
	}

	/* ip / mask */
	y = 0;
	while (y < KATTACH_MAX_FW_ZNODES) {
		if (y < kattach_fw_shm->zones.zone[zindex].nindex) {
			sprintf(cliask,"IPv4 address [%lu.%lu.%lu.%lu]: ", ((kattach_fw_shm->zones.zone[zindex].node[y].ip >> 24) & 0xff), ((kattach_fw_shm->zones.zone[zindex].node[y].ip >> 16) & 0xff),
										((kattach_fw_shm->zones.zone[zindex].node[y].ip >> 8) & 0xff), ((kattach_fw_shm->zones.zone[zindex].node[y].ip) & 0xff));
		} else {
			sprintf(cliask,"IPv4 address: ");
		}
		memset(clians,0,sizeof(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			if (y >= kattach_fw_shm->zones.zone[zindex].nindex) {
				break;
			} else {
				sip[y] = kattach_fw_shm->zones.zone[zindex].node[y].ip;
			}
		} else {
			sip[y] = appqueue_cli_parseip(clians);
		}
		if ((sip[y] != 0) || (!strncmp(clians,"0.0.0.0",strlen(clians))) || ((sip[y] == 0) && (kattach_fw_shm->zones.zone[zindex].node[y].ip == 0))) {
			memset(cliask,0,sizeof(cliask));
			d = 0;
			while (!d) {
				memset(clians,0,sizeof(clians));
				if (y < kattach_fw_shm->zones.zone[zindex].nindex) {
					sprintf(cliask,"CIDR mask [%u]: ", kattach_fw_shm->zones.zone[zindex].node[y].mask);
				} else {
					sprintf(cliask,"CIDR mask: ");
				}
				appqueue_cli_askq(cliask,1,clians);
				if ((strlen(clians) > 2)) {
					mask[y] = appqueue_cli_parseip(clians);		/* FIXME: need to properly parse non-CIDR netmask */
				} else if (strlen(clians) != 0) {
					mask[y] = atoi(clians);
				} else {
					mask[y] = kattach_fw_shm->zones.zone[zindex].node[y].mask;
				}
				if ((mask[y] != 0) || (sip[y] == 0)) {
					d++;
					y++;
					break;
				}
			}
		} else {
			memset(clians,0,sizeof(clians));
		}
	}

	appqueue_cli_lock_fw();
	kattach_fw_shm->zones.zone[zindex].vlan = zvlan;
	kattach_fw_shm->zones.zone[zindex].nindex = y;
	sprintf(kattach_fw_shm->zones.zone[zindex].name,"%s",zname);
	for (d = 0; d < y; d++) {
		kattach_fw_shm->zones.zone[zindex].node[d].ip = sip[d];
		kattach_fw_shm->zones.zone[zindex].node[d].mask = mask[d];
	}
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_ZONES;
	return;
}

void appqueue_cli_mf_net_fw_zones_delete(void)
{
	char cliask[255];
	char clians[255];
	u32 zindex = 0;
	u16 fw_update = KATTACH_FW_CH_ZONES;
	u8 y = 0, d = 0;

	sprintf(cliask,"\nZone Name: ");
	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_ZONES;
			return;
		}
		if (strlen(clians) > 32) {
			printf("\nZone name %s is too long. Must be less than 32 characters.\n",clians);
			continue;
		}
		for (zindex = 0; zindex < kattach_fw_shm->zones.index; zindex++) {
			if (kattach_fw_shm->zones.zone[zindex].name[0] == '\0') continue;
			if ((strlen(kattach_fw_shm->zones.zone[zindex].name)) != (strlen(clians))) continue;
			if (!strncmp(clians,kattach_fw_shm->zones.zone[zindex].name,strlen(clians))) {
				d = 1;
				break;
			}
		}
		if (!d) {
			printf("\nZone %s not found. Use create or check the zone name.\n",clians);
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_ZONES;
			return;
		} else {
			d = 0;
			y++;
			break;
		}
	}
	y = 0;

	appqueue_cli_lock_fw();
	kattach_fw_shm->zones.zone[zindex].vlan = 0;
	kattach_fw_shm->zones.zone[zindex].nindex = 0;
	kattach_fw_shm->zones.zone[zindex].name[0] = '\0';
	for (d = 0; d < y; d++) {
		kattach_fw_shm->zones.zone[zindex].node[d].ip = 0;
		kattach_fw_shm->zones.zone[zindex].node[d].mask = 0;
	}
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();

	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_ZONES;
	return;
}

void appqueue_cli_mf_net_fw_apps_list(void)
{
	u32 aindex = 0;
	u8 pindex = 0;

	printf("\n\n  Index              Name     State        Ports    Direction      Protocols\n");
	printf("-------  ----------------  --------  -----------  -----------  -------------\n");

	for (aindex = 0; aindex < kattach_fw_shm->apps.index; aindex++) {
		if (kattach_fw_shm->apps.app[aindex].name[0] == '\0') continue;
		printf("%6lu:  %16s   ",aindex, kattach_fw_shm->apps.app[aindex].name);
		if (kattach_fw_shm->apps.app[aindex].statemask == (kattach_fw_shm->apps.app[aindex].statemask | KATTACH_FW_STMASK_NEW)) {
			printf("N:");
		} else {
			printf("-:");
		}
		if (kattach_fw_shm->apps.app[aindex].statemask == (kattach_fw_shm->apps.app[aindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) {
			printf("E:");
		} else {
			printf("-:");
		}
		if (kattach_fw_shm->apps.app[aindex].statemask == (kattach_fw_shm->apps.app[aindex].statemask | KATTACH_FW_STMASK_RELATED)) {
			printf("R:");
		} else {
			printf("-:");
		}
		if (kattach_fw_shm->apps.app[aindex].statemask == (kattach_fw_shm->apps.app[aindex].statemask | KATTACH_FW_STMASK_INVALID)) {
			printf("I");
		} else {
			printf("-");
		}
		for (pindex = 0; pindex < kattach_fw_shm->apps.app[aindex].pindex; pindex++) {
			printf("  %5u:%-5u  ",kattach_fw_shm->apps.app[aindex].port[pindex].port[0],kattach_fw_shm->apps.app[aindex].port[pindex].port[1]);
			switch(kattach_fw_shm->apps.app[aindex].port[pindex].direction) {
				case KATTACH_FW_DIR_SOURCE:
					printf("     source");
					break;

				case KATTACH_FW_DIR_DESTINATION:
					printf("destination");
					break;

				case KATTACH_FW_DIR_BOTH:
					printf("       both");
					break;

				default:
					printf(    "unknown");
					break;
			}		
			if (kattach_fw_shm->apps.app[aindex].port[pindex].protmask == (kattach_fw_shm->apps.app[aindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) {
				printf("   icmp ");
			} else {
				printf("        ");
			}
			if (kattach_fw_shm->apps.app[aindex].port[pindex].protmask == (kattach_fw_shm->apps.app[aindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_TCP)) {
				printf("tcp ");
			} else {
				printf("    ");
			}
			if (kattach_fw_shm->apps.app[aindex].port[pindex].protmask == (kattach_fw_shm->apps.app[aindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_UDP)) {
				printf("udp\n");
			} else {
				printf("   \n");
			}
		}
	}
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
	return;
}

void appqueue_cli_mf_net_fw_apps_create(void)
{
	char cliask[255];
	char clians[255];
	char zname[32];
	u32 zindex = 0;
	u16 fw_update = KATTACH_FW_CH_APPS;
	u16 asport[KATTACH_MAX_FW_APP_PORTS];
	u16 adport[KATTACH_MAX_FW_APP_PORTS];
	u8 aprotocol[KATTACH_MAX_FW_APP_PORTS];
	u8 adirection[KATTACH_MAX_FW_APP_PORTS];
	u8 y = 0, d = 0, statemask = 0;

	sprintf(cliask,"\nApp Name: ");
	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
			return;
		}
		if (strlen(clians) > 32) {
			printf("\nZone name %s is too long. Must be less than 32 characters.\n",clians);
			continue;
		}
		for (zindex = 0; zindex < kattach_fw_shm->apps.index; zindex++) {
			if (kattach_fw_shm->apps.app[zindex].name[0] == '\0') continue;
			if ((strlen(kattach_fw_shm->apps.app[zindex].name)) != (strlen(clians))) continue;
			if (!strncmp(clians,kattach_fw_shm->apps.app[zindex].name,strlen(clians))) {
				printf("\nApp %s already exists. Please edit or select a unique name.\n",clians);
				d = 1;
				break;
			}
		}
		if (!d) {
			sprintf(zname,"%s",clians);
			y++;
			break;
		} else {
			d = 0;
		}
	}

	y = 0;
	sprintf(cliask,"\nMatch on NEW state (y/n) ? ");
	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
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
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
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
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
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
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
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

	/* port loop */
	y = 0;
	d = 0;
	while (y < KATTACH_MAX_FW_APP_PORTS) {
		printf("\nNew Application Port\n");
		printf("----------------------\n");
		d = 0;
		sprintf(cliask,"\nICMP Protocol (y/n) ? ");
		while (!d) {
			memset(clians,0,sizeof(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (strlen(clians) == 0) {
				if (y) {
					d = 3;
					break;
				} else {
					appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
					return;
				}
			}
			if (clians[0] == 'y') {
				aprotocol[y] |= KATTACH_FW_PROTOCOL_ICMP;
				d++;
				break;
			} else if (clians[0] == 'n') {
				d++;
				break;
			}
		}
		if (d == 3) break;
		d = 0;
		memset(cliask,0,strlen(cliask));
		sprintf(cliask,"\nTCP Protocol (y/n) ? ");
		while (!d) {
			memset(clians,0,sizeof(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (strlen(clians) == 0) {
				if (y) {
					d = 3;
					break;
				} else {
					appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
					return;
				}
			}
			if (clians[0] == 'y') {
				aprotocol[y] |= KATTACH_FW_PROTOCOL_TCP;
				d++;
				break;
			} else if (clians[0] == 'n') {
				d++;
				break;
			}
		}
		if (d == 3) break;
		d = 0;
		memset(cliask,0,strlen(cliask));
		sprintf(cliask,"\nUDP Protocol (y/n) ? ");
		while (!d) {
			memset(clians,0,sizeof(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (strlen(clians) == 0) {
				if (y) {
					d = 3;
					break;
				} else {
					appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
					return;
				}
			}
			if (clians[0] == 'y') {
				aprotocol[y] |= KATTACH_FW_PROTOCOL_UDP;
				d++;
				break;
			} else if (clians[0] == 'n') {
				d++;
				break;
			}
		}
		if (d == 3) break;
		d = 0;
		memset(cliask,0,strlen(cliask));
		sprintf(cliask,"\nPort (start): ");
		while (!d) {
			memset(clians,0,sizeof(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (strlen(clians) == 0) {
				if (y) {
					d = 3;
					break;
				} else {
					appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
					return;
				}
			}
			asport[y] = atoi(clians);
			if ((asport[y] > 0) && (asport[y] < 65536)) {
				d++;
				break;
			}
		}
		if (d == 3) break;
		d = 0;
		memset(cliask,0,strlen(cliask));
		sprintf(cliask,"\nPort (end): ");
		while (!d) {
			memset(clians,0,sizeof(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (strlen(clians) == 0) {
				if (y) {
					d = 3;
					break;
				} else {
					appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
					return;
				}
			}
			adport[y] = atoi(clians);
			if ((adport[y] > 0) && (adport[y] < 65536)) {
				d++;
				break;
			}
		}
		if (d == 3) break;
		d = 0;
		memset(cliask,0,strlen(cliask));
		sprintf(cliask,"\nPort Direction: (s)ource (d)estination (b)oth: ");
		while (!d) {
			memset(clians,0,sizeof(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (strlen(clians) == 0) {
				if (y) {
					d = 3;
					break;
				} else {
					appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
					return;
				}
			}
			if (clians[0] == 's') {
				adirection[y] = KATTACH_FW_DIR_SOURCE;
				y++;
				d++;
				break;
			} else if (clians[0] == 'd') {
				adirection[y] = KATTACH_FW_DIR_DESTINATION;
				y++;
				d++;
				break;
			} else if (clians[0] == 'b') {
				adirection[y] = KATTACH_FW_DIR_BOTH;
				y++;
				d++;
				break;
			}
		}
		if (d == 3) break;
	}

	appqueue_cli_lock_fw();
	kattach_fw_shm->apps.app[kattach_fw_shm->apps.index].statemask = statemask;
	kattach_fw_shm->apps.app[kattach_fw_shm->apps.index].pindex = y;
	sprintf(kattach_fw_shm->apps.app[kattach_fw_shm->apps.index].name,"%s",zname);
	for (d = 0; d < y; d++) {
		kattach_fw_shm->apps.app[kattach_fw_shm->apps.index].port[d].direction = adirection[d];
		kattach_fw_shm->apps.app[kattach_fw_shm->apps.index].port[d].protmask = aprotocol[d];
		kattach_fw_shm->apps.app[kattach_fw_shm->apps.index].port[d].port[0] = asport[d];
		kattach_fw_shm->apps.app[kattach_fw_shm->apps.index].port[d].port[1] = adport[d];
	}
	kattach_fw_shm->apps.index++;
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
	return;
}

void appqueue_cli_mf_net_fw_apps_edit(void)
{
	char cliask[255];
	char clians[255];
	char zname[32];
	u32 zindex = 0;
	u16 fw_update = KATTACH_FW_CH_APPS;
	u16 asport[KATTACH_MAX_FW_APP_PORTS];
	u16 adport[KATTACH_MAX_FW_APP_PORTS];
	u8 aprotocol[KATTACH_MAX_FW_APP_PORTS];
	u8 adirection[KATTACH_MAX_FW_APP_PORTS];
	u8 y = 0, d = 0, statemask = 0;

	sprintf(cliask,"\nApp Name: ");
	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
			return;
		}
		if (strlen(clians) > 32) {
			printf("\nZone name %s is too long. Must be less than 32 characters.\n",clians);
			continue;
		}
		for (zindex = 0; zindex < kattach_fw_shm->apps.index; zindex++) {
			if (kattach_fw_shm->apps.app[zindex].name[0] == '\0') continue;
			if ((strlen(kattach_fw_shm->apps.app[zindex].name)) != (strlen(clians))) continue;
			if (!strncmp(clians,kattach_fw_shm->apps.app[zindex].name,strlen(clians))) {
				d = 1;
				break;
			}
		}
		if (!d) {
			printf("\nApp %s not found. Use create or check app name\n",clians);
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
			return;
		} else {
			d = 0;
			sprintf(zname,"%s",clians);
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
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
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
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
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
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
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
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
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

	/* port loop */
	y = 0;
	d = 0;
	while (y < KATTACH_MAX_FW_APP_PORTS) {
		printf("\nNew Application Port\n");
		printf("----------------------\n");
		d = 0;
		sprintf(cliask,"\nICMP Protocol (y/n) ? ");
		while (!d) {
			memset(clians,0,sizeof(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (strlen(clians) == 0) {
				if (y) {
					d = 3;
					break;
				} else {
					appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
					return;
				}
			}
			if (clians[0] == 'y') {
				aprotocol[y] |= KATTACH_FW_PROTOCOL_ICMP;
				d++;
				break;
			} else if (clians[0] == 'n') {
				d++;
				break;
			}
		}
		if (d == 3) break;
		d = 0;
		memset(cliask,0,strlen(cliask));
		sprintf(cliask,"\nTCP Protocol (y/n) ? ");
		while (!d) {
			memset(clians,0,sizeof(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (strlen(clians) == 0) {
				if (y) {
					d = 3;
					break;
				} else {
					appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
					return;
				}
			}
			if (clians[0] == 'y') {
				aprotocol[y] |= KATTACH_FW_PROTOCOL_TCP;
				d++;
				break;
			} else if (clians[0] == 'n') {
				d++;
				break;
			}
		}
		if (d == 3) break;
		d = 0;
		memset(cliask,0,strlen(cliask));
		sprintf(cliask,"\nUDP Protocol (y/n) ? ");
		while (!d) {
			memset(clians,0,sizeof(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (strlen(clians) == 0) {
				if (y) {
					d = 3;
					break;
				} else {
					appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
					return;
				}
			}
			if (clians[0] == 'y') {
				aprotocol[y] |= KATTACH_FW_PROTOCOL_UDP;
				d++;
				break;
			} else if (clians[0] == 'n') {
				d++;
				break;
			}
		}
		if (d == 3) break;
		d = 0;
		memset(cliask,0,strlen(cliask));
		sprintf(cliask,"\nPort (start): ");
		while (!d) {
			memset(clians,0,sizeof(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (strlen(clians) == 0) {
				if (y) {
					d = 3;
					break;
				} else {
					appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
					return;
				}
			}
			asport[y] = atoi(clians);
			if ((asport[y] > 0) && (asport[y] < 65536)) {
				d++;
				break;
			}
		}
		if (d == 3) break;
		d = 0;
		memset(cliask,0,strlen(cliask));
		sprintf(cliask,"\nPort (end): ");
		while (!d) {
			memset(clians,0,sizeof(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (strlen(clians) == 0) {
				if (y) {
					d = 3;
					break;
				} else {
					appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
					return;
				}
			}
			adport[y] = atoi(clians);
			if ((adport[y] > 0) && (adport[y] < 65536)) {
				d++;
				break;
			}
		}
		if (d == 3) break;
		d = 0;
		memset(cliask,0,strlen(cliask));
		sprintf(cliask,"\nPort Direction: (s)ource (d)estination (b)oth: ");
		while (!d) {
			memset(clians,0,sizeof(clians));
			appqueue_cli_askq(cliask,1,clians);
			if (strlen(clians) == 0) {
				if (y) {
					d = 3;
					break;
				} else {
					appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
					return;
				}
			}
			if (clians[0] == 's') {
				adirection[y] = KATTACH_FW_DIR_SOURCE;
				y++;
				d++;
				break;
			} else if (clians[0] == 'd') {
				adirection[y] = KATTACH_FW_DIR_DESTINATION;
				y++;
				d++;
				break;
			} else if (clians[0] == 'b') {
				adirection[y] = KATTACH_FW_DIR_BOTH;
				y++;
				d++;
				break;
			}
		}
		if (d == 3) break;
	}

	appqueue_cli_lock_fw();
	kattach_fw_shm->apps.app[zindex].statemask = statemask;
	kattach_fw_shm->apps.app[zindex].pindex = y;
	sprintf(kattach_fw_shm->apps.app[zindex].name,"%s",zname);
	for (d = 0; d < y; d++) {
		kattach_fw_shm->apps.app[zindex].port[d].direction = adirection[d];
		kattach_fw_shm->apps.app[zindex].port[d].protmask = aprotocol[d];
		kattach_fw_shm->apps.app[zindex].port[d].port[0] = asport[d];
		kattach_fw_shm->apps.app[zindex].port[d].port[1] = adport[d];
	}
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
	return;
}

void appqueue_cli_mf_net_fw_apps_delete(void)
{
	char cliask[255];
	char clians[255];
	char zname[32];
	u32 zindex = 0;
	u16 fw_update = KATTACH_FW_CH_APPS;
	u8 y = 0, d = 0;

	sprintf(cliask,"\nApp Name: ");
	while (!y) {
		memset(clians,0,strlen(clians));
		appqueue_cli_askq(cliask,1,clians);
		if (strlen(clians) == 0) {
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
			return;
		}
		if (strlen(clians) > 32) {
			printf("\nZone name %s is too long. Must be less than 32 characters.\n",clians);
			continue;
		}
		for (zindex = 0; zindex < kattach_fw_shm->apps.index; zindex++) {
			if (kattach_fw_shm->apps.app[zindex].name[0] == '\0') continue;
			if ((strlen(kattach_fw_shm->apps.app[zindex].name)) != (strlen(clians))) continue;
			if (!strncmp(clians,kattach_fw_shm->apps.app[zindex].name,strlen(clians))) {
				d = 1;
				break;
			}
		}
		if (!d) {
			printf("\nApp %s not found. Use create or check app name\n",clians);
			appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
			return;
		} else {
			d = 0;
			sprintf(zname,"%s",clians);
			y++;
			break;
		}
	}

	appqueue_cli_lock_fw();
	kattach_fw_shm->apps.app[zindex].statemask = 0;
	kattach_fw_shm->apps.app[zindex].pindex = 0;
	kattach_fw_shm->apps.app[zindex].name[0] = '\0';
	for (d = 0; d < y; d++) {
		kattach_fw_shm->apps.app[zindex].port[d].direction = 0;
		kattach_fw_shm->apps.app[zindex].port[d].protmask = 0;
		kattach_fw_shm->apps.app[zindex].port[d].port[0] = 0;
		kattach_fw_shm->apps.app[zindex].port[d].port[1] = 0;
	}
	if (kattach_fw_shm->fw_update != (kattach_fw_shm->fw_update | fw_update)) {
		kattach_fw_shm->fw_update |= fw_update;
	}
	appqueue_cli_upd_fw();
	appqueue_cli_current_menu = APPQUEUE_CLI_FP_NET_FW_APPS;
	return;
}

