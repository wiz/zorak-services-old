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
 * $Id: db.h,v 1.2 2002/06/23 22:20:37 wiz Exp $
 */

static char *databases[] = {
#ifndef	NULL
#define	NULL		0
#endif
	NULL,
#define	DB_SNUGGLE	1
	"db/snuggle.db",
#define	DB_ACLONE	2
	"db/aclone.db",
#define	DB_NICK		3
	"db/nick.db",
#define	DB_CHAN		4
	"db/chan.db",
#define	DB_MEMO		5
	"db/memo.db",
	NULL
};
