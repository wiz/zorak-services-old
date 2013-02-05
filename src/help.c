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

#include "defines.h"
#include "replies.h"
#include "privs.h"
#include "help.h"
#include "send.h"
#include "me.h"

WIZOSID("$Id: help.c,v 1.17 2002/07/13 20:37:46 wiz Exp $");

u_short
help(user_t *cptr, u_short snick, u_short v)
{
	reply(snick, cptr->nick, replies[RPL_HELP_A], me.sclients[snick].nick);
	reply(snick, cptr->nick, " ");

switch (snick) {
	case OS:
		reply(snick, cptr->nick, "Oper commands:");
#ifdef	HAVE_LIBCRYPTO
		reply(snick, cptr->nick, "\2CHALLENGE\2   Identify using publickey authentication");
#endif
		reply(snick, cptr->nick, "\2IDENTIFY\2    Identify using your password");
		reply(snick, cptr->nick, "\2MODE\2        Change a channel's modes");
		reply(snick, cptr->nick, "\2KICK\2        Kick a user from a channel");
		reply(snick, cptr->nick, "\2KILL\2        Kill a user off the network");
		reply(snick, cptr->nick, "\2GLINE\2       GLINE a given usermask@hostmask");
		reply(snick, cptr->nick, "\2ANONGLINE\2   Anonymously GLINE a given usermask@hostmask");
		reply(snick, cptr->nick, "\2WHO\2         Lists all clients matching mask provided");
		reply(snick, cptr->nick, "\2WHOKILL\2     Kills all clients matching mask provided");
		reply(snick, cptr->nick, "\2PLACEHOLD\2   Makes a placeholder for a nick not in use");
		reply(snick, cptr->nick, "\2SNUGGLE\2     Modify the list of SNUGGLE records");
		reply(snick, cptr->nick, "\2ACLONE\2      Modify the list of clone allow records");
		reply(snick, cptr->nick, "\2CLONES\2      Lists all unauthorized clones");
		reply(snick, cptr->nick, "\2KILLCLONES\2  Kills all unauthorized clones");
		if (me.servtype == SERV_IRCNN)
			reply(snick, cptr->nick, "\2SETTIME\2     Syncronize the servers' clocks with mine");
		reply(snick, cptr->nick, "\2HTM\2         Toggle high traffic mode ON or OFF");
		reply(snick, cptr->nick, "\2ADMINS\2      Lists current operators and admins");
		reply(snick, cptr->nick, "\2STATUS\2      Status of the network and services");

		if (!IsSetCoAdmin(cptr))
			goto end;

		reply(snick, cptr->nick, " ");
		reply(snick, cptr->nick, "Co-Admin commands:");
		reply(snick, cptr->nick, "\2SAVE\2        Saves all databases");
		reply(snick, cptr->nick, "\2JUPE\2        Forcefully removes a server from the network");
		reply(snick, cptr->nick, "\2REHASH\2      Reloads all configurations and databases");
		reply(snick, cptr->nick, "\2RESTART\2     Saves all databases and restarts");

		if (!IsSetAdmin(cptr))
			goto end;

		reply(snick, cptr->nick, " ");
		reply(snick, cptr->nick, "Admin commands:");
		reply(snick, cptr->nick, "\2DIE\2         Saves all databases and shuts down");
		reply(snick, cptr->nick, "\2RAW\2         Sends a string to my current hub server");
		goto end;
	case CS:
		if (!IsOper(cptr))
			goto end;
		reply(snick, cptr->nick, " ");
		reply(snick, cptr->nick, "Oper commands:");
		reply(snick, cptr->nick, "\2LIST\2        Lists all channels matching a given mask");
		reply(snick, cptr->nick, "\2WHO\2         Displays known info for a given channel");
		goto end;
	case NS:
		reply(snick, cptr->nick, "\2REGISTER\2    Registers a nickname with services");
		reply(snick, cptr->nick, "\2IDENTIFY\2    Identifies yourself to a registered nickname");
		reply(snick, cptr->nick, "\2DROP\2        Drops your nickname (requires identify first)");
		reply(snick, cptr->nick, "\2INFO\2        Displays info on given registered nickname");
		if (!IsOper(cptr))
			goto end;
		reply(snick, cptr->nick, " ");
		reply(snick, cptr->nick, "Oper commands:");
		reply(snick, cptr->nick, "\2LIST\2        Lists all registered nicks that match a pattern");
		goto end;
	case MS:
	case HS:
		reply(snick, cptr->nick, "(no commands available at this time)");
		goto end;
}
end:reply(snick, cptr->nick, " ");
	reply(snick, cptr->nick, "Remember, all commands sent to %s are logged!", me.sclients[snick].nick);
	return 1;
}

