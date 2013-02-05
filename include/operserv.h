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
 *  $Id: operserv.h,v 1.3 2002/10/02 02:56:15 wiz Exp $
 */

u_short	os_chat		(user_t *, char *);
u_short	os_status	(user_t *, char *);
u_short	os_clones	(user_t *, char *);
u_short	os_killclones	(user_t *, char *);
u_short	os_aclones	(user_t *, char *);
u_short	os_mode		(user_t *, char *);
u_short	os_kick		(user_t *, char *);
u_short	os_kill		(user_t *, char *);
u_short	os_settime	(user_t *, char *);
u_short	os_placehold	(user_t *, char *);
u_short	os_kline	(user_t *, char *);
u_short	os_gline	(user_t *, char *);
u_short	os_anongline	(user_t *, char *);
u_short	os_snuggle	(user_t *, char *);
u_short	os_aclone	(user_t *, char *);
void	chan_status	(user_t *, char *, char *);
void	chan_clones	(user_t *, char *, char *);
void	chan_killclones	(user_t *, char *, char *);
