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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <ctype.h>
#include "abms.h"

u8 abms_net(void)
{
	int rc = RC_OK;

	abms_mgmtfound = 0;

	if (abms_cfg.mode == ABMS_MODE_NOTCFG) {
	    /* not configured, use the default - dhcp and managed */
	    abms_net_default();
	} else if ((abms_cfg.mode >= ABMS_MODE_RESERVED) && (abms_cfg.mode < ABMS_MODE_NOTCFG)) {
	    /* special handling */
	    switch(abms_cfg.mode) {
		default:
	    	    abms_net_default();
		    break;
	    }

	} else if ((abms_cfg.mode >= ABMS_MODE_DEPLOYED) && (abms_cfg.mode < ABMS_MODE_RESERVED)) {
	    /* system is deployed in production */
	    switch(abms_cfg.mode) {

		case ABMS_MODE_DEPLOYED_MANAGED:
		    abms_net_d_default();
		    break;

		case ABMS_MODE_DEPLOYED_REMOTE:
		    abms_net_d_remote();
		    break;

		case ABMS_MODE_DEPLOYED_STANDALONE:
		    abms_net_d_standalone();
		    break;

		/* not implemented yet */

		case ABMS_MODE_DEPLOYED_NETDEV:

		case ABMS_MODE_DEPLOYED_VM:

		case ABMS_MODE_DEPLOYED:
		default:
	    	    abms_net_default();
		    break;

	    }
	} else {
	    /* system is not deployed in production */
	    switch(abms_cfg.mode) {

		case ABMS_MODE_MANAGED:
		    abms_net_default();
		    break;

		case ABMS_MODE_REMOTE:
		    abms_net_remote();
		    break;

		case ABMS_MODE_STANDALONE:
		    abms_net_standalone();
		    break;

		/* not implemented yet */

		case ABMS_MODE_NETDEV:

		case ABMS_MODE_VM:

		case ABMS_MODE_UNKNOWN:
		default:
	    	    abms_net_default();
		    break;

	    }

	}
	return(rc);

}

void abms_net_d_default(void)
{
	abms_net_default();		/* do basic network setup */
	abms_deployed = 1;		/* flag as production deployment */
	return;
}

void abms_net_d_remote(void)
{
	abms_net_remote();		/* do basic network setup */
	abms_deployed = 1;
	return;
}

void abms_net_d_standalone(void)
{
	abms_net_standalone();		/* do basic network setup */
	abms_deployed = 1;
	return;
}

void abms_net_default(void)
{
	if (abms_cfg.mode != ABMS_MODE_NOTCFG) {
	    if (abms_cfg.mac[1] != 0) {
		abms_net_findmac();		/* find the management device */
	    }
	}

	if ((abms_mgmtfound == 0) || (abms_cfg.mode == ABMS_MODE_NOTCFG)) {
	   abms_net_dhcpscan();			/* crazy scan each device via dhcp */
	}

	if (abms_mgmtfound) {
	   abms_net_setup();			/* setup the network interface */
	}

	return;
}

void abms_net_findmac(void)
{
	int rc = RC_FAIL;
        char entry[512];
        char *delimit;
        char *devname;
        FILE *fp;

	/* note: this code ASSUMES that all network interfaces are down */
	/* this routine finds the mac / dev pair */

        if (!(fp = fopen("/proc/net/dev", "r")))
                return;

        while((devname = fgets(entry, 512, fp))) {
                while(isspace(devname[0]))
                    devname++;

                delimit = strchr (devname, ':');
                if (delimit) {
                    *delimit = 0;
		    if (strncmp(devname,"lo",2)) {
			abms_net_linkup(devname);
			abms_net_linklocal(devname, ABMS_LLADD);
			rc = abms_net_chkmac(devname);
			abms_net_linklocal(devname, ABMS_LLDEL);
			abms_net_linkdown(devname);
		    } else if ((!strncmp(devname,"lo",2)) && (strlen(devname) > 3)) {
			abms_net_linkup(devname);
			abms_net_linklocal(devname, ABMS_LLADD);
			rc = abms_net_chkmac(devname);
			abms_net_linklocal(devname, ABMS_LLDEL);
			abms_net_linkdown(devname);
		    }
		    if (rc == RC_OK) {
		    	sprintf(abms_mgmtdev,"%s",devname);
			abms_mgmtfound = 1;
		    	fclose(fp);
		    	return;
		    }
                }
        }
        fclose(fp);

}

