/*
 * Linux Image Boot System (LImBS)
 * Copyright (c) 2002-2004 Spliced Networks LLC.
 * All Rights Reserved.
 *
 * Written by John Buswell <johnb@splicednetworks.com>
 *
 * 02-Apr-2004: [Release 1.0.0 rc1]
 * 03-Apr-2004: [Release 1.0.0 rc2]
 * 05-Apr-2004: [Release 1.0.0 fc1]
 * 26-Apr-2004: [Release 1.0.0 fc2]
 *
 * The following is the roadmap for future releases.
 *
 * <Release 1.0.1>
 *	+ Implement Linux Software RAID support (limbs_raid())
 *	+ Implement Volume Manager support (limbs_volmgr())
 *	+ Add reiserfs and xfs checkfs support (limbs_chkfs())
 *      + Add support for compressed images (gzip/bzip2)
 *	+ Add support for fast boot configuration option (boot.cfg)
 *
 * <Release 1.0.2>
 *	+ Failure / Recovery support:
 *		- CHKFS failure recovery or drop to maintenance mode
 *		- RIMT failure, needs to auto-recover
 *		- Handle image loading failures (limbs_loadimg)
 *		- Handle pivot root or chroot failure
 *		- checkfs failure, retry, ctrl+c drops to maintenance mode, syn/lib error
 *		- add checking for mount result codes and error handling
 *		- Handle missing config files (attempt auto detection)
 *
 * <Release 1.0.3>
 *	+ Add support for check failure, failure avoidance feature
 *	+ Add support for compressed file systems
 *
 * <Release 1.0.4>
 *	+ Add networking support
 *	+ Implement tftp, scp, rsync, ftp and http transfer options
 *
 * <Release 1.0.5>
 *	+ Menu drive maintenance mode
 *	+ Support for non-ext2 image types
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <linux/unistd.h>
#include <ncurses.h>
#include <curses.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* uncomment the following lines to use debug mode (commands don't run) */
/* #define LIMBS_DEBUG		1 */
/* #define LIMBS_DEBUG_VERBOSE	1 */

/* definitions */

/* result codes, will be used by the error handler for self-healing */
#define LRC_CHKFS_NEXT		0x000000
#define LRC_CHKFS_OK		0x000001
#define LRC_CHKFS_FAIL		0x000002
#define LRC_RIMT_OK		0x000004
#define LRC_RIMT_FAIL		0x000008
#define LRC_RBCFG_OK		0x000010
#define LRC_RBCFG_FAIL		0x000020
#define LRC_RBCFG_MISSING	0x000040
#define LRC_LCFG_MISSING	0x000080
#define LRC_LOADIMG_OK		0x000100

#define LIMBS_MMODE_WAIT	3000
#define LIMBS_BOOT_WAIT		1500000
#define LIMBS_CFG_MAX		4
#define BOOT_CFG_MAX		9
#define LIMBS_VERSION		"1.0.0"
#define LIMBS_ARCH		"x86/32"
#define LIMBS_RELEASE		"fc2"
#define SN_COPYRIGHT		"Copyright (c) 2002-2004 Spliced Networks LLC."
#define	B_WH			23		/* Boot Window Height */
#define B_WA			17		/* Boot Window Area */
#define LIMBS_NEWROOT		"/os-root"
#define LIMBS_OLDROOT		"/os-root/os/boot/mgr"
#define LIMBS_IMAGES		"/os-root/os/images"
#define LIMBS_ACTIVE		"/os-root/os/images/active"
#define LIMBS_OSINIT		"/os/boot/init"
#define LIMBS_CONSOLE		"/dev/console"

/* supported filesystems */
#define LFS_EXT2		0
#define LFS_EXT3		1
#define LFS_REISERFS		2
#define LFS_JFS			3
#define LFS_XFS			4
#define LFS_ISO9660		5
#define LFS_CRAMFS		6
#define LFS_AUTO		7

/* e2fsck result codes */
#define LFS_E2FSCK_OK             0		/* ok */
#define LFS_E2FSCK_FIXED          1		/* fixed errors and ok */
#define LFS_E2FSCK_FIXEDREBOOT    2		/* fixed errors, needs reboot */
#define LFS_E2FSCK_NOTFIXED       4		/* errors found but not fixed */
#define LFS_E2FSCK_OPERR          8		/* operation error */
#define LFS_E2FSCK_SYNERR        16		/* developer screwed up? */
#define LFS_E2FSCK_USRERR        32		/* idiot hit ctrl+c :) */
#define LFS_E2FSCK_LIBERR       128		/* developer REALLY screwed up */

/* reiserfsck result codes */
#define LFS_REISERFSCK_OK	  0		/* ok */
#define LFS_REISERFSCK_FIXED	  1		/* fixed */
#define LFS_REISERFSCK_FIXBOOT	  2		/* fixed, needs reboot */
#define LFS_REISERFSCK_FATALRT	  4		/* --rebuild-tree, maint mode */
#define LFS_REISERFSCK_FATALFF	  6		/* do --fix-fixable */
#define LFS_REISERFSCK_OPERR	  8		/* operational error */
#define LFS_REISERFSCK_SYNERR	 16		/* syntax error */

/* boot image locations, file is only one implemented */
#define BOOT_IMAGE_TYPE_UNKNOWN   0
#define BOOT_IMAGE_TYPE_FILE	  1
#define BOOT_IMAGE_TYPE_TFTP	  2
#define BOOT_IMAGE_TYPE_SCP	  3
#define BOOT_IMAGE_TYPE_RSYNC     4
#define BOOT_IMAGE_TYPE_FTP       5
#define BOOT_IMAGE_TYPE_HTTP      6

