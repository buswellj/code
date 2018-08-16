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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "sqlite3.h"
#include "appmgr.h"
#include "appmod_shared.h"

static int appmgr_sql_cb_empty(void *NotUsed, int cols, char **results, char **colname);
static int appmgr_sql_cb_read(void *NotUsed, int cols, char **results, char **colname);

void
appmgr_sql_init(void)
{
	int rc = 0, ret = 0;

	rc = sqlite3_open(APPMGR_DB, &db_appm);
	ret = chmod(APPMGR_DB, APPMGR_PERM_SECURE);

//	rc = sqlite3_open("./appm.db", &db_appm);
//	ret = chmod("./appm.db", APPMGR_PERM_SECURE);

	if (rc) {
		printf(" [X] FATAL: Unable to open database %s (%d)\n", APPMGR_DB, rc);
	}

	return;

}

void
appmgr_sql_read(char *cfggrp)
{
	char *dbErrMsg = 0;
	char sqlq[1024];
	int rc = 0;

	memset(sqlq,0,strlen(sqlq));
	sprintf(sqlq,"SELECT * from %s;", cfggrp);
	rc = sqlite3_exec(db_appm, sqlq, appmgr_sql_cb_read, 0, &dbErrMsg);	

	if (rc != SQLITE_OK) {
                printf("\n [!] Warning: SELECT failure %d: %s\n", rc, dbErrMsg);
        }
        sqlite3_free(dbErrMsg);
	return;
}

void
appmgr_sql_write(char *cfggrp)
{
	char sqlq[1024];
	char *dbErrMsg = 0;
	int rc = 0;

	memset(sqlq,0,strlen(sqlq));
	sprintf(sqlq,"DROP TABLE %s;",cfggrp);
	rc = sqlite3_exec(db_appm, sqlq, appmgr_sql_cb_empty, 0, &dbErrMsg);

	if (rc != SQLITE_OK) {
		printf("\n [!] Warning: DROP failure %d: %s\n", rc, dbErrMsg);
	}
	sqlite3_free(dbErrMsg);

	memset(sqlq,0,strlen(sqlq));
	sprintf(sqlq,"CREATE table %s(version INTEGER, cfggrp TEXT, cfgoption INTEGER);",cfggrp);
	rc = sqlite3_exec(db_appm, sqlq, appmgr_sql_cb_empty, 0, &dbErrMsg);

	if (rc != SQLITE_OK) {
		printf("\n [!] Warning: CREATE failure %d: %s\n", rc, dbErrMsg);
	}
	sqlite3_free(dbErrMsg);

	memset(sqlq,0,strlen(sqlq));
	sprintf(sqlq,"INSERT into %s VALUES (1,'%s',%lu);",cfggrp,cfggrp,appmod_test.config_option);
	rc = sqlite3_exec(db_appm, sqlq, appmgr_sql_cb_empty, 0, &dbErrMsg);

	if (rc != SQLITE_OK) {
		printf("\n [!] Warning: INSERT failure %d: %s\n", rc, dbErrMsg);
	}
	sqlite3_free(dbErrMsg);

	return;

}

static int
appmgr_sql_cb_empty(void *NotUsed, int cols, char **results, char **colname)
{
	return 0;
}

static int
appmgr_sql_cb_read(void *NotUsed, int cols, char **results, char **colname)
{
	int i = 0;

	for (i = 0; i < cols; i++) {
		if (!strncmp("version",colname[i],strlen(colname[i]))) {
			continue;
		} else if (!strncmp("cfggrp",colname[i],strlen(colname[i]))) {
			if (strlen(results[i])) {
				sprintf(appmod_test_shm->cfggrp,"%s",results[i]);
				sprintf(appmod_test.cfggrp,"%s",results[i]);
			} else {
				appmod_test_shm->cfggrp[0] = '\0';
				appmod_test.cfggrp[0] = '\0';
			}
		} else if (!strncmp("cfgoption",colname[i],strlen(colname[i]))) {
			appmod_test_shm->config_option = (u32) (atol(results[i]));
			appmod_test.config_option = (u32) (atol(results[i]));
			continue;
		}	
	}
	return 0;
}
