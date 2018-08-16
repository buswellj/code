/*
 * kattach (kernel attach)
 * Copyright (c) 2009-2010 Carbon Mountain LLC.
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

#define KAOS_DBVER			2
#define KATTACH_KAOS_DB			"/appq/db/kaos.db"
#define KATTACH_APPQUEUE_DB             "/appq/db/appqueue.db"
#define KATTACH_VMSESSION_DB		"/appq/db/vmsession.db"

#define K_DB_KAOS			1
#define K_DB_APPQ			2
#define K_DB_VMSESSION			3

/* globals */
sqlite3 *db_kaos, *db_appq, *db_vmsession;


/* function prototypes */
u8 kattach_sql_dbopen(char *dbpath, u8 dbPtr);
u8 kattach_sql_testdb(char *sqlq, u8 dbPtr);
u8 kattach_sql_chkdb(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_vmsession(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_vbridge(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_vmports(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_vmimage(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_vmapps(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_cfggrp(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_appmodule(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_iu(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_vns_vsn(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_vns_vslink(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_fw_main(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_fw_znodes(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_fw_zones(char *sqlq, u8 dbPtr); 
u8 kattach_vm_sql_fw_zlink(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_fw_apps(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_fw_appports(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_fw_alink(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_fw_filter_input(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_fw_filter_output(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_fw_filter_forward(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_fw_nat_prerouting(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_fw_nat_postrouting(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_fw_nat_output(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_fw_mangle_input(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_fw_mangle_output(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_fw_mangle_forward(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_fw_mangle_prerouting(char *sqlq, u8 dbPtr);
u8 kattach_vm_sql_fw_mangle_postrouting(char *sqlq, u8 dbPtr);

void kattach_sql_create_kaosdb(void);
void kattach_sql_create_appqdb(void);
void kattach_sql_create_vmsessdb(void);
void kattach_sql_update_kaosdb(void);
void kattach_sql_logdb(char *sqlq);

/* SQL Queries */

/* Database Upgrades */
/* alter tables */
/* version 1 to version 2 db */
#define CMSQL_V1_V2_ALTER_TABLE_APPMODULE "ALTER TABLE appmodule ADD COLUMN appsize INTEGER;"
#define CMSQL_V1_V2_UPDATE_TABLE_CMINFO "UPDATE cminfo SET status = 2 WHERE status = '1';"
#define CMSQL_V1_V2_INSERT_TABLE_CMINFO "INSERT INTO cminfo VALUES (2,1);"



/* kaos.db */
/* create tables */
#define CMSQL_KAOS_CREATE_TABLE_CMINFO "CREATE table cminfo(version INTEGER, status INTEGER);"

#define CMSQL_KAOS_CREATE_TABLE_NETDEV "\
CREATE TABLE netdev(\
ifindex INTEGER PRIMARY KEY, devname TEXT, ip INTEGER, gw INTEGER, mask INTEGER, pvid INTEGER, lacpidx INTEGER, mtu INTEGER,\
pseudo INTEGER, status INTEGER, macA INTERGER, macB INTEGER, macC INTEGER, macD INTEGER, macE INTEGER, macF INTEGER);"

#define CMSQL_KAOS_CREATE_TABLE_BOOTCFG "\
CREATE TABLE bootcfg(\
bindex INTEGER PRIMARY KEY, ip INTEGER, gw INTEGER, dns INTEGER, slash INTEGER,\
mode INTEGER, dhcp INTEGER, macA INTEGER, macB INTEGER, macC INTEGER, macD INTEGER,\
macE INTEGER, macF INTEGER, netdev TEXT, storedev TEXT);"

#define CMSQL_KAOS_CREATE_TABLE_DEVICES "\
CREATE TABLE devices(\
dindex INTEGER PRIMARY KEY, devname TEXT, devtype INTEGER, major INTEGER, minor INTEGER, res INTEGER);"

#define CMSQL_KAOS_CREATE_TABLE_EXTRADEVICES "\
CREATE TABLE extradevices(\
dindex INTEGER PRIMARY KEY, devname TEXT, devtype INTEGER, major INTEGER, minor INTEGER, res INTEGER);"

#define CMSQL_KAOS_CREATE_TABLE_CONFIG "\
CREATE TABLE config(\
cindex INTEGER PRIMARY KEY, hostname TEXT, autoupgrade INTEGER, bootdev TEXT, swapdev TEXT, appqdev TEXT, datadev TEXT, domain TEXT,\
dnsA INTEGER, dnsB INTEGER, dnsC INTEGER, dnsD INTEGER, dnsE INTEGER, dnsF INTEGER, ntpint INTEGER, ntpA INTEGER, ntpB INTEGER, ntpC INTEGER,\
aquser TEXT, aqpass TEXT, clipass TEXT, rootpass TEXT, root INTEGER);"

/* inserts */
#define CMSQL_KAOS_INSERT_CMINFO "INSERT INTO cminfo VALUES(1,1);"

/* select queries */
#define CMSQL_KAOS_SELECT_CMINFO "SELECT * from cminfo WHERE status=1 LIMIT 1;"
#define CMSQL_KAOS_SELECT_BOOTCFG "SELECT * from bootcfg ORDER BY DESC LIMIT 1;"
#define CMSQL_KAOS_SELECT_NETDEV "SELECT * from netdev;"

/* drop */
#define CMSQL_KAOS_DROP_DEVICES "DROP TABLE devices;"
#define CMSQL_KAOS_DROP_NETDEV "DROP TABLE netdev;"

/* appqueue.db */
/* create triggers */
#define CMSQL_APPQ_CREATE_TRIGGER_APPMODULE "CREATE TRIGGER insert_appmodule_installstamp AFTER INSERT on appmodule \
BEGIN\
 UPDATE appmodule SET installstamp = DATETIME('NOW') WHERE rowid = new.rowid; \
END;"

/* create tables */
#define CMSQL_APPQ_CREATE_TABLE_CMINFO "CREATE table cminfo(version INTEGER, status INTEGER);"

#define CMSQL_APPQ_CREATE_TABLE_APPMODULE "\
CREATE TABLE appmodule(\
appindex INTEGER PRIMARY KEY, name TEXT, vendor INTEGER, vurl TEXT, version TEXT, release TEXT,\
srctree INTEGER, buildstamp DATE, buildinfo TEXT, chksum TEXT, license INTEGER, installstamp DATE,\
filename TEXT, revision INTEGER, latest INTEGER, appsize INTEGER);"

#define CMSQL_APPQ_CREATE_TABLE_VMIMAGE "CREATE TABLE vmimage(vmindex INTEGER PRIMARY KEY, active INTEGER, vminame TEXT, appindex INTEGER, FOREIGN KEY (appindex) REFERENCES appmodule(appindex));"
#define CMSQL_APPQ_CREATE_TABLE_VMAPPS "CREATE TABLE vmapps(vmappidx INTEGER PRIMARY KEY, appindex INTEGER, vmindex INTEGER, cfgindex, FOREIGN KEY (appindex) REFERENCES appmodule(appindex),FOREIGN KEY (vmindex) REFERENCES vmimage(vmindex), FOREIGN KEY (cfgindex) REFERENCES cfggrp(cfggidx));"
#define CMSQL_APPQ_CREATE_TABLE_CFGGRP "CREATE TABLE cfggrp(cfggidx INTEGER PRIMARY KEY, name TEXT, appindex INTEGER, FOREIGN KEY (appindex) REFERENCES appmodule(appindex));"

/* inserts */
#define CMSQL_APPQ_INSERT_CMINFO "INSERT INTO cminfo VALUES(1,1);"

/* select queries */
#define CMSQL_APPQ_SELECT_CMINFO "SELECT * from cminfo WHERE status=1 LIMIT 1;"
#define CMSQL_APPQ_SELECT_VMIMAGE "SELECT * from vmimage;"
#define CMSQL_APPQ_SELECT_VMAPPS "SELECT * from vmapps;"
#define CMSQL_APPQ_SELECT_APPMODULE "SELECT * from appmodule;"
#define CMSQL_APPQ_SELECT_CFGGRP "SELECT * from cfggrp;"

/* vmsession.db */

/* Firewall rules belong to virtual ports */
/* Virtual ports are the ethX interfaces within a VM, and belong to a virtual bridge */
/* Virtual bridges belong to a VLAN and subnet, and (optionally) can be linked to an external device */
/* VM session entries tie a virtual machine instance to a vmimage, vmport and virtual bridge */

/* create triggers */
#define CMSQL_VMSESS_CREATE_TRIGGER_VMSESS "CREATE TRIGGER insert_vmsess_vmboot AFTER INSERT on vmsess \
BEGIN\
 UPDATE vmsess SET vmboot = DATETIME('NOW') WHERE rowid = new.rowid; \
END;"

/* create tables */
#define CMSQL_VMSESS_CREATE_TABLE_CMINFO "CREATE table cminfo(version INTEGER, status INTEGER);"
#define CMSQL_VMSESS_CREATE_TABLE_VMSESS "CREATE table vmsess(\
vmindex INTEGER PRIMARY KEY, vmstatus INTEGER, vmimage INTEGER, vmboot DATE, vmname TEXT, vmem INTEGER, vcpu INTEGER,\
priority INTEGER, vmport INTEGER, FOREIGN KEY (vmport) REFERENCES vmport(vmpindex));"

/* Virtual Bridge Table, one bridge per VLAN, vlan id, vsubnet is the ip network for the vlan, vmask is the netmask in slash notation */
/* vbrip is the hypervisors ip in this subnet, its assigned to vbrX where X is the VLAN ID, vbrlocal indicates if the VLAN is local to the hypervisor */
/* vlanext is used if vbrlocal is not local, its the external device used for 802.1Q, NAT or routing. vpfree is the number of free IPs in the pool */
/* When vpfree is 0, no more VMs can be spawned using this VLAN. vbruse is incremented up, if its non-zero, the VLAN cannot be deleted by AppQueue */

#define CMSQL_VMSESS_CREATE_TABLE_VBRIDGE "CREATE table vbridge(\
vbrindex INTEGER PRIMARY KEY, vlan INTEGER, vsubnet INTEGER, vmask INTEGER, vbrip INTEGER, vbrlocal INTEGER, vlanext TEXT, vpfree INTEGER, vbruse INTEGER);"

#define CMSQL_VMSESS_CREATE_TABLE_VMPORT "CREATE table vmport(\
vmpindex INTEGER PRIMARY KEY, vmacA INTEGER, vmacB INTEGER, vmacC INTEGER, vmacD INTEGER, vmacE INTEGER, vmacF INTEGER, vmowner INTEGER,\
vbridge INTEGER, vmpip INTEGER, FOREIGN KEY (vmowner) REFERENCES vmsess(vmindex), FOREIGN KEY (vbridge) REFERENCES vbridge(vbrindex));"

/* The zlink table joins nodes with zones */
#define CMSQL_VMSESS_CREATE_TABLE_ZNODES "CREATE table znodes(znindex INTEGER PRIMARY KEY, ip INTEGER, mask INTEGER);"
#define CMSQL_VMSESS_CREATE_TABLE_ZONES "CREATE table zones(zindex INTEGER PRIMARY KEY, name TEXT, vlan INTEGER);"
#define CMSQL_VMSESS_CREATE_TABLE_ZLINK "CREATE table zlink(zlindex INTEGER PRIMARY KEY, zone INTEGER, node INTEGER,\
FOREIGN KEY (zone) REFERENCES zones(zindex), FOREIGN KEY (node) REFERENCES znodes(znindex));"

#define CMSQL_VMSESS_CREATE_TABLE_APPS "CREATE table apps(aindex INTEGER PRIMARY KEY, name TEXT, statemask INTEGER);"
#define CMSQL_VMSESS_CREATE_TABLE_APPPORTS "CREATE table appports(apppindex INTEGER PRIMARY KEY, portA INTEGER, portB INTEGER,\
protmask INTEGER, direction INTEGER, tcpall INTEGER, tcpnone INTEGER, tcpsyn INTEGER, tcpack INTEGER, tcpfin INTEGER, tcpreset INTEGER,\
tcppush INTEGER, tcpurgent INTEGER);"
#define CMSQL_VMSESS_CREATE_TABLE_ALINK "CREATE table alink(pindex INTEGER PRIMARY KEY, app INTEGER, port INTEGER,\
FOREIGN KEY (app) REFERENCES apps(aindex), FOREIGN KEY (port) REFERENCES appports(apppindex));"

#define CMSQL_VMSESS_CREATE_TABLE_FWMAIN "CREATE table fwmain(\
chainindex INTEGER PRIMARY KEY, head INTEGER, end INTEGER, fwindex INTEGER);"

/* Firewall: Chain - rules: */
#define CMSQL_VMSESS_CREATE_TABLE_FWFINPUT "CREATE table fwfinput(\
fwindex INTEGER PRIMARY KEY, pindex INTEGER, nindex INTEGER, szindex INTEGER, dzindex INTEGER, appindex INTEGER, rlimitpkt INTEGER,\
rlimitint INTEGER, action INTEGER, ttlA INTEGER, ttlB INTEGER, tosA INTEGER, tosB INTEGER, enabled INTEGER, ruletype INTEGER, rejectwith INTEGER,\
reverse INTEGER, logging INTEGER, logprefix TEXT, FOREIGN KEY (szindex) REFERENCES zones(zindex), FOREIGN KEY (dzindex) REFERENCES zones(zindex),\
FOREIGN KEY (appindex) REFERENCES apps(aindex));"

#define CMSQL_VMSESS_CREATE_TABLE_FWFOUTPUT "CREATE table fwfoutput(\
fwindex INTEGER PRIMARY KEY, pindex INTEGER, nindex INTEGER, szindex INTEGER, dzindex INTEGER, appindex INTEGER, rlimitpkt INTEGER,\
rlimitint INTEGER, action INTEGER, ttlA INTEGER, ttlB INTEGER, tosA INTEGER, tosB INTEGER, enabled INTEGER, ruletype INTEGER, rejectwith INTEGER,\
reverse INTEGER, logging INTEGER, logprefix TEXT, FOREIGN KEY (szindex) REFERENCES zones(zindex), FOREIGN KEY (dzindex) REFERENCES zones(zindex),\
FOREIGN KEY (appindex) REFERENCES apps(aindex));"

#define CMSQL_VMSESS_CREATE_TABLE_FWFFORWARD "CREATE table fwfforward(\
fwindex INTEGER PRIMARY KEY, pindex INTEGER, nindex INTEGER, szindex INTEGER, dzindex INTEGER, appindex INTEGER, rlimitpkt INTEGER,\
rlimitint INTEGER, action INTEGER, ttlA INTEGER, ttlB INTEGER, tosA INTEGER, tosB INTEGER, enabled INTEGER, ruletype INTEGER, rejectwith INTEGER,\
reverse INTEGER, logging INTEGER, logprefix TEXT, FOREIGN KEY (szindex) REFERENCES zones(zindex), FOREIGN KEY (dzindex) REFERENCES zones(zindex),\
FOREIGN KEY (appindex) REFERENCES apps(aindex));"

#define CMSQL_VMSESS_CREATE_TABLE_FWNPREROUTING "CREATE table fwnprerouting(\
fwindex INTEGER PRIMARY KEY, pindex INTEGER, nindex INTEGER, szindex INTEGER, dzindex INTEGER, nzindex INTEGER, appindex INTEGER, nappindex INTEGER,\
rlimitpkt INTEGER, rlimitint INTEGER, action INTEGER, ttlA INTEGER, ttlB INTEGER, tosA INTEGER, tosB INTEGER, enabled INTEGER, ruletype INTEGER,\
rejectwith INTEGER, reverse INTEGER, logging INTEGER, logprefix TEXT, FOREIGN KEY (szindex) REFERENCES zones(zindex),\
FOREIGN KEY (dzindex) REFERENCES zones(zindex), FOREIGN KEY (appindex) REFERENCES apps(aindex), FOREIGN KEY (nzindex) REFERENCES zones(zindex),\
FOREIGN KEY (nappindex) REFERENCES apps(aindex));"

#define CMSQL_VMSESS_CREATE_TABLE_FWNPOSTROUTING "CREATE table fwnpostrouting(\
fwindex INTEGER PRIMARY KEY, pindex INTEGER, nindex INTEGER, szindex INTEGER, dzindex INTEGER, nzindex INTEGER, appindex INTEGER, nappindex INTEGER,\
rlimitpkt INTEGER, rlimitint INTEGER, action INTEGER, ttlA INTEGER, ttlB INTEGER, tosA INTEGER, tosB INTEGER, enabled INTEGER, ruletype INTEGER,\
rejectwith INTEGER, reverse INTEGER, logging INTEGER, logprefix TEXT, FOREIGN KEY (szindex) REFERENCES zones(zindex),\
FOREIGN KEY (dzindex) REFERENCES zones(zindex), FOREIGN KEY (appindex) REFERENCES apps(aindex), FOREIGN KEY (nzindex) REFERENCES zones(zindex),\
FOREIGN KEY (nappindex) REFERENCES apps(aindex));"

#define CMSQL_VMSESS_CREATE_TABLE_FWNOUTPUT "CREATE table fwnoutput(\
fwindex INTEGER PRIMARY KEY, pindex INTEGER, nindex INTEGER, szindex INTEGER, dzindex INTEGER, nzindex INTEGER, appindex INTEGER, nappindex INTEGER,\
rlimitpkt INTEGER, rlimitint INTEGER, action INTEGER, ttlA INTEGER, ttlB INTEGER, tosA INTEGER, tosB INTEGER, enabled INTEGER, ruletype INTEGER,\
rejectwith INTEGER, reverse INTEGER, logging INTEGER, logprefix TEXT, FOREIGN KEY (szindex) REFERENCES zones(zindex),\
FOREIGN KEY (dzindex) REFERENCES zones(zindex), FOREIGN KEY (appindex) REFERENCES apps(aindex), FOREIGN KEY (nzindex) REFERENCES zones(zindex),\
FOREIGN KEY (nappindex) REFERENCES apps(aindex));"

#define CMSQL_VMSESS_CREATE_TABLE_FWMINPUT "CREATE table fwminput(\
fwindex INTEGER PRIMARY KEY, pindex INTEGER, nindex INTEGER, szindex INTEGER, dzindex INTEGER, appindex INTEGER, pktmark INTEGER, rlimitpkt INTEGER,\
rlimitint INTEGER, action INTEGER, ttlA INTEGER, ttlB INTEGER, tosA INTEGER, tosB INTEGER, enabled INTEGER, ruletype INTEGER, rejectwith INTEGER,\
reverse INTEGER, logging INTEGER, logprefix TEXT, FOREIGN KEY (szindex) REFERENCES zones(zindex), FOREIGN KEY (dzindex) REFERENCES zones(zindex),\
FOREIGN KEY (appindex) REFERENCES apps(aindex));"

#define CMSQL_VMSESS_CREATE_TABLE_FWMOUTPUT "CREATE table fwmoutput(\
fwindex INTEGER PRIMARY KEY, pindex INTEGER, nindex INTEGER, szindex INTEGER, dzindex INTEGER, appindex INTEGER, pktmark INTEGER, rlimitpkt INTEGER,\
rlimitint INTEGER, action INTEGER, ttlA INTEGER, ttlB INTEGER, tosA INTEGER, tosB INTEGER, enabled INTEGER, ruletype INTEGER, rejectwith INTEGER,\
reverse INTEGER, logging INTEGER, logprefix TEXT, FOREIGN KEY (szindex) REFERENCES zones(zindex), FOREIGN KEY (dzindex) REFERENCES zones(zindex),\
FOREIGN KEY (appindex) REFERENCES apps(aindex));"

#define CMSQL_VMSESS_CREATE_TABLE_FWMFORWARD "CREATE table fwmforward(\
fwindex INTEGER PRIMARY KEY, pindex INTEGER, nindex INTEGER, szindex INTEGER, dzindex INTEGER, appindex INTEGER, pktmark INTEGER, rlimitpkt INTEGER,\
rlimitint INTEGER, action INTEGER, ttlA INTEGER, ttlB INTEGER, tosA INTEGER, tosB INTEGER, enabled INTEGER, ruletype INTEGER, rejectwith INTEGER,\
reverse INTEGER, logging INTEGER, logprefix TEXT, FOREIGN KEY (szindex) REFERENCES zones(zindex), FOREIGN KEY (dzindex) REFERENCES zones(zindex),\
FOREIGN KEY (appindex) REFERENCES apps(aindex));"

#define CMSQL_VMSESS_CREATE_TABLE_FWMPREROUTING "CREATE table fwmprerouting(\
fwindex INTEGER PRIMARY KEY, pindex INTEGER, nindex INTEGER, szindex INTEGER, dzindex INTEGER, appindex INTEGER, pktmark INTEGER, rlimitpkt INTEGER,\
rlimitint INTEGER, action INTEGER, ttlA INTEGER, ttlB INTEGER, tosA INTEGER, tosB INTEGER, enabled INTEGER, ruletype INTEGER, rejectwith INTEGER,\
reverse INTEGER, logging INTEGER, logprefix TEXT, FOREIGN KEY (szindex) REFERENCES zones(zindex), FOREIGN KEY (dzindex) REFERENCES zones(zindex),\
FOREIGN KEY (appindex) REFERENCES apps(aindex));"

#define CMSQL_VMSESS_CREATE_TABLE_FWMPOSTROUTING "CREATE table fwmpostrouting(\
fwindex INTEGER PRIMARY KEY, pindex INTEGER, nindex INTEGER, szindex INTEGER, dzindex INTEGER, appindex INTEGER, pktmark INTEGER, rlimitpkt INTEGER,\
rlimitint INTEGER, action INTEGER, ttlA INTEGER, ttlB INTEGER, tosA INTEGER, tosB INTEGER, enabled INTEGER, ruletype INTEGER, rejectwith INTEGER,\
reverse INTEGER, logging INTEGER, logprefix TEXT, FOREIGN KEY (szindex) REFERENCES zones(zindex), FOREIGN KEY (dzindex) REFERENCES zones(zindex),\
FOREIGN KEY (appindex) REFERENCES apps(aindex));"

/* Virtual Network Services */
#define CMSQL_VMSESS_CREATE_TABLE_VSP "CREATE table vsp(\
vspindex INTEGER PRIMARY KEY, ratein INTEGER, rateout INTEGER, vsport INTEGER, vmsport INTEGER, vmport INTEGER, timein INTEGER, timeout INTEGER,\
sproto INTEGER, enabled INTEGER, FOREIGN KEY (vmport) REFERENCES vmport(vmpindex));"

#define CMSQL_VMSESS_CREATE_TABLE_VSN "CREATE table vsn(\
vsindex INTEGER PRIMARY KEY, vsip INTEGER, vsmsk INTEGER, enabled INTEGER, mstate INTEGER, netifidx INTEGER);"

#define CMSQL_VMSESS_CREATE_TABLE_VSLINK "CREATE table vslink(\
vslindex INTEGER PRIMARY KEY, vsnindex INTEGER, vspindex INTEGER, FOREIGN KEY (vsnindex) REFERENCES vsn(vsindex),\
FOREIGN KEY (vspindex) REFERENCES vsp(vspindex));"

/* inserts */
#define CMSQL_VMSESS_INSERT_CMINFO "INSERT INTO cminfo VALUES(1,1);"


/* queries */
#define CMSQL_VMSESS_SELECT_CMINFO "SELECT * from cminfo WHERE status=1 LIMIT 1;"
#define CMSQL_VMSESS_SELECT_VMSESS "SELECT * from vmsess;"
#define CMSQL_VMSESS_SELECT_VBRIDGE "SELECT * from vbridge;"
#define CMSQL_VMSESS_SELECT_VMPORTS "SELECT * from vmport;"
#define CMSQL_VMSESS_SELECT_FW_MAIN "SELECT * from fwmain;"
#define CMSQL_VMSESS_SELECT_FW_Z1 "SELECT * from zlink;"
#define CMSQL_VMSESS_SELECT_FW_Z2 "SELECT * from zones;"
#define CMSQL_VMSESS_SELECT_FW_Z3 "SELECT * from znodes;"
#define CMSQL_VMSESS_SELECT_FW_A1 "SELECT * from alink;"
#define CMSQL_VMSESS_SELECT_FW_A2 "SELECT * from appports;"
#define CMSQL_VMSESS_SELECT_FW_A3 "SELECT * from apps;"
#define CMSQL_VMSESS_SELECT_FW_F1 "SELECT * from fwfinput;"
#define CMSQL_VMSESS_SELECT_FW_F2 "SELECT * from fwfoutput;"
#define CMSQL_VMSESS_SELECT_FW_F3 "SELECT * from fwfforward;"
#define CMSQL_VMSESS_SELECT_FW_F4 "SELECT * from fwnpostrouting;"
#define CMSQL_VMSESS_SELECT_FW_F5 "SELECT * from fwnprerouting;"
#define CMSQL_VMSESS_SELECT_FW_F6 "SELECT * from fwnoutput;"
#define CMSQL_VMSESS_SELECT_FW_F7 "SELECT * from fwminput;"
#define CMSQL_VMSESS_SELECT_FW_F8 "SELECT * from fwmoutput;"
#define CMSQL_VMSESS_SELECT_FW_F9 "SELECT * from fwmforward;"
#define CMSQL_VMSESS_SELECT_FW_F10 "SELECT * from fwmprerouting;"
#define CMSQL_VMSESS_SELECT_FW_F11 "SELECT * from fwmpostrouting;"
#define CMSQL_VMSESS_SELECT_VNS_VSN "SELECT * from vsn;"
#define CMSQL_VMSESS_SELECT_VNS_VSLINK "SELECT * from vslink;"

/* drop */
#define CMSQL_VMSESS_DROP_FWMAIN "DROP TABLE fwmain;"
#define CMSQL_VMSESS_DROP_Z1 "DROP TABLE zlink;"
#define CMSQL_VMSESS_DROP_Z2 "DROP TABLE zones;"
#define CMSQL_VMSESS_DROP_Z3 "DROP TABLE znodes;"
#define CMSQL_VMSESS_DROP_A1 "DROP TABLE alink;"
#define CMSQL_VMSESS_DROP_A2 "DROP TABLE appports;"
#define CMSQL_VMSESS_DROP_A3 "DROP TABLE apps;"
#define CMSQL_VMSESS_DROP_F1 "DROP TABLE fwfinput;"
#define CMSQL_VMSESS_DROP_F2 "DROP TABLE fwfoutput;"
#define CMSQL_VMSESS_DROP_F3 "DROP TABLE fwfforward;"
#define CMSQL_VMSESS_DROP_F4 "DROP TABLE fwnpostrouting;"
#define CMSQL_VMSESS_DROP_F5 "DROP TABLE fwnprerouting;"
#define CMSQL_VMSESS_DROP_F6 "DROP TABLE fwnoutput;"
#define CMSQL_VMSESS_DROP_F7 "DROP TABLE fwminput;"
#define CMSQL_VMSESS_DROP_F8 "DROP TABLE fwmoutput;"
#define CMSQL_VMSESS_DROP_F9 "DROP TABLE fwmforward;"
#define CMSQL_VMSESS_DROP_F10 "DROP TABLE fwmprerouting;"
#define CMSQL_VMSESS_DROP_F11 "DROP TABLE fwmpostrouting;"
#define CMSQL_VMSESS_DROP_VSN "DROP TABLE vsn;"
#define CMSQL_VMSESS_DROP_VSP "DROP TABLE vsp;"
#define CMSQL_VMSESS_DROP_VSLINK "DROP TABLE vslink;"
