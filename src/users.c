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

#include <assert.h>
#include "irc_types.h"
#include "irc_string.h"
#include "irc_time.h"
#include "channels.h"
#include "servers.h"
#include "snuggle.h"
#include "defines.h"
#include "client.h"
#include "config.h"
#include "oline.h"
#include "clone.h"
#include "privs.h"
#include "main.h"
#include "send.h"
#include "mem.h"
#include "net.h"
#include "me.h"

WIZOSID("$Id: users.c,v 1.35 2002/10/02 17:39:53 wiz Exp $");

user_t *main_user_t = NULL;

user_t *
add_user_to_list(void)
{
	user_t *user = leetcalloc(sizeof(user_t), 1);

	user->next = main_user_t;
	main_user_t = user;
	if (user->next)
		user->next->prev = user;
	return user;
}

user_t *
add_user(char *nick, char *username, char *host, char *realname, char *server, int privs)
{
	user_t *user, *sptr = find_client(server);

	if (!sptr)
		return NULL;

	if ((user = find_client(nick)))
		del_user(user);

	user = add_user_to_list();
	user->nick = leetstrdup(nick);
	user->username = leetstrdup(username);
	user->host = leetstrdup(host);
	user->realname = leetstrdup(realname);
	user->server = sptr;

	user->ts = time(NULL);
	user->privs = privs;
	me.users++;

	return user;
}

void
jupe(char *name, char *reason)
{
	user_t *sptr = find_client(name);

	if (sptr && IsServer(sptr))
		squit(sptr->nick, "JUPED");
	add_linked_server(me.servname, name, reason);
	switch (me.servtype) {
		case SERV_IRCNN:
			toserv(":%s SERVER %s 2 %lu %lu P09 :%s\r\n", me.servname, name, time(NULL), time(NULL) + 1, reason);
			break;
		case SERV_HYBRD:
			toserv(":%s SERVER %s 1 :%s\r\n", me.servname, name, reason);
			break;
	}
}

void
add_linked_server(char *hub, char *leaf, char *comment)
{
	user_t *server, *uplink;

	if ((server = find_client(leaf)))
		del_server(server);

	if (!(uplink = find_client(hub)) && !is_me(leaf))
		return;

	server = add_user_to_list();
	server->nick = leetstrdup(leaf);
	server->realname = leetstrdup(comment);
	server->server = uplink ? uplink : server;
	server->ts = time(NULL);
	server->privs = SERVER;
	me.servers++;
}

void
del_user_from_list(user_t *user)
{
	if (user->prev) {
		user->prev->next = user->next;
	} else {
		main_user_t = user->next;
		if (user->next)
			user->next->prev = NULL;
	}
	if (user->next)
		user->next->prev = user->prev;
}

void
del_user(user_t *user)
{
	if (IsServer(user)) {
		me.servers--;
		del_user_from_list(user);
	} else {
		if (IsOper(user)) {
			me.opers--;
			if (user->privs & (CADMIN|ADMIN))
				me.admins--;
		}
		me.users--;
		del_clone_user(user);
		del_user_in_channels(user);
		del_user_from_list(user);
		leetfree(user->host, strlen(user->host) + 1);
		leetfree(user->username, strlen(user->username) + 1);
#ifdef	HAVE_LIBCRYPTO
		if (user->response)
			leetfree(user->response, strlen(user->response) + 1);
#endif
	}
	leetfree(user->nick, strlen(user->nick) + 1);
	leetfree(user->realname, strlen(user->realname) + 1);
	leetfree(user, sizeof(user_t));
}

void
del_all_users(void)
{
	user_t *tmp, *user = main_user_t;

	del_all_channels();
	del_all_clones();
	while (user) {
		leetfree(user->nick, strlen(user->nick) + 1);
		if (user->host)
			leetfree(user->host, strlen(user->host) + 1);
		if (user->username)
			leetfree(user->username, strlen(user->username) + 1);
#ifdef	HAVE_LIBCRYPTO
		if (user->response)
			leetfree(user->response, strlen(user->response) + 1);
#endif
		leetfree(user->realname, strlen(user->realname) + 1);
		tmp = user->next;
		leetfree(user, sizeof(user_t));
		user = tmp;
	}
	me.admins = me.opers = me.servers = me.gotping = me.eob = me.conn = 0;
	main_user_t = NULL;
}

u_short
check_privs(char *nick)
{
	user_t *user = main_user_t;

	for (; user; user = user->next)
		if (strcasecmp(nick, user->nick) == 0)
			return user->privs;
	return -1;
}

