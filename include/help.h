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
 *  $Id: help.h,v 1.1 2002/07/13 20:37:39 wiz Exp $
 */

#include "irc_types.h"
#include "client.h"

u_short		help			(user_t *, u_short, u_short);

void		os_identify_help	(user_t *, u_short);
void		os_aclone_help		(user_t *, u_short);
void		os_mode_help		(user_t *, u_short);
void		os_kick_help		(user_t *, u_short);
void		os_kill_help		(user_t *, u_short);
void		os_who_help		(user_t *, u_short);
void		os_whokill_help		(user_t *, u_short);
void		os_gline_help		(user_t *, u_short);
void		os_snuggle_help		(user_t *, u_short);
void		os_jupe_help		(user_t *, u_short);

void		cs_list_help		(user_t *, u_short);
void		cs_who_help		(user_t *, u_short);

void		ns_register_help	(user_t *, u_short);
void		ns_identify_help	(user_t *, u_short);
void		ns_drop_help		(user_t *, u_short);
