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
 *  $Id: dcc.c,v 1.5 2002/10/11 04:19:19 wiz Exp $
 */

#include <errno.h>

#include "irc_types.h"
#include "irc_string.h"
#include "irc_time.h"
#include "defines.h"
#include "replies.h"
#include "client.h"
#include "conf.h"
#include "oline.h"
#include "privs.h"
#include "main.h" /* for log() */
#include "send.h"
#include "dcc.h"
#include "md5.h"
#include "mem.h"
#include "net.h"
#include "rsa.h"
#include "db.h"
#include "me.h"

u_short	dcc_status	(sock_t *, char *);
void	dcc_status_help	(sock_t *, u_short);
u_short	dcc_who		(sock_t *, char *);
void	dcc_who_help	(sock_t *, u_short);
u_short	dcc_whokill	(sock_t *, char *);
void	dcc_whokill_help(sock_t *, u_short);
u_short	dcc_me		(sock_t *, char *);
void	dcc_me_help	(sock_t *, u_short);
u_short	dcc_save	(sock_t *, char *);
void	dcc_save_help	(sock_t *, u_short);
u_short	dcc_jupe	(sock_t *, char *);
void	dcc_jupe_help	(sock_t *, u_short);
u_short	dcc_rehash	(sock_t *, char *);
void	dcc_rehash_help	(sock_t *, u_short);
u_short	dcc_reload	(sock_t *, char *);
void	dcc_reload_help	(sock_t *, u_short);
u_short	dcc_restart	(sock_t *, char *);
void	dcc_restart_help(sock_t *, u_short);
u_short	dcc_die		(sock_t *, char *);
void	dcc_die_help	(sock_t *, u_short);
u_short	dcc_raw		(sock_t *, char *);
void	dcc_raw_help	(sock_t *, u_short);
u_short	dcc_help	(sock_t *, char *);
u_short	dcc_quit	(sock_t *, char *);

static struct _dcctab {
	char *cmd;
	u_short privs;
	u_short (*func)(sock_t *, char *);
	void (*help)(sock_t *, u_short);
} dcctab[] = {
	{ "LOGOUT",	0,	dcc_quit,	NULL		},
	{ "EXIT",	0,	dcc_quit,	NULL		},
	{ "QUIT",	0,	dcc_quit,	NULL		},
	{ "HELP",	0,	dcc_help,	NULL		},
	{ "STATUS",	0,	dcc_status,	dcc_status_help	},
	{ "WHOM",	0,	dcc_status,	NULL		},
	{ "WHO",	0,	dcc_who,	dcc_who_help	},
	{ "WHOKILL",	0,	dcc_whokill,	dcc_whokill_help},
	{ "JUPE",	CADMIN,	dcc_jupe,	dcc_jupe_help	},
	{ "SAVE",	CADMIN,	dcc_save,	dcc_save_help	},
	{ "REHASH",	CADMIN,	dcc_rehash,	dcc_rehash_help	},
	{ "RELOAD",	CADMIN,	dcc_reload,	dcc_reload_help	},
	{ "RESTART",	CADMIN,	dcc_restart,	dcc_restart_help},
	{ "DIE",	ADMIN,	dcc_die,	dcc_die_help	},
	{ "RAW",	ADMIN,	dcc_raw,	dcc_raw_help	},
	{ NULL,		NULL,	NULL,		NULL		}
};

void
greet_dcc(sock_t *sock)
{
	tosock(sock, " \nWelcome to the %s interface for %s!\n", ((sock->flags & SOCK_DCC) ? "DCC" : "telnet"),
		me.servname);
	tosock(sock, "You are connecting from %s:%d and your connection will be logged.\n", inet_ntoa(sock->addr), sock->port);
	tosock(sock, " \nUsername\n");
}

