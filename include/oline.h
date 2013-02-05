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
 *  $Id: oline.h,v 1.1 2002/07/13 20:37:39 wiz Exp $
 */

#ifndef __INCLUDE_OLINE_H_
#define __INCLUDE_OLINE_H_
#include "irc_types.h"
#include "client.h"
#include "sys.h"
#ifdef	HAVE_LIBCRYPTO
#include <openssl/rsa.h>
#endif

typedef struct oline {
	struct oline *next;
	char *um;
	char *hm;
	char *nick;
	u_short privs;
	char *passwd;
#ifdef	HAVE_LIBCRYPTO
	char *rsa_public_key_file;
	RSA *rsa_public_key;
#endif
} oline_t;

void		add_oline		(char *, char *, char *, int);
oline_t *	find_oline		(char *, char *);
oline_t *	find_oline_by_nick	(char *);
char *		find_oper_nick		(user_t *);
int		match_oline		(user_t *);
void		del_all_olines		(void);
u_short		stats_o			(char *);
u_short		has_oline		(user_t *);
#endif
