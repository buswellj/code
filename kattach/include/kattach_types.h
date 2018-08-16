/*
 * kattach (kernel attach)
 * Copyright (c) 2009-2010 Carbon Mountain LLC.
 * All Rights Reserved.
 *
 * John Buswell <buswellj@carbonmountain.com>
 * version 0.6.1.0
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

/* kattach limits */
#define KATTACH_MAX_DEV                 256             /* maximum startup devices */
#define KATTACH_MAX_VBRIDGES            512             /* maximum number of VLANs */
#define KATTACH_MAX_VMSESSIONS          512             /* maximum number of VMs */
#define KATTACH_MAX_VMPORTS             2048            /* maximum number of Virtual Ports */
#define KATTACH_MAX_VPORTS              4               /* maximum number of Virtual Ports per VM */
#define KATTACH_MAX_VMIMAGES            1024            /* maximum number of VM images */
#define KATTACH_MAX_APPS		32		/* maximum number of app modules per VM */
#define KATTACH_MAX_APPMODULES		2048		/* maximum number of app modules in the image library */
#define KATTACH_MAX_IFDEV		256		/* maximum number of hypervisor interfaces */
#define KATTACH_MAX_VNS			512		/* maximum number of virtual network service IPs */
#define KATTACH_MAX_CFGGRP		2048		/* maximum number of configuration groups */
#define KATTACH_MAX_FW			512		/* maximum number of fw filters per chain */
#define KATTACH_MAX_FW_APPS		1024		/* maximum number of support app profiles */
#define KATTACH_MAX_FW_ZONES		512		/* maximum number of firewall zones */
#define KATTACH_MAX_FW_APP_PORTS	16		/* maximum number of ports per app profile */
#define KATTACH_MAX_FW_ZNODES		64		/* maximum number of nodes per zone */
#define KATTACH_MAX_VSP			32		/* maximum number of VNS virtual service ports */

/* kattach defines */
#define KATTACH_LINK_STATUS_DOWN	0x00
#define KATTACH_LINK_STATUS_UP		0x01
#define KATTACH_LINK_STATUS_LACP	0x02
#define KATTACH_LINK_STATUS_DISABLED	0x03
#define KATTACH_LINK_STATUS_UP_10000	0x04
#define KATTACH_LINK_STATUS_UP_1000	0x05
#define KATTACH_LINK_STATUS_UP_100	0x06
#define KATTACH_LINK_STATUS_UP_10	0x07
#define KATTACH_LINK_STATUS_UP_H10000	0x08
#define KATTACH_LINK_STATUS_UP_H1000	0x09
#define KATTACH_LINK_STATUS_UP_H100	0x0a
#define KATTACH_LINK_STATUS_UP_H10	0x0b
#define KATTACH_LINK_STATUS_LACP_NEW	0xad
#define KATTACH_LINK_STATUS_DELETED	0xfe

#define KATTACH_LINK_TYPE_UNKNOWN	0x00
#define KATTACH_LINK_TYPE_ETHERNET	0x01
#define KATTACH_LINK_TYPE_1GBE		0x02
#define KATTACH_LINK_TYPE_10GBE		0x03
#define KATTACH_LINK_TYPE_PSUEDO	0x04
#define KATTACH_LINK_TYPE_INFINIBAND	0x05
#define KATTACH_LINK_TYPE_USB		0x06

#define KATTACH_FW_STMASK_NONE		0x00
#define KATTACH_FW_STMASK_NEW		0x01
#define KATTACH_FW_STMASK_ESTABLISHED	0x02
#define KATTACH_FW_STMASK_RELATED	0x04
#define KATTACH_FW_STMASK_INVALID	0x08

#define KATTACH_FW_PROTOCOL_ICMP	0x01
#define KATTACH_FW_PROTOCOL_TCP		0x02
#define KATTACH_FW_PROTOCOL_UDP		0x04

#define KATTACH_FW_DIR_SOURCE		0x01
#define KATTACH_FW_DIR_DESTINATION	0x02
#define KATTACH_FW_DIR_BOTH		0x03

