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
 *   $Id: flags.h,v 1.6 2002/10/02 02:56:15 wiz Exp $
 */

#include "irc_types.h"

static struct privs {
	char flag;
	u_short privs;
} privs[] = {
	{ 'A',		ADMIN		},
	{ 'C',		CADMIN		},
	{ 'O',		LEET		},
	{ (u_char)NULL,	(u_short)NULL	}
};
