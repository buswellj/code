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
 * Source File :                kattach-sql.c
 * Purpose     :                sqlite3 database functions
 * Callers     :                kattach-sys.c
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/swap.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sqlite3.h"
#include "kattach_types.h"
#include "kattach.h"
#include "kaosdb.h"

/* prototypes */
static int kattach_sql_cb_chkdb_kaos(void *NotUsed, int cols, char **results, char **colname);
static int kattach_sql_cb_chkdb_appq(void *NotUsed, int cols, char **results, char **colname);
static int kattach_sql_cb_chkdb_vm(void *NotUsed, int cols, char **results, char **colname);
static int kattach_sql_cb_testdb(void *NotUsed, int cols, char **results, char **colname);
static int kattach_sql_cb_kaosconfig(void *NotUsed, int cols, char **results, char **colname);
static int kattach_sql_cb_kaosnetdev(void *NotUsed, int cols, char **results, char **colname);


void
kattach_sql_init(void)
{
	u8 rc = 0;

	kattach_dbactive = 0;

	/* initialize and read the sqlite database */
	/* we assume that /appq is mounted and setup properly */

	rc = kattach_sql_dbopen(KATTACH_KAOS_DB, K_DB_KAOS);
	if (rc == RC_OK) kattach_dbactive = 1;

	rc = kattach_sql_dbopen(KATTACH_APPQUEUE_DB, K_DB_APPQ);
	if (rc == RC_OK) kattach_dbactive += 10;

	rc = kattach_sql_dbopen(KATTACH_VMSESSION_DB, K_DB_VMSESSION);
	if (rc == RC_OK) kattach_dbactive += 100;

	if (kattach_dbactive != 111) {
		printf(" [!] FATAL -- Database access failed (sql_init)\n");
		return;
	}

	rc = kattach_sql_chkdb(CMSQL_KAOS_SELECT_CMINFO, K_DB_KAOS);
	if (rc == RC_FAIL) {
		kattach_sql_create_kaosdb();
	}

	rc = kattach_sql_chkdb(CMSQL_APPQ_SELECT_CMINFO, K_DB_APPQ);
	if (rc == RC_FAIL) {
		kattach_sql_create_appqdb();
	}

	rc = kattach_sql_chkdb(CMSQL_VMSESS_SELECT_CMINFO, K_DB_VMSESSION);
	if (rc == RC_FAIL) {
		kattach_sql_create_vmsessdb();
	}

	kattach_sql_update_kaosdb();

	return;

}

u8
kattach_sql_dbopen(char *dbpath, u8 dbPtr)
{
	int rc = 0, ret = 0;

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_open(dbpath, &db_kaos);
			ret = chmod(dbpath, KATTACH_PERM_SECURE);
			break;

		case K_DB_APPQ:
			rc = sqlite3_open(dbpath, &db_appq);
			ret = chmod(dbpath, KATTACH_PERM_SECURE);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_open(dbpath, &db_vmsession);
			ret = chmod(dbpath, KATTACH_PERM_SECURE);
			break;

		default:
			printf(" [!] Unable to open database %s (%d)\n", dbpath, rc);
			return (RC_FAIL);
			break;
	}
			

	if (rc) {
		printf(" [!] Unable to open database %s (%d)\n", dbpath, rc);
		return (RC_FAIL);
	} else {
		return (RC_OK);
	}
}

void
kattach_sql_dbclose(void)
{
	sqlite3_close(db_kaos);
	sqlite3_close(db_appq);
	sqlite3_close(db_vmsession);

	return;
}

void
kattach_sql_logdb(char *sqlq)
{
	FILE *stream;
	char *logfile = "/appq/db/sql.log";
	int ret = 0;

	stream = fopen(logfile,"a");
	if (stream == (FILE *)0) {
		printf("\n [#] ERROR: Unable to write SQL log\n      %s",sqlq);
	} else {
		fprintf(stream,"%s\n",sqlq);
		fclose(stream);
		ret = chmod(logfile,KATTACH_PERM_SECURE_GRD);
		ret = chown(logfile,KATTACH_UID_ROOT,KATTACH_GID_APPQ);
	}
	return;
}

