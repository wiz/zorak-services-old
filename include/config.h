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
 * $Id: config.h,v 1.14 2002/07/13 20:37:39 wiz Exp $
 */

#define	CLONE_URL	"http://www.newnet.net/clones.php"

#define LOGFILE		"services.log"
#define	PIDFILE		"services.pid"
#define CONF		"services.conf"

#define DEBUGMODE

/* all clone checking is done PER HOST, that is, *@hostname */
#define	CLONE_LIMIT			3	/* everyone is allowed this many clones */
#define	CLONE_GLINE			7	/* services will GLINE the host at this many clones, MUST BE > CLONE_LIMIT */
#define	CLONE_GLINE_TIME	1	/* how many hours services will make the above GLINE for */
#define	CLONE_OVER			3	/* clone exempted hosts will be GLINED if they go this many clones over their limit */
#define	FCLONE_GLINE_TIME	12	/* if someone brings flood clones on very fast, GLINE their host for this many hours */
/* sanity checks */
#define MAXKILL				64	/* max number of clients that can be matched in a GLINE, ANONGLINE, or WHOKILL */
