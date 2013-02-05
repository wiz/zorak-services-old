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

#include <unistd.h> /* execl() */
#include <errno.h>

#include "irc_types.h"
#include "irc_string.h"
#include "irc_time.h"
#include "operserv.h"
#include "channels.h"
#include "snuggle.h"
#include "defines.h"
#include "servers.h"
#include "config.h"
#include "privs.h"
#include "oline.h"
#include "clone.h"
#include "conf.h"
#include "main.h"
#include "send.h"
#include "dcc.h"
#include "mem.h"
#include "md5.h"
#include "net.h"
#include "rsa.h"
#include "db.h"
#include "me.h"

WIZOSID("$Id: operserv.c,v 1.26 2002/10/02 02:56:20 wiz Exp $");

void
chan_clones(user_t *cptr, char *to, char *args)
{
	list_clones(to ? to : cptr->nick);
}

void
chan_status(user_t *cptr, char *to, char *args)
{
	time_t upt = time(NULL) - me.start;
	reply(OS, to ? to : cptr->nick, "%5.5s up %d day%s, %d:%02d:%02d, %d admin%s, memory usage: %d bytes",
			leetctime(time(NULL)) + 11, upt / 86400, S(upt / 86400), (upt / 3600) % 24, (upt / 60) % 60, upt % 60,
			me.admins, S(me.admins), me.memusage);
	list_admins(cptr, to ? to : cptr->nick);
	reply(OS, to ? to : cptr->nick, "There are %d user%s, %d oper%s, on %d server%s, and %d channel%s formed.",
			me.users, S(me.users), me.opers, S(me.opers), me.servers, S(me.servers), me.channels, S(me.channels));
}

void
chan_killclones(user_t *cptr, char *to, char *args)
{
	u_short i = kill_clones();

	if (i < 1) {
		reply(OS, to ? to : cptr->nick, "no clones found.");
		return;
	}
	log("%s!%s@%s on %s KILLCLONES killed %d clones", cptr->nick, cptr->username, cptr->host, cptr->server->nick, i);
	tell_chans("\2[CLONES]\2 A total of %d clones were killed by %s", i, cptr->nick);
	reply(OS, cptr->nick, "Killed a total of %d clones.", i);
}

u_short
os_clones(user_t *cptr, char *args)
{
	chan_clones(cptr, NULL, args);
	return 1;
}

u_short
os_killclones(user_t *cptr, char *args)
{
	chan_killclones(cptr, NULL, args);
	return 1;
}

u_short
os_status(user_t *cptr, char *args)
{
	chan_status(cptr, NULL, args);
	return 1;
}

u_short
os_chat(user_t *cptr, char *args)
{
	dcc_chat_offer(cptr);
	return 1;
}

u_short
os_aclones(user_t *cptr, char *args)
{
	return tell_clone_allows(cptr->nick);
}

u_short
os_mode(user_t *cptr, char *args)
{
	channel *channel;
	char *chan, *modes;

	if (!args || !(chan = strtok(args, " ")) || !(modes = strtok(NULL, "")))
		return NEED_HELP;

	if (chan[0] != '#')
		return reply(OS, cptr->nick, "\"%s\" is not a valid channel name.", chan);

	if (!(channel = find_channel(chan)))
		return reply(OS, cptr->nick, "I thought %s was empty?", chan);

	if (modes[0] != '+' && modes[0] != '-')
		return reply(OS, cptr->nick, "\"%s\" is not a valid channel mode", modes);

	log("%s!%s@%s on %s MODE: %s %s", cptr->nick, cptr->username, cptr->host, cptr->server->nick, chan, modes);
	operwall(0, "%s!%s@%s on %s requested: MODE %s %s", cptr->nick, cptr->username, cptr->host, cptr->server->nick, chan, modes);
	servmode(chan, modes);
	return 1;
}

u_short
os_kick(user_t *cptr, char *args)
{
	user_t *kptr;
	channel *chan;
	char *where, *who, *reason;

	if (!args || !(where = strtok(args, " ")) || !(who = strtok(NULL, " ")) || !(reason = strtok(NULL, "")))
		return NEED_HELP;

	if (is_services_client(who))
		return reply(OS, cptr->nick, "Now why would I want to do that?");

	if (!(chan = find_channel(where)))
		return reply(OS, cptr->nick, "%s is not a valid channel.", where);
	if (!(kptr = find_client(who)) || !del_user_from_channel(kptr, chan))
		return reply(OS, cptr->nick, "%s is not in %s right now.", who, where);

	log("%s!%s@%s on %s KICK: %s from %s (%s)", cptr->nick, cptr->username, cptr->host, cptr->server->nick, who, where, reason);
	operwall(0, "%s!%s@%s on %s requested: KICK %s %s (%s)", cptr->nick, cptr->username, cptr->host, cptr->server->nick, where, who, reason);
	toserv(":%s KICK %s %s :%s\r\n", me.servname, where, who, reason);
	return 1;
}