void
os_snuggle_help(user_t *cptr, u_short v)
{
	reply(OS, cptr->nick, "Usage: \2SNUGGLE ADD\2 \2nickmask\2!\2usermask\2@\2hostmask\2$\2namemask:reason:type");
	if (!v)
		goto next;
	reply(OS, cptr->nick, "Adds a SNUGGLE record with the mask provided and a type that is one of the following:");
	reply(OS, cptr->nick, " ");
	reply(OS, cptr->nick, "\0021\2: \2warn\2 the monitoring channel(s)");
	reply(OS, cptr->nick, "\0022\2: \2kill\2 the matching client");
	reply(OS, cptr->nick, "\0023\2: \2jupe\2 the matching nick");
	reply(OS, cptr->nick, "\0024\2: \2gline\2 the matching client's \2username@host\2");
	reply(OS, cptr->nick, "\0025\2: \2gline\2 the matching client's \2*@host\2");
	reply(OS, cptr->nick, "\0026\2: \2gline\2 the matching client's \2~*@host\2");
	reply(OS, cptr->nick, " ");
next:	reply(OS, cptr->nick, "Usage: \2SNUGGLE DEL\2 \2nickmask\2!\2usermask\2@\2hostmask\2$\2namemask");
	if (v) {
		reply(OS, cptr->nick, "Removes a SNUGGLE record with the mask provided.");
		reply(OS, cptr->nick, " ");
	}
	reply(OS, cptr->nick, "Usage: \2SNUGGLE LIST \2(\2COUNT\2|\2ALL\2|\2MASK\2|\2CRECNT\2|\2CREATOR\2|\2TOP\2|\2BOTTOM\2|\2ID\2)\2 \2[\2args\2]\2");
	if (!v)
		return;
	reply(OS, cptr->nick, "Lists snuggles using the following filters:");
	reply(OS, cptr->nick, " ");
	reply(OS, cptr->nick, "\2COUNT\2");
	reply(OS, cptr->nick, "\tCounts all SNUGGLE records.");
	reply(OS, cptr->nick, "\2ALL\2");
	reply(OS, cptr->nick, "\tLists all SNUGGLE records.");
	reply(OS, cptr->nick, "\2MASK nm\2!\2um\2@\2hm\2$\2rm\2");
	reply(OS, cptr->nick, "\tLists all SNUGGLE records matching the mask given.");
	reply(OS, cptr->nick, "\2CRECNT nickmask\2");
	reply(OS, cptr->nick, "\tCounts all SNUGGLE records with a creator matching the mask given.");
	reply(OS, cptr->nick, "\2CREATOR nickmask\2");
	reply(OS, cptr->nick, "\tLists all SNUGGLE records with a creator matching the mask given.");
	reply(OS, cptr->nick, "\2TOP \2[\2number\2]\2");
	reply(OS, cptr->nick, "\tSorts all SNUGGLE records by highest matches and displays the top 10, or top number if provided.");
	reply(OS, cptr->nick, "\2BOTTOM \2[\2number\2]\2");
	reply(OS, cptr->nick, "\tSorts all SNUGGLE records by highest matches and displays the bottom 10, or bottom [number] if provided.");
	reply(OS, cptr->nick, "\2ID idhash\2");
	reply(OS, cptr->nick, "\tLists the SNUGGLE record that matches the given SNUGGLE ID");
}

