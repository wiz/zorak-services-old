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

#include <unistd.h> /* alarm() */
#include <errno.h>

#include "irc_types.h"
#include "irc_string.h"
#include "irc_time.h"
#include "channels.h"
#include "defines.h"
#include "replies.h"
#include "servers.h"
#include "snuggle.h"
#include "config.h"
#include "oline.h"
#include "clone.h"
#include "privs.h"
#include "send.h"
#include "main.h"
#include "msg.h"
#include "net.h"
#include "me.h"

WIZOSID("$Id: parse.c,v 1.58 2002/10/02 17:39:53 wiz Exp $");

void
parse(char *toparse)
{
	struct command *mptr = cmdtab;
	static char *parv[16];
	char *tmp, *s;
	user_t *cptr = NULL;
	int i = 0;
#ifdef DEBUGMODE
	if (me.debug) {
		printf("<<< %s\r\n", toparse);
		fflush(stdout);
	}
#endif
	parv[0] = NULL;

	if (*toparse == ':')
	{
		s = &toparse[1];
		if(!(*s))
			return;

		parv[0] = s;
		 
		if(!(s = strchr((char *)&toparse[1], ' ')))
			return;

		*s++ = '\0';
		 
	}
	else
		s = &toparse[0];

	if(!(tmp = (char *)index(s, ' ')))
		return;

	*tmp++ = '\0';

	for (; mptr->cmd; mptr++)
		if (strcmp(mptr->cmd, s) == 0)
			goto next;
	return;
next:
	s = tmp;
	i = 0;

	for (;;)
	{
		while (*s == ' ')
			*s++ = '\0';

		if (*s == '\0')
			break;

		if (*s == ':')
		{
			parv[++i] = s + 1;
			break;
		}

		parv[++i] = s;

		if (i >= 15)
			break;

		for (; *s != ' ' && *s; s++);
	}

	if (parv[0] && !(cptr = find_client(parv[0])))
		return;

	(*mptr->func)(cptr, i + 1, parv);
}

int
m_eob(user_t *cptr, int parc, char **parv)
{
	toserv(":%s EOB\r\n", me.servname);
	tell_chans("\2[EOB]\2 End of burst (took %d seconds)", time(NULL) - me.htmtime);
	me.eob = 1;
	return 1;
}

int
m_ping(user_t *cptr, int parc, char **parv)
{
	if (cptr)
		toserv(":%s PONG %s :%s\r\n", me.servname, me.servname, cptr->nick);
	else
		toserv("PONG :%s\r\n", parv[1]);
/* XXX this is all shitty code */
	if (me.gotping == 0)
		me.gotping = 1;
	if (!me.eob && me.servtype == SERV_HYBRD)
		m_eob(NULL, 0, NULL);
	if (me.settime < time(NULL)) {
		tell_chans("syncronizing servers' clocks...");
		settime();
		me.settime = time(NULL) + (60*60*4);
	}
	if (me.lifesux && me.htmtime && me.htmtime <= time(NULL)) {
		tell_chans("\2[HTM]\2 resuming normal operation....");
		me.lifesux = 0;
		me.htmtime = time(NULL);
	}
	return 1;
}

int
m_pong(user_t *cptr, int parc, char **parv)
{
	return 1;
}

int
m_notice(user_t *cptr, int parc, char **parv)
{
	/* XXX - very lame cheap hack because ircnn doesn't have EOB commands */
	if (!parv[0] || !parv[1] || strcasecmp(parv[0], "NickServ") != 0 || strcasecmp(parv[1], me.sclients[OS].nick) != 0)
		return 1;
	if (!me.eob) {
		tell_chans("\2[EOB]\2 End of burst received (took %d seconds)", time(NULL) - me.htmtime);
		me.eob = 1;
	}
	return 1;
}

