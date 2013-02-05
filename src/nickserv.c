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
#include "nickserv.h"
#include "nicks.h"
#include "defines.h"
#include "replies.h"
#include "privs.h"
#include "main.h"
#include "send.h"
#include "me.h"

WIZOSID("$Id: nickserv.c,v 1.6 2002/10/02 02:56:20 wiz Exp $");

u_short
ns_register(user_t *cptr, char *args)
{
	if (get_nick(cptr->nick))
		return reply(NS, cptr->nick, "The nick \"%s\" is already registered!", cptr->nick);
	switch (check_nickpass(cptr, args)) {
		case 1:
			return 1;
		case NEED_HELP:
			return NEED_HELP;
	}
	log("%s!%s@%s$%s on %s REGISTER", cptr->nick, cptr->username, cptr->host, cptr->realname, cptr->server->nick);
	new_registered_nick(cptr, args);
	SetAuth(cptr);
	return reply(NS, cptr->nick, "%s is now registered to you.", cptr->nick);
}

u_short
ns_identify(user_t *cptr, char *args)
{
	switch (check_nickpass(NULL, args)) {
		case 1:
			goto bad;
		case NEED_HELP:
			return NEED_HELP;
	}
	switch (try_passwd(cptr->nick, args)) {
		case 2:
			return reply(NS, cptr->nick, "The nick \"%s\" is not currently registered. For help on registering nicks, "
										"/msg %s help register", cptr->nick, me.sclients[NS].nick);
		case 1:
		bad:return reply(NS, cptr->nick, "Incorrect password.");
		case 0:
			log("%s!%s@%s$%s on %s IDENTIFY", cptr->nick, cptr->username, cptr->host, cptr->realname, cptr->server->nick);
			SetAuth(cptr);
			update_last_info(cptr, get_nick(cptr->nick));
			return reply(NS, cptr->nick, replies[RPL_IDENTIFIED]);
	}
	return -1;
}

u_short
ns_drop(user_t *cptr, char *args)
{
	nick_t *nick;

	if (!args) {
		if (!(nick = get_nick(cptr->nick)))
			return reply(NS, cptr->nick, "The nick \"%s\" is not currently registered. For help on registering nicks, "
										"/msg %s help register", cptr->nick, me.sclients[NS].nick);
		if (IsAuth(cptr)) {
			ClearAuth(cptr);
			drop_registered_nick(nick);
			log("%s!%s@%s$%s on %s DROP", cptr->nick, cptr->username, cptr->host, cptr->realname, cptr->server->nick);
			return reply(NS, cptr->nick, "Your nickname has been dropped.");
		} else
			return reply(NS, cptr->nick, "You must identify before using the DROP command.");
	} else {
		if (!IsOper(cptr) || strchr(args, ' ') || !(nick = get_nick(args)))
			return NEED_HELP;
		if (!IsLeet(cptr))
			return reply(NS, cptr->nick, replies[ERR_NOACCESS]);
		if (!(nick = get_nick(args)))
			return reply(NS, cptr->nick, "The nick \"%s\" is not currently registered.", args);
		log("%s!%s@%s$%s on %s OPERDROP %s", cptr->nick, cptr->username, cptr->host, cptr->realname, cptr->server->nick, nick->nick);
		drop_registered_nick(nick);
		return reply(NS, cptr->nick, "Nickname %s has been dropped.", nick->nick);
	}
}

u_short
ns_list(user_t *cptr, char *args)
{
	if (!args || strchr(args, ' '))
		return NEED_HELP;
	if (check_nm(cptr, args))
		return 1;
	log("%s!%s@%s$%s on %s LIST %s", cptr->nick, cptr->username, cptr->host, cptr->realname, cptr->server->nick, args);
	list_nicks(cptr, args);
	return 1;
}

u_short
ns_info(user_t *cptr, char *args)
{
	nick_t *nick;

	if (!args || strchr(args, ' '))
		return NEED_HELP;
	if (!(nick = get_nick(args)))
		return reply(NS, cptr->nick, "The nick \"%s\" is not currently registered. For help on registering nicks, "
									"/msg %s help register", args, me.sclients[NS].nick);
	reply(NS, cptr->nick, "Info for \2%s\2:", nick->nick);
	reply(NS, cptr->nick, "Last seen address: %s", nick->lasthost);
	reply(NS, cptr->nick, "  Time registered: %s", leetctime(nick->regtime));
	reply(NS, cptr->nick, "   Last seen time: %s", leetctime(nick->lasttime));
	reply(NS, cptr->nick, "         Settings: none");
	reply(NS, cptr->nick, "Access List for \2%s\2:", nick->nick);
	return 1;
}
