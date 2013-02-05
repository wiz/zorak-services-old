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
#include "chanserv.h"
#include "channels.h"
#include "replies.h"
#include "defines.h"
#include "send.h"

WIZOSID("$Id: chanserv.c,v 1.6 2002/10/02 02:56:20 wiz Exp $");

u_short
cs_list(user_t *cptr, char *args)
{
	if (!args)
		return NEED_HELP;
	list_channels(cptr, args);
	return 1;
}

u_short
cs_who(user_t *cptr, char *args)
{
	channel *chan;

	if (!args)
		return NEED_HELP;
	if (check_chan(cptr, args))
		return 1;
	if (!(chan = find_channel(args)))
		return reply(CS, cptr->nick, replies[ERR_NOSUCHCHAN], args);

	return channel_who(cptr, chan);
}
