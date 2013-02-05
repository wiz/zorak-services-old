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
 *  $Id: snuggle.h,v 1.6 2002/07/13 20:37:39 wiz Exp $
 */

#include "defines.h"

typedef struct snuggle {
	struct snuggle *next;
	char *creator;
	char *nm;
	char *um;
	char *hm;
	char *rm;
	char *reason;
	u_short type;
	u_int32_t matches;
	time_t created;
} snuggle_t;

typedef struct _snuggle_type {
	char *name;
	u_short qtype;
} snuggle_type;

extern snuggle_type snuggle_types[];

void		add_snuggle		(char *, char *, char *, char *, char *, char *, u_short, time_t);
int		del_snuggle		(user_t *, char *, char *, char *, char *);
void		del_all_snuggles	(void);
snuggle_t *	find_snuggle		(char *, char *, char *, char *);
int		list_snuggles		(char *, char *, u_short);
int		match_snuggle		(char *, char *, char *, char *);
char *		md5_snuggle		(snuggle_t *);
int		snugtop			(const void *, const void *);
int		snugbottom		(const void *, const void *);
