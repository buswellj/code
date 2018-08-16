/*
	+---------------------------------------+
	|                                       |
	| Ion Linux (tm) Command Line Interface |
	|                                       |
	+---------------------------------------+
	|                                       |
	| Created : Dec 14th 2003               |
        | Author  : John Buswell                |
        |                                       |
        | Filename: cli-main.c                  |
	|                                       |
	+---------------------------------------+
	|                                       |
	| This is the main cli routine. The cli |
	| provides a text based, ncurses router |
	| style interface to a linux system.    |
	|                                       |
	+---------------------------------------+
	|                                       |
	| Copyright (c) 2002-2004 		|
	|                 Spliced Networks LLC  |
	|                                       |
	| This software may not be distributed  |
	| in source form.                       |
	|                                       |
	+---------------------------------------+
 */

/* Includes */

#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>
#include <malloc.h>
#include <memory.h>

/* defines */

#define ION_MAX_HISTORY		256

/* Function Prototypes */

/* specific */
void ion_init_cli(void);
void ion_disp_welcome(void);
void ion_set_bold(int ion_bold_on);
void ion_set_prompt(char *prompt);
void ion_process_cmd(char *cmd);

/* commands */
void ion_cmd_copy(char *cmd);
void ion_cmd_help(void);
void ion_cmd_history(void);
void ion_cmd_ls(char *cmd);
void ion_cmd_mode(char *cmd);
void ion_cmd_ping(char *cmd);
void ion_cmd_restart(char *cmd);
void ion_cmd_set(char *cmd);
void ion_cmd_show(char *cmd);
void ion_cmd_ssh(char *cmd);
void ion_cmd_stats(char *cmd);
void ion_cmd_telnet(char *cmd);
void ion_cmd_traceroute(char *cmd);
void ion_cmd_who(void);
void ion_cmd_write(void);
void ion_cmd_print_top(void);

/* common */
void ion_exit(int exit_code);

/* Global Variables */

char *ion_cli_history[ION_MAX_HISTORY];
int ion_history_count;
int scrx;
int scry;

/* 
 +---------------------------------------------------------------------------+

   Function   :		ion_exit()
   Parameters :		int exit (pass EXIT_SUCCESS or EXIT_FAILURE)
   Returns    :		none

   Purpose    :		This function is called just before the program
			exits. The function is used to clean up after
			the program.

 +---------------------------------------------------------------------------+

*/

void
ion_exit(int exit_code)
{
    echo();
    noraw();
    refresh();
    endwin();			/* end curses mode, this should only be here */
    exit(EXIT_SUCCESS);		/* end of execution */

}

/* 
 +---------------------------------------------------------------------------+

   Function   :		main()
   Parameters :		argc, argv[]
   Returns    :		int

   Purpose    :		Main CLI routine

 +---------------------------------------------------------------------------+

*/