int
m_privmsg(user_t *cptr, int parc, char **parv)
{
	struct message *mptr;
	struct chancmd *ccptr;
	char *cmd, *args;
	u_short snick, to_chan = 0;

	if (!cptr || parc < 3 || !(cmd = strtok(parv[2], " ")))
		return -1;
	args = strtok(NULL, "");

	if ((snick = is_services_client(strtok(parv[1], "@"))) > 0) {
		me.to = parv[1];
		goto cmdcheck;
	} else if (is_services_chan(parv[1]) && IsOper(cptr)) {
		to_chan = 1;
		goto cmdcheck;
	}
	return 0;
cmdcheck:
	if (snick == OS && !IsOper(cptr))
		return reply(snick, cptr->nick, replies[ERR_NOACCESS]);
	if (to_chan == 1)
		for (ccptr = chantab; ccptr->msg; ccptr++)
			if (strcasecmp(ccptr->msg, cmd) == 0) {
				(*ccptr->func)(cptr, parv[1], args);
				return 1;
			}
	for (mptr = msgtab; mptr->msg; mptr++)
		if ((mptr->snick == ALL || mptr->snick == snick) && strcasecmp(mptr->msg, cmd) == 0) {
			if ((mptr->privs == NOPRIV || (cptr->privs & mptr->privs) == mptr->privs))
				goto floodcheck;
			else
				return (to_chan ? 1 : reply(snick, cptr->nick, replies[ERR_NOACCESS]));
		}
	if (to_chan || check_flood(cptr))
		return 0;
	return ((!snick || snick == GN) ?1: reply(snick, cptr->nick, replies[ERR_NOSUCHCMD], me.sclients[snick].nick));
floodcheck:
	if (check_flood(cptr))
		return 0;
	if ((*mptr->func) && (*mptr->func)(cptr, args) == NEED_HELP) {
			if (*mptr->help)
				(*mptr->help)(cptr, 0); 
			else
				reply(snick, cptr->nick, replies[ERR_SYNTAX]);
	}
	return 1;
}

int
m_version(user_t *cptr, int parc, char **parv)
{
	if (cptr)
		toserv(":%s 351 %s zorak-services(%s) :%s TS3\r\n", me.servname, cptr->nick, SERIALNUM, me.servname);
	return 1;
}

u_short
pm_help(user_t *cptr, char *args)
{
	struct message *mptr;
	char *cmd = strtok(args, " ");
	u_short snick;

	snick = is_services_client(me.to);
	if (!args)
		return help(cptr, snick, 1);
	for (mptr = msgtab; mptr->msg; mptr++)
		if (	(mptr->snick == ALL || mptr->snick == snick) &&
			(mptr->privs == NOPRIV || (cptr->privs & mptr->privs) == mptr->privs) &&
			(strcasecmp(mptr->msg, cmd) == 0)
		) {
			if (*mptr->help)
				(*mptr->help)(cptr, 1);
			else
				reply(snick, cptr->nick, replies[ERR_NOHELPCMD]);
			return 1;
		}
	return reply(snick, cptr->nick, replies[ERR_NOSUCHCMD], me.sclients[snick].nick);
}

int
m_mode(user_t *cptr, int parc, char **parv)
{
	/* ircnn :Flat-Line MODE Flat-Line :+giow */
	/* hybrid does it through the NICK command and MODE command */

	if (!cptr || parv[1][0] == '#' || parv[1][0] == '&' || !parv[2])
		return -1;
	parse_mode(cptr, parv[2]);
	return 1;
}

int
m_nick(user_t *cptr, int parc, char **parv)
{
	/* :irc.lslandgirls.org NICK Flat-Line 1 1007155636 flatline islandgirls.org irc.islandgirls.org :blah blah */
	/* :wiz NICK blah :1007165553 */
	/* NICK wiz6 1 1021703400 +iw jason rr.wiz.cx h6.wiz.cx :monkey mushroom */

	if (parc < 7)
		return change_user_nick(parv[0], parv[1]);

	switch (me.servtype) {
		case SERV_IRCNN:
			if (strncasecmp(parv[6], "services", 8) == 0) {
				add_user(parv[1], parv[4], parv[5], parv[7], parv[6], NOKILL);
				return 1;
			}
			if (match_snuggle(parv[1], parv[4], parv[5], parv[7]))
				return 1;

			check_clone(parv[1], parv[5]);
			if (!(cptr = add_user(parv[1], parv[4], parv[5], parv[7], parv[6], NOPRIV)))
				break;
			break;
		case SERV_HYBRD:
			if (match_snuggle(parv[1], parv[5], parv[6], parv[8]))
				return 1;

			check_clone(parv[1], parv[6]);
			if (!(cptr = add_user(parv[1], parv[5], parv[6], parv[8], parv[7], NOPRIV)))
				break;
			parse_mode(cptr, parv[4]);
			break;
	}
	return 1;
}

u_short
parse_mode(user_t *cptr, char *mode)
{
	u_short y = 0;

	if (!mode)
		return -1;
	while (*mode != '\0')
		switch (*mode++) {
			case '-':
				y = DEL;
				break;
			case '+':
				y = ADD;
				break;
			case 'o':
				switch (y) {
					case ADD:
						if (me.opernotice && me.eob && strncasecmp(cptr->server->nick,
							"services", 8) != 0 && !IsAdmin(cptr))
							is_an_oper_notice(cptr->server->nick, "%s!%s@%s on %s is now an operator (O)", cptr->nick, cptr->username, cptr->host, cptr->server->nick);
						add_privs(cptr, OPER);
						add_privs(cptr, SNOTICE);
						break;
					case DEL:
						del_privs(cptr, OPER);
						del_privs(cptr, SNOTICE);
						break;
				}
				break;
		}
	return 1;
}

