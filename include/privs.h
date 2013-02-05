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
 *  $Id: privs.h,v 1.10 2002/07/13 20:37:39 wiz Exp $
 */

/* we're working with an unsigned short 16 bit integer */
#define NOPRIV		0x0000
#define NOKILL		0x0001
#define SERVER		0x0002
#define SNOTICE		0x0004
#define OPER		0x0008
#define AUTH		0x0010
#define OS_AUTH		0x0020
#define LEET		0x0040
#define CADMIN		0x0080
#define ADMIN		0x0100

/* macros that work with above flags */
#define IsAdmin(x)	((x->privs & (OPER|ADMIN)) == (OPER|ADMIN))
#define IsSetAdmin(x)	(x->privs & ADMIN)
#define IsCoAdmin(x)	(IsSetCoAdmin(x) && IsOper(x))
#define IsSetCoAdmin(x)	(x->privs & (CADMIN|ADMIN))
#define IsLeet(x)	(x->privs & (LEET|CADMIN|ADMIN) && x->privs & OPER)
#define IsOper(x)	(x->privs & OPER)
#define IsServNotice(x)	((x->privs & (SNOTICE|OPER)) == (SNOTICE|OPER))
#define IsServer(x)	(x->privs & SERVER)
#define IsPerson(x)	(!(x->privs & SERVER))
#define NoKill(x)	(x->privs & NOKILL)
#define	IsAuth(x)	(x->privs & AUTH)
#define	SetAuth(x)	(x->privs |= AUTH)
#define	ClearAuth(x)	(x->privs &= ~AUTH)
#define	IsOSAuth(x)	(x->privs & OS_AUTH)
#define	SetOSAuth(x)	(x->privs |= OS_AUTH)
#define	ClearOSAuth(x)	(x->privs &= ~OS_AUTH)
