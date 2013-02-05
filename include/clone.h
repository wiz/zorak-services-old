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
 *  $Id: clone.h,v 1.1 2002/07/13 20:37:39 wiz Exp $
 */

#include "irc_types.h"
#include "client.h"

typedef struct clone {
	struct clone *next;
	char *host;
	u_short clones;
} clone_t;

int		check_clone		(char *, char *);
void		add_clone_host		(char *);
int		del_clone_host		(char *);
int		del_clone_user		(user_t *);
void		del_all_clones		(void);
clone_t *	find_clone_host		(char *);
int		list_clones		(char *);
int		kill_clones		(void);
void		add_clone_allow		(char *, int);
int		del_clone_allow		(char *);
void		del_all_clone_allows	(void);
clone_t *	find_clone_allow	(char *);
int		tell_clone_allows	(char *);