u_short
os_settime(user_t *cptr, char *args)
{
	log("%s!%s@%s on %s SETTIME", cptr->nick, cptr->username, cptr->host, cptr->server->nick);
	operwall(OS, "%s!%s@%s on %s requested: SETTIME", cptr->nick, cptr->username, cptr->host, cptr->server->nick);
	return reply(OS, cptr->nick, "clocks set to: %lu", settime());
}

u_short
os_kill(user_t *cptr, char *args)
{
	char *who, *reason;
	user_t *user;

	if (!args || !(who = strtok(args, " ")) || !(reason = strtok(NULL, "")))
		return NEED_HELP;

	if (strcasecmp(who, me.sclients[OS].nick) == 0)
		return reply(OS, cptr->nick, "fek u");

	if (!(user = find_client(who)))
		return reply(OS, cptr->nick, "%s: No such nick.", who);

	if (check_privs(who) != NOPRIV && !IsAdmin(cptr))
		return reply(OS, cptr->nick, "I'm not going to kill %s!.", who);

	log("%s!%s@%s on %s KILL: %s (%s)", cptr->nick, cptr->username, cptr->host, cptr->server->nick, who, reason);
	operwall(0, "%s!%s@%s on %s requested: KILL %s (%s)", cptr->nick, cptr->username, cptr->host, cptr->server->nick, who, reason);
	irc_kill(OS, who, reason);
	del_user(user);
	return 1;
}

u_short
os_placehold(user_t *cptr, char *args)
{
	char *who, *reason;

	if (!args || !(who = strtok(args, " ")) || !(reason = strtok(NULL, "")))
		return NEED_HELP;

	if (check_nick(cptr, who) || check_reason(cptr, reason))
		return 1;

	if (match("ChanServ*", who) || match("OperServ*", who) || match("NickServ*", who) || match("MemoServ*", who) || match("HelpServ*", who) || match("Security*", who))
		return reply(OS, cptr->nick, "You may not placehold services nicks.");

	if (find_client(who))
		return reply(OS, cptr->nick, "Nickname %s is already in use; cannot be placeheld.", who);

	operwall(0, "%s!%s@%s on %s requested: PLACEHOLD %s (%s)", cptr->nick, cptr->username, cptr->host, cptr->server->nick, who, reason);
	switch (me.servtype) {
		case SERV_IRCNN:
			toserv(":%s NICK %s 1 %lu %s %s %s :%s\r\n", me.servname, who, time(NULL), who, me.servname, me.servname, reason);
			toserv(":%s MODE %s :+i\r\n", who, who);
			break;
		case SERV_HYBRD:
			toserv(":%s NICK %s 1 %lu +i %s %s %s :%s\r\n", me.servname, who, time(NULL), who, me.servname, me.servname, reason);
			break;
	}
	add_user(who, who, me.servname, reason, me.servname, NOPRIV);
	log("%s!%s@%s on %s PLACEHOLD: %s (%s)", cptr->nick, cptr->username, cptr->host, cptr->server->nick, who, reason);
	return 1;
}

u_short
os_kline(user_t *cptr, char *args)
{
	return reply(OS, cptr->nick, "KLINE is no longer used, please GLINE instead.");
}

u_short
os_gline(user_t *cptr, char *args)
{
	char *um, *hm, *length, *reason;
	int gtime;

	if (!args || !(um = strtok(args, "@")) || !(hm = strtok(NULL, " ")) || !(length = strtok(NULL, " ")) ||
	    !(reason = strtok(NULL, "")) || (!(gtime = atoi(length)) && strcmp(length, "0") != 0))
		return NEED_HELP;

	if (gtime < 1 || gtime > 9999 || (gtime > 72 && !IsCoAdmin(cptr)))
		return reply(OS, cptr->nick, "Please choose a time for your GLINE between 1 and 72 hours.");

	if (check_um(cptr, um) || check_hm(cptr, hm) || check_reason(cptr, reason))
		return 1;

	log("%s!%s@%s on %s GLINE: %s@%s %d (%s)", cptr->nick, cptr->username, cptr->host,
	    cptr->server->nick, um, hm, gtime, reason);
	gline(cptr, um, hm, gtime, reason, 0);
	return 1;
}

