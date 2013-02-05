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
 */

#include "irc_types.h"
#include "irc_string.h"
#include "irc_time.h"
#include "defines.h"
#include "events.h"
#include "format.h"
#include "config.h"
#include "main.h"
#include "send.h"
#include "mem.h"
#include "db.h"
#include "me.h"
#include "servers.h"
#include "clone.h"
#include "oline.h"
#include "snuggle.h"
#include "conf.h"
#include "channels.h"
#include "net.h"
#include "dcc.h"

#include <sys/stat.h>
#include <errno.h>
#include <err.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

WIZOSID("$Id: main.c,v 1.41 2002/10/02 23:28:51 wiz Exp $");

struct _me me;

u_short
log(char *input, ...)
{
	char logbuf[BUFSIZE];
	va_list ap;

	if (!me.logfd) {
		tell_chans("\2[ERROR]\2 Unable to open logfile!");
		return 1;
	}
	va_start(ap, input);
	leet_vsprintf(logbuf, input, ap);
	fprintf(me.logfd, "%s %s\n", leetctime(time(NULL)), logbuf);
	fflush(me.logfd);
#ifdef DEBUGMODE
	if (me.debug)
		printf("LOG %s\n", logbuf);
#endif
	va_end(ap);
	return 1;
}


u_short
rehash(void)
{
	short i = 0;
	
	tell_chans("rehashing...");
	log("rehashing...");
	if (me.logfd) {
		fflush(me.logfd);
		fclose(me.logfd);
	}
	if (!(me.logfd = fopen(LOGFILE, "a"))) {
		tell_chans("\2[ERROR]\2 Unable to open logfile: %s", strerror(errno));
		goto r_err;
	}
	while (i < MAXCHANS && me.chans[i])
		leetfree(me.chans[i], strlen(me.chans[i++]) + 1);
	del_all_servers();
	del_all_olines();
	if (read_conf()) {
		tell_chans("\2[ERROR]\2 Unable to read configuration file: %s", strerror(errno));
		goto r_err;
	}
	redo_privs();
	tell_chans("done!");
	log("done!");
	return 0;
r_err:	return tell_chans("\2[ERROR]\2 Error while rehashing!");
}

u_short
reload(void)
{
	u_short i = 0;

	tell_chans("reloading databases...");
	log("reloading databases...");
	del_all_clone_allows();
	del_all_snuggles();
	if ((i = read_db(0)) > 0) {
		tell_chans("\2[ERROR]\2 Unable to read %s: %s", databases[i], strerror(errno));
		goto r_err;
	}
	tell_chans("done!");
	log("done!");
	return 0;
r_err:	return tell_chans("\2[ERROR]\2 Error while rehashing!");
}

int
main(int argc, char **argv)
{
	int x;

	umask(0077);

	/* initialize stuff */
	memset(&me, 0, sizeof(me));
	me.argc       = argc;
	me.argv       = argv;
	me.start      = time(NULL);
	me.settime    = time(NULL) + (60*60*4);

	/* setup signals */
	signal(SIGHUP,  do_signal);
	signal(SIGINT,  do_signal);
	signal(SIGQUIT, do_signal);
	signal(SIGILL,  do_signal);
	signal(SIGTRAP, do_signal);
	signal(SIGBUS,  do_signal);
	signal(SIGSEGV, do_signal);
	signal(SIGSYS,  do_signal);
	signal(SIGALRM, SIG_IGN);
	signal(SIGTERM, do_signal);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGUSR1, SIG_IGN);
	signal(SIGUSR2, SIG_IGN);

	while ((x = getopt(me.argc, me.argv, "qd")) != -1) {
		switch (x) {
			case 'd':
				me.debug = 1;
				break;
		}
	}

	if (me.debug == 0)
		printf("%s by %s\n", VER_STR, MYEMAIL);

	if (read_conf())
		err(1, "ERROR! Unable to read configuration file");

	if (!me.servname || !me.servdesc)
		errx(1, "ERROR! Your M: line is invalid, incomplete, or missing!");

	for (x = 0; x < ALL; x++)
		if (!me.sclients[x].nick || check_nick(NULL, me.sclients[x].nick) ||
			!me.sclients[x].username || check_um(NULL, me.sclients[x].username) ||
			!me.sclients[x].realname || check_rm(NULL, me.sclients[x].realname))
			errx(1, "ERROR! Your N: line for %d is invalid, incomplete, or missing!", x);

	if (!me.chans[0])
		errx(1, "ERROR! You need at least one R: line in %s", CONF);

	if (!(me.logfd = fopen(PIDFILE, "w")))
		err(1, "ERROR! Unable to open pidfile");
	fprintf(me.logfd, "%d\n", getpid());
	fclose(me.logfd);

	if (!(me.logfd = fopen(LOGFILE, "a")))
		err(1, "ERROR! Unable to open logfile");

	if ((x = read_db(0)) > 0)
		err(1, "ERROR! Unable to read %s", databases[x]);

	if (me.ip.s_addr == NULL) {
		if (!self_lookup())
			err(1, "ERROR! self-lookup failed!");
		else
			printf("WARNING: no vhost defined in conf, using default of %s\n", inet_ntoa(me.ip));
	}

	if (me.debug == 0)
		switch (me.pid = fork())
		{
			case -1:
				err(1, "ERROR! Unable to fork");
			case 0:
				for (x = 0; x < 12; x++)
					if (x != fileno(me.logfd))
						close(x);
				break;
			default:
				printf("forked into the background: pid = %d\n", me.pid);
				return 0;
		}
	listen_sock(7272);
	try_next_hub();
	add_event(1, SERVTIMEOUT / 2, &ping_hub);
	add_event(1, CONNTIMEOUT / 3, &cleanup_dcc);
	io_loop();
	/* NOTREACHED */
	exit(0);
}


void
do_signal(int sig) 
{
	switch (sig)
	{
		case SIGHUP:
			tell_chans("SIGHUP received!");
			rehash();
			return;
		case SIGINT:
		case SIGQUIT:
		case SIGTERM:
			me.crashing = 1;
			write_db(0);
			if (me.servtype == SERV_IRCNN)
				toserv(":%s OPERWALL :services terminating!\r\n", me.sclients[OS].nick, sig);
			else
				toserv(":%s OPERWALL :services terminating!\r\n", me.servname, sig);
			toserv(":%s QUIT :bye!\r\n", me.sclients[OS].nick);
			fclose(me.logfd);
			close_all_connections();
			exit(1);
		case SIGILL:
		case SIGTRAP:
		case SIGBUS:
		case SIGSEGV:
		case SIGSYS:
			signal(sig, SIG_DFL);
			if (me.crashing == 1)
				abort();
			me.crashing = 1;
			toserv(":%s OPERWALL :\2ACK!\2 signal %d recieved!\r\n", me.servname, sig);
			kill(getpid(), sig);
	}
}
