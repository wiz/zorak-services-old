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
#include "replies.h"
#include "snuggle.h"
#include "oline.h"
#include "privs.h"
#include "send.h"
#include "mem.h"
#include "md5.h"
#include "me.h"

WIZOSID("$Id: snuggle.c,v 1.30 2002/07/13 20:37:46 wiz Exp $");

snuggle_t *main_snuggle = NULL;

snuggle_type snuggle_types[] = {
	{ "COUNT",	SNUGGLE_CNT	},
	{ "ALL",	SNUGGLE_ALL	},
	{ "MASK",	SNUGGLE_MASK	},
	{ "CRECNT",	SNUGGLE_CRECNT	},
	{ "CREATOR",	SNUGGLE_CREMASK	},
	{ "TOP",	SNUGGLE_TOP	},
	{ "BOTTOM",	SNUGGLE_BOTTOM	},
	{ "ID",		SNUGGLE_ID	},
	{ (char *)NULL,	(u_short)NULL	}
};

void
add_snuggle(char *creator, char *nm, char *um, char *hm, char *rm, char *reason, u_short type, time_t created)
{
	snuggle_t *snuggle = leetmalloc(sizeof(snuggle_t));
	snuggle->next = main_snuggle;
	main_snuggle = snuggle;

	snuggle->creator = leetstrdup(creator);
	snuggle->nm = leetstrdup(nm);
	snuggle->um = leetstrdup(um);
	snuggle->hm = leetstrdup(hm);
	snuggle->rm = leetstrdup(rm);
	snuggle->reason = leetstrdup(reason);

	snuggle->type = type;
	if (created == 0)
		snuggle->created = time(NULL);
	else
		snuggle->created = created;
	snuggle->matches = 0;
}

int
del_snuggle(user_t *cptr, char *nm, char *um, char *hm, char *rm)
{
	snuggle_t *tmp, **all;
	user_t *utmp;

	for (all = &main_snuggle; *all; all = &((*all)->next))
		if (strcasecmp((*all)->nm, nm) == 0 && strcasecmp((*all)->um, um) == 0 && strcasecmp((*all)->hm, hm) == 0 && strcasecmp((*all)->rm, rm) == 0) {
			if (cptr && IsLeet(cptr))
				tell_chans(replies[RPL_CREMSNUGGLE], find_oper_nick(cptr), (*all)->type, nm, um, hm, rm, (*all)->reason);
			else if (cptr && (utmp = find_client(strnull(strtok((*all)->creator, "!")))) && IsLeet(utmp)) {
				reply(OS, utmp->nick, replies[RPL_PFORCESNUG], cptr->nick, cptr->username, cptr->host, nm, um, hm, rm);
				tell_chans(replies[RPL_CFORCESNUG], cptr->nick, cptr->username, cptr->host, (*all)->type, nm, um, hm, rm, (*all)->reason);
			}
			leetfree((*all)->creator, strlen((*all)->creator) + 1);
			leetfree((*all)->nm, strlen((*all)->nm) + 1);
			leetfree((*all)->um, strlen((*all)->um) + 1);
			leetfree((*all)->hm, strlen((*all)->hm) + 1);
			leetfree((*all)->rm, strlen((*all)->rm) + 1);
			leetfree((*all)->reason, strlen((*all)->reason) + 1);
			tmp = *all;
			*all = (*all)->next;
			leetfree(tmp, sizeof(snuggle_t));
			return 1;
		}
	return 0;
}

snuggle_t *
find_snuggle(char *nm, char *um, char *hm, char *rm)
{
	snuggle_t *snuggle = main_snuggle;

	for (; snuggle; snuggle = snuggle->next)
		if (strcasecmp(snuggle->nm, nm) == 0 && strcasecmp(snuggle->um, um) == 0 && strcasecmp(snuggle->hm, hm) == 0 && strcasecmp(snuggle->rm, rm) == 0)
			return snuggle;
	return NULL;
}

