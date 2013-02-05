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
#include "defines.h"
#include "config.h"
#include "format.h"
#include "privs.h"
#include "send.h"
#include "net.h"
#include "me.h"

WIZOSID("$Id: send.c,v 1.22 2002/10/02 02:56:20 wiz Exp $");

int
sendto_opers(char *sendbuf, ...)
{
	char notice[BUFSIZE];
	extern user_t *main_user_t;
	user_t *user;
	va_list ap;

	va_start(ap, sendbuf);
	leet_vsprintf(notice, sendbuf, ap);

	for (user = main_user_t; user; user = user->next)
		if (IsOper(user))
			toserv(":%s NOTICE %s :*** Notice -- %s\r\n", me.servname, user->nick, notice);

	va_end(ap);
	return 1;
}

int
sendto_admins(char *sendbuf, ...)
{
	char notice[BUFSIZE];
	extern user_t *main_user_t;
	user_t *user;
	va_list ap;

	va_start(ap, sendbuf);
	leet_vsprintf(notice, sendbuf, ap);

	for (user = main_user_t; user; user = user->next)
		if (IsAdmin(user))
			toserv(":%s NOTICE %s :*** Notice -- %s\r\n", me.servname, user->nick, notice);

	va_end(ap);
	return 1;
}

int
is_an_oper_notice(char *server, char *sendbuf, ...)
{
	char notice[BUFSIZE];
	extern user_t *main_user_t;
	user_t *user;
	va_list ap;

	va_start(ap, sendbuf);
	leet_vsprintf(notice, sendbuf, ap);
	for (user = main_user_t; user; user = user->next)
		if (IsServNotice(user) && strcmp(user->server->nick, server) != 0)
			toserv(":%s NOTICE %s :*** Notice -- %s\r\n", me.servname, user->nick, notice);

	va_end(ap);
	return 1;
}

time_t
settime(void)
{
	extern user_t *main_user_t;
	time_t now = time(NULL);
	user_t *user;

	for (user = main_user_t; user; user = user->next)
		if (IsServer(user))
			toserv(":%s SETTIME %lu :%s\r\n", me.sclients[OS].nick, now, user->nick);

	return now;
}

int
list_admins(user_t *cptr, char *to)
{
	extern user_t *main_user_t;
	user_t *user;

	for (user = main_user_t; user; user = user->next)
		if (IsAdmin(user))
			reply(OS, to, "[A] %s (%s@%s) [%s]", user->nick, user->username, user->host, user->server->nick);
	for (user = main_user_t; user; user = user->next)
		if (IsCoAdmin(user) && !IsAdmin(user))
			reply(OS, to, "[C] %s (%s@%s) [%s]", user->nick, user->username, user->host, user->server->nick);
	for (user = main_user_t; user; user = user->next)
		if (IsLeet(user) && !IsCoAdmin(user))
			reply(OS, to, "[O] %s (%s@%s) [%s]", user->nick, user->username, user->host, user->server->nick);
	return 1;
}

int
tell_chans(char *msg, ...)
{
	char msgbuf[BUFSIZE];
	va_list ap;
	int i;

	va_start(ap, msg);
	leet_vsprintf(msgbuf, msg, ap);

	for (i = 0; me.chans[i]; i++)
		toserv(":%s PRIVMSG %s :%s\r\n", me.sclients[OS].nick, me.chans[i], msgbuf);

	va_end(ap);
	return 1;
}

int
reply(u_short snick, char *to, char *notice, ...)
{
	char msgbuf[BUFSIZE];
	va_list ap;

	if (snick < 1 || snick > ALL - 1)
		return 0;
	va_start(ap, notice);
	leet_vsprintf(msgbuf, notice, ap);
	if (to[0] == '=')
		todcc(to, msgbuf);
	else if (to[0] == '#')
		toserv(":%s PRIVMSG %s :%s\r\n", me.sclients[snick].nick, to, msgbuf);
	else
		toserv(":%s NOTICE %s :%s\r\n", me.sclients[snick].nick, to, msgbuf);

	va_end(ap);
	return 1;
}

