/*
 * kattach (kernel attach)
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
 * Source File : 		kattach-net.c  
 * Purpose     :                network initialization
 * Callers     :                kattach.c - main()
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netpacket/packet.h>
#include <netinet/in.h>
#include <net/ethernet.h>
#include <ctype.h>
#include <unistd.h>
#include <math.h>
#include "kattach_types.h"
#include "kattach.h"

void
kattach_network(void)
{
	kattach_netdev_t *nPtr = &kattach_netdev;
	memset(nPtr,0,sizeof(kattach_netdev_t));

	kattach_net_loop();

	switch (kattach_cfg.mode) {

		case KATTACH_MODE_VKAOS:
			kattach_net_vkaos();
			break;

		case KATTACH_MODE_KAOS:
			kattach_net_findmac();
			kattach_net_setup();
			break;

		case KATTACH_MODE_NOTCFG:
		case KATTACH_MODE_UNKNOWN:
		default:
			kattach_net_dhcpscan();
			break;


	}

	return;

}

void
kattach_net_loop(void)
{
	char lcmd[255];

	printf("\n [*] Enabling lo network interface \n");

	sprintf(lcmd,"%s%s link set lo up",KATTACH_BINPATH,KCMD_IP);
	kattach_sysexec(lcmd);

	sprintf(lcmd,"%s%s addr add 127.0.0.1/8 dev lo",KATTACH_BINPATH,KCMD_IP);
	kattach_sysexec(lcmd);

	return;
}

void
kattach_net_vkaos(void)
{
	char entry[512];
	char *delimit;
        char *devname;
        FILE *fp;

        /* note: this code ASSUMES that all network interfaces are down */
	/*       it finds eth* devices and brings them up with dhcp */

        if (!(fp = fopen("/proc/net/dev", "r")))
                return;

        while((devname = fgets(entry, 512, fp))) {
                while(isspace(devname[0]))
                    devname++;

                delimit = strchr (devname, ':');
                if (delimit) {
                    *delimit = 0;
                    if (strncmp(devname,"eth",3)) {
			kattach_net_dhcp(devname);
                    }
                }
        }
        fclose(fp);
	return;
}

void
kattach_net_dhcpscan(void)
{
        char entry[512];
        char *delimit;
        char *devname;
        FILE *fp;

        /* note: this code ASSUMES that all network interfaces are down */
        /* this is a bit of a hack, we puke dhcp requests out all interfaces!  */

        if (!(fp = fopen("/proc/net/dev", "r")))
                	return;

        while((devname = fgets(entry, 512, fp))) {
                while(isspace(devname[0]))
                	devname++;

                delimit = strchr (devname, ':');
                if (delimit) {
                	*delimit = 0;
		    	if ((strncmp(devname,"bond",4)) && (strncmp(devname,"eql",3)) &&
				(strncmp(devname,"teql",4)) && (strncmp(devname,"tunl",4)) &&
				(strncmp(devname,"gre",3)) && (strncmp(devname,"sit",3))) {
                    		if (strncmp(devname,"lo",2)) {
                        		kattach_net_dhcp(devname);
                    		} else if ((!strncmp(devname,"lo",2)) && (strlen(devname) > 3)) {
                        		kattach_net_dhcp(devname);
                    		}
		    	}
                }
        }
        fclose(fp);
	return;
}

void
kattach_net_dhcp(char *devname)
{
	char lcmd[255];

	kattach_net_linkup(devname);

	sprintf(lcmd,"%s%s -b -q -C hostname -C resolv.conf -C mtu %s",KATTACH_DHCPCPATH,KCMD_DHCPC,devname);
	kattach_sysexec(lcmd);

	return;
}

void
kattach_net_linkup(char *devname)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s link set %s up",KATTACH_BINPATH,KCMD_IP,devname);
	kattach_sysexec(lcmd);
	return;
}

void
kattach_net_linkdown(char *devname)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s link set %s down",KATTACH_BINPATH,KCMD_IP,devname);
	kattach_sysexec(lcmd);
	return;
}

u8
kattach_net_chkmask(u16 slash)
{
        if ((slash > KATTACH_NET_MAXSLASH) || (slash < KATTACH_NET_MINSLASH)) {
            return(RC_FAIL);
        } else {
            return(RC_OK);
        }
}

char *
kattach_net_parseip(u32 iptp)
{
        kattach_netipp = (char *) malloc(24);

        sprintf(kattach_netipp,"%u.%u.%u.%u",(u16) ((iptp >> 24) & 0xff), (u16) ((iptp >> 16) & 0xff), (u16) ((iptp >> 8) & 0xff), (u16) ((iptp) & 0xff));
        return(kattach_netipp);

}

