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
#include "irc_time.h"
#include "channels.h"
#include "sclients.h"
#include "defines.h"
#include "privs.h"
#include "mem.h"
#include "net.h"
#include "me.h"

WIZOSID("$Id: sclients.c,v 1.2 2002/10/02 17:39:53 wiz Exp $");

void
intro_nicks(u_short nicknum)
{
	short i = nicknum;

	if (i == ALL)
		i--;

	switch (me.servtype) {
	case SERV_IRCNN:
	for (; i >= 0; i--)
	{
		toserv(":%s NICK %s 1 %lu %s %s %s :%s\r\n", me.servname, me.sclients[i].nick, time(NULL),
			me.sclients[i].username, me.servname, me.servname, me.sclients[i].realname);
		toserv(":%s MODE %s :+ko\r\n", me.sclients[i].nick, me.sclients[i].nick);
		me.opers++;
		add_user(me.sclients[i].nick, me.sclients[i].username, me.servname, me.sclients[i].realname,
			 me.servname, NOKILL);

		/* XXX - lame services will kill operserv2 if not identified */
		if (i == OS)
			toserv(":%s PRIVMSG NickServ :identify abcd123\r\n", me.sclients[i].nick);
		if (nicknum != ALL)
			break;
	}
	break;

	case SERV_HYBRD:
	for (; i >= 0; i--)
	{
		/* NICK wiz6 1 1021703400 +iw jason rr.wiz.cx h6.wiz.cx :monkey mushroom */
		toserv(":%s NICK %s 1 %lu +omw %s %s %s :%s\r\n", me.servname, me.sclients[i].nick, time(NULL),
			me.sclients[i].username, me.servname, me.servname, me.sclients[i].realname);
		add_user(me.sclients[i].nick, me.sclients[i].username, me.servname, me.sclients[i].realname,
			 me.servname, NOKILL);
		me.htmtime = time(NULL);
		if (me.eob == 1)
			join_channels(i);

		if (nicknum != ALL)
			break;
	}
	break;
	}
}

u_short
is_services_client(char *nick)
{
	u_short i = ALL - 1;

	for (; i > 0; i--)
		if (strcasecmp(nick, me.sclients[i].nick) == 0)
			return i;
	return 0;
}