/* structures */
typedef unsigned int u8;
typedef unsigned long u16;
typedef unsigned long long u32;

/* limbs.cfg format:
 *
 *  [config-id]
 *  root_dev = /dev/<device>;
 *  image_dev = /dev/<device>;
 *  root_fs = <fs_type>;
 *  image_fs = <fs_type>;
 *  raid = <yes|no>;
 *  volmgr = <yes|no>;
 *  network = <yes|no>;
 *  ip = <ip/mask>;
 *  net_dev = <iface>;
 *
 *  limbs supports up to 4 configurations, you can order the config
 *  options in any order, however, each config option must end in a ;
 *  and each seperate config must be seperated by a [identifier]. The
 *  identifier can be anything and is ignored.
 *
 *  root_dev = root device containing the rootfs
 *  image_dev = device containing the imagefs
 *  root_fs = the type of filesystem on the root device
 *  image_fs = the type of filesystem on the image device
 *  raid = does the system use software raid 
 *  volmgr = does the system use volume managers (eg. EVM, LVM etc)
 *  network = enable networking support in LImBS
 *  ip = ip/mask, with the mask in slash notation, if 0.0.0.0/0, dhcp is used
 *  net_dev = interface for the network connection (eg. eth0, eth1 etc)
 *
 *  notes: raid, and volmgr options enable the ability to use utilities to
 *         setup, manage, recover, backup and restore raid arrays using
 *         linux software raid.
 *
 *         the volmgr options, enable features to use your favourite volmgr.
 *         these utils are designed to provide recovery, and easy setup.
 *         they are also intended for booting using raid/lvm devices as root,
 *         where the images are stored on the array or transferred via scp.
 *
 */
typedef struct {
	char *root_dev;			/* root device */
	char *image_dev;		/* image device */
	u8 root_fs;			/* root fs */
	u8 image_fs;			/* image fs */
	u8 raid;			/* raid */
	u8 volmgr;			/* volume manager */
	u8 net;				/* network support */
	u8 cfgd;			/* configured ? */
	char *limb_ip;			/* ip/mask */
	char *limb_if;			/* interface */
} limbs_cfg_t;

typedef struct {
	u8 cfgd;			/* configured ? */
	u8 type;			/* file type */
	char *boot_img;			/* filename */
	char *server_ip;		/* server */
	char *user;			/* username */
} boot_cfg_t;

/* This is for pivot_root */
static
_syscall2(int,pivot_root,const char *,new_root,const char *,put_old)

/* Prototypes */
void limbs_disp_init(void);
void limbs_scr_layout(int MWH);
void limbs_create_border(int stop_h, int stop_w, int start_h, int start_w);
void limbs_window_refresh(void);
void limbs_banner(void);
void limbs_scr_clear(int area);
void limbs_init(void);
void limbs_readcfg(void);
void limbs_printf(char *text, int cr);
u8   limbs_yesno(char *lq);
u8   limbs_fs(char *whatfs);
void limbs_debug(void);
void limbs_disp_close(void);
void limbs_mmode(void);
void limbs_set_bold(int limbs_bold_on);
u8   limbs_checkfs(void);
u8   limbs_rimnt(void);
u8   limbs_rbcfg(void);
u8   limbs_chkfail(void);
u8   limbs_loadimg(void);
u8   limbs_boot(void);
void limbs_erh(u8 rc);
u8   limbs_raid(u8 index);
u8   limbs_volmgr(u8 index);
u8   limbs_system(char *cmd);

/* Globals */
boot_cfg_t bcfg[BOOT_CFG_MAX];
limbs_cfg_t lcfg[LIMBS_CFG_MAX];
WINDOW *t_status_win, *main_win;
int max_h, max_w, center_h, center_w;
int window_start_h, window_start_w;
int window_stop_h, window_stop_w;
int cy, cx, lc, bdx;
int cindex, bindex;
int fd_con;

int main(int argc, char **argv, char **envp)
{

	int ch;
	u8 rc = 0;
	cindex = 0;

#if !defined(LIMBS_DEBUG)
	/* We should be the linuxrc on the initial ramdisk that
         * is used at boot time. The console hasn't been opened
         * yet, so we need to do that so we can see :)
         */
	fd_con = open(LIMBS_CONSOLE, O_RDONLY | O_NONBLOCK);
	system("/sbin/mount proc /proc -t proc");
#endif /* !defined(LIMBS_DEBUG) */

	limbs_disp_init();		/* init display */
	limbs_banner();			/* display banner */
	limbs_init();			/* read the local cfg */
#if defined(LIMBS_DEBUG_VERBOSE)
	limbs_debug();			/* output structure info */
#endif /* defined(LIMBS_DEBUG_VERBOSE) */
	limbs_mmode();			/* see if user is going to override */

	/* being boot process */
        /* stage 1: check the root partitions */
	rc = limbs_checkfs();
	if (rc != LRC_CHKFS_OK) {
	    /* Unrecoverable FS error */
	    /* XXX - drop to maintenance mode if network disabled */
	    /* XXX - try network recovery mode (NOT IMPLEMENTED YET) :) */
	    cindex = 1;			/* XXX this is a hack */
	}
	system("/sbin/umount /proc");

	/* stage 2: mount root and image partitions */
	rc = limbs_rimnt();
	if (rc != LRC_RIMT_OK) {
	    /* Unrecoverable mount problem */
	    /* XXX - call error handler */
	    /* XXX - if error handler failed, then drop to maintenance mode */
	}

	/* stage 3: read /os-root/os/boot/boot.cfg */
	rc = limbs_rbcfg();

	/* XXX stage 4: check and process /os-root/etc/AppOS/failure */
//	rc = limbs_chkfail();

	/* stage 5: losetup, and mount system image */
	rc = limbs_loadimg();

	/* stage 6: pivot root, and chroot */
	rc = limbs_boot();

	/* stage 7: fatal error handling */
	limbs_disp_close();
	limbs_system("/bin/ash");		/* we can do better :) */

#if defined(LIMBS_DEBUG_VERBOSE)
	while ((ch = getch()) != 10) { } /* While loop to wait for <enter> */
	limbs_scr_clear(B_WA);		/* clear screen areas */
	limbs_disp_close();
#endif /* defined(LIMBS_DEBUG_VERBOSE) */

	exit(0);
}