void abms_net_linkup(char *devname)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s link set %s up",ABMS_BINPATH,ACMD_IP,devname);
	abms_system(lcmd);
	return;
}

void abms_net_linkdown(char *devname)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s link set %s down",ABMS_BINPATH,ACMD_IP,devname);
	abms_system(lcmd);
	return;

}

u8 abms_net_chkmac(char *devname)
{
	int sck;
	char buf[1024];
	struct ifconf ifc;
        struct ifreq *ifr;
        int           nInterfaces;
        int           i;

	sck = socket(AF_INET, SOCK_DGRAM, 0);
        if (sck < 0) {
                perror("socket");
		return RC_FAIL;
	}	

	ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = buf;
        if (ioctl(sck, SIOCGIFCONF, &ifc) < 0) {
                perror("ioctl(SIOCGIFCONF)");
                return RC_FAIL;
        }

        ifr         = ifc.ifc_req;
        nInterfaces = ifc.ifc_len / sizeof(struct ifreq);

	/* scan interfaces for matching mac */

        for (i = 0; i < nInterfaces; i++) {
                struct ifreq *item = &ifr[i];
                struct ether_addr *mac;
		int devlen = 0, iflen = 0, biglen = 0, macc = 0, macm = 0;

                if (ioctl(sck, SIOCGIFHWADDR, item) < 0) {
                        perror("ioctl(SIOCGIFHWADDR)");
                        continue;
                }

		devlen = strlen(devname);
		iflen = strlen(item->ifr_name);
		biglen = (devlen > iflen) ? devlen : iflen;

		/* check to see if this is the device we are interested in */
		if (strncmp(devname,item->ifr_name,biglen)) continue;

		/* grab the mac from the active device */
                mac = (struct ether_addr *)item->ifr_hwaddr.sa_data;
		
		/* now compare the mac */
		for (macc = 0; macc < 6; macc++) {
		    if (mac->ether_addr_octet[macc] == abms_cfg.mac[macc]) {
			macm++;
		    } else {
			macm = 0;
			break;
		    }
		}

		if (macm == 6) {
		    printf("\n[*] Found %02x:%02x:%02x:%02x:%02x:%02x on %s\n", abms_cfg.mac[0], abms_cfg.mac[1], abms_cfg.mac[2], abms_cfg.mac[3], abms_cfg.mac[4], abms_cfg.mac[5], item->ifr_name);
		    return(RC_OK);
		} else {
		    printf("\n[*] %02x:%02x:%02x:%02x:%02x:%02x is NOT on %s\n", abms_cfg.mac[0], abms_cfg.mac[1], abms_cfg.mac[2], abms_cfg.mac[3], abms_cfg.mac[4], abms_cfg.mac[5], item->ifr_name);
		    return(RC_FAIL);
		}
        }
	return(RC_FAIL);

}

void abms_net_setup(void)
{
	char lcmd[255];
	u8 did_dhcp = 0;

	/* configure IP or DHCP on abms_mgmtdev */
	abms_net_linkup(abms_mgmtdev);
	if ((abms_cfg.ip == 0) || (!abms_net_chkmask(abms_cfg.slash))) {
	    abms_net_dhcp(abms_mgmtdev);
	    did_dhcp = 1;
	} else {
	    /* add IP */
	    sprintf(lcmd,"%s%s addr add %s/%lu dev %s",ABMS_BINPATH,ACMD_IP,abms_net_parseip(abms_cfg.ip),abms_cfg.slash,abms_mgmtdev);
	    abms_system(lcmd);
	    free(abms_netipp);
	}
	if (abms_cfg.gw != 0) {
	    /* add default route */
	    /* XXX: we assume if gw configured that we want to override DHCP.
	            we should check that the configured route works before doing this!
	     */
	    if (did_dhcp) {
	        sprintf(lcmd,"%s%s route del default",ABMS_BINPATH,ACMD_IP);
	        abms_system(lcmd);
	    }
	    sprintf(lcmd,"%s%s route add default via %s dev %s",ABMS_BINPATH,ACMD_IP,abms_net_parseip(abms_cfg.gw),abms_mgmtdev);
	    abms_system(lcmd);
	    free(abms_netipp);
	}
	if (abms_cfg.dns != 0) {
	    /* update dns */
	    sprintf(lcmd,"%s%s 'nameserver %s' > /etc/resolv.conf",ABMS_BINPATH,ACMD_ECHO,abms_net_parseip(abms_cfg.dns));
	    abms_system(lcmd);
	    free(abms_netipp);
	}
	return;
}