void
os_mode_help(user_t *cptr, u_short v)
{
	reply(OS, cptr->nick, "Usage: \2MODE #channel [modes]\2");
}

void
os_kick_help(user_t *cptr, u_short v)
{
	reply(OS, cptr->nick, "Usage: \2KICK #channel nick reason...\2");
}

void
os_kill_help(user_t *cptr, u_short v)
{
	reply(OS, cptr->nick, "Usage: \2KILL nick reason...\2");
}

void
os_gline_help(user_t *cptr, u_short v)
{
	reply(OS, cptr->nick, "Usage: (\2GLINE\2|\2ANONGLINE\2) user@host hours reason...\2");
	if (!v)
		return;
	reply(OS, cptr->nick, "Bans a user@host from the network for the number of hours specified.");
	reply(OS, cptr->nick, "ANONGLINE is the anonymous version of GLINE.");
}

void
os_who_help(user_t *cptr, u_short v)
{
	reply(OS, cptr->nick, "Usage: \2WHO\2 nm!um@hm$rm");
	if (v)
		reply(OS, cptr->nick, "Lists all users on the network matching the mask provided.");
}

void
os_whokill_help(user_t *cptr, u_short v)
{
	reply(OS, cptr->nick, "Usage: \2WHOKILL\2 nm!um@hm$rm:reason...");
	if (v)
		reply(OS, cptr->nick, "Kills all users on the network matching the mask provided.");
}

void
os_jupe_help(user_t *cptr, u_short v)
{
	reply(OS, cptr->nick, "Usage: \2JUPE\2 irc.server.name reason...");
	if (v)
		reply(OS, cptr->nick, "Forcefully jupes a server from the network with the reason specified.");
}

void
os_aclone_help(user_t *cptr, u_short v)
{
	reply(OS, cptr->nick, "Usage: \2ACLONE ADD\2 host clones");
	reply(OS, cptr->nick, "Usage: \2ACLONE DEL\2 host");
	reply(OS, cptr->nick, "Usage: \2ACLONE LIST\2");
	if (v)
		reply(OS, cptr->nick, "Adds, removes or lists clone exemptions from the database.");
}

void
cs_who_help(user_t *cptr, u_short v)
{
	reply(CS, cptr->nick, "Usage: \2WHO #channel\2");
	if (v)
		reply(CS, cptr->nick, "Displays all known info for a given channel.");
}

void
ns_register_help(user_t *cptr, u_short v)
{
	reply(NS, cptr->nick, "Usage: \2REGISTER password\2");
	reply(NS, cptr->nick, "Your password must be 5 - 25 characters in length, and not contain any spaces.");
	if (v)
		reply(NS, cptr->nick, "This command registers your nick with %s, ensuring that it cannot be used by someone else.", me.sclients[NS].nick);
}

void
ns_identify_help(user_t *cptr, u_short v)
{
	reply(NS, cptr->nick, "Usage: \2IDENTIFY password\2");
	if (v)
		reply(NS, cptr->nick, "This command identifies you to %s enabling you to use services.", me.sclients[NS].nick);
}

void
os_identify_help(user_t *cptr, u_short v)
{
	reply(OS, cptr->nick, "Usage: \2IDENTIFY opernick password\2");
	if (v)
		reply(OS, cptr->nick, "This command identifies you to services, if you have special access.");
}

void
ns_drop_help(user_t *cptr, u_short v)
{
	if (IsOper(cptr)) {
		reply(NS, cptr->nick, "Usage: \2DROP nickname\2");
		if (v)
			reply(NS, cptr->nick, "This command drops the given nickname.");
	} else {
		reply(NS, cptr->nick, "Usage: \2DROP\2");
		if (v)
			reply(NS, cptr->nick, "This command drops your registered nickname with %s, and frees it for re-regisitration.", me.sclients[NS].nick);
	}
}

void
cs_list_help(user_t *cptr, u_short v)
{
	reply(CS, cptr->nick, "Usage: \2LIST \2[\2mask\2]");
	if (v)
		reply(CS, cptr->nick, "Lists all channels matching the mask provided.");
}
