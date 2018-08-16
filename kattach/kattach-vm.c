/*
 * kattach (kernel attach)
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
 * Source File :                kattach-vm.c
 * Purpose     :                launch and deployment control of vms
 * Callers     :                kattach.c - main() and kattach-loop.c
 *
 */

#include <ctype.h>
#include <sys/mount.h>
#include <linux/fs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include "sqlite3.h"
#include "kattach_types.h"
#include "kattach.h"
#include "kaosdb.h"
#include "kattach_shm.h"

/* prototypes */
static int kattach_vm_cb_vmsess(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_vbridge(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_vmports(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_vmimage(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_vmapps(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_cfggrp(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_appmodule(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_empty(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_fwmain(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_fw_znodes(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_fw_zones(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_fw_zlink(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_fw_apps(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_fw_appports(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_fw_alink(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_fw_filter_input(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_fw_filter_output(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_fw_filter_forward(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_fw_nat_prerouting(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_fw_nat_postrouting(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_fw_nat_output(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_fw_mangle_input(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_fw_mangle_output(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_fw_mangle_forward(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_fw_mangle_prerouting(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_fw_mangle_postrouting(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_vns_vsn(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_vns_vsp(void *NotUsed, int cols, char **results, char **colname);
static int kattach_vm_cb_vns_vslink(void *NotUsed, int cols, char **results, char **colname);

void
kattach_vm_launch(void)
{
	u8 rc = 0;

	/* query vmsession table and populate in-memory session table */
	rc = kattach_vm_sql_vmsession(CMSQL_VMSESS_SELECT_VMSESS, K_DB_VMSESSION);

	/* query virtual bridge table and populate in-memory structures */
	rc = kattach_vm_sql_vbridge(CMSQL_VMSESS_SELECT_VBRIDGE, K_DB_VMSESSION);

	/* query virtual port table and populate in-memory structures */
	rc = kattach_vm_sql_vmports(CMSQL_VMSESS_SELECT_VMPORTS, K_DB_VMSESSION);

	/* query vmimage table, this is necessary to get name for the disk filename from the table */
	rc = kattach_vm_sql_vmimage(CMSQL_APPQ_SELECT_VMIMAGE, K_DB_APPQ);

	/* query appmodule table, this has the info for the app modules */
	rc = kattach_vm_sql_appmodule(CMSQL_APPQ_SELECT_APPMODULE, K_DB_APPQ);

	/* query vmapps table, this joins the app table and the vmimages together */
	rc = kattach_vm_sql_vmapps(CMSQL_APPQ_SELECT_VMAPPS, K_DB_APPQ);

	/* query cfggrp table */
	rc = kattach_vm_sql_cfggrp(CMSQL_APPQ_SELECT_CFGGRP, K_DB_APPQ);

	/* query fw tables */
	kattach_vm_db_fw();

	/* build VLAN and virtual networking */
	kattach_vm_build_vlans();

	/* generate and launch ldapd */
	kattach_vm_ldapd();

	/* start VMs */
	kattach_vm_launch_all();

	/* process fw and vns rules */
	kattach_vm_apply_fw();

	/* launch app mgr processes */
	kattach_vm_launch_mgr();

	return;
}

void
kattach_vm_init(void)
{
	kattach_vmst_t *sPtr = &kattach_vmst;
	kattach_vmp_t *pPtr = &kattach_vmports;
	kattach_vbr_t *bPtr = &kattach_vbridge;
	kattach_vmi_t *iPtr = &kattach_vmimages;
	kattach_am_t *aPtr = &kattach_appmods;

	memset(sPtr,0,sizeof(kattach_vmst));
	memset(pPtr,0,sizeof(kattach_vmports));
	memset(bPtr,0,sizeof(kattach_vbridge));
	memset(iPtr,0,sizeof(kattach_vmimages));
	memset(aPtr,0,sizeof(kattach_appmods));

	return;
}

u8
kattach_vm_sql_vmsession(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_vmsess, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_vmsess, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_vmsess, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] VM Session Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_vbridge(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_vbridge, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_vbridge, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_vbridge, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] Virtual Bridge Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_vmports(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_vmports, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_vmports, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_vmports, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] Virtual Port Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_vmimage(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_vmimage, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_vmimage, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_vmimage, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] VM Image Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_vmapps(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_vmapps, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_vmapps, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_vmapps, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] VM Apps Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_cfggrp(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_cfggrp, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_cfggrp, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_cfggrp, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] Config Group Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_appmodule(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_appmodule, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_appmodule, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_appmodule, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] App Module Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

static int 
kattach_vm_cb_vmsess(void *NotUsed, int cols, char **results, char **colname)
{
        int i = 0;
	u32 index = kattach_vmst.index;
	char qres[512];

	kattach_vmst.vmsess[index].vpid = 0;

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("vmindex",colname[i],strlen(colname[i]))) {
			kattach_vmst.index++;
			continue;
		} else if (!strncmp("vmstatus",colname[i],strlen(colname[i]))) {
			u8 vmstatus = (u8) atoi(results[i]);
			if (vmstatus > KATTACH_VM_STATUS_END) {
				kattach_vmst.vmsess[index].vmstatus = KATTACH_VM_STATUS_UNKNOWN;
			} else {
				kattach_vmst.vmsess[index].vmstatus = vmstatus;
			}
			kattach_vmst.vmsess[index].vmoper = kattach_vmst.vmsess[index].vmstatus;
			continue;
		} else if (!strncmp("vmimage",colname[i],strlen(colname[i]))) {
			kattach_vmst.vmsess[index].vmimage = (u32) (atol(results[i]) - 1);
			continue;
		} else if (!strncmp("vmboot",colname[i],strlen(colname[i]))) {
			continue;	/* skip for now */
		} else if (!strncmp("vmname",colname[i],strlen(colname[i]))) {
			sprintf(kattach_vmst.vmsess[index].vmname,"%s",results[i]);
			continue;
		} else if (!strncmp("vmem",colname[i],strlen(colname[i]))) {
			kattach_vmst.vmsess[index].vmem = (u8) atoi(results[i]);
			continue;
		} else if (!strncmp("vcpu",colname[i],strlen(colname[i]))) {
			kattach_vmst.vmsess[index].vcpu = (u8) atoi(results[i]);
			continue;		
		} else if (!strncmp("vmport",colname[i],strlen(colname[i]))) {
			kattach_vmst.vmsess[index].vmport[0] = (u16) (atoi(results[i]) - 1);
			kattach_vmst.vmsess[index].vmpidx = 1;
			continue;
		} else if (!strncmp("priority",colname[i],strlen(colname[i]))) {
			kattach_vmst.vmsess[index].priority = (u8) atoi(results[i]);
			continue;				
		}
        }
        return 0;

}

static int 
kattach_vm_cb_vbridge(void *NotUsed, int cols, char **results, char **colname)
{
        int i = 0;
	u16 index = kattach_vbridge.index;
	char qres[512];

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("vbrindex",colname[i],strlen(colname[i]))) {
			/* new entry */
			kattach_vbridge.index++;
			continue;
		} else if (!strncmp("vlan",colname[i],strlen(colname[i]))) {
			kattach_vbridge.vbridge[index].vlan = (u16) atoi(results[i]);
			continue;
		} else if (!strncmp("vsubnet",colname[i],strlen(colname[i]))) {
			kattach_vbridge.vbridge[index].vsubnet = (u32) atol(results[i]);
			continue;
		} else if (!strncmp("vmask",colname[i],strlen(colname[i]))) {
			kattach_vbridge.vbridge[index].vmask = (u16) atoi(results[i]);
			continue;
		} else if (!strncmp("vbrip",colname[i],strlen(colname[i]))) {
			kattach_vbridge.vbridge[index].vbrip = (u32) atol(results[i]);
			continue;	/* skip for now */
		} else if (!strncmp("vbrlocal",colname[i],strlen(colname[i]))) {
			kattach_vbridge.vbridge[index].vbrlocal = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("vlanext",colname[i],strlen(colname[i]))) {
			if (strlen(results[i]) == 0) {
				kattach_vbridge.vbridge[index].vlanext[0] = '\0';
			} else {
				sprintf(kattach_vbridge.vbridge[index].vlanext,"%s",results[i]);
			}
			continue;
		} else if (!strncmp("vpfree",colname[i],strlen(colname[i]))) {
			kattach_vbridge.vbridge[index].vpfree = (u16) atoi(results[i]);
			continue;
		} else if (!strncmp("vbruse",colname[i],strlen(colname[i]))) {
			kattach_vbridge.vbridge[index].vbruse = (u16) atoi(results[i]);
			continue;
		}
        }

        return 0;

}

static int 
kattach_vm_cb_vmports(void *NotUsed, int cols, char **results, char **colname)
{
        int i = 0;
	u16 index = kattach_vmports.index;
	u8 vmidx = 0, vmpidx = 0;
	char qres[512];

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("vmpindex",colname[i],strlen(colname[i]))) {
			/* new entry */
			kattach_vmports.index++;
			continue;
		} else if (!strncmp("vmacA",colname[i],strlen(colname[i]))) {
			kattach_vmports.vmports[index].vmac[0] = (u8) atoi(results[i]);
			continue;
		} else if (!strncmp("vmacB",colname[i],strlen(colname[i]))) {
			kattach_vmports.vmports[index].vmac[1] = (u8) atoi(results[i]);
			continue;
		} else if (!strncmp("vmacC",colname[i],strlen(colname[i]))) {
			kattach_vmports.vmports[index].vmac[2] = (u8) atoi(results[i]);
			continue;
		} else if (!strncmp("vmacD",colname[i],strlen(colname[i]))) {
			kattach_vmports.vmports[index].vmac[3] = (u8) atoi(results[i]);
			continue;
		} else if (!strncmp("vmacE",colname[i],strlen(colname[i]))) {
			kattach_vmports.vmports[index].vmac[4] = (u8) atoi(results[i]);
			continue;
		} else if (!strncmp("vmacF",colname[i],strlen(colname[i]))) {
			kattach_vmports.vmports[index].vmac[5] = (u8) atoi(results[i]);
			continue;
		} else if (!strncmp("vmowner",colname[i],strlen(colname[i]))) {
			kattach_vmports.vmports[index].vmst = (u32) (atol(results[i]) - 1);
			vmidx = kattach_vmports.vmports[index].vmst;
			if (vmidx > kattach_vmst.index) continue;
			if (kattach_vmst.vmsess[vmidx].vmstatus == KATTACH_VM_STATUS_DELETED) continue;
			if (index == kattach_vmst.vmsess[vmidx].vmport[0]) {
				continue;
			} else {		/* this is a secondary virtual port */
				vmpidx = kattach_vmst.vmsess[vmidx].vmpidx;
				kattach_vmst.vmsess[vmidx].vmport[vmpidx] = index;
				kattach_vmst.vmsess[vmidx].vmpidx++;
				continue;
			}
		} else if (!strncmp("vbridge",colname[i],strlen(colname[i]))) {
			kattach_vmports.vmports[index].vbridge = (u16) (atoi(results[i]) - 1);
			continue;
		} else if (!strncmp("vmpip",colname[i],strlen(colname[i]))) {
                        kattach_vmports.vmports[index].vmpip = (u32) (atol(results[i]));
                        continue;
                }
        }

        return 0;
}

static int
kattach_vm_cb_vmimage(void *NotUsed, int cols, char **results, char **colname)
{
	int i = 0;
	u32 index = kattach_vmimages.index;
	char qres[512];

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("vmindex",colname[i],strlen(colname[i]))) {
                        /* new entry */
                        kattach_vmimages.index++;
                        continue;
		} else if (!strncmp("active",colname[i],strlen(colname[i]))) {
			kattach_vmimages.vmimage[index].active = (u8) atoi(results[i]);
			continue;
		} else if (!strncmp("vminame",colname[i],strlen(colname[i]))) {
                        sprintf(kattach_vmimages.vmimage[index].vminame,"%s",results[i]);
			continue;
		}
	}
        return 0;
}

static int
kattach_vm_cb_vmapps(void *NotUsed, int cols, char **results, char **colname)
{
	int i = 0;
	u32 appidx = 0, vmidx = 0, cfggidx = 0;
	u8 appi = 0;
	char qres[512];

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("vmappidx",colname[i],strlen(colname[i]))) {
                        /* new entry */
                        continue;
		} else if (!strncmp("appindex",colname[i],strlen(colname[i]))) {
			appidx = (u32) (atol(results[i]));
			kattach_appmods.appmodules[appidx].config++;
			continue;
		} else if (!strncmp("vmindex",colname[i],strlen(colname[i]))) {
			vmidx = (u32) ((atol(results[i])) - 1);
			appi = kattach_vmimages.vmimage[vmidx].appi;
			kattach_vmimages.vmimage[vmidx].appindex[appi] = (appidx - 1);
			kattach_vmimages.vmimage[vmidx].appi++;
			continue;
		} else if (!strncmp("cfgindex",colname[i],strlen(colname[i]))) {
			cfggidx = (u32) (atol(results[i]));
			kattach_vmimages.vmimage[vmidx].cfggrp[appi] = (cfggidx - 1);
			continue;
		}
	}
        return 0;
}

static int
kattach_vm_cb_cfggrp(void *NotUsed, int cols, char **results, char **colname)
{
	int i = 0;
	u32 index = kattach_cfggrp.index;
	char qres[512];

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("cfggidx",colname[i],strlen(colname[i]))) {
                        /* new entry */
			kattach_cfggrp.index++;
                        continue;
		} else if (!strncmp("name",colname[i],strlen(colname[i]))) {
			if (strlen(results[i])) {
				sprintf(kattach_cfggrp.cfggrp[index].name,"%s",results[i]);
			} else {
				kattach_cfggrp.cfggrp[index].name[0] = '\0';
			}
			continue;
		} else if (!strncmp("appindex",colname[i],strlen(colname[i]))) {
			kattach_cfggrp.cfggrp[index].appmidx = (u32) ((atol(results[i])) - 1);
			continue;
		}
	}
        return 0;
}

static int
kattach_vm_cb_appmodule(void *NotUsed, int cols, char **results, char **colname)
{
	int i = 0, u = 0;
	u32 index = kattach_appmods.index;
	char sqlq[1024];
	char qres[512];
	u8 rc = 0;

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("appindex",colname[i],strlen(colname[i]))) {
                        /* new entry */
			kattach_appmods.index++;
			u = 1;
                        continue;
		} else if (!strncmp("name",colname[i],strlen(colname[i]))) {
			sprintf(kattach_appmods.appmodules[index].name,"%s",results[i]);
			continue;
		} else if (!strncmp("filename",colname[i],strlen(colname[i]))) {
			sprintf(kattach_appmods.appmodules[index].filename,"%s",results[i]);
			continue;
		} else if (!strncmp("vurl",colname[i],strlen(colname[i]))) {
			sprintf(kattach_appmods.appmodules[index].url,"%s",results[i]);
			continue;
		} else if (!strncmp("version",colname[i],strlen(colname[i]))) {
			sprintf(kattach_appmods.appmodules[index].version,"%s",results[i]);
			continue;
		} else if (!strncmp("release",colname[i],strlen(colname[i]))) {
			sprintf(kattach_appmods.appmodules[index].release,"%s",results[i]);
			continue;
		} else if (!strncmp("buildinfo",colname[i],strlen(colname[i]))) {
			sprintf(kattach_appmods.appmodules[index].buildinfo,"%s",results[i]);
			continue;
		} else if (!strncmp("chksum",colname[i],strlen(colname[i]))) {
			sprintf(kattach_appmods.appmodules[index].chksum,"%s",results[i]);
			continue;
		} else if (!strncmp("vendor",colname[i],strlen(colname[i]))) {
			kattach_appmods.appmodules[index].vendor_id  = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("revision",colname[i],strlen(colname[i]))) {
			kattach_appmods.appmodules[index].revision  = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("srctree",colname[i],strlen(colname[i]))) {
			kattach_appmods.appmodules[index].srctree  = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("license",colname[i],strlen(colname[i]))) {
			kattach_appmods.appmodules[index].license  = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("latest",colname[i],strlen(colname[i]))) {
			kattach_appmods.appmodules[index].latest  = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("appsize",colname[i],strlen(colname[i]))) {
			/* appsize was added by db version 2 */
			if (results[i] == NULL) {
				kattach_appmods.appmodules[index].app_size = 31;
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE appmodule SET appsize = %u WHERE appindex = '%lu';",31,(1 + index));
				rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);
			} else {
				kattach_appmods.appmodules[index].app_size  = (u8) (atoi(results[i]));
			}
			u = 0;
			continue;
		}
	}
	if (u == 1) {
		/* appsize not hit */
		kattach_appmods.appmodules[index].app_size = 31;
		memset(sqlq,0,sizeof(sqlq));
		sprintf(sqlq,"UPDATE appmodule SET appsize = %u WHERE appindex = '%lu';",31,(1 + index));
		rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);
	}
        return 0;
}

void
kattach_vm_build_vlans(void)
{
	char lcmd[255];
	u16 index = 0;

	for (index = 0; index < kattach_vbridge.index; index++) {
		if (index >= KATTACH_MAX_VBRIDGES) break;
		if (kattach_vbridge.vbridge[index].vlan == 0) continue;
		if ((kattach_vbridge.vbridge[index].state == KATTACH_VBR_STATE_DELETED) ||
			(kattach_vbridge.vbridge[index].state == KATTACH_VBR_STATE_DISABLED)) 
			continue;
		kattach_vm_create_bridge(kattach_vbridge.vbridge[index].vlan);
		kattach_vm_bridge_up(index);
		if ((kattach_vbridge.vbridge[index].vbrlocal == KATTACH_NET_VLAN_8021Q) ||
			(kattach_vbridge.vbridge[index].vbrlocal == KATTACH_NET_VLAN_ROUTED)) {
			/* vconfig add eth0 254 */
			sprintf(lcmd,"%s%s add %s %u",KATTACH_SBINPATH,KCMD_VCONFIG,kattach_vbridge.vbridge[index].vlanext,kattach_vbridge.vbridge[index].vlan);
			kattach_sysexec(lcmd);

			/* ip link set eth0.254 up */
			sprintf(lcmd,"%s%s link set %s.%u up",KATTACH_BINPATH,KCMD_IP,kattach_vbridge.vbridge[index].vlanext,
								kattach_vbridge.vbridge[index].vlan);
			kattach_sysexec(lcmd);

			/* brctl addif vbr254 eth0.254 */
			sprintf(lcmd,"%s%s addif vbr%u %s.%u",KATTACH_SBINPATH,KCMD_BRCTL,kattach_vbridge.vbridge[index].vlan,
								kattach_vbridge.vbridge[index].vlanext, kattach_vbridge.vbridge[index].vlan);
			kattach_sysexec(lcmd);
		} else if ((kattach_vbridge.vbridge[index].vbrlocal == KATTACH_NET_VLAN_LOCAL) ||
				(kattach_vbridge.vbridge[index].vbrlocal == KATTACH_NET_VLAN_NAT)) {
			/* vconfig add vbr254 254 */
			sprintf(lcmd,"%s%s add vbr%u %u",KATTACH_SBINPATH,KCMD_VCONFIG,kattach_vbridge.vbridge[index].vlan,kattach_vbridge.vbridge[index].vlan);
			kattach_sysexec(lcmd);

			/* ip link set vbr254.254 up */
			sprintf(lcmd,"%s%s link set vbr%u.%u up",KATTACH_BINPATH,KCMD_IP, kattach_vbridge.vbridge[index].vlan,kattach_vbridge.vbridge[index].vlan);
			kattach_sysexec(lcmd);

			/* brctl addif vbr254 vbr254.254 */
			sprintf(lcmd,"%s%s addif vbr%u vbr%u.%u",KATTACH_SBINPATH,KCMD_BRCTL,kattach_vbridge.vbridge[index].vlan,
									kattach_vbridge.vbridge[index].vlan, kattach_vbridge.vbridge[index].vlan);
			kattach_sysexec(lcmd);
		}
	}
	return;
}

void
kattach_vm_bridge_addif(u16 vmport, u16 vlan)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s addif vbr%u vmtap%u",KATTACH_SBINPATH,KCMD_BRCTL,vlan,vmport);
	kattach_sysexec(lcmd);
	return;

}

void
kattach_vm_bridge_delif(u16 vmport, u16 vlan)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s delif vbr%u vmtap%u",KATTACH_SBINPATH,KCMD_BRCTL,vlan,vmport);
	kattach_sysexec(lcmd);
	return;

}

void
kattach_vm_bridge_up(u16 index)
{
	char lcmd[255];
	int i = 0;
	u8 vmacX = 0;								/* reserved value for virtual bridges */
	u8 vmacY = 0;
	u8 vmacZ = 0;

	kattach_vbridge.vbridge[index].state = KATTACH_VBR_STATE_ACTIVE;

	if ((kattach_vbridge.vbridge[index].bmac[3] == 0) &&
		(kattach_vbridge.vbridge[index].bmac[4] == 0) &&
		(kattach_vbridge.vbridge[index].bmac[5] == 0)) {
		kattach_net_genmac();							/* generate a random mac address */

		for (i = 0; i <= 5; i++) {
			kattach_vbridge.vbridge[index].bmac[i] = kattach_genmac[i];
		}
	}

	vmacX = kattach_vbridge.vbridge[index].bmac[3];
	vmacY = kattach_vbridge.vbridge[index].bmac[4];
	vmacZ = kattach_vbridge.vbridge[index].bmac[5];

	sprintf(lcmd,"%s%s link set addr %s:%02x:%02x:%02x dev vbr%u",KATTACH_BINPATH,KCMD_IP,KATTACH_NET_MACPREFIX,vmacX,vmacY,vmacZ,kattach_vbridge.vbridge[index].vlan);
	kattach_sysexec(lcmd);

	/* ip addr add 192.168.254.1/24 dev vbr254 (example: vlan254) */

	if ((kattach_vbridge.vbridge[index].vbrlocal != KATTACH_NET_VLAN_ROUTED) &&
		(kattach_vbridge.vbridge[index].vbrlocal != KATTACH_NET_VLAN_NAT)) {
		sprintf(lcmd,"%s%s addr add %s/%u dev vbr%u",KATTACH_BINPATH,KCMD_IP,kattach_net_parseip(kattach_vbridge.vbridge[index].vbrip),
							kattach_vbridge.vbridge[index].vmask, kattach_vbridge.vbridge[index].vlan);
		kattach_sysexec(lcmd);
	}

	/* ip link set vbr254 up (example: vlan 254) */
	sprintf(lcmd,"%s%s link set vbr%u up",KATTACH_BINPATH,KCMD_IP,kattach_vbridge.vbridge[index].vlan);
	kattach_sysexec(lcmd);
	return;
}

void
kattach_vm_bridge_down(u16 index)
{
	char lcmd[255];

	kattach_vbridge.vbridge[index].state = KATTACH_VBR_STATE_INACTIVE;

	if ((kattach_vbridge.vbridge[index].vbrlocal != KATTACH_NET_VLAN_ROUTED) &&
		(kattach_vbridge.vbridge[index].vbrlocal != KATTACH_NET_VLAN_NAT)) {
		sprintf(lcmd,"%s%s addr del %s/%u dev vbr%u",KATTACH_BINPATH,KCMD_IP,kattach_net_parseip(kattach_vbridge.vbridge[index].vbrip),
							kattach_vbridge.vbridge[index].vmask, kattach_vbridge.vbridge[index].vlan);
		kattach_sysexec(lcmd);
	}

	/* ip link set vbr254 up (example: vlan 254) */
	sprintf(lcmd,"%s%s link set vbr%u down",KATTACH_BINPATH,KCMD_IP,kattach_vbridge.vbridge[index].vlan);
	kattach_sysexec(lcmd);
	return;
}

void
kattach_vm_create_bridge(u16 vlan)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s addbr vbr%u",KATTACH_SBINPATH,KCMD_BRCTL,vlan);
	kattach_sysexec(lcmd);
	return;
}

void
kattach_vm_delete_bridge(u16 vlan)
{
	char lcmd[255];

	sprintf(lcmd,"%s%s delbr vbr%u",KATTACH_SBINPATH,KCMD_BRCTL,vlan);
	kattach_sysexec(lcmd);
	return;
}

void
kattach_vm_launch_all(void)
{
	u32 index = 0, netx = 0, aindex = 0, vmidx = 0;
	u32 vmimage = 0;
	u16 vmport = 0, vmbridge = 0;
	u8 vmpidx = 0, dhcpdlaunch = 0, appidx = 0, kwaitcnt = 0;
	int rc = 0, xstat = 0;
	char ccmd[64];
	char lcmd[2048];
	char netcmd[64];
	char ecmd[256];
	struct stat k_procstatus;
	

	/* now launch each of the VMs */
	/* /kaos/hv/kvm/bin/qemu-system-x86_64 -enable-kvm -boot c -net nic,vlan=VLAN,macaddr=MAC,model=virtio */
	/*    -net tap,ifname=vmtapX,script=no (repeat if in multiple vlans) */
	/*    -drive file=/path/to/disk,if=virtio,boot=on */
	/*    -m XXX -smp N -vnc 0.0.0.0:X -kernel -append  kaos=9:vda1:MAC:ip:mask:gw:dns */

	/* search through vmst */
	/* for each vmst entry, search vmports to see if more than 1 entry mapped to vmst (multiple VLAN support) */
	/* the index for the vmports entry is the ID for vmtapX */
	/* note: the virtual disk image mounted as /dev/vda1 contains the /kaos/apps path for vKaOS */
	/* the disk image itself is generated from vdisk-[vmimage_name].vdi */

	for (index = 0; index < kattach_vmst.index; index++) {
		if ((kattach_vmst.vmsess[index].vmstatus == KATTACH_VM_STATUS_NEW) && (kattach_vmst.vmsess[index].vmoper != KATTACH_VM_STATUS_OP_DEPLOY)) continue;
		if ((kattach_vmst.vmsess[index].vmstatus == KATTACH_VM_STATUS_DISABLED) || (kattach_vmst.vmsess[index].vmstatus == KATTACH_VM_STATUS_DELETED)) continue;
		if ((dhcpdlaunch == 0) && (kattach_cfg.pid_dhcpd == 0)) {
			/* generate dhcpd.conf and launch dhcpd */
			kattach_vm_dhcpd();
			dhcpdlaunch++;
		}
		if (kattach_vmst.vmsess[index].vcpu == 0) {
			kattach_vmst.vmsess[index].vcpu = 2;			/* FIXME: this is temporary */
		}
		if (kattach_vmst.vmsess[index].vmem == 0) {
			kattach_vmst.vmsess[index].vmem = 512;			/* FIXME: this is temporary */
		}
		sprintf(ccmd,"%s/%s",KATTACH_HVPATH,KCMD_QEMU);
		sprintf(lcmd,"-enable-kvm -boot c -m %d -smp %d",kattach_vmst.vmsess[index].vmem,
						kattach_vmst.vmsess[index].vcpu);
		if (kattach_vmst.vmsess[index].vmpidx > KATTACH_MAX_VPORTS) {
			vmpidx = KATTACH_MAX_VPORTS;
		} else {
			vmpidx = kattach_vmst.vmsess[index].vmpidx;
		}
		for (netx = 0; netx < vmpidx; netx++) {
			vmport = kattach_vmst.vmsess[index].vmport[netx];
			vmbridge = kattach_vmports.vmports[vmport].vbridge;
			sprintf(netcmd," -net nic,vlan=%u,macaddr=%02x:%02x:%02x:%02x:%02x:%02x,model=virtio",
					kattach_vbridge.vbridge[vmbridge].vlan, kattach_vmports.vmports[vmport].vmac[0],
					kattach_vmports.vmports[vmport].vmac[1], kattach_vmports.vmports[vmport].vmac[2],
					kattach_vmports.vmports[vmport].vmac[3], kattach_vmports.vmports[vmport].vmac[4],
					kattach_vmports.vmports[vmport].vmac[5]);
			strncat(lcmd,netcmd,strlen(netcmd));
			memset(netcmd,0,sizeof(netcmd));
			sprintf(netcmd," -net tap,ifname=vmtap%u,script=no,vlan=%u ",vmport,kattach_vbridge.vbridge[vmbridge].vlan);
			strncat(lcmd,netcmd,strlen(netcmd));
			memset(netcmd,0,sizeof(netcmd));
		}
		/* disk images */
		vmimage = kattach_vmst.vmsess[index].vmimage;
		vmport = kattach_vmst.vmsess[index].vmport[0];
		sprintf(ecmd," -drive file=%svdisk-%s.%s,if=virtio,boot=on ",KATTACH_APPQUEUE_VMDISKS,kattach_vmimages.vmimage[vmimage].vminame,
							KATTACH_VDISKEXT);
		strncat(lcmd,ecmd,strlen(ecmd));
		memset(ecmd,0,sizeof(ecmd));
		sprintf(ecmd," -drive file=%s%02x-%02x-%02x-%02x-%02x-%02x.%s,if=virtio ",KATTACH_APPQUEUE_CFGPATH,kattach_vmports.vmports[vmport].vmac[0],
					kattach_vmports.vmports[vmport].vmac[1], kattach_vmports.vmports[vmport].vmac[2],
					kattach_vmports.vmports[vmport].vmac[3], kattach_vmports.vmports[vmport].vmac[4],
					kattach_vmports.vmports[vmport].vmac[5], KATTACH_VDISKCFG);
		strncat(lcmd,ecmd,strlen(ecmd));
		memset(ecmd,0,sizeof(ecmd));
		sprintf(ecmd," -vnc 0.0.0.0:%lu -kernel %s%s -append  kaos=9:vda1:%02x%02x%02x%02x%02x%02x:0", index,
					KATTACH_VKAOS_KERNELPATH, KATTACH_VKAOS_KERNEL, kattach_vmports.vmports[vmport].vmac[0],
					kattach_vmports.vmports[vmport].vmac[1], kattach_vmports.vmports[vmport].vmac[2],
					kattach_vmports.vmports[vmport].vmac[3], kattach_vmports.vmports[vmport].vmac[4],
					kattach_vmports.vmports[vmport].vmac[5]);
		strncat(lcmd,ecmd,strlen(ecmd));
		memset(ecmd,0,sizeof(ecmd));
		if (kattach_vmst.vmsess[index].vpid == 0) {
			kattach_vmst.vmsess[index].vpid = kattach_bkexec(ccmd,lcmd);			/* this probably isn't nice!  */
		} else {
			/* FIXME: probably should stat proc/pid */
			printf("\n [!] WARNING: Virtual Guest %lu already running as %u.\n",(index+1), kattach_vmst.vmsess[index].vpid);
		}
		if (kattach_vmst.vmsess[index].vpid > 0) {
			kattach_vmst.vmsess[index].vmstatus = KATTACH_VM_STATUS_RUNNING;
			kattach_vmst.vmsess[index].vmoper = KATTACH_VM_STATUS_RUNNING;
		} else {
			kattach_vmst.vmsess[index].vmstatus = KATTACH_VM_STATUS_STOPPED;
			kattach_vmst.vmsess[index].vmoper = KATTACH_VM_STATUS_OP_START;
		}
	}

	rc = usleep(KATTACH_TIMER_QEMU_SPINUP);

	/* add vmtapX devices to proper vbridges */

	for (index = 0; index < kattach_vmst.index; index++) {
		/* FIXME: make sure we handle the scenario where the VM does not come up until later and the ports are still added */
		if (kattach_vmst.vmsess[index].vmstatus != KATTACH_VM_STATUS_RUNNING) continue;
		for (netx = 0; netx < kattach_vmst.vmsess[index].vmpidx; netx++) {
			vmport = kattach_vmst.vmsess[index].vmport[netx];
			vmbridge = kattach_vmports.vmports[vmport].vbridge;
			sprintf(ecmd,"%s%svmtap%u/operstate",KATTACH_SYSPATH,KATTACH_SYSNETPATH,vmport);
			kwaitcnt = 0;
			while ((xstat = stat(ecmd,&k_procstatus))) {
				kwaitcnt++;
				rc = usleep(KATTACH_TIMER_QEMU_SPINUP);
				if (kwaitcnt > 40) {
					/* waited about 20 seconds, no dice, probably not a good sign */
					kwaitcnt = 0xee;
					break;
				}
			}
			if (kwaitcnt == 0xee) {
				/* FIXME: probably need a CRASHED status */
				kattach_vmst.vmsess[index].vmstatus = KATTACH_VM_STATUS_STOPPED;
				kattach_vmst.vmsess[index].vmoper = KATTACH_VM_STATUS_STOPPED;
				kattach_vmst.vmsess[index].vpid = 0;
			} else {
				memset(ecmd,0,sizeof(ecmd));
				sprintf(ecmd,"%s%s link set vmtap%u up", KATTACH_BINPATH,KCMD_IP,vmport);
				kattach_sysexec(ecmd);
				sprintf(lcmd,"%s%s addif vbr%u vmtap%u", KATTACH_SBINPATH,KCMD_BRCTL,kattach_vbridge.vbridge[vmbridge].vlan, vmport);
				kattach_sysexec(lcmd);
				memset(ecmd,0,sizeof(ecmd));
				sprintf(ecmd,"%s%s %d -p %u",KATTACH_BINPATH,KCMD_RENICE,(int)((kattach_vmst.vmsess[index].priority) - 19),kattach_vmst.vmsess[index].vpid);
				kattach_sysexec(ecmd);
			}
		}
        	/* update appindex */
        	vmidx = kattach_vmst.vmsess[index].vmimage;
        	for (appidx = 0; appidx < kattach_vmimages.vmimage[vmidx].appi; appidx++) {
                	aindex = kattach_vmimages.vmimage[vmidx].appindex[appidx];
                	kattach_appmods.appmodules[aindex].deployed++;
        	}
	}
	return;

}

void
kattach_vm_apply_fw(void)
{
	char lcmd[255];
	char ecmd[128];
	char ccmd[64];
	char mcmd[64];
	char ncmd[64];
	char txtch[16];
	u32 index = 0;
	u8 y = 0, l = 0, d = 0, nindex = 0, pindex = 0, dindex = 0, ntype = 0, natindex = 0, napindex = 0;
	int x = 0;
        kattach_fw_chain_t *chain;
        kattach_fw_n_chain_t *nchain;
        kattach_fw_m_chain_t *mchain;

	x = 1;
	sprintf(lcmd,"%s%s %u > %s",KATTACH_BINPATH,KCMD_ECHO,x,"/proc/sys/net/ipv4/ip_forward");
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s%s %u > %s",KATTACH_BINPATH,KCMD_ECHO,x,"/proc/sys/net/ipv4/tcp_syncookies");
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s%s %u > %s",KATTACH_BINPATH,KCMD_ECHO,x,"/proc/sys/net/ipv4/icmp_echo_ignore_broadcasts");
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s%s %u > %s",KATTACH_BINPATH,KCMD_ECHO,x,"/proc/sys/net/ipv4/icmp_ignore_bogus_error_responses");
	kattach_sysexec(lcmd);

	/* flush tables */
	sprintf(lcmd,"%s -F",KCMD_IPT);
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -t nat -F",KCMD_IPT);
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -t mangle -F",KCMD_IPT);
	kattach_sysexec(lcmd);

	/* set policy */
	sprintf(lcmd,"%s -P INPUT DROP",KCMD_IPT);
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -P OUTPUT DROP",KCMD_IPT);
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -P FORWARD DROP",KCMD_IPT);
	kattach_sysexec(lcmd);

	sprintf(lcmd,"%s -t nat -P PREROUTING ACCEPT",KCMD_IPT);
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -t nat -P POSTROUTING ACCEPT",KCMD_IPT);
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -t nat -P OUTPUT ACCEPT",KCMD_IPT);
	kattach_sysexec(lcmd);


	sprintf(lcmd,"%s -t mangle -P PREROUTING ACCEPT",KCMD_IPT);
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -t mangle -P POSTROUTING ACCEPT",KCMD_IPT);
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -t mangle -P INPUT ACCEPT",KCMD_IPT);
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -t mangle -P OUTPUT ACCEPT",KCMD_IPT);
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -t mangle -P FORWARD ACCEPT",KCMD_IPT);
	kattach_sysexec(lcmd);

	/* apply vns rules here to avoid the table flush */
	kattach_vm_apply_vns();

	/* Add hooks to custom chains */
	sprintf(lcmd,"%s -t nat -A PREROUTING -j VNS-PREROUTING",KCMD_IPT);
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -t nat -A POSTROUTING -j VNS-POSTROUTING",KCMD_IPT);
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -A FORWARD -j VNS-FORWARD",KCMD_IPT);
	kattach_sysexec(lcmd);

	/* FIXME: special case rules should be integrated into the fw framework */
	/* special case rules */
	/* drop INVALID state traffic */
	sprintf(lcmd,"%s -A INPUT -m state --state INVALID -j DROP",KCMD_IPT);
	kattach_sysexec(lcmd);

	/* drop ICMP fragments */
	sprintf(lcmd,"%s -A INPUT -p icmp -m icmp --fragment -j DROP",KCMD_IPT);
	kattach_sysexec(lcmd);

	/* drop invalid TCP traffic */
	sprintf(lcmd,"%s -A INPUT -m state --state NEW -p tcp --tcp-flags ALL ALL -j DROP",KCMD_IPT);
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -A INPUT -m state --state NEW -p tcp --tcp-flags ALL NONE -j DROP",KCMD_IPT);
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -A INPUT -p tcp ! --syn -m state --state NEW -j DROP",KCMD_IPT);
	kattach_sysexec(lcmd);

	/* allow input traffic from the loopback interface */
	sprintf(lcmd,"%s -A INPUT -i lo -j ACCEPT", KCMD_IPT);
	kattach_sysexec(lcmd);

	/* allow input traffic from related and established connections */
	sprintf(lcmd,"%s -A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT",KCMD_IPT);
	kattach_sysexec(lcmd);

	/* allow hypervisor traffic out */
	sprintf(lcmd,"%s -A OUTPUT -o lo -j ACCEPT",KCMD_IPT);
	kattach_sysexec(lcmd);

	sprintf(lcmd,"%s -A OUTPUT -m state --state RELATED,ESTABLISHED -j ACCEPT",KCMD_IPT);
	kattach_sysexec(lcmd);

	sprintf(lcmd,"%s -A OUTPUT -m state --state NEW -j ACCEPT",KCMD_IPT);
	kattach_sysexec(lcmd);
	/* end special case rules */

	/* process rules */
	/* filter */
	while (!y) {
                if (l == 0) {
                        chain = &kattach_fw.filter.input;
			sprintf(txtch,"INPUT");
                } else if (l == 1) {
                        chain = &kattach_fw.filter.output;
			sprintf(txtch,"OUTPUT");
                } else if (l == 2) {
                        chain = &kattach_fw.filter.forward;
			sprintf(txtch,"FORWARD");
                } else {
                        y++;
                        break;
                }
		index = chain->hindex;
		d = 0;
		while (!d) {
			if (chain->filter[index].enabled) {
				for (pindex = 0; pindex < kattach_fw.apps.app[chain->filter[index].appindex].pindex; pindex++) {
					if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].direction == KATTACH_FW_DIR_SOURCE) {
						if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[chain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[chain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m icmp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (chain->filter[index].logging) {
										sprintf(lcmd,"%s -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0], ecmd,
											chain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(chain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((chain->filter[index].rlimitpkt != 0) && (chain->filter[index].rlimitint != 0)) {
											if (chain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec", chain->filter[index].rlimitpkt);
											} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min", chain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour", chain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_TCP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[chain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[chain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m tcp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (chain->filter[index].logging) {
										sprintf(lcmd,"%s -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], ecmd,
											chain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(chain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu:'",txtch,index);
											break;
									}
									if ((chain->filter[index].rlimitpkt != 0) && (chain->filter[index].rlimitint != 0)) {
											if (chain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",chain->filter[index].rlimitpkt);
											} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",chain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",chain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_UDP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[chain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[chain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m udp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (chain->filter[index].logging) {
										sprintf(lcmd,"%s -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], ecmd,
											chain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(chain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((chain->filter[index].rlimitpkt != 0) && (chain->filter[index].rlimitint != 0)) {
											if (chain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",chain->filter[index].rlimitpkt);
											} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",chain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",chain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}

						}
					} else if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].direction == KATTACH_FW_DIR_DESTINATION) {
						if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[chain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[chain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m icmp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (chain->filter[index].logging) {
										sprintf(lcmd,"%s -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0], ecmd,
											chain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(chain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((chain->filter[index].rlimitpkt != 0) && (chain->filter[index].rlimitint != 0)) {
											if (chain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec", chain->filter[index].rlimitpkt);
											} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min", chain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour", chain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_TCP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[chain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[chain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m tcp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (chain->filter[index].logging) {
										sprintf(lcmd,"%s -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], ecmd,
											chain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(chain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((chain->filter[index].rlimitpkt != 0) && (chain->filter[index].rlimitint != 0)) {
											if (chain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",chain->filter[index].rlimitpkt);
											} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",chain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",chain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_UDP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[chain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[chain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m udp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (chain->filter[index].logging) {
										sprintf(lcmd,"%s -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], ecmd,
											chain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(chain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((chain->filter[index].rlimitpkt != 0) && (chain->filter[index].rlimitint != 0)) {
											if (chain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",chain->filter[index].rlimitpkt);
											} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",chain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",chain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}

						}
					} else if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].direction == KATTACH_FW_DIR_BOTH) {
						if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[chain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[chain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m icmp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (chain->filter[index].logging) {
										sprintf(lcmd,"%s -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0], ecmd,
											chain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(chain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((chain->filter[index].rlimitpkt != 0) && (chain->filter[index].rlimitint != 0)) {
											if (chain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec", chain->filter[index].rlimitpkt);
											} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min", chain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour", chain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_TCP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[chain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[chain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m tcp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (chain->filter[index].logging) {
										sprintf(lcmd,"%s -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], ecmd,
											chain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(chain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((chain->filter[index].rlimitpkt != 0) && (chain->filter[index].rlimitint != 0)) {
											if (chain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",chain->filter[index].rlimitpkt);
											} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",chain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",chain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_UDP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[chain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[chain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m udp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (chain->filter[index].logging) {
										sprintf(lcmd,"%s -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], ecmd,
											chain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(chain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((chain->filter[index].rlimitpkt != 0) && (chain->filter[index].rlimitint != 0)) {
											if (chain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",chain->filter[index].rlimitpkt);
											} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",chain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",chain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}

						}
					}
				}

			}
			if (index == chain->eindex) {
				d++;
				break;
			} else {
				index = chain->filter[index].nindex;
			}
			
		}
		l++;
	}

	/* nat */
	y = 0;
	d = 0;
	l = 0;
	while (!y) {
                if (l == 0) {
                        nchain = &kattach_fw.nat.prerouting;
			sprintf(txtch,"PREROUTING");
                } else if (l == 1) {
                        nchain = &kattach_fw.nat.postrouting;
			sprintf(txtch,"POSTROUTING");
                } else if (l == 2) {
                        nchain = &kattach_fw.nat.output;
			sprintf(txtch,"OUTPUT");
                } else {
                        y++;
                        break;
                }
		index = nchain->hindex;
		d = 0;
		while (!d) {
			if (nchain->filter[index].enabled) {
				for (pindex = 0; pindex < kattach_fw.apps.app[nchain->filter[index].appindex].pindex; pindex++) {
					if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].direction == KATTACH_FW_DIR_SOURCE) {
						if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[nchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[nchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m icmp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (nchain->filter[index].logging) {
										sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], ecmd,
											nchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(nchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_SNAT:
											if (l == 1) {
												sprintf(ccmd,"-j SNAT");
												ntype = 1;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu SNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_DNAT:
											if (l == 0) {
												sprintf(ccmd,"-j DNAT");
												ntype = 2;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu DNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_MASQ:
											if (l == 1) {
												sprintf(ccmd,"-j MASQ");
												ntype = 4;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu MASQ not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_REDIR:
											if (l == 0) {
												sprintf(ccmd,"-j SNAT");
												ntype = 3;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu REDIRECT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_NETMAP:
											if ((l == 1) || (l == 0)) {
												sprintf(ccmd,"-j NETMAP");
												ntype = 5;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu NETMAP not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((nchain->filter[index].rlimitpkt != 0) && (nchain->filter[index].rlimitint != 0)) {
											if (nchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec", nchain->filter[index].rlimitpkt);
											} else if ((nchain->filter[index].rlimitint >= 60) && (nchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min", nchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour", nchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									if (ntype == 0) {
										sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd);
										kattach_sysexec(lcmd);
									} else if (ntype != 5) {
										/* duplicate the rule for each NAT Application, and each NAT Target Zone */
										for (napindex = 0; pindex < kattach_fw.apps.app[nchain->filter[index].nappindex].pindex; napindex++) {
											if (kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask != 
												(kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) continue;
											for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
												if (ntype == 1) {
													sprintf(ncmd," --to-source %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else if (ntype == 2) {
													sprintf(ncmd," --to-destination %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else {
													sprintf(ncmd," ");
												}
												sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s %s",
													KCMD_IPT,txtch,
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
													kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd, ncmd);
												kattach_sysexec(lcmd);
											}
										}
									} else {
										/* netmap case */
										for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
											sprintf(ncmd," --to %lu.%lu.%lu.%lu/%u",
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].mask);
											sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s %s",
												KCMD_IPT,txtch,
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
												kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd, ncmd);
											kattach_sysexec(lcmd);
										}

									}
									ntype = 0;
								}
							}
						}
						if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_TCP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[nchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[nchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m tcp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (nchain->filter[index].logging) {
										sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], ecmd,
											nchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(nchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;
										case KATTACH_FW_ACTION_SNAT:
											if (l == 1) {
												sprintf(ccmd,"-j SNAT");
												ntype = 1;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu SNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_DNAT:
											if (l == 0) {
												sprintf(ccmd,"-j DNAT");
												ntype = 2;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu DNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_MASQ:
											if (l == 1) {
												sprintf(ccmd,"-j MASQ");
												ntype = 4;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu MASQ not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_REDIR:
											if (l == 0) {
												sprintf(ccmd,"-j SNAT");
												ntype = 3;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu REDIRECT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_NETMAP:
											if ((l == 1) || (l == 0)) {
												sprintf(ccmd,"-j NETMAP");
												ntype = 5;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu NETMAP not permitted in %u: '",index,l);
											}
											break;


										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((nchain->filter[index].rlimitpkt != 0) && (nchain->filter[index].rlimitint != 0)) {
											if (nchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",nchain->filter[index].rlimitpkt);
											} else if ((nchain->filter[index].rlimitint >= 60) && (nchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",nchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",nchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}

									if (ntype == 0) {
										sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
										kattach_sysexec(lcmd);
									} else if (ntype != 5) {
										/* duplicate the rule for each NAT Application, and each NAT Target Zone */
										for (napindex = 0; pindex < kattach_fw.apps.app[nchain->filter[index].nappindex].pindex; napindex++) {
											if (kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask != 
												(kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) continue;
											for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
												if (ntype == 1) {
													/* FIXME: add support for app ports */
													sprintf(ncmd," --to-source %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else if (ntype == 2) {
													/* FIXME: add support for app ports */
													sprintf(ncmd," --to-destination %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else {
													sprintf(ncmd," ");
												}
												sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s %s",
													KCMD_IPT,txtch,
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
													kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
													kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd, ncmd);
												kattach_sysexec(lcmd);
											}
										}
									} else {
										/* netmap case */
										for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
											sprintf(ncmd," --to %lu.%lu.%lu.%lu/%u",
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].mask);
											sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s %s",
												KCMD_IPT,txtch,
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
												kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
												kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd, ncmd);
											kattach_sysexec(lcmd);
										}

									}
									ntype = 0;
								}
							}
						}
						if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_UDP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[nchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[nchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m udp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (nchain->filter[index].logging) {
										sprintf(lcmd,"%s -t nat -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], ecmd,
											nchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(nchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_SNAT:
											if (l == 1) {
												sprintf(ccmd,"-j SNAT");
												ntype = 1;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu SNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_DNAT:
											if (l == 0) {
												sprintf(ccmd,"-j DNAT");
												ntype = 2;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu DNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_MASQ:
											if (l == 1) {
												sprintf(ccmd,"-j MASQ");
												ntype = 4;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu MASQ not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_REDIR:
											if (l == 0) {
												sprintf(ccmd,"-j SNAT");
												ntype = 3;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu REDIRECT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_NETMAP:
											if ((l == 1) || (l == 0)) {
												sprintf(ccmd,"-j NETMAP");
												ntype = 5;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu NETMAP not permitted in %u: '",index,l);
											}
											break;



										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((nchain->filter[index].rlimitpkt != 0) && (nchain->filter[index].rlimitint != 0)) {
											if (nchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",nchain->filter[index].rlimitpkt);
											} else if ((nchain->filter[index].rlimitint >= 60) && (nchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",nchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",nchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t nat -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}

						}
					} else if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].direction == KATTACH_FW_DIR_DESTINATION) {
						if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[nchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[nchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m icmp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (nchain->filter[index].logging) {
										sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], ecmd,
											nchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(nchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_SNAT:
											if (l == 1) {
												sprintf(ccmd,"-j SNAT");
												ntype = 1;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu SNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_DNAT:
											if (l == 0) {
												sprintf(ccmd,"-j DNAT");
												ntype = 2;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu DNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_MASQ:
											if (l == 1) {
												sprintf(ccmd,"-j MASQ");
												ntype = 4;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu MASQ not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_REDIR:
											if (l == 0) {
												sprintf(ccmd,"-j SNAT");
												ntype = 3;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu REDIRECT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_NETMAP:
											if ((l == 1) || (l == 0)) {
												sprintf(ccmd,"-j NETMAP");
												ntype = 5;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu NETMAP not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((nchain->filter[index].rlimitpkt != 0) && (nchain->filter[index].rlimitint != 0)) {
											if (nchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec", nchain->filter[index].rlimitpkt);
											} else if ((nchain->filter[index].rlimitint >= 60) && (nchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min", nchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour", nchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									if (ntype == 0) {
										sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd);
										kattach_sysexec(lcmd);
									} else if (ntype != 5) {
										/* duplicate the rule for each NAT Application, and each NAT Target Zone */
										for (napindex = 0; pindex < kattach_fw.apps.app[nchain->filter[index].nappindex].pindex; napindex++) {
											if (kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask != 
												(kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) continue;
											for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
												if (ntype == 1) {
													sprintf(ncmd," --to-source %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else if (ntype == 2) {
													sprintf(ncmd," --to-destination %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else {
													sprintf(ncmd," ");
												}
												sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s %s",
													KCMD_IPT,txtch,
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
													kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd, ncmd);
												kattach_sysexec(lcmd);
											}
										}
									} else {
										/* netmap case */
										for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
											sprintf(ncmd," --to %lu.%lu.%lu.%lu/%u",
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].mask);
											sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s %s",
												KCMD_IPT,txtch,
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
												kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd, ncmd);
											kattach_sysexec(lcmd);
										}

									}
									ntype = 0;
								}
							}
						}
						if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_TCP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[nchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[nchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m tcp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (nchain->filter[index].logging) {
										sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], ecmd,
											nchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(nchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_SNAT:
											if (l == 1) {
												sprintf(ccmd,"-j SNAT");
												ntype = 1;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu SNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_DNAT:
											if (l == 0) {
												sprintf(ccmd,"-j DNAT");
												ntype = 2;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu DNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_MASQ:
											if (l == 1) {
												sprintf(ccmd,"-j MASQ");
												ntype = 4;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu MASQ not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_REDIR:
											if (l == 0) {
												sprintf(ccmd,"-j SNAT");
												ntype = 3;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu REDIRECT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_NETMAP:
											if ((l == 1) || (l == 0)) {
												sprintf(ccmd,"-j NETMAP");
												ntype = 5;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu NETMAP not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((nchain->filter[index].rlimitpkt != 0) && (nchain->filter[index].rlimitint != 0)) {
											if (nchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",nchain->filter[index].rlimitpkt);
											} else if ((nchain->filter[index].rlimitint >= 60) && (nchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",nchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",nchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}

									if (ntype == 0) {
										sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
										kattach_sysexec(lcmd);
									} else if (ntype != 5) {
										/* duplicate the rule for each NAT Application, and each NAT Target Zone */
										for (napindex = 0; pindex < kattach_fw.apps.app[nchain->filter[index].nappindex].pindex; napindex++) {
											if (kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask != 
												(kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) continue;
											for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
												if (ntype == 1) {
													/* FIXME: add support for app ports */
													sprintf(ncmd," --to-source %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else if (ntype == 2) {
													/* FIXME: add support for app ports */
													sprintf(ncmd," --to-destination %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else {
													sprintf(ncmd," ");
												}
												sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s %s",
													KCMD_IPT,txtch,
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
													kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
													kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd, ncmd);
												kattach_sysexec(lcmd);
											}
										}
									} else {
										/* netmap case */
										for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
											sprintf(ncmd," --to %lu.%lu.%lu.%lu/%u",
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].mask);
											sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s %s",
												KCMD_IPT,txtch,
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
												kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
												kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd, ncmd);
											kattach_sysexec(lcmd);
										}

									}
									ntype = 0;
								}
							}
						}
						if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_UDP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[nchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[nchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m udp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (nchain->filter[index].logging) {
										sprintf(lcmd,"%s -t nat -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], ecmd,
											nchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(nchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_SNAT:
											if (l == 1) {
												sprintf(ccmd,"-j SNAT");
												ntype = 1;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu SNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_DNAT:
											if (l == 0) {
												sprintf(ccmd,"-j DNAT");
												ntype = 2;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu DNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_MASQ:
											if (l == 1) {
												sprintf(ccmd,"-j MASQ");
												ntype = 4;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu MASQ not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_REDIR:
											if (l == 0) {
												sprintf(ccmd,"-j SNAT");
												ntype = 3;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu REDIRECT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_NETMAP:
											if ((l == 1) || (l == 0)) {
												sprintf(ccmd,"-j NETMAP");
												ntype = 5;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu NETMAP not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((nchain->filter[index].rlimitpkt != 0) && (nchain->filter[index].rlimitint != 0)) {
											if (nchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",nchain->filter[index].rlimitpkt);
											} else if ((nchain->filter[index].rlimitint >= 60) && (nchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",nchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",nchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t nat -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}

						}
					} else if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].direction == KATTACH_FW_DIR_BOTH) {
						if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[nchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[nchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m icmp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (nchain->filter[index].logging) {
										sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], ecmd,
											nchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(nchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_SNAT:
											if (l == 1) {
												sprintf(ccmd,"-j SNAT");
												ntype = 1;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu SNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_DNAT:
											if (l == 0) {
												sprintf(ccmd,"-j DNAT");
												ntype = 2;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu DNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_MASQ:
											if (l == 1) {
												sprintf(ccmd,"-j MASQ");
												ntype = 4;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu MASQ not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_REDIR:
											if (l == 0) {
												sprintf(ccmd,"-j SNAT");
												ntype = 3;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu REDIRECT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_NETMAP:
											if ((l == 1) || (l == 0)) {
												sprintf(ccmd,"-j NETMAP");
												ntype = 5;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu NETMAP not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((nchain->filter[index].rlimitpkt != 0) && (nchain->filter[index].rlimitint != 0)) {
											if (nchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec", nchain->filter[index].rlimitpkt);
											} else if ((nchain->filter[index].rlimitint >= 60) && (nchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min", nchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour", nchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									if (ntype == 0) {
										sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd);
										kattach_sysexec(lcmd);
									} else if (ntype != 5) {
										/* duplicate the rule for each NAT Application, and each NAT Target Zone */
										for (napindex = 0; pindex < kattach_fw.apps.app[nchain->filter[index].nappindex].pindex; napindex++) {
											if (kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask != 
												(kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) continue;
											for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
												if (ntype == 1) {
													sprintf(ncmd," --to-source %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else if (ntype == 2) {
													sprintf(ncmd," --to-destination %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else {
													sprintf(ncmd," ");
												}
												sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s %s",
													KCMD_IPT,txtch,
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
													kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd, ncmd);
												kattach_sysexec(lcmd);
											}
										}
									} else {
										/* netmap case */
										for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
											sprintf(ncmd," --to %lu.%lu.%lu.%lu/%u",
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].mask);
											sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s %s",
												KCMD_IPT,txtch,
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
												kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd, ncmd);
											kattach_sysexec(lcmd);
										}

									}
									ntype = 0;
								}
							}
						}
						if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_TCP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[nchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[nchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m tcp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (nchain->filter[index].logging) {
										sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], ecmd,
											nchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(nchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_SNAT:
											if (l == 1) {
												sprintf(ccmd,"-j SNAT");
												ntype = 1;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu SNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_DNAT:
											if (l == 0) {
												sprintf(ccmd,"-j DNAT");
												ntype = 2;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu DNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_MASQ:
											if (l == 1) {
												sprintf(ccmd,"-j MASQ");
												ntype = 4;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu MASQ not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_REDIR:
											if (l == 0) {
												sprintf(ccmd,"-j SNAT");
												ntype = 3;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu REDIRECT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_NETMAP:
											if ((l == 1) || (l == 0)) {
												sprintf(ccmd,"-j NETMAP");
												ntype = 5;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu NETMAP not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((nchain->filter[index].rlimitpkt != 0) && (nchain->filter[index].rlimitint != 0)) {
											if (nchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",nchain->filter[index].rlimitpkt);
											} else if ((nchain->filter[index].rlimitint >= 60) && (nchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",nchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",nchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}

									if (ntype == 0) {
										sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
										kattach_sysexec(lcmd);
									} else if (ntype != 5) {
										/* duplicate the rule for each NAT Application, and each NAT Target Zone */
										for (napindex = 0; pindex < kattach_fw.apps.app[nchain->filter[index].nappindex].pindex; napindex++) {
											if (kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask != 
												(kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) continue;
											for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
												if (ntype == 1) {
													/* FIXME: add support for app ports */
													sprintf(ncmd," --to-source %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else if (ntype == 2) {
													/* FIXME: add support for app ports */
													sprintf(ncmd," --to-destination %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else {
													sprintf(ncmd," ");
												}
												sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s %s",
													KCMD_IPT,txtch,
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
													kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
													kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd, ncmd);
												kattach_sysexec(lcmd);
											}
										}
									} else {
										/* netmap case */
										for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
											sprintf(ncmd," --to %lu.%lu.%lu.%lu/%u",
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].mask);
											sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s %s",
												KCMD_IPT,txtch,
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
												kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
												kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd, ncmd);
											kattach_sysexec(lcmd);
										}

									}
									ntype = 0;
								}
							}
						}
						if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_UDP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[nchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[nchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m udp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (nchain->filter[index].logging) {
										sprintf(lcmd,"%s -t nat -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], ecmd,
											nchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(nchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_SNAT:
											if (l == 1) {
												sprintf(ccmd,"-j SNAT");
												ntype = 1;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu SNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_DNAT:
											if (l == 0) {
												sprintf(ccmd,"-j DNAT");
												ntype = 2;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu DNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_MASQ:
											if (l == 1) {
												sprintf(ccmd,"-j MASQ");
												ntype = 4;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu MASQ not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_REDIR:
											if (l == 0) {
												sprintf(ccmd,"-j SNAT");
												ntype = 3;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu REDIRECT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_NETMAP:
											if ((l == 1) || (l == 0)) {
												sprintf(ccmd,"-j NETMAP");
												ntype = 5;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu NETMAP not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((nchain->filter[index].rlimitpkt != 0) && (nchain->filter[index].rlimitint != 0)) {
											if (nchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",nchain->filter[index].rlimitpkt);
											} else if ((nchain->filter[index].rlimitint >= 60) && (nchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",nchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",nchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t nat -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}

						}
					}
				}

			}
			if (index == nchain->eindex) {
				d++;
				break;
			} else {
				index = nchain->filter[index].nindex;
			}
			
		}
		l++;
	}

	/* mangle */
	y = 0;
	d = 0;	
	l = 0;
	while (!y) {
                if (l == 0) {
                        mchain = &kattach_fw.mangle.input;
			sprintf(txtch,"INPUT");
                } else if (l == 1) {
                        mchain = &kattach_fw.mangle.output;
			sprintf(txtch,"OUTPUT");
                } else if (l == 2) {
                        mchain = &kattach_fw.mangle.forward;
			sprintf(txtch,"FORWARD");
                } else if (l == 3) {
                        mchain = &kattach_fw.mangle.prerouting;
			sprintf(txtch,"PREROUTING");
                } else if (l == 4) {
                        mchain = &kattach_fw.mangle.postrouting;
			sprintf(txtch,"POSTROUTING");
                } else {
                        y++;
                        break;
                }
		index = mchain->hindex;
		d = 0;
		while (!d) {
			if (mchain->filter[index].enabled) {
				for (pindex = 0; pindex < kattach_fw.apps.app[mchain->filter[index].appindex].pindex; pindex++) {
					if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].direction == KATTACH_FW_DIR_SOURCE) {
						if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[mchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[mchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m icmp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (mchain->filter[index].logging) {
										sprintf(lcmd,"%s -t mangle -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0], ecmd,
											mchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(mchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_MARK:
											if (mchain->filter[index].ttl[0] == 3) {
												sprintf(ccmd,"-m ttl --ttl-eq %u -j MARK --set-mark %lu", mchain->filter[index].ttl[1], mchain->filter[index].mark);
											} else if (mchain->filter[index].tos[0] == 1) {
												sprintf(ccmd,"-m tos --tos 0x%x -j MARK --set-mark %lu", mchain->filter[index].tos[1], mchain->filter[index].mark);
											} else {
												sprintf(ccmd,"-j MARK --set-mark %lu", mchain->filter[index].mark);
											}
											break;

										case KATTACH_FW_ACTION_TTL:
											switch (mchain->filter[index].ttl[0]) {
												case 1:
													sprintf(ccmd,"-j TTL --ttl-dec %u", mchain->filter[index].ttl[1]);
													break;

												case 2:
													sprintf(ccmd,"-j TTL --ttl-inc %u", mchain->filter[index].ttl[1]);
													break;

												case 0:
												default:
													sprintf(ccmd,"-j TTL --set-ttl %u", mchain->filter[index].ttl[1]);
													break;
											}
											break;

										case KATTACH_FW_ACTION_TOS:
											sprintf(ccmd,"-j TOS --set-tos 0x%x",mchain->filter[index].tos[1]);
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((mchain->filter[index].rlimitpkt != 0) && (mchain->filter[index].rlimitint != 0)) {
											if (mchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec", mchain->filter[index].rlimitpkt);
											} else if ((mchain->filter[index].rlimitint >= 60) && (mchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min", mchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour", mchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t mangle -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_TCP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[mchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[mchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m tcp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (mchain->filter[index].logging) {
										sprintf(lcmd,"%s -t mangle -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], ecmd,
											mchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(mchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_MARK:
											if (mchain->filter[index].ttl[0] == 3) {
												sprintf(ccmd,"-m ttl --ttl-eq %u -j MARK --set-mark %lu", mchain->filter[index].ttl[1], mchain->filter[index].mark);
											} else if (mchain->filter[index].tos[0] == 1) {
												sprintf(ccmd,"-m tos --tos 0x%x -j MARK --set-mark %lu", mchain->filter[index].tos[1], mchain->filter[index].mark);
											} else {
												sprintf(ccmd,"-j MARK --set-mark %lu", mchain->filter[index].mark);
											}
											break;

										case KATTACH_FW_ACTION_TTL:
											switch (mchain->filter[index].ttl[0]) {
												case 1:
													sprintf(ccmd,"-j TTL --ttl-dec %u", mchain->filter[index].ttl[1]);
													break;

												case 2:
													sprintf(ccmd,"-j TTL --ttl-inc %u", mchain->filter[index].ttl[1]);
													break;

												case 0:
												default:
													sprintf(ccmd,"-j TTL --set-ttl %u", mchain->filter[index].ttl[1]);
													break;
											}
											break;

										case KATTACH_FW_ACTION_TOS:
											sprintf(ccmd,"-j TOS --set-tos 0x%x",mchain->filter[index].tos[1]);
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((mchain->filter[index].rlimitpkt != 0) && (mchain->filter[index].rlimitint != 0)) {
											if (mchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",mchain->filter[index].rlimitpkt);
											} else if ((mchain->filter[index].rlimitint >= 60) && (mchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",mchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",mchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t mangle -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_UDP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[mchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[mchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m udp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (mchain->filter[index].logging) {
										sprintf(lcmd,"%s -t mangle -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], ecmd,
											mchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(mchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_MARK:
											if (mchain->filter[index].ttl[0] == 3) {
												sprintf(ccmd,"-m ttl --ttl-eq %u -j MARK --set-mark %lu", mchain->filter[index].ttl[1], mchain->filter[index].mark);
											} else if (mchain->filter[index].tos[0] == 1) {
												sprintf(ccmd,"-m tos --tos 0x%x -j MARK --set-mark %lu", mchain->filter[index].tos[1], mchain->filter[index].mark);
											} else {
												sprintf(ccmd,"-j MARK --set-mark %lu", mchain->filter[index].mark);
											}
											break;

										case KATTACH_FW_ACTION_TTL:
											switch (mchain->filter[index].ttl[0]) {
												case 1:
													sprintf(ccmd,"-j TTL --ttl-dec %u", mchain->filter[index].ttl[1]);
													break;

												case 2:
													sprintf(ccmd,"-j TTL --ttl-inc %u", mchain->filter[index].ttl[1]);
													break;

												case 0:
												default:
													sprintf(ccmd,"-j TTL --set-ttl %u", mchain->filter[index].ttl[1]);
													break;
											}
											break;

										case KATTACH_FW_ACTION_TOS:
											sprintf(ccmd,"-j TOS --set-tos 0x%x",mchain->filter[index].tos[1]);
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((mchain->filter[index].rlimitpkt != 0) && (mchain->filter[index].rlimitint != 0)) {
											if (mchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",mchain->filter[index].rlimitpkt);
											} else if ((mchain->filter[index].rlimitint >= 60) && (mchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",mchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",mchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t mangle -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}

						}
					} else if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].direction == KATTACH_FW_DIR_DESTINATION) {
						if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[mchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[mchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m icmp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (mchain->filter[index].logging) {
										sprintf(lcmd,"%s -t mangle -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0], ecmd,
											mchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(mchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_MARK:
											if (mchain->filter[index].ttl[0] == 3) {
												sprintf(ccmd,"-m ttl --ttl-eq %u -j MARK --set-mark %lu", mchain->filter[index].ttl[1], mchain->filter[index].mark);
											} else if (mchain->filter[index].tos[0] == 1) {
												sprintf(ccmd,"-m tos --tos 0x%x -j MARK --set-mark %lu", mchain->filter[index].tos[1], mchain->filter[index].mark);
											} else {
												sprintf(ccmd,"-j MARK --set-mark %lu", mchain->filter[index].mark);
											}
											break;

										case KATTACH_FW_ACTION_TTL:
											switch (mchain->filter[index].ttl[0]) {
												case 1:
													sprintf(ccmd,"-j TTL --ttl-dec %u", mchain->filter[index].ttl[1]);
													break;

												case 2:
													sprintf(ccmd,"-j TTL --ttl-inc %u", mchain->filter[index].ttl[1]);
													break;

												case 0:
												default:
													sprintf(ccmd,"-j TTL --set-ttl %u", mchain->filter[index].ttl[1]);
													break;
											}
											break;

										case KATTACH_FW_ACTION_TOS:
											sprintf(ccmd,"-j TOS --set-tos 0x%x",mchain->filter[index].tos[1]);
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((mchain->filter[index].rlimitpkt != 0) && (mchain->filter[index].rlimitint != 0)) {
											if (mchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec", mchain->filter[index].rlimitpkt);
											} else if ((mchain->filter[index].rlimitint >= 60) && (mchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min", mchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour", mchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t mangle -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_TCP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[mchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[mchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m tcp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (mchain->filter[index].logging) {
										sprintf(lcmd,"%s -t mangle -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], ecmd,
											mchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(mchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_MARK:
											if (mchain->filter[index].ttl[0] == 3) {
												sprintf(ccmd,"-m ttl --ttl-eq %u -j MARK --set-mark %lu", mchain->filter[index].ttl[1], mchain->filter[index].mark);
											} else if (mchain->filter[index].tos[0] == 1) {
												sprintf(ccmd,"-m tos --tos 0x%x -j MARK --set-mark %lu", mchain->filter[index].tos[1], mchain->filter[index].mark);
											} else {
												sprintf(ccmd,"-j MARK --set-mark %lu", mchain->filter[index].mark);
											}
											break;

										case KATTACH_FW_ACTION_TTL:
											switch (mchain->filter[index].ttl[0]) {
												case 1:
													sprintf(ccmd,"-j TTL --ttl-dec %u", mchain->filter[index].ttl[1]);
													break;

												case 2:
													sprintf(ccmd,"-j TTL --ttl-inc %u", mchain->filter[index].ttl[1]);
													break;

												case 0:
												default:
													sprintf(ccmd,"-j TTL --set-ttl %u", mchain->filter[index].ttl[1]);
													break;
											}
											break;

										case KATTACH_FW_ACTION_TOS:
											sprintf(ccmd,"-j TOS --set-tos 0x%x",mchain->filter[index].tos[1]);
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((mchain->filter[index].rlimitpkt != 0) && (mchain->filter[index].rlimitint != 0)) {
											if (mchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",mchain->filter[index].rlimitpkt);
											} else if ((mchain->filter[index].rlimitint >= 60) && (mchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",mchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",mchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t mangle -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_UDP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[mchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[mchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m udp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (mchain->filter[index].logging) {
										sprintf(lcmd,"%s -t mangle -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], ecmd,
											mchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(mchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_MARK:
											if (mchain->filter[index].ttl[0] == 3) {
												sprintf(ccmd,"-m ttl --ttl-eq %u -j MARK --set-mark %lu", mchain->filter[index].ttl[1], mchain->filter[index].mark);
											} else if (mchain->filter[index].tos[0] == 1) {
												sprintf(ccmd,"-m tos --tos 0x%x -j MARK --set-mark %lu", mchain->filter[index].tos[1], mchain->filter[index].mark);
											} else {
												sprintf(ccmd,"-j MARK --set-mark %lu", mchain->filter[index].mark);
											}
											break;

										case KATTACH_FW_ACTION_TTL:
											switch (mchain->filter[index].ttl[0]) {
												case 1:
													sprintf(ccmd,"-j TTL --ttl-dec %u", mchain->filter[index].ttl[1]);
													break;

												case 2:
													sprintf(ccmd,"-j TTL --ttl-inc %u", mchain->filter[index].ttl[1]);
													break;

												case 0:
												default:
													sprintf(ccmd,"-j TTL --set-ttl %u", mchain->filter[index].ttl[1]);
													break;
											}
											break;

										case KATTACH_FW_ACTION_TOS:
											sprintf(ccmd,"-j TOS --set-tos 0x%x",mchain->filter[index].tos[1]);
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((mchain->filter[index].rlimitpkt != 0) && (mchain->filter[index].rlimitint != 0)) {
											if (mchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",mchain->filter[index].rlimitpkt);
											} else if ((mchain->filter[index].rlimitint >= 60) && (mchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",mchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",mchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t mangle -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}

						}
					} else if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].direction == KATTACH_FW_DIR_BOTH) {
						if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[mchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[mchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m icmp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (mchain->filter[index].logging) {
										sprintf(lcmd,"%s -t mangle -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0], ecmd,
											mchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(mchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_MARK:
											if (mchain->filter[index].ttl[0] == 3) {
												sprintf(ccmd,"-m ttl --ttl-eq %u -j MARK --set-mark %lu", mchain->filter[index].ttl[1], mchain->filter[index].mark);
											} else if (mchain->filter[index].tos[0] == 1) {
												sprintf(ccmd,"-m tos --tos 0x%x -j MARK --set-mark %lu", mchain->filter[index].tos[1], mchain->filter[index].mark);
											} else {
												sprintf(ccmd,"-j MARK --set-mark %lu", mchain->filter[index].mark);
											}
											break;

										case KATTACH_FW_ACTION_TTL:
											switch (mchain->filter[index].ttl[0]) {
												case 1:
													sprintf(ccmd,"-j TTL --ttl-dec %u", mchain->filter[index].ttl[1]);
													break;

												case 2:
													sprintf(ccmd,"-j TTL --ttl-inc %u", mchain->filter[index].ttl[1]);
													break;

												case 0:
												default:
													sprintf(ccmd,"-j TTL --set-ttl %u", mchain->filter[index].ttl[1]);
													break;
											}
											break;

										case KATTACH_FW_ACTION_TOS:
											sprintf(ccmd,"-j TOS --set-tos 0x%x",mchain->filter[index].tos[1]);
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((mchain->filter[index].rlimitpkt != 0) && (mchain->filter[index].rlimitint != 0)) {
											if (mchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec", mchain->filter[index].rlimitpkt);
											} else if ((mchain->filter[index].rlimitint >= 60) && (mchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min", mchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour", mchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t mangle -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_TCP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[mchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[mchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m tcp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (mchain->filter[index].logging) {
										sprintf(lcmd,"%s -t mangle -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], ecmd,
											mchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(mchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_MARK:
											if (mchain->filter[index].ttl[0] == 3) {
												sprintf(ccmd,"-m ttl --ttl-eq %u -j MARK --set-mark %lu", mchain->filter[index].ttl[1], mchain->filter[index].mark);
											} else if (mchain->filter[index].tos[0] == 1) {
												sprintf(ccmd,"-m tos --tos 0x%x -j MARK --set-mark %lu", mchain->filter[index].tos[1], mchain->filter[index].mark);
											} else {
												sprintf(ccmd,"-j MARK --set-mark %lu", mchain->filter[index].mark);
											}
											break;

										case KATTACH_FW_ACTION_TTL:
											switch (mchain->filter[index].ttl[0]) {
												case 1:
													sprintf(ccmd,"-j TTL --ttl-dec %u", mchain->filter[index].ttl[1]);
													break;

												case 2:
													sprintf(ccmd,"-j TTL --ttl-inc %u", mchain->filter[index].ttl[1]);
													break;

												case 0:
												default:
													sprintf(ccmd,"-j TTL --set-ttl %u", mchain->filter[index].ttl[1]);
													break;
											}
											break;

										case KATTACH_FW_ACTION_TOS:
											sprintf(ccmd,"-j TOS --set-tos 0x%x",mchain->filter[index].tos[1]);
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((mchain->filter[index].rlimitpkt != 0) && (mchain->filter[index].rlimitint != 0)) {
											if (mchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",mchain->filter[index].rlimitpkt);
											} else if ((mchain->filter[index].rlimitint >= 60) && (mchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",mchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",mchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t mangle -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_UDP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[mchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[mchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m udp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (mchain->filter[index].logging) {
										sprintf(lcmd,"%s -t mangle -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], ecmd,
											mchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(mchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_MARK:
											if (mchain->filter[index].ttl[0] == 3) {
												sprintf(ccmd,"-m ttl --ttl-eq %u -j MARK --set-mark %lu", mchain->filter[index].ttl[1], mchain->filter[index].mark);
											} else if (mchain->filter[index].tos[0] == 1) {
												sprintf(ccmd,"-m tos --tos 0x%x -j MARK --set-mark %lu", mchain->filter[index].tos[1], mchain->filter[index].mark);
											} else {
												sprintf(ccmd,"-j MARK --set-mark %lu", mchain->filter[index].mark);
											}
											break;

										case KATTACH_FW_ACTION_TTL:
											switch (mchain->filter[index].ttl[0]) {
												case 1:
													sprintf(ccmd,"-j TTL --ttl-dec %u", mchain->filter[index].ttl[1]);
													break;

												case 2:
													sprintf(ccmd,"-j TTL --ttl-inc %u", mchain->filter[index].ttl[1]);
													break;

												case 0:
												default:
													sprintf(ccmd,"-j TTL --set-ttl %u", mchain->filter[index].ttl[1]);
													break;
											}
											break;

										case KATTACH_FW_ACTION_TOS:
											sprintf(ccmd,"-j TOS --set-tos 0x%x",mchain->filter[index].tos[1]);
											break;


										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((mchain->filter[index].rlimitpkt != 0) && (mchain->filter[index].rlimitint != 0)) {
											if (mchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",mchain->filter[index].rlimitpkt);
											} else if ((mchain->filter[index].rlimitint >= 60) && (mchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",mchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",mchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t mangle -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}

						}
					}
				}

			}
			if (index == mchain->eindex) {
				d++;
				break;
			} else {
				index = mchain->filter[index].nindex;
			}
			
		}
		l++;
	}


	return;
}

void
kattach_vm_apply_vns(void)
{
	char lcmd[255];	
	u32 index = 0, stip = 0, endip = 0, curip = 0;
	u16 netifdev = 0, vlan = 0;
	u8 vspindex = 0;

	/* create VNS chains */
	sprintf(lcmd,"%s -t nat -N VNS-PREROUTING",KCMD_IPT);
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -t nat -N VNS-POSTROUTING",KCMD_IPT);
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -t filter -N VNS-FORWARD",KCMD_IPT);
	kattach_sysexec(lcmd);

	/* process VNS configuration */
	/* add IP address to interface */
	/* add rules */

	for (index = 0; index < KATTACH_MAX_VNS; index++) {
		if (kattach_vns.vns[index].enabled == 0) continue;		/* skip */
		if (kattach_vns.vns[index].vsmsk != 32) {
			/* this is a subnet range rather than a single ip */
			stip = ((kattach_net_netaddr(kattach_vns.vns[index].vsip, kattach_vns.vns[index].vsmsk)) + 1);
			endip = kattach_net_bcast(stip, kattach_vns.vns[index].vsmsk);
			for (curip = stip; curip < endip; curip++) {
				sprintf(lcmd,"%s%s addr add %s/%u dev %s",KATTACH_BINPATH,KCMD_IP,kattach_net_parseip(curip),
						kattach_vns.vns[index].vsmsk, kattach_netdev.pif[kattach_vns.vns[index].netifidx].devname);
				kattach_sysexec(lcmd);
			}
		} else if (kattach_vns.vns[index].vsip != kattach_netdev.pif[kattach_vns.vns[index].netifidx].ip) {
			/* Virtual Service IP does not match the interface IP, so lets add it */
			/* FIXME: Check for LACP interface, pvid == 0x8023 */
			/* FIXME: Test that interface is actually up */
			/* FIXME: This gets called by update routine, errors are harmless but we should check if IP is already added */
			sprintf(lcmd,"%s%s addr add %s/%u dev %s",KATTACH_BINPATH,KCMD_IP,kattach_net_parseip(kattach_vns.vns[index].vsip),
									kattach_vns.vns[index].vsmsk, kattach_netdev.pif[kattach_vns.vns[index].netifidx].devname);
			kattach_sysexec(lcmd);
		}
		kattach_netdev.pif[kattach_vns.vns[index].netifidx].vns = 1;
		for (vspindex = 0; vspindex < kattach_vns.vns[index].vspindex; vspindex++) {
			if (kattach_vns.vns[index].vsp[vspindex].enabled == 0) continue;		/* skip */
			vlan = kattach_vbridge.vbridge[kattach_vmports.vmports[kattach_vns.vns[index].vsp[vspindex].vmport].vbridge].vlan;
			if (kattach_vns.vns[index].vsmsk != 32) {
				if (kattach_vns.vns[index].vsp[vspindex].sproto == 0) {
					sprintf(lcmd,"%s -t nat -A VNS-PREROUTING -i %s -p tcp -d %s/%u --dport %u -j DNAT --to-destination %s:%u",KCMD_IPT,
							kattach_netdev.pif[kattach_vns.vns[index].netifidx].devname, 
							kattach_net_parseip(kattach_vns.vns[index].vsip),
							kattach_vns.vns[index].vsmsk,
							kattach_vns.vns[index].vsp[vspindex].vsport, 
							kattach_net_parseip(kattach_vmports.vmports[kattach_vns.vns[index].vsp[vspindex].vmport].vmpip),
							kattach_vns.vns[index].vsp[vspindex].vmsport);
					kattach_sysexec(lcmd);
					sprintf(lcmd,"%s -t nat -A VNS-POSTROUTING -o vbr%u -p tcp -s %s --sport %u -j SNAT --to-source %s/%u",KCMD_IPT,
							vlan, 
							kattach_net_parseip(kattach_vmports.vmports[kattach_vns.vns[index].vsp[vspindex].vmport].vmpip),
							kattach_vns.vns[index].vsp[vspindex].vmsport, 
							kattach_net_parseip(kattach_vns.vns[index].vsip),
							kattach_vns.vns[index].vsmsk);
					kattach_sysexec(lcmd);
					sprintf(lcmd,"%s -t filter -A VNS-FORWARD -i %s -o vbr%u -p tcp -d %s --dport %u -m state --state NEW -j ACCEPT", KCMD_IPT,
							kattach_netdev.pif[kattach_vns.vns[index].netifidx].devname,
							vlan,
							kattach_net_parseip(kattach_vmports.vmports[kattach_vns.vns[index].vsp[vspindex].vmport].vmpip),
							kattach_vns.vns[index].vsp[vspindex].vmsport);
					kattach_sysexec(lcmd);	
				} else {
					sprintf(lcmd,"%s -t nat -A VNS-PREROUTING -i %s -p udp -d %s/%u --dport %u -j DNAT --to-destination %s:%u",KCMD_IPT,
							kattach_netdev.pif[kattach_vns.vns[index].netifidx].devname, 
							kattach_net_parseip(kattach_vns.vns[index].vsip),
							kattach_vns.vns[index].vsmsk,
							kattach_vns.vns[index].vsp[vspindex].vsport, 
							kattach_net_parseip(kattach_vmports.vmports[kattach_vns.vns[index].vsp[vspindex].vmport].vmpip),
							kattach_vns.vns[index].vsp[vspindex].vmsport);
					kattach_sysexec(lcmd);
					sprintf(lcmd,"%s -t nat -A VNS-POSTROUTING -o vbr%u -p udp -s %s --sport %u -j SNAT --to-source %s/%u",KCMD_IPT,
							vlan, 
							kattach_net_parseip(kattach_vmports.vmports[kattach_vns.vns[index].vsp[vspindex].vmport].vmpip),
							kattach_vns.vns[index].vsp[vspindex].vmsport, 
							kattach_net_parseip(kattach_vns.vns[index].vsip),
							kattach_vns.vns[index].vsmsk);
					kattach_sysexec(lcmd);
					sprintf(lcmd,"%s -t filter -A VNS-FORWARD -i %s -o vbr%u -p udp -d %s --dport %u -m state --state NEW -j ACCEPT", KCMD_IPT,
							kattach_netdev.pif[kattach_vns.vns[index].netifidx].devname,
							vlan,
							kattach_net_parseip(kattach_vmports.vmports[kattach_vns.vns[index].vsp[vspindex].vmport].vmpip),
							kattach_vns.vns[index].vsp[vspindex].vmsport);
					kattach_sysexec(lcmd);	
				}
			} else {
				if (kattach_vns.vns[index].vsp[vspindex].sproto == 0) {
					sprintf(lcmd,"%s -t nat -A VNS-PREROUTING -i %s -p tcp -d %s --dport %u -j DNAT --to-destination %s:%u",KCMD_IPT,
							kattach_netdev.pif[kattach_vns.vns[index].netifidx].devname, 
							kattach_net_parseip(kattach_vns.vns[index].vsip),
							kattach_vns.vns[index].vsp[vspindex].vsport, 
							kattach_net_parseip(kattach_vmports.vmports[kattach_vns.vns[index].vsp[vspindex].vmport].vmpip),
							kattach_vns.vns[index].vsp[vspindex].vmsport);
					kattach_sysexec(lcmd);
					sprintf(lcmd,"%s -t nat -A VNS-POSTROUTING -o vbr%u -p tcp -s %s --sport %u -j SNAT --to-source %s:%u",KCMD_IPT,
							vlan, 
							kattach_net_parseip(kattach_vmports.vmports[kattach_vns.vns[index].vsp[vspindex].vmport].vmpip),
							kattach_vns.vns[index].vsp[vspindex].vmsport, 
							kattach_net_parseip(kattach_vns.vns[index].vsip),
							kattach_vns.vns[index].vsp[vspindex].vsport);
					kattach_sysexec(lcmd);
					sprintf(lcmd,"%s -t filter -A VNS-FORWARD -i %s -o vbr%u -p tcp -d %s --dport %u -m state --state NEW -j ACCEPT", KCMD_IPT,
							kattach_netdev.pif[kattach_vns.vns[index].netifidx].devname,
							vlan,
							kattach_net_parseip(kattach_vmports.vmports[kattach_vns.vns[index].vsp[vspindex].vmport].vmpip),
							kattach_vns.vns[index].vsp[vspindex].vmsport);
					kattach_sysexec(lcmd);	
				} else {
					sprintf(lcmd,"%s -t nat -A VNS-PREROUTING -i %s -p udp -d %s --dport %u -j DNAT --to-destination %s:%u",KCMD_IPT,
							kattach_netdev.pif[kattach_vns.vns[index].netifidx].devname, 
							kattach_net_parseip(kattach_vns.vns[index].vsip),
							kattach_vns.vns[index].vsp[vspindex].vsport, 
							kattach_net_parseip(kattach_vmports.vmports[kattach_vns.vns[index].vsp[vspindex].vmport].vmpip),
							kattach_vns.vns[index].vsp[vspindex].vmsport);
					kattach_sysexec(lcmd);
					sprintf(lcmd,"%s -t nat -A VNS-POSTROUTING -o vbr%u -p udp -s %s --sport %u -j SNAT --to-source %s:%u",KCMD_IPT,
							vlan, 
							kattach_net_parseip(kattach_vmports.vmports[kattach_vns.vns[index].vsp[vspindex].vmport].vmpip),
							kattach_vns.vns[index].vsp[vspindex].vmsport, 
							kattach_net_parseip(kattach_vns.vns[index].vsip),
							kattach_vns.vns[index].vsp[vspindex].vsport);
					kattach_sysexec(lcmd);
					sprintf(lcmd,"%s -t filter -A VNS-FORWARD -i %s -o vbr%u -p udp -d %s --dport %u -m state --state NEW -j ACCEPT", KCMD_IPT,
							kattach_netdev.pif[kattach_vns.vns[index].netifidx].devname,
							vlan,
							kattach_net_parseip(kattach_vmports.vmports[kattach_vns.vns[index].vsp[vspindex].vmport].vmpip),
							kattach_vns.vns[index].vsp[vspindex].vmsport);
					kattach_sysexec(lcmd);	
				}
			}

		}
		
	}

	/* loop netdev here */
	for (netifdev = 0; netifdev < KATTACH_MAX_IFDEV; netifdev++) {
		if ((kattach_netdev.pif[netifdev].status == KATTACH_LINK_STATUS_DISABLED) ||
		    (kattach_netdev.pif[netifdev].status == KATTACH_LINK_STATUS_DELETED)) continue;
		if (kattach_netdev.pif[netifdev].vns == 0) continue;
		sprintf(lcmd,"%s -t filter -A VNS-FORWARD -o %s -m state --state NEW,ESTABLISHED,RELATED -j ACCEPT",KCMD_IPT, kattach_netdev.pif[netifdev].devname);
		kattach_sysexec(lcmd);
		sprintf(lcmd,"%s -t filter -A VNS-FORWARD -i %s -m state --state ESTABLISHED,RELATED -j ACCEPT",KCMD_IPT, kattach_netdev.pif[netifdev].devname);
		kattach_sysexec(lcmd);
		kattach_netdev.pif[netifdev].vns = 0;
	}

	sprintf(lcmd,"%s -t filter -A VNS-FORWARD -j RETURN",KCMD_IPT);
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -t nat -A VNS-PREROUTING -j RETURN",KCMD_IPT);
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -t nat -A VNS-POSTROUTING -j RETURN",KCMD_IPT);
	kattach_sysexec(lcmd);

	return;
}

void
kattach_vm_vns_update(void)
{
	u32 index = 0, vsp = 0;
	u8 vspindex = 0;
	char lcmd[255];
	char sqlq[1024];
	u8 rc = 0;

	/* FIXME: harmless but these will throw console errors on first time configuration */
	sprintf(lcmd,"%s -t filter -F VNS-FORWARD",KCMD_IPT);	
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -t nat -F VNS-PREROUTING",KCMD_IPT);	
	kattach_sysexec(lcmd);
	sprintf(lcmd,"%s -t nat -F VNS-POSTROUTING",KCMD_IPT);	
	kattach_sysexec(lcmd);

	kattach_vm_apply_vns();

	/* drop database tables */
	rc = kattach_vm_sql_iu(CMSQL_VMSESS_DROP_VSLINK, K_DB_VMSESSION);
	rc = kattach_vm_sql_iu(CMSQL_VMSESS_DROP_VSN, K_DB_VMSESSION);
	rc = kattach_vm_sql_iu(CMSQL_VMSESS_DROP_VSP, K_DB_VMSESSION);

	/* create database tables */
	rc = kattach_vm_sql_iu(CMSQL_VMSESS_CREATE_TABLE_VSP, K_DB_VMSESSION);
	rc = kattach_vm_sql_iu(CMSQL_VMSESS_CREATE_TABLE_VSN, K_DB_VMSESSION);
	rc = kattach_vm_sql_iu(CMSQL_VMSESS_CREATE_TABLE_VSLINK, K_DB_VMSESSION);

	/* add to database */
	for (index = 0; index < KATTACH_MAX_VNS; index++) {
		if (kattach_vns.vns[index].enabled == 0) continue;              /* skip */
		memset(sqlq,0,sizeof(sqlq));
		sprintf(sqlq,"INSERT into vsn values (%lu,%lu,%u,%u,%u,%lu);",(1 + index), kattach_vns.vns[index].vsip, kattach_vns.vns[index].vsmsk,
				kattach_vns.vns[index].enabled, kattach_vns.vns[index].mstate, kattach_vns.vns[index].netifidx);
		rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
		for (vspindex = 0; vspindex < 32; vspindex++) {
			if (kattach_vns.vns[index].vsp[vspindex].enabled == 0) continue;		/* skip */
			sprintf(sqlq,"INSERT into vsp values (%lu, %lu, %lu, %u, %u, %u, %u, %u, %u, %u);",(1 + vsp), kattach_vns.vns[index].vsp[vspindex].rate_in,
					kattach_vns.vns[index].vsp[vspindex].rate_out, kattach_vns.vns[index].vsp[vspindex].vsport, 
					kattach_vns.vns[index].vsp[vspindex].vmsport, (1 + kattach_vns.vns[index].vsp[vspindex].vmport),
					kattach_vns.vns[index].vsp[vspindex].time_in, kattach_vns.vns[index].vsp[vspindex].time_out,
					kattach_vns.vns[index].vsp[vspindex].sproto, kattach_vns.vns[index].vsp[vspindex].enabled);
			rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
			sprintf(sqlq,"INSERT into vslink (vspindex, vsnindex) values (%lu,%lu);",(1 + vsp), (1 + index));
			rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
			vsp++;
		}	
	}

	return;
}

void
kattach_vm_launch_mgr(void)
{
	u32 index = 0;
	char ccmd[64];
	char lcmd[128];

	/* FIXME: add support for multiple vendors */

	for (index = 0; index < KATTACH_MAX_CFGGRP; index++) {
		if (kattach_cfggrp.cfggrp[index].name[0] == '\0') continue;
		if (kattach_appmods.appmodules[kattach_cfggrp.cfggrp[index].appmidx].mgrpid == 0) {
			sprintf(ccmd,"%s%s/%s%s",KATTACH_APPQUEUE_APPMOD,KATTACH_APPQUEUE_VENDOR_CM,
				kattach_appmods.appmodules[kattach_cfggrp.cfggrp[index].appmidx].name,KATTACH_APPQUEUE_MGRPATH);
			sprintf(lcmd,"%s",kattach_cfggrp.cfggrp[index].name);
			kattach_appmods.appmodules[kattach_cfggrp.cfggrp[index].appmidx].mgrpid = kattach_bkexec(ccmd,lcmd);
		}
	}
	return;
}

void
kattach_vm_dhcpd(void)
{
	FILE *stream;
	char dhcpdconf[255];
	char lcmd[255];
	char ccmd[64];
	u32 vbrindex = 0, vmask = 0, vbcast = 0;
	u16 vmpindex = 0;
	int res = 0;

	sprintf(dhcpdconf,"%s%s",KATTACH_APPQUEUE_SVCCFGPATH,KATTACH_CONF_DHCPD);
	stream = fopen(dhcpdconf,"w");

	if (stream == (FILE *)0) {
		printf("\n [!] FATAL -- Unable to write %s\n",dhcpdconf);
		return;
	}

	fprintf(stream,"# dhcpd.conf\n# Auto-generated by kattach\n# DO NOT EDIT THIS FILE \n\n");
	fprintf(stream,"authoritative;\n");
	fprintf(stream,"log-facility local7;\n");
	fprintf(stream,"ignore client-updates;\n\n");

	/* here we need to write :

	   subnet [ip] netmask [mask] {
		range start_ip end_ip;
		option domain-name-servers dns_ip;
		option domain-name "domain";
		option routers gw-is-bridge-ip;
		option subnet-mask 255.255.255.0;
		option broadcast-address 10.255.255.255
		option ip-forwarding off;
		default-lease-time 86400;
		max-lease-time 86400;

		host foo {
			hardware ethernet 00:1e:6c:aa:bb:cc;
			fixed-address 192.168.1.1;
		}

	 */
	
	for (vbrindex = 0; vbrindex < kattach_vbridge.index; vbrindex++) {
		if (kattach_vbridge.vbridge[vbrindex].vlan == 0) continue;
		if ((kattach_vbridge.vbridge[vbrindex].state == KATTACH_VBR_STATE_DELETED) ||
                        (kattach_vbridge.vbridge[vbrindex].state == KATTACH_VBR_STATE_DISABLED))
                        continue;
		if (kattach_vbridge.vbridge[vbrindex].vbrlocal == KATTACH_NET_VLAN_ROUTED)
			continue;
		vmask = kattach_net_mask(kattach_vbridge.vbridge[vbrindex].vmask);
		vbcast = kattach_net_bcast(kattach_vbridge.vbridge[vbrindex].vbrip, kattach_vbridge.vbridge[vbrindex].vmask);
		fprintf(stream,"\nsubnet %s netmask %s {\n",kattach_net_parseip(kattach_vbridge.vbridge[vbrindex].vsubnet),kattach_net_parseip(vmask));
		fprintf(stream,"\trange %s %s;\n",kattach_net_parseip((kattach_vbridge.vbridge[vbrindex].vsubnet + 0x1)), kattach_net_parseip((vbcast - 0x1)));
		fprintf(stream,"\toption routers %s;\n",kattach_net_parseip(kattach_vbridge.vbridge[vbrindex].vbrip));
		fprintf(stream,"\toption subnet-mask %s;\n",kattach_net_parseip(vmask));
		fprintf(stream,"\toption broadcast-address %s;\n",kattach_net_parseip(vbcast));
		fprintf(stream,"\toption ip-forwarding off;\n");
		fprintf(stream,"\tdefault-lease-time 86400;\n");
		fprintf(stream,"\tmax-lease-time 86400;\n\n");

		for (vmpindex = 0; vmpindex < kattach_vmports.index; vmpindex++) {
			if (kattach_vmports.vmports[vmpindex].vbridge != vbrindex) continue;
			fprintf(stream,"\thost vguest-%u {\n",vmpindex);
			fprintf(stream,"\t\thardware ethernet %02x:%02x:%02x:%02x:%02x:%02x;\n", kattach_vmports.vmports[vmpindex].vmac[0],
								kattach_vmports.vmports[vmpindex].vmac[1], kattach_vmports.vmports[vmpindex].vmac[2],
								kattach_vmports.vmports[vmpindex].vmac[3], kattach_vmports.vmports[vmpindex].vmac[4],
								kattach_vmports.vmports[vmpindex].vmac[5]);
			if (kattach_vmports.vmports[vmpindex].vmpip) {
				fprintf(stream,"\t\tfixed-address %s;\n",kattach_net_parseip(kattach_vmports.vmports[vmpindex].vmpip));
			} else {
				u16 kmy_bridge = kattach_vmports.vmports[vmpindex].vbridge;
				kattach_vmports.vmports[vmpindex].vmpip = kattach_net_ip_assign(kattach_vbridge.vbridge[kmy_bridge].vsubnet,
												kattach_vbridge.vbridge[kmy_bridge].vmask);
				fprintf(stream,"\t\tfixed-address %s;\n",kattach_net_parseip(kattach_vmports.vmports[vmpindex].vmpip));
			}
			fprintf(stream,"\t}\n\n");
		}
		fprintf(stream,"\n}\n");
	}
	fclose(stream);
	sprintf(ccmd,"%s%s",KATTACH_APPQUEUE_DHCPPATH,KCMD_DHCPD);
	sprintf(lcmd,"-cf %s",dhcpdconf);
	if (!kattach_cfg.pid_dhcpd) {
		kattach_cfg.pid_dhcpd = kattach_bkexec(ccmd,lcmd);
	} else {
		res = kill(kattach_cfg.pid_dhcpd, SIGKILL);
		if (res) {
			printf("\n [!] FATAL - Unable to kill DHCP\n");
		} 
		kattach_cfg.pid_dhcpd = kattach_bkexec(ccmd,lcmd);
	}
	return;
}

void
kattach_vm_ldapd(void)
{
	/* LDAP stuff goes here */
	return;
}

u8
kattach_vm_dupmac(void)
{
	u32 vbrindex = 0;
	u16 vmpindex = 0;
	int m = 0, i = 0;

	/* check virtual bridges for duplicate macs */
	for (vbrindex = 0; vbrindex < kattach_vbridge.index; vbrindex++) {
		for (i = 0; i <= 5; i++) {
			if (kattach_genmac[i] != kattach_vbridge.vbridge[vbrindex].bmac[i]) {
				m = 0;
				break;
			} else {
				m++;
			}
		}
		if (m == 6) return 1;
	}

	/* check virtual ports for duplicate macs */
	m = 0;
	for (vmpindex = 0; vmpindex < kattach_vmports.index; vmpindex++) {
		for (i = 0; i <= 5; i++) {
			if (kattach_genmac[i] != kattach_vmports.vmports[vmpindex].vmac[i]) {
				m = 0;
				break;
			} else {
				m++;
			}
		}
		if (m == 6) return 1;
	}

	return 0;
}

void
kattach_vm_deploy(void)
{
	u32 index = 0;

	kattach_vm_new();			/* setup new VMs */
	kattach_vm_dhcpd();			/* regenerate dhcpd.conf and restart dhcpd */

	for (index = 0; index < kattach_vmst.index; index++) {
		if (kattach_vmst.vmsess[index].vmstatus != KATTACH_VM_STATUS_STARTUP)
			continue;
		kattach_vm_start(index);	/* start new VMs */
		kattach_vm_bradd(index);	/* add new VM to bridge */
	}
	return;
}

u8
kattach_vm_sql_iu(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_empty, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_empty, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_empty, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] INSERT or UPDATE failure %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_dupip(u32 genip)
{
	u32 index = 0;

	for (index = 0; index < kattach_vmports.index; index++) {
		if (genip != kattach_vmports.vmports[index].vmpip) continue;
		return (RC_OK);			/* duplicate found */
	}

	for (index = 0; index < kattach_vbridge.index; index++) {
		if (genip != kattach_vbridge.vbridge[index].vbrip) continue;
		return (RC_OK);			/* duplicate found */
	}

	return(RC_FAIL);			/* no duplicatess found */

}

static int 
kattach_vm_cb_empty(void *NotUsed, int cols, char **results, char **colname)
{
	return 0;
}

void
kattach_vm_monitor(void)
{
	u32 index = 0;
	char procpath[32];
	struct stat k_procstatus;
	int x = 0, y = 0;
	u8 rc = 0;

	for (index = 0; index < kattach_vmst.index; index++) {
		if (kattach_vmst.vmsess[index].vmstatus != KATTACH_VM_STATUS_RUNNING) continue;
		memset(procpath,0,sizeof(procpath));
		sprintf(procpath,"%s/%d",KATTACH_PROCPATH,kattach_vmst.vmsess[index].vpid);
		x = stat(procpath,&k_procstatus);

		if (x < 0) {
			/* this vm is not running, restart it */
			kattach_vm_start(index);
			y++;
			/* FIXME: add stats stuff here */
		}	
	}

	if (y > 0) {
		rc = kattach_sys_shm_setsync_vmst();
		/* FIXME: error processing */
	}
	return;
}

void
kattach_vm_priority_update(u32 index)
{
	char sqlq[1024];
	char lcmd[64];
	char ccmd[64];
	int xpid = 0;
	u8 rc = 0;

	/* execute renice in the background */
	sprintf(ccmd,"%s%s",KATTACH_BINPATH,KCMD_RENICE);
	sprintf(lcmd,"%d -p %u",(int)((kattach_vmst.vmsess[index].priority) - 19), kattach_vmst.vmsess[index].vpid);
	xpid = kattach_bkexec(ccmd,lcmd);

	/* update the database */
	sprintf(sqlq,"UPDATE vmsess SET priority = %u WHERE vmindex = '%lu';", kattach_vmst.vmsess[index].priority, (1 + index));
	rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
}

void 
kattach_vm_start(u32 index)
{
	u32 netx = 0, vmimage = 0, aindex = 0, vmidx = 0;
	u16 vmport = 0, vmbridge = 0;
	u8 rc = 0, vmpidx = 0, appidx = 0, kwaitcnt = 0;
	int xstat = 0;
	char lcmd[2048];
        char netcmd[64];
        char ecmd[256];
	char sqlq[1024];
	char ccmd[64];
	struct stat k_procstatus;

	if ((kattach_vmst.vmsess[index].vmstatus == KATTACH_VM_STATUS_DELETED) ||
		(kattach_vmst.vmsess[index].vmstatus == KATTACH_VM_STATUS_DISABLED))
			return;

	/* execute qemu */
	sprintf(ccmd,"%s/%s",KATTACH_HVPATH,KCMD_QEMU);
        sprintf(lcmd,"-enable-kvm -boot c -m %d -smp %d",kattach_vmst.vmsess[index].vmem,
		kattach_vmst.vmsess[index].vcpu);
        if (kattach_vmst.vmsess[index].vmpidx > KATTACH_MAX_VPORTS) {
        	vmpidx = KATTACH_MAX_VPORTS;
        } else {
		vmpidx = kattach_vmst.vmsess[index].vmpidx;
        }
        for (netx = 0; netx < vmpidx; netx++) {
        	vmport = kattach_vmst.vmsess[index].vmport[netx];
                vmbridge = kattach_vmports.vmports[vmport].vbridge;
                sprintf(netcmd," -net nic,vlan=%u,macaddr=%02x:%02x:%02x:%02x:%02x:%02x,model=virtio",
                	kattach_vbridge.vbridge[vmbridge].vlan, kattach_vmports.vmports[vmport].vmac[0],
                        kattach_vmports.vmports[vmport].vmac[1], kattach_vmports.vmports[vmport].vmac[2],
                        kattach_vmports.vmports[vmport].vmac[3], kattach_vmports.vmports[vmport].vmac[4],
                        kattach_vmports.vmports[vmport].vmac[5]);
                strncat(lcmd,netcmd,strlen(netcmd));
		memset(netcmd,0,sizeof(netcmd));
                sprintf(netcmd," -net tap,ifname=vmtap%u,script=no,vlan=%u ",vmport,kattach_vbridge.vbridge[vmbridge].vlan);
                strncat(lcmd,netcmd,strlen(netcmd));
		memset(netcmd,0,sizeof(netcmd));
	}
        /* disk images */
        vmimage = kattach_vmst.vmsess[index].vmimage;
        vmport = kattach_vmst.vmsess[index].vmport[0];
        sprintf(ecmd," -drive file=%svdisk-%s.%s,if=virtio,boot=on ",KATTACH_APPQUEUE_VMDISKS,kattach_vmimages.vmimage[vmimage].vminame, KATTACH_VDISKEXT);
        strncat(lcmd,ecmd,strlen(ecmd));
	memset(ecmd,0,sizeof(ecmd));
        sprintf(ecmd," -drive file=%s%02x-%02x-%02x-%02x-%02x-%02x.%s,if=virtio ",KATTACH_APPQUEUE_CFGPATH,kattach_vmports.vmports[vmport].vmac[0],
        	kattach_vmports.vmports[vmport].vmac[1], kattach_vmports.vmports[vmport].vmac[2],
                kattach_vmports.vmports[vmport].vmac[3], kattach_vmports.vmports[vmport].vmac[4],
                kattach_vmports.vmports[vmport].vmac[5], KATTACH_VDISKCFG);
        strncat(lcmd,ecmd,strlen(ecmd));
	memset(ecmd,0,sizeof(ecmd));
        sprintf(ecmd," -vnc 0.0.0.0:%lu -kernel %s%s -append  kaos=9:vda1:%02x%02x%02x%02x%02x%02x:0", index,
        	KATTACH_VKAOS_KERNELPATH, KATTACH_VKAOS_KERNEL, kattach_vmports.vmports[vmport].vmac[0],
                kattach_vmports.vmports[vmport].vmac[1], kattach_vmports.vmports[vmport].vmac[2],
                kattach_vmports.vmports[vmport].vmac[3], kattach_vmports.vmports[vmport].vmac[4],
                kattach_vmports.vmports[vmport].vmac[5]);
        strncat(lcmd,ecmd,strlen(ecmd));
	memset(ecmd,0,sizeof(ecmd));
	if (kattach_vmst.vmsess[index].vpid == 0) {
		kattach_vmst.vmsess[index].vpid = kattach_bkexec(ccmd,lcmd);			/* this probably isn't nice!  */
	} else {
		/* FIXME: need to stat here, see launch all */
		printf("\n [!] WARNING: Virtual Guest %lu already running as %u.\n",(index+1), kattach_vmst.vmsess[index].vpid);
	}
	kattach_vmst.vmsess[index].vmstatus = KATTACH_VM_STATUS_RUNNING;
	kattach_vmst.vmsess[index].vmoper = KATTACH_VM_STATUS_RUNNING;

	rc = (u8) usleep(KATTACH_TIMER_QEMU_SPINUP);

	for (netx = 0; netx < vmpidx; netx++) {
		vmport = kattach_vmst.vmsess[index].vmport[netx];
		vmbridge = kattach_vmports.vmports[vmport].vbridge;
		sprintf(ecmd,"%s%svmtap%u/operstate",KATTACH_SYSPATH,KATTACH_SYSNETPATH,vmport);
		kwaitcnt = 0;
		while ((xstat = stat(ecmd,&k_procstatus))) {
			kwaitcnt++;
			rc = usleep(KATTACH_TIMER_QEMU_SPINUP);
			if (kwaitcnt > 40) {
				/* waited about 20 seconds, no dice, probably not a good sign */
				kwaitcnt = 0xee;
				break;
			}
		}
		if (kwaitcnt == 0xee) {
			/* FIXME: probably need a CRASHED status */
			kattach_vmst.vmsess[index].vmstatus = KATTACH_VM_STATUS_STOPPED;
			kattach_vmst.vmsess[index].vmoper = KATTACH_VM_STATUS_STOPPED;
			kattach_vmst.vmsess[index].vpid = 0;
		} else {
			memset(ecmd,0,sizeof(ecmd));
			sprintf(ecmd,"%s%s link set vmtap%u up", KATTACH_BINPATH,KCMD_IP,vmport);
			kattach_sysexec(ecmd);
			memset(ecmd,0,sizeof(ecmd));
			sprintf(ecmd,"%s%s addif vbr%u vmtap%u",KATTACH_SBINPATH,KCMD_BRCTL,kattach_vbridge.vbridge[vmbridge].vlan, vmport);
			kattach_sysexec(ecmd);
			memset(ecmd,0,sizeof(ecmd));
			sprintf(ecmd,"%s%s %d -p %u",KATTACH_BINPATH,KCMD_RENICE,(int)((kattach_vmst.vmsess[index].priority) - 19),kattach_vmst.vmsess[index].vpid);
			kattach_sysexec(ecmd);
		}
	}

	sprintf(sqlq,"UPDATE vmsess SET vmstatus = %u WHERE vmindex = '%lu';", kattach_vmst.vmsess[index].vmstatus, (1 + index));
	rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);

	/* update appindex */
	vmidx = kattach_vmst.vmsess[index].vmimage;
	for (appidx = 0; appidx < kattach_vmimages.vmimage[vmidx].appi; appidx++) {
		aindex = kattach_vmimages.vmimage[vmidx].appindex[appidx];
		kattach_appmods.appmodules[aindex].deployed++;
	}

	return;
}

void
kattach_vm_stop(u32 index)
{
	u32 netx = 0, vmidx = 0, aindex = 0;
	u16 vmport = 0, vmbridge = 0;
	u8 appidx = 0;
	int res = 0;

	res = kill(kattach_vmst.vmsess[index].vpid, SIGKILL);			/* FIXME: This is evil - TBF in 0.7.0! */ 
	if (res < 0) {
		char lcmd[32];
		sprintf(lcmd,"%s/%s -9 %u",KATTACH_BINPATH,KCMD_KILL,kattach_vmst.vmsess[index].vpid);
		kattach_sysexec(lcmd);
	}
	kattach_vmst.vmsess[index].vpid = 0;
	kattach_vmst.vmsess[index].vmstatus = KATTACH_VM_STATUS_STOPPED;
	kattach_vmst.vmsess[index].vmoper = kattach_vmst.vmsess[index].vmstatus;

	for (netx = 0; netx < kattach_vmst.vmsess[index].vmpidx; netx++) {
		vmport = kattach_vmst.vmsess[index].vmport[netx];
		vmbridge = kattach_vmports.vmports[vmport].vbridge;
		kattach_vm_bridge_delif(vmport, kattach_vbridge.vbridge[vmbridge].vlan);
	}

        /* update appindex */
        vmidx = kattach_vmst.vmsess[index].vmimage;
        for (appidx = 0; appidx < kattach_vmimages.vmimage[vmidx].appi; appidx++) {
                aindex = kattach_vmimages.vmimage[vmidx].appindex[appidx];
                kattach_appmods.appmodules[aindex].deployed--;
        }

	return;
}

void
kattach_vm_bradd(u32 index)
{
	u32 netx = 0;
	u16 vmport = 0, vmbridge = 0;

	for (netx = 0; netx < kattach_vmst.vmsess[index].vmpidx; netx++) {
                vmport = kattach_vmst.vmsess[index].vmport[netx];
                vmbridge = kattach_vmports.vmports[vmport].vbridge;
                kattach_vm_bridge_addif(vmport, kattach_vbridge.vbridge[vmbridge].vlan);
        }
	return;
}

void
kattach_vm_new(void)
{
	u32 index = 0;
	u8 rc = 0;

	for (index = 0; index < kattach_vmst.index; index++) {
		if (kattach_vmst.vmsess[index].vmstatus != KATTACH_VM_STATUS_NEW) continue;
		kattach_vmst.vmsess[index].vmstatus = KATTACH_VM_STATUS_STARTUP;
		kattach_vmst.vmsess[index].vmoper = KATTACH_VM_STATUS_STARTUP;
		kattach_vm_new_net(index);
		kattach_vm_new_cfgdisk(index);               /* generate configuration disk image for vKaOS */
	}

	rc = kattach_sys_shm_setsync_vbr();
	rc = kattach_sys_shm_setsync_vmp();
	rc = kattach_sys_shm_setsync_vmst();
	/* FIXME: error processing */

	return;
}

void
kattach_vm_new_net(u32 index)
{
	u32 vmpindex = 0, subnet = 0;
	u16 vbridge = 0;
	u8 y = 0, vmpidx = 0, rc = 0, portzero = 0, parsed = 0;
	char sqlq[1024];

	for (vmpindex = 0; vmpindex < kattach_vmports.index; vmpindex++) {
		if (kattach_vmports.vmports[vmpindex].vmst != index) continue;

		/* note: AppQueue sets vmst and vbridge values */
		vbridge = kattach_vmports.vmports[vmpindex].vbridge;

		if (kattach_vbridge.vbridge[vbridge].vpfree == 0) {
			/* VLAN has no free IPs. Abort */
			printf("\n\n [X] ERROR - VM %lu unable to join bridge %u (bridge full)\n", index, vbridge);
			continue;
		}

		if (kattach_vbridge.vbridge[vbridge].vlan == 0) {
			/* VLAN is not configured. Abort */
			printf("\n\n [#] WARNING - Virtual Bridge %u has no VLAN configured.\n", vbridge);
			continue;
		}

		parsed = 1;

		if (kattach_vmst.vmsess[index].vmport[0] == vmpindex) {
			/* this is port entry 0, already added by AppQueue */
			portzero = 1;
		} else {
			vmpidx = kattach_vmst.vmsess[index].vmpidx;
			if (vmpidx < KATTACH_MAX_VPORTS) {
				kattach_vmst.vmsess[index].vmport[vmpidx] = vmpindex;
				kattach_vmst.vmsess[index].vmpidx++;
			} /* FIXME: should flag this as maxed out */
		}

		/* generate a new mac address for this virtual port */
		kattach_net_genmac();
		for (y = 0; y <= 5; y++) {
			kattach_vmports.vmports[vmpindex].vmac[y] = kattach_genmac[y];
		}

		if (portzero) {
			/* set the name of the vm */
			sprintf(kattach_vmst.vmsess[index].vmname,"vkaos-%02x-%02x-%02x",kattach_vmports.vmports[vmpindex].vmac[3],
											kattach_vmports.vmports[vmpindex].vmac[4],
											kattach_vmports.vmports[vmpindex].vmac[5]);
		}

		/* do bridge stuff */
		subnet = kattach_vbridge.vbridge[vbridge].vsubnet;

		/* generate an IP for this virtual port */
		kattach_vmports.vmports[vmpindex].vmpip = kattach_net_ip_assign(subnet, kattach_vbridge.vbridge[vbridge].vmask);

		if (kattach_vmports.vmports[vmpindex].vmpip) {
			kattach_vbridge.vbridge[vbridge].vpfree--;
			kattach_vbridge.vbridge[vbridge].vbruse++;
		}

		/* update databases */
		memset(sqlq,0,sizeof(sqlq));
		sprintf(sqlq,"INSERT into vmport (vmacA,vmacB,vmacC,vmacD,vmacE,vmacF,vmowner,vbridge,vmpip) values (%u,%u,%u,%u,%u,%u,%lu,%u,%lu);",
			kattach_vmports.vmports[vmpindex].vmac[0],kattach_vmports.vmports[vmpindex].vmac[1],
			kattach_vmports.vmports[vmpindex].vmac[2],kattach_vmports.vmports[vmpindex].vmac[3],
			kattach_vmports.vmports[vmpindex].vmac[4],kattach_vmports.vmports[vmpindex].vmac[5],
			(kattach_vmports.vmports[vmpindex].vmst + 1),(kattach_vmports.vmports[vmpindex].vbridge + 1),
			kattach_vmports.vmports[vmpindex].vmpip);
		rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);

		if (portzero) {
			memset(sqlq,0,sizeof(sqlq));
			sprintf(sqlq,"INSERT into vmsess (vmstatus,vmimage,vmname,vmem,vcpu,vmport,priority) values (%u,%lu,'%s',%u,%u,%u,%u);",
				kattach_vmst.vmsess[index].vmstatus, (kattach_vmst.vmsess[index].vmimage + 1),
				kattach_vmst.vmsess[index].vmname, kattach_vmst.vmsess[index].vmem, kattach_vmst.vmsess[index].vcpu,
				((kattach_vmst.vmsess[index].vmport[0]) + 1),kattach_vmst.vmsess[index].priority);
			rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
			portzero = 0;
		}

		memset(sqlq,0,sizeof(sqlq));
		sprintf(sqlq,"UPDATE vbridge SET vpfree = %u WHERE vbrindex = '%u';", kattach_vbridge.vbridge[vbridge].vpfree, (vbridge + 1));
		rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);

		memset(sqlq,0,sizeof(sqlq));
		sprintf(sqlq,"UPDATE vbridge SET vbruse = %u WHERE vbrindex = '%u';", kattach_vbridge.vbridge[vbridge].vbruse, (vbridge + 1));
		rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);		

		if (vmpidx == KATTACH_MAX_VPORTS) {
			break;
		}
	}

	if (parsed) {
		rc = kattach_sys_shm_setsync_vmst();
		return;
	} else {
		kattach_vmst.vmsess[index].vmstatus = KATTACH_VM_STATUS_NEW;
		kattach_vmst.vmsess[index].vmoper = KATTACH_VM_STATUS_NEW;	
		printf("\n\n [!] FATAL - VM %lu not a member of any VLANs. Deployment aborted!\n",index);
		return;
	}

}

void
kattach_vm_new_vbridge(u16 vbridge)
{
	u32 subnet = 0, index = 0;
	u8 unmask = (32 - kattach_vbridge.vbridge[vbridge].vmask);
	u8 y = 0, rc = 0;
	char sqlq[512];
	char lcmd[255];

	kattach_vbridge.vbridge[vbridge].vpfree = ((unsigned long) pow(2,unmask) - 3);
	kattach_vbridge.vbridge[vbridge].vbruse = 0;

	if (kattach_vbridge.vbridge[vbridge].vbrlocal != KATTACH_NET_VLAN_NAT) {
		subnet = kattach_net_netaddr(kattach_vbridge.vbridge[vbridge].vbrip, kattach_vbridge.vbridge[vbridge].vmask);

		if (subnet != kattach_vbridge.vbridge[vbridge].vsubnet) {
			kattach_vbridge.vbridge[vbridge].vsubnet = subnet;		/* user is on crack */
		}
	} else {
		subnet = kattach_net_netaddr(kattach_vbridge.vbridge[vbridge].vbrip, kattach_vbridge.vbridge[vbridge].vmask);
	}

	kattach_net_genmac();
	for (y = 0; y <= 5; y++) {
		kattach_vbridge.vbridge[vbridge].bmac[y] = kattach_genmac[y];
	}

	kattach_vm_create_bridge(kattach_vbridge.vbridge[vbridge].vlan);
	kattach_vm_bridge_up(vbridge);
	if ((kattach_vbridge.vbridge[vbridge].vbrlocal == KATTACH_NET_VLAN_8021Q) ||
		(kattach_vbridge.vbridge[vbridge].vbrlocal == KATTACH_NET_VLAN_ROUTED)) {
		/* vconfig add eth0 254 */
		sprintf(lcmd,"%s%s add %s %u",KATTACH_SBINPATH,KCMD_VCONFIG,kattach_vbridge.vbridge[vbridge].vlanext,kattach_vbridge.vbridge[vbridge].vlan);
		kattach_sysexec(lcmd);

		/* ip link set eth0.254 up */
		sprintf(lcmd,"%s%s link set %s.%u up",KATTACH_BINPATH,KCMD_IP,kattach_vbridge.vbridge[vbridge].vlanext,
							kattach_vbridge.vbridge[vbridge].vlan);
		kattach_sysexec(lcmd);

		/* brctl addif vbr254 eth0.254 */
		sprintf(lcmd,"%s%s addif vbr%u %s.%u",KATTACH_SBINPATH,KCMD_BRCTL,kattach_vbridge.vbridge[vbridge].vlan,
							kattach_vbridge.vbridge[vbridge].vlanext, kattach_vbridge.vbridge[vbridge].vlan);
		kattach_sysexec(lcmd);
	}

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"INSERT into vbridge (vlan,vsubnet,vmask,vbrip,vbrlocal,vlanext,vpfree,vbruse) values (%u,%lu,%u,%lu,%u,'%s',%u,%u);",
		kattach_vbridge.vbridge[vbridge].vlan, kattach_vbridge.vbridge[vbridge].vsubnet, kattach_vbridge.vbridge[vbridge].vmask, kattach_vbridge.vbridge[vbridge].vbrip,
		kattach_vbridge.vbridge[vbridge].vbrlocal, kattach_vbridge.vbridge[vbridge].vlanext, kattach_vbridge.vbridge[vbridge].vpfree,
		kattach_vbridge.vbridge[vbridge].vbruse);

	rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);

	rc = kattach_sys_shm_setsync_vbr();
	/* FIXME: do error processing */

	index = kattach_fw.zones.index;
	kattach_fw.zones.index++;				/* FIXME: test for limits */
	sprintf(kattach_fw.zones.zone[index].name,"vlan_%u", kattach_vbridge.vbridge[vbridge].vlan);
	kattach_fw.zones.zone[index].vlan = kattach_vbridge.vbridge[vbridge].vlan;
	kattach_fw.zones.zone[index].nindex = 1;
	kattach_fw.zones.zone[index].node[0].ip = kattach_vbridge.vbridge[vbridge].vsubnet;
	kattach_fw.zones.zone[index].node[0].mask = kattach_vbridge.vbridge[vbridge].vmask;
	kattach_vm_fw_update_zones();

	/* FIXME: add automatic NAT rules here */
	/* FIXME: do we need better checks here?  */
	kattach_sys_shm_sync_fw();

	return;
}

void
kattach_vm_update_vbridge(void)
{
	u16 vbridge = 0;
	u8 rc = 0;
	char sqlq[512];

	for (vbridge = 0; vbridge < kattach_vbridge.index; vbridge++) {
		switch (kattach_vbridge.vbridge[vbridge].state) {
			case KATTACH_VBR_STATE_NEW:
				if (kattach_vbridge.vbridge[vbridge].vlan == 0) continue;
				kattach_vm_new_vbridge(vbridge);
				break;

			case KATTACH_VBR_STATE_ACTIVE:
				/* FIXME: add check to monitor bridge */
				break;

			case KATTACH_VBR_STATE_MODIFIED:
				kattach_vm_bridge_down(vbridge);
				kattach_vm_bridge_up(vbridge);
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE vbridge SET vsubnet = %lu WHERE vbrindex = '%u';",kattach_vbridge.vbridge[vbridge].vsubnet, (vbridge + 1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE vbridge SET vbrip = %lu WHERE vbrindex = '%u';",kattach_vbridge.vbridge[vbridge].vbrip, (vbridge + 1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE vbridge SET vlan = %u WHERE vbrindex = '%u';",kattach_vbridge.vbridge[vbridge].vlan, (vbridge + 1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE vbridge SET vmask = %u WHERE vbrindex = '%u';",kattach_vbridge.vbridge[vbridge].vmask, (vbridge + 1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE vbridge SET vpfree = %u WHERE vbrindex = '%u';",kattach_vbridge.vbridge[vbridge].vpfree, (vbridge + 1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE vbridge SET vbruse = %u WHERE vbrindex = '%u';",kattach_vbridge.vbridge[vbridge].vbruse, (vbridge + 1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE vbridge SET vbrlocal = %u WHERE vbrindex = '%u';",kattach_vbridge.vbridge[vbridge].vbrlocal, (vbridge + 1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE vbridge SET vlanext = '%s' WHERE vbrindex = '%u';",kattach_vbridge.vbridge[vbridge].vlanext, (vbridge + 1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
				break;

			case KATTACH_VBR_STATE_DELETED:
				kattach_vm_bridge_down(vbridge);
				kattach_vbridge.vbridge[vbridge].state = KATTACH_VBR_STATE_EMPTY;
				kattach_vbridge.vbridge[vbridge].vsubnet = 0;
				kattach_vbridge.vbridge[vbridge].vbrip = 0;
				kattach_vbridge.vbridge[vbridge].vlan = 0;
				kattach_vbridge.vbridge[vbridge].vmask = 0;
				kattach_vbridge.vbridge[vbridge].vpfree = 0;
				kattach_vbridge.vbridge[vbridge].vbrlocal = 0;
				kattach_vbridge.vbridge[vbridge].vlanext[0] = '\0';
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"DELETE from vbridge WHERE vbrindex='%u';",(vbridge + 1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
				rc = kattach_sys_shm_setsync_vbr();
				if (rc == RC_FAIL) {
					/* FIXME: do something to handle locking failures */
				}
				break;

			case KATTACH_VBR_STATE_DISABLED:
				kattach_vm_bridge_down(vbridge);
				break;	

			case KATTACH_VBR_STATE_EMPTY:
			default:
				break;

		}
	}
	return;
}

void
kattach_vm_check_vmst(void)
{
	u32 index = 0, vmpid = 0;
	u16 vmport = 0, vbridge = 0, vbcnt = 0;
	u8 rc = 0, i = 0, y = 0, cnt = 0, defib = 0;
	int res = 0, x = 0;
	struct stat k_procstatus;
	char ecmd[64];
	char sqlq[512];

	for (index = 0; index < kattach_vmst.index; index++) {
		if ((kattach_vmst.vmsess[index].vmstatus != KATTACH_VM_STATUS_NEW) && 
			(kattach_vmst.vmsess[index].vmstatus == kattach_vmst.vmsess[index].vmoper)) continue;

		switch(kattach_vmst.vmsess[index].vmoper) {
			case KATTACH_VM_STATUS_OP_DEPLOY:
				kattach_vmst.vmsess[index].vmstatus = KATTACH_VM_STATUS_NEW;
				kattach_vm_deploy();	
				break;

			case KATTACH_VM_STATUS_OP_GRPKILL:
			case KATTACH_VM_STATUS_OP_STOP:
				if ((kattach_vmst.vmsess[index].vmstatus == KATTACH_VM_STATUS_RUNNING) &&
					(kattach_vmst.vmsess[index].vpid > 0)) {
					kattach_vmst.vmsess[index].vmstatus = KATTACH_VM_STATUS_STOPPED;
					kattach_vm_stop(index);
				} else {
					kattach_vmst.vmsess[index].vmstatus = KATTACH_VM_STATUS_STOPPED;
				}
				break;

			case KATTACH_VM_STATUS_OP_RESTART:
				vmpid = kattach_vmst.vmsess[index].vpid;
				if ((kattach_vmst.vmsess[index].vmstatus == KATTACH_VM_STATUS_RUNNING) && (vmpid > 0)) {
					kattach_vmst.vmsess[index].vmstatus = KATTACH_VM_STATUS_STOPPED;
					kattach_vm_stop(index);
					/* wait for it to stop */
					sprintf(ecmd,"%s/%lu",KATTACH_PROCPATH,vmpid);
					while ((x = stat(ecmd,&k_procstatus)) == 0) {
						if (cnt > 20) {
							if (defib > 6) {
								printf("\n [!] WARNING - Unable to restart VM guest - %lu index - %lu\n",vmpid,index);
								break;		/* escape if we get REALLY stuck */
							}
							res = kill(vmpid, SIGKILL);
							cnt = 0;
							defib++;
						}
						rc = usleep(KATTACH_TIMER_QEMU_SPINUP);
						cnt++;
					}
				} else {
					kattach_vmst.vmsess[index].vmstatus = KATTACH_VM_STATUS_STOPPED;
				}
				kattach_vmst.vmsess[index].vmoper = KATTACH_VM_STATUS_OP_START;
				/* this is supposed to continue through to the START code -- no break.. */
				
			case KATTACH_VM_STATUS_OP_START:
				kattach_vmst.vmsess[index].vmstatus = KATTACH_VM_STATUS_STARTUP;
				kattach_vm_start(index);
				kattach_vm_bradd(index);
				break;

			case KATTACH_VM_STATUS_OP_REMOVE:
				vmpid = kattach_vmst.vmsess[index].vpid;
				if ((kattach_vmst.vmsess[index].vmstatus == KATTACH_VM_STATUS_RUNNING) && (vmpid > 0)) {
					kattach_vmst.vmsess[index].vmstatus = KATTACH_VM_STATUS_STOPPED;
					kattach_vm_stop(index);
				} else {
					kattach_vmst.vmsess[index].vmstatus = KATTACH_VM_STATUS_STOPPED;
				}
				kattach_vmst.vmsess[index].vmimage = 0;
				kattach_vmst.vmsess[index].vmem = 0;
				kattach_vmst.vmsess[index].vcpu = 0;
				for (i = 0; i < kattach_vmst.vmsess[index].vmpidx; i++) {
					vmport = kattach_vmst.vmsess[index].vmport[i];
					if (kattach_vmports.vmports[vmport].vmst != index) continue;		/* sanity check */
					vbridge = kattach_vmports.vmports[vmport].vbridge;
					if (kattach_vbridge.vbridge[vbridge].vbruse) {
						kattach_vbridge.vbridge[vbridge].vbruse--;
						kattach_vbridge.vbridge[vbridge].vpfree++;
						kattach_vbridge.vbridge[vbridge].state = KATTACH_VBR_STATE_MODIFIED;
						vbcnt++;
					}

					kattach_vmports.vmports[vmport].vmst = 0;
					kattach_vmports.vmports[vmport].vmpip = 0;
					kattach_vmports.vmports[vmport].vbridge = 0;
					memset(sqlq,0,sizeof(sqlq));
					sprintf(sqlq,"UPDATE vmport SET vmpip = %lu WHERE vmpindex = '%u';", kattach_vmports.vmports[vmport].vmpip, (vmport + 1));
					rc =  kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
					memset(sqlq,0,sizeof(sqlq));
					sprintf(sqlq,"UPDATE vmport SET vmowner = %lu WHERE vmpindex = '%u';", kattach_vmports.vmports[vmport].vmst, (vmport + 1));
					rc =  kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
					memset(sqlq,0,sizeof(sqlq));
					sprintf(sqlq,"UPDATE vmport SET vbridge = %u WHERE vmpindex = '%u';", kattach_vmports.vmports[vmport].vbridge, (vmport + 1));
					rc =  kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);

					for (y = 0; y <= 5; y++) {
						kattach_vmports.vmports[vmport].vmac[y] = 0;
						memset(sqlq,0,sizeof(sqlq));
						if (y == 0) sprintf(sqlq,"UPDATE vmport SET vmacA = %u WHERE vmpindex = '%u';", kattach_vmports.vmports[vmport].vmac[y], (vmport + 1));
						if (y == 1) sprintf(sqlq,"UPDATE vmport SET vmacB = %u WHERE vmpindex = '%u';", kattach_vmports.vmports[vmport].vmac[y], (vmport + 1));
						if (y == 2) sprintf(sqlq,"UPDATE vmport SET vmacC = %u WHERE vmpindex = '%u';", kattach_vmports.vmports[vmport].vmac[y], (vmport + 1));
						if (y == 3) sprintf(sqlq,"UPDATE vmport SET vmacD = %u WHERE vmpindex = '%u';", kattach_vmports.vmports[vmport].vmac[y], (vmport + 1));
						if (y == 4) sprintf(sqlq,"UPDATE vmport SET vmacE = %u WHERE vmpindex = '%u';", kattach_vmports.vmports[vmport].vmac[y], (vmport + 1));
						if (y == 5) sprintf(sqlq,"UPDATE vmport SET vmacF = %u WHERE vmpindex = '%u';", kattach_vmports.vmports[vmport].vmac[y], (vmport + 1));
						rc =  kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
					}
					kattach_vmst.vmsess[index].vmport[i] = 0;
				}
				kattach_vmst.vmsess[index].vmpidx = 0;
				kattach_vmst.vmsess[index].vmstatus = KATTACH_VM_STATUS_DELETED;
				kattach_vmst.vmsess[index].vmoper = KATTACH_VM_STATUS_DELETED;
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE vmsess SET vmstatus = %u WHERE vmindex = '%lu';", kattach_vmst.vmsess[index].vmstatus, (index+1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE vmsess SET vmimage = %u WHERE vmindex = '%lu';", 0, (index+1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE vmsess SET vmname = 'NULL' WHERE vmindex = '%lu';", (index+1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE vmsess SET vmport = %u WHERE vmindex = '%lu';", 0, (index+1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);				
				break;

			default:
				break;
			
		}

	}
	if (vbcnt) {
		kattach_vm_update_vbridge();			/* update bridge database */
		rc = kattach_sys_shm_setsync_vmp();		/* sync back the vmp */
		rc = kattach_sys_shm_setsync_vbr();		/* sync back the vbr */
	}
	rc = kattach_sys_shm_setsync_vmst();
	return;	
}

void
kattach_vm_appmods(void)
{
	u32 index = 0;
	char sqlq[512];
	u8 rc = 0;

	for (index = 0; index < kattach_appmods.index; index++) {
		if (kattach_appmods.appmodules[index].state == CM_APP_M_STATE_UNCHANGED) continue;

		switch (kattach_appmods.appmodules[index].state) {
			case CM_APP_M_STATE_NEW:
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"INSERT into appmodule (name,vendor,vurl,version,release,srctree,buildinfo,chksum,license,filename,revision,latest) values ('%s',%lu,'%s','%s','%s',%u,'%s','%s',%u,'%s',%u,%u);",
									kattach_appmods.appmodules[index].name, kattach_appmods.appmodules[index].vendor_id,
									kattach_appmods.appmodules[index].url, kattach_appmods.appmodules[index].version,
									kattach_appmods.appmodules[index].release, kattach_appmods.appmodules[index].srctree,
									kattach_appmods.appmodules[index].buildinfo, kattach_appmods.appmodules[index].chksum,
									kattach_appmods.appmodules[index].license, kattach_appmods.appmodules[index].filename,
									kattach_appmods.appmodules[index].revision, kattach_appmods.appmodules[index].latest);
				rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);
				kattach_appmods.appmodules[index].state = CM_APP_M_STATE_UNCHANGED;
				break;

			case CM_APP_M_STATE_DELETED:
				kattach_appmods.appmodules[index].name[0] = '\0';
				kattach_appmods.appmodules[index].vendor_id = 0;
				kattach_appmods.appmodules[index].url[0] = '\0';
				kattach_appmods.appmodules[index].version[0] = '\0';
				kattach_appmods.appmodules[index].release[0] = '\0';
				kattach_appmods.appmodules[index].srctree = 0;
				kattach_appmods.appmodules[index].buildinfo[0] = '\0';
				kattach_appmods.appmodules[index].chksum[0] = '\0';
				kattach_appmods.appmodules[index].license = 0;
				kattach_appmods.appmodules[index].filename[0] = '\0';
				kattach_appmods.appmodules[index].revision = 0;
				kattach_appmods.appmodules[index].latest = 0;

			case CM_APP_M_STATE_MODIFIED:
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE appmodule SET name = '%s' WHERE appindex = '%lu';",kattach_appmods.appmodules[index].name, (index+1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);

				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE appmodule SET vendor = %lu WHERE appindex = '%lu';",kattach_appmods.appmodules[index].vendor_id, (index+1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);

				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE appmodule SET vurl = '%s' WHERE appindex = '%lu';",kattach_appmods.appmodules[index].url, (index+1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);

				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE appmodule SET version = '%s' WHERE appindex = '%lu';",kattach_appmods.appmodules[index].version, (index+1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);

				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE appmodule SET release = '%s' WHERE appindex = '%lu';",kattach_appmods.appmodules[index].release, (index+1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);

				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE appmodule SET srctree = %u WHERE appindex = '%lu';",kattach_appmods.appmodules[index].srctree, (index+1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);

				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE appmodule SET buildinfo = '%s' WHERE appindex = '%lu';",kattach_appmods.appmodules[index].buildinfo, (index+1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);

				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE appmodule SET chksum = '%s' WHERE appindex = '%lu';",kattach_appmods.appmodules[index].chksum, (index+1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);

				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE appmodule SET license = %u WHERE appindex = '%lu';",kattach_appmods.appmodules[index].license, (index+1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);

				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE appmodule SET filename = '%s' WHERE appindex = '%lu';",kattach_appmods.appmodules[index].filename, (index+1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);

				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE appmodule SET revision = %u WHERE appindex = '%lu';",kattach_appmods.appmodules[index].revision, (index+1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);

				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE appmodule SET latest = %u WHERE appindex = '%lu';",kattach_appmods.appmodules[index].latest, (index+1));
				rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);

				kattach_appmods.appmodules[index].state = CM_APP_M_STATE_UNCHANGED;
				break;


			default:
				break;

		}		
	}
	kattach_sys_shm_setsync_appmods();			/* sync back changes */
	return;
}

void
kattach_vm_vmi_update(void)
{
	FILE *stream;
	u32 index = 0, appindex = 0;
	u16 disksize = 0;
	u8 aindex = 0, rc = 0, wtf = 0;
	char rawdisk[64];
	char lcmd[255];
	char fsuuid[40];
	char sqlq[512];
	int ret = 0;

	for (index = 0; index < kattach_vmimages.index; index++) {
		if (strlen(kattach_vmimages.vmimage[index].vminame) == 0) continue;
		if (!kattach_vmimages.vmimage[index].changed) continue;
		disksize = 0;									/* in MB */

		if (kattach_vmimages.vmimage[index].active == 0) {
			/* entry has been deleted */
			memset(sqlq,0,sizeof(sqlq));
			sprintf(sqlq,"DELETE from vmimage WHERE vmindex = %lu;", (1 + index));
			rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);
			kattach_vmimages.vmimage[index].vminame[0] = '\0';
			memset(sqlq,0,sizeof(sqlq));
			sprintf(sqlq,"DELETE from vmapps WHERE vmindex = %lu;", (1 + index));
			rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);
			kattach_vmimages.vmimage[index].appi = 0;
			/* FIXME: should clean up filesystem here */
		} else {
			/* FIXME: stat file, if it exists, back it up to vdisk-vmi.kvd_unixtime */

			sprintf(rawdisk,"vdisk-%s.ext4",kattach_vmimages.vmimage[index].vminame);
	
			for (aindex = 0; aindex < kattach_vmimages.vmimage[index].appi; aindex++) {
				appindex = kattach_vmimages.vmimage[index].appindex[aindex];
				if (kattach_appmods.appmodules[appindex].app_size) {
					disksize += kattach_appmods.appmodules[appindex].app_size;
				} else {
					disksize += 2;
				}
				kattach_appmods.appmodules[appindex].config++;
			}

			disksize += 10;			/* FIXME: Add a 10MB cushion, this might be unnecessary */

			/* execute dd to create the raw disk image */
			sprintf(lcmd,"%s%s if=/dev/zero of=%s%s bs=1M count=%u",KATTACH_BINPATH,KCMD_DD,KATTACH_APPQUEUE_RAWDISKS,rawdisk,disksize);
			kattach_sysexec(lcmd);

			/* losetup to the reserved loopback device */
			sprintf(lcmd,"%s%s %s %s%s",KATTACH_SBINPATH,KCMD_LOSETUP,KATTACH_RESERVED_LOOPBACK,KATTACH_APPQUEUE_RAWDISKS,rawdisk);
			kattach_sysexec(lcmd);

			/* generate a UUID for this disk image */
			kattach_getruuid();
			sprintf(fsuuid,"%s",kattach_ruuid);

			/* FIXME: should store the UUID in the structure and DB for future use */

			/* mkfs the image */
			sprintf(lcmd,"%s%s -q -U %s -t %s -j -m0 -L vkaos.%s %s",KATTACH_SBINPATH, KCMD_MKFS, fsuuid, KATTACH_FS_APPQUEUE, 
											kattach_vmimages.vmimage[index].vminame, KATTACH_RESERVED_LOOPBACK);
			kattach_sysexec(lcmd);

			/* mount new raw disk image */
			ret = mount(KATTACH_RESERVED_LOOPBACK,KATTACH_APPQUEUE_RAWMOUNT,KATTACH_FS_APPQUEUE, MS_RELATIME, KATTACH_FS_EXT4);

			memset(sqlq,0,sizeof(sqlq));
			sprintf(sqlq,"INSERT into vmimage values (%lu,%u,'%s',%lu);",(1 + index), kattach_vmimages.vmimage[index].active,
											kattach_vmimages.vmimage[index].vminame, (kattach_vmimages.vmimage[index].appindex[0] + 1));
			rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);

			/* open vkaos_launch */
			stream = fopen(KATTACH_APPQUEUE_VKLPATH,"w");

			if (stream == (FILE *)0) {
				printf("\n [!] WARNING: Unable to write launch script for %s\n", kattach_vmimages.vmimage[index].vminame);
				wtf = 0;
			} else {
				wtf = 1;
				fprintf(stream,"#!/bin/sh\n\n");
			}

			/* copy over contents of appmodules */
			for (aindex = 0; aindex < kattach_vmimages.vmimage[index].appi; aindex++) {
				appindex = kattach_vmimages.vmimage[index].appindex[aindex];

				/* losetup the aqi image */
				/* /appq/am/<module>/cm/filename */
				sprintf(lcmd,"%s%s %s %s%s/%s/images/%s",KATTACH_SBINPATH,KCMD_LOSETUP,KATTACH_RESERVED_LOOPBACK_AQI,KATTACH_APPQUEUE_APPMOD,
								KATTACH_APPQUEUE_VENDOR_CM,
								kattach_appmods.appmodules[appindex].name,
								kattach_appmods.appmodules[appindex].filename);
				kattach_sysexec(lcmd);

				ret = mount(KATTACH_RESERVED_LOOPBACK_AQI,KATTACH_APPQUEUE_AQIPATH,KATTACH_FS_SQUASHFS,MS_RELATIME,"");

				/* copy the contents */
				sprintf(lcmd,"%s%s -a %s%c %s",KATTACH_BINPATH,KCMD_CP,KATTACH_APPQUEUE_AQIPATH,'*',
					KATTACH_APPQUEUE_RAWMOUNT);
				kattach_sysexec(lcmd);
			
				/* FIXME: This is a hack for 0.6.0, copies over files from cfggroup raw directory */
				sprintf(lcmd,"%s%s -a %s%s/raw/* %s%s/",KATTACH_BINPATH,KCMD_CP,KATTACH_APPQUEUE_APPMODCFG,
					kattach_cfggrp.cfggrp[kattach_vmimages.vmimage[index].cfggrp[aindex]].name,
					KATTACH_APPQUEUE_RAWMOUNT, kattach_appmods.appmodules[appindex].name);
				kattach_sysexec(lcmd);

				sprintf(lcmd,"%s%s %s",KATTACH_BINPATH,KCMD_UMOUNT,KATTACH_RESERVED_LOOPBACK_AQI);
				kattach_sysexec(lcmd);

				/* unplug loopback */
				sprintf(lcmd,"%s%s -d %s",KATTACH_SBINPATH,KCMD_LOSETUP,KATTACH_RESERVED_LOOPBACK_AQI);
				kattach_sysexec(lcmd);

				/* update database */
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"INSERT into vmapps (appindex, vmindex, cfgindex) values (%lu,%lu,%lu);", (1 + appindex), (1 + index), (1 + kattach_vmimages.vmimage[index].cfggrp[aindex]));
				rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);

				if (wtf) {
					fprintf(stream,"/sbin/chroot /kaos/apps/%s /launch\n", kattach_appmods.appmodules[appindex].name);
				}
			}

			if (wtf) {
				fclose(stream);
				wtf = 0;
				ret = chmod(KATTACH_APPQUEUE_VKLPATH, KATTACH_PERM);
			}

			/* umount raw disk image */
			sprintf(lcmd,"%s%s %s",KATTACH_BINPATH,KCMD_UMOUNT,KATTACH_RESERVED_LOOPBACK);
			kattach_sysexec(lcmd);

			/* unplug loopback */
			sprintf(lcmd,"%s%s -d %s",KATTACH_SBINPATH,KCMD_LOSETUP,KATTACH_RESERVED_LOOPBACK);
			kattach_sysexec(lcmd);

			/* convert image */
			sprintf(lcmd,"%s/%s convert -f raw %s%s -O qcow2 %svdisk-%s.%s",KATTACH_HVPATH,KCMD_QEMUIMG,KATTACH_APPQUEUE_RAWDISKS,rawdisk,
											KATTACH_APPQUEUE_VMDISKS,kattach_vmimages.vmimage[index].vminame,
											KATTACH_VDISKEXT);
			kattach_sysexec(lcmd);

			/* FIXME: We should delete the raw image here, but for 0.6.0 and debugging we will leave it there */

		}
		kattach_vmimages.vmimage[index].changed = 0;
	}

	rc = kattach_sys_shm_setsync_appmods();	
	return;
}

void
kattach_vm_new_cfgdisk(u32 index)
{
	u16 vmport = kattach_vmst.vmsess[index].vmport[0];
	char lcmd[255];
	char vdiskcfg[255];
	char fsuuid[40];
	u8 disksize = 4;			/* FIXME: this should NOT be hardcoded!! */

	sprintf(vdiskcfg,"%s%02x-%02x-%02x-%02x-%02x-%02x.%s",KATTACH_APPQUEUE_CFGPATH,kattach_vmports.vmports[vmport].vmac[0],
								kattach_vmports.vmports[vmport].vmac[1], kattach_vmports.vmports[vmport].vmac[2],
								kattach_vmports.vmports[vmport].vmac[3], kattach_vmports.vmports[vmport].vmac[4],
								kattach_vmports.vmports[vmport].vmac[5], KATTACH_VDISKCFG);

	/* execute dd to create the raw disk image */
	sprintf(lcmd,"%s%s if=/dev/zero of=%s_raw bs=1M count=%u",KATTACH_BINPATH,KCMD_DD,vdiskcfg,disksize);
	kattach_sysexec(lcmd);

	/* losetup to the reserved loopback device */
	sprintf(lcmd,"%s%s %s %s_raw",KATTACH_SBINPATH,KCMD_LOSETUP,KATTACH_RESERVED_LOOPBACK,vdiskcfg);
	kattach_sysexec(lcmd);

	/* generate a UUID for this disk image */
	kattach_getruuid();
	sprintf(fsuuid,"%s",kattach_ruuid);

	/* FIXME: should store the UUID in the structure and DB for future use */

	/* mkfs the image */
	sprintf(lcmd,"%s%s -q -U %s -t %s -j -m0 -L vkaos.%s %s",KATTACH_SBINPATH, KCMD_MKFS, fsuuid, KATTACH_FS_APPQUEUE,
									"vkaos.cfg", KATTACH_RESERVED_LOOPBACK);
	kattach_sysexec(lcmd);

	/* FIXME: should generate the config for the app here!! */
	
	/* unplug loopback */
	sprintf(lcmd,"%s%s -d %s",KATTACH_SBINPATH,KCMD_LOSETUP,KATTACH_RESERVED_LOOPBACK);
	kattach_sysexec(lcmd);

	/* convert the image */
	sprintf(lcmd,"%s/%s convert -f raw %s_raw -O qcow2 %s",KATTACH_HVPATH,KCMD_QEMUIMG,vdiskcfg,vdiskcfg);
	kattach_sysexec(lcmd);

	return;
}

void
kattach_vm_netdev_init(void)
{
	char entry[512];
	char lcmd[255];
	char fname[64];
	char nicinfo[32];
	char *nicval;
	char *delimit;
	char *devname;
	char c = '\0';
	char d = '\0';
	FILE *fp, *stream;
	u16 index = 0, nindex = 0, espeed = 0, lacpidx = 0;
	u8 f = 0, l = 0, m = 0;
	int ret = 0;

	/* remove bond0 via sysfs */
	sprintf(lcmd,"%s%s -bond0 > %s%sbonding_masters",KATTACH_BINPATH,KCMD_ECHO,KATTACH_SYSPATH,KATTACH_SYSNETPATH);
	kattach_sysexec(lcmd);


	if (!(fp = fopen("/proc/net/dev", "r"))) {
		printf("\n\n [!] FATAL - Unable to open /proc/net/dev initialization failed\n");
		return;
	}

	while ((devname = fgets(entry, 512, fp))) {
		while(isspace(devname[0]))
			devname++;

		delimit = strchr (devname, ':');
		if (delimit) {
			*delimit = 0;
			if (devname[0] == '\0') continue;
			for (index = 0; index < kattach_netdev.index; index++) {
				if (kattach_netdev.pif[index].devname[0] == '\0') continue;
				if (strncmp(devname,kattach_netdev.pif[index].devname,strlen(devname))) continue;
				if (kattach_netdev.pif[index].status == KATTACH_LINK_STATUS_DELETED) continue;
				f = 1;
				if ((strlen(kattach_netdev.pif[index].devname) == 2) && (!strncmp(kattach_netdev.pif[index].devname,"lo",2))) {
					/* loopback -- special case */
					sprintf(kattach_netdev.pif[index].devname,"%s",devname);
					kattach_netdev.pif[index].ip = 0x7f000001;
					kattach_netdev.pif[index].gw = 0;
					kattach_netdev.pif[index].mask = 8;
					kattach_netdev.pif[index].mtu = 16436;
					kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_PSUEDO;
					kattach_netdev.pif[index].psuedo = 0;
					kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP;
					kattach_netdev.pif[index].lacpidx = 0;
					kattach_netdev.pif[index].pvid = 1;
					sprintf(lcmd,"%s%s link set dev %s mtu %u",KATTACH_BINPATH,KCMD_IP,devname,kattach_netdev.pif[index].mtu);
					kattach_sysexec(lcmd);
					sprintf(lcmd,"%s%s addr add %lu.%lu.%lu.%lu/%u dev %s",KATTACH_BINPATH,KCMD_IP,
									(kattach_netdev.pif[index].ip >> 24) & 0xff,
									(kattach_netdev.pif[index].ip >> 16) & 0xff,
									(kattach_netdev.pif[index].ip >> 8) & 0xff,
									(kattach_netdev.pif[index].ip) & 0xff, kattach_netdev.pif[index].mask, devname);
					kattach_sysexec(lcmd);
				} else if ((strncmp(devname,"lacp",4)) && (kattach_netdev.pif[index].psuedo == KATTACH_LINK_STATUS_LACP_NEW)) {
					/* this is an LACP psuedo device */
					if (kattach_netdev.pif[index].status != KATTACH_LINK_STATUS_LACP) {
						sprintf(lcmd,"%s%s +lacp%u > %s%sbonding_masters",KATTACH_BINPATH,KCMD_ECHO,kattach_netdev.pif[index].pvid,
													KATTACH_SYSPATH,KATTACH_SYSNETPATH);
						kattach_sysexec(lcmd);
						/* set mode to 802.3ad */
						sprintf(lcmd,"%s%s 802.3ad > %s%slacp%u/bonding/mode",KATTACH_BINPATH,KCMD_ECHO,
													KATTACH_SYSPATH,KATTACH_SYSNETPATH,
													kattach_netdev.pif[index].pvid);
						kattach_sysexec(lcmd);
						/* bring up 802.3ad device */
						sprintf(lcmd,"%s%s link set lacp%u up",KATTACH_BINPATH,KCMD_IP,kattach_netdev.pif[index].pvid);
						kattach_sysexec(lcmd);
					}
					sprintf(lcmd,"%s%s link set dev %s mtu %u",KATTACH_BINPATH,KCMD_IP,devname,kattach_netdev.pif[index].mtu);
					kattach_sysexec(lcmd);
					if (kattach_netdev.pif[index].ip == KATTACH_NET_HASDHCPIP) {
						kattach_hasdhcpif++;
						kattach_net_dhcp(kattach_netdev.pif[index].devname);
					} else {
						sprintf(lcmd,"%s%s addr add %lu.%lu.%lu.%lu/%u dev %s",KATTACH_BINPATH,KCMD_IP,
										(kattach_netdev.pif[index].ip >> 24) & 0xff,
										(kattach_netdev.pif[index].ip >> 16) & 0xff,
										(kattach_netdev.pif[index].ip >> 8) & 0xff,
										(kattach_netdev.pif[index].ip) & 0xff, kattach_netdev.pif[index].mask, devname);
						kattach_sysexec(lcmd);
					}
					kattach_netdev.pif[lacpidx].status = KATTACH_LINK_STATUS_UP;
				} else if (kattach_netdev.pif[index].pvid == 0x8023) {
					kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_LACP;
					lacpidx = kattach_netdev.pif[index].lacpidx;
					if (lacpidx > index) {
						sprintf(lcmd,"%s%s +lacp%u > %s%sbonding_masters",KATTACH_BINPATH,KCMD_ECHO,kattach_netdev.pif[lacpidx].pvid,
													KATTACH_SYSPATH,KATTACH_SYSNETPATH);
						kattach_sysexec(lcmd);
						/* set mode to 802.3ad */
						sprintf(lcmd,"%s%s 802.3ad > %s%slacp%u/bonding/mode",KATTACH_BINPATH,KCMD_ECHO,
													KATTACH_SYSPATH,KATTACH_SYSNETPATH,
													kattach_netdev.pif[lacpidx].pvid);
						kattach_sysexec(lcmd);
						/* bring up 802.3ad device */
						sprintf(lcmd,"%s%s link set lacp%u up",KATTACH_BINPATH,KCMD_IP,kattach_netdev.pif[lacpidx].pvid);
						kattach_sysexec(lcmd);

						kattach_netdev.pif[lacpidx].status = KATTACH_LINK_STATUS_LACP;
					}
					/* add interface to Link */
					sprintf(lcmd,"%s%s +%s > %s%slacp%u/bonding/slaves",KATTACH_BINPATH,KCMD_ECHO,kattach_netdev.pif[lacpidx].devname,
												KATTACH_SYSPATH,KATTACH_SYSNETPATH,
												kattach_netdev.pif[lacpidx].pvid);
					kattach_sysexec(lcmd);
					/* bring up interface */
					sprintf(lcmd,"%s%s link set %s up",KATTACH_BINPATH,KCMD_IP,kattach_netdev.pif[index].devname);
					kattach_sysexec(lcmd);
				} else if (kattach_netdev.pif[index].status != KATTACH_LINK_STATUS_DISABLED) {
					sprintf(fname,"%s%s%s/carrier",KATTACH_SYSPATH,KATTACH_SYSNETPATH,devname);
					stream = fopen(fname,"r");
					c = (char) fgetc(stream);
					fclose(stream);
					if (!strncmp(devname,"eth",3)) {
						/* FIXME: Need a better way to check */
						sprintf(fname,"%s%s%s/speed",KATTACH_SYSPATH,KATTACH_SYSNETPATH,devname);
						stream = fopen(fname,"r");
						nicval = fgets(nicinfo,32,stream);
						if (nicval != NULL) {
							espeed = (u16) atoi(nicval);
						} else {
							espeed = 0;
						}
						fclose(stream);
						sprintf(fname,"%s%s%s/duplex",KATTACH_SYSPATH,KATTACH_SYSNETPATH,devname);
						stream = fopen(fname,"r");
						d = (char) fgetc(stream);
						fclose (stream);
					} else {
						espeed = 100;
						d = 'f';
					}
					if (kattach_netdev.pif[index].mtu) {
						sprintf(lcmd,"%s%s link set dev %s up",KATTACH_BINPATH,KCMD_IP,devname);
						kattach_sysexec(lcmd);
						sprintf(lcmd,"%s%s link set dev %s mtu %u",KATTACH_BINPATH,KCMD_IP,devname,kattach_netdev.pif[index].mtu);
						kattach_sysexec(lcmd);
						if (kattach_netdev.pif[index].ip == KATTACH_NET_HASDHCPIP) {
							kattach_hasdhcpif++;
							kattach_net_dhcp(devname);
						} else {
							sprintf(lcmd,"%s%s addr add %lu.%lu.%lu.%lu/%u dev %s",KATTACH_BINPATH,KCMD_IP,
											(kattach_netdev.pif[index].ip >> 24) & 0xff,
											(kattach_netdev.pif[index].ip >> 16) & 0xff,
											(kattach_netdev.pif[index].ip >> 8) & 0xff,
											(kattach_netdev.pif[index].ip) & 0xff, kattach_netdev.pif[index].mask, devname);
							kattach_sysexec(lcmd);
						}
					}
					/* FIXME: should check operstate for up to be accurate */
					if (c == '0') {
						kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_DOWN;
					} else {
						if (d == 'h') {
							if (espeed >= 10000) {
								kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_H10000;
							} else if (espeed >= 1000) {
								kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_H1000;
							} else if (espeed >= 100) {
								kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_H100;
							} else {
								kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP;
							}
						} else {
							if (espeed >= 10000) {
								kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_10000;
							} else if (espeed >= 1000) {
								kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_1000;
							} else if (espeed >= 100) {
								kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_100;
							} else {
								kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP;
							}
						}
						if (!strncmp(devname,kattach_cfg.netdev,strlen(kattach_cfg.netdev)) && (kattach_netdev.pif[index].mtu)) {
							if (kattach_netdev.pif[index].ip != KATTACH_NET_HASDHCPIP) {
								sprintf(lcmd,"ip route add default via %lu.%lu.%lu.%lu dev %s",
										(kattach_netdev.pif[index].gw >> 24) & 0xff,
										(kattach_netdev.pif[index].gw >> 16) & 0xff,
										(kattach_netdev.pif[index].gw >> 8) & 0xff,
										(kattach_netdev.pif[index].gw) & 0xff, devname);
								kattach_sysexec(lcmd);
							}
						}
					}
					if ((strncmp(devname,"eth",3)) && (strncmp(devname,"wan",3)) && (strncmp(devname,"ib",2)) &&
						(strncmp(devname,"usb",3))) {
						kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_PSUEDO;
					} else if (!strncmp(devname,"usb",3)) {
						kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_USB;
					} else if (!strncmp(devname,"ib",2)) {
						kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_INFINIBAND;
					} else {
						if (espeed >= 10000) {
							kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_10GBE;
						} else if (espeed >= 1000) {
							kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_1GBE;
						} else {
							kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_ETHERNET;
						}
					}

				}
				break;
			}
			if (!f) {
				if ((strlen(devname) == 2) && (!strncmp(devname,"lo",2))) {
					/* loopback -- special case */
					nindex = kattach_netdev.index;
					kattach_netdev.index++;
					sprintf(kattach_netdev.pif[nindex].devname,"%s",devname);
					kattach_netdev.pif[nindex].ip = 0x7f000001;
					kattach_netdev.pif[nindex].gw = 0;
					kattach_netdev.pif[nindex].mask = 8;
					kattach_netdev.pif[nindex].mtu = 16436;
					kattach_netdev.pif[nindex].type = KATTACH_LINK_TYPE_PSUEDO;
					kattach_netdev.pif[nindex].psuedo = 0;
					kattach_netdev.pif[nindex].status = KATTACH_LINK_STATUS_UP;
					kattach_netdev.pif[nindex].lacpidx = 0;
					kattach_netdev.pif[nindex].pvid = 1;
					sprintf(lcmd,"%s%s link set dev %s mtu %u",KATTACH_BINPATH,KCMD_IP,devname,kattach_netdev.pif[nindex].mtu);
					kattach_sysexec(lcmd);
					sprintf(lcmd,"%s%s addr add %lu.%lu.%lu.%lu/%u dev %s",KATTACH_BINPATH,KCMD_IP,
									(kattach_netdev.pif[nindex].ip >> 24) & 0xff,
									(kattach_netdev.pif[nindex].ip >> 16) & 0xff,
									(kattach_netdev.pif[nindex].ip >> 8) & 0xff,
									(kattach_netdev.pif[nindex].ip) & 0xff, kattach_netdev.pif[nindex].mask, devname);
					kattach_sysexec(lcmd);
				} else if ((strncmp(devname,"vmtap",5)) || (strncmp(devname,"vbr",3))) {
					/* device is not our tap or bridging device */
					/* FIXME: add code to remove unused bonding devices via sysfs */
					nindex = kattach_netdev.index;
					kattach_netdev.index++;
					sprintf(kattach_netdev.pif[nindex].devname,"%s",devname);
					kattach_netdev.pif[nindex].ip = 0;
					kattach_netdev.pif[nindex].gw = 0;
					kattach_netdev.pif[nindex].mask = 0;
					kattach_netdev.pif[nindex].pvid = 0;
					kattach_netdev.pif[nindex].lacpidx = 0;
					kattach_netdev.pif[nindex].mtu = 0;
                                        sprintf(fname,"%s%s%s/carrier",KATTACH_SYSPATH,KATTACH_SYSNETPATH,devname);
                                        stream = fopen(fname,"r");
                                        c = (char) fgetc(stream);
                                        fclose(stream);
					/* FIXME: expand this to include ib and wan? */
					if (!strncmp(devname,"eth",3)) {
                                        	sprintf(fname,"%s%s%s/speed",KATTACH_SYSPATH,KATTACH_SYSNETPATH,devname);
                                        	stream = fopen(fname,"r");
                                        	nicval = fgets(nicinfo,32,stream);
						if (nicval != NULL) {
                                        		espeed = (u16) atoi(nicval);
						} else {
							espeed = 0;
						}
                                        	fclose(stream);
                                        	sprintf(fname,"%s%s%s/duplex",KATTACH_SYSPATH,KATTACH_SYSNETPATH,devname);
                                        	stream = fopen(fname,"r");
                                        	d = (char) fgetc(stream);
                                        	fclose (stream);
					} else {
						espeed = 100;
						d = 'f';
					}
					if ((strncmp(devname,"eth",3)) && (strncmp(devname,"wan",3)) && (strncmp(devname,"ib",2)) &&
						(strncmp(devname,"usb",3))) {
						kattach_netdev.pif[nindex].type = KATTACH_LINK_TYPE_PSUEDO;
					} else if (!strncmp(devname,"usb",3)) {
						kattach_netdev.pif[nindex].type = KATTACH_LINK_TYPE_USB;
					} else if (!strncmp(devname,"ib",2)) {
						kattach_netdev.pif[nindex].type = KATTACH_LINK_TYPE_INFINIBAND;
					} else {
						if (espeed >= 10000) {
							kattach_netdev.pif[nindex].type = KATTACH_LINK_TYPE_10GBE;
						} else if (espeed >= 1000) {
							kattach_netdev.pif[nindex].type = KATTACH_LINK_TYPE_1GBE;
						} else {
							kattach_netdev.pif[nindex].type = KATTACH_LINK_TYPE_ETHERNET;
						}
					}
					if (c == '0') {
						kattach_netdev.pif[nindex].status = KATTACH_LINK_STATUS_DOWN;
					} else {
						if (d == 'h') {
							if (espeed >= 10000) {
								kattach_netdev.pif[nindex].status = KATTACH_LINK_STATUS_UP_H10000;
							} else if (espeed >= 1000) {
								kattach_netdev.pif[nindex].status = KATTACH_LINK_STATUS_UP_H1000;
							} else if (espeed >= 100) {
								kattach_netdev.pif[nindex].status = KATTACH_LINK_STATUS_UP_H100;
							} else {
								kattach_netdev.pif[nindex].status = KATTACH_LINK_STATUS_UP;
							}
						} else {
							if (espeed >= 10000) {
								kattach_netdev.pif[nindex].status = KATTACH_LINK_STATUS_UP_10000;
							} else if (espeed >= 1000) {
								kattach_netdev.pif[nindex].status = KATTACH_LINK_STATUS_UP_1000;
							} else if (espeed >= 100) {
								kattach_netdev.pif[nindex].status = KATTACH_LINK_STATUS_UP_100;
							} else {
								kattach_netdev.pif[nindex].status = KATTACH_LINK_STATUS_UP;
							}
						}
					}
					ret = kattach_net_getmac(devname);
					for (m = 0; m <= 5; m++) {
						kattach_netdev.pif[nindex].mac[m] = kattach_genmac[m];
						kattach_genmac[m] = 0x00;
					}
				} 
			} else {
				ret = kattach_net_getmac(devname);
				for (m = 0; m <= 5; m++) {
					kattach_netdev.pif[index].mac[m] = kattach_genmac[m];
					kattach_genmac[m] = 0x00;
				}
				f = 0;
			}
		}

	}
	fclose(fp);

	sprintf(fname,"%s/iproute2",KATTACH_CFGPATH);
	ret = mkdir(fname,KATTACH_PERM);
	sprintf(fname,"%s/iproute2/rt_tables",KATTACH_CFGPATH);
	stream = fopen(fname,"w");

	if (stream == (FILE *)0) {
		printf("\n [!] FATAL -- Unable to write %s\n",fname);
	}

	fprintf(stream,"# KaOS \n");
	fprintf(stream,"# autogenerated routing tables by kattach\n");
	fprintf(stream,"# \n\n");
	fprintf(stream,"0\tunspec\n");

	for (index = 0; index < kattach_netdev.index; index++) {
		if (kattach_netdev.pif[index].pvid == 0x8023) continue;					/* skip interfaces part of an 802.3ad Link */
		if (kattach_netdev.pif[index].devname[0] == '\0') continue;
		if ((strlen(kattach_netdev.pif[index].devname) == 2) && (!strncmp(kattach_netdev.pif[index].devname,"lo",2))) continue;
		f = 0;
		l = strlen(kattach_netdev.pif[index].devname);
		fp = fopen("/proc/net/dev", "r");
		while ((devname = fgets(entry, 512, fp))) {
			while(isspace(devname[0]))
				devname++;

			delimit = strchr (devname, ':');
			if (delimit) {
				*delimit = 0;
				if (!strncmp(devname,kattach_netdev.pif[index].devname,l)) {
					f = 1;
					break;
				}
			}
		}
		fclose(fp);
		if (!f) {
			/* configured device is not found, disable it */
			kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_DISABLED;
			kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_UNKNOWN;
		} else {
			fprintf(stream,"%u\thv-%s\n",(index + 1),kattach_netdev.pif[index].devname);
		}
	}

	fprintf(stream,"253\tdefault\n");
	fprintf(stream,"254\tmain\n");
	fprintf(stream,"255\tlocal\n");

	fclose(stream);
	return;

}

void
kattach_vm_netdev_update(void)
{
	u16 index = 0, espeed = 0, lacpidx = 0, nindex = 0;
	char fname[64];
	char nicinfo[32];
	char lcmd[255];
	char *nicval;
	char *onicval;
	char c = '\0';
	char d = '\0';
	FILE *stream;
	u8 f = 0;

	/* FIXME: Need to handle someone changing an IP address */

	for (index = 0; index < kattach_netdev.index; index++) {
		if (strlen(kattach_netdev.pif[index].devname) == 0) continue;
		if ((strlen(kattach_netdev.pif[index].devname) == 2) && (!strncmp(kattach_netdev.pif[index].devname,"lo",2))) continue;
		if ((kattach_netdev.pif[index].pvid == 0) || (kattach_netdev.pif[index].mtu == 0)) continue;			/* not configured */
		if ((kattach_netdev.pif[index].status == KATTACH_LINK_STATUS_LACP_NEW) && (kattach_netdev.pif[index].pvid == 0x8023)) {
			/* this is a new LACP interface */
			lacpidx = kattach_netdev.pif[index].lacpidx;
			if (lacpidx > index) {
				sprintf(lcmd,"%s%s +lacp%u > %s%sbonding_masters",KATTACH_BINPATH,KCMD_ECHO,kattach_netdev.pif[lacpidx].pvid,
											KATTACH_SYSPATH,KATTACH_SYSNETPATH);
				kattach_sysexec(lcmd);
				/* set mode to 802.3ad */
				sprintf(lcmd,"%s%s 802.3ad > %s%slacp%u/bonding/mode",KATTACH_BINPATH,KCMD_ECHO,
											KATTACH_SYSPATH,KATTACH_SYSNETPATH,
											kattach_netdev.pif[lacpidx].pvid);
				kattach_sysexec(lcmd);
				/* bring up 802.3ad device */
				sprintf(lcmd,"%s%s link set lacp%u up",KATTACH_BINPATH,KCMD_IP,kattach_netdev.pif[lacpidx].pvid);
				kattach_sysexec(lcmd);

				kattach_netdev.pif[lacpidx].status = KATTACH_LINK_STATUS_LACP;
			}
			/* add interface to Link */
			sprintf(lcmd,"%s%s +%s > %s%slacp%u/bonding/slaves",KATTACH_BINPATH,KCMD_ECHO,kattach_netdev.pif[lacpidx].devname,
										KATTACH_SYSPATH,KATTACH_SYSNETPATH,
										kattach_netdev.pif[lacpidx].pvid);
			kattach_sysexec(lcmd);
			/* bring up interface */
			sprintf(lcmd,"%s%s link set %s up",KATTACH_BINPATH,KCMD_IP,kattach_netdev.pif[index].devname);
			kattach_sysexec(lcmd);
		} else if ((strncmp(kattach_netdev.pif[index].devname,"lacp",4)) && (kattach_netdev.pif[index].psuedo == KATTACH_LINK_STATUS_LACP_NEW)) {
			/* check to see if this device is still needed */
			for (nindex = 0; nindex < kattach_netdev.index; index++) {
				if (kattach_netdev.pif[index].pvid != 0x8023) continue;
				if (kattach_netdev.pif[index].lacpidx != index) continue;
				f = 1;
				break;
			}
			if (!f) {
				if (kattach_netdev.pif[index].ip == KATTACH_NET_HASDHCPIP) {
					/* FIXME: add code to kill dhcp client via pid */
					if (kattach_hasdhcpif) {
						kattach_hasdhcpif--;
					}
					sprintf(lcmd,"%s%s addr del %lu.%lu.%lu.%lu dev %s",KATTACH_BINPATH,KCMD_IP,
									(kattach_netdev.pif[index].gw >> 24) & 0xff,
									(kattach_netdev.pif[index].gw >> 16) & 0xff,
									(kattach_netdev.pif[index].gw >> 8) & 0xff,
									(kattach_netdev.pif[index].gw) & 0xff, kattach_netdev.pif[index].devname);
					kattach_sysexec(lcmd);
				} else {
					sprintf(lcmd,"%s%s addr del %lu.%lu.%lu.%lu/%u dev %s",KATTACH_BINPATH,KCMD_IP,
									(kattach_netdev.pif[index].ip >> 24) & 0xff,
									(kattach_netdev.pif[index].ip >> 16) & 0xff,
									(kattach_netdev.pif[index].ip >> 8) & 0xff,
									(kattach_netdev.pif[index].ip) & 0xff, kattach_netdev.pif[index].mask, kattach_netdev.pif[index].devname);
					kattach_sysexec(lcmd);
				}
				/* bring down 802.3ad device */
				sprintf(lcmd,"%s%s link set lacp%u down",KATTACH_BINPATH,KCMD_IP,kattach_netdev.pif[index].pvid);
				kattach_sysexec(lcmd);
				sprintf(lcmd,"%s%s -lacp%u > %s%sbonding_masters",KATTACH_BINPATH,KCMD_ECHO,kattach_netdev.pif[index].pvid,
											KATTACH_SYSPATH,KATTACH_SYSNETPATH);
				kattach_sysexec(lcmd);
				kattach_netdev.pif[index].mtu = 0;
				kattach_netdev.pif[index].pvid = 0;
				kattach_netdev.pif[index].ip = 0;
				kattach_netdev.pif[index].gw = 0;
				kattach_netdev.pif[index].type = 0;
				kattach_netdev.pif[index].psuedo = 0;
				kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_DELETED;
			} else if (kattach_netdev.pif[index].status != KATTACH_LINK_STATUS_LACP) {
				sprintf(lcmd,"%s%s +lacp%u > %s%sbonding_masters",KATTACH_BINPATH,KCMD_ECHO,kattach_netdev.pif[index].pvid,
											KATTACH_SYSPATH,KATTACH_SYSNETPATH);
				kattach_sysexec(lcmd);
				/* set mode to 802.3ad */
				sprintf(lcmd,"%s%s 802.3ad > %s%slacp%u/bonding/mode",KATTACH_BINPATH,KCMD_ECHO,
											KATTACH_SYSPATH,KATTACH_SYSNETPATH,
											kattach_netdev.pif[index].pvid);
				kattach_sysexec(lcmd);
				/* bring up 802.3ad device */
				sprintf(lcmd,"%s%s link set lacp%u up",KATTACH_BINPATH,KCMD_IP,kattach_netdev.pif[index].pvid);
				kattach_sysexec(lcmd);
			
				sprintf(lcmd,"%s%s link set dev %s mtu %u",KATTACH_BINPATH,KCMD_IP,kattach_netdev.pif[index].devname,kattach_netdev.pif[index].mtu);
				kattach_sysexec(lcmd);
				if (kattach_netdev.pif[index].ip == KATTACH_NET_HASDHCPIP) {
					kattach_hasdhcpif++;
					kattach_net_dhcp(kattach_netdev.pif[index].devname);
				} else {
					sprintf(lcmd,"%s%s addr add %lu.%lu.%lu.%lu/%u dev %s",KATTACH_BINPATH,KCMD_IP,
									(kattach_netdev.pif[index].ip >> 24) & 0xff,
									(kattach_netdev.pif[index].ip >> 16) & 0xff,
									(kattach_netdev.pif[index].ip >> 8) & 0xff,
									(kattach_netdev.pif[index].ip) & 0xff, kattach_netdev.pif[index].mask, kattach_netdev.pif[index].devname);
					kattach_sysexec(lcmd);
				}
				kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP;
			}
		} else {	
			sprintf(fname,"%s%s%s/operstate",KATTACH_SYSPATH,KATTACH_SYSNETPATH,kattach_netdev.pif[index].devname);
			stream = fopen(fname,"r");
			if (stream == (FILE *)0) {
				/* this device does not exist */
				if (kattach_netdev.pif[index].status != KATTACH_LINK_STATUS_DELETED) {
					kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_DISABLED;				/* flag it disabled */
				}
			} else {
				onicval = fgets(nicinfo,32,stream);
				if (onicval == NULL) continue;
				fclose(stream);
				if ((!strncmp(onicval,"unknown",7)) || (!strncmp(onicval,"down",4))) {
					/* nic is down */
					if ((kattach_netdev.pif[index].mtu) && (kattach_netdev.pif[index].status != KATTACH_LINK_STATUS_DISABLED) &&
						(kattach_netdev.pif[index].status != KATTACH_LINK_STATUS_DELETED)) {
							/* bring it back up */
						sprintf(fname,"%s%s%s/carrier",KATTACH_SYSPATH,KATTACH_SYSNETPATH,kattach_netdev.pif[index].devname);
						stream = fopen(fname,"r");
						c = (char) fgetc(stream);
						fclose(stream);
						if (!strncmp(kattach_netdev.pif[index].devname,"eth",3)) {
							sprintf(fname,"%s%s%s/speed",KATTACH_SYSPATH,KATTACH_SYSNETPATH,kattach_netdev.pif[index].devname);
							stream = fopen(fname,"r");
							nicval = fgets(nicinfo,32,stream);
							if (nicval != NULL) {
								espeed = (u16) atoi(nicval);
							} else {
								espeed = 0;
							}
							fclose(stream);
							sprintf(fname,"%s%s%s/duplex",KATTACH_SYSPATH,KATTACH_SYSNETPATH,kattach_netdev.pif[index].devname);
							stream = fopen(fname,"r");
							d = (char) fgetc(stream);
							fclose (stream);
						} else {
							espeed = 100;
							d = 'f';
						}
						sprintf(lcmd,"%s%s link set dev %s up",KATTACH_BINPATH,KCMD_IP,kattach_netdev.pif[index].devname);
						kattach_sysexec(lcmd);
						sprintf(lcmd,"%s%s link set dev %s mtu %u",KATTACH_BINPATH,KCMD_IP,kattach_netdev.pif[index].devname,kattach_netdev.pif[index].mtu);
						kattach_sysexec(lcmd);
						if (kattach_netdev.pif[index].ip == KATTACH_NET_HASDHCPIP) {
							kattach_hasdhcpif++;
							kattach_net_dhcp(kattach_netdev.pif[index].devname);
						} else {
							sprintf(lcmd,"%s%s addr add %lu.%lu.%lu.%lu/%u dev %s",KATTACH_BINPATH,KCMD_IP,
											(kattach_netdev.pif[index].ip >> 24) & 0xff,
											(kattach_netdev.pif[index].ip >> 16) & 0xff,
											(kattach_netdev.pif[index].ip >> 8) & 0xff,
											(kattach_netdev.pif[index].ip) & 0xff, kattach_netdev.pif[index].mask, kattach_netdev.pif[index].devname);
							kattach_sysexec(lcmd);
						}
						/* FIXME: should check operstate for up to be accurate */
						if (c == '0') {
							kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_DOWN;
						} else {
							if (d == 'h') {
								if (espeed >= 10000) {
									kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_H10000;
								} else if (espeed >= 1000) {
									kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_H1000;
								} else if (espeed >= 100) {
									kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_H100;
								} else {
									kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP;
								}
							} else {
								if (espeed >= 10000) {
									kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_10000;
								} else if (espeed >= 1000) {
									kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_1000;
								} else if (espeed >= 100) {
									kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_100;
								} else {
									kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP;
								}
							}
							if (!strncmp(kattach_netdev.pif[index].devname,kattach_cfg.netdev,strlen(kattach_cfg.netdev))) {
								if (kattach_netdev.pif[index].ip != KATTACH_NET_HASDHCPIP) {
									sprintf(lcmd,"ip route add default via %lu.%lu.%lu.%lu dev %s",
											(kattach_netdev.pif[index].gw >> 24) & 0xff,
											(kattach_netdev.pif[index].gw >> 16) & 0xff,
											(kattach_netdev.pif[index].gw >> 8) & 0xff,
											(kattach_netdev.pif[index].gw) & 0xff, kattach_netdev.pif[index].devname);
									kattach_sysexec(lcmd);
								}
							}
						}
						if ((strncmp(kattach_netdev.pif[index].devname,"eth",3)) && (strncmp(kattach_netdev.pif[index].devname,"wan",3)) && 
							(strncmp(kattach_netdev.pif[index].devname,"ib",2)) && (strncmp(kattach_netdev.pif[index].devname,"usb",3))) {
							kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_PSUEDO;
						} else if (!strncmp(kattach_netdev.pif[index].devname,"usb",3)) {
							kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_USB;
						} else if (!strncmp(kattach_netdev.pif[index].devname,"ib",2)) {
							kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_INFINIBAND;
						} else {
							if (espeed >= 10000) {
								kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_10GBE;
							} else if (espeed >= 1000) {
								kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_1GBE;
							} else {
								kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_ETHERNET;
							}
						}
					}
				} else if (!strncmp(onicval,"up",2)) {
					/* nic is up */
					if ((kattach_netdev.pif[index].mtu == 0) || (kattach_netdev.pif[index].status == KATTACH_LINK_STATUS_DISABLED) ||
						(kattach_netdev.pif[index].status == KATTACH_LINK_STATUS_DELETED)) {
							/* bring it down */
						if (kattach_netdev.pif[index].ip == KATTACH_NET_HASDHCPIP) {
							/* FIXME: add code to kill dhcp via dhcppid file */
							if (kattach_hasdhcpif) {
								kattach_hasdhcpif--;
							}
							sprintf(lcmd,"%s%s addr del %lu.%lu.%lu.%lu dev %s",KATTACH_BINPATH,KCMD_IP,
											(kattach_netdev.pif[index].ip >> 24) & 0xff,
											(kattach_netdev.pif[index].ip >> 16) & 0xff,
											(kattach_netdev.pif[index].ip >> 8) & 0xff,
											(kattach_netdev.pif[index].ip) & 0xff, kattach_netdev.pif[index].devname);
							kattach_sysexec(lcmd);
						} else {
							sprintf(lcmd,"%s%s addr del %lu.%lu.%lu.%lu/%u dev %s",KATTACH_BINPATH,KCMD_IP,
											(kattach_netdev.pif[index].ip >> 24) & 0xff,
											(kattach_netdev.pif[index].ip >> 16) & 0xff,
											(kattach_netdev.pif[index].ip >> 8) & 0xff,
											(kattach_netdev.pif[index].ip) & 0xff, kattach_netdev.pif[index].mask, kattach_netdev.pif[index].devname);
							kattach_sysexec(lcmd);
						}
						sprintf(lcmd,"%s%s link set dev %s down",KATTACH_BINPATH,KCMD_IP,kattach_netdev.pif[index].devname);
						kattach_sysexec(lcmd);
					} else {
						sprintf(fname,"%s%s%s/carrier",KATTACH_SYSPATH,KATTACH_SYSNETPATH,kattach_netdev.pif[index].devname);
						stream = fopen(fname,"r");
						c = (char) fgetc(stream);
						fclose(stream);
						if (!strncmp(kattach_netdev.pif[index].devname,"eth",3)) {
							sprintf(fname,"%s%s%s/speed",KATTACH_SYSPATH,KATTACH_SYSNETPATH,kattach_netdev.pif[index].devname);
							stream = fopen(fname,"r");
							nicval = fgets(nicinfo,32,stream);
							if (nicval != NULL) {
								espeed = (u16) atoi(nicval);
							} else {
								espeed = 0;
							}
							fclose(stream);
							sprintf(fname,"%s%s%s/duplex",KATTACH_SYSPATH,KATTACH_SYSNETPATH,kattach_netdev.pif[index].devname);
							stream = fopen(fname,"r");
							d = (char) fgetc(stream);
							fclose (stream);
						} else {
							espeed = 100;
							d = 'f';
						}
						/* FIXME: should check operstate for up to be accurate */
						if (c == '0') {
							kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_DOWN;
						} else {
							if (d == 'h') {
								if (espeed >= 10000) {
									kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_H10000;
								} else if (espeed >= 1000) {
									kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_H1000;
								} else if (espeed >= 100) {
									kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_H100;
								} else {
									kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP;
								}
							} else {
								if (espeed >= 10000) {
									kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_10000;
								} else if (espeed >= 1000) {
									kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_1000;
								} else if (espeed >= 100) {
									kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_100;
								} else {
									kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP;
								}
							}
							if (!strncmp(kattach_netdev.pif[index].devname,kattach_cfg.netdev,strlen(kattach_cfg.netdev))) {
								if (kattach_netdev.pif[index].ip != KATTACH_NET_HASDHCPIP) {
									sprintf(lcmd,"ip route add default via %lu.%lu.%lu.%lu dev %s",
											(kattach_netdev.pif[index].gw >> 24) & 0xff,
											(kattach_netdev.pif[index].gw >> 16) & 0xff,
											(kattach_netdev.pif[index].gw >> 8) & 0xff,
											(kattach_netdev.pif[index].gw) & 0xff, kattach_netdev.pif[index].devname);
									kattach_sysexec(lcmd);
								}
							}
						}
						if ((strncmp(kattach_netdev.pif[index].devname,"eth",3)) && (strncmp(kattach_netdev.pif[index].devname,"wan",3)) && 
							(strncmp(kattach_netdev.pif[index].devname,"ib",2)) && (strncmp(kattach_netdev.pif[index].devname,"usb",3))) {
							kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_PSUEDO;
						} else if (!strncmp(kattach_netdev.pif[index].devname,"usb",3)) {
							kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_USB;
						} else if (!strncmp(kattach_netdev.pif[index].devname,"ib",2)) {
							kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_INFINIBAND;
						} else {
							if (espeed >= 10000) {
								kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_10GBE;
							} else if (espeed >= 1000) {
								kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_1GBE;
							} else {
								kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_ETHERNET;
							}
						}
					}
				} else {
					/* nic is in a funny state */
					printf("\n\n [X] WARNING: Interface %s is in unknown state %s\n",kattach_netdev.pif[index].devname,onicval);
				}
			}
		}
	}

	kattach_sql_clear_netdev();
	kattach_sys_shm_setsync_netdev();
	return;
}

void
kattach_netdev_checklink(void)
{
	u16 index = 0, espeed = 0;
	FILE *stream;
	char fname[64];
	char d[32];
	char *c;
	char h = '\0';
	u8 haslink = 0, netdevch = 0;

	for (index = 0; index < kattach_netdev.index; index++) {
		if (kattach_netdev.pif[index].devname[0] == '\0') continue;
		if ((strlen(kattach_netdev.pif[index].devname) == 2) && (!strncmp(kattach_netdev.pif[index].devname,"lo",2))) continue;
		if ((kattach_netdev.pif[index].status == KATTACH_LINK_STATUS_DISABLED) ||
			(kattach_netdev.pif[index].status == KATTACH_LINK_STATUS_DELETED) || 
			(kattach_netdev.pif[index].mtu == 0)) continue;
		sprintf(fname,"%s%s%s/carrier",KATTACH_SYSPATH,KATTACH_SYSNETPATH,kattach_netdev.pif[index].devname);
		stream = fopen(fname,"r");
		if (stream == (FILE *) 0) {
			kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_DOWN;
		} else {
			c = fgets(d,32,stream);
			haslink = (u8) atoi(c);
			fclose(stream);
			if (haslink) {
				if (!strncmp(kattach_netdev.pif[index].devname,"eth",3)) {
					sprintf(fname,"%s%s%s/duplex",KATTACH_SYSPATH,KATTACH_SYSNETPATH,kattach_netdev.pif[index].devname);
					stream = fopen(fname,"r");
					h = (char) fgetc(stream);
					fclose(stream);
					sprintf(fname,"%s%s%s/speed",KATTACH_SYSPATH,KATTACH_SYSNETPATH,kattach_netdev.pif[index].devname);
					stream = fopen(fname,"r");
					c = fgets(d,32,stream);
					if (c != NULL) {
						espeed = (u16) atoi(c);
					} else {
						espeed = 0;
					}
					fclose(stream);
				} else {
					espeed = 100;
					h = 'f';
				}
				if (espeed >= 10000) {
					if (h == 'h') {
						if (kattach_netdev.pif[index].status != KATTACH_LINK_STATUS_UP_H10000) netdevch++;
						kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_H10000;
					} else {
						if (kattach_netdev.pif[index].status != KATTACH_LINK_STATUS_UP_10000) netdevch++;
						kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_10000;
					}
					/* FIXME: this is being lazy! */
					if ((kattach_netdev.pif[index].type == (KATTACH_LINK_TYPE_1GBE)) || 
						(kattach_netdev.pif[index].type == (KATTACH_LINK_TYPE_ETHERNET)) || 
						(kattach_netdev.pif[index].type == (KATTACH_LINK_TYPE_UNKNOWN))) {
						kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_10GBE;
					}
				} else if (espeed >= 1000) {
					if (h == 'h') {
						if (kattach_netdev.pif[index].status != KATTACH_LINK_STATUS_UP_H1000) netdevch++;
						kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_H1000;
					} else {
						if (kattach_netdev.pif[index].status != KATTACH_LINK_STATUS_UP_1000) netdevch++;
						kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_1000;
					}
					/* FIXME: this is being lazy! */
					if ((kattach_netdev.pif[index].type == (KATTACH_LINK_TYPE_10GBE)) ||
						(kattach_netdev.pif[index].type == (KATTACH_LINK_TYPE_ETHERNET)) || 
						(kattach_netdev.pif[index].type == (KATTACH_LINK_TYPE_UNKNOWN))) {
						kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_1GBE;
					}
				} else if (espeed >= 100) {
					if (h == 'h') {
						if (kattach_netdev.pif[index].status != KATTACH_LINK_STATUS_UP_H100) netdevch++;
						kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_H100;
					} else {
						if (kattach_netdev.pif[index].status != KATTACH_LINK_STATUS_UP_100) netdevch++;
						kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_100;
					}
					/* FIXME: this is being lazy! */
					if ((kattach_netdev.pif[index].type == (KATTACH_LINK_TYPE_1GBE)) || 
						(kattach_netdev.pif[index].type == (KATTACH_LINK_TYPE_10GBE)) || 
						(kattach_netdev.pif[index].type == (KATTACH_LINK_TYPE_UNKNOWN))) {
						kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_ETHERNET;
					}
				} else if (espeed >= 10) {
					if (h == 'h') {
						if (kattach_netdev.pif[index].status != KATTACH_LINK_STATUS_UP_H10) netdevch++;
						kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_H10;
					} else {
						if (kattach_netdev.pif[index].status != KATTACH_LINK_STATUS_UP_10) netdevch++;
						kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP_10;
					}
					/* FIXME: this is being lazy! */
					if ((kattach_netdev.pif[index].type == (KATTACH_LINK_TYPE_1GBE)) || 
						(kattach_netdev.pif[index].type == (KATTACH_LINK_TYPE_10GBE)) || 
						(kattach_netdev.pif[index].type == (KATTACH_LINK_TYPE_UNKNOWN))) {
						kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_ETHERNET;
					}
				} else {
					if (kattach_netdev.pif[index].status != KATTACH_LINK_STATUS_UP) netdevch++;
					kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_UP;
					/* FIXME: this is being lazy! */
					if ((kattach_netdev.pif[index].type == (KATTACH_LINK_TYPE_1GBE)) ||
						(kattach_netdev.pif[index].type == (KATTACH_LINK_TYPE_ETHERNET)) || 
						(kattach_netdev.pif[index].type == (KATTACH_LINK_TYPE_ETHERNET))) {
						kattach_netdev.pif[index].type = KATTACH_LINK_TYPE_UNKNOWN;
					}
				}
			} else {
				if (kattach_netdev.pif[index].status != KATTACH_LINK_STATUS_DOWN) netdevch++;
				kattach_netdev.pif[index].status = KATTACH_LINK_STATUS_DOWN;
			}
		}
	}
	if (netdevch) {
		kattach_sys_shm_setsync_netdev();
	}
	return;
}

void
kattach_vm_shutdown(void)
{
	u32 index = 0;

	for (index = 0; index < kattach_vmst.index; index++) {
		if (kattach_vmst.vmsess[index].vmstatus == KATTACH_VM_STATUS_RUNNING) kattach_vm_stop(index);
	}

	return;
}

void
kattach_vm_cfggrp_update(void)
{
	u32 index = 0, nindex = 0;
	char ccmd[64];
	char lcmd[64];
	char sqlq[1024];
	u8 rc = 0, n = 0;
	int res = 0;

	/* FIXME: add multiple vendor support */

	for (index = 0; index < KATTACH_MAX_CFGGRP; index++) {
		if (kattach_cfggrp.cfggrp[index].name[0] == '\0') {
			if ((kattach_cfggrp.cfggrp[index].appmidx != 0) &&
				(kattach_appmods.appmodules[kattach_cfggrp.cfggrp[index].appmidx].mgrpid != 0)) {
				/* [270]: Only shutdown the manager process if its not used by another group. */
				for (nindex = (1 + index); nindex < KATTACH_MAX_CFGGRP; nindex++) {
					if (kattach_cfggrp.cfggrp[index].appmidx == kattach_cfggrp.cfggrp[nindex].appmidx) {
						n++;
						break;
					}
				}
				if (!n) {
					/* FIXME: need proper shutdown of the mgr process */
					res = kill(kattach_appmods.appmodules[kattach_cfggrp.cfggrp[index].appmidx].mgrpid, SIGKILL);
					kattach_appmods.appmodules[kattach_cfggrp.cfggrp[index].appmidx].mgrpid = 0;
				}
				n = 0;
				kattach_cfggrp.cfggrp[index].appmidx = 0;
				/* DELETE item from data base */
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"DELETE from cfggrp WHERE cfggidx = '%lu';", (1 + index));
				rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);
			}
			continue;
		} else {
			/* FIXME: we update the db everytime -- do we care? */
			/* [270]: Update the cfggrp database table appropriately */
			memset(sqlq,0,sizeof(sqlq));
			sprintf(sqlq,"INSERT into cfggrp values (%lu,'%s',%lu);", (1 + index), kattach_cfggrp.cfggrp[index].name, (1 + kattach_cfggrp.cfggrp[index].appmidx));
			rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);

			if (rc == RC_FAIL) {
				/* try update instead */
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE cfggrp SET name = '%s' WHERE cfggidx = '%lu';", kattach_cfggrp.cfggrp[index].name, (1 + index));
				rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);
				memset(sqlq,0,sizeof(sqlq));
				sprintf(sqlq,"UPDATE cfggrp SET appindex = '%lu' WHERE cfggidx = '%lu';", (1 + kattach_cfggrp.cfggrp[index].appmidx), (1 + index));
				rc = kattach_vm_sql_iu(sqlq, K_DB_APPQ);
			} 
		}
		if (kattach_appmods.appmodules[kattach_cfggrp.cfggrp[index].appmidx].mgrpid == 0) {
			sprintf(ccmd,"%s%s/%s%s",KATTACH_APPQUEUE_APPMOD,KATTACH_APPQUEUE_VENDOR_CM,
				kattach_appmods.appmodules[kattach_cfggrp.cfggrp[index].appmidx].name,KATTACH_APPQUEUE_MGRPATH);
			sprintf(lcmd,"%s",kattach_cfggrp.cfggrp[index].name);
			kattach_appmods.appmodules[kattach_cfggrp.cfggrp[index].appmidx].mgrpid = kattach_bkexec(ccmd,lcmd);
		}
	}

	return;
}

void
kattach_vm_fw_update(void)
{
	if ((kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_FILTER_INPUT)) ||
		(kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_FILTER_OUTPUT)) ||
		(kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_FILTER_FORWARD))) {
		kattach_vm_fw_update_filter();
	}

	if ((kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_NAT_PREROUTING)) ||
		(kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_NAT_POSTROUTING)) ||
		(kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_NAT_OUTPUT))) {
		kattach_vm_fw_update_nat();
	}

	if ((kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_MANGLE_PREROUTING)) ||
		(kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_MANGLE_POSTROUTING)) ||
		(kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_MANGLE_INPUT)) ||
		(kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_MANGLE_OUTPUT)) ||
		(kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_MANGLE_FORWARD))) {
		kattach_vm_fw_update_mangle();
	}

	if (kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_ZONES)) {
		kattach_vm_fw_update_zones();
	}	

	if (kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_APPS)) {
		kattach_vm_fw_update_apps();
	}	

	return;
}

void
kattach_vm_fw_update_filter(void)
{
	char lcmd[255];
	char ecmd[128];
	char ccmd[64];
	char mcmd[64];
	char txtch[16];
	u32 index = 0;
	u8 y = 0, l = 0, d = 0, nindex = 0, pindex = 0, dindex = 0;
	u8 fwupd[3];
        kattach_fw_chain_t *chain;

	if (kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_FILTER_INPUT)) {
		fwupd[0] = 1;
		kattach_fw.fw_update ^= KATTACH_FW_CH_FILTER_INPUT;
	} else {
		fwupd[0] = 0;
	}

	if (kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_FILTER_OUTPUT)) {
		fwupd[1] = 1;
		kattach_fw.fw_update ^= KATTACH_FW_CH_FILTER_OUTPUT;
	} else {
		fwupd[1] = 0;
	}

	if (kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_FILTER_FORWARD)) {
		fwupd[2] = 1;
		kattach_fw.fw_update ^= KATTACH_FW_CH_FILTER_FORWARD;
	} else {
		fwupd[2] = 0;
	}

	/* filter */
	while (!y) {
		if ((l == 0) && (fwupd[0] == 0)) {
			l = 1;
		}
		if ((l == 1) && (fwupd[1] == 0)) {
			l = 2;
		}
		if ((l == 2) && (fwupd[2] == 0)) {
			y++;
			break;
		}
                if (l == 0) {
                        chain = &kattach_fw.filter.input;
			sprintf(txtch,"INPUT");
                } else if (l == 1) {
                        chain = &kattach_fw.filter.output;
			sprintf(txtch,"OUTPUT");
                } else if (l == 2) {
                        chain = &kattach_fw.filter.forward;
			sprintf(txtch,"FORWARD");
                } else {
                        y++;
                        break;
                }
		/* Flush the chain and rebuild */
		sprintf(lcmd,"%s -F %s",KCMD_IPT,txtch);
		kattach_sysexec(lcmd);

		if (l == 0) {
			/* FIXME: special case rules should be integrated into the fw framework */
			/* special case rules */
			/* drop INVALID state traffic */
			sprintf(lcmd,"%s -A INPUT -m state --state INVALID -j DROP",KCMD_IPT);
			kattach_sysexec(lcmd);

			/* drop ICMP fragments */
			sprintf(lcmd,"%s -A INPUT -p icmp -m icmp --fragment -j DROP",KCMD_IPT);
			kattach_sysexec(lcmd);

			/* drop invalid TCP traffic */
			sprintf(lcmd,"%s -A INPUT -m state --state NEW -p tcp --tcp-flags ALL ALL -j DROP",KCMD_IPT);
			kattach_sysexec(lcmd);
			sprintf(lcmd,"%s -A INPUT -m state --state NEW -p tcp --tcp-flags ALL NONE -j DROP",KCMD_IPT);
			kattach_sysexec(lcmd);
			sprintf(lcmd,"%s -A INPUT -p tcp ! --syn -m state --state NEW -j DROP",KCMD_IPT);
			kattach_sysexec(lcmd);

			/* allow input traffic from the loopback interface */
			sprintf(lcmd,"%s -A INPUT -i lo -j ACCEPT", KCMD_IPT);
			kattach_sysexec(lcmd);

			/* allow input traffic from related and established connections */
			sprintf(lcmd,"%s -A INPUT -m state --state RELATED,ESTABLISHED -j ACCEPT",KCMD_IPT);
			kattach_sysexec(lcmd);
		} else if (l == 1) {
			/* allow hypervisor traffic out */
			sprintf(lcmd,"%s -A OUTPUT -o lo -j ACCEPT",KCMD_IPT);
			kattach_sysexec(lcmd);

			sprintf(lcmd,"%s -A OUTPUT -m state --state RELATED,ESTABLISHED -j ACCEPT",KCMD_IPT);
			kattach_sysexec(lcmd);

			sprintf(lcmd,"%s -A OUTPUT -m state --state NEW -j ACCEPT",KCMD_IPT);
			kattach_sysexec(lcmd);
			/* end special case rules */
		} else if (l == 2) {
			/* Add custom chains */
			sprintf(lcmd,"%s -A FORWARD -j VNS-FORWARD",KCMD_IPT);
			kattach_sysexec(lcmd);
		}
		index = chain->hindex;
		d = 0;
		while (!d) {
			if (chain->filter[index].enabled) {
				for (pindex = 0; pindex < kattach_fw.apps.app[chain->filter[index].appindex].pindex; pindex++) {
					if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].direction == KATTACH_FW_DIR_SOURCE) {
						if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[chain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[chain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m icmp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (chain->filter[index].logging) {
										sprintf(lcmd,"%s -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0], ecmd,
											chain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(chain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((chain->filter[index].rlimitpkt != 0) && (chain->filter[index].rlimitint != 0)) {
											if (chain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec", chain->filter[index].rlimitpkt);
											} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min", chain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour", chain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_TCP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[chain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[chain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m tcp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (chain->filter[index].logging) {
										sprintf(lcmd,"%s -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], ecmd,
											chain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(chain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((chain->filter[index].rlimitpkt != 0) && (chain->filter[index].rlimitint != 0)) {
											if (chain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",chain->filter[index].rlimitpkt);
											} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",chain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",chain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_UDP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[chain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[chain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m udp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (chain->filter[index].logging) {
										sprintf(lcmd,"%s -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], ecmd,
											chain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(chain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((chain->filter[index].rlimitpkt != 0) && (chain->filter[index].rlimitint != 0)) {
											if (chain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",chain->filter[index].rlimitpkt);
											} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",chain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",chain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}

						}
					} else if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].direction == KATTACH_FW_DIR_DESTINATION) {
						if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[chain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[chain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m icmp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (chain->filter[index].logging) {
										sprintf(lcmd,"%s -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0], ecmd,
											chain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(chain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((chain->filter[index].rlimitpkt != 0) && (chain->filter[index].rlimitint != 0)) {
											if (chain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec", chain->filter[index].rlimitpkt);
											} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min", chain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour", chain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_TCP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[chain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[chain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m tcp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (chain->filter[index].logging) {
										sprintf(lcmd,"%s -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], ecmd,
											chain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(chain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((chain->filter[index].rlimitpkt != 0) && (chain->filter[index].rlimitint != 0)) {
											if (chain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",chain->filter[index].rlimitpkt);
											} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",chain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",chain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_UDP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[chain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[chain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m udp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (chain->filter[index].logging) {
										sprintf(lcmd,"%s -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], ecmd,
											chain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(chain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((chain->filter[index].rlimitpkt != 0) && (chain->filter[index].rlimitint != 0)) {
											if (chain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",chain->filter[index].rlimitpkt);
											} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",chain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",chain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}

						}
					} else if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].direction == KATTACH_FW_DIR_BOTH) {
						if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[chain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[chain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m icmp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (chain->filter[index].logging) {
										sprintf(lcmd,"%s -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0], ecmd,
											chain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(chain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((chain->filter[index].rlimitpkt != 0) && (chain->filter[index].rlimitint != 0)) {
											if (chain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec", chain->filter[index].rlimitpkt);
											} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min", chain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour", chain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_TCP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[chain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[chain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m tcp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (chain->filter[index].logging) {
										sprintf(lcmd,"%s -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], ecmd,
											chain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(chain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((chain->filter[index].rlimitpkt != 0) && (chain->filter[index].rlimitint != 0)) {
											if (chain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",chain->filter[index].rlimitpkt);
											} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",chain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",chain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_UDP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[chain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[chain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m udp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[chain->filter[index].appindex].statemask == (kattach_fw.apps.app[chain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (chain->filter[index].logging) {
										sprintf(lcmd,"%s -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], ecmd,
											chain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(chain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((chain->filter[index].rlimitpkt != 0) && (chain->filter[index].rlimitint != 0)) {
											if (chain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",chain->filter[index].rlimitpkt);
											} else if ((chain->filter[index].rlimitint >= 60) && (chain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",chain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",chain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[chain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[chain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}

						}
					}
				}

			}
			if (index == chain->eindex) {
				d++;
				break;
			} else {
				index = chain->filter[index].nindex;
			}
			
		}
		l++;
	}
	kattach_vm_fw_syncdb_filter();
	kattach_vm_fw_syncdb_fwmain();
	return;
}

void
kattach_vm_fw_update_nat(void)
{
	char lcmd[255];
	char ecmd[128];
	char ccmd[64];
	char mcmd[64];
	char ncmd[64];
	char txtch[16];
	u32 index = 0;
	u8 y = 0, l = 0, d = 0, nindex = 0, pindex = 0, dindex = 0, ntype = 0, natindex = 0, napindex = 0;
	u8 fwupd[3];
        kattach_fw_n_chain_t *nchain;

	if (kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_NAT_PREROUTING)) {
		fwupd[0] = 1;
		kattach_fw.fw_update ^= KATTACH_FW_CH_NAT_PREROUTING;
	} else {
		fwupd[0] = 0;
	}

	if (kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_NAT_POSTROUTING)) {
		fwupd[1] = 1;
		kattach_fw.fw_update ^= KATTACH_FW_CH_NAT_POSTROUTING;
	} else {
		fwupd[1] = 0;
	}

	if (kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_NAT_OUTPUT)) {
		fwupd[2] = 1;
		kattach_fw.fw_update ^= KATTACH_FW_CH_NAT_OUTPUT;
	} else {
		fwupd[2] = 0;
	}

	/* nat */
	while (!y) {
                if ((l == 0) && (fwupd[0] == 0)) {
                        l = 1;
                }
                if ((l == 1) && (fwupd[1] == 0)) {
                        l = 2;
                }
                if ((l == 2) && (fwupd[2] == 0)) {
                        y++;
                        break;
                }
                if (l == 0) {
                        nchain = &kattach_fw.nat.prerouting;
			sprintf(txtch,"PREROUTING");
                } else if (l == 1) {
                        nchain = &kattach_fw.nat.postrouting;
			sprintf(txtch,"POSTROUTING");
                } else if (l == 2) {
                        nchain = &kattach_fw.nat.output;
			sprintf(txtch,"OUTPUT");
                } else {
                        y++;
                        break;
                }
		/* Flush the chain and rebuild */
		sprintf(lcmd,"%s -t nat -F %s",KCMD_IPT,txtch);
		kattach_sysexec(lcmd);

		if (l == 0) {
			/* Add custom chains */
			sprintf(lcmd,"%s -t nat -A PREROUTING -j VNS-PREROUTING",KCMD_IPT);
			kattach_sysexec(lcmd);
		} else if (l == 1) {
			/* Add custom chains */
			sprintf(lcmd,"%s -t nat -A POSTROUTING -j VNS-POSTROUTING",KCMD_IPT);
			kattach_sysexec(lcmd);
		}

		index = nchain->hindex;
		d = 0;
		while (!d) {
			if (nchain->filter[index].enabled) {
				for (pindex = 0; pindex < kattach_fw.apps.app[nchain->filter[index].appindex].pindex; pindex++) {
					if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].direction == KATTACH_FW_DIR_SOURCE) {
						if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[nchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[nchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m icmp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (nchain->filter[index].logging) {
										sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], ecmd,
											nchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(nchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_SNAT:
											if (l == 1) {
												sprintf(ccmd,"-j SNAT");
												ntype = 1;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu SNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_DNAT:
											if (l == 0) {
												sprintf(ccmd,"-j DNAT");
												ntype = 2;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu DNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_MASQ:
											if (l == 1) {
												sprintf(ccmd,"-j MASQ");
												ntype = 4;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu MASQ not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_REDIR:
											if (l == 0) {
												sprintf(ccmd,"-j SNAT");
												ntype = 3;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu REDIRECT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_NETMAP:
											if ((l == 1) || (l == 0)) {
												sprintf(ccmd,"-j NETMAP");
												ntype = 5;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu NETMAP not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((nchain->filter[index].rlimitpkt != 0) && (nchain->filter[index].rlimitint != 0)) {
											if (nchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec", nchain->filter[index].rlimitpkt);
											} else if ((nchain->filter[index].rlimitint >= 60) && (nchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min", nchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour", nchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									if (ntype == 0) {
										sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd);
										kattach_sysexec(lcmd);
									} else if (ntype != 5) {
										/* duplicate the rule for each NAT Application, and each NAT Target Zone */
										for (napindex = 0; pindex < kattach_fw.apps.app[nchain->filter[index].nappindex].pindex; napindex++) {
											if (kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask != 
												(kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) continue;
											for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
												if (ntype == 1) {
													sprintf(ncmd," --to-source %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else if (ntype == 2) {
													sprintf(ncmd," --to-destination %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else {
													sprintf(ncmd," ");
												}
												sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s %s",
													KCMD_IPT,txtch,
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
													kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd, ncmd);
												kattach_sysexec(lcmd);
											}
										}
									} else {
										/* netmap case */
										for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
											sprintf(ncmd," --to %lu.%lu.%lu.%lu/%u",
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].mask);
											sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s %s",
												KCMD_IPT,txtch,
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
												kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd, ncmd);
											kattach_sysexec(lcmd);
										}

									}
									ntype = 0;
								}
							}
						}
						if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_TCP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[nchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[nchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m tcp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (nchain->filter[index].logging) {
										sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], ecmd,
											nchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(nchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;
										case KATTACH_FW_ACTION_SNAT:
											if (l == 1) {
												sprintf(ccmd,"-j SNAT");
												ntype = 1;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu SNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_DNAT:
											if (l == 0) {
												sprintf(ccmd,"-j DNAT");
												ntype = 2;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu DNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_MASQ:
											if (l == 1) {
												sprintf(ccmd,"-j MASQ");
												ntype = 4;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu MASQ not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_REDIR:
											if (l == 0) {
												sprintf(ccmd,"-j SNAT");
												ntype = 3;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu REDIRECT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_NETMAP:
											if ((l == 1) || (l == 0)) {
												sprintf(ccmd,"-j NETMAP");
												ntype = 5;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu NETMAP not permitted in %u: '",index,l);
											}
											break;


										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((nchain->filter[index].rlimitpkt != 0) && (nchain->filter[index].rlimitint != 0)) {
											if (nchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",nchain->filter[index].rlimitpkt);
											} else if ((nchain->filter[index].rlimitint >= 60) && (nchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",nchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",nchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}

									if (ntype == 0) {
										sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
										kattach_sysexec(lcmd);
									} else if (ntype != 5) {
										/* duplicate the rule for each NAT Application, and each NAT Target Zone */
										for (napindex = 0; pindex < kattach_fw.apps.app[nchain->filter[index].nappindex].pindex; napindex++) {
											if (kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask != 
												(kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) continue;
											for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
												if (ntype == 1) {
													/* FIXME: add support for app ports */
													sprintf(ncmd," --to-source %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else if (ntype == 2) {
													/* FIXME: add support for app ports */
													sprintf(ncmd," --to-destination %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else {
													sprintf(ncmd," ");
												}
												sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s %s",
													KCMD_IPT,txtch,
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
													kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
													kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd, ncmd);
												kattach_sysexec(lcmd);
											}
										}
									} else {
										/* netmap case */
										for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
											sprintf(ncmd," --to %lu.%lu.%lu.%lu/%u",
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].mask);
											sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s %s",
												KCMD_IPT,txtch,
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
												kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
												kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd, ncmd);
											kattach_sysexec(lcmd);
										}

									}
									ntype = 0;
								}
							}
						}
						if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_UDP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[nchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[nchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m udp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (nchain->filter[index].logging) {
										sprintf(lcmd,"%s -t nat -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], ecmd,
											nchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(nchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_SNAT:
											if (l == 1) {
												sprintf(ccmd,"-j SNAT");
												ntype = 1;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu SNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_DNAT:
											if (l == 0) {
												sprintf(ccmd,"-j DNAT");
												ntype = 2;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu DNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_MASQ:
											if (l == 1) {
												sprintf(ccmd,"-j MASQ");
												ntype = 4;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu MASQ not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_REDIR:
											if (l == 0) {
												sprintf(ccmd,"-j SNAT");
												ntype = 3;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu REDIRECT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_NETMAP:
											if ((l == 1) || (l == 0)) {
												sprintf(ccmd,"-j NETMAP");
												ntype = 5;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu NETMAP not permitted in %u: '",index,l);
											}
											break;



										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((nchain->filter[index].rlimitpkt != 0) && (nchain->filter[index].rlimitint != 0)) {
											if (nchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",nchain->filter[index].rlimitpkt);
											} else if ((nchain->filter[index].rlimitint >= 60) && (nchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",nchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",nchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t nat -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}

						}
					} else if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].direction == KATTACH_FW_DIR_DESTINATION) {
						if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[nchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[nchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m icmp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (nchain->filter[index].logging) {
										sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], ecmd,
											nchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(nchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_SNAT:
											if (l == 1) {
												sprintf(ccmd,"-j SNAT");
												ntype = 1;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu SNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_DNAT:
											if (l == 0) {
												sprintf(ccmd,"-j DNAT");
												ntype = 2;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu DNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_MASQ:
											if (l == 1) {
												sprintf(ccmd,"-j MASQ");
												ntype = 4;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu MASQ not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_REDIR:
											if (l == 0) {
												sprintf(ccmd,"-j SNAT");
												ntype = 3;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu REDIRECT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_NETMAP:
											if ((l == 1) || (l == 0)) {
												sprintf(ccmd,"-j NETMAP");
												ntype = 5;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu NETMAP not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((nchain->filter[index].rlimitpkt != 0) && (nchain->filter[index].rlimitint != 0)) {
											if (nchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec", nchain->filter[index].rlimitpkt);
											} else if ((nchain->filter[index].rlimitint >= 60) && (nchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min", nchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour", nchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									if (ntype == 0) {
										sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd);
										kattach_sysexec(lcmd);
									} else if (ntype != 5) {
										/* duplicate the rule for each NAT Application, and each NAT Target Zone */
										for (napindex = 0; pindex < kattach_fw.apps.app[nchain->filter[index].nappindex].pindex; napindex++) {
											if (kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask != 
												(kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) continue;
											for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
												if (ntype == 1) {
													sprintf(ncmd," --to-source %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else if (ntype == 2) {
													sprintf(ncmd," --to-destination %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else {
													sprintf(ncmd," ");
												}
												sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s %s",
													KCMD_IPT,txtch,
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
													kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd, ncmd);
												kattach_sysexec(lcmd);
											}
										}
									} else {
										/* netmap case */
										for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
											sprintf(ncmd," --to %lu.%lu.%lu.%lu/%u",
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].mask);
											sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s %s",
												KCMD_IPT,txtch,
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
												kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd, ncmd);
											kattach_sysexec(lcmd);
										}

									}
									ntype = 0;
								}
							}
						}
						if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_TCP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[nchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[nchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m tcp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (nchain->filter[index].logging) {
										sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], ecmd,
											nchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(nchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_SNAT:
											if (l == 1) {
												sprintf(ccmd,"-j SNAT");
												ntype = 1;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu SNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_DNAT:
											if (l == 0) {
												sprintf(ccmd,"-j DNAT");
												ntype = 2;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu DNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_MASQ:
											if (l == 1) {
												sprintf(ccmd,"-j MASQ");
												ntype = 4;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu MASQ not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_REDIR:
											if (l == 0) {
												sprintf(ccmd,"-j SNAT");
												ntype = 3;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu REDIRECT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_NETMAP:
											if ((l == 1) || (l == 0)) {
												sprintf(ccmd,"-j NETMAP");
												ntype = 5;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu NETMAP not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((nchain->filter[index].rlimitpkt != 0) && (nchain->filter[index].rlimitint != 0)) {
											if (nchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",nchain->filter[index].rlimitpkt);
											} else if ((nchain->filter[index].rlimitint >= 60) && (nchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",nchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",nchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}

									if (ntype == 0) {
										sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
										kattach_sysexec(lcmd);
									} else if (ntype != 5) {
										/* duplicate the rule for each NAT Application, and each NAT Target Zone */
										for (napindex = 0; pindex < kattach_fw.apps.app[nchain->filter[index].nappindex].pindex; napindex++) {
											if (kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask != 
												(kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) continue;
											for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
												if (ntype == 1) {
													/* FIXME: add support for app ports */
													sprintf(ncmd," --to-source %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else if (ntype == 2) {
													/* FIXME: add support for app ports */
													sprintf(ncmd," --to-destination %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else {
													sprintf(ncmd," ");
												}
												sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s %s",
													KCMD_IPT,txtch,
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
													kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
													kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd, ncmd);
												kattach_sysexec(lcmd);
											}
										}
									} else {
										/* netmap case */
										for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
											sprintf(ncmd," --to %lu.%lu.%lu.%lu/%u",
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].mask);
											sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s %s",
												KCMD_IPT,txtch,
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
												kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
												kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd, ncmd);
											kattach_sysexec(lcmd);
										}

									}
									ntype = 0;
								}
							}
						}
						if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_UDP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[nchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[nchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m udp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (nchain->filter[index].logging) {
										sprintf(lcmd,"%s -t nat -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], ecmd,
											nchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(nchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_SNAT:
											if (l == 1) {
												sprintf(ccmd,"-j SNAT");
												ntype = 1;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu SNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_DNAT:
											if (l == 0) {
												sprintf(ccmd,"-j DNAT");
												ntype = 2;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu DNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_MASQ:
											if (l == 1) {
												sprintf(ccmd,"-j MASQ");
												ntype = 4;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu MASQ not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_REDIR:
											if (l == 0) {
												sprintf(ccmd,"-j SNAT");
												ntype = 3;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu REDIRECT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_NETMAP:
											if ((l == 1) || (l == 0)) {
												sprintf(ccmd,"-j NETMAP");
												ntype = 5;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu NETMAP not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu'",txtch,index);
											break;
									}
									if ((nchain->filter[index].rlimitpkt != 0) && (nchain->filter[index].rlimitint != 0)) {
											if (nchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",nchain->filter[index].rlimitpkt);
											} else if ((nchain->filter[index].rlimitint >= 60) && (nchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",nchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",nchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t nat -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}

						}
					} else if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].direction == KATTACH_FW_DIR_BOTH) {
						if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[nchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[nchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m icmp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (nchain->filter[index].logging) {
										sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], ecmd,
											nchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(nchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_SNAT:
											if (l == 1) {
												sprintf(ccmd,"-j SNAT");
												ntype = 1;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu SNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_DNAT:
											if (l == 0) {
												sprintf(ccmd,"-j DNAT");
												ntype = 2;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu DNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_MASQ:
											if (l == 1) {
												sprintf(ccmd,"-j MASQ");
												ntype = 4;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu MASQ not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_REDIR:
											if (l == 0) {
												sprintf(ccmd,"-j SNAT");
												ntype = 3;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu REDIRECT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_NETMAP:
											if ((l == 1) || (l == 0)) {
												sprintf(ccmd,"-j NETMAP");
												ntype = 5;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu NETMAP not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((nchain->filter[index].rlimitpkt != 0) && (nchain->filter[index].rlimitint != 0)) {
											if (nchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec", nchain->filter[index].rlimitpkt);
											} else if ((nchain->filter[index].rlimitint >= 60) && (nchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min", nchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour", nchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									if (ntype == 0) {
										sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd);
										kattach_sysexec(lcmd);
									} else if (ntype != 5) {
										/* duplicate the rule for each NAT Application, and each NAT Target Zone */
										for (napindex = 0; pindex < kattach_fw.apps.app[nchain->filter[index].nappindex].pindex; napindex++) {
											if (kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask != 
												(kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) continue;
											for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
												if (ntype == 1) {
													sprintf(ncmd," --to-source %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else if (ntype == 2) {
													sprintf(ncmd," --to-destination %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else {
													sprintf(ncmd," ");
												}
												sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s %s",
													KCMD_IPT,txtch,
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
													kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd, ncmd);
												kattach_sysexec(lcmd);
											}
										}
									} else {
										/* netmap case */
										for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
											sprintf(ncmd," --to %lu.%lu.%lu.%lu/%u",
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].mask);
											sprintf(lcmd,"%s -t nat -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s %s",
												KCMD_IPT,txtch,
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
												kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd, ncmd);
											kattach_sysexec(lcmd);
										}

									}
									ntype = 0;
								}
							}
						}
						if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_TCP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[nchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[nchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m tcp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (nchain->filter[index].logging) {
										sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], ecmd,
											nchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(nchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_SNAT:
											if (l == 1) {
												sprintf(ccmd,"-j SNAT");
												ntype = 1;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu SNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_DNAT:
											if (l == 0) {
												sprintf(ccmd,"-j DNAT");
												ntype = 2;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu DNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_MASQ:
											if (l == 1) {
												sprintf(ccmd,"-j MASQ");
												ntype = 4;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu MASQ not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_REDIR:
											if (l == 0) {
												sprintf(ccmd,"-j SNAT");
												ntype = 3;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu REDIRECT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_NETMAP:
											if ((l == 1) || (l == 0)) {
												sprintf(ccmd,"-j NETMAP");
												ntype = 5;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu NETMAP not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((nchain->filter[index].rlimitpkt != 0) && (nchain->filter[index].rlimitint != 0)) {
											if (nchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",nchain->filter[index].rlimitpkt);
											} else if ((nchain->filter[index].rlimitint >= 60) && (nchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",nchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",nchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}

									if (ntype == 0) {
										sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
										kattach_sysexec(lcmd);
									} else if (ntype != 5) {
										/* duplicate the rule for each NAT Application, and each NAT Target Zone */
										for (napindex = 0; pindex < kattach_fw.apps.app[nchain->filter[index].nappindex].pindex; napindex++) {
											if (kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask != 
												(kattach_fw.apps.app[nchain->filter[index].appindex].port[napindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) continue;
											for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
												if (ntype == 1) {
													/* FIXME: add support for app ports */
													sprintf(ncmd," --to-source %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else if (ntype == 2) {
													/* FIXME: add support for app ports */
													sprintf(ncmd," --to-destination %lu.%lu.%lu.%lu",
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
														((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff));
												} else {
													sprintf(ncmd," ");
												}
												sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s %s",
													KCMD_IPT,txtch,
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
													((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
													kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
													kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
													kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd, ncmd);
												kattach_sysexec(lcmd);
											}
										}
									} else {
										/* netmap case */
										for (natindex = 0; natindex < kattach_fw.zones.zone[nchain->filter[index].nzindex].nindex; natindex++) {
											sprintf(ncmd," --to %lu.%lu.%lu.%lu/%u",
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].nzindex].node[natindex].mask);
											sprintf(lcmd,"%s -t nat -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s %s",
												KCMD_IPT,txtch,
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
												((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
												kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
												kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
												kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd, ncmd);
											kattach_sysexec(lcmd);
										}

									}
									ntype = 0;
								}
							}
						}
						if (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_UDP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[nchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[nchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m udp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[nchain->filter[index].appindex].statemask == (kattach_fw.apps.app[nchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (nchain->filter[index].logging) {
										sprintf(lcmd,"%s -t nat -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], ecmd,
											nchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(nchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_SNAT:
											if (l == 1) {
												sprintf(ccmd,"-j SNAT");
												ntype = 1;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu SNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_DNAT:
											if (l == 0) {
												sprintf(ccmd,"-j DNAT");
												ntype = 2;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu DNAT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_MASQ:
											if (l == 1) {
												sprintf(ccmd,"-j MASQ");
												ntype = 4;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu MASQ not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_REDIR:
											if (l == 0) {
												sprintf(ccmd,"-j SNAT");
												ntype = 3;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu REDIRECT not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_NETMAP:
											if ((l == 1) || (l == 0)) {
												sprintf(ccmd,"-j NETMAP");
												ntype = 5;
											} else {
												sprintf(ccmd,"-j LOG --log-prefix 'filter_%lu NETMAP not permitted in %u: '",index,l);
											}
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((nchain->filter[index].rlimitpkt != 0) && (nchain->filter[index].rlimitint != 0)) {
											if (nchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",nchain->filter[index].rlimitpkt);
											} else if ((nchain->filter[index].rlimitint >= 60) && (nchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",nchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",nchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t nat -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[nchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[nchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[nchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}

						}
					}
				}

			}
			if (index == nchain->eindex) {
				d++;
				break;
			} else {
				index = nchain->filter[index].nindex;
			}
			
		}
		l++;
	}

	kattach_vm_fw_syncdb_nat();
	kattach_vm_fw_syncdb_fwmain();
	return;
}

void
kattach_vm_fw_update_mangle(void)
{
	char lcmd[255];
	char ecmd[128];
	char ccmd[64];
	char mcmd[64];
	char txtch[16];
	u32 index = 0;
	u8 y = 0, l = 0, d = 0, nindex = 0, pindex = 0, dindex = 0;
	u8 fwupd[5];
        kattach_fw_m_chain_t *mchain;

        if (kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_MANGLE_INPUT)) {
                fwupd[0] = 1;
		kattach_fw.fw_update ^= KATTACH_FW_CH_MANGLE_INPUT;
        } else {
                fwupd[0] = 0;
        }

        if (kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_MANGLE_OUTPUT)) {
                fwupd[1] = 1;
		kattach_fw.fw_update ^= KATTACH_FW_CH_MANGLE_OUTPUT;
        } else {
                fwupd[1] = 0;
        }

        if (kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_MANGLE_FORWARD)) {
                fwupd[2] = 1;
		kattach_fw.fw_update ^= KATTACH_FW_CH_MANGLE_FORWARD;
        } else {
                fwupd[2] = 0;
        }

        if (kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_MANGLE_PREROUTING)) {
                fwupd[3] = 1;
		kattach_fw.fw_update ^= KATTACH_FW_CH_MANGLE_PREROUTING;
        } else {
                fwupd[3] = 0;
        }

        if (kattach_fw.fw_update == (kattach_fw.fw_update | KATTACH_FW_CH_MANGLE_POSTROUTING)) {
                fwupd[4] = 1;
		kattach_fw.fw_update ^= KATTACH_FW_CH_MANGLE_POSTROUTING;
        } else {
                fwupd[4] = 0;
        }


	/* mangle */
	while (!y) {
                if ((l == 0) && (fwupd[0] == 0)) {
                        l = 1;
                }
                if ((l == 1) && (fwupd[1] == 0)) {
                        l = 2;
                }
                if ((l == 2) && (fwupd[2] == 0)) {
			l = 3;
                }
                if ((l == 3) && (fwupd[3] == 0)) {
			l = 4;
                }
                if ((l == 4) && (fwupd[4] == 0)) {
			y++;
			break;
                }

                if (l == 0) {
                        mchain = &kattach_fw.mangle.input;
			sprintf(txtch,"INPUT");
                } else if (l == 1) {
                        mchain = &kattach_fw.mangle.output;
			sprintf(txtch,"OUTPUT");
                } else if (l == 2) {
                        mchain = &kattach_fw.mangle.forward;
			sprintf(txtch,"FORWARD");
                } else if (l == 3) {
                        mchain = &kattach_fw.mangle.prerouting;
			sprintf(txtch,"PREROUTING");
                } else if (l == 4) {
                        mchain = &kattach_fw.mangle.postrouting;
			sprintf(txtch,"POSTROUTING");
                } else {
                        y++;
                        break;
                }
		/* Flush the chain and rebuild */
		sprintf(lcmd,"%s -t mangle -F %s",KCMD_IPT,txtch);
		kattach_sysexec(lcmd);
		index = mchain->hindex;
		d = 0;
		while (!d) {
			if (mchain->filter[index].enabled) {
				for (pindex = 0; pindex < kattach_fw.apps.app[mchain->filter[index].appindex].pindex; pindex++) {
					if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].direction == KATTACH_FW_DIR_SOURCE) {
						if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[mchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[mchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m icmp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (mchain->filter[index].logging) {
										sprintf(lcmd,"%s -t mangle -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0], ecmd,
											mchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(mchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_MARK:
											if (mchain->filter[index].ttl[0] == 3) {
												sprintf(ccmd,"-m ttl --ttl-eq %u -j MARK --set-mark %lu", mchain->filter[index].ttl[1], mchain->filter[index].mark);
											} else if (mchain->filter[index].tos[0] == 1) {
												sprintf(ccmd,"-m tos --tos 0x%x -j MARK --set-mark %lu", mchain->filter[index].tos[1], mchain->filter[index].mark);
											} else {
												sprintf(ccmd,"-j MARK --set-mark %lu", mchain->filter[index].mark);
											}
											break;

										case KATTACH_FW_ACTION_TTL:
											switch (mchain->filter[index].ttl[0]) {
												case 1:
													sprintf(ccmd,"-j TTL --ttl-dec %u", mchain->filter[index].ttl[1]);
													break;

												case 2:
													sprintf(ccmd,"-j TTL --ttl-inc %u", mchain->filter[index].ttl[1]);
													break;

												case 0:
												default:
													sprintf(ccmd,"-j TTL --set-ttl %u", mchain->filter[index].ttl[1]);
													break;
											}
											break;

										case KATTACH_FW_ACTION_TOS:
											sprintf(ccmd,"-j TOS --set-tos 0x%x",mchain->filter[index].tos[1]);
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu'",txtch,index);
											break;
									}
									if ((mchain->filter[index].rlimitpkt != 0) && (mchain->filter[index].rlimitint != 0)) {
											if (mchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec", mchain->filter[index].rlimitpkt);
											} else if ((mchain->filter[index].rlimitint >= 60) && (mchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min", mchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour", mchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t mangle -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_TCP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[mchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[mchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m tcp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (mchain->filter[index].logging) {
										sprintf(lcmd,"%s -t mangle -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], ecmd,
											mchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(mchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_MARK:
											if (mchain->filter[index].ttl[0] == 3) {
												sprintf(ccmd,"-m ttl --ttl-eq %u -j MARK --set-mark %lu", mchain->filter[index].ttl[1], mchain->filter[index].mark);
											} else if (mchain->filter[index].tos[0] == 1) {
												sprintf(ccmd,"-m tos --tos 0x%x -j MARK --set-mark %lu", mchain->filter[index].tos[1], mchain->filter[index].mark);
											} else {
												sprintf(ccmd,"-j MARK --set-mark %lu", mchain->filter[index].mark);
											}
											break;

										case KATTACH_FW_ACTION_TTL:
											switch (mchain->filter[index].ttl[0]) {
												case 1:
													sprintf(ccmd,"-j TTL --ttl-dec %u", mchain->filter[index].ttl[1]);
													break;

												case 2:
													sprintf(ccmd,"-j TTL --ttl-inc %u", mchain->filter[index].ttl[1]);
													break;

												case 0:
												default:
													sprintf(ccmd,"-j TTL --set-ttl %u", mchain->filter[index].ttl[1]);
													break;
											}
											break;

										case KATTACH_FW_ACTION_TOS:
											sprintf(ccmd,"-j TOS --set-tos 0x%x",mchain->filter[index].tos[1]);
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((mchain->filter[index].rlimitpkt != 0) && (mchain->filter[index].rlimitint != 0)) {
											if (mchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",mchain->filter[index].rlimitpkt);
											} else if ((mchain->filter[index].rlimitint >= 60) && (mchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",mchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",mchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t mangle -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_UDP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[mchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[mchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m udp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (mchain->filter[index].logging) {
										sprintf(lcmd,"%s -t mangle -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], ecmd,
											mchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(mchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_MARK:
											if (mchain->filter[index].ttl[0] == 3) {
												sprintf(ccmd,"-m ttl --ttl-eq %u -j MARK --set-mark %lu", mchain->filter[index].ttl[1], mchain->filter[index].mark);
											} else if (mchain->filter[index].tos[0] == 1) {
												sprintf(ccmd,"-m tos --tos 0x%x -j MARK --set-mark %lu", mchain->filter[index].tos[1], mchain->filter[index].mark);
											} else {
												sprintf(ccmd,"-j MARK --set-mark %lu", mchain->filter[index].mark);
											}
											break;

										case KATTACH_FW_ACTION_TTL:
											switch (mchain->filter[index].ttl[0]) {
												case 1:
													sprintf(ccmd,"-j TTL --ttl-dec %u", mchain->filter[index].ttl[1]);
													break;

												case 2:
													sprintf(ccmd,"-j TTL --ttl-inc %u", mchain->filter[index].ttl[1]);
													break;

												case 0:
												default:
													sprintf(ccmd,"-j TTL --set-ttl %u", mchain->filter[index].ttl[1]);
													break;
											}
											break;

										case KATTACH_FW_ACTION_TOS:
											sprintf(ccmd,"-j TOS --set-tos 0x%x",mchain->filter[index].tos[1]);
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((mchain->filter[index].rlimitpkt != 0) && (mchain->filter[index].rlimitint != 0)) {
											if (mchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",mchain->filter[index].rlimitpkt);
											} else if ((mchain->filter[index].rlimitint >= 60) && (mchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",mchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",mchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t mangle -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}

						}
					} else if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].direction == KATTACH_FW_DIR_DESTINATION) {
						if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[mchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[mchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m icmp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (mchain->filter[index].logging) {
										sprintf(lcmd,"%s -t mangle -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0], ecmd,
											mchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(mchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_MARK:
											if (mchain->filter[index].ttl[0] == 3) {
												sprintf(ccmd,"-m ttl --ttl-eq %u -j MARK --set-mark %lu", mchain->filter[index].ttl[1], mchain->filter[index].mark);
											} else if (mchain->filter[index].tos[0] == 1) {
												sprintf(ccmd,"-m tos --tos 0x%x -j MARK --set-mark %lu", mchain->filter[index].tos[1], mchain->filter[index].mark);
											} else {
												sprintf(ccmd,"-j MARK --set-mark %lu", mchain->filter[index].mark);
											}
											break;

										case KATTACH_FW_ACTION_TTL:
											switch (mchain->filter[index].ttl[0]) {
												case 1:
													sprintf(ccmd,"-j TTL --ttl-dec %u", mchain->filter[index].ttl[1]);
													break;

												case 2:
													sprintf(ccmd,"-j TTL --ttl-inc %u", mchain->filter[index].ttl[1]);
													break;

												case 0:
												default:
													sprintf(ccmd,"-j TTL --set-ttl %u", mchain->filter[index].ttl[1]);
													break;
											}
											break;

										case KATTACH_FW_ACTION_TOS:
											sprintf(ccmd,"-j TOS --set-tos 0x%x",mchain->filter[index].tos[1]);
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((mchain->filter[index].rlimitpkt != 0) && (mchain->filter[index].rlimitint != 0)) {
											if (mchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec", mchain->filter[index].rlimitpkt);
											} else if ((mchain->filter[index].rlimitint >= 60) && (mchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min", mchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour", mchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t mangle -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_TCP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[mchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[mchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m tcp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (mchain->filter[index].logging) {
										sprintf(lcmd,"%s -t mangle -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], ecmd,
											mchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(mchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_MARK:
											if (mchain->filter[index].ttl[0] == 3) {
												sprintf(ccmd,"-m ttl --ttl-eq %u -j MARK --set-mark %lu", mchain->filter[index].ttl[1], mchain->filter[index].mark);
											} else if (mchain->filter[index].tos[0] == 1) {
												sprintf(ccmd,"-m tos --tos 0x%x -j MARK --set-mark %lu", mchain->filter[index].tos[1], mchain->filter[index].mark);
											} else {
												sprintf(ccmd,"-j MARK --set-mark %lu", mchain->filter[index].mark);
											}
											break;

										case KATTACH_FW_ACTION_TTL:
											switch (mchain->filter[index].ttl[0]) {
												case 1:
													sprintf(ccmd,"-j TTL --ttl-dec %u", mchain->filter[index].ttl[1]);
													break;

												case 2:
													sprintf(ccmd,"-j TTL --ttl-inc %u", mchain->filter[index].ttl[1]);
													break;

												case 0:
												default:
													sprintf(ccmd,"-j TTL --set-ttl %u", mchain->filter[index].ttl[1]);
													break;
											}
											break;

										case KATTACH_FW_ACTION_TOS:
											sprintf(ccmd,"-j TOS --set-tos 0x%x",mchain->filter[index].tos[1]);
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((mchain->filter[index].rlimitpkt != 0) && (mchain->filter[index].rlimitint != 0)) {
											if (mchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",mchain->filter[index].rlimitpkt);
											} else if ((mchain->filter[index].rlimitint >= 60) && (mchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",mchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",mchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t mangle -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_UDP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[mchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[mchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m udp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (mchain->filter[index].logging) {
										sprintf(lcmd,"%s -t mangle -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], ecmd,
											mchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(mchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_MARK:
											if (mchain->filter[index].ttl[0] == 3) {
												sprintf(ccmd,"-m ttl --ttl-eq %u -j MARK --set-mark %lu", mchain->filter[index].ttl[1], mchain->filter[index].mark);
											} else if (mchain->filter[index].tos[0] == 1) {
												sprintf(ccmd,"-m tos --tos 0x%x -j MARK --set-mark %lu", mchain->filter[index].tos[1], mchain->filter[index].mark);
											} else {
												sprintf(ccmd,"-j MARK --set-mark %lu", mchain->filter[index].mark);
											}
											break;

										case KATTACH_FW_ACTION_TTL:
											switch (mchain->filter[index].ttl[0]) {
												case 1:
													sprintf(ccmd,"-j TTL --ttl-dec %u", mchain->filter[index].ttl[1]);
													break;

												case 2:
													sprintf(ccmd,"-j TTL --ttl-inc %u", mchain->filter[index].ttl[1]);
													break;

												case 0:
												default:
													sprintf(ccmd,"-j TTL --set-ttl %u", mchain->filter[index].ttl[1]);
													break;
											}
											break;

										case KATTACH_FW_ACTION_TOS:
											sprintf(ccmd,"-j TOS --set-tos 0x%x",mchain->filter[index].tos[1]);
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((mchain->filter[index].rlimitpkt != 0) && (mchain->filter[index].rlimitint != 0)) {
											if (mchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",mchain->filter[index].rlimitpkt);
											} else if ((mchain->filter[index].rlimitint >= 60) && (mchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",mchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",mchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t mangle -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --dport %u:%u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}

						}
					} else if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].direction == KATTACH_FW_DIR_BOTH) {
						if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_ICMP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[mchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[mchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m icmp -m state --state NEW");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m icmp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m icmp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m icmp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (mchain->filter[index].logging) {
										sprintf(lcmd,"%s -t mangle -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0], ecmd,
											mchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(mchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_MARK:
											if (mchain->filter[index].ttl[0] == 3) {
												sprintf(ccmd,"-m ttl --ttl-eq %u -j MARK --set-mark %lu", mchain->filter[index].ttl[1], mchain->filter[index].mark);
											} else if (mchain->filter[index].tos[0] == 1) {
												sprintf(ccmd,"-m tos --tos 0x%x -j MARK --set-mark %lu", mchain->filter[index].tos[1], mchain->filter[index].mark);
											} else {
												sprintf(ccmd,"-j MARK --set-mark %lu", mchain->filter[index].mark);
											}
											break;

										case KATTACH_FW_ACTION_TTL:
											switch (mchain->filter[index].ttl[0]) {
												case 1:
													sprintf(ccmd,"-j TTL --ttl-dec %u", mchain->filter[index].ttl[1]);
													break;

												case 2:
													sprintf(ccmd,"-j TTL --ttl-inc %u", mchain->filter[index].ttl[1]);
													break;

												case 0:
												default:
													sprintf(ccmd,"-j TTL --set-ttl %u", mchain->filter[index].ttl[1]);
													break;
											}
											break;

										case KATTACH_FW_ACTION_TOS:
											sprintf(ccmd,"-j TOS --set-tos 0x%x",mchain->filter[index].tos[1]);
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((mchain->filter[index].rlimitpkt != 0) && (mchain->filter[index].rlimitint != 0)) {
											if (mchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec", mchain->filter[index].rlimitpkt);
											} else if ((mchain->filter[index].rlimitint >= 60) && (mchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min", mchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour", mchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t mangle -A %s -p icmp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --icmp-type %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_TCP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[mchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[mchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m tcp -m state --state NEW");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m tcp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m tcp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m tcp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (mchain->filter[index].logging) {
										sprintf(lcmd,"%s -t mangle -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], ecmd,
											mchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(mchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_MARK:
											if (mchain->filter[index].ttl[0] == 3) {
												sprintf(ccmd,"-m ttl --ttl-eq %u -j MARK --set-mark %lu", mchain->filter[index].ttl[1], mchain->filter[index].mark);
											} else if (mchain->filter[index].tos[0] == 1) {
												sprintf(ccmd,"-m tos --tos 0x%x -j MARK --set-mark %lu", mchain->filter[index].tos[1], mchain->filter[index].mark);
											} else {
												sprintf(ccmd,"-j MARK --set-mark %lu", mchain->filter[index].mark);
											}
											break;

										case KATTACH_FW_ACTION_TTL:
											switch (mchain->filter[index].ttl[0]) {
												case 1:
													sprintf(ccmd,"-j TTL --ttl-dec %u", mchain->filter[index].ttl[1]);
													break;

												case 2:
													sprintf(ccmd,"-j TTL --ttl-inc %u", mchain->filter[index].ttl[1]);
													break;

												case 0:
												default:
													sprintf(ccmd,"-j TTL --set-ttl %u", mchain->filter[index].ttl[1]);
													break;
											}
											break;

										case KATTACH_FW_ACTION_TOS:
											sprintf(ccmd,"-j TOS --set-tos 0x%x",mchain->filter[index].tos[1]);
											break;

										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((mchain->filter[index].rlimitpkt != 0) && (mchain->filter[index].rlimitint != 0)) {
											if (mchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",mchain->filter[index].rlimitpkt);
											} else if ((mchain->filter[index].rlimitint >= 60) && (mchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",mchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",mchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t mangle -A %s -p tcp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}
						}
						if (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask == (kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].protmask | KATTACH_FW_PROTOCOL_UDP)) {
							for (nindex = 0; nindex < kattach_fw.zones.zone[mchain->filter[index].szindex].nindex; nindex++) {
								for (dindex = 0; dindex < kattach_fw.zones.zone[mchain->filter[index].dzindex].nindex; dindex++) {
									if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state NEW,ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED)) &&
										(kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW,RELATED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_NEW))) {
										sprintf(ecmd," -m udp -m state --state NEW");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_ESTABLISHED))) {
										sprintf(ecmd," -m udp -m state --state ESTABLISHED");
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_INVALID))) {
										sprintf(ecmd," -m udp -m state --state INVALID");	
									} else if ((kattach_fw.apps.app[mchain->filter[index].appindex].statemask == (kattach_fw.apps.app[mchain->filter[index].appindex].statemask | KATTACH_FW_STMASK_RELATED))) {
										sprintf(ecmd," -m udp -m state --state RELATED");
									} else {
										/* FIXME: */
										sprintf(ecmd," ");
									}
									if (mchain->filter[index].logging) {
										sprintf(lcmd,"%s -t mangle -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s -j LOG --log-prefix '%s'",
											KCMD_IPT,txtch,
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
											((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
											kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
											kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], ecmd,
											mchain->filter[index].logprefix);
										kattach_sysexec(lcmd);
									}
									switch(mchain->filter[index].action) {
										case KATTACH_FW_ACTION_ALLOW:
											sprintf(ccmd,"-j ACCEPT");
											break;

										case KATTACH_FW_ACTION_DROP:
											sprintf(ccmd,"-j DROP");
											break;

										case KATTACH_FW_ACTION_REJECT:
											sprintf(ccmd,"-j REJECT");
											break;

										case KATTACH_FW_ACTION_MARK:
											if (mchain->filter[index].ttl[0] == 3) {
												sprintf(ccmd,"-m ttl --ttl-eq %u -j MARK --set-mark %lu", mchain->filter[index].ttl[1], mchain->filter[index].mark);
											} else if (mchain->filter[index].tos[0] == 1) {
												sprintf(ccmd,"-m tos --tos 0x%x -j MARK --set-mark %lu", mchain->filter[index].tos[1], mchain->filter[index].mark);
											} else {
												sprintf(ccmd,"-j MARK --set-mark %lu", mchain->filter[index].mark);
											}
											break;

										case KATTACH_FW_ACTION_TTL:
											switch (mchain->filter[index].ttl[0]) {
												case 1:
													sprintf(ccmd,"-j TTL --ttl-dec %u", mchain->filter[index].ttl[1]);
													break;

												case 2:
													sprintf(ccmd,"-j TTL --ttl-inc %u", mchain->filter[index].ttl[1]);
													break;

												case 0:
												default:
													sprintf(ccmd,"-j TTL --set-ttl %u", mchain->filter[index].ttl[1]);
													break;
											}
											break;

										case KATTACH_FW_ACTION_TOS:
											sprintf(ccmd,"-j TOS --set-tos 0x%x",mchain->filter[index].tos[1]);
											break;


										case KATTACH_FW_ACTION_LOG:
										default:
											sprintf(ccmd,"-j LOG --log-prefix 'filter_%s_%lu: '",txtch,index);
											break;
									}
									if ((mchain->filter[index].rlimitpkt != 0) && (mchain->filter[index].rlimitint != 0)) {
											if (mchain->filter[index].rlimitint < 60) {
												sprintf(mcmd, "-m limit --limit %lu/sec",mchain->filter[index].rlimitpkt);
											} else if ((mchain->filter[index].rlimitint >= 60) && (mchain->filter[index].rlimitint < 3600)) {
												sprintf(mcmd, "-m limit --limit %lu/min",mchain->filter[index].rlimitpkt);
											} else {
												sprintf(mcmd, "-m limit --limit %lu/hour",mchain->filter[index].rlimitpkt);
											}		
									} else {
										sprintf(mcmd," ");
									}
									sprintf(lcmd,"%s -t mangle -A %s -p udp -s %lu.%lu.%lu.%lu/%u -d %lu.%lu.%lu.%lu/%u --sport %u --dport %u %s %s %s",
										KCMD_IPT,txtch,
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].szindex].node[nindex].mask,
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 24) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 16) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip >> 8) & 0xff),
										((kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].ip) & 0xff),
										kattach_fw.zones.zone[mchain->filter[index].dzindex].node[dindex].mask,
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[0],
										kattach_fw.apps.app[mchain->filter[index].appindex].port[pindex].port[1], mcmd, ecmd, ccmd);
									kattach_sysexec(lcmd);
								}
							}

						}
					}
				}

			}
			if (index == mchain->eindex) {
				d++;
				break;
			} else {
				index = mchain->filter[index].nindex;
			}
			
		}
		l++;
	}

	kattach_vm_fw_syncdb_mangle();
	kattach_vm_fw_syncdb_fwmain();
	return;
}

void
kattach_vm_fw_update_apps(void)
{
	/* update SQL */
	kattach_vm_fw_syncdb_apps();
	kattach_vm_fw_syncdb_fwmain();

	/* FIXME: rescan firewall rules that use these apps, if found in a table, must flag and update table */
	return;
}

void
kattach_vm_fw_update_zones(void)
{
	/* update SQL */
	kattach_vm_fw_syncdb_zones();
	kattach_vm_fw_syncdb_fwmain();

	/* FIXME: rescan firewall rules that use these zones, if found in a table, must flag and update table */
	return;
}

void
kattach_vm_fw_syncdb(void)
{
	/* this function re-syncs the database with whats in memory */

	kattach_vm_fw_syncdb_zones();
	kattach_vm_fw_syncdb_apps();
	kattach_vm_fw_syncdb_fwmain();
	kattach_vm_fw_syncdb_filter();
	kattach_vm_fw_syncdb_nat();
	kattach_vm_fw_syncdb_mangle();

	return;
}

void
kattach_vm_fw_syncdb_zones(void)
{
	u32 index, ncount = 0;
	u8 rc = 0, nindex = 0;
	char sqlq[1024];

	/* FIXME: error handling */
	rc = kattach_sql_testdb(CMSQL_VMSESS_DROP_Z3, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_DROP_Z2, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_DROP_Z1, K_DB_VMSESSION);

	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_ZNODES, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_ZONES, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_ZLINK, K_DB_VMSESSION);

	for (index = 0; index < kattach_fw.zones.index; index++) {
		if (kattach_fw.zones.zone[index].name[0] == '\0') continue;
		sprintf(sqlq,"INSERT into zones values (%lu,'%s',%u);",(1 + index), kattach_fw.zones.zone[index].name, kattach_fw.zones.zone[index].vlan);
		rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
		for (nindex = 0; nindex < kattach_fw.zones.zone[index].nindex; nindex++) {
			sprintf(sqlq,"INSERT into znodes values (%lu,%lu,%u);",(1 + ncount), kattach_fw.zones.zone[index].node[nindex].ip,
				kattach_fw.zones.zone[index].node[nindex].mask);
			rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
			sprintf(sqlq,"INSERT into zlink (zone,node) values (%lu,%lu);",(1 + index), (1 + ncount));
			rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
			ncount++;
		}
	}

	return;
}

void
kattach_vm_fw_syncdb_apps(void)
{
	u32 index, acount = 0;
	u8 rc = 0, aindex = 0;
	char sqlq[1024];

	/* FIXME: error handling */
	rc = kattach_sql_testdb(CMSQL_VMSESS_DROP_A3, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_DROP_A2, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_DROP_A1, K_DB_VMSESSION);

	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_APPS, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_APPPORTS, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_ALINK, K_DB_VMSESSION);


	for (index = 0; index < kattach_fw.apps.index; index++) {
		if (kattach_fw.apps.app[index].name[0] == '\0') continue;
		sprintf(sqlq,"INSERT into apps values (%lu,'%s',%u);",(1 + index), kattach_fw.apps.app[index].name, kattach_fw.apps.app[index].statemask);
		rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
		for (aindex = 0; aindex < kattach_fw.apps.app[index].pindex; aindex++) {
			sprintf(sqlq,"INSERT into appports values (%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u);",(1 + acount), kattach_fw.apps.app[index].port[aindex].port[0],
				kattach_fw.apps.app[index].port[aindex].port[1],
				kattach_fw.apps.app[index].port[aindex].protmask,
				kattach_fw.apps.app[index].port[aindex].direction,
				kattach_fw.apps.app[index].port[aindex].tcp_flags.all,
				kattach_fw.apps.app[index].port[aindex].tcp_flags.none,
				kattach_fw.apps.app[index].port[aindex].tcp_flags.syn,
				kattach_fw.apps.app[index].port[aindex].tcp_flags.ack,
				kattach_fw.apps.app[index].port[aindex].tcp_flags.fin,
				kattach_fw.apps.app[index].port[aindex].tcp_flags.reset,
				kattach_fw.apps.app[index].port[aindex].tcp_flags.push,
				kattach_fw.apps.app[index].port[aindex].tcp_flags.urgent);
			rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
			sprintf(sqlq,"INSERT into alink (app,port) values (%lu,%lu);",(1 + index), (1 + acount));
			rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
			acount++;
		}
	}

	return;
}

void
kattach_vm_fw_syncdb_fwmain(void)
{
	u8 rc = 0;
	char sqlq[1024];

	rc = kattach_sql_testdb(CMSQL_VMSESS_DROP_FWMAIN, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWMAIN, K_DB_VMSESSION);

	sprintf(sqlq,"INSERT into fwmain values (%u,%lu,%lu,%lu);",1,kattach_fw.filter.input.hindex,kattach_fw.filter.input.eindex,kattach_fw.filter.input.index);
	rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	sprintf(sqlq,"INSERT into fwmain values (%u,%lu,%lu,%lu);",2,kattach_fw.filter.output.hindex,kattach_fw.filter.output.eindex,kattach_fw.filter.output.index);
	rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	sprintf(sqlq,"INSERT into fwmain values (%u,%lu,%lu,%lu);",3,kattach_fw.filter.forward.hindex,kattach_fw.filter.forward.eindex,kattach_fw.filter.forward.index);
	rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	sprintf(sqlq,"INSERT into fwmain values (%u,%lu,%lu,%lu);",4,kattach_fw.nat.postrouting.hindex,kattach_fw.nat.postrouting.eindex,kattach_fw.nat.postrouting.index);
	rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	sprintf(sqlq,"INSERT into fwmain values (%u,%lu,%lu,%lu);",5,kattach_fw.nat.prerouting.hindex,kattach_fw.nat.prerouting.eindex,kattach_fw.nat.prerouting.index);
	rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	sprintf(sqlq,"INSERT into fwmain values (%u,%lu,%lu,%lu);",6,kattach_fw.nat.output.hindex,kattach_fw.nat.output.eindex,kattach_fw.nat.output.index);
	rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	sprintf(sqlq,"INSERT into fwmain values (%u,%lu,%lu,%lu);",7,kattach_fw.mangle.input.hindex,kattach_fw.mangle.input.eindex,kattach_fw.mangle.input.index);
	rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	sprintf(sqlq,"INSERT into fwmain values (%u,%lu,%lu,%lu);",8,kattach_fw.mangle.output.hindex,kattach_fw.mangle.output.eindex,kattach_fw.mangle.output.index);
	rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	sprintf(sqlq,"INSERT into fwmain values (%u,%lu,%lu,%lu);",9,kattach_fw.mangle.forward.hindex,kattach_fw.mangle.forward.eindex,kattach_fw.mangle.forward.index);
	rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	sprintf(sqlq,"INSERT into fwmain values (%u,%lu,%lu,%lu);",10,kattach_fw.mangle.prerouting.hindex,kattach_fw.mangle.prerouting.eindex,kattach_fw.mangle.prerouting.index);
	rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	sprintf(sqlq,"INSERT into fwmain values (%u,%lu,%lu,%lu);",11,kattach_fw.mangle.postrouting.hindex,kattach_fw.mangle.postrouting.eindex,kattach_fw.mangle.postrouting.index);
	rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);

	return;
}

void
kattach_vm_fw_syncdb_filter(void)
{
	u32 index = 0;
	u8 rc = 0;
	char sqlq[1024];
	kattach_fw_chain_t *chain;

	rc = kattach_sql_testdb(CMSQL_VMSESS_DROP_F1, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_DROP_F2, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_DROP_F3, K_DB_VMSESSION);

	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWFINPUT, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWFOUTPUT, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWFFORWARD, K_DB_VMSESSION);


	chain = &kattach_fw.filter.input;
	for (index = 0; index < chain->index; index++) {
		sprintf(sqlq,"INSERT into fwfinput (pindex,nindex,szindex,dzindex,appindex,rlimitpkt,rlimitint,action,ttlA,ttlB,tosA,tosB,enabled,ruletype,rejectwith,reverse,logging,logprefix) values (%lu,%lu,%lu,%lu,%lu,%lu,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,'%s');",
			chain->filter[index].pindex, chain->filter[index].nindex, (1 + chain->filter[index].szindex), (1 + chain->filter[index].dzindex),
			(1 + chain->filter[index].appindex), chain->filter[index].rlimitpkt, chain->filter[index].rlimitint, chain->filter[index].action,
			chain->filter[index].ttl[0], chain->filter[index].ttl[1], chain->filter[index].tos[0], chain->filter[index].tos[1],
			chain->filter[index].enabled, chain->filter[index].type, chain->filter[index].rejectwith, chain->filter[index].reverse,
			chain->filter[index].logging,chain->filter[index].logprefix);
		rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	}

	chain = &kattach_fw.filter.output;
	for (index = 0; index < chain->index; index++) {
		sprintf(sqlq,"INSERT into fwfoutput (pindex,nindex,szindex,dzindex,appindex,rlimitpkt,rlimitint,action,ttlA,ttlB,tosA,tosB,enabled,ruletype,rejectwith,reverse,logging,logprefix) values (%lu,%lu,%lu,%lu,%lu,%lu,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,'%s');",
			chain->filter[index].pindex, chain->filter[index].nindex, (1 + chain->filter[index].szindex), (1 + chain->filter[index].dzindex),
			(1 + chain->filter[index].appindex), chain->filter[index].rlimitpkt, chain->filter[index].rlimitint, chain->filter[index].action,
			chain->filter[index].ttl[0], chain->filter[index].ttl[1], chain->filter[index].tos[0], chain->filter[index].tos[1],
			chain->filter[index].enabled, chain->filter[index].type, chain->filter[index].rejectwith, chain->filter[index].reverse,
			chain->filter[index].logging,chain->filter[index].logprefix);
		rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	}

	chain = &kattach_fw.filter.forward;
	for (index = 0; index < chain->index; index++) {
		sprintf(sqlq,"INSERT into fwfforward (pindex,nindex,szindex,dzindex,appindex,rlimitpkt,rlimitint,action,ttlA,ttlB,tosA,tosB,enabled,ruletype,rejectwith,reverse,logging,logprefix) values (%lu,%lu,%lu,%lu,%lu,%lu,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,'%s');",
			chain->filter[index].pindex, chain->filter[index].nindex, (1 + chain->filter[index].szindex), (1 + chain->filter[index].dzindex),
			(1 + chain->filter[index].appindex), chain->filter[index].rlimitpkt, chain->filter[index].rlimitint, chain->filter[index].action,
			chain->filter[index].ttl[0], chain->filter[index].ttl[1], chain->filter[index].tos[0], chain->filter[index].tos[1],
			chain->filter[index].enabled, chain->filter[index].type, chain->filter[index].rejectwith, chain->filter[index].reverse,
			chain->filter[index].logging,chain->filter[index].logprefix);
		rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	}

	return;

}

void
kattach_vm_fw_syncdb_nat(void)
{
	u32 index = 0;
	u8 rc = 0;
	char sqlq[1024];
	kattach_fw_n_chain_t *chain;

	rc = kattach_sql_testdb(CMSQL_VMSESS_DROP_F4, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_DROP_F5, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_DROP_F6, K_DB_VMSESSION);

	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWNPOSTROUTING, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWNPREROUTING, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWNOUTPUT, K_DB_VMSESSION);

	chain = &kattach_fw.nat.postrouting;
	for (index = 0; index < chain->index; index++) {
		sprintf(sqlq,"INSERT into fwnpostrouting (pindex,nindex,szindex,dzindex,nzindex,appindex,nappindex,rlimitpkt,rlimitint,action,ttlA,ttlB,tosA,tosB,enabled,ruletype,rejectwith,reverse,logging,logprefix) values (%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,'%s');",
			chain->filter[index].pindex, chain->filter[index].nindex, (1 + chain->filter[index].szindex), (1 + chain->filter[index].dzindex), (1 + chain->filter[index].nzindex),
			(1 + chain->filter[index].appindex), (1 + chain->filter[index].nappindex), chain->filter[index].rlimitpkt, chain->filter[index].rlimitint, chain->filter[index].action,
			chain->filter[index].ttl[0], chain->filter[index].ttl[1], chain->filter[index].tos[0], chain->filter[index].tos[1],
			chain->filter[index].enabled, chain->filter[index].type, chain->filter[index].rejectwith, chain->filter[index].reverse,
			chain->filter[index].logging,chain->filter[index].logprefix);
		rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	}

	chain = &kattach_fw.nat.prerouting;
	for (index = 0; index < chain->index; index++) {
		sprintf(sqlq,"INSERT into fwnprerouting (pindex,nindex,szindex,dzindex,nzindex,appindex,nappindex,rlimitpkt,rlimitint,action,ttlA,ttlB,tosA,tosB,enabled,ruletype,rejectwith,reverse,logging,logprefix) values (%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,'%s');",
			chain->filter[index].pindex, chain->filter[index].nindex, (1 + chain->filter[index].szindex), (1 + chain->filter[index].dzindex), (1 + chain->filter[index].nzindex),
			(1 + chain->filter[index].appindex), (1 + chain->filter[index].nappindex), chain->filter[index].rlimitpkt, chain->filter[index].rlimitint, chain->filter[index].action,
			chain->filter[index].ttl[0], chain->filter[index].ttl[1], chain->filter[index].tos[0], chain->filter[index].tos[1],
			chain->filter[index].enabled, chain->filter[index].type, chain->filter[index].rejectwith, chain->filter[index].reverse,
			chain->filter[index].logging,chain->filter[index].logprefix);
		rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	}

	chain = &kattach_fw.nat.output;
	for (index = 0; index < chain->index; index++) {
		sprintf(sqlq,"INSERT into fwnoutput (pindex,nindex,szindex,dzindex,nzindex,appindex,nappindex,rlimitpkt,rlimitint,action,ttlA,ttlB,tosA,tosB,enabled,ruletype,rejectwith,reverse,logging,logprefix) values (%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,'%s');",
			chain->filter[index].pindex, chain->filter[index].nindex, (1 + chain->filter[index].szindex), (1 + chain->filter[index].dzindex), (1 + chain->filter[index].nzindex),
			(1 + chain->filter[index].appindex), (1 + chain->filter[index].nappindex), chain->filter[index].rlimitpkt, chain->filter[index].rlimitint, chain->filter[index].action,
			chain->filter[index].ttl[0], chain->filter[index].ttl[1], chain->filter[index].tos[0], chain->filter[index].tos[1],
			chain->filter[index].enabled, chain->filter[index].type, chain->filter[index].rejectwith, chain->filter[index].reverse,
			chain->filter[index].logging,chain->filter[index].logprefix);
		rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	}


	return;

}

void
kattach_vm_fw_syncdb_mangle(void)
{
	u32 index = 0;
	u8 rc = 0;
	char sqlq[1024];
	kattach_fw_m_chain_t *chain;

	rc = kattach_sql_testdb(CMSQL_VMSESS_DROP_F7, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_DROP_F8, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_DROP_F9, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_DROP_F10, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_DROP_F11, K_DB_VMSESSION);

	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWMINPUT, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWMOUTPUT, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWMFORWARD, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWMPREROUTING, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWMPOSTROUTING, K_DB_VMSESSION);

	chain = &kattach_fw.mangle.input;
	for (index = 0; index < chain->index; index++) {
		sprintf(sqlq,"INSERT into fwminput (pindex,nindex,szindex,dzindex,appindex,pktmark,rlimitpkt,rlimitint,action,ttlA,ttlB,tosA,tosB,enabled,ruletype,rejectwith,reverse,logging,logprefix) values (%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,'%s');",
			chain->filter[index].pindex, chain->filter[index].nindex, (1 + chain->filter[index].szindex), (1 + chain->filter[index].dzindex),
			(1 + chain->filter[index].appindex), chain->filter[index].mark, chain->filter[index].rlimitpkt, chain->filter[index].rlimitint, chain->filter[index].action,
			chain->filter[index].ttl[0], chain->filter[index].ttl[1], chain->filter[index].tos[0], chain->filter[index].tos[1],
			chain->filter[index].enabled, chain->filter[index].type, chain->filter[index].rejectwith, chain->filter[index].reverse,
			chain->filter[index].logging,chain->filter[index].logprefix);
		rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	}

	chain = &kattach_fw.mangle.output;
	for (index = 0; index < chain->index; index++) {
		sprintf(sqlq,"INSERT into fwmoutput (pindex,nindex,szindex,dzindex,appindex,pktmark,rlimitpkt,rlimitint,action,ttlA,ttlB,tosA,tosB,enabled,ruletype,rejectwith,reverse,logging,logprefix) values (%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,'%s');",
			chain->filter[index].pindex, chain->filter[index].nindex, (1 + chain->filter[index].szindex), (1 + chain->filter[index].dzindex),
			(1 + chain->filter[index].appindex), chain->filter[index].mark, chain->filter[index].rlimitpkt, chain->filter[index].rlimitint, chain->filter[index].action,
			chain->filter[index].ttl[0], chain->filter[index].ttl[1], chain->filter[index].tos[0], chain->filter[index].tos[1],
			chain->filter[index].enabled, chain->filter[index].type, chain->filter[index].rejectwith, chain->filter[index].reverse,
			chain->filter[index].logging,chain->filter[index].logprefix);
		rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	}

	chain = &kattach_fw.mangle.forward;
	for (index = 0; index < chain->index; index++) {
		sprintf(sqlq,"INSERT into fwmforward (pindex,nindex,szindex,dzindex,appindex,pktmark,rlimitpkt,rlimitint,action,ttlA,ttlB,tosA,tosB,enabled,ruletype,rejectwith,reverse,logging,logprefix) values (%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,'%s');",
			chain->filter[index].pindex, chain->filter[index].nindex, (1 + chain->filter[index].szindex), (1 + chain->filter[index].dzindex),
			(1 + chain->filter[index].appindex), chain->filter[index].mark, chain->filter[index].rlimitpkt, chain->filter[index].rlimitint, chain->filter[index].action,
			chain->filter[index].ttl[0], chain->filter[index].ttl[1], chain->filter[index].tos[0], chain->filter[index].tos[1],
			chain->filter[index].enabled, chain->filter[index].type, chain->filter[index].rejectwith, chain->filter[index].reverse,
			chain->filter[index].logging,chain->filter[index].logprefix);
		rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	}

	chain = &kattach_fw.mangle.prerouting;
	for (index = 0; index < chain->index; index++) {
		sprintf(sqlq,"INSERT into fwmprerouting (pindex,nindex,szindex,dzindex,appindex,pktmark,rlimitpkt,rlimitint,action,ttlA,ttlB,tosA,tosB,enabled,ruletype,rejectwith,reverse,logging,logprefix) values (%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,'%s');",
			chain->filter[index].pindex, chain->filter[index].nindex, (1 + chain->filter[index].szindex), (1 + chain->filter[index].dzindex),
			(1 + chain->filter[index].appindex), chain->filter[index].mark, chain->filter[index].rlimitpkt, chain->filter[index].rlimitint, chain->filter[index].action,
			chain->filter[index].ttl[0], chain->filter[index].ttl[1], chain->filter[index].tos[0], chain->filter[index].tos[1],
			chain->filter[index].enabled, chain->filter[index].type, chain->filter[index].rejectwith, chain->filter[index].reverse,
			chain->filter[index].logging,chain->filter[index].logprefix);
		rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	}

	chain = &kattach_fw.mangle.postrouting;
	for (index = 0; index < chain->index; index++) {
		sprintf(sqlq,"INSERT into fwmpostrouting (pindex,nindex,szindex,dzindex,appindex,pktmark,rlimitpkt,rlimitint,action,ttlA,ttlB,tosA,tosB,enabled,ruletype,rejectwith,reverse,logging,logprefix) values (%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,'%s');",
			chain->filter[index].pindex, chain->filter[index].nindex, (1 + chain->filter[index].szindex), (1 + chain->filter[index].dzindex),
			(1 + chain->filter[index].appindex), chain->filter[index].mark, chain->filter[index].rlimitpkt, chain->filter[index].rlimitint, chain->filter[index].action,
			chain->filter[index].ttl[0], chain->filter[index].ttl[1], chain->filter[index].tos[0], chain->filter[index].tos[1],
			chain->filter[index].enabled, chain->filter[index].type, chain->filter[index].rejectwith, chain->filter[index].reverse,
			chain->filter[index].logging,chain->filter[index].logprefix);
		rc = kattach_vm_sql_iu(sqlq, K_DB_VMSESSION);
	}

	return;
}

void
kattach_vm_db_fw(void)
{
	u8 rc = 0;

	rc = kattach_vm_sql_vns_vsn(CMSQL_VMSESS_SELECT_VNS_VSN,K_DB_VMSESSION);
	rc = kattach_vm_sql_vns_vslink(CMSQL_VMSESS_SELECT_VNS_VSLINK,K_DB_VMSESSION);
	rc = kattach_vm_sql_fw_main(CMSQL_VMSESS_SELECT_FW_MAIN,K_DB_VMSESSION);
	rc = kattach_vm_sql_fw_zones(CMSQL_VMSESS_SELECT_FW_Z2,K_DB_VMSESSION);
	rc = kattach_vm_sql_fw_zlink(CMSQL_VMSESS_SELECT_FW_Z1,K_DB_VMSESSION);
	rc = kattach_vm_sql_fw_apps(CMSQL_VMSESS_SELECT_FW_A3,K_DB_VMSESSION);
	rc = kattach_vm_sql_fw_alink(CMSQL_VMSESS_SELECT_FW_A1,K_DB_VMSESSION);
	rc = kattach_vm_sql_fw_filter_input(CMSQL_VMSESS_SELECT_FW_F1,K_DB_VMSESSION);
	rc = kattach_vm_sql_fw_filter_output(CMSQL_VMSESS_SELECT_FW_F2,K_DB_VMSESSION);
	rc = kattach_vm_sql_fw_filter_forward(CMSQL_VMSESS_SELECT_FW_F3,K_DB_VMSESSION);
	rc = kattach_vm_sql_fw_nat_postrouting(CMSQL_VMSESS_SELECT_FW_F4,K_DB_VMSESSION);
	rc = kattach_vm_sql_fw_nat_prerouting(CMSQL_VMSESS_SELECT_FW_F5,K_DB_VMSESSION);
	rc = kattach_vm_sql_fw_nat_output(CMSQL_VMSESS_SELECT_FW_F6,K_DB_VMSESSION);
	rc = kattach_vm_sql_fw_mangle_input(CMSQL_VMSESS_SELECT_FW_F7,K_DB_VMSESSION);
	rc = kattach_vm_sql_fw_mangle_output(CMSQL_VMSESS_SELECT_FW_F8,K_DB_VMSESSION);
	rc = kattach_vm_sql_fw_mangle_forward(CMSQL_VMSESS_SELECT_FW_F9,K_DB_VMSESSION);
	rc = kattach_vm_sql_fw_mangle_prerouting(CMSQL_VMSESS_SELECT_FW_F10,K_DB_VMSESSION);
	rc = kattach_vm_sql_fw_mangle_postrouting(CMSQL_VMSESS_SELECT_FW_F11,K_DB_VMSESSION);

	return;

}

u8
kattach_vm_sql_vns_vsn(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_vns_vsn, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_vns_vsn, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_vns_vsn, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] Virtual Network Service Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_vns_vslink(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_vns_vslink, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_vns_vslink, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_vns_vslink, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] VNS Link Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}


u8
kattach_vm_sql_vns_vsp(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_vns_vsp, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_vns_vsp, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_vns_vsp, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] Virtual Service Port Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}


u8
kattach_vm_sql_fw_main(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_fwmain, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_fwmain, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_fwmain, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] Main Firewall Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_fw_znodes(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_fw_znodes, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_fw_znodes, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_fw_znodes, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] FW Zone Nodes Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_fw_zones(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_fw_zones, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_fw_zones, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_fw_zones, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] FW Zone Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_fw_zlink(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_fw_zlink, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_fw_zlink, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_fw_zlink, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] FW Zone-Node Link Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_fw_apps(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_fw_apps, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_fw_apps, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_fw_apps, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] FW Apps Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_fw_appports(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_fw_appports, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_fw_appports, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_fw_appports, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] FW App Port Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_fw_alink(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_fw_alink, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_fw_alink, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_fw_alink, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] FW App - App Port Link Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_fw_filter_input(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_fw_filter_input, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_fw_filter_input, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_fw_filter_input, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] FW Filter: Input Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_fw_filter_output(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_fw_filter_output, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_fw_filter_output, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_fw_filter_output, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] FW Filter: Output Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_fw_filter_forward(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_fw_filter_forward, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_fw_filter_forward, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_fw_filter_forward, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] FW Filter: Forward Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_fw_nat_prerouting(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_fw_nat_prerouting, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_fw_nat_prerouting, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_fw_nat_prerouting, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] FW NAT: Prerouting Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_fw_nat_postrouting(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_fw_nat_postrouting, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_fw_nat_postrouting, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_fw_nat_postrouting, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] FW NAT: Postrouting Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_fw_nat_output(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_fw_nat_output, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_fw_nat_output, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_fw_nat_output, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] FW NAT: Output Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_fw_mangle_input(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_fw_mangle_input, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_fw_mangle_input, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_fw_mangle_input, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] FW Mangle: Input Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_fw_mangle_output(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_fw_mangle_output, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_fw_mangle_output, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_fw_mangle_output, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] FW Mangle: Output Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}


u8
kattach_vm_sql_fw_mangle_forward(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_fw_mangle_forward, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_fw_mangle_forward, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_fw_mangle_forward, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] FW Mangle: Forward Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_fw_mangle_prerouting(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_fw_mangle_prerouting, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_fw_mangle_prerouting, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_fw_mangle_prerouting, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] FW Mangle: Prerouting Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

u8
kattach_vm_sql_fw_mangle_postrouting(char *sqlq, u8 dbPtr)
{
        char *dbErrMsg = 0;
        int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_vm_cb_fw_mangle_postrouting, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_vm_cb_fw_mangle_postrouting, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_vm_cb_fw_mangle_postrouting, 0, &dbErrMsg);
			break;

		default:
			printf("\n [!] Unknown Database %u (%s)\n",dbPtr,sqlq);
			break;

	}

        if (rc != SQLITE_OK) {
                printf(" [!] FW Mangle: Postrouting Table Read error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
                return (RC_FAIL);
        } else {
		sqlite3_free(dbErrMsg);
                return (RC_OK);
        }
}

static int
kattach_vm_cb_fwmain(void *NotUsed, int cols, char **results, char **colname)
{
	int i = 0;
	u8 chainindex = 0;
	char qres[512];

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("chainindex",colname[i],strlen(colname[i]))) {
			chainindex = (u8) (atoi(results[i]));
			if (chainindex > 11) {
				printf(" [!] WARNING: Database Index for Chain Exceeds Maximum (11) - %u\n", chainindex);
			}
                        continue;
		} else if (!strncmp("head",colname[i],strlen(colname[i]))) {
			switch (chainindex) {
				case 1:
					/* FW INPUT */
					kattach_fw.filter.input.hindex = (u8) atoi(results[i]);
					break;

				case 2:
					/* FW OUTPUT */
					kattach_fw.filter.output.hindex = (u8) atoi(results[i]);
					break;

				case 3:
					/* FW FORWARD */
					kattach_fw.filter.forward.hindex = (u8) atoi(results[i]);
					break;
	
				case 4:
					/* NAT POSTROUTING */
					kattach_fw.nat.postrouting.hindex = (u8) atoi(results[i]);
					break;

				case 5:
					/* NAT PREROUTING */
					kattach_fw.nat.prerouting.hindex = (u8) atoi(results[i]);
					break;

				case 6:
					/* NAT OUTPUT */
					kattach_fw.nat.output.hindex = (u8) atoi(results[i]);
					break;

				case 7:
					/* MANGLE INPUT */
					kattach_fw.mangle.input.hindex = (u8) atoi(results[i]);
					break;

				case 8:
					/* MANGLE OUTPUT */
					kattach_fw.mangle.output.hindex = (u8) atoi(results[i]);
					break;

				case 9:
					/* MANGLE FORWARD */
					kattach_fw.mangle.input.hindex = (u8) atoi(results[i]);
					break;

				case 10:
					/* MANGLE PREROUTING */
					kattach_fw.mangle.input.hindex = (u8) atoi(results[i]);
					break;

				case 11:
					/* MANGLE POSTROUTING */
					kattach_fw.mangle.input.hindex = (u8) atoi(results[i]);
					break;

				default:
					/* unknown */
					printf("\n [!] WARNING: Unknown firewall chain index - head: %u\n",chainindex);
					break;
			}
			continue;
		} else if (!strncmp("end",colname[i],strlen(colname[i]))) {
			switch (chainindex) {
				case 1:
					/* FW INPUT */
					kattach_fw.filter.input.eindex = (u8) atoi(results[i]);
					break;

				case 2:
					/* FW OUTPUT */
					kattach_fw.filter.output.eindex = (u8) atoi(results[i]);
					break;

				case 3:
					/* FW FORWARD */
					kattach_fw.filter.forward.eindex = (u8) atoi(results[i]);
					break;
	
				case 4:
					/* NAT POSTROUTING */
					kattach_fw.nat.postrouting.eindex = (u8) atoi(results[i]);
					break;

				case 5:
					/* NAT PREROUTING */
					kattach_fw.nat.prerouting.eindex = (u8) atoi(results[i]);
					break;

				case 6:
					/* NAT OUTPUT */
					kattach_fw.nat.output.eindex = (u8) atoi(results[i]);
					break;

				case 7:
					/* MANGLE INPUT */
					kattach_fw.mangle.input.eindex = (u8) atoi(results[i]);
					break;

				case 8:
					/* MANGLE OUTPUT */
					kattach_fw.mangle.output.eindex = (u8) atoi(results[i]);
					break;

				case 9:
					/* MANGLE FORWARD */
					kattach_fw.mangle.input.eindex = (u8) atoi(results[i]);
					break;

				case 10:
					/* MANGLE PREROUTING */
					kattach_fw.mangle.input.eindex = (u8) atoi(results[i]);
					break;

				case 11:
					/* MANGLE POSTROUTING */
					kattach_fw.mangle.input.eindex = (u8) atoi(results[i]);
					break;

				default:
					/* unknown */
					printf("\n [!] WARNING: Unknown firewall chain index - head: %u\n",chainindex);
					break;
			}
			continue;
		} else if (!strncmp("fwindex",colname[i],strlen(colname[i]))) {
			/* FIXME: use this to cross-reference */
			continue;
		}
	}
        return 0;
}

static int
kattach_vm_cb_fw_zones(void *NotUsed, int cols, char **results, char **colname)
{
	int i = 0;
	u32 index = kattach_fw.zones.index;
	char qres[512];

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("zindex",colname[i],strlen(colname[i]))) {
			/* new entry */
			kattach_fw.zones.index++;
                        continue;
		} else if (!strncmp("name",colname[i],strlen(colname[i]))) {
			if (strlen(results[i])) {
				sprintf(kattach_fw.zones.zone[index].name,"%s",results[i]);
			} else {
				kattach_fw.zones.zone[index].name[0] = '\0';
			}
			continue;
		} else if (!strncmp("vlan",colname[i],strlen(colname[i]))) {
			kattach_fw.zones.zone[index].vlan = (u16) (atoi(results[i]));
			continue;
		}
	}
	return 0;
}

static int
kattach_vm_cb_fw_zlink(void *NotUsed, int cols, char **results, char **colname)
{
	int i = 0;
	u32 znode = 0;
	u8 rc = 0;
	char sqlq[1024];
	char qres[512];

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("zlindex",colname[i],strlen(colname[i]))) {
                        continue;
		} else if (!strncmp("zone",colname[i],strlen(colname[i]))) {
			kattach_dbtmpindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("node",colname[i],strlen(colname[i]))) {
			znode = (u32) (atol(results[i]));
			sprintf(sqlq,"SELECT * from znodes WHERE znindex='%lu';",znode);
			rc = kattach_vm_sql_fw_znodes(sqlq, K_DB_VMSESSION);
			kattach_dbtmpindex = 0;
			continue;
		}
	}
	return 0;
}


static int
kattach_vm_cb_fw_znodes(void *NotUsed, int cols, char **results, char **colname)
{
	int i = 0;
	u8 znode = kattach_fw.zones.zone[kattach_dbtmpindex].nindex;
	char qres[512];

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("znindex",colname[i],strlen(colname[i]))) {
			kattach_fw.zones.zone[kattach_dbtmpindex].nindex++;
                        continue;
		} else if (!strncmp("ip",colname[i],strlen(colname[i]))) {
			kattach_fw.zones.zone[kattach_dbtmpindex].node[znode].ip = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("mask",colname[i],strlen(colname[i]))) {
			kattach_fw.zones.zone[kattach_dbtmpindex].node[znode].mask = (u8) (atoi(results[i]));
			continue;
		}
	}
	return 0;
}


static int
kattach_vm_cb_fw_apps(void *NotUsed, int cols, char **results, char **colname)
{
	int i = 0;
	u32 index = kattach_fw.apps.index;
	char qres[512];

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("aindex",colname[i],strlen(colname[i]))) {
			/* new entry */
			kattach_fw.apps.index++;
                        continue;
		} else if (!strncmp("name",colname[i],strlen(colname[i]))) {
			if (strlen(results[i])) {
				sprintf(kattach_fw.apps.app[index].name,"%s",results[i]);
			} else {
				kattach_fw.apps.app[index].name[0] = '\0';
			}
			continue;
		} else if (!strncmp("statemask",colname[i],strlen(colname[i]))) {
			kattach_fw.apps.app[index].statemask = (u16) (atoi(results[i]));
			continue;
		}
	}
	return 0;
}

static int
kattach_vm_cb_fw_alink(void *NotUsed, int cols, char **results, char **colname)
{
	int i = 0;
	u32 aport = 0;
	u8 rc = 0;
	char sqlq[1024];
	char qres[512];

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("pindex",colname[i],strlen(colname[i]))) {
                        continue;
		} else if (!strncmp("app",colname[i],strlen(colname[i]))) {
			kattach_dbtmpindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("port",colname[i],strlen(colname[i]))) {
			aport = (u32) (atol(results[i]));
			sprintf(sqlq,"SELECT * from appports WHERE apppindex='%lu';",aport);
			rc = kattach_vm_sql_fw_appports(sqlq, K_DB_VMSESSION);
			kattach_dbtmpindex = 0;
			continue;
		}
	}
	return 0;
}


static int
kattach_vm_cb_fw_appports(void *NotUsed, int cols, char **results, char **colname)
{
	int i = 0;
	u8 aport = kattach_fw.apps.app[kattach_dbtmpindex].pindex;
	char qres[512];

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("apppindex",colname[i],strlen(colname[i]))) {
			kattach_fw.apps.app[kattach_dbtmpindex].pindex++;
                        continue;
		} else if (!strncmp("portA",colname[i],strlen(colname[i]))) {
			kattach_fw.apps.app[kattach_dbtmpindex].port[aport].port[0] = (u16) (atoi(results[i]));
			continue;
		} else if (!strncmp("portB",colname[i],strlen(colname[i]))) {
			kattach_fw.apps.app[kattach_dbtmpindex].port[aport].port[1] = (u16) (atoi(results[i]));
			continue;
		} else if (!strncmp("protmask",colname[i],strlen(colname[i]))) {
			kattach_fw.apps.app[kattach_dbtmpindex].port[aport].protmask = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("direction",colname[i],strlen(colname[i]))) {
			kattach_fw.apps.app[kattach_dbtmpindex].port[aport].direction = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tcpall",colname[i],strlen(colname[i]))) {
			kattach_fw.apps.app[kattach_dbtmpindex].port[aport].tcp_flags.all = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tcpnone",colname[i],strlen(colname[i]))) {
			kattach_fw.apps.app[kattach_dbtmpindex].port[aport].tcp_flags.none = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tcpsyn",colname[i],strlen(colname[i]))) {
			kattach_fw.apps.app[kattach_dbtmpindex].port[aport].tcp_flags.syn = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tcpack",colname[i],strlen(colname[i]))) {
			kattach_fw.apps.app[kattach_dbtmpindex].port[aport].tcp_flags.ack = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tcpfin",colname[i],strlen(colname[i]))) {
			kattach_fw.apps.app[kattach_dbtmpindex].port[aport].tcp_flags.fin = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tcpreset",colname[i],strlen(colname[i]))) {
			kattach_fw.apps.app[kattach_dbtmpindex].port[aport].tcp_flags.reset = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tcppush",colname[i],strlen(colname[i]))) {
			kattach_fw.apps.app[kattach_dbtmpindex].port[aport].tcp_flags.push = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tcpurgent",colname[i],strlen(colname[i]))) {
			kattach_fw.apps.app[kattach_dbtmpindex].port[aport].tcp_flags.urgent = (u8) (atoi(results[i]));
			continue;
		}
	}
	return 0;
}

static int
kattach_vm_cb_fw_filter_input(void *NotUsed, int cols, char **results, char **colname)
{
	u32 index = kattach_fw.filter.input.index;
	int i = 0;
	char qres[512];
	kattach_fw_chain_t *chain;

	chain = &kattach_fw.filter.input;

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("fwindex",colname[i],strlen(colname[i]))) {
			chain->index++;
                        continue;
		} else if (!strncmp("pindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].pindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("nindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].nindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("szindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].szindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("dzindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].dzindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("appindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].appindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("rlimitpkt",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitpkt = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("rlimitint",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitint = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("action",colname[i],strlen(colname[i]))) {
			chain->filter[index].action = (u16) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlA",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlB",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosA",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosB",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("enabled",colname[i],strlen(colname[i]))) {
			chain->filter[index].enabled = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ruletype",colname[i],strlen(colname[i]))) {
			chain->filter[index].type = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("rejectwith",colname[i],strlen(colname[i]))) {
			chain->filter[index].rejectwith = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("reverse",colname[i],strlen(colname[i]))) {
			chain->filter[index].reverse = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logging",colname[i],strlen(colname[i]))) {
			chain->filter[index].logging = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logprefix",colname[i],strlen(colname[i]))) {
			sprintf(chain->filter[index].logprefix,"%s",results[i]);
			continue;
		}
	}
	return 0;
}


static int
kattach_vm_cb_fw_filter_output(void *NotUsed, int cols, char **results, char **colname)
{
	u32 index = kattach_fw.filter.output.index;
	int i = 0;
	char qres[512];
	kattach_fw_chain_t *chain;

	chain = &kattach_fw.filter.output;

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("fwindex",colname[i],strlen(colname[i]))) {
			chain->index++;
                        continue;
		} else if (!strncmp("pindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].pindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("nindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].nindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("szindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].szindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("dzindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].dzindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("appindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].appindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("rlimitpkt",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitpkt = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("rlimitint",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitint = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("action",colname[i],strlen(colname[i]))) {
			chain->filter[index].action = (u16) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlA",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlB",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosA",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosB",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("enabled",colname[i],strlen(colname[i]))) {
			chain->filter[index].enabled = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ruletype",colname[i],strlen(colname[i]))) {
			chain->filter[index].type = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("rejectwith",colname[i],strlen(colname[i]))) {
			chain->filter[index].rejectwith = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("reverse",colname[i],strlen(colname[i]))) {
			chain->filter[index].reverse = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logging",colname[i],strlen(colname[i]))) {
			chain->filter[index].logging = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logprefix",colname[i],strlen(colname[i]))) {
			sprintf(chain->filter[index].logprefix,"%s",results[i]);
			continue;
		}
	}
	return 0;
}

static int
kattach_vm_cb_fw_filter_forward(void *NotUsed, int cols, char **results, char **colname)
{
	u32 index = kattach_fw.filter.forward.index;
	int i = 0;
	char qres[512];
	kattach_fw_chain_t *chain;

	chain = &kattach_fw.filter.forward;

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("fwindex",colname[i],strlen(colname[i]))) {
			chain->index++;
                        continue;
		} else if (!strncmp("pindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].pindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("nindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].nindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("szindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].szindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("dzindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].dzindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("appindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].appindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("rlimitpkt",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitpkt = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("rlimitint",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitint = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("action",colname[i],strlen(colname[i]))) {
			chain->filter[index].action = (u16) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlA",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlB",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosA",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosB",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("enabled",colname[i],strlen(colname[i]))) {
			chain->filter[index].enabled = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ruletype",colname[i],strlen(colname[i]))) {
			chain->filter[index].type = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("rejectwith",colname[i],strlen(colname[i]))) {
			chain->filter[index].rejectwith = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("reverse",colname[i],strlen(colname[i]))) {
			chain->filter[index].reverse = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logging",colname[i],strlen(colname[i]))) {
			chain->filter[index].logging = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logprefix",colname[i],strlen(colname[i]))) {
			sprintf(chain->filter[index].logprefix,"%s",results[i]);
			continue;
		}
	}
	return 0;
}

static int
kattach_vm_cb_fw_nat_prerouting(void *NotUsed, int cols, char **results, char **colname)
{
	u32 index = kattach_fw.nat.prerouting.index;
	int i = 0;
	char qres[512];
	kattach_fw_n_chain_t *chain;

	chain = &kattach_fw.nat.prerouting;

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("fwindex",colname[i],strlen(colname[i]))) {
			chain->index++;
                        continue;
		} else if (!strncmp("pindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].pindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("nindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].nindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("szindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].szindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("dzindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].dzindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("nzindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].nzindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("appindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].appindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("nappindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].nappindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("rlimitpkt",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitpkt = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("rlimitint",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitint = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("action",colname[i],strlen(colname[i]))) {
			chain->filter[index].action = (u16) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlA",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlB",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosA",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosB",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("enabled",colname[i],strlen(colname[i]))) {
			chain->filter[index].enabled = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ruletype",colname[i],strlen(colname[i]))) {
			chain->filter[index].type = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("rejectwith",colname[i],strlen(colname[i]))) {
			chain->filter[index].rejectwith = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("reverse",colname[i],strlen(colname[i]))) {
			chain->filter[index].reverse = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logging",colname[i],strlen(colname[i]))) {
			chain->filter[index].logging = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logprefix",colname[i],strlen(colname[i]))) {
			sprintf(chain->filter[index].logprefix,"%s",results[i]);
			continue;
		}
	}
	return 0;
}


static int
kattach_vm_cb_fw_nat_postrouting(void *NotUsed, int cols, char **results, char **colname)
{
	u32 index = kattach_fw.nat.postrouting.index;
	int i = 0;
	char qres[512];
	kattach_fw_n_chain_t *chain;

	chain = &kattach_fw.nat.postrouting;

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("fwindex",colname[i],strlen(colname[i]))) {
			chain->index++;
                        continue;
		} else if (!strncmp("pindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].pindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("nindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].nindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("szindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].szindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("dzindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].dzindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("nzindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].nzindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("appindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].appindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("nappindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].nappindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("rlimitpkt",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitpkt = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("rlimitint",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitint = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("action",colname[i],strlen(colname[i]))) {
			chain->filter[index].action = (u16) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlA",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlB",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosA",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosB",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("enabled",colname[i],strlen(colname[i]))) {
			chain->filter[index].enabled = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ruletype",colname[i],strlen(colname[i]))) {
			chain->filter[index].type = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("rejectwith",colname[i],strlen(colname[i]))) {
			chain->filter[index].rejectwith = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("reverse",colname[i],strlen(colname[i]))) {
			chain->filter[index].reverse = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logging",colname[i],strlen(colname[i]))) {
			chain->filter[index].logging = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logprefix",colname[i],strlen(colname[i]))) {
			sprintf(chain->filter[index].logprefix,"%s",results[i]);
			continue;
		}
	}
	return 0;
}

static int
kattach_vm_cb_fw_nat_output(void *NotUsed, int cols, char **results, char **colname)
{
	u32 index = kattach_fw.nat.output.index;
	int i = 0;
	char qres[512];
	kattach_fw_n_chain_t *chain;

	chain = &kattach_fw.nat.output;

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("fwindex",colname[i],strlen(colname[i]))) {
			chain->index++;
                        continue;
		} else if (!strncmp("pindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].pindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("nindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].nindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("szindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].szindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("dzindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].dzindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("nzindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].nzindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("appindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].appindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("nappindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].nappindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("rlimitpkt",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitpkt = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("rlimitint",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitint = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("action",colname[i],strlen(colname[i]))) {
			chain->filter[index].action = (u16) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlA",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlB",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosA",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosB",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("enabled",colname[i],strlen(colname[i]))) {
			chain->filter[index].enabled = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ruletype",colname[i],strlen(colname[i]))) {
			chain->filter[index].type = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("rejectwith",colname[i],strlen(colname[i]))) {
			chain->filter[index].rejectwith = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("reverse",colname[i],strlen(colname[i]))) {
			chain->filter[index].reverse = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logging",colname[i],strlen(colname[i]))) {
			chain->filter[index].logging = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logprefix",colname[i],strlen(colname[i]))) {
			sprintf(chain->filter[index].logprefix,"%s",results[i]);
			continue;
		}
	}
	return 0;
}


static int
kattach_vm_cb_fw_mangle_input(void *NotUsed, int cols, char **results, char **colname)
{
	u32 index = kattach_fw.mangle.input.index;
	int i = 0;
	char qres[512];
	kattach_fw_m_chain_t *chain;

	chain = &kattach_fw.mangle.input;

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("fwindex",colname[i],strlen(colname[i]))) {
			chain->index++;
                        continue;
		} else if (!strncmp("pindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].pindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("nindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].nindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("szindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].szindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("dzindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].dzindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("appindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].appindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("rlimitpkt",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitpkt = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("rlimitint",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitint = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("pktmark",colname[i],strlen(colname[i]))) {
			chain->filter[index].mark = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("action",colname[i],strlen(colname[i]))) {
			chain->filter[index].action = (u16) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlA",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlB",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosA",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosB",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("enabled",colname[i],strlen(colname[i]))) {
			chain->filter[index].enabled = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ruletype",colname[i],strlen(colname[i]))) {
			chain->filter[index].type = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("rejectwith",colname[i],strlen(colname[i]))) {
			chain->filter[index].rejectwith = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("reverse",colname[i],strlen(colname[i]))) {
			chain->filter[index].reverse = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logging",colname[i],strlen(colname[i]))) {
			chain->filter[index].logging = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logprefix",colname[i],strlen(colname[i]))) {
			sprintf(chain->filter[index].logprefix,"%s",results[i]);
			continue;
		}
	}
	return 0;
}

static int
kattach_vm_cb_fw_mangle_output(void *NotUsed, int cols, char **results, char **colname)
{
	u32 index = kattach_fw.mangle.output.index;
	int i = 0;
	char qres[512];
	kattach_fw_m_chain_t *chain;

	chain = &kattach_fw.mangle.output;

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("fwindex",colname[i],strlen(colname[i]))) {
			chain->index++;
                        continue;
		} else if (!strncmp("pindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].pindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("nindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].nindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("szindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].szindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("dzindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].dzindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("appindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].appindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("rlimitpkt",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitpkt = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("rlimitint",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitint = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("pktmark",colname[i],strlen(colname[i]))) {
			chain->filter[index].mark = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("action",colname[i],strlen(colname[i]))) {
			chain->filter[index].action = (u16) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlA",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlB",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosA",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosB",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("enabled",colname[i],strlen(colname[i]))) {
			chain->filter[index].enabled = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ruletype",colname[i],strlen(colname[i]))) {
			chain->filter[index].type = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("rejectwith",colname[i],strlen(colname[i]))) {
			chain->filter[index].rejectwith = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("reverse",colname[i],strlen(colname[i]))) {
			chain->filter[index].reverse = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logging",colname[i],strlen(colname[i]))) {
			chain->filter[index].logging = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logprefix",colname[i],strlen(colname[i]))) {
			sprintf(chain->filter[index].logprefix,"%s",results[i]);
			continue;
		}
	}
	return 0;
}

static int
kattach_vm_cb_fw_mangle_forward(void *NotUsed, int cols, char **results, char **colname)
{
	u32 index = kattach_fw.mangle.forward.index;
	int i = 0;
	char qres[512];
	kattach_fw_m_chain_t *chain;

	chain = &kattach_fw.mangle.forward;

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("fwindex",colname[i],strlen(colname[i]))) {
			chain->index++;
                        continue;
		} else if (!strncmp("pindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].pindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("nindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].nindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("szindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].szindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("dzindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].dzindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("appindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].appindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("rlimitpkt",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitpkt = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("rlimitint",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitint = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("pktmark",colname[i],strlen(colname[i]))) {
			chain->filter[index].mark = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("action",colname[i],strlen(colname[i]))) {
			chain->filter[index].action = (u16) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlA",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlB",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosA",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosB",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("enabled",colname[i],strlen(colname[i]))) {
			chain->filter[index].enabled = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ruletype",colname[i],strlen(colname[i]))) {
			chain->filter[index].type = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("rejectwith",colname[i],strlen(colname[i]))) {
			chain->filter[index].rejectwith = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("reverse",colname[i],strlen(colname[i]))) {
			chain->filter[index].reverse = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logging",colname[i],strlen(colname[i]))) {
			chain->filter[index].logging = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logprefix",colname[i],strlen(colname[i]))) {
			sprintf(chain->filter[index].logprefix,"%s",results[i]);
			continue;
		}
	}
	return 0;
}

static int
kattach_vm_cb_fw_mangle_prerouting(void *NotUsed, int cols, char **results, char **colname)
{
	u32 index = kattach_fw.mangle.prerouting.index;
	int i = 0;
	char qres[512];
	kattach_fw_m_chain_t *chain;

	chain = &kattach_fw.mangle.prerouting;

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("fwindex",colname[i],strlen(colname[i]))) {
			chain->index++;
                        continue;
		} else if (!strncmp("pindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].pindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("nindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].nindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("szindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].szindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("dzindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].dzindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("appindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].appindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("rlimitpkt",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitpkt = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("rlimitint",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitint = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("pktmark",colname[i],strlen(colname[i]))) {
			chain->filter[index].mark = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("action",colname[i],strlen(colname[i]))) {
			chain->filter[index].action = (u16) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlA",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlB",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosA",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosB",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("enabled",colname[i],strlen(colname[i]))) {
			chain->filter[index].enabled = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ruletype",colname[i],strlen(colname[i]))) {
			chain->filter[index].type = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("rejectwith",colname[i],strlen(colname[i]))) {
			chain->filter[index].rejectwith = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("reverse",colname[i],strlen(colname[i]))) {
			chain->filter[index].reverse = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logging",colname[i],strlen(colname[i]))) {
			chain->filter[index].logging = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logprefix",colname[i],strlen(colname[i]))) {
			sprintf(chain->filter[index].logprefix,"%s",results[i]);
			continue;
		}
	}
	return 0;
}

static int
kattach_vm_cb_fw_mangle_postrouting(void *NotUsed, int cols, char **results, char **colname)
{
	u32 index = kattach_fw.mangle.postrouting.index;
	int i = 0;
	char qres[512];
	kattach_fw_m_chain_t *chain;

	chain = &kattach_fw.mangle.postrouting;

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
		if (!strncmp("fwindex",colname[i],strlen(colname[i]))) {
			chain->index++;
                        continue;
		} else if (!strncmp("pindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].pindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("nindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].nindex = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("szindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].szindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("dzindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].dzindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("appindex",colname[i],strlen(colname[i]))) {
			chain->filter[index].appindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("rlimitpkt",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitpkt = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("rlimitint",colname[i],strlen(colname[i]))) {
			chain->filter[index].rlimitint = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("pktmark",colname[i],strlen(colname[i]))) {
			chain->filter[index].mark = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("action",colname[i],strlen(colname[i]))) {
			chain->filter[index].action = (u16) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlA",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ttlB",colname[i],strlen(colname[i]))) {
			chain->filter[index].ttl[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosA",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[0] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("tosB",colname[i],strlen(colname[i]))) {
			chain->filter[index].tos[1] = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("enabled",colname[i],strlen(colname[i]))) {
			chain->filter[index].enabled = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("ruletype",colname[i],strlen(colname[i]))) {
			chain->filter[index].type = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("rejectwith",colname[i],strlen(colname[i]))) {
			chain->filter[index].rejectwith = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("reverse",colname[i],strlen(colname[i]))) {
			chain->filter[index].reverse = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logging",colname[i],strlen(colname[i]))) {
			chain->filter[index].logging = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("logprefix",colname[i],strlen(colname[i]))) {
			sprintf(chain->filter[index].logprefix,"%s",results[i]);
			continue;
		}
	}
	return 0;
}

static int
kattach_vm_cb_vns_vsn(void *NotUsed, int cols, char **results, char **colname)
{
	int i = 0;
	u32 index = kattach_vns.index;
	char qres[512];

	for (i = 0; i < cols; i++) {
		sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
		kattach_sql_logdb(qres);
		if (!strncmp("vsindex",colname[i],strlen(colname[i]))) {
			/* new entry */
			kattach_vns.index++;
			continue;
		} else if (!strncmp("vsip",colname[i],strlen(colname[i]))) {
			kattach_vns.vns[index].vsip = (u32) (atol(results[i]));
			continue;
		} else if (!strncmp("vsmsk",colname[i],strlen(colname[i]))) {
			kattach_vns.vns[index].vsmsk = (u16) (atoi(results[i]));
			continue;
		} else if (!strncmp("enabled",colname[i],strlen(colname[i]))) {
			kattach_vns.vns[index].enabled = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("mstate",colname[i],strlen(colname[i]))) {
			kattach_vns.vns[index].mstate = (u8) (atoi(results[i]));
			continue;
		} else if (!strncmp("netifidx",colname[i],strlen(colname[i]))) {
			kattach_vns.vns[index].netifidx = (u32) (atol(results[i]));
			continue;
		}
	}

	return 0;
}

static int
kattach_vm_cb_vns_vslink(void *NotUsed, int cols, char **results, char **colname)
{
	int i = 0;
	u32 index = 0;
	u8 rc = 0;
	char qres[512];
	char sqlq[1024];


	for (i = 0; i < cols; i++) {
		sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
		kattach_sql_logdb(qres);
		if (!strncmp("vslindex",colname[i],strlen(colname[i]))) {
			/* new entry */
			continue;
		} else if (!strncmp("vsnindex",colname[i],strlen(colname[i]))) {
			kattach_dbtmpindex = (u32) ((atol(results[i])) - 1);
			continue;
		} else if (!strncmp("vspindex",colname[i],strlen(colname[i]))) {
			index = (u32) (atol(results[i]));
			sprintf(sqlq,"SELECT * from vsp WHERE vspindex = '%lu';", index);
			rc = kattach_vm_sql_vns_vsp(sqlq, K_DB_VMSESSION);
			kattach_dbtmpindex = 0;
			continue;
		}
	}

	return 0;
}

static int
kattach_vm_cb_vns_vsp(void *NotUsed, int cols, char **results, char **colname)
{
        int i = 0;
        u32 index = kattach_vns.vns[kattach_dbtmpindex].vspindex;
        char qres[512];

        for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
                kattach_sql_logdb(qres);
                if (!strncmp("vspindex",colname[i],strlen(colname[i]))) {
                        /* new entry */
                        kattach_vns.vns[kattach_dbtmpindex].vspindex++;
                        continue;
                } else if (!strncmp("ratein",colname[i],strlen(colname[i]))) {
                        kattach_vns.vns[kattach_dbtmpindex].vsp[index].rate_in = (u32) (atol(results[i]));
                        continue;
                } else if (!strncmp("rateout",colname[i],strlen(colname[i]))) {
                        kattach_vns.vns[kattach_dbtmpindex].vsp[index].rate_out = (u32) (atol(results[i]));
                        continue;
                } else if (!strncmp("vsport",colname[i],strlen(colname[i]))) {
                        kattach_vns.vns[kattach_dbtmpindex].vsp[index].vsport = (u16) (atoi(results[i]));
                        continue;
                } else if (!strncmp("vmsport",colname[i],strlen(colname[i]))) {
                        kattach_vns.vns[kattach_dbtmpindex].vsp[index].vmsport = (u16) (atoi(results[i]));
                        continue;
                } else if (!strncmp("vmport",colname[i],strlen(colname[i]))) {
                        kattach_vns.vns[kattach_dbtmpindex].vsp[index].vmport = ((u32) (atol(results[i])) - 1);
                        continue;
                } else if (!strncmp("timein",colname[i],strlen(colname[i]))) {
                        kattach_vns.vns[kattach_dbtmpindex].vsp[index].time_in = (u8) (atoi(results[i]));
                        continue;
                } else if (!strncmp("timeout",colname[i],strlen(colname[i]))) {
                        kattach_vns.vns[kattach_dbtmpindex].vsp[index].time_in = (u8) (atoi(results[i]));
                        continue;
                } else if (!strncmp("sproto",colname[i],strlen(colname[i]))) {
                        kattach_vns.vns[kattach_dbtmpindex].vsp[index].sproto = (u8) (atoi(results[i]));
                        continue;
                } else if (!strncmp("enabled",colname[i],strlen(colname[i]))) {
                        kattach_vns.vns[kattach_dbtmpindex].vsp[index].enabled = (u8) (atoi(results[i]));
                        continue;
                }
        }

	return 0;
}