/* firewall actions */
#define KATTACH_FW_ACTION_ALLOW		0x01
#define KATTACH_FW_ACTION_DROP		0x02
#define KATTACH_FW_ACTION_REJECT	0x03
#define KATTACH_FW_ACTION_LOG		0x04
#define KATTACH_FW_ACTION_TOS		0x05
#define KATTACH_FW_ACTION_MARK		0x06
#define KATTACH_FW_ACTION_TTL		0x07
#define KATTACH_FW_ACTION_DNAT		0x08
#define KATTACH_FW_ACTION_SNAT		0x09
#define KATTACH_FW_ACTION_MASQ		0x10
#define KATTACH_FW_ACTION_REDIR		0x11
#define KATTACH_FW_ACTION_NETMAP	0x12

/* firewall change mask */
#define KATTACH_FW_CH_FILTER_INPUT		0x0001
#define KATTACH_FW_CH_FILTER_OUTPUT		0x0002
#define KATTACH_FW_CH_FILTER_FORWARD		0x0004
#define KATTACH_FW_CH_NAT_PREROUTING		0x0008
#define KATTACH_FW_CH_NAT_POSTROUTING		0x0010
#define KATTACH_FW_CH_NAT_OUTPUT		0x0020
#define KATTACH_FW_CH_MANGLE_PREROUTING		0x0040
#define KATTACH_FW_CH_MANGLE_INPUT		0x0080
#define KATTACH_FW_CH_MANGLE_FORWARD		0x0100
#define KATTACH_FW_CH_MANGLE_OUTPUT		0x0200
#define KATTACH_FW_CH_MANGLE_POSTROUTING	0x0400
#define KATTACH_FW_CH_ZONES			0x0800
#define KATTACH_FW_CH_APPS			0x1000

/* firewall rule origination type */
#define KATTACH_FW_ORIG_USER			0x00
#define KATTACH_FW_ORIG_SYSTEM			0x02
#define KATTACH_FW_ORIG_VM			0x04

/* vendors */
#define KATTACH_VID_LOCAL		0x00
#define KATTACH_VID_CM			0x01

typedef unsigned u8;
typedef unsigned int u16;
typedef unsigned long u32;
typedef unsigned long long u64;

typedef struct {
	u16 status;
        u8 ka_dev;
        u8 ka_cfg;
        u8 ka_inst;
        u8 ka_vmst;
        u8 ka_vmports;
        u8 ka_vbridge;
        u8 ka_vmimages;
	u8 ka_netdev;
	u8 ka_vns;
	u8 ka_cfggrp;
	u8 ka_fw;
        u8 ak_update;
} cm_ak_ping_pong_t;

/* configuration is passed to kattach using /proc/cmdline
 *
 * kaos=[mode]:[partition]:[net]:[ip]:[slash]:[gw]:[dns]
 *
 * kaos=2:001e6caabbcc:0
 *
 * options:
 *
 *  [mode]      unsigned        0 = install, 1 = kaos, 2 = vkaos, 255 = not configured
 *  [net]       string          mac
 *  [ip]        string          0 = DHCP
 *  [slash]     string          slash notation, only used if ip is not 0
 *  [gw]        string          IP of gateway
 *  [dns]       string          IP of DNS server
 *
 *
 */

typedef struct {
        u32 ip;                         /* ip address */
        u32 gw;                         /* gateway ip */
        u32 dns;                        /* dns ip */
        u16 slash;                      /* slash notation */
	u16 ntpint;			/* ntp interval */
	u8 root;			/* enable root access */
        u8 mode;                        /* mode */
        u8 dhcp;                        /* dhcp */
        u8 mac[6];                      /* mac address */
        char netdev[16];                /* network device */
        char storedev[64];              /* storage device */
	char hostname[64];		/* hostname */
	char domain[64];		/* domain */
	char aquser[16];		/* cli username */
	char aqpass[140];		/* cli user SHA encrypted passwd */
	char clipass[140];		/* cli SHA encrypted passwd */
	char rootpass[140];		/* root SHA encrypted passwd */
	char rootpwck[140];
	u32 dns_ip[6];			/* dns ips */
	u32 ntp_ip[3];			/* ntp */
        int pid_dhcpd;                  /* dhcpd pid */
} kattach_cfg_t;

typedef struct {
        char diskboot[64];              /* boot partition */
        char diskswap[64];              /* swap partition */
        char diskappq[64];              /* appq partition */
        char diskdata[64];              /* data partition */
} kattach_install_t;

typedef struct {
        char devname[64];                               /* device name */
        u16 devtype;                                    /* block or char */
        u16 major;                                      /* major number */
        u16 minor;                                      /* minor number */
        u16 res;                                        /* result */
} kattach_dev_entry_t;