int
match_snuggle(char *nick, char *username, char *host, char *realname)
{
	snuggle_t *snuggle = main_snuggle;
	user_t *user;

	for (; snuggle; snuggle = snuggle->next)
		if (snuggle->type != 1 && match(snuggle->nm, nick) && (snuggle->type == 3 || (match(snuggle->hm, host) && match(snuggle->um, username) && match(snuggle->rm, realname)))) {
			snuggle->matches++;
			switch (snuggle->type) {
				case 5:
					md5_snuggle(snuggle);
					gline(NULL, "*", host, 12, snuggle->reason, 2);
					tell_chans("\2[AGLINE]\2 %s!%s@%s$%s - %s - %s!%s@%s$%s", nick, username, host, realname,
						   snuggle->reason, snuggle->nm, snuggle->um, snuggle->hm, snuggle->rm);
					return 1;
				case 6:
					md5_snuggle(snuggle);
					gline(NULL, "~*", host, 12, snuggle->reason, 2);
					tell_chans("\2[AGLINE]\2 %s!%s@%s$%s - %s - %s!%s@%s$%s", nick, username, host, realname,
						   snuggle->reason, snuggle->nm, snuggle->um, snuggle->hm, snuggle->rm);
					return 1;
				case 4:
					md5_snuggle(snuggle);
					gline(NULL, username, host, 12, snuggle->reason, 2);
					tell_chans("\2[AGLINE]\2 %s!%s@%s$%s - %s - %s!%s@%s$%s", nick, username, host, realname,
						   snuggle->reason, snuggle->nm, snuggle->um, snuggle->hm, snuggle->rm);
					return 1;
				case 3:
					if ((user = find_client(nick))) {
						irc_kill(OS, user->nick, snuggle->reason);
						del_user(user);
					}
					switch (me.servtype) {
						case SERV_IRCNN:
							toserv(":%s NICK %s 1 %lu %s %s %s :%s\r\n", me.servname, nick, time(NULL) - 60, nick, me.servname, me.servname, snuggle->reason);
							toserv(":%s MODE %s +i\r\n", nick, nick);
							break;
						case SERV_HYBRD:
							toserv(":%s NICK %s 1 %lu +i %s %s %s :%s\r\n", me.servname, nick, time(NULL) - 60, nick, me.servname, me.servname, snuggle->reason);
							break;
					}
					add_user(nick, nick, me.servname, snuggle->reason, me.servname, NOPRIV);
					tell_chans("\2[NJUPE]\2 %s!%s@%s$%s - %s - %s!%s@%s$%s", nick, username, host, realname,
	 						snuggle->reason, snuggle->nm, snuggle->um, snuggle->hm, snuggle->rm);
					return 1;
				case 2:
					irc_kill(OS, nick, snuggle->reason);
					tell_chans("\2[AKILL]\2 %s!%s@%s$%s - %s - %s!%s@%s$%s", nick, username, host, realname,
							snuggle->reason, snuggle->nm, snuggle->um, snuggle->hm, snuggle->rm);
					return 1;
			}
		}
	if (me.eob)
		for (snuggle = main_snuggle; snuggle; snuggle = snuggle->next)
			if (snuggle->type == 1 && match(snuggle->nm, nick) && match(snuggle->um, username) && match(snuggle->hm, host) && match(snuggle->rm, realname)) {
				snuggle->matches++;
				tell_chans("\2[SNUGGLE]\2 %s!%s@%s$%s - %s - %s!%s@%s$%s", nick, username, host, realname,
						snuggle->reason, snuggle->nm, snuggle->um, snuggle->hm, snuggle->rm);
				}
	return 0;
}

char *
md5_snuggle(snuggle_t *snuggle)
{
	char buf[BUFSIZE];
	snprintf(buf, BUFSIZE, "%s!%s@%s$%s:%d", snuggle->nm, snuggle->um, snuggle->hm, snuggle->rm, snuggle->type);
	return wiz_md5(buf);
}

void
del_all_snuggles(void)
{
	snuggle_t *tmp, **all = &main_snuggle;

	while (*all) {
		leetfree((*all)->creator, strlen((*all)->creator) + 1);
		leetfree((*all)->nm, strlen((*all)->nm) + 1);
		leetfree((*all)->um, strlen((*all)->um) + 1);
		leetfree((*all)->hm, strlen((*all)->hm) + 1);
		leetfree((*all)->rm, strlen((*all)->rm) + 1);
		leetfree((*all)->reason, strlen((*all)->reason) + 1);
		tmp = *all;
		*all = (*all)->next;
		leetfree(tmp, sizeof(snuggle_t));
	}
	main_snuggle = NULL;
}