/*
 * Function: limbs_disp_close
 * Purpose : Closes the ncurses style display
 * 
 * Passed  : void
 * Returns : void
 *
 * Impact  : Returns the console to its former self
 *
 */
void
limbs_disp_close(void)
{
	echo();
	noraw();
	nocbreak();
	refresh();
	endwin();
	return;
}

/*
 * Function: limbs_disp_init
 * Purpose : Initialize ncurses style display
 * 
 * Passed  : void
 * Returns : void
 *
 * Impact  : Places the console in raw, ncurses mode.
 *
 */
void 
limbs_disp_init(void)
{
	cy = 0;				/* line position of cursor */
	cx = 0;				/* line position of cursor */
	lc = 0;
	initscr();			/* initialize ncurses screen */
	raw();				/* enter raw mode */
	start_color();			/* start colour */
	cbreak();			/* disable buffering */
	keypad(stdscr, TRUE);		/* enable function keys */
	noecho();			/* turn off echo */
	getmaxyx(stdscr, max_h, max_w); /* get max x,y coords */
	refresh();			/* refresh the screen */

  	/*  Initialize color pairs  */
  	init_pair(1, COLOR_RED, COLOR_BLACK);
  	init_pair(2, COLOR_GREEN, COLOR_BLACK);
  	init_pair(3, COLOR_YELLOW, COLOR_BLACK);
  	init_pair(4, COLOR_BLUE, COLOR_BLACK);
  	init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
  	init_pair(6, COLOR_CYAN, COLOR_BLACK);
  	init_pair(7, COLOR_WHITE, COLOR_BLACK);
  	init_pair(8, COLOR_BLACK, COLOR_BLACK);

	limbs_scr_layout(B_WH);		/* create the screen layout */

}

/*
 * Function: limbs_scr_layout
 * Purpose : Display banner on the console to the user
 * 
 * Passed  : int MWH
 * Returns : void
 *
 * Impact  : Displays text to the user.
 *
 */
void
limbs_scr_layout(int MWH)
{

	/*  Create Top Status Window  */
	window_start_h = 1;
	window_start_w = 1;
	window_stop_h = 4;
	window_stop_w = max_w - 3;
	t_status_win = newwin((window_stop_h - window_start_h - 1), (window_stop_w - window_start_w - 1),
			      (window_start_h + 1), (window_start_w + 1));
	limbs_create_border(window_stop_h, window_stop_w, window_start_h, window_start_w);

  	/*  Create Main Window  */
  	window_start_h = 5;
  	window_start_w = 1;
  	window_stop_h = MWH;
  	window_stop_w = max_w - 3;
  	main_win = newwin((window_stop_h - window_start_h - 1), (window_stop_w - window_start_w - 1),
                      	  (window_start_h + 1), (window_start_w + 1));
  	limbs_create_border(window_stop_h, window_stop_w, window_start_h, window_start_w);
  	scrollok(main_win, TRUE);
  	idlok(main_win, TRUE);
	keypad(main_win, TRUE);

  	limbs_window_refresh();    /* Refreshes the windows */

}

/*
 * Function: limbs_create_border
 * Purpose : creates nice border
 * 
 * Passed  : int stop_h, int stop_w, int start_h, int start_w
 * Returns : void
 *
 * Impact  : displays stuff
 *
 */
void 
limbs_create_border(int stop_h, int stop_w, int start_h, int start_w) 
{
	int max_h, max_w, center_h, center_w;

	init_pair(8, COLOR_BLACK, COLOR_BLACK);
	getmaxyx(stdscr, max_h, max_w);

	attron(COLOR_PAIR(8));
	attron(A_BOLD);
	mvaddch(start_h, start_w, ACS_ULCORNER);
  	mvaddch(start_h, stop_w, ACS_URCORNER);
  	mvaddch(stop_h, start_w, ACS_LLCORNER);
  	mvaddch(stop_h, stop_w, ACS_LRCORNER);
  	mvhline(start_h,(start_w + 1), ACS_HLINE, (stop_w - start_w - 1));
  	mvhline(stop_h,(start_w + 1), ACS_HLINE, (stop_w - start_w - 1));
  	mvvline((start_h + 1),start_w, ACS_VLINE, (stop_h - start_h - 1));
  	mvvline((start_h + 1),stop_w, ACS_VLINE, (stop_h - start_h - 1));
  	attroff(COLOR_PAIR(8));
  	attroff(A_BOLD);
  	refresh();
}

/*
 * Function: limbs_window_refresh
 * Purpose : refreshes limbs window
 * 
 * Passed  : void
 * Returns : void
 *
 * Impact  : refreshes window
 *
 */
