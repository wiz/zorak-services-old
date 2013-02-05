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
 *  $Id: parse.h,v 1.2 2002/10/02 02:56:15 wiz Exp $
 */

#include "irc_types.h"

void	parse		(char *);
u_short	parse_mode	(user_t *, char *);
int	m_privmsg	(user_t *, int, char **);
int	m_notice	(user_t *, int, char **);
int	m_server	(user_t *, int, char **);
int	m_sjoin		(user_t *, int, char **);
int	m_admin		(user_t *, int, char **);
int	m_stats		(user_t *, int, char **);
int	m_whois		(user_t *, int, char **);
int	m_squit		(user_t *, int, char **);
int	m_join		(user_t *, int, char **);
int	m_part		(user_t *, int, char **);
int	m_nick		(user_t *, int, char **);
int	m_mode		(user_t *, int, char **);
int	m_kick		(user_t *, int, char **);
int	m_kill		(user_t *, int, char **);
int	m_quit		(user_t *, int, char **);
int	m_motd		(user_t *, int, char **);
int	m_trace		(user_t *, int, char **);
int	m_ping		(user_t *, int, char **);
int	m_version	(user_t *, int, char **);
int	m_eob		(user_t *, int, char **);
int	m_error		(user_t *, int, char **);
int	m_pong		(user_t *, int, char **);
u_short	pm_help		(user_t *, char *);
