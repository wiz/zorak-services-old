/*
 *  Zorak IRC Services
 *
 *  Copyright (C) 2001-2002 Jason Dambrosio <jason@wiz.cx>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of version 1 of the GNU General Public License,
 *  as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the warranty of MERCHANTABILITY
 *  or FITNESS FOR A PARTICULAR PURPOSE. The GNU General Public License
 *  contains more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id: me.h,v 1.3 2002/10/02 02:56:15 wiz Exp $
 */

#include "irc_types.h"
#include "sclients.h"
#include "defines.h"
#include "net.h"

struct _me {
	int argc;
	char **argv;
	u_short servtype, channels, servers, users, opers, admins, dcc;
	unsigned int eob:1, debug:1, lifesux:1, ro:1, conn:1, gotping:1, crashing:1, opernotice:1;
	FILE *logfd;
	char md5buf[33];
	char *chans[MAXCHANS];
	char *servdesc;
	char *servname;
	sclient sclients[ALL];
	sock_t *hub, lasthub;
	struct in_addr ip;
	char hostbuf[BUFSIZE];
	time_t start;
	time_t settime;
	time_t htmtime;
	fd_set readfds, writefds;
	pid_t pid;
	size_t memusage;
	char *to;
};

extern struct _me me;
