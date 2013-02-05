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
#include "clone.h"
#include "privs.h"
#include "send.h"
#include "mem.h"

WIZOSID("$Id: aclone.c,v 1.11 2002/07/13 20:37:46 wiz Exp $");

clone_t *main_clone_allow = NULL;

void
add_clone_allow(char *host, int clones)
{
	clone_t *allow;

	allow = leetmalloc(sizeof(clone_t));
	allow->next = main_clone_allow;
	main_clone_allow = allow;

	allow->host = leetstrdup(host);

	allow->clones = clones;
}

int
del_clone_allow(char *host)
{
	clone_t *tmp, **all = &main_clone_allow;

	for (; *all; all = &((*all)->next))
		if (strcasecmp((*all)->host, host) == 0) {
			leetfree((*all)->host, strlen((*all)->host) + 1);
			tmp = *all;
			*all = (*all)->next;
			leetfree(tmp, sizeof(clone_t));
			return 1;
		}
	return 0;
}

void
del_all_clone_allows(void)
{
	clone_t *tmp, **all = &main_clone_allow;
		 
	while (*all) {   
		leetfree((*all)->host, strlen((*all)->host) + 1);
		tmp = *all;
		*all = (*all)->next;
		leetfree(tmp, sizeof(clone_t));
	}
	main_clone_allow = NULL;
}
   
clone_t *
find_clone_allow(char *host)
{
	clone_t *clone = main_clone_allow;

	for (; clone; clone = clone->next)
		if (strcasecmp(clone->host, host) == 0)
			return clone;
	return NULL;
}

int
tell_clone_allows(char *to)
{
	clone_t *clone = main_clone_allow;
	extern user_t *main_user_t;
	user_t *user;
	u_short count;

	if (!clone)
		return reply(OS, to, "no clone allow records in my database.");
	for (; clone; clone = clone->next) {
		for (user = main_user_t, count = 0; user; user = user->next)
			if (IsPerson(user) && strcasecmp(user->host, clone->host) == 0)
				count++;
		reply(OS, to, "(%d/%d) %s", count, clone->clones, clone->host);
	}
	return 1;
}