u8
kattach_sql_testdb(char *sqlq, u8 dbPtr)
{
	char *dbErrMsg = 0;
	int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_sql_cb_testdb, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_sql_cb_testdb, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_sql_cb_testdb, 0, &dbErrMsg);
			break;

		default:
			printf(" [!] SQL Unknown Database - %u\n",dbPtr);
			break;
	}

	if (rc != SQLITE_OK) {
		printf(" [!] SQL error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
		return (RC_FAIL);
	} else {
		sqlite3_free(dbErrMsg);
		return (RC_OK);
	}
}

u8
kattach_sql_chkdb(char *sqlq, u8 dbPtr)
{
	char *dbErrMsg = 0;
	int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_sql_cb_chkdb_kaos, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_sql_cb_chkdb_appq, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_sql_cb_chkdb_vm, 0, &dbErrMsg);
			break;

		default:
			printf(" [!] SQL Unknown Database - %u\n",dbPtr);
			break;
	}

	if (rc != SQLITE_OK) {
		printf(" [!] SQL error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
		return (RC_FAIL);
	} else {
		sqlite3_free(dbErrMsg);
		return (RC_OK);
	}
}

u8
kattach_sql_kaosconfig(char *sqlq, u8 dbPtr)
{
	char *dbErrMsg = 0;
	int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_sql_cb_kaosconfig, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_sql_cb_kaosconfig, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_sql_cb_kaosconfig, 0, &dbErrMsg);
			break;

		default:
			printf(" [!] SQL Unknown Database - %u\n",dbPtr);
			break;
	}


	if (rc != SQLITE_OK) {
		printf(" [!] Unable to read KaOS Config DB error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
		return (RC_FAIL);
	} else {
		sqlite3_free(dbErrMsg);
		return (RC_OK);
	}
}

u8
kattach_sql_kaosnetdev(char *sqlq, u8 dbPtr)
{
	char *dbErrMsg = 0;
	int rc = 0;

	kattach_sql_logdb(sqlq);

	switch (dbPtr) {
		case K_DB_KAOS:
			rc = sqlite3_exec(db_kaos, sqlq, kattach_sql_cb_kaosnetdev, 0, &dbErrMsg);
			break;

		case K_DB_APPQ:
			rc = sqlite3_exec(db_appq, sqlq, kattach_sql_cb_kaosnetdev, 0, &dbErrMsg);
			break;

		case K_DB_VMSESSION:
			rc = sqlite3_exec(db_vmsession, sqlq, kattach_sql_cb_kaosnetdev, 0, &dbErrMsg);
			break;

		default:
			printf(" [!] SQL Unknown Database - %u\n",dbPtr);
			break;
	}

	if (rc != SQLITE_OK) {
		printf(" [!] Unable to read KaOS Net Devices DB error %d: %s\n", rc, dbErrMsg);
		sqlite3_free(dbErrMsg);
		return (RC_FAIL);
	} else {
		sqlite3_free(dbErrMsg);
		return (RC_OK);
	}
}

void
kattach_sql_create_kaosdb(void)
{
	u8 rc = kattach_sql_testdb(CMSQL_KAOS_CREATE_TABLE_CMINFO, K_DB_KAOS);

	/* todo: add error handling */

	rc = kattach_sql_testdb(CMSQL_KAOS_CREATE_TABLE_BOOTCFG, K_DB_KAOS);
	rc = kattach_sql_testdb(CMSQL_KAOS_CREATE_TABLE_DEVICES, K_DB_KAOS);
	rc = kattach_sql_testdb(CMSQL_KAOS_CREATE_TABLE_EXTRADEVICES, K_DB_KAOS);
	rc = kattach_sql_testdb(CMSQL_KAOS_CREATE_TABLE_CONFIG, K_DB_KAOS);
	rc = kattach_sql_testdb(CMSQL_KAOS_CREATE_TABLE_NETDEV, K_DB_KAOS);

	rc = kattach_sql_testdb(CMSQL_KAOS_INSERT_CMINFO, K_DB_KAOS);

	return;		/* completely oblivious to any problems */

}

