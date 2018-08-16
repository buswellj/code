/*
 * UxMI Header File
 *
 * uxmi.h
 * 
 * All Rights Reserved.
 * Copyright (c) 2002-2005 Spliced Networks LLC
 *
 */

#define UXMI_VERSION		"1.0.0"
#define UXMI_CONFIG		"/etc/AppOS/uxmi.cfg"
#define UXMI_COPYRIGHT		"Copyright (c) 2002-2005 Spliced Networks LLC"

#define UXMI_DEBUG		1

#if defined(UXMI_DEBUG)
#define UXMI_DEBUG_NOEXEC	1
/* #define UXMI_DEBUG_OUTPUT	1 */
#endif /* defined(UXMI_DEBUG) */

/* define run levels */

#define UXMI_RL_HALT		0
#define UXMI_RL_MAINT		1
#define UXMI_RL_UNUSED_TWO	2
#define UXMI_RL_MULTIUSER	3
#define UXMI_RL_UPGRADE		4
#define UXMI_RL_UNUSED_FIVE	5
#define UXMI_RL_REBOOT		6

/* define colours */

#define UXMI_BLACK		0
#define UXMI_BLUE		1
#define UXMI_GREEN		2
#define UXMI_CYAN		3
#define UXMI_RED		4
#define UXMI_PURPLE		5
#define UXMI_BROWN		6
#define UXMI_LGRAY		7
#define UXMI_DGRAY		8
#define UXMI_LBLUE		9
#define UXMI_LGREEN		10
#define UXMI_LCYAN		11
#define UXMI_LRED		12
#define UXMI_LPURPLE		13
#define UXMI_YELLOW		14
#define UXMI_WHITE		15

/* define commands */

#define UXMI_CMD_DMESG		"/bin/dmesg"
#define UXMI_CMD_MOUNT		"/bin/mount"
#define UXMI_CMD_RM		"/bin/rm"
#define UXMI_CMD_TOUCH		"/bin/touch"
#define UXMI_CMD_CHMOD		"/bin/chmod"
#define UXMI_CMD_LOADKEYS	"/bin/loadkeys"
#define UXMI_CMD_SETFONT	"/usr/bin/setfont"
#define UXMI_CMD_HWCLOCK	"/usr/bin/hwclock"
#define UXMI_CMD_IP		"/sbin/ip"
#define UXMI_CMD_MODPROBE	"/sbin/modprobe"

/* define config file limits */

#define UXMI_CFG_MAXLEN		128		/* maximum cfg line length */
#define UXMI_CFG_ID_MAX		128		/* maximum length of ID */

#define UXMI_CFG_STYLE_KEY	0		/* keyword style */
#define UXMI_CFG_STYLE_PATH	1		/* path style */
#define UXMI_CFG_STYLE_OPTION	2		/* option style */

#define CMAX_ENTRIES		1024		/* max entries */

#define UXMI_CFG_TREE_SYSTEM		0x000001	
#define UXMI_CFG_TREE_INTERFACE		0x000002
#define UXMI_CFG_TREE_ROUTE		0x000004
#define UXMI_CFG_TREE_AUTH		0x000008
#define UXMI_CFG_TREE_IP		0x000010
#define UXMI_CFG_TREE_ACL		0x000020

#define UXMI_CFG_KEY_SYSTEM		"system"
#define UXMI_CFG_KEY_INTERFACE		"interface"
#define UXMI_CFG_KEY_ROUTE		"route"
#define UXMI_CFG_KEY_AUTH		"auth"
#define UXMI_CFG_KEY_IP			"ip"
#define UXMI_CFG_KEY_ACL		"acl"

/* structures */

typedef unsigned int u8;
typedef unsigned long u16;
typedef unsigned long long u32;

typedef struct {
	char id[128];				/* hardware identifier */
	char cchar;				/* comment character */
	u8 style;				/* UXMI_CFG_STYLE_XXX */
	u8 type;				/* type write, apply, instant */
	u8 ver_rel;				/* release version */
	u8 ver_maint;				/* maintenance version */
	u8 ver_patch;				/* patch version */
} uxmi_cfg_hdr_t;

typedef struct {
	char p_keyword[255];			/* primary keyword */
	char s_keyword[255];			/* sec keyword */
	char cmd[255];				/* command */
	char value[255];			/* value */
	char t_keyword[128];			/* target keyword */
	char t_file[128];			/* target config file */	
} uxmi_cfg_entry_t;

typedef struct {
	uxmi_cfg_hdr_t cfg_header;			/* config header */
	uxmi_cfg_entry_t cfg_system[CMAX_ENTRIES];	/* system tree */
	uxmi_cfg_entry_t cfg_interface[CMAX_ENTRIES];	/* interface tree */
	uxmi_cfg_entry_t cfg_route[CMAX_ENTRIES];	/* route tree */
	uxmi_cfg_entry_t cfg_auth[CMAX_ENTRIES];	/* auth tree */
	uxmi_cfg_entry_t cfg_ip[CMAX_ENTRIES];		/* ip networking tree */
	uxmi_cfg_entry_t cfg_acl[CMAX_ENTRIES];		/* acl tree */
	u16 last_index;					/* last_index */
} uxmi_cfg_t;


/* externs */

extern void uxmi_halt(void);
extern void uxmi_maint(void);
extern void uxmi_multiuser(void);
extern void uxmi_upgrade(void);
extern void uxmi_reboot(void);
extern void uxmi_sysinit(void);

extern void uxmi_color_reset(void);
extern void uxmi_color_bg(int uxmi_color);
extern void uxmi_color_fg(int uxmi_color);

extern int uxmi_system(char *xcmd);
extern int uxmi_do_cfg(void);