void
kattach_net_findmac(void)
{
        u8 rc = RC_FAIL;
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
		    	if ((strncmp(devname,"bond",4)) && (strncmp(devname,"eql",3)) &&
                        	(strncmp(devname,"teql",4)) && (strncmp(devname,"tunl",4)) &&
                                (strncmp(devname,"gre",3)) && (strncmp(devname,"sit",3))) {

                    		if (strncmp(devname,"lo",2)) {
                        		rc = kattach_net_chkmac(devname);
                    		} else if ((!strncmp(devname,"lo",2)) && (strlen(devname) > 3)) {
                        		rc = kattach_net_chkmac(devname);
                    		}
                    		if (rc == RC_OK) {
                        		sprintf(kattach_cfg.netdev,"%s",devname);
                        		kattach_macfound = 1;
                        		fclose(fp);
                        		return;
                    		}
			}
                }
        }
        fclose(fp);
	return;
}

u8
kattach_net_chkmac(char *devname)
{
        int sck, ret, macc = 0, macm = 0;
	struct sockaddr_ll l2eth;
	socklen_t l2len;
        struct ifreq ifr;

	sck = socket(PF_PACKET, SOCK_DGRAM, 0);
        if (sck < 0) {
                perror("socket");
                return RC_FAIL;
        }       

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, devname, strlen(devname));
        if (ioctl(sck, SIOCGIFINDEX, &ifr) < 0) {
                perror("ioctl(SIOCGIFINDEX)");
                return RC_FAIL;
        }

	memset(&l2eth, 0, sizeof(l2eth));
	l2eth.sll_family = AF_PACKET;
        l2eth.sll_ifindex = ifr.ifr_ifindex;
        l2eth.sll_protocol = htons(ETH_P_LOOP);
	bind(sck, (struct sockaddr*)&l2eth, sizeof(l2eth));
	l2len = sizeof(l2eth);
	ret = getsockname(sck, (struct sockaddr*)&l2eth, &l2len);
	if (ret < 0) return RC_FAIL;
	close(sck);

	if ((unsigned int) l2eth.sll_halen == 6) {
		for (macc = 0; macc < 6; macc++) {
			if ((u8) l2eth.sll_addr[macc] == kattach_cfg.mac[macc]) {
				macm++;
			} else {
				macm = 0;
				break;
			}
		}

		if (macm == 6) {
			return RC_OK;
		} else {
			return RC_FAIL;
		}
	} else {
		return RC_FAIL;
	}
}

u8
kattach_net_getmac(char *devname)
{
        int sck, ret, macc = 0;
	struct sockaddr_ll l2eth;
	socklen_t l2len;
        struct ifreq ifr;

	sck = socket(PF_PACKET, SOCK_DGRAM, 0);
        if (sck < 0) {
                perror("socket");
                return RC_FAIL;
        }       

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, devname, strlen(devname));
        if (ioctl(sck, SIOCGIFINDEX, &ifr) < 0) {
                perror("ioctl(SIOCGIFINDEX)");
                return RC_FAIL;
        }

	memset(&l2eth, 0, sizeof(l2eth));
	l2eth.sll_family = AF_PACKET;
        l2eth.sll_ifindex = ifr.ifr_ifindex;
        l2eth.sll_protocol = htons(ETH_P_LOOP);
	bind(sck, (struct sockaddr*)&l2eth, sizeof(l2eth));
	l2len = sizeof(l2eth);
	ret = getsockname(sck, (struct sockaddr*)&l2eth, &l2len);
	if (ret < 0) return RC_FAIL;
	close(sck);

	if ((unsigned int) l2eth.sll_halen == 6) {
		for (macc = 0; macc < 6; macc++) {
			kattach_genmac[macc] = (u8) l2eth.sll_addr[macc];
		}
		return RC_OK;
	} else {
		return RC_FAIL;
	}
}

void
kattach_net_setup(void)
{
	if (kattach_macfound != 1) {
		printf(" [*] Unable to locate MAC %02x:%02x:%02x:%02x:%02x:%02x\n", kattach_cfg.mac[0],kattach_cfg.mac[1],kattach_cfg.mac[2],
								kattach_cfg.mac[3],kattach_cfg.mac[4],kattach_cfg.mac[5]);
		printf(" [*] Attemping DHCP scan instead...\n");
		kattach_net_dhcpscan();
		return;
	}

	kattach_net_linkup(kattach_cfg.netdev);

	if (kattach_cfg.dhcp) {
		kattach_net_dhcp(kattach_cfg.netdev);
	} else if (kattach_net_chkmask(kattach_cfg.slash) == RC_FAIL) {
		kattach_net_dhcp(kattach_cfg.netdev);
	} else {
		kattach_net_ip_add(kattach_cfg.ip, kattach_cfg.slash, kattach_cfg.netdev);
		kattach_net_rte_add_default(kattach_cfg.gw);
		kattach_net_dns_add(kattach_cfg.dns);
	}
	return;
}

void
kattach_net_ip_add(u32 ip, u16 slash, char *netdev)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s addr add %s/%u dev %s",KATTACH_BINPATH,KCMD_IP,kattach_net_parseip(kattach_cfg.ip),slash,netdev);
	printf(" [*] ip-add: %s\n",lcmd);
	kattach_sysexec(lcmd);
	free(kattach_netipp);

	return;

}