void
parse_dcc(sock_t *sock, char *buffer)
{
	struct _dcctab *dptr = dcctab;
	oline_t *oline;
	u_short wp = 0;
	char *chal;

	if (!sock->name) {
		sock->name = leetstrdup(strtok(buffer, " "));
		wp = 1;
	}
	if (!(oline = sock->oline) && (!(sock->oline = oline = find_oline_by_nick(sock->name)) || dcc_privs(sock, oline->privs) == 0)) {
		tosock(sock, "I don't know who you are.\n");
		sock->flags |= SOCK_DEL;
		return;
	}
	if (!(sock->flags & SOCK_AUTH)) {
		if (wp) {
			if (oline->rsa_public_key && (chal = make_challenge(&sock->response, oline->rsa_public_key))) {
				tosock(sock, " \nChallenge:\n%s\n \nResponse:\n", chal);
				leetfree(chal, strlen(chal) + 1);
				return;
			} else {
				tosock(sock, " \nPassword %s\n", ((sock->flags & SOCK_TELNET) ? "\377\373\001 " : ""));
				return;
			}
		} else {
			if (oline->passwd && sock->flags & SOCK_TELNET) {
				tosock(sock, "\377\374\001\n");
				tosock(sock, "DEBUG: pass buf = %s (end)\n", buffer);
			}
			if ((sock->response && strcasecmp(sock->response, buffer) == 0) ||
					(oline->passwd && strcmp(oline->passwd, wiz_md5(buffer)) == 0)) {
				del_dcc(sock->name); /* remove their "ghost" login, if it exists */
				sock->flags |= SOCK_AUTH;
				tosock(sock, " \n");
				alldcc("*** %s has logged in.", sock->name);
				return;
			} else {
				tosock(sock, "Authentication failed.\n");
				sock->flags |= SOCK_DEL;
				alldcc("Failed login from %s as %s", inet_ntoa(sock->addr), sock->name);
				return;
			}
		}
	}
	if (*buffer == '.') {
		if (strlen(buffer) < 2)
			return;
		strtok(++buffer, " ");
		for (; dptr->cmd; dptr++)
			if (strcasecmp(dptr->cmd, buffer) == 0)
				goto privcheck;
		tosock(sock, "Unknown command.\n");
		return;
privcheck:	if (dptr->privs == 0 || (sock->flags & dptr->privs) == dptr->privs)
			goto floodcheck;
		tosock(sock, "%s\n", replies[ERR_NOACCESS]);
		return;
floodcheck:	/* XXX insert flood checking here */
		tosocks(CADMIN, "#%s# %s %s\n", sock->name, buffer, (chal = strtok(NULL, "")) ? chal : "");
		goto cmdcheck;
cmdcheck:	if ((*dptr->func)(sock, chal) == NEED_HELP && *dptr->help) {
			tosock(sock, "Usage: ");
			(*dptr->help)(sock, HELP_USAGE);
		}
	} else {
		dcc_partyline(sock, buffer);
	}
}

u_short
dcc_privs(sock_t *sock, u_short new_privs)
{
	if (new_privs & ADMIN)
		new_privs |= CADMIN;
	if (new_privs & CADMIN) {
		new_privs |= LEET;
		me.admins++;
	}
	return sock->flags |= new_privs;
}

sock_t *
dcc_chat_offer(user_t *cptr)
{
	sock_t *sock = listen_sock(0);

	toserv(":%s PRIVMSG %s :\1DCC CHAT chat %lu %d\1\r\n", me.sclients[SCI].nick, cptr->nick, iptolong((u_int)me.ip.s_addr), sock->port);
	return sock;
}


/* dcc commands */


u_short
dcc_help(sock_t *sock, char *args)
{
	struct _dcctab *dptr = dcctab;

	if (args) {
		strtok(args, " ");
		for (; dptr->cmd; dptr++)
			if (strcasecmp(dptr->cmd, args) == 0 && *dptr->help) {
				tosock(sock, "Usage: ");
				(*dptr->help)(sock, HELP_USAGE|HELP_DESC);
				return 1;
			}
		tosock(sock, "No help on that command. Try .help for a complete list of all commands.\n");
	} else {
		for (; dptr->cmd; dptr++)
			if (*dptr->help) {
				write(sock->socket, dptr->cmd, strlen(dptr->cmd));
				write(sock->socket, "               ", 15 - strlen(dptr->cmd));
				(*dptr->help)(sock, HELP_DESC);
			}
	}
	return 1;
}

u_short
dcc_save(sock_t *sock, char *args)
{
	short x;

	log("%s SAVE", sock->name);
	alldcc("%s is manually saving all databases...", sock->name);
	if ((x = write_db(0)) > 0) {
		log("%s SAVE ERROR! %s: %s", sock->name, databases[x], strerror(errno));
		alldcc("ERROR! Unable to write %s: %s", databases[x], strerror(errno));
		return 0;
	}
	alldcc("database save complete.", sock->name);
	return 1;
}

u_short
dcc_restart(sock_t *sock, char *args)
{
	short x;

	if (!args)
		return NEED_HELP;
	log("%s RESTART (%s)", sock->name, args);
	alldcc("%s is restarting services! (%s)", sock->name, args);
	if ((x = write_db(0)) > 0) {
		log("%s SAVE ERROR! %s: %s", sock->name, databases[x], strerror(errno));
		alldcc("ERROR! save failed on %s: %s", databases[x], strerror(errno));
		return 0;
	}
	alldcc("restarting.......");
	operwall(0, "\2RESTART\2 requested by \2%s\2", sock->name);
	toserv(":%s QUIT :services restarting...\r\n", me.sclients[CS].nick);
	toserv(":%s QUIT :restarting...\r\n", me.sclients[OS].nick);
	toserv("ERROR :restart requested\r\n");
	fclose(me.logfd);
	close_all_connections();
	execl(me.argv[0], me.argv[0], NULL);
	/* execl() may fail, and return... oh well */
	exit(1);
}

