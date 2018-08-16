/*
 * AppOS Boot Management System (ABMS)
 * Copyright (c) 2002-2008 Spliced Networks LLC.
 * All Rights Reserved.
 *
 * Author: John Buswell <buswellj@splicednetworks.com>
 * Release: 4.0.0
 *
 * LICENSE: GPL v3
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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
 * To contact Spliced Networks LLC:
 * 
 * Spliced Networks LLC
 * 4820 Fisher Road
 * Athens, OH 45701
 * USA
 * 
 * Tel: (408) 416-3832
 * email: support@splicednetworks.com
 *
 * http://www.splicednetworks.com
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include "abms.h"

void abms_disp_init(void)
{
	char lcmd[255];
	fd_con = open(ABMS_CONSOLE, O_RDONLY | O_NONBLOCK);

	sprintf(lcmd,"%s%s",ABMS_BINPATH,ACMD_CLEAR);
	abms_system(lcmd);

	printf("\n\n");
	printf("Spliced Networks (R) Appliance Operating System Software\n");
	printf("AppOS (R) Boot Management System, Version %s\n", ABMS_VERSION);
	printf("%s\n\n",SN_COPYRIGHT);

	return;
}