void
kattach_net_ip_del(u32 ip, u16 slash, char *netdev)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s addr del %s/%u dev %s",KATTACH_BINPATH,KCMD_IP,kattach_net_parseip(kattach_cfg.ip),slash,netdev);
	printf(" [*] ip-del: %s\n",lcmd);
	kattach_sysexec(lcmd);
	free(kattach_netipp);

	return;

}

void
kattach_net_rte_add_default(u32 ip)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s route add default via %s dev %s",KATTACH_BINPATH,KCMD_IP,kattach_net_parseip(kattach_cfg.gw),kattach_cfg.netdev);
	printf(" [*] rte-add: %s\n",lcmd);
	kattach_sysexec(lcmd);
	free(kattach_netipp);

	return;

}

void
kattach_net_rte_del_default(u32 ip)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s route del default via %s dev %s",KATTACH_BINPATH,KCMD_IP,kattach_net_parseip(kattach_cfg.gw),kattach_cfg.netdev);
	printf(" [*] rte-del: %s\n",lcmd);
	kattach_sysexec(lcmd);
	free(kattach_netipp);

	return;

}

void
kattach_net_dns_add(u32 ip)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s 'nameserver %s' > %s",KATTACH_BINPATH,KCMD_ECHO,kattach_net_parseip(kattach_cfg.dns),KATTACH_NET_RESOLVCONF);
	printf(" [*] dns-add: %s\n",lcmd);
	kattach_sysexec(lcmd);
	free(kattach_netipp);

	return;
}

void
kattach_net_genmac(void)
{
	unsigned long xresult = 0;
        unsigned long yresult = 0;
        unsigned long aresult = 0;
        unsigned long bresult = 0;
        unsigned long cresult = 0;
        int i = 0;
#if !defined(KATTACH_BLD_VKAOS)
	u8 dup = 0;
#endif /* !defined(KATTACH_BLD_VKAOS) */

        kattach_getbuuid();
        kattach_getruuid();

        xresult = kattach_hash(kattach_buuid);
        yresult = kattach_hash(kattach_ruuid);
        aresult = xresult ^ yresult;

        kattach_getruuid();
        yresult = kattach_hash(kattach_ruuid);
        bresult = xresult ^ yresult;

        kattach_getruuid();
        yresult = kattach_hash(kattach_ruuid);
        cresult = xresult ^ yresult;

        kattach_genmac[0] = 0x00;
        kattach_genmac[1] = 0x1e;
        kattach_genmac[2] = 0x6c;
        kattach_genmac[3] = (aresult & 0xff);
        kattach_genmac[4] = (bresult & 0xff);
        kattach_genmac[5] = (cresult & 0xff);

        for (i = 8; i <= 56; i+=8) {
                kattach_genmac[3] ^= ((aresult >> i) & 0xff);
                kattach_genmac[4] ^= ((bresult >> i) & 0xff);
                kattach_genmac[5] ^= ((cresult >> i) & 0xff);
        }

#if !defined(KATTACH_BLD_VKAOS)
	dup = kattach_vm_dupmac();

	if (dup) {
		kattach_net_genmac();
		return;
	} else {
		return;
	}
#else /* !defined(KATTACH_BLD_VKAOS) */
	return;
#endif /* !defined(KATTACH_BLD_VKAOS) */
}

u32
kattach_net_mask(u16 slash)
{
	u32 mask = ~(0xffffffff >> slash);
	return (mask);
}

u32
kattach_net_bcast(u32 ip, u16 slash)
{
	u32 mask = kattach_net_mask(slash);
	u32 hostmask = mask ^ 0xffffffff;
	u32 bcast = ip | hostmask;

	return (bcast);
}

u32
kattach_net_netaddr(u32 ip, u16 slash)
{
	u32 mask = kattach_net_mask(slash);
	u32 neta = ip & mask;

	return (neta);

}

u32
kattach_net_ip_assign(u32 subnet, u16 slash)
{
	u32 genip = 0, xresult = 0, yresult = 0, zresult = 0;
	u8 unmask = (32 - slash);	
#if !defined(KATTACH_BLD_VKAOS)
	u8 dup = 0;
#endif /* !defined(KATTACH_BLD_VKAOS) */
	u32 ip_usable = ((unsigned long) pow(2,unmask) - 2);

        kattach_getruuid();
	xresult = kattach_hash(kattach_ruuid);
        kattach_getruuid();
	yresult = kattach_hash(kattach_ruuid);
	zresult = ((xresult ^ yresult) & ip_usable);

	genip = subnet | zresult;

	while (genip == (kattach_net_netaddr(genip,slash))) {
	        kattach_getruuid();
		xresult = kattach_hash(kattach_ruuid);
	        kattach_getruuid();
		yresult = kattach_hash(kattach_ruuid);
		zresult = ((xresult ^ yresult) & ip_usable);
		genip = subnet | zresult;
	}
	
#if !defined(KATTACH_BLD_VKAOS)
	dup = kattach_vm_dupip(genip);

	if (dup) {
		genip = kattach_net_ip_assign(subnet, slash);
	} 
#endif /* !defined(KATTACH_BLD_VKAOS) */
	return(genip);
}