typedef struct {
        kattach_dev_entry_t device[KATTACH_MAX_DEV];
        u32 index;                                      /* index, init this to 0 */
} kattach_dev_t;

typedef struct {
        u32 vmimage;                                    /* index into vmimage table */
        u16 vmport[KATTACH_MAX_VPORTS];                 /* index into kattach_vmp_t */
        u8 vmem;                                        /* amount of assigned vmem */
        u8 vcpu;                                        /* amount of assigned vcpus */
        u8 vmstatus;                                    /* vmstatus */
        u8 vmoper;                                      /* operational tag */
        u8 vmpidx;                                      /* next free index in vmport above */
	u8 priority;					/* vm priority */
        int vpid;                                       /* process id for this VM */
        char vmname[255];                               /* vmname */
} kattach_vmst_entry_t;

typedef struct {
        kattach_vmst_entry_t vmsess[KATTACH_MAX_VMSESSIONS];    /* virtual machine session table */
        u32 index;
} kattach_vmst_t;

typedef struct {
        u32 vsubnet;                                    /* subnet assigned to this bridge */
        u32 vbrip;                                      /* ip assigned to the bridge */
        u16 vlan;                                       /* vlan associated with this bridge */
        u16 vmask;                                      /* netmask for this bridge in slash notation */
        u16 vpfree;                                     /* number of IPs available in the pool */
        u16 vbruse;                                     /* number of IPs in use in the pool */
	u8 state;					/* bridge state */
        u8 bmac[6];                                     /* mac address for the bridge */
        u8 vbrlocal;                                    /* vlan mode - local, external etc */
        char vlanext[255];                              /* device to map external vlans to for 802.1Q */
} kattach_vbr_entry_t;

typedef struct {
        kattach_vbr_entry_t vbridge[KATTACH_MAX_VBRIDGES];
        u16 index;
} kattach_vbr_t;

typedef struct {
        u32 vmst;                                       /* index into virtual machine session table */
        u32 vmpip;                                      /* IP assigned to this port, 0 if none */
        u16 vbridge;                                    /* index into virtual bridging table */
        u8 vmac[6];                                     /* virtual mac address assigned to this port */
} kattach_vmp_entry_t;

typedef struct {
        kattach_vmp_entry_t vmports[KATTACH_MAX_VMPORTS];
        u16 index;
} kattach_vmp_t;

typedef struct {
        char vminame[255];
	u32 appindex[KATTACH_MAX_APPS];
	u32 cfggrp[KATTACH_MAX_APPS];
	u8 import;
	u8 appi;
        u8 active;
	u8 changed;
} kattach_vmi_entry_t;

typedef struct {
        kattach_vmi_entry_t vmimage[KATTACH_MAX_VMIMAGES];  
        u32 index;
} kattach_vmi_t;

typedef struct {
	u32 deployed;					/* counter for deployed vms */
	u32 config;					/* counter for number of configured vm images */
	u32 vendor_id;					/* vendor id */
	u16 app_size;					/* size in MB the app takes up */
	u8 revision;					/* revision number */
	u8 srctree;					/* source tree it was pulled from - edge, supported, dev */
	u8 license;					/* license type */
	u8 latest;					/* latest flag */
	char filename[255];				/* filename */
	char buildinfo[255];				/* build info string */
	char url[255];					/* url */
	char name[128];					/* module name */
	char version[32];				/* version */
	char release[32];				/* release - dev, fcs, beta, etc */
	char chksum[512];				/* checksum */
	u8 state;					/* state - new, modified, unchanged */
	int mgrpid;					/* mgrpid */
} kattach_am_entry_t;

typedef struct {
	kattach_am_entry_t appmodules[KATTACH_MAX_APPMODULES];
	u32 index;
} kattach_am_t;

typedef struct {
        char devname[16];                               /* name */
        u32 ip;                                         /* assigned ip, 0 = use dhcp */
        u32 gw;                                         /* gateway */
        u16 mask;                                       /* mask */
        u16 pvid;                                       /* pvid */
        u16 lacpidx;                                    /* if used for lacp, lacpidx */
        u16 mtu;                                        /* mtu */
        u8 type;                                        /* FE, GbE, 10GbE, Infiniband, Psuedo */
        u8 psuedo;                                      /* psuedo type - reserved */
        u8 status;                                      /* up, down, disabled */
        u8 mac[6];                                      /* if mac[] type device */
	u8 vns:1;					/* tag 1 or 0 for VNS */
	u8 spare:7;					/* spare */
} kattach_netdev_entry_t;