int
main(int argc, char *argv[])
{

    char *prompt, *cmd, *cmdb;
    int ch, cr, nohist;
    int y,x, minx, tmp_hist;

    ion_init_cli();				/* init stuff */
    ion_disp_welcome();				/* display welcome banner */
    ion_set_prompt(prompt);			/* set the prompt */
    minx = strlen(prompt);			/* minx is the x co-ordinate
						   for the start of the cmd
						   line. This is directly
						   after the prompt */

    /* initialize cmd line variables */
    cr = 0;					/* carrage return (enter key) */
    cmd = malloc(4096);				/* alloc mem for cmd line */
    cmdb = malloc(4096);			/* tmp buf when we backspace */
    bzero(cmd,sizeof(cmd));			
    bzero(cmdb,sizeof(cmdb));
 
    /* initialize command line history */
    tmp_hist = 0;				/* used to walk cmd history */
    nohist = 0;					/* skip saving history */
    ion_history_count = 0;			/* next cmd history entry */
    memset(ion_cli_history, 0, ION_MAX_HISTORY);
    bzero(ion_cli_history,sizeof(ion_cli_history));

    /* 
	This is the master loop of the CLI, it is all control from here.
	The cli runs in an infinite loop until the user exits. There are
	two while loops, the outer while loop only exits as the program
	exits, the inner while loop gathers cmd input until enter is hit
	at which point, the command is processed and we start again.

     */

    while(1) {

	ion_set_bold(0);		/* turn bold off */
	printw("\n\r%s",prompt);	/* set the prompt */
	refresh();			/* display the prompt */
	cr = 0;				/* enter hasn't been hit yet */

	/* get input until enter key is hit */

	while (!cr) {
	    ch = getch();			/* get input */
	    getyx(stdscr,y,x);			/* get the cursor location */
	    getmaxyx(stdscr,scry,scrx);		/* screen resized? */
	    switch (ch) {

		case KEY_LEFT:			/* nav: move left */
		     if (x > minx) {		/* don't go into prompt */
			x--;
			move(y,x);
		     }
		     break;

		case KEY_RIGHT:			/* nav: move right */
		     /* make sure we don't go past the last character typed */
		     if (x < (strlen(cmd)+minx)) { 
			x++;
			move(y,x);
		     }
		     break;


		/* history commands :

			up arrow	-	previous command in history
			down arrow	-	next command in history

		   buffer wrapping:

			The history buffer is a circular buffer, meaning
			when we hit the end, we go back to the start and
			start over-writing. 

			we must add code to both KEY_UP and KEY_DOWN
			below to handle this. we know that the position
			is currently ion_history_count, we could check to
			see if ion_history_count+1 is a null string in the
			ion_cli_history[] array, if it is, then we know we
			haven't looped around. If its not, then we should
			loop in reverse order.
		 */

		case KEY_UP:
		     if (tmp_hist && (tmp_hist < ION_MAX_HISTORY)) {
			tmp_hist--;
			if (ion_cli_history[tmp_hist]) {
			    while (x > minx) {
				x--;
		         	mvdelch(y,x);
			    }
			    bzero(cmd,sizeof(cmd));
			    x = minx;
			    delch();
			    *cmd = '\0';
			    cmd = strdup(ion_cli_history[tmp_hist]);
			    printw("%s",cmd);
			} else {
			  tmp_hist++;
			}
		     }
		     break;

		case KEY_DOWN:
		     tmp_hist++;
		     if (ion_cli_history[tmp_hist]) {
			while (x > minx) {
			    x--;
		            mvdelch(y,x);
			}
			bzero(cmd,sizeof(cmd));
			x = minx;
			delch();
			*cmd = '\0';
			cmd = strdup(ion_cli_history[tmp_hist]);
			printw("%s",cmd);
		     } else {
			while (x > minx) {
			    x--;
		            mvdelch(y,x);
			}
			tmp_hist = ion_history_count;
			bzero(cmd,sizeof(cmd));
			x = minx;
			delch();
			*cmd = '\0';
			printw("%s",cmd);			
		     }
		     break;

		/* handle break */

		case KEY_BREAK:
		case '\003':
		case '\004':
		     ion_exit(EXIT_SUCCESS);
		     break;

		/* handle enter */

		case KEY_ENTER:
		case '\n':
		case '\r':
		     cr = 1;
		     break;

		/* handle backspace, should take care of ^? and ^H.
		   this works by copying the current cmd line into a
		   temp string cmdb, clearing the current cmd line,
		   and copying back the backup over the original, minus
		   one character, and moving the cursor back 1 position.

		   if we are in the first cursor position, we just delch()
		   it and set cmd to null.
		 */

		case '\177':
		case KEY_SDC:
		case KEY_DC:
		case KEY_BACKSPACE:
		     /* bug: need to fix mid-deletion in a cmd line */
		     strcpy(cmdb,cmd);
		     if (x > minx) {
		         x--;
		         mvdelch(y,x);
			 bzero(cmd,sizeof(cmd));
		     	 strncat(cmd,cmdb,strlen(cmdb)-1);
		     } else {
			 delch();
			 *cmd='\0';
		     }
		     break;

		/* handle ? help */

		case '\077':
		    if (strlen(cmd) < 2) {
			ion_cmd_print_top();
		    }
		    cr = 1;
		    nohist = 1;
		    break;

		/* handle tab completion */

		case '\011':
		    printw("\n\r\n\rFuture home of tab completion\n\r\n\r");
		    cr = 1;
		    nohist = 1;
		    break;

		/* default, count the character as input */

		default:
		     if (ch >= 0x20) {
		     	addch(ch);
			sprintf(cmd,"%s%c",cmd,ch);
		     }
		     break;

	    }
	    refresh();			/* trigger display */
	}

	ion_process_cmd(cmd);		/* process the cmd just entered */

	/* Add and process the history buffer */

	if ((ion_history_count < ION_MAX_HISTORY) && !nohist) {
	    ion_cli_history[ion_history_count] = strdup(cmd);
	} else if (!nohist) {
	    ion_history_count = 0;
	    ion_cli_history[ion_history_count] = strdup(cmd);
	}
	if (!nohist) {
	    ion_history_count++;
	    tmp_hist = ion_history_count;
	}
	bzero(cmd,sizeof(cmd));
	nohist = 0;
	refresh();
    }
}