user_t *
find_client(char *nick)
{
	user_t *user = main_user_t;

	for (; user; user = user->next)
		if (strcasecmp(user->nick, nick) == 0)
			return user;
	return NULL;
}

int
change_user_nick(char *old, char *new)
{
	user_t *user = find_client(old);

	if (!user)
		return -1;
	if (match_snuggle(new, user->username, user->host, user->realname)) {
		del_user(user);
		return 1;
	}
	user->nick = leetrestrdup(user->nick, new);
	if (strcasecmp(old, new) != 0)
		ClearAuth(user);
	return 0;
}

short
add_privs(user_t *cptr, u_short new_privs)
{
	cptr->privs |= new_privs;
	switch (new_privs)
	{
		case ADMIN:
			cptr->privs |= CADMIN;
		case CADMIN:
			cptr->privs |= LEET;
			if (cptr->privs & OPER)
				me.admins++;
			break;
		case OPER:
			if (cptr->privs & (CADMIN|ADMIN))
				me.admins++;
			me.opers++;
			break;
	}
	return cptr->privs;
}

short
del_privs(user_t *cptr, u_short new_privs)
{
	cptr->privs &= ~new_privs;
	switch (new_privs)
	{
		case ADMIN:
			cptr->privs &= ~CADMIN;
		case CADMIN:
			cptr->privs &= ~LEET;
			if (cptr->privs & OPER)
				me.admins--;
			break;
		case OPER:
			if (cptr->privs & (CADMIN|ADMIN))
				me.admins--;
			me.opers--;
			break;
	}
	return cptr->privs;
}

int
check_flood(user_t *cptr)
{
	if (cptr->ts >= time(NULL)) {
		if (cptr->flood++ + 1 < 2)
			return 0;
		if (cptr->flood == 2) {
			cptr->ts = time(NULL) + 10;
			if (!IsOper(cptr))
				serv_notice(cptr->nick, "Flood detected, ignoring you for 10 seconds.");
			else
				tell_chans("\2[FLOOD]\2 %s!%s@%s$%s is flooding services!", cptr->nick, cptr->username, cptr->host, cptr->realname);
		}
		return 1;
	} else {
		cptr->ts = time(NULL);
		cptr->flood = 0;
		return 0;
	}
}

int
whokill(sock_t *sock, char *nm, char *um, char *hm, char *rm, char *reason)
{
	user_t *user = main_user_t;
	user_t *victim[MAXKILL + 1];
	int i, x;

	for (i = x = 0; user; user = user->next) {
		if (IsPerson(user) && match(nm, user->nick) && match(um, user->username) && match(hm, user->host) && match(rm, user->realname) && user->privs == NOPRIV) {
			victim[i] = user;
			if (i++ >= MAXKILL) {
				alldcc("\2%s\2 attempted to \2WHOKILL\2 over %d people! (%s!%s@%s$%s (%s))",
					    sock->name, MAXKILL, nm, um, hm, rm, reason); 
				log("%s was disconnected for attempting to WHOKILL over %d people!", sock->name, MAXKILL);
				return -5;
			}
		}
	}
	if (!i) {
		tosock(sock, "Your WHOKILL request did not match any clients.\n");
		return 0;
	}
	operwall(0, "%s requested: WHOKILL %s!%s@%s$%s (%s)", sock->name, nm, um, hm, rm, reason);
	tosock(sock, "Your WHOKILL request matched %d client%s.\n", i, S(i));
	while (i >= ++x) {
		toserv(":%s KILL %s :%s!%s (%s (%d/%d))\r\n", me.servname, victim[x - 1]->nick, me.servname, me.sclients[OS].nick, reason, x, i);
		del_user(victim[x - 1]);
	}
	return i;
}

int
who(sock_t *sock, char *nm, char *um, char *hm, char *rm)
{
	user_t *user = main_user_t;
	int i = 0;

	for (; user && (i < 200 || sock->flags & ADMIN); user = user->next)
		if (IsPerson(user) && match(nm, user->nick) && match(um, user->username) && match(hm, user->host) && match(rm, user->realname))
			tosock(sock, "(%d) %s!%s@%s$%s [%s]\n", ++i, user->nick, user->username, user->host, user->realname, user->server->nick);

	return i;
}

void
redo_privs(void)
{
	user_t *user = main_user_t;

	for (; user; user = user->next) {
		if (IsServer(user))
			continue;
		del_privs(user, LEET);
		del_privs(user, CADMIN);
		del_privs(user, ADMIN);
	}
	me.admins = 0;
	for (user = main_user_t; user; user = user->next)
		if (IsOSAuth(user))
			match_oline(user);
}

