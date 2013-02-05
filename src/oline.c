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
 */

#include "irc_types.h"
#include "irc_string.h"
#include "defines.h"
#include "oline.h"
#include "privs.h"
#include "flags.h"
#include "send.h"
#include "mem.h"
#include "rsa.h"
#include "me.h"
#ifdef	HAVE_LIBCRYPTO
#include <openssl/rsa.h>
#endif

WIZOSID("$Id: oline.c,v 1.18 2002/10/02 02:56:20 wiz Exp $");

oline_t *main_oline = NULL;

void
add_oline(char *hm, char *nick, char *passwd, int privs)
{
	oline_t *oline;

	oline = leetcalloc(sizeof(oline_t), 1);
	oline->next = main_oline;
	main_oline = oline;

	oline->um = leetstrdup(strtok(hm, "@"));
	oline->hm = leetstrdup(strtok(NULL, ""));
	oline->nick = leetstrdup(nick);
	oline->privs = privs;
#ifdef	HAVE_LIBCRYPTO
	if (*passwd == '/') {
		oline->rsa_public_key_file = leetstrdup(passwd);
		read_public_key(oline);
		return;
	}
#endif
	oline->passwd = leetstrdup(passwd);
}

oline_t *
find_oline(char *um, char *hm)
{
	oline_t *oline = main_oline;

	for (; oline; oline = oline->next)
		if (match(oline->um, um) && match(oline->hm, hm))
			return oline;
	return NULL;
}

oline_t *
find_oline_by_nick(char *nick)
{
	oline_t *oline = main_oline;

	for (; oline; oline = oline->next)
		if (strcasecmp(oline->nick, nick) == 0)
			return oline;
	return NULL;
}

char *
find_oper_nick(user_t *cptr)
{
	oline_t *oline = find_oline(cptr->username, cptr->host);
	if (!oline)
		return NULL;
	return oline->nick;
}

u_short
has_oline(user_t *cptr)
{
	oline_t *oline = main_oline;

	for (; oline; oline = oline->next)
		if (match(oline->um, cptr->username) && match(oline->hm, cptr->host))
			return 1;
	return 0;
}

int
match_oline(user_t *cptr)
{
	oline_t *oline = main_oline;

	for (; oline; oline = oline->next)
		if (match(oline->um, cptr->username) && match(oline->hm, cptr->host))
			return add_privs(cptr, oline->privs);
	return 0;
}

void
del_all_olines(void)
{
	extern sock_t *main_sock;
	sock_t *sock = main_sock;
	oline_t *tmp, *all = main_oline;

	for (; sock; sock = sock->next) {
		sock->oline = NULL;
		sock->flags &= ~(LEET|CADMIN|ADMIN);
	}
	while ((tmp = all)) {
		leetfree(tmp->um, strlen(tmp->um) + 1);
		leetfree(tmp->hm, strlen(tmp->hm) + 1);
		leetfree(tmp->nick, strlen(tmp->nick) + 1);
#ifdef	HAVE_LIBCRYPTO
		if (tmp->rsa_public_key_file)
			leetfree(tmp->rsa_public_key_file, strlen(tmp->rsa_public_key_file) + 1);
		if (tmp->rsa_public_key)
			RSA_free(tmp->rsa_public_key);
#endif
		all = tmp->next;
		leetfree(tmp, sizeof(oline_t));
	}
	main_oline = NULL;
}

u_short
stats_o(char *from)
{
	oline_t *oline = main_oline;
	struct privs *pptr;

	for (; oline; oline = oline->next)
	{
		for (pptr = privs; pptr; pptr++)
			if (oline->privs == pptr->privs)
				goto found;
		continue;
found:		toserv(":%s 243 %s O %s@%s * %s %c\r\n", me.servname, from, oline->um, oline->hm, oline->nick, pptr->flag);
		toserv(":%s 219 %s o :End of /STATS report\r\n", me.servname, from);
	}
	return 1;
}