void 
limbs_window_refresh(void)
{
	wrefresh(t_status_win);
	wrefresh(main_win);
}


/*
 * Function: limbs_banner
 * Purpose : Display banner on the console to the user
 * 
 * Passed  : void
 * Returns : void
 *
 * Impact  : Displays text to the user.
 *
 */
void 
limbs_banner(void)
{
	mvwprintw(t_status_win,0,0,"AppOS (tm) Linux Image Boot System %s, version %s [%s]",LIMBS_ARCH, LIMBS_VERSION, LIMBS_RELEASE);
	mvwprintw(t_status_win,1,0,"%s",SN_COPYRIGHT);
	limbs_printf("Initializing system...", 2);
}

/*
 * Function: limbs_scr_clear
 * Purpose : clears window
 * 
 * Passed  : int area
 * Returns : void
 *
 * Impact  : clears window.
 *
 */
void
limbs_scr_clear(int area)
{
	int tc;
	for (tc = 0; tc <= area; tc++) {
	    mvwprintw(main_win,tc,0,"                                                                                ");
	}
}

/*
 * Function: limbs_init
 * Purpose : Initialize LImBS
 * 
 * Passed  : void
 * Returns : void
 *
 * Impact  : determines LImBS recovery mode, initializes system
 *
 */
void 
limbs_init(void)
{
	/* read the config, and save the options */
	char ftmp[100];
	char *buf;
	char c;
	FILE *stream;
	int i=0;
	int key = 0;
	int index = 0;
	int skip = 0;
	int parsed = 0;

	for (index=0; index <= LIMBS_CFG_MAX;index++) {
	    lcfg[index].cfgd = 0;
	}

	for (index=0; index <= BOOT_CFG_MAX;index++) {
	    bcfg[index].cfgd = 0;
	}

	index = 0;

	buf = (char *) malloc(sizeof(ftmp));
	stream = fopen("limbs.cfg","r");
	if (stream == (FILE *) 0) {
	     limbs_erh(LRC_LCFG_MISSING);
	     return;
	}
	while(!feof(stream)) {
	    c = (char) fgetc(stream);
	    if (c == '[') {
		skip = 1;
		if (parsed) {
		 index++;
		}
		if (index > LIMBS_CFG_MAX) break;
		continue;
	    } else if (c == ']') {
		skip = 0;
		continue;
	    }
	    if ((c == '\n')) continue;
	    if ((c == ' ') || (skip)) {
		continue;
	    } else if (c == '=') {
		parsed = 1;
		lcfg[index].cfgd = 1;
		buf[i] = '\0';
		if (!strncmp(buf,"root_dev", 8)) {
		    key = 1;
		} else if (!strncmp(buf,"image_dev", 9)) {
		    key = 2;
		} else if (!strncmp(buf,"root_fs", 7)) {
		    key = 3;
		} else if (!strncmp(buf,"image_fs", 8)) {
		    key = 4;
		} else if (!strncmp(buf,"raid", 4)) {
		    key = 5;
		} else if (!strncmp(buf,"volmgr", 6)) {
		    key = 6;
		} else if (!strncmp(buf,"network", 7)) {
		    key = 7;
		} else if (!strncmp(buf,"ip", 2)) {
		    key = 8;
		} else if (!strncmp(buf,"net_dev", 7)) {
		    key = 9;
		} else {
		    key = 0;
		}
		i = 0;
		memset(buf,0,sizeof(ftmp));
	    } else if (c == ';') {
		buf[i] = '\0';
		switch (key) {
		    case 1:
			lcfg[index].root_dev = (char *) malloc(sizeof(ftmp));
			strcpy(lcfg[index].root_dev, buf);
		    	break;
		    case 2:
			lcfg[index].image_dev = (char *) malloc(sizeof(ftmp));
			strcpy(lcfg[index].image_dev, buf);
		    	break;
		    case 3:
			lcfg[index].root_fs = limbs_fs(buf);
		    	break;
		    case 4:
			lcfg[index].image_fs = limbs_fs(buf);
		    	break;
		    case 5:
			lcfg[index].raid = limbs_yesno(buf);
		    	break;
		    case 6:
			lcfg[index].volmgr = limbs_yesno(buf);
		    	break;
		    case 7:
			lcfg[index].net = limbs_yesno(buf);
		    	break;
		    case 8:
			lcfg[index].limb_ip = (char *) malloc(sizeof(ftmp));
			strcpy(lcfg[index].limb_ip, buf);
		    	break;
		    case 9:
			lcfg[index].limb_if = (char *) malloc(sizeof(ftmp));
			strcpy(lcfg[index].limb_if, buf);
		    	break;
		    default:
		    	break;
		}
		i = 0;
		key = 0;
		memset(buf,0,sizeof(ftmp));
	    } else {
		buf[i] = tolower(c);
		i++;
	    }	
	}

	free(buf);
	fclose(stream);

}

/*
 * Function: limbs_yesno
 * Purpose : returns 1 for yes, 0 for no
 * 
 * Passed  : string
 * Returns : u8
 *
 * Impact  : reads config into bcfg structure
 *
 */
u8
limbs_yesno(char *lq)
{
	if (!strncmp(lq,"ye",sizeof("ye"))) {
	    return 1;
	} else {
	    return 0;
	}
}

/*
 * Function: limbs_fs
 * Purpose : returns what filesystem
 * 
 * Passed  : string
 * Returns : u8
 *
 * Impact  : reads config into bcfg structure
 *
 */