u_short
os_anongline(user_t *cptr, char *args)
{
	char *um, *hm, *length, *reason;
	int gtime;

	if (!args || !(um = strtok(args, "@")) || !(hm = strtok(NULL, " ")) || !(length = strtok(NULL, " ")) ||
	    !(reason = strtok(NULL, "")) || (!(gtime = atoi(length)) && strcmp(length, "0") != 0))
		return NEED_HELP;

	if (gtime < 1 || gtime > 9999 || (gtime > 72 && !IsCoAdmin(cptr)))
		return reply(OS, cptr->nick, "Please choose a time for your ANONGLINE between 1 and 72 hours.");

	if (check_um(cptr, um) || check_hm(cptr, hm) || check_reason(cptr, reason))
		return 1;

	log("%s!%s@%s on %s ANONGLINE: %s@%s %d (%s)", cptr->nick, cptr->username, cptr->host,
	    cptr->server->nick, um, hm, gtime, reason);
	gline(cptr, um, hm, gtime, reason, 1);
	return 1;
}

u_short
os_snuggle(user_t *cptr, char *args)
{
	char *what, *nm, *um, *hm, *rm, *reason, *tmp;
	snuggle_t *snuggle;
	int type;

	if (!args || !(what = strtok(args, " ")))
		return NEED_HELP;

	if (strcasecmp(what, "LIST") == 0)
	{
		snuggle_type *sptr = snuggle_types;

		if ((tmp = strtok(NULL, " ")))
			for (; sptr->name; sptr++)
				if (strcasecmp(sptr->name, tmp) == 0) {
					tmp = strtok(NULL, "");
					return list_snuggles(cptr->nick, tmp, sptr->qtype);
				}
		return NEED_HELP;
	}
	if (!(nm = strtok(NULL, "!")) || !(um = strtok(NULL, "@")) || !(hm = strtok(NULL, "$")) || !(rm = strtok(NULL, ":")))
		return NEED_HELP;

	if (strcasecmp(what, "DEL") == 0) {
		if (!(snuggle = find_snuggle(nm, um, hm, rm)))
			return reply(OS, cptr->nick, "No matching SNUGGLE records were found.");

		log("%s!%s@%s removed SNUGGLE for %s!%s@%s$%s - %s", cptr->nick, cptr->username, cptr->host, nm, um, hm, rm, snuggle->reason);
		del_snuggle(cptr, nm, um, hm, rm);
		if (write_db(DB_SNUGGLE) == -1) {
			tell_chans("ERROR! Unable to write %s: %s", databases[DB_SNUGGLE], strerror(errno));
			return reply(OS, cptr->nick, "ERROR! Unable to write %s: %s", databases[DB_SNUGGLE], strerror(errno));
		}
		return reply(OS, cptr->nick, "The matching SNUGGLE was removed successfully.");
	}

	if (!IsLeet(cptr))
		return reply(OS, cptr->nick, "You do not have access to modify the list of SNUGGLE records, please contact an admin.");

	if (!(reason = strtok(NULL, ":")) || !(tmp = strtok(NULL, "")))
		return NEED_HELP;

	if (strcasecmp(what, "ADD") == 0) {
		if (check_nm(cptr, nm) || check_um(cptr, um) || check_hm(cptr, hm) || check_rm(cptr, rm) || check_reason(cptr, reason))
			return 1;

		if ((!(type = atoi(tmp)) && strcmp(tmp, "0") != 0) || type < 1 || type > 6)
			return reply(OS, cptr->nick, "Please choose a SNUGGLE type between 1 and 6.");

		if (match(nm, me.sclients[OS].nick) && match(um, me.sclients[OS].username) && match(hm, me.servname) &&
		    match(rm, me.sclients[OS].realname))
			return reply(OS, cptr->nick, "You cannot SNUGGLE that.");

		if (type == 3 && check_nick(NULL, nm) && !IsCoAdmin(cptr))
			return reply(OS, cptr->nick, "Wildcards are not allowed in a type 3 SNUGGLE nickmask; ask an admin to add it for you.");

		if (type == 6 && um[0] != '~')
			return reply(OS, cptr->nick, "For a type 6 SNUGGLE, '~' must be the first character of your username mask.");

		if ((snuggle = find_snuggle(nm, um, hm, rm))) {
			char tmp[BUFSIZE];
			snprintf(tmp, BUFSIZE, "%s!%s@%s$%s", snuggle->nm, snuggle->um, snuggle->hm, snuggle->rm);
			reply(OS, cptr->nick, "The following entry already exists and matches your request:");
			return list_snuggles(cptr->nick, tmp, SNUGGLE_MASK);
		}
		log("%s!%s@%s added SNUGGLE %d for %s!%s@%s$%s - %s", cptr->nick, cptr->username, cptr->host, type, nm, um, hm, rm, reason);
		add_snuggle(find_oper_nick(cptr), nm, um, hm, rm, reason, type, 0);
		tell_chans("%s added SNUGGLE(%d) for %s!%s@%s$%s (%s)", find_oper_nick(cptr), type, nm, um, hm, rm, reason);
		if (write_db(DB_SNUGGLE) == -1) {
			tell_chans("ERROR! Unable to write %s: %s", databases[DB_SNUGGLE], strerror(errno));
			return reply(OS, cptr->nick, "ERROR! Unable to write %s: %s", databases[DB_SNUGGLE], strerror(errno));
		}
		return reply(OS, cptr->nick, "Your SNUGGLE was added successfully.");
	}
	return NEED_HELP;
}

