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
 *  $Id: msg.h,v 1.35 2002/10/02 02:56:15 wiz Exp $
 */

#include "irc_types.h"
#include "config.h"
#include "parse.h"
#include "help.h"

static struct command {
        char *cmd;
        int (*func)();
} cmdtab[] = {
        { "NICK",	m_nick		},
	{ "JOIN",	m_join		},
        { "PART",	m_part		},
        { "QUIT",	m_quit		},
        { "MODE",	m_mode		},
	{ "SJOIN",	m_sjoin		},
        { "PRIVMSG",	m_privmsg	},
        { "KILL",	m_kill		},
        { "KICK",	m_kick		},
	{ "SERVER",	m_server	},
	{ "SQUIT",	m_squit		},
        { "ADMIN",	m_admin		},
        { "WHOIS",	m_whois		},
	{ "STATS",	m_stats		},
	{ "NOTICE",	m_notice	},
        { "PING",	m_ping		},
        { "PONG",	m_pong		},
        { "MOTD",	m_motd		},
	{ "TRACE",	m_trace		},
        { "VERSION",	m_version	},
	{ "EOB",	m_eob		},
	{ "ERROR",	m_error		},
        { NULL,		NULL		}
};

#include "operserv.h"
#include "nickserv.h"
#include "chanserv.h"

static struct message {
	char *msg;
	u_short privs;
	u_short snick;
	u_short (*func)(user_t *, char *);
	void (*help)(user_t *, u_short);
} msgtab[] = {
	{ "HELP",	NOPRIV,		ALL,	pm_help,	NULL		},
	{ "REGISTER",	OPER|LEET,	NS,	ns_register,	ns_register_help},
	{ "IDENTIFY",	NOPRIV,		NS,	ns_identify,	ns_identify_help},
	{ "DROP",	NOPRIV,		NS,	ns_drop,	ns_drop_help	},
	{ "INFO",	NOPRIV,		NS,	ns_info,	NULL		},
	{ "LIST",	OPER,		NS,	ns_list,	NULL		},
	{ "LIST",	OPER,		CS,	cs_list,	cs_list_help	},
	{ "WHO",	OPER,		CS,	cs_who,		cs_who_help	},
	{ "CHAT",	OPER,		OS,	os_chat,	NULL		},
	{ "STATUS",	OPER,		OS,	os_status,	NULL		},
	{ "CLONES",	OPER,		OS,	os_clones,	NULL		},
	{ "KILLCLONES",	OPER,		OS,	os_killclones,	NULL		},
	{ "ACLONES",	OPER,		OS,	os_aclones,	NULL		},
	{ "MODE",	OPER,		OS,	os_mode,	os_mode_help	},
	{ "KICK",	OPER,		OS,	os_kick,	os_kick_help	},
	{ "KILL",	OPER,		OS,	os_kill,	os_kill_help	},
	{ "SETTIME",	OPER,		OS,	os_settime,	NULL		},
	{ "PLACEHOLD",	OPER,		OS,	os_placehold,	NULL		},
	{ "KLINE",	OPER,		OS,	os_kline,	NULL		},
	{ "GLINE",	OPER,		OS,	os_gline,	os_gline_help	},
	{ "ANONGLINE",	OPER,		OS,	os_anongline,	os_gline_help	},
	{ "SNUGGLE",	OPER,		OS,	os_snuggle,	os_snuggle_help	},
	{ "ACLONE",	OPER,		OS,	os_aclone,	os_aclone_help	},
	{ (char *)NULL,	(u_short)NULL,	0,	NULL,		NULL		}
};

static struct chancmd {
	char *msg;
	void (*func)(user_t *, char *, char *);
} chantab[] = {
	{ "CLONES",	chan_clones	},
	{ "KILLCLONES",	chan_killclones	},
	{ "STATUS",	chan_status	},
	{ NULL,		NULL		}
};