u8
limbs_fs(char *whatfs)
{
	if (!strncmp(whatfs,"ext2", sizeof("ext2"))) {
	    return(LFS_EXT2);
	} else if (!strncmp(whatfs,"reiser", sizeof("reiser"))) {
	    return(LFS_REISERFS);	
	} else if (!strncmp(whatfs,"jfs", sizeof("jfs"))) {
	    return(LFS_JFS);	
	} else if (!strncmp(whatfs,"xfs", sizeof("xfs"))) {
	    return(LFS_XFS);	
	} else if (!strncmp(whatfs,"iso", sizeof("iso"))) {
	    return(LFS_ISO9660);	
	} else if (!strncmp(whatfs,"cram", sizeof("cram"))) {
	    return(LFS_CRAMFS);	
	} else if (!strncmp(whatfs,"ext3", sizeof("ext3"))) {
	    return(LFS_EXT3);	
	} else {
	    return(LFS_AUTO);
	}
}

/*
 * Function: limbs_printf
 * Purpose : print text
 * 
 * Passed  : string to display, number of carrage returns
 * Returns : void
 *
 * Impact  : displays string
 *
 */
void
limbs_printf(char *text, int cr)
{
	wprintw(main_win,"%s",text);
	while(cr) {
	    wprintw(main_win,"\n");
	    cr--;
	}

	getyx(main_win,cy,cx);
	lc++;
	if (lc >= 14) {
	    usleep(400000);
	}
        limbs_window_refresh();
}
#if defined(LIMBS_DEBUG)
/*
 * Function: limbs_debug
 * Purpose : print structures
 * 
 * Passed  : void
 * Returns : void
 *
 * Impact  : displays debug information
 *
 */
void
limbs_debug(void)
{
	int i = 0;
	int x = 0;
	char foo[128];
	limbs_printf("Config Debug Information: ",2);
	
	for (i = 0; i <= LIMBS_CFG_MAX; i++) {
		if (lcfg[i].cfgd) x++;
	}
	sprintf(foo,"%d configurations found.",x);
	limbs_printf(foo,1);

	for (i = 0; i <= LIMBS_CFG_MAX; i++) {
		if (!lcfg[i].cfgd) continue;
		sprintf(foo,"[Config %d]",i);
		limbs_printf(foo,2);
		sprintf(foo,"root = %s",lcfg[i].root_dev);
		limbs_printf(foo,1);
		sprintf(foo,"image = %s",lcfg[i].image_dev);
		limbs_printf(foo,1);
		sprintf(foo,"rootfs = %d",lcfg[i].root_fs);
		limbs_printf(foo,1);
		sprintf(foo,"imagefs = %d",lcfg[i].image_fs);
		limbs_printf(foo,1);
		sprintf(foo,"raid = %d",lcfg[i].raid);
		limbs_printf(foo,1);
		sprintf(foo,"volmgr = %d",lcfg[i].volmgr);
		limbs_printf(foo,1);
		sprintf(foo,"network = %d",lcfg[i].net);
		limbs_printf(foo,1);
		sprintf(foo,"ip = %s",lcfg[i].limb_ip);
		limbs_printf(foo,1);
		sprintf(foo,"if = %s",lcfg[i].limb_if);
		limbs_printf(foo,2);
	}
	limbs_printf("End Debug Information: ",2);
}
#endif /* defined(LIMBS_DEBUG) */

/*
 * Function: limbs_set_bold
 * Purpose : enable/disable bold
 * 
 * Passed  : int 0 = off, 1 = on
 * Returns : void
 *
 * Impact  : sets bold on or off
 *
 */
void
limbs_set_bold(int limbs_bold_on)
{
    if (limbs_bold_on) {
        wattron(main_win,A_BOLD);
    } else {
        wattroff(main_win,A_BOLD);
    }
}

/*
 * Function: limbs_mmode
 * Purpose : prompt user for maintenance mode
 * 
 * Passed  : void
 * Returns : void
 *
 * Impact  : switches to maintenance mode
 *
 */
void
limbs_mmode(void)
{
	int ch;
#if defined(LIMBS_DEBUG_VERBOSE)
	char foo[20];
#endif /* defined(LIMBS_DEBUG_VERBOSE) */

	limbs_set_bold(1);
	limbs_printf("Press <F9> for maintenance mode",2);
	limbs_set_bold(0);
	wtimeout(main_win, LIMBS_MMODE_WAIT);
	ch = wgetch(main_win);
#if defined(LIMBS_DEBUG_VERBOSE)
	sprintf(foo,"\n key = %d\n",ch);
	limbs_printf(foo,2);
#endif /* defined(LIMBS_DEBUG_VERBOSE) */
	switch (ch) {
	   case KEY_F(9):
	    limbs_printf("Entering maintenance mode....",3);
	    def_prog_mode();
	    endwin();
	    /* XXX insert menu driven maintenance mode in here */
	    limbs_system("/bin/sh");
	    reset_prog_mode();
	    refresh();
	    break;

	   default:
		limbs_printf("Booting...",2);
		break;
	}
}

/*
 * Function: limbs_checkfs
 * Purpose : runs the appropriate fs check and handles errors
 * 
 * Passed  : void
 * Returns : u8 selected config
 *
 * Impact  : checks file systems and selects config to use for boot
 *
 */