int
serv_notice(char *to, char *notice, ...)
{
	char msgbuf[BUFSIZE];
	va_list ap;

	va_start(ap, notice);
	leet_vsprintf(msgbuf, notice, ap);
	toserv(":%s NOTICE %s :%s\r\n", me.servname, to, msgbuf);

	va_end(ap);
	return 1;
}

void
clone_warn(char *to, u_int clones)
{
	serv_notice(to, "An excessive number of clones (%d) has been detected from your host. "
		        "If you do not limit your connections to 3, your host will be banned from the network. "
		        "If you would like to apply for an exemption to the clone rule, please visit %s - "
			"This may be your only warning before you are forcefully removed from the network.",
		    clones, CLONE_URL);
}

void
clone_allow_warn(char *to, u_int clones, u_int allowed)
{
	serv_notice(to, "An excessive number of clones (%d) has been detected from your host. "
			"This is over your current allowed number of clones (%d). "
			"Please contact an active operator to request that your limit be raised, "
			"or limit your connections to the amount you are allowed. "
			"If you ignore this warning, your host may be banned from the network.",
		clones, allowed);
}

int
operwall(u_short snick, char *operwall, ...)
{
	char wallbuf[BUFSIZE];
	va_list ap;

	va_start(ap, operwall);
	leet_vsprintf(wallbuf, operwall, ap);
	switch (me.servtype) {
		case SERV_IRCNN:
			toserv(":%s OPERWALL :%s\r\n", (snick == 0 ? me.sclients[OS].nick : me.sclients[snick].nick), wallbuf);
			break;
		case SERV_HYBRD:
			if (snick == 0)
				toserv(":%s OPERWALL :%s\r\n", me.servname, wallbuf);
			else
				toserv(":%s OPERWALL :%s: %s\r\n", me.servname, me.sclients[snick].nick, wallbuf);
			break;
	}
	va_end(ap);
	return 1;
}

void
servmode(char *chan, char *modes)
{
#if 1
/*
 * with hyb7/ircnn we can just do this
 * with hyb6 we have to get the ts, sjoin, mode, part, blah
 */
	toserv(":%s MODE %s %s\r\n", me.servname, chan, modes);
#else
	channel *channel;
	u_short i;

	switch (me.servtype) {
		case SERV_IRCNN:
			toserv(":%s MODE %s %s\r\n", me.servname, chan, modes);
			break;
		case SERV_HYBRD:
			channel = find_channel(chan);
			for (i = 0; me.chans[i]; i++)
				if (strcasecmp(me.chans[i], chan) == 0)
					goto there;
			if (!channel)
				return;
			/* :h6.wiz.cx SJOIN 1021703401 #stuff +tn :@wiz6 */
			toserv(":%s SJOIN %lu %s %s :@%s\r\n", me.servname, channel->ts, channel->name,
				get_channel_modes(channel), me.sclients[OS].nick);
		there:
			assert(0 != channel);
			toserv(":%s MODE %s %s\r\n", me.sclients[OS].nick, channel->name, modes);
			if (!me.chans[i])
				toserv(":%s PART %s\r\n", me.sclients[OS].nick, channel->name);
			break;
	}
#endif
}

void
hackops(char *chan, char *nick)
{
	toserv(":%s MODE %s +o %s\r\n", me.servname, chan, nick);
}

void
irc_kill(u_short snick, char *who, char *reason)
{
	toserv(":%s KILL %s :%s!%s (%s)\r\n", me.sclients[snick].nick, who, me.servname, me.sclients[snick].nick, reason);
}

void
squit(char *server, char *reason)
{
	switch (me.servtype) {
		case SERV_IRCNN:
			toserv(":%s SQUIT %s %lu :%s\r\n", me.servname, server, time(NULL), reason);
			break;
		case SERV_HYBRD:
			toserv(":%s SQUIT %s :%s\r\n", me.servname, server, reason);
			break;
	}
}