void
kattach_sql_create_appqdb(void)
{
	u8 rc = kattach_sql_testdb(CMSQL_APPQ_CREATE_TABLE_CMINFO, K_DB_APPQ);
	
	rc = kattach_sql_testdb(CMSQL_APPQ_CREATE_TABLE_APPMODULE, K_DB_APPQ);
	rc = kattach_sql_testdb(CMSQL_APPQ_CREATE_TABLE_VMIMAGE, K_DB_APPQ);
	rc = kattach_sql_testdb(CMSQL_APPQ_CREATE_TABLE_VMAPPS, K_DB_APPQ);
	rc = kattach_sql_testdb(CMSQL_APPQ_CREATE_TABLE_CFGGRP, K_DB_APPQ);
	rc = kattach_sql_testdb(CMSQL_APPQ_CREATE_TRIGGER_APPMODULE, K_DB_APPQ);
	rc = kattach_sql_testdb(CMSQL_APPQ_INSERT_CMINFO, K_DB_APPQ);

	return;		/* completely oblivious to any problems */

}

void
kattach_sql_create_vmsessdb(void)
{
	u8 rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_CMINFO, K_DB_VMSESSION);

	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_VMSESS, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_VBRIDGE, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_VMPORT, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_ZNODES, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_ZONES, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_ZLINK, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_APPS, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_APPPORTS, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_ALINK, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWMAIN, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWFINPUT, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWFOUTPUT, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWFFORWARD, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWNPREROUTING, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWNPOSTROUTING, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWNOUTPUT, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWMINPUT, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWMOUTPUT, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWMFORWARD, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWMPREROUTING, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_FWMPOSTROUTING, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_VSP, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_VSN, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TABLE_VSLINK, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_CREATE_TRIGGER_VMSESS, K_DB_VMSESSION);
	rc = kattach_sql_testdb(CMSQL_VMSESS_INSERT_CMINFO, K_DB_VMSESSION);
	return;		/* completely oblivious to any problems */

}

static int
kattach_sql_cb_testdb(void *NotUsed, int cols, char **results, char **colname)
{
	int i;
	char qres[512];

	for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
		kattach_sql_logdb(qres);
        }
        return 0;

}

static int
kattach_sql_cb_chkdb_kaos(void *NotUsed, int cols, char **results, char **colname)
{
	int i = 0;
	u16 dbversion = 0, dbstatus = 0;
	u8 rc = 0;
	char qres[512];

	for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
		kattach_sql_logdb(qres);
		if (!strncmp("version",colname[i],strlen(colname[i]))) {
			dbversion = (u16) atoi(results[i]);
		} else if (!strncmp("status",colname[i],strlen(colname[i]))) {
			dbstatus = (u16) atoi(results[i]);
			if (dbstatus != 1) {
				printf(" [!] FATAL - Database status incorrect. Check sql.log!\n");
				return 0;
			} else if (dbversion > KAOS_DBVER) {
				printf(" [!] WARNING - Database (%u) is newer than code (%u). Upgrade ASAP!\n\n",dbversion,KAOS_DBVER);
				return 0;
			} else if (dbversion < KAOS_DBVER) {
				printf(" [*] Upgrade Database from %u to %u.\n",dbversion,KAOS_DBVER);
				if (dbversion == 1) {
					/* add upgrades here */
					/* upgrade the CMINFO to the latest version */
					rc = kattach_sql_testdb(CMSQL_V1_V2_UPDATE_TABLE_CMINFO, K_DB_KAOS);
					rc = kattach_sql_testdb(CMSQL_V1_V2_INSERT_TABLE_CMINFO, K_DB_KAOS);
				}
			}
		}	
        }
        return 0;

}