int
list_snuggles(char *from, char *args, u_short qtype)
{
	snuggle_t *snuggle = main_snuggle;
	char *nm, *um, *hm, *rm;
	u_short i = 0;

	if (!snuggle)
		return reply(OS, from, replies[ERR_NOSNUGGLES]);

	switch (qtype)
	{
		case SNUGGLE_CNT:
			for (; snuggle; snuggle = snuggle->next)
				i++;
			reply(OS, from, replies[RPL_SNUGGLECNT], i);
			break;
		case SNUGGLE_ALL:
			for (; snuggle; snuggle = snuggle->next)
			{
				reply(OS, from, replies[RPL_SNUGLISTA], ++i, snuggle->nm, snuggle->um,
					 snuggle->hm, snuggle->rm, snuggle->reason, snuggle->matches);
				reply(OS, from, replies[RPL_SNUGLISTB], leetctime(snuggle->created),
					 snuggle->creator, snuggle->type);
			}
			reply(OS, from, replies[RPL_ENDOFSNUGL]);
			break;
		case SNUGGLE_MASK:
			if (!((nm = strtok(args, "!")) && (um = strtok(NULL, "@")) &&
			    (hm = strtok(NULL, "$")) && (rm = strtok(NULL, ""))))
				return NEED_HELP;
			for (; snuggle; snuggle = snuggle->next)
				if (match(nm, snuggle->nm) && match(um, snuggle->um) &&
				    match(hm, snuggle->hm) && match(rm, snuggle->rm))
				{
					reply(OS, from, replies[RPL_SNUGLISTA], ++i, snuggle->nm,
						 snuggle->um, snuggle->hm, snuggle->rm, snuggle->reason,
						 snuggle->matches);
					reply(OS, from, replies[RPL_SNUGLISTB], leetctime(snuggle->created),
						 snuggle->creator, snuggle->type);
				}
			if (i > 0)
				reply(OS, from, replies[RPL_ENDOFSNUGL]);
			else
				reply(OS, from, replies[ERR_NOMATCHSNUG]);
			break;
		case SNUGGLE_CRECNT:
			if (!args)
				return NEED_HELP;
			for (; snuggle; snuggle = snuggle->next)
				if (match(args, snuggle->creator))
					i++;
			reply(OS, from, replies[RPL_CRECNTSNUG], i);
			break;
		case SNUGGLE_CREMASK:
			if (!args)
				return NEED_HELP;
			for (; snuggle; snuggle = snuggle->next)
				if (match(args, snuggle->creator))
				{
					reply(OS, from, replies[RPL_SNUGLISTA], ++i, snuggle->nm,
						 snuggle->um, snuggle->hm, snuggle->rm, snuggle->reason,
						 snuggle->matches);
					reply(OS, from, replies[RPL_SNUGLISTB], leetctime(snuggle->created),
						 snuggle->creator, snuggle->type);
				}
			if (i > 0)
				reply(OS, from, replies[RPL_ENDOFSNUGL]);
			else
				reply(OS, from, replies[ERR_NOMATCHSNUG]);
			break;
		case SNUGGLE_TOP:
			if (!args || atoi(args) <= 0)
				return NEED_HELP;
			for (; snuggle; snuggle = snuggle->next)
				i++;
			if (atoi(args) > i)
				return reply(OS, from, replies[ERR_TOOMANYSNUG], i);
		{
			snuggle_t **all = leetcalloc(i + 1, sizeof(snuggle_t *));
			short j = i = 0;
			for (snuggle = main_snuggle; (all[i++] = snuggle); snuggle = snuggle->next);
			qsort(all, --i, sizeof(snuggle_t *), &snugtop);
			for (i = atoi(args); j < i && (snuggle = all[j]);)
			{
				reply(OS, from, replies[RPL_SNUGLISTA], ++j, snuggle->nm, snuggle->um,
					 snuggle->hm, snuggle->rm, snuggle->reason, snuggle->matches);
				reply(OS, from, replies[RPL_SNUGLISTB], leetctime(snuggle->created),
					 snuggle->creator, snuggle->type);
			}
			leetfree(all, (--j * sizeof(snuggle_t *)));
		}
			reply(OS, from, replies[RPL_ENDOFSNUGL]);
			break;
		case SNUGGLE_BOTTOM:
			if (!args || atoi(args) <= 0)
				return NEED_HELP;
			for (; snuggle; snuggle = snuggle->next)
				i++;
			if (atoi(args) > i)
				return reply(OS, from, replies[ERR_TOOMANYSNUG], i);
		{
			snuggle_t **all = leetcalloc(i + 1, sizeof(snuggle_t *));
			short j = i = 0;
			for (snuggle = main_snuggle; (all[i++] = snuggle); snuggle = snuggle->next);
			qsort(all, --i, sizeof(snuggle_t *), &snugbottom);
			for (i = atoi(args); j < i && (snuggle = all[j]);)
			{
				reply(OS, from, replies[RPL_SNUGLISTA], ++j, snuggle->nm, snuggle->um,
					 snuggle->hm, snuggle->rm, snuggle->reason, snuggle->matches);
				reply(OS, from, replies[RPL_SNUGLISTB], leetctime(snuggle->created),
					 snuggle->creator, snuggle->type);
			}
			leetfree(all, (--j * sizeof(snuggle_t *)));
		}
			reply(OS, from, replies[RPL_ENDOFSNUGL]);
			break;
		case SNUGGLE_ID:
			if (!args)
				return NEED_HELP;
			if (strlen(args) != 32 || strstr(args, " "))
				return reply(OS, from, replies[ERR_SNUGIDA]);

			for (snuggle = main_snuggle; snuggle; snuggle = snuggle->next)
				if (strcmp(md5_snuggle(snuggle), args) == 0)
					goto okid;
			return reply(OS, from, replies[ERR_SNUGIDB]);
		okid:
			reply(OS, from, replies[RPL_SNUGLISTA], 1, snuggle->nm, snuggle->um, snuggle->hm, snuggle->rm,
				snuggle->reason, snuggle->matches);
			reply(OS, from, replies[RPL_SNUGLISTB], leetctime(snuggle->created), snuggle->creator, snuggle->type);
			break;
	}
	return 1;
}

int
snugtop(const void *a, const void *b)
{
	snuggle_t *c = (snuggle_t *)*(unsigned *)a, *d = (snuggle_t *)*(unsigned *)b;
	return d->matches - c->matches;
}

int
snugbottom(const void *a, const void *b)
{
	snuggle_t *c = (snuggle_t *)*(unsigned *)a, *d = (snuggle_t *)*(unsigned *)b;
	return c->matches - d->matches;
}
