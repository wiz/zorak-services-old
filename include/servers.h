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
 *  $Id: servers.h,v 1.2 2002/07/30 03:23:01 wiz Exp $
 */

#ifndef __SERVERS_H_
#define __SERVERS_H_

typedef struct server {
	struct server *next;
	char *host;
	u_short port;
	char *pass;
	char *name;
} server_t;

void		add_server		(char *, char *, char *, int);
void		del_all_servers		(void);
u_short		is_me			(char *);
user_t *	is_juped		(char *);
server_t *	find_server_t		(char *);
int		tell_server_hash	(char *);
#endif