/* 
 +---------------------------------------------------------------------------+

   Function   :		ion_init_cli()
   Parameters :		none
   Returns    :		none

   Purpose    :		initialize the cli

 +---------------------------------------------------------------------------+

*/


void 
ion_init_cli(void)
{
    initscr();				/* start curses mode */
    raw();				/* disable line buffering */
    keypad(stdscr, TRUE);		/* enable F1, F2, etc */
    noecho();				/* disable echo */
    getmaxyx(stdscr,scry,scrx);
    idlok(stdscr,TRUE);
    scrollok(stdscr,TRUE);
}

/* 
 +---------------------------------------------------------------------------+

   Function   :		ion_disp_welcome()
   Parameters :		none
   Returns    :		none

   Purpose    :		displays initial banner

 +---------------------------------------------------------------------------+

*/
void
ion_disp_welcome(void)
{
    printw("\n\r");
    ion_set_bold(1);
    printw("Ion Linux (tm)");
    ion_set_bold(0);
    printw(" Command Line Interface (CLI)\n\r");
    printw("Copyright (c) 2002-2003 by Spliced Networks LLC.\n\r");
    printw("\n\r");
}

/* 
 +---------------------------------------------------------------------------+

   Function   :		ion_set_bold()
   Parameters :		int ion_bold_on
   Returns    :		none

   Purpose    :		toggles bold on or off

 +---------------------------------------------------------------------------+

*/

void
ion_set_bold(int ion_bold_on)
{
    if (ion_bold_on) {
	attron(A_BOLD);
    } else {
	attroff(A_BOLD);
    }
}

/* 
 +---------------------------------------------------------------------------+

   Function   :		ion_set_prompt
   Parameters :		char pointer to prompt
   Returns    :		none

   Purpose    :		creates a prompt based on hostname

 +---------------------------------------------------------------------------+

*/

void
ion_set_prompt(char *prompt)
{
    char tmp_str[128];
    int c = 0;

    /* The following code parses the hostname to create a prompt */

    if (gethostname(tmp_str, sizeof(tmp_str))) {
        strcpy(prompt,"ion");
    } else {
        while (c < sizeof(tmp_str)) {
            if (tmp_str[c] == '.') {
                prompt[c] = '\0';
                break;
            }
            prompt[c] = tmp_str[c];
            c++;
        }
    }
    strncat(prompt, "> ", sizeof(prompt));
}

/* 
 +---------------------------------------------------------------------------+

   Function   :		ion_process_cmd
   Parameters :		char pointer to cmdline
   Returns    :		none

   Purpose    :		processes user supplied command line

 +---------------------------------------------------------------------------+

*/

void
ion_process_cmd(char *cmd)
{

	unsigned char c;
	int a = 0;

	c = cmd[0];


	/* 
	    Top Level Commands
	    -------------------

		copy		-		copy
		exit		-		logout
		help		-		help system
		ls		-		ls of virtual flash
		mode		-		switch auth mode
		ping		-		icmp ping
		restart		-		reboot
		set		-		sets a config option
		show		-		show
		ssh		-		open an ssh connnection
		stats		-		display or clear stats
		telnet		-		open a telnet connection
		traceroute	-		perform traceroute
		who		-		who is logged in
		write		-		write config / apply

	 */

	switch(c) {
		case 'c':
		    if (!strcasecmp(cmd,"copy")) ion_cmd_copy(cmd);
		    break;

		case 'e':
		    if (!strcasecmp(cmd,"exit")) ion_exit(EXIT_SUCCESS);
		    break;

		case 'h':
		    if (!strcasecmp(cmd,"help")) ion_cmd_help();
		    if (!strcasecmp(cmd,"history")) ion_cmd_history();
		    break;

		case 'l':
		    if (!strcasecmp(cmd,"logoff")) ion_exit(EXIT_SUCCESS);
		    if (!strcasecmp(cmd,"logout")) ion_exit(EXIT_SUCCESS);
		    if (!strcasecmp(cmd,"ls")) ion_cmd_ls(cmd);
		    break;

		case 'm':
		    if (!strcasecmp(cmd,"mode")) ion_cmd_mode(cmd);
		    break;

		case 'p':
		    if (!strcasecmp(cmd,"ping")) ion_cmd_ping(cmd);
		    break;

		case 'q':
		    if (!strcasecmp(cmd,"quit")) ion_exit(EXIT_SUCCESS);
		    break;

		case 'r':
		    if (!strcasecmp(cmd,"reboot")) ion_cmd_restart(cmd);
		    if (!strcasecmp(cmd,"restart")) ion_cmd_restart(cmd);
		    break;

		case 's':
		    if (!strcasecmp(cmd,"set")) ion_cmd_set(cmd);
		    if (!strcasecmp(cmd,"show")) ion_cmd_show(cmd);
		    if (!strcasecmp(cmd,"ssh")) ion_cmd_ssh(cmd);
		    if (!strcasecmp(cmd,"stats")) ion_cmd_stats(cmd);
		    break;

		case 't':
		    if (!strcasecmp(cmd,"telnet")) ion_cmd_telnet(cmd);
		    if (!strcasecmp(cmd,"trace")) ion_cmd_traceroute(cmd);
		    break;

		case 'w':
		    if (!strcasecmp(cmd,"who")) ion_cmd_who();
		    if (!strcasecmp(cmd,"write")) ion_cmd_write();
		    break;

	}


}

