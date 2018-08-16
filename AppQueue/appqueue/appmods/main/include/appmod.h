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

/* definitions */
/* AppQueue release information */
#define APPQUEUE_VERSION                "0.6.1.0"
#define APPQUEUE_ARCH                   "x86 / x64"
#define APPQUEUE_RELEASE                "dev"
#define APPQUEUE_COPYRIGHT              "Copyright (c) 2009 - 2010 Carbon Mountain LLC."
#define APPQUEUE_LICENSE                "This program may be freely redistributed under the terms of the GNU GPLv2"
#define APPQUEUE_LINK                   "http://www.carbonmountain.com"
#define APPQUEUE_CLI_LINES              54

/* result codes */
#define RC_FAIL                         0
#define RC_OK                           1
#define RC_MISSING                      2

/* cli cli permissions */
//#define APPMOD_CLI_AUTH_UID           500
#define APPMOD_CLI_AUTH_UID           0x64
#define APPMOD_CLI_AUTH_USER          0x00
#define APPMOD_CLI_AUTH_ADMIN         0xf0
#define APPMOD_CLI_AUTH_DEV           0xf7

/* cli menu defines */
#define APPMOD_MENU_MAX_CMDLEN			16
#define APPMOD_MENU_MAX_DESCLEN			64
#define APPMOD_MENU_MAX_ITEMS			16
#define APPMOD_MENU_BAR				"-----------------------------------------------------------------"
#define APPMOD_MENU_BARXL			"-------------------------------------------------------------------------------------------"

typedef unsigned u8;
typedef unsigned int u16;
typedef unsigned long u32;
typedef unsigned long long u64;

typedef struct {
        char menu_cmd[APPMOD_MENU_MAX_CMDLEN];                        	/* menu command */
        char menu_desc[APPMOD_MENU_MAX_DESCLEN];                      	/* menu description */
        u32 menu_func;                                                  /* menu function index */
        u8 menu_perms;                                                  /* menu item minimum permission level */
} appmod_cli_menu_entry_t;

typedef struct {
        appmod_cli_menu_entry_t climenu[APPMOD_MENU_MAX_ITEMS];
        char cli_menu_title[APPMOD_MENU_MAX_DESCLEN];
        u8 index;                                                       /* menu index */
} appmod_cli_menu_t;


/* declarations */
void appmod_cli(void);
void appmod_cli_init(void);
void appmod_cli_loop(void);
void appmod_cli_askq(char *askprompt, int do_echo, char *cmdline);
int appmod_cli_getch(int do_echo);

/* menus */
void appmod_cli_init_main(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appmod_cli_init_test(char *cli_menu_cmd, char *cli_menu_desc, u16 cli_menu_func);
void appmod_cli_mf_main(void);
void appmod_cli_mf_test(void);
void appmod_cli_mf_test_hello(void);
void appmod_cli_mf_exit(void);

/* cli function pointers */
#define APPMOD_CLI_FP_MAIN			0
#define APPMOD_CLI_FP_TEST			1
#define APPMOD_CLI_FP_EXIT			2
#define APPMOD_CLI_FP_TEST_HELLO		3

#define APPMOD_CLI_FP_END			4
#define APPMOD_MENU_MAX_CLI			APPMOD_CLI_FP_END

/* globals */
char appmod_cmd[255];
char appmod_prompt[64];
u8 appmod_cli_current_menu;
u8 appmod_cli_user_auth;
u8 appmod_exit;
u8 appmod_po;

appmod_cli_menu_t appmod_cli_menu_main;
appmod_cli_menu_t appmod_cli_menu_test;

void (*appmod_xmenu[APPMOD_MENU_MAX_CLI]) (void);
void (*appmod_cli_caller) (void);