int
m_join(user_t *cptr, int parc, char **parv)
{
	channel *chan;
	char *ptr;

	if (!cptr || IsServer(cptr) || !parv[1] || !(ptr = strtok(parv[1], ",")))
		return 0;
	goto loopy;
	while ((ptr = strtok(NULL, ","))) {
loopy:		if (ptr[0] != '#')
			continue;
		if (!(chan = find_channel(ptr)))
			chan = add_channel(ptr, 0, 0);
		add_user_to_channel(cptr, chan, 0);
	}
	return 1;
}

int
m_sjoin(user_t *cptr, int parc, char **parv)
{	/* :h6.wiz.cx SJOIN 1021703401 #stuff +tn :@wiz6 */
	channel *channel;
	time_t ts;
	char *tmp;
	u_short flag;

	if (parc < 4 || (ts = strtol(parv[1], (char **)NULL, 10)) < 0)
		return 0;
	if (!(channel = find_channel(parv[2])))
		channel = add_channel(parv[2], ts, 0);
	if (!(tmp = strtok(parv[4], " ")))
		return 0;
	goto nloop;
	while((tmp = strtok(NULL, " "))) {
nloop:		flag = 0;
		switch (*tmp) {
			case '@':
				flag = OP;
				break;
			case '+':
				flag = VOICE;
				break;
			case '%':
				flag = HALFOP;
				break;
		}
		if (flag != 0)
			tmp++;
		add_user_to_channel(find_client(tmp), channel, flag);
	}
	return 1;
}

int
m_part(user_t *cptr, int parc, char **parv)
{
	/* :wiz PART #stuff */
	channel *chan;

	if (!cptr || parc < 2 || !(chan = find_channel(parv[1])))
		return 0;
	return del_user_from_channel(cptr, chan);
}

int
m_quit(user_t *cptr, int parc, char **parv)
{
	user_t *user = find_client(parv[0]);

	if (user)
		del_user(user);
	return 1;
}

int
m_squit(user_t *cptr, int parc, char **parv)
{
	user_t *sptr = find_client(strnull(parv[1]));
	if (sptr)
	switch (me.servtype) {
		case SERV_IRCNN:
			return del_server(sptr);
		case SERV_HYBRD:
			return del_server(sptr);
	}
	return 1;
}

int
m_server(user_t *cptr, int parc, char **parv)
{
	if (me.servers == 1 && me.servtype == SERV_HYBRD) {
		intro_nicks(ALL);
		join_channels(OS);
	}
	if (parc > 3)
		add_linked_server((parv[0] ? parv[0] : me.servname), parv[1], strnull(parv[3]));
	return 1;
}

int
m_kill(user_t *cptr, int parc, char **parv)
{
	user_t *user = find_client(parv[1]);

	if (user)
		del_user(user);
	if (is_services_client(parv[1]) > 0) {
		me.opers--;
		intro_nicks(is_services_client(parv[1]));
	}
	return 1;
}

int
m_kick(user_t *cptr, int parc, char **parv)
{
	channel *chan;
	user_t *kptr;
	u_short snick;

	if (parc < 3 || !(chan = find_channel(parv[1])) || !(kptr = find_client(parv[2])))
		return 0;
	if ((snick = is_services_client(parv[2])) > 0) {
		switch (me.servtype) {
			case SERV_IRCNN:
				toserv(":%s JOIN %s\r\n", me.sclients[snick].nick, chan->name);
				hackops(chan->name, me.sclients[snick].nick);
				break;
			case SERV_HYBRD:
				toserv(":%s SJOIN %lu %s + :@%s\r\n", me.servname, chan->ts, chan->name, me.sclients[snick].nick);
				break;
		}
		return 1;
	}
	return del_user_from_channel(kptr, chan);
}

