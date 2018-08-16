/*
 * AppQueue Module Framework
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
 */

/* result codes */
#define RC_FAIL                         0
#define RC_OK                           1
#define RC_MISSING                      2


typedef unsigned u8;
typedef unsigned int u16;
typedef unsigned long u32;
typedef unsigned long long u64;

/* app specific stuff */
/* replace this definitation with /appq/am/<vendor>/<appmodule>/appm.db */

#define APPMGR_DB			"/appq/am/cm/appmod/appm.db"
#define APPMGR_PERM_SECURE		(S_IRWXU)
#define APPMGR_PERM			(S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH)
#define APPMOD_PERM_SHM			(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define APPMOD_UID_ROOT			0
#define APPMOD_GID_APPQ			77


/* SQL stuff */

/* this is just a guide, we create tables in the code on a per group basis */
#define APPMGR_SQL_CREATE_TABLE "CREATE TABLE test(testindex INTEGER, configA INTEGER);"

/* globals */

sqlite3 *db_appm;

/* declarations */
void appmgr_loop(void);
void appmgr_shm_init(void);
void appmgr_sql_init(void);
void appmgr_sql_read(char *cfggrp);
void appmgr_shm_close(void);
int appmgr_shm_open(char *appmod_shm_path);
void appmgr_shm_sync(void);
void appmgr_cfg_write(void);
void appmgr_sql_read(char *cfggrp);
void appmgr_sql_write(char *cfggrp);