static int
kattach_sql_cb_chkdb_appq(void *NotUsed, int cols, char **results, char **colname)
{
	int i = 0;
	u16 dbversion = 0, dbstatus = 0;
	u8 rc = 0;
	char qres[512];

	for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
		kattach_sql_logdb(qres);
		if (!strncmp("version",colname[i],strlen(colname[i]))) {
			dbversion = (u16) atoi(results[i]);
		} else if (!strncmp("status",colname[i],strlen(colname[i]))) {
			dbstatus = (u16) atoi(results[i]);
			if (dbstatus != 1) {
				printf(" [!] FATAL - Database status incorrect. Check sql.log!\n");
				return 0;
			} else if (dbversion > KAOS_DBVER) {
				printf(" [!] WARNING - Database (%u) is newer than code (%u). Upgrade ASAP!\n\n",dbversion,KAOS_DBVER);
				return 0;
			} else if (dbversion < KAOS_DBVER) {
				printf(" [*] Upgrade Database from %u to %u.\n",dbversion,KAOS_DBVER);
				if (dbversion == 1) {
					/* add upgrades here */
					/* add appsize column to appmodule table */
					rc = kattach_sql_testdb(CMSQL_V1_V2_ALTER_TABLE_APPMODULE, K_DB_APPQ);

					/* upgrade the CMINFO to the latest version */
					rc = kattach_sql_testdb(CMSQL_V1_V2_UPDATE_TABLE_CMINFO, K_DB_APPQ);
					rc = kattach_sql_testdb(CMSQL_V1_V2_INSERT_TABLE_CMINFO, K_DB_APPQ);
				}
			}
		}	
        }
        return 0;

}

static int
kattach_sql_cb_chkdb_vm(void *NotUsed, int cols, char **results, char **colname)
{
	int i = 0;
	u16 dbversion = 0, dbstatus = 0;
	u8 rc = 0;
	char qres[512];

	for (i = 0; i < cols; i++) {
                sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
		kattach_sql_logdb(qres);
		if (!strncmp("version",colname[i],strlen(colname[i]))) {
			dbversion = (u16) atoi(results[i]);
		} else if (!strncmp("status",colname[i],strlen(colname[i]))) {
			dbstatus = (u16) atoi(results[i]);
			if (dbstatus != 1) {
				printf(" [!] FATAL - Database status incorrect. Check sql.log!\n");
				return 0;
			} else if (dbversion > KAOS_DBVER) {
				printf(" [!] WARNING - Database (%u) is newer than code (%u). Upgrade ASAP!\n\n",dbversion,KAOS_DBVER);
				return 0;
			} else if (dbversion < KAOS_DBVER) {
				printf(" [*] Upgrade Database from %u to %u.\n",dbversion,KAOS_DBVER);
				if (dbversion == 1) {
					/* add upgrades here */
					/* upgrade the CMINFO to the latest version */
					rc = kattach_sql_testdb(CMSQL_V1_V2_UPDATE_TABLE_CMINFO, K_DB_VMSESSION);
					rc = kattach_sql_testdb(CMSQL_V1_V2_INSERT_TABLE_CMINFO, K_DB_VMSESSION);
				}
			}
		}	
        }
        return 0;

}

