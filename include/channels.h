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
 *  $Id: channels.h,v 1.1 2002/07/13 20:37:39 wiz Exp $
 */

#include "irc_types.h"
#include "client.h"

typedef struct _member {
	struct _member *next;
	user_t *user;
	u_short status;
} member;

typedef struct _channel {
	struct _channel *next;
	char *name; /* includes channel prefix character */
	char *topic;
	time_t ts;
	member *members;
	u_short nmembers;
	u_short flags;
} channel;

channel *	add_channel		(char *, time_t, u_short);
u_short		del_channel		(channel *);
void		del_all_channels	(void);
void		del_user_in_channels	(user_t *);
u_short		del_user_from_channel	(user_t *, channel *);
u_short		add_user_to_channel	(user_t *, channel *, u_short);
u_short		del_user_from_channel	(user_t *, channel *);
channel *	find_channel		(char *);
u_short		list_channels		(user_t *, char *);
u_short		is_member_channel	(user_t *, channel *);
char *		get_channel_modes	(channel *);
char *		get_user_prefix		(channel *, member *);
u_short		channel_who		(user_t *, channel *);
u_short		is_services_chan	(char *);
void		join_channels		(u_short);
