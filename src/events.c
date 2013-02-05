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
 *  $Id: events.c,v 1.2 2002/10/02 02:56:20 wiz Exp $
 */

#include "irc_types.h"
#include "events.h"
#include "mem.h"

event_t *main_event = NULL;

event_t *
add_event(time_t when, u_short often, void (*func)())
{
	event_t *event = leetcalloc(sizeof(event_t), 1);
  
	event->next = main_event;
	main_event = event;
	if (event->next)
		event->next->prev = event;
	event->when = when;
	event->often = often;
	event->func = func;
	return event;
}

event_t *
del_event(event_t *event)
{
	if (event->prev) {   
		event->prev->next = event->next;
	} else {
		main_event = event->next;
		if (event->next)
			event->next->prev = NULL;
	}
	if (event->next)
		event->next->prev = event->prev;
	leetfree(event, sizeof(event_t));
	return event->next;
}

u_short
del_event_func(void (*func)())
{
	event_t *event = main_event;

	for (; event; event = event->next)
		if (event->func == func) {
			del_event(event);
			return 1;
		}
	return 0;
}

void
do_events(time_t now)
{
	event_t *event = main_event;

	while (event) {
		if (event->when <= now) {
			(*event->func)();
			if (event->often != 0)
				event->when = now + event->often;
			else
				event->when = 0;
		}
		if (event->when == 0)
			event = del_event(event);
		else
			event = event->next;
	}
}
