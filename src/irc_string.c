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

#include <assert.h>

#include "irc_types.h"
#include "irc_string.h"
#include "irc_time.h"
#include "defines.h"
#include "send.h"

WIZOSID("$Id: irc_string.c,v 1.8 2002/10/02 02:56:20 wiz Exp $");

#ifndef HAVE_STRLCPY
size_t
strlcpy(char *dst, const char *src, size_t siz)
{
	register char *d = dst;
	register const char *s = src;
	register size_t n = siz;

	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}

	if (n == 0) {
		if (siz != 0)
			*d = '\0';
		while (*s++)
			;
	}

	return(s - src - 1);
}
#endif
#ifndef HAVE_STRLCAT
size_t
strlcat(char *dst, const char *src, size_t siz)
{
	register char *d = dst;
	register const char *s = src;
	register size_t n = siz;
	size_t dlen;

	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return(dlen + (s - src));
}
#endif

char *
strnull(char *stuff)
{
	return (stuff ? stuff : "(null)");
}

/* match() originally written by Douglas A Lewis (dalewis@acsu.buffalo.edu) */
u_short
match(const char *mask, const char *name)
{
	const unsigned char *m = (const unsigned char *)mask;
	const unsigned char *n = (const unsigned char *)name;
	const unsigned char *ma = (const unsigned char *)mask;
	const unsigned char *na = (const unsigned char *)name;
	u_short wild = 0;

	assert(0 != mask);
	assert(0 != name);
loopy:
	if (*m == '*') {
		while (*m == '*')
			m++;
		wild = 1;
		ma = m;
		na = n;
	}
	if (!*m) {
		if (!*n)
			return 1;
		for (m--; (m > (const unsigned char *)mask) && (*m == '?'); m--) ;
		if (*m == '*' && (m > (const unsigned char *) mask))
			return 1;
		if (!wild)
			return 0;
		m = ma;
		n = ++na;
	} else if (!*n) {
		while (*m == '*')
			m++;
		return (*m == 0);
	}
	if (tolower(*m) != tolower(*n) && *m != '?') {
		if (!wild)
			return 0;
		m = ma;
		n = ++na;
	} else {
		if (*m)
			m++;
		if (*n)
			n++;
	}
	goto loopy;
}

u_short
check_nick(user_t *cptr, char *nick)
{
	if (strlen(nick) > NICKLEN)
		return (cptr ? reply(OS, cptr->nick, "The nickname is too long.") : 1);
	if (isdigit(*nick) || *nick == '-')
		return (cptr ? reply(OS, cptr->nick, "Erroneus Nickname.") : 1);
	for (; *nick; nick++)
		if (!IsNickChar(*nick))
			return (cptr ? reply(OS, cptr->nick, "The character '%c' is not allowed in your nick.", *nick) : 1);
	return 0;
}

u_short
check_nm(user_t *cptr, char *nm)
{
	if (strlen(nm) > NICKLEN)
		return (cptr ? reply(OS, cptr->nick, "The nickmask is too long.") : 1);
	for (; *nm; nm++)
		if (!IsNickChar(*nm) && *nm != '?' && *nm != '*')
			return (cptr ? reply(OS, cptr->nick, "The character '%c' is not allowed in your nickmask.", *nm) : 1);
	return 0;
}

u_short
check_um(user_t *cptr, char *um)
{
	if (strlen(um) > USERLEN)
		return (cptr ? reply(OS, cptr->nick, "The usermask is too long.") : 1);
	for (; *um; um++)
		if (!IsUserChar(*um) && *um != '*' && *um != '?')
			return (cptr ? reply(OS, cptr->nick, "The character '%c' is not allowed in your usermask.", *um) : 1);
	return 0;
}

u_short
check_host(user_t *cptr, char *hm)
{
	if (strlen(hm) > HOSTLEN)
		return (cptr ? reply(OS, cptr->nick, "Your host is too long.") : 1);
	if (!strchr(hm, '.'))
		return (cptr ? reply(OS, cptr->nick, "Your host must contain at least one period.") : 1);
	for (; *hm; hm++)
		if (!IsHostChar(*hm))
			return (cptr ? reply(OS, cptr->nick, "The %s '%c' is not allowed in your hostmask.", ((*hm == '*' || *hm == '?') ? "wildcard" : "character"), *hm) : 1);
	return 0;
}