u8   
limbs_checkfs(void)
{
	u8 i = 0;
        u8 rc = 0;
	char *fscmd;
	char ftmp[100];

	for (i=0;i < LIMBS_CFG_MAX;i++) {
	    if (!lcfg[i].cfgd) continue;
	    if (lcfg[i].raid) {
		rc = limbs_raid(i);
		if (!rc) continue;		/* error with this cfg */
		cindex = i;
		return(rc);
	    } else if (lcfg[i].volmgr) {
		rc = limbs_volmgr(i);
		if (!rc) continue;
		cindex = i;
		return(rc);
	    } else {
		/* standard setup, lets pick the fschk tool and run it */
		switch (lcfg[i].root_fs) {
		    case LFS_EXT2:
		    case LFS_EXT3:
			sprintf(ftmp,"/sbin/e2fsck -y %s 2>/dev/null 1>/dev/null",lcfg[i].root_dev);
			fscmd = (char *) malloc(sizeof(ftmp));
			strcpy(fscmd,ftmp);
			break;

		    case LFS_REISERFS:
			sprintf(ftmp,"/sbin/reiserfsck --check -q -y %s",lcfg[i].root_dev);
			fscmd = (char *) malloc(sizeof(ftmp));
			strcpy(fscmd,ftmp);
			break;

		    case LFS_JFS:
			sprintf(ftmp,"/sbin/jfs_fsck -y %s",lcfg[i].root_dev);
			fscmd = (char *) malloc(sizeof(ftmp));
			strcpy(fscmd,ftmp);
			break;

		    case LFS_XFS:		/* XXX */
			sprintf(ftmp,"/sbin/e2fsck -y %s",lcfg[i].root_dev);
			fscmd = (char *) malloc(sizeof(ftmp));
			strcpy(fscmd,ftmp);
			break;

		    case LFS_ISO9660:
		    case LFS_CRAMFS:
		    default:
			fscmd = (char *) malloc(sizeof(1));
			fscmd[0] = '\0';
			break;
		}
		if (fscmd[0] != '\0') {
		    rc = limbs_system(fscmd);
		} else rc = LFS_E2FSCK_OK;
		switch (rc>>8) {
			case LFS_E2FSCK_OPERR:
			case LFS_E2FSCK_NOTFIXED:
			    /* XXX retry until max retries fail, then log failure */
			    break;

			case LFS_E2FSCK_USRERR:
			    /* XXX someone hit ctrl+c, should go to maint mode */
			    break;

			case LFS_E2FSCK_SYNERR:
			case LFS_E2FSCK_LIBERR:
			    /* XXX problem with e2fsck, need to handle it */
			    break;

			case LFS_E2FSCK_FIXEDREBOOT:
			case LFS_E2FSCK_FIXED:
			case LFS_E2FSCK_OK:
			default:
			    break;
		}
		free(fscmd);		    
		switch (lcfg[i].image_fs) {
		    case LFS_EXT2:
		    case LFS_EXT3:
			sprintf(ftmp,"/sbin/e2fsck -y %s 2>/dev/null 1>/dev/null",lcfg[i].image_dev);
			fscmd = (char *) malloc(sizeof(ftmp));
			strcpy(fscmd,ftmp);
			break;

		    case LFS_REISERFS:
			sprintf(ftmp,"/sbin/reiserfsck --check -q -y %s",lcfg[i].image_dev);
			fscmd = (char *) malloc(sizeof(ftmp));
			strcpy(fscmd,ftmp);
			break;

		    case LFS_JFS:
			sprintf(ftmp,"/sbin/jfs_fsck -y %s",lcfg[i].image_dev);
			fscmd = (char *) malloc(sizeof(ftmp));
			strcpy(fscmd,ftmp);
			break;

		    case LFS_XFS:
			sprintf(ftmp,"/sbin/e2fsck -y %s",lcfg[i].image_dev);
			fscmd = (char *) malloc(sizeof(ftmp));
			strcpy(fscmd,ftmp);
			break;

		    case LFS_ISO9660:
		    case LFS_CRAMFS:
		    default:
			fscmd = (char *) malloc(sizeof(1));
			fscmd[0] = '\0';
			break;
		}
		if (fscmd[0] != '\0') {
		    rc = limbs_system(fscmd);
		} else rc = LFS_E2FSCK_OK;
		switch (rc>>8) {
			case LFS_E2FSCK_OPERR:
			case LFS_E2FSCK_NOTFIXED:
			    /* XXX retry until max retries fail, then log failure */
			    break;

			case LFS_E2FSCK_USRERR:
			    /* XXX someone hit ctrl+c, should go to maint mode */
			    break;

			case LFS_E2FSCK_SYNERR:
			case LFS_E2FSCK_LIBERR:
			    /* XXX problem with e2fsck, need to handle it */
			    break;

			case LFS_E2FSCK_FIXEDREBOOT:
			case LFS_E2FSCK_FIXED:
			case LFS_E2FSCK_OK:
			default:
			    rc = LRC_CHKFS_OK;
			    break;
		}
		free(fscmd);
	    }
	    if (rc = LRC_CHKFS_OK) {
		cindex = i;
		return(LRC_CHKFS_OK);
	    }
	}

	if (rc == LRC_CHKFS_FAIL) {
	    limbs_erh(rc);			/* handle the error */
	}

}

/*
 * Function: limbs_rimnt
 * Purpose : mounts root and image filesystems
 * 
 * Passed  : void
 * Returns : u8
 *
 * Impact  : mounts file systems
 *
 */
