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
 *  $Id: replies.h,v 1.5 2002/07/04 02:10:01 wiz Exp $
 */

static char *replies[] = {
#define	ERR_NOACCESS		0
	"Permission denied.",
#define	ERR_NOSUCHCMD		1
	"Unknown command, try /msg %s help",
#define	RPL_VERSION		2
	"\1VERSION %s by %s\1",
#define	RPL_PFORCESNUG		3
	"\2ALERT!\2 %s!%s@%s \2forcefully\2 removed your snuggle for %s!%s@%s$%s",
#define	RPL_CFORCESNUG		4
	"%s!%s@%s \2forcefully\2 removed SNUGGLE(%d) for %s!%s@%s$%s (%s)",
#define	RPL_CREMSNUGGLE		5
	"%s removed SNUGGLE(%d) for %s!%s@%s$%s (%s)",
#define	RPL_CADDSNUGGLE		6
	"%s added SNUGGLE(%d) for %s!%s@%s$%s (%s)",
#define	ERR_NOHELPCMD		7
	"Sorry, there is no help for that command.",
#define	ERR_NOSNUGGLES		8
	"There are no SNUGGLE records in my database!",
#define	RPL_SNUGGLECNT		9
	"There are currently %d SNUGGLE records in my database.",
#define	RPL_SNUGLISTA		10
	"%d) %s!%s@%s$%s (%s) [%d matches]",
#define	RPL_SNUGLISTB		11
	"    %24.24s by %s (type %d)",
#define	RPL_ENDOFSNUGL		12
	"End of SNUGGLE LIST.",
#define	ERR_NOMATCHSNUG		13
	"No SNUGGLE records matched your request.",
#define	RPL_CRECNTSNUG		14
	"There are %d SNUGGLE records that matched your request.",
#define	ERR_SNUGIDA		15
	"Invalid SNUGGLE ID.",
#define	ERR_SNUGIDB		16
	"No SNUGGLE records matched the given ID.",
#define	ERR_TOOMANYSNUG		17
	"There are only %d SNUGGLE records in my database!",
#define	ERR_SYNTAX		18
	"Syntax error.",
#define	ERR_NOSUCHCHAN	19
	"Channel %s is empty.",
#define	RPL_HELP_A		20
	"For more help with a specific command, \2/msg %s HELP <command>\2",
#define	ERR_BADPASSWD	21
	"Incorrect password.",
#define	RPL_IDENTIFIED	22
	"You are now identified.",
#ifndef	NULL
#define	NULL 0
#endif
	NULL
};
