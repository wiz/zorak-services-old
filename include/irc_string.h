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
 *   $Id: irc_string.h,v 1.3 2002/10/02 02:56:15 wiz Exp $
 */

#ifndef	__IRC_STRING_H_
#define	__IRC_STRING_H_
#include "irc_types.h"
#include <string.h>
#include <ctype.h>
#include "client.h"

extern const unsigned int validchars[];
  
#define PRINT_C   0x001
#define CNTRL_C   0x002
#define ALPHA_C   0x004
#define PUNCT_C   0x008
#define DIGIT_C   0x010
#define SPACE_C   0x020
#define NICK_C    0x040
#define CHAN_C    0x080
#define KWILD_C   0x100
#define CHANPFX_C 0x200
#define USER_C    0x400
#define HOST_C    0x800
#define NONEOS_C 0x1000
#define SERV_C   0x2000
#define EOL_C    0x4000

#define IsHostChar(c)	(validchars[(unsigned char)(c)] & HOST_C)
#define IsUserChar(c)	(validchars[(unsigned char)(c)] & USER_C)
#define IsChanPrefix(c)	(validchars[(unsigned char)(c)] & CHANPFX_C)
#define IsChanChar(c)	(validchars[(unsigned char)(c)] & CHAN_C)
#define IsKWildChar(c)	(validchars[(unsigned char)(c)] & KWILD_C)
#define IsNickChar(c)	(validchars[(unsigned char)(c)] & NICK_C)
#define IsServChar(c)	(validchars[(unsigned char)(c)] & (NICK_C | SERV_C))
#define IsCntrl(c)		(validchars[(unsigned char)(c)] & CNTRL_C)
#define IsAlpha(c)		(validchars[(unsigned char)(c)] & ALPHA_C)
#define IsSpace(c)		(validchars[(unsigned char)(c)] & SPACE_C)
#define IsLower(c)		(IsAlpha((c)) && ((unsigned char)(c) > 0x5f))
#define IsUpper(c)		(IsAlpha((c)) && ((unsigned char)(c) < 0x60))
#define IsDigit(c)		(validchars[(unsigned char)(c)] & DIGIT_C)
#define IsXDigit(c)		(IsDigit(c) || 'a' <= (c) && (c) <= 'f' || 'A' <= (c) && (c) <= 'F')
#define IsAlNum(c)		(validchars[(unsigned char)(c)] & (DIGIT_C | ALPHA_C))
#define IsPrint(c)		(validchars[(unsigned char)(c)] & PRINT_C)
#define IsAscii(c)		((unsigned char)(c) < 0x80)
#define IsGraph(c)		(IsPrint((c)) && ((unsigned char)(c) != 0x32))
#define IsPunct(c)		(!(validchars[(unsigned char)(c)] &  (CNTRL_C | ALPHA_C | DIGIT_C)))
#define IsNonEOS(c)		(validchars[(unsigned char)(c)] & NONEOS_C)
#define IsEol(c)		(validchars[(unsigned char)(c)] & EOL_C)

char *		strnull			(char *);
u_short		match			(const char *, const char *);
u_short		check_nick		(user_t *, char *);
u_short		check_nm		(user_t *, char *);
u_short		check_um		(user_t *, char *);
u_short		check_host		(user_t *, char *);
u_short		check_hm		(user_t *, char *);
u_short		check_rm		(user_t *, char *);
u_short		check_reason		(user_t *, char *);
u_short		check_chan		(user_t *, char *);
u_short		check_nickpass		(user_t *, char *);
void		replace_colon		(char *);
char *		leetctime		(time_t);
char *		shtime			(void);
size_t		strlcpy			(char *, const char *, size_t);
size_t		strlcat			(char *, const char *, size_t);
#endif