static int
kattach_sql_cb_kaosconfig(void *NotUsed, int cols, char **results, char **colname)
{
	int i = 0, s = 0;
	char lcmd[255];
	char qres[512];

	for (i = 0; i < cols; i++) {
		sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
		kattach_sql_logdb(qres);
		if (!strncmp("cindex",colname[i],strlen(colname[i]))) {
			/* new entry */
			continue;
		} else if (!strncmp("hostname",colname[i],strlen(colname[i]))) {
			sprintf(kattach_cfg.hostname,"%s",results[i]);
			if (strlen(results[i]) >= 2) {
				sprintf(lcmd,"%s%s %s",KATTACH_BINPATH,KCMD_HOSTNAME,kattach_cfg.hostname);
				kattach_sysexec(lcmd);
			}
			continue;
		} else if (!strncmp("bootdev",colname[i],strlen(colname[i]))) {
			sprintf(kattach_install.diskboot,"%s",results[i]);
			continue;
		} else if (!strncmp("swapdev",colname[i],strlen(colname[i]))) {
			sprintf(kattach_install.diskswap,"%s",results[i]);
			if (strlen(results[i]) >= 2) {
				sprintf(lcmd,"/dev/%s",kattach_install.diskswap);
				s = swapon(lcmd,0);
				if (s != 0) {
					printf("\n [*] Creating swap on %s\n", kattach_install.diskswap);
					sprintf(lcmd,"%s%s",KATTACH_SBINPATH,KCMD_MKSWAP);
					kattach_sysexec(lcmd);
				} else {
					continue;
				}
				/* retry after mkswap */
				sprintf(lcmd,"/dev/%s",kattach_install.diskswap);
				s = swapon(lcmd,0);
				if (s != 0) {
					printf("\n [!] WARNING: Unable to active swap on /dev/%s.\n",kattach_install.diskswap);
				}
			}
			continue;
		} else if (!strncmp("appqdev",colname[i],strlen(colname[i]))) {
			sprintf(kattach_install.diskappq,"%s",results[i]);
			continue;
		} else if (!strncmp("datadev",colname[i],strlen(colname[i]))) {
			sprintf(kattach_install.diskdata,"%s",results[i]);
			continue;
		} else if (!strncmp("domain",colname[i],strlen(colname[i]))) {
			sprintf(kattach_cfg.domain,"%s",results[i]);
			continue;
		} else if (!strncmp("dnsA",colname[i],strlen(colname[i]))) {
			kattach_cfg.dns_ip[0] = ((u32) atol(results[i]));
			continue;
		} else if (!strncmp("dnsB",colname[i],strlen(colname[i]))) {
			kattach_cfg.dns_ip[1] = ((u32) atol(results[i]));
			continue;
		} else if (!strncmp("dnsC",colname[i],strlen(colname[i]))) {
			kattach_cfg.dns_ip[2] = ((u32) atol(results[i]));
			continue;
		} else if (!strncmp("dnsD",colname[i],strlen(colname[i]))) {
			kattach_cfg.dns_ip[3] = ((u32) atol(results[i]));
			continue;
		} else if (!strncmp("dnsE",colname[i],strlen(colname[i]))) {
			kattach_cfg.dns_ip[4] = ((u32) atol(results[i]));
			continue;
		} else if (!strncmp("dnsF",colname[i],strlen(colname[i]))) {
			kattach_cfg.dns_ip[5] = ((u32) atol(results[i]));
			continue;
		} else if (!strncmp("ntpA",colname[i],strlen(colname[i]))) {
			kattach_cfg.ntp_ip[0] = ((u32) atol(results[i]));
			continue;
		} else if (!strncmp("ntpB",colname[i],strlen(colname[i]))) {
			kattach_cfg.ntp_ip[1] = ((u32) atol(results[i]));
			continue;
		} else if (!strncmp("ntpC",colname[i],strlen(colname[i]))) {
			kattach_cfg.ntp_ip[2] = ((u32) atol(results[i]));
			continue;
		} else if (!strncmp("ntpint",colname[i],strlen(colname[i]))) {
			kattach_cfg.ntpint = ((u16) atoi(results[i]));
			continue;
		} else if (!strncmp("aquser",colname[i],strlen(colname[i]))) {
			sprintf(kattach_cfg.aquser,"%s",results[i]);
			continue;
		} else if (!strncmp("aqpass",colname[i],strlen(colname[i]))) {
			sprintf(kattach_cfg.aqpass,"%s",results[i]);
			continue;
		} else if (!strncmp("clipass",colname[i],strlen(colname[i]))) {
			sprintf(kattach_cfg.clipass,"%s",results[i]);
			continue;
		} else if (!strncmp("rootpass",colname[i],strlen(colname[i]))) {
			sprintf(kattach_cfg.rootpass,"%s",results[i]);
			kattach_cfg.rootpwck[0] = '\0';
			continue;
		} else if (!strncmp("root",colname[i],strlen(colname[i]))) {
			kattach_cfg.root = ((u8) atoi(results[i]));
			continue;
		}
        }
	if (cols > 0) {
		kattach_sys_write_resolv();		/* write out resolv.conf */
                if (kattach_install.diskboot[0] != '\0') {
                        sprintf(lcmd,"%s%s -p /dev/%s",KATTACH_SBINPATH,KCMD_FSCK,kattach_install.diskboot);
                        kattach_sysexec(lcmd);
                }
                if (kattach_install.diskdata[0] != '\0') {
                        sprintf(lcmd,"%s%s -p /dev/%s",KATTACH_SBINPATH,KCMD_FSCK,kattach_install.diskdata);
                        kattach_sysexec(lcmd);
                }
	}
        return 0;
}

