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
 *  $Id: events.h,v 1.2 2002/10/02 02:56:15 wiz Exp $
 */

#ifndef	__EVENTS_H_
#define	__EVENTS_H_
#include "irc_types.h"
#include "irc_time.h"

typedef struct event {
	struct event *next, *prev;
	time_t when;
	u_short often;
	void (*func)();
} event_t;

event_t *	add_event	(time_t, u_short, void (*)());
event_t *	del_event	(event_t *);
void		do_events	(time_t);
u_short		del_event_func	(void (*)());
#endif
