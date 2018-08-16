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
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include "appmod.h"
#include "appmod_shared.h"

void
appmod_cli_askq(char *askprompt, int do_echo, char *cmdline)
{
        char z;
        int x = 0, y = 0, w = 0;

        if (do_echo == 0x7) {
                w = 1;
                do_echo = 0;
        }

        printf("%s", askprompt);

        while (!x) {
                z = appmod_cli_getch(do_echo);
                if (z == '\n') {
                        cmdline[y] = '\0';
                        return;
                } else {
                        if ((do_echo == 0x6) || (w == 1)) {
                                cmdline[y] = z;
                        } else {
                                cmdline[y] = tolower(z);
                        }
                        y++;
                }
        }
        cmdline[y] = '\0';

        return;
}

int
appmod_cli_getch(int do_echo)
{
        struct termios savedt, newt;
        int ch;

        if (!do_echo) {
                tcgetattr(STDIN_FILENO, &savedt);
                newt = savedt;
                newt.c_lflag &= ~(ICANON|ECHO|IGNBRK);
                tcsetattr(STDIN_FILENO,TCSANOW, &newt);
        } else {
                tcgetattr(STDIN_FILENO, &savedt);
                newt = savedt;
                newt.c_lflag &= ~(IGNBRK);
                tcsetattr(STDIN_FILENO,TCSANOW, &newt);
        }
        
        ch = getchar();
        tcsetattr(STDIN_FILENO,TCSANOW, &savedt);

        return ch;
}
