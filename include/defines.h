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
 *   $Id: defines.h,v 1.3 2002/10/02 17:38:45 wiz Exp $
 */

#include "serno.h"
#include "sys.h"

#define	VER_STR		"Zorak IRC Services ("SERIALNUM")"
#define	MYEMAIL		"wiz <jason@wiz.cx>"

#define	SERVTIMEOUT	240	/* timeout in seconds for select() while connected */
#define	CONNTIMEOUT	30	/* timeout in seconds for select() while connecting */
#define	MAXCHANS	12

#define BUFSIZE		513	/* rfc1459 says do not change this */
#define	NICKLEN		9
#define	CHANLEN		200
#define	USERLEN		10
#define	HOSTLEN 	63
#define	REALLEN		50
#define	TOPICLEN	120
#define	KILLLEN		90

#define MIN(a, b)	(a < b ? a : b)
#define MAX(b, a)	(a < b ? a : b)
#define S(i)		(i != 1 ? "s" : "")

/* thanks eggdrop */
#ifdef WORDS_BIGENDIAN
#  define swap_short(sh) (sh)
#  define swap_long(ln) (ln)
#else
#  define swap_short(sh) ((((sh) & 0xff00) >> 8) | (((sh) & 0x00ff) << 8))
#  define swap_long(ln) (swap_short(((ln)&0xffff0000)>>16) | (swap_short((ln)&0x0000ffff)<<16))
#endif
#define iptolong(a) (0xffffffff & (long)(swap_long((unsigned long)a)))

#define	SERV_IRCNN	1
#define	SERV_HYBRD	2

#define	ADD		1
#define	DEL		2

#define	SNUGGLE_ALL	1
#define	SNUGGLE_MASK	2
#define	SNUGGLE_CNT	3
#define	SNUGGLE_CRECNT	4
#define SNUGGLE_CREMASK	5
#define SNUGGLE_TOP	6
#define SNUGGLE_BOTTOM	7
#define	SNUGGLE_ID	8

#define	VOICE		1
#define	HALFOP		2
#define	OP		3

#define	NEED_HELP	2

#define SCI	0
#define OS	1
#define NS	2
#define CS	3
#define HS	4
#define MS	5
#define GN	6
#define ALL	7

#define	HELP_USAGE	0x10
#define	HELP_DESC	0x04

#define WIZOSID(x)	static const char rcsid[] __attribute__((__unused__)) = x
