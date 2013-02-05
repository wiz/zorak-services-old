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
#include "nicks.h"
#include "nicks.h"
#include "send.h"
#include "mem.h"
#include "md5.h"

WIZOSID("$Id: nicks.c,v 1.4 2002/07/13 20:37:46 wiz Exp $");

nick_t *main_nick = NULL;

nick_t *
add_nick_to_list(void)
{
	nick_t *nick = leetmalloc(sizeof(nick_t));
	nick->prev = NULL;
	nick->next = main_nick;
	main_nick = nick;
	if (nick->next)
		nick->next->prev = nick;
	return nick;
}

void
new_registered_nick(user_t *cptr, char *passwd)
{
	nick_t *nick = add_nick_to_list();

	nick->nick = leetstrdup(cptr->nick);
	nick->passwd = leetstrdup(wiz_md5(passwd));
	nick->regtime = nick->lasttime = time(NULL);
	nick->info = leetstrdup(cptr->realname);
	replace_colon(nick->info);
	nick->lasthost = leetmalloc(strlen(cptr->username) + strlen(cptr->host) + 2);
	sprintf(nick->lasthost, "%s@%s", cptr->username, cptr->host);
	nick->flags = 0;
}

void
update_last_info(user_t *cptr, nick_t *nick)
{
	nick->info = leetrestrdup(nick->info, cptr->realname);
	replace_colon(nick->info);
	nick->lasthost = leetrealloc(nick->lasthost, strlen(cptr->username) + strlen(cptr->host) + 2);
	sprintf(nick->lasthost, "%s@%s", cptr->username, cptr->host);
	nick->lasttime = time(NULL);
}

void
add_registered_nick(char *name, char *passwd, time_t regtime, time_t lasttime, char *info, char *host, u_short flags)
{
	nick_t *nick = add_nick_to_list();

	nick->nick = leetstrdup(name);
	nick->passwd = leetstrdup(passwd);
	nick->info = leetstrdup(info);
	nick->lasthost = leetstrdup(host);
	nick->regtime = regtime;
	nick->lasttime = lasttime;
	nick->flags = flags;
}

void
drop_registered_nick(nick_t *nick)
{
	if (nick->prev) {
		nick->prev->next = nick->next;
	} else {
		main_nick = nick->next;
		if (nick->next)
			nick->next->prev = NULL;
	}
	if (nick->next)
		nick->next->prev = nick->prev;
	leetfree(nick->nick, strlen(nick->nick) + 1);
	leetfree(nick->passwd, strlen(nick->passwd) + 1);
	leetfree(nick->info, strlen(nick->info) + 1);
	leetfree(nick->lasthost, strlen(nick->lasthost) + 1);
	leetfree(nick, sizeof(nick_t));
}

void
del_all_nicks(void)
{
	nick_t *tmp, *nick = main_nick;
	while (nick) {
		leetfree(nick->nick, strlen(nick->nick) + 1);
		leetfree(nick->passwd, strlen(nick->passwd) + 1);
		leetfree(nick->info, strlen(nick->info) + 1);
		leetfree(nick->lasthost, strlen(nick->lasthost) + 1);
		tmp = nick->next;
		leetfree(nick, sizeof(nick_t));
		nick = tmp;
	}
	main_nick = NULL;
}

nick_t *
get_nick(char *name)
{
	nick_t *nick = main_nick;
	
	for (; nick; nick = nick->next)
		if (strcasecmp(nick->nick, name) == 0)
			return nick;
	return NULL;
}

u_short
try_passwd(char *name, char *passwd)
{
	nick_t *nick = get_nick(name);

	if (!nick)
		return 2;
	if (strcmp(nick->passwd, wiz_md5(passwd)) != 0)
		return 1;
	return 0;
}

void
change_nick_passwd(nick_t *nick, char *newpass)
{
	nick->passwd = leetrestrdup(nick->passwd, wiz_md5(newpass));
}

void
list_nicks(user_t *cptr, char *pattern)
{
	nick_t *nick = main_nick;
	u_short i = 0;

	if (!nick) {
		reply(NS, cptr->nick, "No nicks are registered!");
		return;
	}
	for (; nick; nick = nick->next)
		if (match(pattern, nick->nick) || match(pattern, nick->lasthost) || match(pattern, nick->info))
			reply(NS, cptr->nick, "%d) %s %s %s", ++i, nick->nick, nick->lasthost, nick->info);

	reply(NS, cptr->nick, "End of LIST - %d matches found.", i);
}