u8   
limbs_rimnt(void)
{

	char *mount = "/sbin/mount -n";
	char fst[10];
	char fcmd[255];
	char llog[255];
	u8 rc = 0;

	if (lcfg[cindex].root_fs != LFS_AUTO) {
	    switch(lcfg[cindex].root_fs) {
		case LFS_EXT3:
		    strcpy(fst,"ext3");
		    break;
		case LFS_REISERFS:
		    strcpy(fst,"reiserfs");
		    break;
		case LFS_JFS:
		    strcpy(fst,"jfs");
		    break;
		case LFS_XFS:
		    strcpy(fst,"xfs");
		    break;
		case LFS_ISO9660:
		    strcpy(fst,"iso9660");
		    break;
		case LFS_CRAMFS:
		    strcpy(fst,"cramfs");
		    break;
		case LFS_EXT2:
		    strcpy(fst,"ext2");
		default:
		    break;
	    }
	    sprintf(fcmd,"%s -t %s %s %s",mount, fst, lcfg[cindex].root_dev, LIMBS_NEWROOT);
	    sprintf(llog," + Mounting %s -> %s (%s)", lcfg[cindex].root_dev, LIMBS_NEWROOT, fst);
	} else {
	    sprintf(fcmd,"%s %s %s",mount, lcfg[cindex].root_dev, LIMBS_NEWROOT);
	    sprintf(llog," + Mounting %s -> %s", lcfg[cindex].root_dev, LIMBS_NEWROOT);
	}

	limbs_printf(llog,1);
	rc = limbs_system(fcmd);
	
	/* XXX do a check here, set the appropriate rc */

	if (lcfg[cindex].image_fs != LFS_AUTO) {
	    switch(lcfg[cindex].image_fs) {
		case LFS_EXT3:
		    strcpy(fst,"ext3");
		    break;
		case LFS_REISERFS:
		    strcpy(fst,"reiserfs");
		    break;
		case LFS_JFS:
		    strcpy(fst,"jfs");
		    break;
		case LFS_XFS:
		    strcpy(fst,"xfs");
		    break;
		case LFS_ISO9660:
		    strcpy(fst,"iso9660");
		    break;
		case LFS_CRAMFS:
		    strcpy(fst,"cramfs");
		    break;
		case LFS_EXT2:
		    strcpy(fst,"ext2");
		default:
		    break;
	    }
	    sprintf(fcmd,"%s -t %s %s %s",mount, fst, lcfg[cindex].image_dev, LIMBS_IMAGES);
	    sprintf(llog," + Mounting %s -> %s (%s)", lcfg[cindex].image_dev, LIMBS_IMAGES, fst);
	} else {
	    sprintf(fcmd,"%s %s %s",mount, lcfg[cindex].image_dev, LIMBS_IMAGES);
	    sprintf(llog," + Mounting %s -> %s", lcfg[cindex].image_dev, LIMBS_IMAGES);
	}

	limbs_printf(llog,1);
	rc = limbs_system(fcmd);

	/* XXX do a check here, set the appropriate rc */

	rc = LRC_RIMT_OK;
	return(rc);
}

/*
 * Function: limbs_rbcfg
 * Purpose : reads boot config from /os-root/os/boot/boot.cfg
 * 
 * Passed  : void
 * Returns : u8
 *
 * Impact  : selects a config
 *
 */
u8   
limbs_rbcfg(void)
{
	u8 rc;
	char *buf;
	char ftmp[100];
	char c;
	FILE *bstream;
	int index = 0, i = 0;

	bindex = 0;
	bdx = 0;
	buf = (char *) malloc(sizeof(ftmp));
	bstream = fopen("/os-root/os/boot/boot.cfg","r");
	if (bstream == (FILE *) 0) {
	    limbs_erh(LRC_RBCFG_MISSING);
	    return(LRC_RBCFG_MISSING);
	}
	while(!feof(bstream)) {
	    c = (char) fgetc(bstream);
	    if ((c == '\n') || (c == ' ')) continue;
	    if (c == '=') {
		bcfg[index].cfgd = 1;
		buf[i] = '\0';
		if (!strncmp(buf,"file",4)) {
		    bcfg[index].type = BOOT_IMAGE_TYPE_FILE;
		} else if (!strncmp(buf,"tftp",4)) {
		    bcfg[index].type = BOOT_IMAGE_TYPE_TFTP;
		} else if (!strncmp(buf,"scp",3)) {
		    bcfg[index].type = BOOT_IMAGE_TYPE_SCP;
		} else if (!strncmp(buf,"rsync",5)) {
		    bcfg[index].type = BOOT_IMAGE_TYPE_RSYNC;
		} else if (!strncmp(buf,"ftp",3)) {
		    bcfg[index].type = BOOT_IMAGE_TYPE_FTP;
		} else if (!strncmp(buf,"http",4)) {
		    bcfg[index].type = BOOT_IMAGE_TYPE_HTTP;
		} else bcfg[index].type = BOOT_IMAGE_TYPE_UNKNOWN;
		i = 0;
		memset(buf,0,sizeof(ftmp));
	    } else if (c == ';') {
		buf[i] = '\0';
		switch(bcfg[index].type) {
		    case BOOT_IMAGE_TYPE_FILE:
			bcfg[index].boot_img = (char *) malloc(sizeof(ftmp));
			strcpy(bcfg[index].boot_img, buf);
			if (!bdx) {
			   bindex = index;
			   bdx = 1;
			}
			index++;
			break;

		    case BOOT_IMAGE_TYPE_TFTP:
		    case BOOT_IMAGE_TYPE_SCP:
		    case BOOT_IMAGE_TYPE_RSYNC:
		    case BOOT_IMAGE_TYPE_FTP:
		    case BOOT_IMAGE_TYPE_HTTP:
		    case BOOT_IMAGE_TYPE_UNKNOWN:
		    default:
			/* ignore */
			bcfg[index].cfgd = 0;
			break;
		}
		i = 0;
		memset(buf,0,sizeof(ftmp));
	    } else {
		buf[i] = tolower(c);
		i++;
	    }
	}
	free(buf);
	fclose(bstream);
	return(LRC_RBCFG_OK);
}