int
count_users(void)
{
	user_t *user = main_user_t;
	int i = 0;

	for (; user; user = user->next)
		if (IsPerson(user))
			i++;
	return i;
}

int
gline(user_t *cptr, char *um, char *hm, int gtime, char *reason, u_short type)
{
	user_t *user = main_user_t;
	user_t *victim[MAXKILL + 1];
	int i = 0;

	if (type == 2)
		goto stuff;
	if (!cptr) {
		if (me.servtype == SERV_IRCNN)
			toserv(":%s KLINE %s@%s %d :%s\r\n", me.servname, um, hm, gtime, reason);
		else
			toserv(":%s KLINE * %d %s %s :%s\r\n", me.sclients[OS].nick, gtime * 3600, um, hm, reason);
		return 1;
	}
	for (i = -1; user; user = user->next) {
		if (IsPerson(user) && match(um, user->username) && match(hm, user->host)) {
			victim[++i] = user;
			if (i >= MAXKILL) { 
				log("%s!%s@%s on %s was KILLED for attempting to GLINE over %d people!", cptr->nick, cptr->username, cptr->host, cptr->server->nick, MAXKILL);
				operwall(OS, "\2%s!%s@%s\2 on \2%s\2 attempted to \2GLINE\2 over %d people! (%s@%s (%s))", cptr->nick, cptr->username, cptr->host, cptr->server->nick, MAXKILL, um, hm, reason);
				irc_kill(OS, cptr->nick, "DO NOT ATTEMPT TO GLINE THAT MANY PEOPLE!@#");
				del_user(cptr);
				return -5;
			}
		}
	}
	for (; i >= 0; i--) {
		if (IsAdmin(victim[i])) {
			log("%s!%s@%s on %s was KILLED for attempting to GLINE %s!%s@%s on %s!", cptr->nick, cptr->username, cptr->host, cptr->server->nick, victim[i]->nick, victim[i]->username, victim[i]->host, victim[i]->server);
			operwall(OS, "\2%s!%s@%s\2 on \2%s\2 attempted to \2GLINE\2 %s! (%s@%s (%s))", cptr->nick, cptr->username, cptr->host, cptr->server->nick, victim[i]->nick, um, hm, reason);
			irc_kill(OS, cptr->nick, "DO NOT ATTEMPT TO GLINE ADMINS!@#");
			del_user(cptr);
			return -5;
		}
	}
	if (gtime <= 72)
		operwall(OS, "%s!%s@%s on %s added %sGLINE for %s@%s for %d hour%s (%s)", cptr->nick, cptr->username,
			cptr->host, cptr->server->nick, (type ? "ANON" : ""), um, hm, gtime, S(gtime), reason);
	if (me.servtype == SERV_IRCNN)
		toserv(":%s KLINE %s@%s %d :You have been banned from the network%s%s - %s\r\n", me.servname, um,
			hm, gtime, (type ? "" : " by "), (type ? "" : cptr->nick), reason);
	else
		toserv(":%s KLINE * %d %s %s :You have been banned from the network%s%s: %s\r\n", me.sclients[OS].nick,
			gtime * 3600, um, hm, (type ? "" : " by "), (type ? "" : cptr->nick), reason);
	return i;
	stuff:
	if (me.servtype == SERV_IRCNN)
		toserv(":%s KLINE %s@%s %d :AGLINE ID %s -- %s\r\n", me.servname, um, hm, gtime, me.md5buf, reason);
	else
		toserv(":%s KLINE * %d %s %s :AGLINE ID %s (%s)\r\n", me.sclients[OS].nick, gtime * 3600, um, hm, me.md5buf, reason);
	return 0;
}

int
del_server(user_t *server)
{
	user_t *tmp, *user = main_user_t;

	assert(0 != server);
	assert(!is_me(server->nick));

	if (!is_me(server->server->nick))
		while ((tmp = user)) {
			user = user->next;
			if (tmp->server != server)
				continue;
			if (IsServer(tmp))
				del_server(tmp);
			else
				del_user(tmp);
		}
	del_user(server);
	return 1;
}

#ifdef DEBUGMODE
int
tell_user_hash(char *from)
{
	user_t *user = main_user_t;

	for (; user; user = user->next)
		if (IsPerson(user))
			reply(OS, from, "nick = %s, username = %s, host = %s, realname = %s, server = %s, privs = %d", user->nick, user->username, user->host, user->realname, user->server->nick, user->privs);
	return 1;
}
#endif