u_short
dcc_jupe(sock_t *sock, char *args)
{
	char buf[BUFSIZE], *server, *reason;

	if (!args || !(server = strtok(args, " ")) || !(reason = strtok(NULL, "")))
		return NEED_HELP;

	if (!strstr(server, ".")) {
		tosock(sock, "Invalid servername!\n");
		return 0;
	}
	log("%s JUPE: %s (%s)", sock->name, server, reason);
	alldcc("%s has JUPED %s! (%s)", sock->name, server, reason);
	operwall(0, "\2JUPE\2 for \2%s\2 added by %s", server, sock->name, reason);

	snprintf(buf, BUFSIZE, "JUPED: %s", reason);
	jupe(server, buf);

	return 1;
}

u_short
dcc_me(sock_t *sock, char *args)
{
	char *chan, *stuff;

	if (!args || !(chan = strtok(args, " ")) || !(stuff = strtok(NULL, "")))
		return NEED_HELP;
	if (check_chan(NULL, chan))
		return 1;

	toserv(":%s PRIVMSG %s :\1ACTION %s\1\r\n", me.sclients[OS].nick, chan, stuff);
	return 1;
}

u_short
dcc_die(sock_t *sock, char *args)
{
	short x;

	if (!args)
		return NEED_HELP;
	log("%s DIE", sock->name);
	alldcc("%s is shutting down services! (%s)", sock->name, args);
	alldcc("saving databases...");
	if ((x = write_db(0)) > 0) {
		log("%s SAVE ERROR! %s: %s", sock->name, databases[x], strerror(errno));
		alldcc("ERROR! Unable to write %s: %s", databases[x], strerror(errno));
		return 0;
	}
	alldcc("shutting down...");
	operwall(0, "\2DIE\2 requested by \2%s\2", sock->name);
	toserv(":%s QUIT :%s told me to die\r\n", me.sclients[OS].nick, sock->name);
	close_all_connections();
	fclose(me.logfd);
	exit(1);
}

u_short
dcc_rehash(sock_t *sock, char *args)
{
	alldcc("%s is rehashing...", sock->name);
	log("%s REHASH", sock->name);
	if (rehash()) {
		alldcc("ERROR while rehashing!");
		return 0;
	}
	alldcc("rehash complete.");;
	return 1;
}

u_short
dcc_reload(sock_t *sock, char *args)
{
	alldcc("%s is manually reloading databases...", sock->name);
	log("%s RELOAD", sock->name);
	if (reload()) {
		alldcc("ERROR! while reloading databases!");
		return 0;
	}
	alldcc("databases have been reloaded.");;
	return 1;
}

u_short
dcc_raw(sock_t *sock, char *args)
{
	if (!args)
		return NEED_HELP;
	if (me.conn == 0) {
		tosock(sock, "I'm not connected to a hub right now.\n");
		return 0;
	}
	tosock(sock, "args = %s\n", args);
/*	log("%s RAW: %s", sock->name, args); */
	toserv("%s\r\n", args);
	return 1;
}

u_short
dcc_quit(sock_t *sock, char *args)
{
	if (args)
		alldcc("*** %s has logged out: %s", sock->name, args);
	else
		alldcc("*** %s has logged out.", sock->name);
	sock->flags |= SOCK_DEL;
	return 1;
}

u_short
dcc_who(sock_t *sock, char *args)
{
	char *nm, *um, *hm, *rm;

	if (!args || !(nm = strtok(args, "!")) || !(um = strtok(NULL, "@")) || !(hm = strtok(NULL, "$")) || !(rm = strtok(NULL, "")))
		return NEED_HELP;

	log("%s WHO: %s!%s@%s$%s", sock->name, nm, um, hm, rm);
	if (who(sock, nm, um, hm, rm) == 0) {
		tosock(sock, "No clients matched.\n");
		return 0;
	}
	tosock(sock, "End of WHO.\n");
	return 1;
}