static int
kattach_sql_cb_kaosnetdev(void *NotUsed, int cols, char **results, char **colname)
{
	u16 index = kattach_netdev.index;
	int i = 0;
	char qres[512];

	for (i = 0; i < cols; i++) {
		sprintf(qres,"QR: %s = %s (%d of %d)", colname[i], results[i] ? results[i] : "NULL",i,cols);
		kattach_sql_logdb(qres);
		if (!strncmp("ifindex",colname[i],strlen(colname[i]))) {
			/* new entry */
			kattach_netdev.index++;
			kattach_netdev.pif[index].spare = 0;
			kattach_netdev.pif[index].vns = 0;
			continue;
		} else if (!strncmp("devname",colname[i],strlen(colname[i]))) {
			sprintf(kattach_netdev.pif[index].devname,"%s",results[i]);
			continue;
		} else if (!strncmp("ip",colname[i],strlen(colname[i]))) {
			kattach_netdev.pif[index].ip = ((u32) atol(results[i]));
			continue;
		} else if (!strncmp("gw",colname[i],strlen(colname[i]))) {
			kattach_netdev.pif[index].gw = ((u32) atol(results[i]));
			continue;
		} else if (!strncmp("mask",colname[i],strlen(colname[i]))) {
			kattach_netdev.pif[index].mask = ((u16) atoi(results[i]));
			continue;
		} else if (!strncmp("pvid",colname[i],strlen(colname[i]))) {
			kattach_netdev.pif[index].pvid = ((u16) atoi(results[i]));
			continue;
		} else if (!strncmp("lacpidx",colname[i],strlen(colname[i]))) {
			kattach_netdev.pif[index].lacpidx = ((u16) atoi(results[i]));
			continue;
		} else if (!strncmp("mtu",colname[i],strlen(colname[i]))) {
			kattach_netdev.pif[index].mtu = ((u16) atoi(results[i]));
			continue;
		} else if (!strncmp("type",colname[i],strlen(colname[i]))) {
			kattach_netdev.pif[index].type = ((u8) atoi(results[i]));
			continue;
		} else if (!strncmp("psuedo",colname[i],strlen(colname[i]))) {
			kattach_netdev.pif[index].psuedo = ((u8) atoi(results[i]));
			continue;
		} else if (!strncmp("status",colname[i],strlen(colname[i]))) {
			kattach_netdev.pif[index].status = ((u8) atoi(results[i]));
			continue;
		} else if (!strncmp("macA",colname[i],strlen(colname[i]))) {
			kattach_netdev.pif[index].mac[0] = ((u8) atoi(results[i]));
			continue;
		} else if (!strncmp("macB",colname[i],strlen(colname[i]))) {
			kattach_netdev.pif[index].mac[1] = ((u8) atoi(results[i]));
			continue;
		} else if (!strncmp("macC",colname[i],strlen(colname[i]))) {
			kattach_netdev.pif[index].mac[2] = ((u8) atoi(results[i]));
			continue;
		} else if (!strncmp("macD",colname[i],strlen(colname[i]))) {
			kattach_netdev.pif[index].mac[3] = ((u8) atoi(results[i]));
			continue;
		} else if (!strncmp("macE",colname[i],strlen(colname[i]))) {
			kattach_netdev.pif[index].mac[4] = ((u8) atoi(results[i]));
			continue;
		} else if (!strncmp("macF",colname[i],strlen(colname[i]))) {
			kattach_netdev.pif[index].mac[5] = ((u8) atoi(results[i]));
			continue;
		}
        }
        return 0;
}