int
m_whois(user_t *cptr, int parc, char **parv)
{
	user_t *who;

	if (!(who = find_client(parv[2]))) {
		toserv(":%s 401 %s %s :No such nick/channel\r\n", me.servname, parv[0], parv[2]);
		return 0;
	}
	toserv(":%s 311 %s %s %s %s * :%s\r\n", me.servname, parv[0], who->nick, who->username, who->host, who->realname);
	if (cptr && IsOper(cptr))
		toserv(":%s 312 %s %s %s :%s\r\n", me.servname, parv[0], who->nick, who->server->nick, who->server->realname);
	else
		toserv(":%s 312 %s %s %s :%s\r\n", me.servname, parv[0], who->nick, me.servname, me.servdesc);
	if (IsOper(who))
		toserv(":%s 313 %s %s :is being synflooded (IRC Operator)\r\n", me.servname, parv[0], who->nick);
	else if (is_services_client(who->nick))
		toserv(":%s 313 %s %s :is an official services client\r\n", me.servname, parv[0], who->nick);
	if (is_me(who->server->nick))
		toserv(":%s 317 %s %s %lu %lu :seconds idle, signon time\r\n", me.servname, parv[0], who->nick, time(NULL) - who->ts, who->ts);
	toserv(":%s 318 %s %s :End of /WHOIS list.\r\n", me.servname, parv[0], who->nick);
	return 1;
}

int
m_admin(user_t *cptr, int parc, char **parv)
{
	if (!cptr)
		return 0;

	sendto_admins("admin requested by %s (%s@%s) [%s]", cptr->nick, cptr->username, cptr->host, cptr->server->nick);

	toserv(":%s 256 %s :Administrative contact for %s:\r\n", me.servname, parv[0], me.servname);
	toserv(":%s 257 %s :%s\r\n", me.servname, parv[0], VER_STR);
	toserv(":%s 258 %s :written and maintained by:\r\n", me.servname, parv[0]);
	toserv(":%s 259 %s :%s\r\n", me.servname, parv[0], MYEMAIL);

	return 1;
}

int
m_stats(user_t *cptr, int parc, char **parv)
{
	if (!cptr || parc < 3)
		return 0;

	if (strcasecmp(parv[2], me.servname) != 0)
		return 0;

	switch (parv[1][0])
	{
		case 'O':
		case 'o':
			stats_o(cptr->nick);
			break;
		default:
			return 1;
	}
	sendto_admins("STATS %c requested by %s (%s@%s) [%s]", parv[1][0], cptr->nick, cptr->username, cptr->host, cptr->server->nick);
	return 1;
}

int
m_motd(user_t *cptr, int parc, char **parv)
{
	u_short i = 0;
	sendto_admins("motd requested by %s (%s@%s) [%s]", cptr->nick, cptr->username, cptr->host, cptr->server->nick);

	toserv(":%s 375 %s :- %s is running %s by %s\r\n", me.servname, parv[0], me.servname, VER_STR, MYEMAIL);
	toserv(":%s 372 %s :-\r\n", me.servname, cptr->nick);
	toserv(":%s 372 %s :- The following services are ready to take your order:\r\n", me.servname, cptr->nick);
	while (++i < ALL)
		if (i != OS && IsOper(cptr))
			toserv(":%s 372 %s :- /msg %s help\r\n", me.servname, cptr->nick, me.sclients[i].nick);
	toserv(":%s 372 %s :-\r\n", me.servname, cptr->nick);
	toserv(":%s 376 %s :End of MOTD command\r\n", me.servname, parv[0]);
	return 1;
}
#if 0 /* to be used later */
		u_short i = 0;
		while (i++ < 400)
			toserv(":%s %d %s :hi!\r\n", me.servname, i, parv[0]);
#endif

int
m_error(user_t *cptr, int parc, char **parv)
{
	char buf[BUFSIZE];
	u_short i = 0;

	strcpy(buf, "");
	while (++i < parc)
		strlcat(buf, parv[i], BUFSIZE);
	log("ERROR: %s", buf);
	alldcc("Received ERROR from %s: %s", me.hub->name, buf);
	del_sock(NULL);
	return 1;
}

int
m_trace(user_t *cptr, int parc, char **parv)
{
	user_t *user;

	if (!cptr || !parv[1])
		return 0;
	if ((user = find_client(parv[1])) && IsPerson(user)) {
		toserv(":%s %d %s %d 0 :%s[%s@%s]\r\n", me.servname, (IsOper(user) ? 204 : 205), cptr->nick,
				(IsOper(user) ? "Oper" : "User"), user->nick, user->username, user->host);
		goto end;
	}
	if ((user = is_juped(parv[1])) || is_me(parv[1]))
		toserv(":%s 206 %s Serv 10 %dS %dC %s :AutoConn.!*@%s\r\n", me.servname, cptr->nick, me.servers,
				me.users, me.hub->name, me.servname);
	if (user)
		toserv(":%s 206 %s Serv 10 %dS %dC %s :JUPED!*@%s\r\n", user->nick, cptr->nick, me.servers,
				me.users, me.servname, user->nick);
end:
	toserv(":%s 262 %s %s :End of TRACE\r\n", ((user && IsServer(user)) ? user->nick : me.servname), cptr->nick,
			((user && IsServer(user)) ? user->nick : me.servname));
	return 1;
}