typedef struct {
        kattach_netdev_entry_t pif[KATTACH_MAX_IFDEV];
        u16 index;
} kattach_netdev_t;

typedef struct {
	u32 rate_in;					/* pkt cnt (ingress) per time_in interval */
	u32 rate_out;					/* pkt cnt (egress) per time_out interval */
	u16 vsport;					/* virtual service port (outside) */
	u16 vmsport;					/* virtual machine service port (inside) */
	u16 vmport;					/* vm port (on vmst) */
	u8 time_in:1;					/* 0 - minutes or 1 - seconds */
	u8 time_out:1;					/* 0 - minutes or 1 - seconds */
	u8 sproto:1;					/* service protocol, 0 - tcp, 1 - udp */
	u8 enabled:1;					/* port enabled */
	u8 spare:4;					/* unused, pull from here if you need bits */
} kattach_vsp_t;

typedef struct {
	u32 vsip;					/* virtual service IP */
	u8 vsmsk;					/* virtual service mask in CIDR */
	u8 enabled;					/* virtual service is enabled */
	u8 mstate;					/* states allowed -- for firewall */
	u8 vspindex;					/* index into vsp */
	u32 netifidx;					/* index into netdev_t */
	kattach_vsp_t vsp[32];				/* virtual service port */
} kattach_vns_entry_t;

typedef struct {
	kattach_vns_entry_t vns[KATTACH_MAX_VNS];
	u32 index;
} kattach_vns_t;


typedef struct {
	char name[64];
	u32 appmidx;
} kattach_cfggrp_entry_t;

typedef struct {
	kattach_cfggrp_entry_t cfggrp[KATTACH_MAX_CFGGRP];
	u32 index;
} kattach_cfggrp_t;

typedef struct {
	u8 all:1;
	u8 none:1;
	u8 syn:1;
	u8 ack:1;
	u8 fin:1;
	u8 reset:1;
	u8 push:1;
	u8 urgent:1;
} kattach_tcp_flags_t;

typedef struct {
	u32 ip;
	u8 mask;
} kattach_fw_zone_node_t;

typedef struct {
	kattach_fw_zone_node_t	node[KATTACH_MAX_FW_ZNODES];
	u16 vlan;
	u8 nindex;
	char name[32];
} kattach_fw_zone_entry_t;

typedef struct {
	kattach_fw_zone_entry_t zone[KATTACH_MAX_FW_ZONES];
	u32 index;
} kattach_fw_zone_t;

typedef struct {
	u16 port[2];					/* port */
	u8 protmask;					/* protocol mask */
	u8 direction;					/* port direction */
	kattach_tcp_flags_t tcp_flags;			/* tcp flags */
} kattach_fw_app_port_t;

typedef struct {
	kattach_fw_app_port_t port[KATTACH_MAX_FW_APP_PORTS];
	u8 pindex;					/* port index */
	u8 statemask;					/* state machine matching mask - NEW, RELATED, ESTABLISHED, INVALID etc */
	char name[32];					/* application name */
} kattach_fw_app_entry_t;

typedef struct {
	kattach_fw_app_entry_t app[KATTACH_MAX_FW_APPS];
	u32 index;
} kattach_fw_app_t;

typedef struct {
	u32 pindex;					/* previous filter in the chain */
	u32 nindex;					/* next filter in the chain */
	u32 szindex;					/* source zone index */
	u32 dzindex;					/* destination zone index */
	u32 appindex;					/* app index -- provides source / destination port */
	u32 rlimitpkt;					/* rate limit: packets */
	u32 rlimitint;					/* rate limit: interval */
	u16 action;					/* filter action - accept, drop, reject, log, ttl, mark, nat etc */
	u8 ttl[2];					/* TTL - first element is match, second element is set */
	u8 tos[2];					/* TOS - first element is match, second element is set */
	u8 enabled;					/* is the filter enabled */
	u8 type;					/* user defined, system, virtual service */
	u8 rejectwith;					/* if action is reject, reject with this code */
	u8 reverse;					/* reverse the filter logic */
	u8 logging;					/* enable logging */
	char logprefix[32];				/* logging prefix, used if logging is enabled */
} kattach_fw_entry_t;