void
ion_cmd_copy(char *cmd)
{
    printw("\n\rcopy command\n\r\n\r");
}

void
ion_cmd_help()
{
    ion_set_bold(1);
    printw("\n\r\n\rCommand Line Help\n\r\n\r");
    ion_set_bold(0);
    printw("The question mark '?' key can be pressed at any time to\n\r");
    printw("provide you with a list of available commands. Placing\n\r");
    printw("the '?' after a command shows that commands available\n\r");
    printw("options.\n\r\n\r");
    printw("You can use the up and down arrow keys to navigate the\n\r");
    printw("command line history. The left and right arrows van be\n\r");
    printw("used to edit the prompt. The tab key can be pressed to\n\r");
    printw("automatically complete the command or options name.\n\r\n\r");
}

void
ion_cmd_history()
{
    int hdump = 0;

    ion_set_bold(1);
    printw("\n\r\n\rCommand History:\n\r");
    ion_set_bold(0);

    for (hdump = 0; hdump < ion_history_count; hdump++) {
	printw("\n\r[%d]: %s",hdump,ion_cli_history[hdump]);
    }

    printw("\n\r");

}

void
ion_cmd_ls(char *cmd)
{
    printw("\n\rls command\n\r\n\r");
}

void
ion_cmd_ping(char *cmd)
{
    printw("\n\rping command\n\r\n\r");
}

void
ion_cmd_mode(char *cmd)
{
    printw("\n\rmode command\n\r\n\r");
}

void
ion_cmd_restart(char *cmd)
{
    printw("\n\rrestart command\n\r\n\r");
}

void
ion_cmd_set(char *cmd)
{
    printw("\n\rset command\n\r\n\r");
}

void
ion_cmd_show(char *cmd)
{
    printw("\n\rshow command\n\r\n\r");
}

void
ion_cmd_ssh(char *cmd)
{
    printw("\n\rssh command\n\r\n\r");
}

void
ion_cmd_stats(char *cmd)
{
    printw("\n\rstats command\n\r\n\r");
}

void
ion_cmd_telnet(char *cmd)
{
    printw("\n\rtelnet command\n\r\n\r");
}

void
ion_cmd_traceroute(char *cmd)
{
    printw("\n\rtraceroute command\n\r\n\r");
}

void
ion_cmd_who()
{
    printw("\n\rwho command\n\r\n\r");
}

void
ion_cmd_write()
{
    printw("\n\rwrite command\n\r\n\r");
}

void
ion_cmd_print_top()
{
    ion_set_bold(1);
    printw("\n\r\n\rCommands:\n\r");
    printw("----------\n\r\n\r");
    ion_set_bold(0);
    printw("copy\t\tCopy files to/from virtual image slots\n\r");
    printw("exit\t\tLogout of the system\n\r");
    printw("help\t\tDisplay help information\n\r");
    printw("ls\t\tList files in virtual image slots\n\r");
    printw("mode\t\tSwitch user authentication mode\n\r");
    printw("ping\t\tSend icmp echo packets\n\r");
    printw("restart\t\tReboot the system\n\r");
    printw("set\t\tConfigure the system\n\r");
    printw("show\t\tShow system information\n\r");
    printw("ssh\t\tEstablish a secure shell connection\n\r");
    printw("stats\t\tDisplay or clear real-time stats\n\r");
    printw("telnet\t\tEstablish a telnet connection\n\r");
    printw("traceroute\tTraceroute to destination\n\r");
    printw("who\t\tDisplay who is logged into this device\n\r");
    printw("write\t\tSave and apply any pending configuration\n\r");
}
