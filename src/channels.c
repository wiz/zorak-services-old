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
#include "channels.h"
#include "defines.h"
#include "privs.h"
#include "send.h"
#include "mem.h"
#include "me.h"

WIZOSID("$Id: channels.c,v 1.13 2002/07/30 03:23:02 wiz Exp $");

channel *main_channel = NULL;

channel *
add_channel(char *name, time_t ts, u_short flags)
{
	channel *chan;

	chan = leetmalloc(sizeof(channel));
	chan->next = main_channel;
	main_channel = chan;

	chan->name = leetstrdup(name);
	chan->ts = ts;
	chan->nmembers = 0;
	chan->members = NULL;
	chan->topic = NULL;

	me.channels++;
	return chan;
}

u_short
add_user_to_channel(user_t *user, channel *chan, u_short status)
{
	member *cmember;

	if (!user || !chan || is_member_channel(user, chan))
		return 0;
	cmember = leetmalloc(sizeof(member));
	cmember->next = chan->members;
	chan->members = cmember;

	cmember->user = user;
	cmember->status = status;

	chan->nmembers++;
	return 1;
}

u_short
del_channel(channel *chan)
{
	channel *tmp, **all;

	for (all = &main_channel; (tmp = *all); all = &tmp->next)
		if (tmp == chan) {
			me.channels--;
			leetfree(tmp->name, strlen(tmp->name) + 1);
			*all = tmp->next;
			leetfree(tmp, sizeof(channel));
			return 1;
		}
	return 0;
}

u_short
is_member_channel(user_t *user, channel *chan)
{
	member *__member = chan->members;

	for (; __member; __member = __member->next)
		if (__member->user == user)
			return 1;
	return 0;
}

void
del_user_in_channels(user_t *user)
{
	channel *tmp, *chan = main_channel;

	while (chan) {
		tmp = chan->next;
		if (is_member_channel(user, chan))
			del_user_from_channel(user, chan);
		chan = tmp;
	}
}

u_short
del_user_from_channel(user_t *user, channel *chan)
{
	member *tmp, **all;

	if (!user || !chan)
		return 0;
	for (all = &chan->members; (tmp = *all); all = &((*all)->next))
		if (tmp->user == user) {
			chan->nmembers--;
			*all = tmp->next;
			leetfree(tmp, sizeof(member));
			if (chan->nmembers == 0)
				del_channel(chan);
			return 1;
		}
	return 0;
}

void
del_all_channels(void)
{
	channel *tmp, **all = &main_channel;
	member *mtmp, **mall;

	while (*all) {
		mall = &(*all)->members;
		while (*mall)
		{
			mtmp = *mall;
			*mall = (*mall)->next;
			leetfree(mtmp, sizeof(member));
		}
		leetfree((*all)->name, strlen((*all)->name) + 1);
		tmp = *all;
		*all = (*all)->next;
		leetfree(tmp, sizeof(channel));
	}
	me.channels = 0;
	main_channel = NULL;
}

channel *
find_channel(char *name)
{
	channel *__channel = main_channel;

	for (; __channel; __channel = __channel->next)
		if (strcasecmp(__channel->name, name) == 0)
			return __channel;
	return NULL;
}

char *
get_channel_modes(channel *chan)
{
	return "+";
}

char *
get_user_prefix(channel *chan, member *user)
{
	return "";
}

u_short
list_channels(user_t *cptr, char *cm)
{
	channel *chan = main_channel;
	u_short i = 0;

	for (; chan && (i < 200 || IsAdmin(cptr)); chan = chan->next)
		if (match(cm, chan->name))
			reply(CS, cptr->nick, "%d) %s %d", ++i, chan->name, chan->nmembers);
	reply(CS, cptr->nick, "End of LIST.");
	return i;
}

u_short
channel_who(user_t *cptr, channel *chan)
{
	member *people = chan->members;

	reply(CS, cptr->nick, "Topic for %s: %s", chan->name, (chan->topic ? chan->topic : "(none)"));
	reply(CS, cptr->nick, "Channel %s created on %s", chan->name, leetctime(chan->ts));
	reply(CS, cptr->nick, "Mode for channel %s is \"%s\"", chan->name, get_channel_modes(chan));
	reply(CS, cptr->nick, "Users on %s (%d total):", chan->name, chan->nmembers);

	for (; people; people = people->next)
		reply(CS, cptr->nick, "    %s%s (%s@%s) [%s]", get_user_prefix(chan, people), people->user->nick,
			people->user->username, people->user->host, people->user->server->nick);

	reply(CS, cptr->nick, "End of WHO.");
	return 1;
}

void
join_channels(u_short nicknum)
{
	channel *chan;
	u_short i;

	switch (me.servtype) {
		case SERV_IRCNN:
			switch (nicknum) {
				case CS:
					for (i = 0; me.chans[i]; i++) {
						toserv(":%s JOIN %s\r\n", me.sclients[CS].nick, me.chans[i]);
						hackops(me.chans[i], me.sclients[CS].nick);
					}
					break;
				case OS:
					for (i = 0; me.chans[i]; i++) {
						toserv(":%s JOIN %s\r\n", me.sclients[OS].nick, me.chans[i]);
						hackops(me.chans[i], me.sclients[OS].nick);
					}
					if (me.eob == 0) {
						tell_chans("\2[EOB]\2 Receiving burst from %s", me.hub->name);
						me.htmtime = time(NULL);
					}
					/* XXX - eskimo's lame services doesn't know what this channel is */
					toserv(":%s JOIN # 1\r\n", me.sclients[OS].nick);
					hackops("#", me.sclients[OS].nick);
					toserv(":%s MODE # +b *!*@*\r\n", me.sclients[OS].nick);
					toserv(":%s MODE # +inmsl 1\r\n", me.sclients[OS].nick);
				
					/* XXX - reaper sux */
					toserv(":%s JOIN #debug 1\r\n", me.sclients[OS].nick);
					servmode("#debug", me.sclients[OS].nick);
					toserv(":%s MODE #debug +b *!*@*\r\n", me.sclients[OS].nick);
					toserv(":%s MODE #debug +inmsl 1\r\n", me.sclients[OS].nick);
					break;
				default:
					return;
			}
			break;
		case SERV_HYBRD:
			/* :h6.wiz.cx SJOIN 1021703401 #stuff +tn :@wiz6 */
			if (nicknum != CS && nicknum != OS)
				break;
			for (i = 0; me.chans[i]; i++)
				if ((chan = find_channel(me.chans[i]))) {
					toserv(":%s SJOIN %lu %s + :@%s\r\n", me.servname, chan->ts, me.chans[i], me.sclients[nicknum].nick);
					add_user_to_channel(find_client(me.sclients[nicknum].nick), find_channel(me.chans[i]), OP);
				} else {
					toserv(":%s SJOIN %lu %s + :@%s\r\n", me.servname, time(NULL), me.chans[i], me.sclients[nicknum].nick);
					add_user_to_channel(find_client(me.sclients[nicknum].nick), add_channel(me.chans[i], time(NULL), 0), OP);
				}
			break;
	}
}

u_short
is_services_chan(char *chan)
{
	u_short i = 0;

	for (; me.chans[i]; i++)
		if (strcasecmp(me.chans[i], chan) == 0)
			return 1;
	return 0;
}