void
kattach_sql_update_kaosdb(void)
{
	int i = 0;
	u8 rc = 0;
	char sqlq[1024];

	/* we should check bootcfg here but for now lets just dump it into the database */

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"INSERT into bootcfg (ip,gw,dns,slash,mode,dhcp,macA,macB,macC,macD,macE,macF,netdev,storedev) values (%lu,%lu,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,'%s','%s');",
			kattach_cfg.ip, kattach_cfg.gw, kattach_cfg.dns, kattach_cfg.slash, kattach_cfg.mode, kattach_cfg.dhcp,
			kattach_cfg.mac[0], kattach_cfg.mac[1], kattach_cfg.mac[2], kattach_cfg.mac[3], kattach_cfg.mac[4], kattach_cfg.mac[5],
			kattach_cfg.netdev, kattach_cfg.storedev);

	rc = kattach_sql_testdb(sqlq, K_DB_KAOS);

	/* drop the devices table, and rebuild it */

	rc = kattach_sql_testdb(CMSQL_KAOS_DROP_DEVICES, K_DB_KAOS);
	rc = kattach_sql_testdb(CMSQL_KAOS_CREATE_TABLE_DEVICES, K_DB_KAOS);

	for (i = 0; i < kattach_devices.index; i++) {
		memset(sqlq,0,sizeof(sqlq));
		sprintf(sqlq,"INSERT into devices VALUES(%d,'%s',%u,%u,%u,%u);",i+1,kattach_devices.device[i].devname,
				kattach_devices.device[i].devtype, kattach_devices.device[i].major, kattach_devices.device[i].minor,
				kattach_devices.device[i].res);
		rc = kattach_sql_testdb(sqlq,K_DB_KAOS);
	}

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"SELECT * from config WHERE cindex='1';");
	rc = kattach_sql_kaosconfig(sqlq,K_DB_KAOS);
	rc = kattach_sql_kaosnetdev(CMSQL_KAOS_SELECT_NETDEV,K_DB_KAOS);

	return;

}

void
kattach_sql_clear_netdev(void)
{
	u16 index = 0;
	u8 rc = 0;
	char sqlq[1024];

	rc = kattach_sql_testdb(CMSQL_KAOS_DROP_NETDEV, K_DB_KAOS);
	rc = kattach_sql_testdb(CMSQL_KAOS_CREATE_TABLE_NETDEV, K_DB_KAOS);

	for (index = 0; index < kattach_netdev.index; index++) {
		memset(sqlq,0,sizeof(sqlq));
		sprintf(sqlq,"INSERT into netdev VALUES(%u,'%s',%lu,%lu,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u);",index+1,kattach_netdev.pif[index].devname,
									kattach_netdev.pif[index].ip, kattach_netdev.pif[index].gw, kattach_netdev.pif[index].mask,
									kattach_netdev.pif[index].pvid, kattach_netdev.pif[index].lacpidx, kattach_netdev.pif[index].mtu,
									kattach_netdev.pif[index].psuedo, kattach_netdev.pif[index].status, 
									kattach_netdev.pif[index].mac[0], kattach_netdev.pif[index].mac[1], kattach_netdev.pif[index].mac[2],
									kattach_netdev.pif[index].mac[3], kattach_netdev.pif[index].mac[4], kattach_netdev.pif[index].mac[5]);
		rc = kattach_sql_testdb(sqlq,K_DB_KAOS);
	}

	return;
}