u_short
check_hm(user_t *cptr, char *hm)
{
	if (strlen(hm) > HOSTLEN)
		return (cptr ? reply(OS, cptr->nick, "The hostmask is too long.") : 1);
	for (; *hm; hm++)
		if (!IsHostChar(*hm) && *hm != '*' && *hm != '?')
			return (cptr ? reply(OS, cptr->nick, "The character '%c' is not allowed in your hostmask.", *hm) : 1);
	return 0;
}

u_short
check_rm(user_t *cptr, char *rm)
{
	if (strlen(rm) > REALLEN)
		return (cptr ? reply(OS, cptr->nick, "The realname mask is too long.") : 1);
	for (; *rm; rm++)
		if (*rm == ':' || *rm == '#')
			return (cptr ? reply(OS, cptr->nick, "The character '%c' is not allowed in your realname mask.", *rm) : 1);
	return 0;
}

u_short
check_chan(user_t *cptr, char *chan)
{
	if (strlen(chan) > CHANLEN)
		return (cptr ? reply(CS, cptr->nick, "That is too long for a channel name.") : 1);
	if (!IsChanPrefix(*chan))
		return (cptr ? reply(CS, cptr->nick, "That is not a valid channel name.") : 1);
	for (; *chan; chan++)
		if (!IsChanChar(*chan))
			return (cptr ? reply(CS, cptr->nick, "The character '%c' is not allowed in a channel name.", *chan) : 1);
	return 0;
}

u_short
check_reason(user_t *cptr, char *reason)
{
	if (strlen(reason) > KILLLEN)
		return (cptr ? reply(OS, cptr->nick, "The reason is too long.") : 1);
	for (; *reason; reason++)
		if (*reason == ':' || *reason == '#')
			return (cptr ? reply(OS, cptr->nick, "The character '%c' is not allowed in your reason.", *reason) : 1);
	return 0;
}

u_short
check_nickpass(user_t *cptr, char *args)
{
	if (!args || strchr(args, ' '))
		return NEED_HELP;
	if (strlen(args) < 5)
		return (cptr ? reply(NS, cptr->nick, "Your password must be at least 5 characters.") : 1);
	if (strlen(args) > 25)
		return (cptr ? reply(NS, cptr->nick, "Your password may not be longer than 25 characters.") : 1);
	return 0;
}

void
replace_colon(char *in)
{
	for (; *in; in++)
		if (*in == ':')
			*in = ';';
}

char *
leetctime(time_t lclock)
{
	static char buf[25];

	strlcpy(buf, ctime(&lclock), 25);
	return buf;
}

char *
shtime(void)
{
	static char buf[9];
#ifdef SECONDS_IN_TIMESTAMP
	strlcpy(buf, leetctime(time(NULL)) + 11, 9);
#else
	strlcpy(buf, leetctime(time(NULL)) + 11, 6);
#endif
	return buf;
}

