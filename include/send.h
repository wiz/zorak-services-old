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
 *  $Id: send.h,v 1.1 2002/07/13 20:37:39 wiz Exp $
 */

#include "irc_types.h"
#include "client.h"

int		reply			(u_short, char *, char *, ...);
int		operwall		(u_short, char *, ...);
int		tell_chans		(char *, ...);
void		irc_kill		(u_short, char *, char *);
void		servmode		(char *, char *);
void		hackops			(char *, char *);
time_t		settime			(void);
int		serv_notice		(char *, char *, ...);
int		is_an_oper_notice	(char *, char *, ...);
int		sendto_opers		(char *, ...);
int		sendto_admins		(char *, ...);
int		list_admins		(user_t *, char *);
void		clone_warn		(char *, u_int);
void		clone_allow_warn	(char *, u_int, u_int);
void		squit			(char *, char *);
void		toserv			(char *, ...);