/*
 * Function: limbs_chkfail
 * Purpose : checks for failure reports in /os-root/etc/os/failure
 * 
 * Passed  : u8 result code
 * Returns : void
 *
 * Impact  : information used to try to avoid previous failures
 *
 */
u8   
limbs_chkfail(void)
{
	/* NOT IMPLEMENTED */
}

/*
 * Function: limbs_loadimg
 * Purpose : losetup, and mounts image system
 * 
 * Passed  : void
 * Returns : u8 result code
 *
 * Impact  : critical - prepares system for boot
 *
 */
u8   
limbs_loadimg(void)
{
	char *losetup = "/sbin/losetup";
	char *mount = "/sbin/mount -n";
	char fcmd[255];
        char llog[255];
        u8 rc = 0;

	sprintf(fcmd,"%s /dev/os-active %s/%s",losetup,LIMBS_IMAGES,
		bcfg[bindex].boot_img);
	sprintf(llog," + Assigning active image device ");
	limbs_printf(llog,1);
	limbs_system(fcmd);

	/* XXX need to unhardcode this for future features */

	sprintf(fcmd,"%s -t ext2 -o ro /dev/os-active %s",mount,LIMBS_ACTIVE);
	sprintf(llog," + Loading system image %s",bcfg[bindex].boot_img);
	limbs_printf(llog,2);
	limbs_system(fcmd);

	return(LRC_LOADIMG_OK);	

}

/*
 * Function: limbs_boot
 * Purpose : boots system
 * 
 * Passed  : void
 * Returns : u8
 *
 * Impact  : shouldn't return due to execve unless we failed
 *
 */
u8   
limbs_boot(void)
{
	char *testv[6];
	char *envp_init[10] = { "HOME=/", "TERM=linux", NULL, };
#if defined(LIMBS_DEBUG)
	char foo[100];
#endif /* defined(LIMBS_DEBUG) */
	char fcmd[100];
	char llog[255];
	time_t limbs_boot_time;

	sprintf(fcmd,"cd %s",LIMBS_NEWROOT);
        system(fcmd);

	pivot_root(LIMBS_NEWROOT,LIMBS_OLDROOT);

	testv[0] = "chroot";
	testv[1] = ".";
	testv[2] = LIMBS_OSINIT;
	testv[3] = "<dev/console";
	testv[4] = ">dev/console";
	testv[5] = "2>&1";
	testv[6] = NULL;

#if defined(LIMBS_DEBUG)
	sprintf(foo,"bin/chroot %s %s",testv[1],envp_init[0]);
#endif /* defined(LIMBS_DEBUG) */

	time(&limbs_boot_time);
	sprintf(llog,"System boot on %s",ctime(&limbs_boot_time));
	limbs_printf(llog,2);	

	usleep(LIMBS_BOOT_WAIT);
#if defined(LIMBS_DEBUG)
	limbs_printf(foo,2);
	limbs_system(foo);
#endif /* defined(LIMBS_DEBUG) */
	limbs_disp_close();			/* exit ncurses mode */
	close(fd_con);				/* close the tty */
#if !defined(LIMBS_DEBUG)
	/* XXX we should free the malloc'd memory in lcfg/bcfg here */
	execve("bin/chroot",testv,envp_init);
#endif /* defined(LIMBS_DEBUG) */

}

/*
 * Function: limbs_erh
 * Purpose : Error / Recovery Handler
 * 
 * Passed  : u8 rc
 * Returns : void
 *
 * Impact  : tries to recover from an error
 *
 */
void
limbs_erh(u8 rc)
{
	/* NOT IMPLEMENTED */
}

/*
 * Function: limbs_raid
 * Purpose : RAID check
 * 
 * Passed  : u8 index (lcfg[])
 * Returns : void
 *
 * Impact  : checks and tries to fix raid related fs issues
 *
 */
u8
limbs_raid(u8 index)
{
	/* NOT IMPLEMENTED */
}

/*
 * Function: limbs_volmgr
 * Purpose : EVM/LVM check
 * 
 * Passed  : u8 index (lcfg[])
 * Returns : void
 *
 * Impact  : checks and tries to fix volmgr related fs issues
 *
 */
u8
limbs_volmgr(u8 index)
{
	/* NOT IMPLEMENTED */
}

/*
 * Function: limbs_system
 * Purpose : system cmd wrapper
 * 
 * Passed  : char *cmd
 * Returns : u8 result code
 *
 * Impact  : just calls system, in debug mode, just prints the cmd
 *
 */
u8
limbs_system(char *cmd)
{
	u8 rc;
#if !defined(LIMBS_DEBUG)
	rc = system(cmd);
#else
	char foo_debug[255];
#if defined(LIMBS_DEBUG_VERBOSE)
	sprintf(foo_debug,"[*] %s",cmd);
	limbs_printf(foo_debug,1);
#endif /* defined(LIMBS_DEBUG_VERBOSE) */
	rc = 0;
#endif /* !defined(LIMBS_DEBUG) */

	return(rc);
}
