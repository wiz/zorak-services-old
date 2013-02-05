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

#include "irc_string.h"
#include "defines.h"
#include "config.h"
#include "clone.h"
#include "privs.h"
#include "send.h"
#include "mem.h"
#include "net.h"
#include "me.h"

WIZOSID("$Id: clone.c,v 1.16 2002/10/11 04:19:19 wiz Exp $");

clone_t *main_clone_t = NULL;

/*
 * If the new user's host matches the host of an existing client's host,
 * add the host to our clone hash if it doesn't already exist.
 * If it does exist, increment its clone count and check it...
 * If the new clone count == CLONE_GLINE then call gline() so they die.
 */

int
check_clone(char *nick, char *host)
{
	extern user_t *main_user_t;
	char nicklist[BUFSIZE];
	clone_t *clone, *allow;
	user_t *user;

	for (user = main_user_t; user; user = user->next) {
		if (IsPerson(user) && strcasecmp(user->host, host) == 0) {
			if ((clone = find_clone_host(user->host))) {
				if ((allow = find_clone_allow(host))) {
					if (++clone->clones <= allow->clones)
						return 0;
					if (clone->clones - allow->clones >= CLONE_OVER) {
							tell_chans("\2[CLONES]\2 %d clones from %s (over clone allow limit of %d) - GLINE +%d", clone->clones, clone->host, allow->clones, CLONE_GLINE_TIME);
							alldcc("CLONES: %d clones from %s (over clone allow limit of %d) - GLINE +%d", clone->clones, clone->host, allow->clones, CLONE_GLINE_TIME);
							gline(NULL, "*", clone->host, CLONE_GLINE_TIME, "clones (over clone allow limit)", 0);
							return 1;
					} else if (clone->clones - allow->clones > 0) {
							tell_chans("\2[CLONES]\2 %d clones from %s (over clone allow limit of %d)", clone->clones, clone->host, allow->clones);
							alldcc("CLONES: %d clones from %s (over clone allow limit of %d)", clone->clones, clone->host, allow->clones);
							clone_allow_warn(user->nick, clone->clones, allow->clones);
							for (user = main_user_t; user; user = user->next)
								if (IsPerson(user) && strcasecmp(user->host, host) == 0)
									clone_allow_warn(user->nick, clone->clones, allow->clones);
							return 0;
					}
				} else {
					if (++clone->clones <= CLONE_LIMIT)
							return 0;
					if (me.lifesux) {
						tell_chans("\2[CLONES]\2 %d clones from %s - \2HTM\2 GLINE +1", clone->clones, clone->host);
						alldcc("CLONES: %d clones from %s - HTM GLINE +1", clone->clones, clone->host);
						return gline(NULL, "*", clone->host, 1, "clones", 0);
					}
					if (clone->clones < CLONE_GLINE) {
						strlcpy(nicklist, nick, BUFSIZE);
						clone_warn(nick, clone->clones);
						for (user = main_user_t; user; user = user->next)
							if (IsPerson(user) && strcasecmp(user->host, host) == 0) {
								clone_warn(user->nick, clone->clones);
								strlcat(nicklist, ", ", BUFSIZE);
								strlcat(nicklist, user->nick, BUFSIZE);
							}
						tell_chans("\2[CLONES]\2 %d clones from %s: %s", clone->clones, clone->host, nicklist);
						alldcc("CLONES: %d clones from %s: %s", clone->clones, clone->host, nicklist);
					} else if (clone->clones < CLONE_GLINE + CLONE_LIMIT) {
						tell_chans("\2[CLONES]\2 %d clones from %s: GLINE *@%s +%d", clone->clones, clone->host, clone->host, CLONE_GLINE_TIME);
						alldcc("CLONES: %d clones from %s: GLINE *@%s +%d", clone->clones, clone->host, clone->host, CLONE_GLINE_TIME);
						return gline(NULL, "*", clone->host, CLONE_GLINE_TIME, "clones", 0);
					} else {
						tell_chans("\2[CLONES]\2 \2%d\2 clones from %s: GLINE *@%s +%d", clone->clones, clone->host, clone->host, FCLONE_GLINE_TIME);
						alldcc("CLONES: %d clones from %s: GLINE *@%s +%d", clone->clones, clone->host, clone->host, FCLONE_GLINE_TIME);
						return gline(NULL, "*", clone->host, FCLONE_GLINE_TIME, "flood clones", 0);
					}
				}
			} else {
				add_clone_host(user->host);
			}
			return 0;
		}
	}
	return 0;
}

