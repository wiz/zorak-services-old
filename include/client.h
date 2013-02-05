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
 *  $Id: client.h,v 1.2 2002/10/02 02:56:15 wiz Exp $
 */

#ifndef __CLIENT_H_
#define __CLIENT_H_
#include "sys.h"
#ifdef	HAVE_LIBCRYPTO
#include <openssl/rsa.h>
#endif
#include "irc_types.h"

typedef struct user {
	struct user *next;
	struct user *prev;
	char *nick;
	char *username;
	char *host;
	char *realname;
	struct user *server;
	time_t ts;
	u_short flood;
	u_short privs;
#ifdef	HAVE_LIBCRYPTO
	char *response;
#endif
} user_t;

user_t *	add_user_to_list	(void);
void		del_user_from_list	(user_t *);
user_t *	add_user		(char *, char *, char *, char *, char *, int);
void		add_linked_server	(char *, char *, char *);
void		del_user		(user_t *);
u_short		check_privs		(char *);
int		count_users		(void);
int		change_user_nick	(char *, char *);
user_t *	find_client		(char *);
short		add_privs		(user_t *, u_short);
short		del_privs		(user_t *, u_short);
int		check_flood		(user_t *);
int		tell_user_hash		(char *);
void		del_all_users		(void);
int		del_server		(user_t *);
int		gline			(user_t *, char *, char *, int, char *, u_short);
void		jupe			(char *, char *);
void		redo_privs		(void);
#endif