void abms_net_dhcpscan(void)
{
        char entry[512];
        char *delimit;
        char *devname;
        FILE *fp;

	/* note: this code ASSUMES that all network interfaces are down */
	/* this is a bit of a hack, needs to be made smarter! */

        if (!(fp = fopen("/proc/net/dev", "r")))
                return;

        while((devname = fgets(entry, 512, fp))) {
                while(isspace(devname[0]))
                    devname++;

                delimit = strchr (devname, ':');
                if (delimit) {
                    *delimit = 0;
		    if (strncmp(devname,"lo",2)) {
			abms_net_linkup(devname);
			abms_net_dhcp(devname);
		    } else if ((!strncmp(devname,"lo",2)) && (strlen(devname) > 3)) {
			abms_net_linkup(devname);
			abms_net_dhcp(devname);
		    }
                }
        }
        fclose(fp);
}

void abms_net_remote(void)
{
	/*	u8 rc = 0; */

	if (abms_cfg.mac[1] != 0) {
	    abms_net_findmac();             /* find the management device */
	}

	if (abms_mgmtfound == 0) {
	    /* rc = abms_net_remote_recover(); */
	} else {
	    abms_net_setup();
	}
	/* first check the gw */
	/* abms_net_chkgw(); */
	/* next get the vpn package (wget) and start it */
	/* abms_net_dovpn(); */
}

void abms_net_standalone(void)
{
	if (abms_cfg.mode != ABMS_MODE_NOTCFG) {
	    if (abms_cfg.mac[1] != 0) {
		abms_net_findmac();		/* find the management device */
	    }
	}

	if ((abms_mgmtfound == 0) || (abms_cfg.mode == 0xff)) {
	   abms_net_dhcpscan();			/* crazy scan each device via dhcp */
	}

	if (abms_mgmtfound) {
	   abms_net_setup();			/* setup the network interface */
	}

	return;
}

void abms_net_dhcp(char *devname)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s %s",ABMS_BINPATH,ACMD_DHCP,devname);
	abms_system(lcmd);

	return;
}

u8 abms_net_chkmask(u16 slash)
{
	if ((slash > ABMS_MAXSLASH) || (slash < ABMS_MINSLASH)) {
	    return(RC_FAIL);
	} else {
	    return(RC_OK);
	}
}

char *abms_net_parseip(u32 iptp)
{
	abms_netipp = (char *) malloc(24);

	sprintf(abms_netipp,"%lu.%lu.%lu.%lu",(u16) ((iptp >> 24) & 0xff), (u16) ((iptp >> 16) & 0xff), (u16) ((iptp >> 8) & 0xff), (u16) ((iptp) & 0xff));
	return(abms_netipp);

}

void abms_net_loopback(void)
{
        char lcmd[255];

        abms_net_linkup(ABMS_LOOPDEV);
        sprintf(lcmd,"%s%s addr add %s dev %s",ABMS_BINPATH,ACMD_IP,ABMS_LOOPBACK,ABMS_LOOPDEV);
        abms_system(lcmd);

        return;
}

void abms_net_linklocal(char *devname, u8 action)
{
	char lcmd[255];

	if (action == ABMS_LLADD) {
	    sprintf(lcmd,"%s%s addr add %s/%u dev %s",ABMS_BINPATH,ACMD_IP,ABMS_LINKLOCAL,ABMS_LLSLASH,devname);
	    abms_system(lcmd);
	} else {
	    sprintf(lcmd,"%s%s addr del %s/%u dev %s",ABMS_BINPATH,ACMD_IP,ABMS_LINKLOCAL,ABMS_LLSLASH,devname);
	    abms_system(lcmd);
	}
}

void abms_net_sshd(void)
{
	char lcmd[255];

        if ((abms_cfg.sshport == ABMS_SSHPORT) || (abms_cfg.sshport == 0)) {
            sprintf(lcmd,"%s -r %s -p %d -E",ABMS_SSHD,ABMS_KEY,ABMS_SSHPORT);
        } else {
            sprintf(lcmd,"%s -r %s -p %lu -E",ABMS_SSHD,ABMS_KEY,abms_cfg.sshport);
        }
        abms_system(lcmd);
}
