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
 *  $Id: dcc.h,v 1.3 2002/10/02 17:38:45 wiz Exp $
 */

#include "client.h"
#include "net.h"

void		parse_dcc		(sock_t *, char *);
void		greet_dcc		(sock_t *);
u_short		dcc_chat_conn		(user_t *, char *, char *);
sock_t *	dcc_chat_offer		(user_t *);
u_short		dcc_privs		(sock_t *, u_short);
void		cleanup_dcc		(void);