void
add_clone_host(char *host)
{
	clone_t *clone;

	clone = leetmalloc(sizeof(clone_t));
	clone->next = main_clone_t;
	main_clone_t = clone;

	clone->host = leetstrdup(host);
	clone->clones = 2;
}

int
del_clone_host(char *host)
{
	clone_t *tmp, **all = &main_clone_t;

	for (; *all; all = &((*all)->next))
		if (strcmp((*all)->host, host) == 0) {
			leetfree((*all)->host, strlen((*all)->host) + 1);
			tmp = *all;
			*all = (*all)->next;
			leetfree(tmp, sizeof(clone_t));
			return 1;
		}
	return 0;
}

int
del_clone_user(user_t *user)
{
	clone_t *clone;

	if (!(clone = find_clone_host(user->host)))
		return 0;

	if (clone->clones-- - 1 < 2)
		return del_clone_host(clone->host);

	return 1;
}

void
del_all_clones(void)
{
	clone_t *tmp, **all = &main_clone_t;

	while (*all) {
		leetfree((*all)->host, strlen((*all)->host) + 1);
		tmp = *all;
		*all = (*all)->next;
		leetfree(tmp, sizeof(clone_t));
	}
	main_clone_t = NULL;
}

clone_t *
find_clone_host(char *host)
{
	clone_t *clone = main_clone_t;

	for (; clone; clone = clone->next)
		if (strcasecmp(clone->host, host) == 0)
			return clone;
	return NULL;
}

int
list_clones(char *to)
{
	extern user_t *main_user_t;
	clone_t *allow, *clone = main_clone_t;
	int x = 0;
	user_t *user;

	for (; clone; clone = clone->next) {
		if (clone->clones <= CLONE_LIMIT)
			continue;
		if ((allow = find_clone_allow(clone->host))) {
			if (clone->clones <= allow->clones)
				continue;
			reply(OS, to, "(%d) %d clones from %s (over clone allow limit of %d)", ++x,
				 clone->clones, clone->host, allow->clones);
			continue;
		}
		reply(OS, to, "(%d) %d clones from %s", ++x, clone->clones, clone->host);
		for (user = main_user_t; user; user = user->next)
			if (IsPerson(user) && strcasecmp(clone->host, user->host) == 0) {
				reply(OS, to, "   %s!%s@%s$%s [%s]", user->nick, user->username, user->host, user->realname, user->server->nick);
			}
	}
	if (x < 1) {
		return reply(OS, to, "no clones found.");
	}
	return 1;
}

int
kill_clones(void)
{  
	clone_t *clone = main_clone_t;
	extern user_t *main_user_t;
	user_t *user, *tmp;
	int i = 0, x = 0;
	char *host;

	while (clone) {
		if (clone->clones <= CLONE_LIMIT || find_clone_allow(clone->host)) {
			clone = clone->next;
			continue;
		}
		host = leetstrdup(clone->host);
		clone = clone->next;
		for (user = main_user_t; user;) {
			if (IsPerson(user) && strcasecmp(user->host, host) == 0 && !IsCoAdmin(user)) {
				toserv(":%s KILL %s :%s!%s (clones - over %d per host limit (%d))\r\n", me.sclients[OS].nick, user->nick, me.servname, me.sclients[OS].nick, CLONE_LIMIT, ++x);
				tmp = user;
				user = user->next;
				del_user(tmp);
				i++;
			}
			else
				user = user->next;
		}
		tell_chans("\2[CLONES]\2 killed %d clones on %s", x, host);
		alldcc("CLONES: killed %d clones from %s", x, host);
		leetfree(host, strlen(host) + 1);
		x = 0;
	}
	return i;
}