void
kattach_sql_update_config(void)
{
	char sqlq[1024];
	u8 rc = 0;

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET hostname = '%s' WHERE cindex='1';",kattach_cfg.hostname);
	rc =  kattach_sql_testdb(sqlq, K_DB_KAOS);

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET bootdev = '%s' WHERE cindex='1';",kattach_install.diskboot);
	rc =  kattach_sql_testdb(sqlq, K_DB_KAOS);

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET swapdev = '%s' WHERE cindex='1';",kattach_install.diskswap);
	rc =  kattach_sql_testdb(sqlq, K_DB_KAOS);

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET appqdev = '%s' WHERE cindex='1';",kattach_install.diskappq);
	rc =  kattach_sql_testdb(sqlq, K_DB_KAOS);

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET datadev = '%s' WHERE cindex='1';",kattach_install.diskdata);
	rc =  kattach_sql_testdb(sqlq, K_DB_KAOS);

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET domain = '%s' WHERE cindex='1';",kattach_cfg.domain);
	rc =  kattach_sql_testdb(sqlq, K_DB_KAOS);

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET dnsA = '%lu' WHERE cindex='1';",kattach_cfg.dns_ip[0]);
	rc =  kattach_sql_testdb(sqlq, K_DB_KAOS);

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET dnsB = '%lu' WHERE cindex='1';",kattach_cfg.dns_ip[1]);
	rc =  kattach_sql_testdb(sqlq, K_DB_KAOS);

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET dnsC = '%lu' WHERE cindex='1';",kattach_cfg.dns_ip[2]);
	rc =  kattach_sql_testdb(sqlq, K_DB_KAOS);

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET dnsD = '%lu' WHERE cindex='1';",kattach_cfg.dns_ip[3]);
	rc =  kattach_sql_testdb(sqlq, K_DB_KAOS);

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET dnsE = '%lu' WHERE cindex='1';",kattach_cfg.dns_ip[4]);
	rc =  kattach_sql_testdb(sqlq, K_DB_KAOS);

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET dnsF = '%lu' WHERE cindex='1';",kattach_cfg.dns_ip[5]);
	rc =  kattach_sql_testdb(sqlq, K_DB_KAOS);

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET ntpint = '%u' WHERE cindex='1';",kattach_cfg.ntpint);
	rc = kattach_sql_testdb(sqlq, K_DB_KAOS);
	
	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET ntpA = '%lu' WHERE cindex='1';",kattach_cfg.ntp_ip[0]);
	rc = kattach_sql_testdb(sqlq, K_DB_KAOS);

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET ntpB = '%lu' WHERE cindex='1';",kattach_cfg.ntp_ip[1]);
	rc = kattach_sql_testdb(sqlq, K_DB_KAOS);

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET ntpC = '%lu' WHERE cindex='1';",kattach_cfg.ntp_ip[2]);
	rc = kattach_sql_testdb(sqlq, K_DB_KAOS);

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET aquser = '%s' WHERE cindex='1';",kattach_cfg.aquser);
	rc = kattach_sql_testdb(sqlq, K_DB_KAOS);

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET aqpass = '%s' WHERE cindex='1';",kattach_cfg.aqpass);
	rc = kattach_sql_testdb(sqlq, K_DB_KAOS);

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET clipass = '%s' WHERE cindex='1';",kattach_cfg.clipass);
	rc = kattach_sql_testdb(sqlq, K_DB_KAOS);

	memset(sqlq,0,sizeof(sqlq));

	/* root is a special case since the shared memory interface will nuke the root password from memory */
	if (kattach_cfg.root == 1) {
		if (strlen(kattach_cfg.rootpass) > 20) {
			sprintf(sqlq,"UPDATE config SET rootpass = '%s' WHERE cindex='1';",kattach_cfg.rootpass);
			rc = kattach_sql_testdb(sqlq, K_DB_KAOS);
		} else {
			kattach_cfg.rootpass[0] = '\0';
		}
	} else if (strlen(kattach_cfg.rootpass) < 20) {
		kattach_cfg.rootpass[0] = '\0';
		kattach_cfg.root = 0;
		sprintf(sqlq,"UPDATE config SET rootpass = '%s' WHERE cindex='1';",kattach_cfg.rootpass);
		rc = kattach_sql_testdb(sqlq, K_DB_KAOS);
	}

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"UPDATE config SET root = '%u' WHERE cindex='1';",kattach_cfg.root);
	rc = kattach_sql_testdb(sqlq, K_DB_KAOS);

	return;
}

void
kattach_sql_insert_config(void)
{
	char sqlq[1024];
	u8 rc = 0;

	memset(sqlq,0,sizeof(sqlq));
	sprintf(sqlq,"INSERT into config VALUES(%u,'%s',%u,'%s','%s','%s','%s','%s',%lu,%lu,%lu,%lu,%lu,%lu,%u,%lu,%lu,%lu,'%s','%s','%s','%s',%u);",1,kattach_cfg.hostname,0,
						kattach_install.diskboot, kattach_install.diskswap, kattach_install.diskappq,
						kattach_install.diskdata, kattach_cfg.domain, kattach_cfg.dns_ip[0],
						kattach_cfg.dns_ip[1], kattach_cfg.dns_ip[2], kattach_cfg.dns_ip[3],
						kattach_cfg.dns_ip[4], kattach_cfg.dns_ip[5], kattach_cfg.ntpint, kattach_cfg.ntp_ip[0], kattach_cfg.ntp_ip[1],
						kattach_cfg.ntp_ip[2], kattach_cfg.aquser, kattach_cfg.aqpass, kattach_cfg.clipass, kattach_cfg.rootpass, kattach_cfg.root);
	rc = kattach_sql_testdb(sqlq, K_DB_KAOS);
	return;
}


