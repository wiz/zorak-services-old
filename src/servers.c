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
 */

#include "irc_types.h"
#include "irc_string.h"
#include "defines.h"
#include "servers.h"
#include "config.h"
#include "privs.h"
#include "mem.h"
#include "me.h"

WIZOSID("$Id: servers.c,v 1.12 2002/07/30 03:23:02 wiz Exp $");

server_t *main_server = NULL;

void
add_server(char *host, char *pass, char *name, int port)
{
	server_t *server;

	server = leetmalloc(sizeof(server_t));
	server->next = main_server;
	main_server = server;

	server->host = leetstrdup(host);
	server->pass = leetstrdup(pass);
	server->name = leetstrdup(name);
	server->port = port;
}

void
del_all_servers(void)
{
	server_t *tmp, **all = &main_server;

	while (*all) {
		leetfree((*all)->host, strlen((*all)->host) + 1);
		leetfree((*all)->pass, strlen((*all)->pass) + 1);
		leetfree((*all)->name, strlen((*all)->name) + 1);
		tmp = *all;
		*all = (*all)->next;
		leetfree(tmp, sizeof(server_t));
	}
	main_server = NULL;
}

u_short
is_me(char *name)
{
	if (strcasecmp(me.servname, name) == 0)
		return 1;
	return 0;
}

user_t *
is_juped(char *name)
{
	user_t *sptr = find_client(me.servname);

	if (sptr && IsServer(sptr) && strcasecmp(sptr->server->nick, me.servname) == 0 && strcasecmp(sptr->nick, me.servname) != 0)
		return sptr;
	return NULL;
}

server_t *
find_server_t(char *name)
{
	server_t *server = main_server;

	for (; server; server = server->next)
		if (strcasecmp(server->name, name) == 0)
			return server;
	return NULL;
}

#ifdef DEBUGMODE
#include "send.h"
int
tell_server_hash(char *from)
{
	server_t *server = main_server;

	for (; server; server = server->next)
		reply(OS, from, "C:%s:%s:%s:%d", server->host, server->pass, server->name, server->port);

	return reply(OS, from, "stuff");
}
#endif