u_short
os_aclone(user_t *cptr, char *args)
{
	char *what, *hm, *tmp;
	clone_t *allow;
	int clones;

	if (!args || !(what = strtok(args, " ")))
		return NEED_HELP;

	if (strcasecmp(what, "LIST") == 0)
		return tell_clone_allows(cptr->nick);

	if (!(hm = strtok(NULL, " ")))
		return NEED_HELP;

	if (!IsLeet(cptr))
		return reply(OS, cptr->nick, "You do not have access to modify the list of ACLONE records, please contact an admin.");

	if (strcasecmp(what, "DEL") == 0) {
		if (!(allow = find_clone_allow(hm)))
			return reply(OS, cptr->nick, "No existng ACLONE entry matched your request.");

		log("%s!%s@%s on %s CLONE ALLOW: DEL (%d) for %s", cptr->nick, cptr->username, cptr->host, cptr->server->nick, allow->clones, allow->host);
		tell_chans("%s!%s@%s removed CLONE ALLOW %s for %d clones.", cptr->nick, cptr->username, cptr->host, allow->host, allow->clones);
		del_clone_allow(hm);
		if (write_db(DB_ACLONE) == -1) {
			tell_chans("ERROR! Unable to write %s: %s", databases[DB_ACLONE], strerror(errno));
			return reply(OS, cptr->nick, "ERROR! Unable to write %s: %s", databases[DB_ACLONE], strerror(errno));
		}
		return reply(OS, cptr->nick, "The matching ACLONE entry was removed successfully.");
	}
	if ((clones = atoi((tmp = strnull(strtok(NULL, ""))))) < 1 && match("0*", tmp))
		return NEED_HELP;

	if (strcasecmp(what, "ADD") == 0) {
		if (check_hm(cptr, hm))
			return 1;

		if (clones <= CLONE_LIMIT)
			return reply(OS, cptr->nick, "Everyone is already allowed to have at least 3 clones!");

		if (check_host(cptr, hm))
			return 1;

		if ((allow = find_clone_allow(hm)))
			return reply(OS, cptr->nick, "%s is \2ALREADY\2 allowed %d clones, so no.", allow->host, allow->clones);

		log("%s!%s@%s on %s CLONE ALLOW: ADD (%d) for %s", cptr->nick, cptr->username, cptr->host, cptr->server->nick, clones, hm);
		tell_chans("%s!%s@%s added CLONE ALLOW %s for %d clones.", cptr->nick, cptr->username, cptr->host, hm, clones);
		add_clone_allow(hm, clones);
		if (write_db(DB_ACLONE) == -1) {
			log("%s!%s@%s on %s SAVE ERROR! %s", cptr->nick, cptr->username, cptr->host, cptr->server->nick, strerror(errno));
			tell_chans("ERROR! Unable to write %s: %s", databases[DB_ACLONE], strerror(errno));
			return reply(OS, cptr->nick, "ERROR! Unable to write %s: %s", databases[DB_ACLONE], strerror(errno));
		}
		return reply(OS, cptr->nick, "%s is now allowed to have %d clones.", hm, clones);
	}
	return NEED_HELP;
}
