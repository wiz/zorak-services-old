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
 *  $Id: nicks.h,v 1.2 2002/10/02 02:56:15 wiz Exp $
 */

#ifndef __INCLUDE_NICKS_H_
#define __INCLUDE_NICKS_H_

typedef struct nick {
	struct nick *next;
	struct nick *prev;
	char *nick;
	char *passwd;
	time_t regtime;
	time_t lasttime;
	char *info;
	char *lasthost;
	u_short flags;
} nick_t;

/* 0x0040 - 0x0100 reserved for LEET, CADMIN, ADMIN flags */

nick_t *	add_nick_to_list	(void);
void		new_registered_nick	(user_t *, char *);
void		add_registered_nick	(char *, char *, time_t, time_t, char *, char *, u_short);
void		drop_registered_nick	(nick_t *);
nick_t *	get_nick		(char *);
u_short		try_passwd		(char *, char *);
void		change_nick_passwd	(nick_t *, char *);
void		update_last_info	(user_t *, nick_t *);
void		list_nicks		(user_t *, char *);
void		del_all_nicks		(void);
#endif