const unsigned int validchars[] = {
/* 0  */     CNTRL_C,
/* 1  */     CNTRL_C|CHAN_C|NONEOS_C,
/* 2  */     CNTRL_C|CHAN_C|NONEOS_C,
/* 3  */     CNTRL_C|CHAN_C|NONEOS_C,
/* 4  */     CNTRL_C|CHAN_C|NONEOS_C,
/* 5  */     CNTRL_C|CHAN_C|NONEOS_C,
/* 6  */     CNTRL_C|CHAN_C|NONEOS_C,
/* 7 BEL */  CNTRL_C|NONEOS_C,
/* 8  \b */  CNTRL_C|CHAN_C|NONEOS_C,
/* 9  \t */  CNTRL_C|SPACE_C|CHAN_C|NONEOS_C,
/* 10 \n */  CNTRL_C|SPACE_C|CHAN_C|NONEOS_C|EOL_C,
/* 11 \v */  CNTRL_C|SPACE_C|CHAN_C|NONEOS_C,
/* 12 \f */  CNTRL_C|SPACE_C|CHAN_C|NONEOS_C,
/* 13 \r */  CNTRL_C|SPACE_C|CHAN_C|NONEOS_C|EOL_C,
/* 14 */     CNTRL_C|CHAN_C|NONEOS_C,
/* 15 */     CNTRL_C|CHAN_C|NONEOS_C,
/* 16 */     CNTRL_C|CHAN_C|NONEOS_C,
/* 17 */     CNTRL_C|CHAN_C|NONEOS_C,
/* 18 */     CNTRL_C|CHAN_C|NONEOS_C,
/* 19 */     CNTRL_C|CHAN_C|NONEOS_C,
/* 20 */     CNTRL_C|CHAN_C|NONEOS_C,
/* 21 */     CNTRL_C|CHAN_C|NONEOS_C,
/* 22 */     CNTRL_C|CHAN_C|NONEOS_C,
/* 23 */     CNTRL_C|CHAN_C|NONEOS_C,
/* 24 */     CNTRL_C|CHAN_C|NONEOS_C,
/* 25 */     CNTRL_C|CHAN_C|NONEOS_C,
/* 26 */     CNTRL_C|CHAN_C|NONEOS_C,
/* 27 */     CNTRL_C|CHAN_C|NONEOS_C,
/* 28 */     CNTRL_C|CHAN_C|NONEOS_C,
/* 29 */     CNTRL_C|CHAN_C|NONEOS_C,
/* 30 */     CNTRL_C|CHAN_C|NONEOS_C,
/* 31 */     CNTRL_C|CHAN_C|NONEOS_C,
/* SP */     PRINT_C|SPACE_C,
/* ! */      PRINT_C|KWILD_C|CHAN_C|NONEOS_C,
/* " */      PRINT_C|CHAN_C|NONEOS_C,
/* # */      PRINT_C|CHANPFX_C|CHAN_C|NONEOS_C,
/* $ */      PRINT_C|CHAN_C|NONEOS_C|USER_C,
/* % */      PRINT_C|CHAN_C|NONEOS_C,
/* & */      PRINT_C|CHANPFX_C|CHAN_C|NONEOS_C,
/* ' */      PRINT_C|CHAN_C|NONEOS_C,
/* ( */      PRINT_C|CHAN_C|NONEOS_C,
/* ) */      PRINT_C|CHAN_C|NONEOS_C,
/* * */      PRINT_C|KWILD_C|CHAN_C|NONEOS_C|SERV_C,
/* + */      PRINT_C|CHAN_C|NONEOS_C,
/* , */      PRINT_C|NONEOS_C,
/* - */      PRINT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* . */      PRINT_C|KWILD_C|CHAN_C|NONEOS_C|USER_C|HOST_C|SERV_C,
/* / */      PRINT_C|CHAN_C|NONEOS_C,
/* 0 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* 1 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* 2 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* 3 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* 4 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* 5 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* 6 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* 7 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* 8 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* 9 */      PRINT_C|DIGIT_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* : */      PRINT_C|CHAN_C|NONEOS_C,
/* ; */      PRINT_C|CHAN_C|NONEOS_C,
/* < */      PRINT_C|CHAN_C|NONEOS_C,
/* = */      PRINT_C|CHAN_C|NONEOS_C,
/* > */      PRINT_C|CHAN_C|NONEOS_C,
/* ? */      PRINT_C|KWILD_C|CHAN_C|NONEOS_C,
/* @ */      PRINT_C|KWILD_C|CHAN_C|NONEOS_C,
/* A */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* B */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* C */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* D */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* E */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* F */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* G */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* H */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* I */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* J */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* K */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* L */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* M */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* N */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* O */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* P */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* Q */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* R */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* S */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* T */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* U */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* V */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* W */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* X */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* Y */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* Z */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* [ */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C,
/* \ */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C,
/* ] */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C,
/* ^ */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C,
/* _ */      PRINT_C|NICK_C|CHAN_C|NONEOS_C|USER_C,
/* ` */      PRINT_C|NICK_C|CHAN_C|NONEOS_C|USER_C,
/* a */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* b */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* c */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* d */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* e */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* f */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* g */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* h */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* i */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* j */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* k */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* l */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* m */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* n */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* o */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* p */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* q */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* r */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* s */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* t */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* u */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* v */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* w */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* x */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* y */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* z */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C|HOST_C,
/* { */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C,
/* | */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C,
/* } */      PRINT_C|ALPHA_C|NICK_C|CHAN_C|NONEOS_C|USER_C,
/* ~ */      PRINT_C|ALPHA_C|CHAN_C|NONEOS_C|USER_C,
/* del  */   CHAN_C|NONEOS_C,
/* 0x80 */   CHAN_C|NONEOS_C,
/* 0x81 */   CHAN_C|NONEOS_C,
/* 0x82 */   CHAN_C|NONEOS_C,
/* 0x83 */   CHAN_C|NONEOS_C,
/* 0x84 */   CHAN_C|NONEOS_C,
/* 0x85 */   CHAN_C|NONEOS_C,
/* 0x86 */   CHAN_C|NONEOS_C,
/* 0x87 */   CHAN_C|NONEOS_C,
/* 0x88 */   CHAN_C|NONEOS_C,
/* 0x89 */   CHAN_C|NONEOS_C,
/* 0x8A */   CHAN_C|NONEOS_C,
/* 0x8B */   CHAN_C|NONEOS_C,
/* 0x8C */   CHAN_C|NONEOS_C,
/* 0x8D */   CHAN_C|NONEOS_C,
/* 0x8E */   CHAN_C|NONEOS_C,
/* 0x8F */   CHAN_C|NONEOS_C,
/* 0x90 */   CHAN_C|NONEOS_C,
/* 0x91 */   CHAN_C|NONEOS_C,
/* 0x92 */   CHAN_C|NONEOS_C,
/* 0x93 */   CHAN_C|NONEOS_C,
/* 0x94 */   CHAN_C|NONEOS_C,
/* 0x95 */   CHAN_C|NONEOS_C,
/* 0x96 */   CHAN_C|NONEOS_C,
/* 0x97 */   CHAN_C|NONEOS_C,
/* 0x98 */   CHAN_C|NONEOS_C,
/* 0x99 */   CHAN_C|NONEOS_C,
/* 0x9A */   CHAN_C|NONEOS_C,
/* 0x9B */   CHAN_C|NONEOS_C,
/* 0x9C */   CHAN_C|NONEOS_C,
/* 0x9D */   CHAN_C|NONEOS_C,
/* 0x9E */   CHAN_C|NONEOS_C,
/* 0x9F */   CHAN_C|NONEOS_C,
/* 0xA0 */   CHAN_C|NONEOS_C,
/* 0xA1 */   CHAN_C|NONEOS_C,
/* 0xA2 */   CHAN_C|NONEOS_C,
/* 0xA3 */   CHAN_C|NONEOS_C,
/* 0xA4 */   CHAN_C|NONEOS_C,
/* 0xA5 */   CHAN_C|NONEOS_C,
/* 0xA6 */   CHAN_C|NONEOS_C,
/* 0xA7 */   CHAN_C|NONEOS_C,
/* 0xA8 */   CHAN_C|NONEOS_C,
/* 0xA9 */   CHAN_C|NONEOS_C,
/* 0xAA */   CHAN_C|NONEOS_C,
/* 0xAB */   CHAN_C|NONEOS_C,
/* 0xAC */   CHAN_C|NONEOS_C,
/* 0xAD */   CHAN_C|NONEOS_C,
/* 0xAE */   CHAN_C|NONEOS_C,
/* 0xAF */   CHAN_C|NONEOS_C,
/* 0xB0 */   CHAN_C|NONEOS_C,
/* 0xB1 */   CHAN_C|NONEOS_C,
/* 0xB2 */   CHAN_C|NONEOS_C,
/* 0xB3 */   CHAN_C|NONEOS_C,
/* 0xB4 */   CHAN_C|NONEOS_C,
/* 0xB5 */   CHAN_C|NONEOS_C,
/* 0xB6 */   CHAN_C|NONEOS_C,
/* 0xB7 */   CHAN_C|NONEOS_C,
/* 0xB8 */   CHAN_C|NONEOS_C,
/* 0xB9 */   CHAN_C|NONEOS_C,
/* 0xBA */   CHAN_C|NONEOS_C,
/* 0xBB */   CHAN_C|NONEOS_C,
/* 0xBC */   CHAN_C|NONEOS_C,
/* 0xBD */   CHAN_C|NONEOS_C,
/* 0xBE */   CHAN_C|NONEOS_C,
/* 0xBF */   CHAN_C|NONEOS_C,
/* 0xC0 */   CHAN_C|NONEOS_C,
/* 0xC1 */   CHAN_C|NONEOS_C,
/* 0xC2 */   CHAN_C|NONEOS_C,
/* 0xC3 */   CHAN_C|NONEOS_C,
/* 0xC4 */   CHAN_C|NONEOS_C,
/* 0xC5 */   CHAN_C|NONEOS_C,
/* 0xC6 */   CHAN_C|NONEOS_C,
/* 0xC7 */   CHAN_C|NONEOS_C,
/* 0xC8 */   CHAN_C|NONEOS_C,
/* 0xC9 */   CHAN_C|NONEOS_C,
/* 0xCA */   CHAN_C|NONEOS_C,
/* 0xCB */   CHAN_C|NONEOS_C,
/* 0xCC */   CHAN_C|NONEOS_C,
/* 0xCD */   CHAN_C|NONEOS_C,
/* 0xCE */   CHAN_C|NONEOS_C,
/* 0xCF */   CHAN_C|NONEOS_C,
/* 0xD0 */   CHAN_C|NONEOS_C,
/* 0xD1 */   CHAN_C|NONEOS_C,
/* 0xD2 */   CHAN_C|NONEOS_C,
/* 0xD3 */   CHAN_C|NONEOS_C,
/* 0xD4 */   CHAN_C|NONEOS_C,
/* 0xD5 */   CHAN_C|NONEOS_C,
/* 0xD6 */   CHAN_C|NONEOS_C,
/* 0xD7 */   CHAN_C|NONEOS_C,
/* 0xD8 */   CHAN_C|NONEOS_C,
/* 0xD9 */   CHAN_C|NONEOS_C,
/* 0xDA */   CHAN_C|NONEOS_C,
/* 0xDB */   CHAN_C|NONEOS_C,
/* 0xDC */   CHAN_C|NONEOS_C,
/* 0xDD */   CHAN_C|NONEOS_C,
/* 0xDE */   CHAN_C|NONEOS_C,
/* 0xDF */   CHAN_C|NONEOS_C,
/* 0xE0 */   CHAN_C|NONEOS_C,
/* 0xE1 */   CHAN_C|NONEOS_C,
/* 0xE2 */   CHAN_C|NONEOS_C,
/* 0xE3 */   CHAN_C|NONEOS_C,
/* 0xE4 */   CHAN_C|NONEOS_C,
/* 0xE5 */   CHAN_C|NONEOS_C,
/* 0xE6 */   CHAN_C|NONEOS_C,
/* 0xE7 */   CHAN_C|NONEOS_C,
/* 0xE8 */   CHAN_C|NONEOS_C,
/* 0xE9 */   CHAN_C|NONEOS_C,
/* 0xEA */   CHAN_C|NONEOS_C,
/* 0xEB */   CHAN_C|NONEOS_C,
/* 0xEC */   CHAN_C|NONEOS_C,
/* 0xED */   CHAN_C|NONEOS_C,
/* 0xEE */   CHAN_C|NONEOS_C,
/* 0xEF */   CHAN_C|NONEOS_C,
/* 0xF0 */   CHAN_C|NONEOS_C,
/* 0xF1 */   CHAN_C|NONEOS_C,
/* 0xF2 */   CHAN_C|NONEOS_C,
/* 0xF3 */   CHAN_C|NONEOS_C,
/* 0xF4 */   CHAN_C|NONEOS_C,
/* 0xF5 */   CHAN_C|NONEOS_C,
/* 0xF6 */   CHAN_C|NONEOS_C,
/* 0xF7 */   CHAN_C|NONEOS_C,
/* 0xF8 */   CHAN_C|NONEOS_C,
/* 0xF9 */   CHAN_C|NONEOS_C,
/* 0xFA */   CHAN_C|NONEOS_C,
/* 0xFB */   CHAN_C|NONEOS_C,
/* 0xFC */   CHAN_C|NONEOS_C,
/* 0xFD */   CHAN_C|NONEOS_C,
/* 0xFE */   CHAN_C|NONEOS_C,
/* 0xFF */   CHAN_C|NONEOS_C
};