u_short
dcc_whokill(sock_t *sock, char *args)
{
	char *nm, *um, *hm, *rm, *reason;

	if (!args || !(nm = strtok(args, "!")) || !(um = strtok(NULL, "@")) || !(hm = strtok(NULL, "$")) || !(rm = strtok(NULL, ":")) || !(reason = strtok(NULL, "")))
		return NEED_HELP;

	log("%s WHOKILL: %s!%s@%s$%s", sock->name, nm, um, hm, rm);
	whokill(sock, nm, um, hm, rm, reason);
	return 1;
}

u_short
dcc_status(sock_t *sock, char *args)
{
	extern sock_t *main_sock;
	sock_t *socks = main_sock;
	time_t upt = time(NULL) - me.start;
	tosock(sock, "%s up %d day%s, %d:%02d:%02d, %d admin%s, memory usage: %d bytes\n", shtime(), upt / 86400,
		S(upt / 86400), (upt / 3600) % 24, (upt / 60) % 60, upt % 60, me.admins, S(me.admins), me.memusage);
	for (; socks; socks = socks->next)
		if (socks->flags & (SOCK_DCC|SOCK_TELNET) && !(socks->flags & SOCK_LISTEN))
			tosock(sock, "[%c] %9.9s | idle %d seconds\n", get_flag(socks->flags), socks->name, time(NULL) - socks->last);
	tosock(sock, "There are %d user%s, %d oper%s, on %d server%s, and %d channel%s formed.\n", me.users,
		S(me.users), me.opers, S(me.opers), me.servers, S(me.servers), me.channels, S(me.channels));
	if (me.conn)
		tosock(sock, "I have been connected to my current HUB %s:%d for %d seconds so far.\n", me.hub->name,
			me.hub->port, time(NULL) - me.hub->conn);
	else
		tosock(sock, "I am not currently connected to any HUB!\n");
	return 1;
}

void
dcc_status_help(sock_t *sock, u_short flags)
{
	if (flags & HELP_USAGE)
		tosock(sock, ".status\n");
	if (flags & HELP_DESC)
		tosock(sock, "Displays who is logged into %s and other useful information\n", me.sclients[SCI].nick);
}

void
dcc_who_help(sock_t *sock, u_short flags)
{
	if (flags & HELP_USAGE)
		tosock(sock, ".who nm!um@hm$rm\n");
	if (flags & HELP_DESC)
		tosock(sock, "Displays all clients on the network matching the given mask\n");
}

void
dcc_whokill_help(sock_t *sock, u_short flags)
{
	if (flags & HELP_USAGE)
		tosock(sock, ".whokill nm!um@hm$rm:reason\n");
	if (flags & HELP_DESC)
		tosock(sock, "Kills all clients on the network matching the given mask\n");
}

void
dcc_me_help(sock_t *sock, u_short flags)
{
	if (flags & HELP_USAGE)
		tosock(sock, ".me target action\n");
	if (flags & HELP_DESC)
		tosock(sock, "Sends a CTCP ACTION to the specified target\n");
}

void
dcc_save_help(sock_t *sock, u_short flags)
{
	if (flags & HELP_USAGE)
		tosock(sock, ".save\n");
	if (flags & HELP_DESC)
		tosock(sock, "Manually saves all services databases\n");
}

void
dcc_jupe_help(sock_t *sock, u_short flags)  
{
	if (flags & HELP_USAGE)
		tosock(sock, ".jupe server.name reason\n");
	if (flags & HELP_DESC)
		tosock(sock, "Forcefully jupes the specified server from the network\n");
}

void
dcc_rehash_help(sock_t *sock, u_short flags)  
{
	if (flags & HELP_USAGE)
		tosock(sock, ".rehash\n");
	if (flags & HELP_DESC)
		tosock(sock, "Reloads the services configuration file\n");
}

void
dcc_reload_help(sock_t *sock, u_short flags)  
{
	if (flags & HELP_USAGE)
		tosock(sock, ".reload\n");
	if (flags & HELP_DESC)
		tosock(sock, "Reloads services databases, reverting to last save\n");
}

void
dcc_restart_help(sock_t *sock, u_short flags)  
{
	if (flags & HELP_USAGE)
		tosock(sock, ".restart reason\n");
	if (flags & HELP_DESC)
		tosock(sock, "Saves all databases and restarts services\n");
}

void
dcc_die_help(sock_t *sock, u_short flags)  
{
	if (flags & HELP_USAGE)
		tosock(sock, ".die reason\n");
	if (flags & HELP_DESC)
		tosock(sock, "Saves all databases and shuts down services\n");
}

void
dcc_raw_help(sock_t *sock, u_short flags)  
{
	if (flags & HELP_USAGE)
		tosock(sock, ".raw string\n");
	if (flags & HELP_DESC)
		tosock(sock, "Sends a raw string to the current hub server\n");
}