typedef struct {
	u32 pindex;					/* previous filter in the chain */
	u32 nindex;					/* next filter in the chain */
	u32 szindex;					/* source zone index */
	u32 dzindex;					/* destination zone index */
	u32 nzindex;					/* nat zone index */
	u32 appindex;					/* app index -- provides source / destination port */
	u32 nappindex;					/* nat / redir port app index -- provides source / destination port */
	u32 rlimitpkt;					/* rate limit: packets */
	u32 rlimitint;					/* rate limit: interval */
	u16 action;					/* filter action - accept, drop, reject, log, ttl, mark, nat etc */
	u8 ttl[2];					/* TTL - first element is match, second element is set */
	u8 tos[2];					/* TOS - first element is match, second element is set */
	u8 enabled;					/* is the filter enabled */
	u8 type;					/* user defined, system, virtual service */
	u8 rejectwith;					/* if action is reject, reject with this code */
	u8 reverse;					/* reverse the filter logic */
	u8 logging;					/* enable logging */
	char logprefix[32];				/* logging prefix, used if logging is enabled */
} kattach_fw_n_entry_t;

typedef struct {
	u32 pindex;					/* previous filter in the chain */
	u32 nindex;					/* next filter in the chain */
	u32 szindex;					/* source zone index */
	u32 dzindex;					/* destination zone index */
	u32 appindex;					/* app index -- provides source / destination port */
	u32 rlimitpkt;					/* rate limit: packets */
	u32 rlimitint;					/* rate limit: interval */
	u32 mark;					/* packet marking value */
	u16 action;					/* filter action - accept, drop, reject, log, ttl, mark, nat etc */
	u8 ttl[2];					/* TTL - first element is action(0 = set, 1 = dec, 2 = inc), second element is set */
	u8 tos[2];					/* TOS - first element is match, second element is set */
	u8 enabled;					/* is the filter enabled */
	u8 type;					/* user defined, system, virtual service */
	u8 rejectwith;					/* if action is reject, reject with this code */
	u8 reverse;					/* reverse the filter logic */
	u8 logging;					/* enable logging */
	char logprefix[32];				/* logging prefix, used if logging is enabled */
} kattach_fw_m_entry_t;

typedef struct {
	kattach_fw_entry_t filter[KATTACH_MAX_FW];	/* filter entries */
	u32 hindex;					/* head */
	u32 eindex;					/* end */
	u32 index;					/* index into the filters in this chain */
} kattach_fw_chain_t;

typedef struct {
	kattach_fw_n_entry_t filter[KATTACH_MAX_FW];	/* filter entries */
	u32 hindex;					/* head */
	u32 eindex;					/* end */
	u32 index;					/* index into the filters in this chain */
} kattach_fw_n_chain_t;

typedef struct {
	kattach_fw_m_entry_t filter[KATTACH_MAX_FW];	/* filter entries */
	u32 hindex;					/* head */
	u32 eindex;					/* end */
	u32 index;					/* index into the filters in this chain */
} kattach_fw_m_chain_t;

typedef struct {
	kattach_fw_chain_t input;			/* input chain */
	kattach_fw_chain_t forward;			/* forward chain */
	kattach_fw_chain_t output;			/* output chain */
} kattach_fw_filter_t;

typedef struct {
	kattach_fw_n_chain_t prerouting;		/* prerouting chain */
	kattach_fw_n_chain_t postrouting;		/* postrouting chain */
	kattach_fw_n_chain_t output;			/* output chain */
} kattach_fw_nat_t;

typedef struct {
	kattach_fw_m_chain_t prerouting;		/* prerouting chain */
	kattach_fw_m_chain_t input;			/* input chain */
	kattach_fw_m_chain_t forward;			/* forward chain */
	kattach_fw_m_chain_t output;			/* output chain */
	kattach_fw_m_chain_t postrouting;		/* postrouting chain */
} kattach_fw_mangle_t;

typedef struct {
	kattach_fw_filter_t filter;			/* IP filtering - in/out/forward */
	kattach_fw_nat_t nat;				/* Network Address Translation */
	kattach_fw_mangle_t mangle;			/* IP Packet mangling */
	kattach_fw_zone_t zones;			/* Firewall Zones */
	kattach_fw_app_t apps;				/* Firewall Applications */
	u16 fw_update;					/* update indicator */
} kattach_fw_t;

