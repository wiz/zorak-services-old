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
#include "channels.h"
#include "servers.h"
#include "format.h"
#include "events.h"
#include "config.h"
#include "privs.h"
#include "parse.h"
#include "main.h"
#include "send.h"
#include "dcc.h"
#include "mem.h"
#include "net.h"
#include "me.h"

#include <sys/time.h>
#include <assert.h>
#include <errno.h>

WIZOSID("$Id: net.c,v 1.38 2002/10/11 04:19:19 wiz Exp $");

sock_t *main_sock = NULL;
char *servpass = NULL;

void strip_telnet(int, char *);
#define ERR strerror(errno)

void
try_next_hub(void)
{
	extern server_t *main_server;
	server_t *next;

	if (me.conn || me.crashing)
		return;
	del_sock(NULL);
	if (!main_server) {
		log("HUB: no C: lines in conf defined! nothing to connect to!");
		return;
	}
	if (me.lasthub.name) {
		if ((next = find_server_t(me.lasthub.name)) && (next = next->next)) {
			me.lasthub.name = leetrestrdup(me.lasthub.name, next->name);
			me.lasthub.addr.s_addr = inet_addr(next->host);
			me.lasthub.port = next->port;
			servpass = next->pass;
		} else {
			leetfree(me.lasthub.name, strlen(me.lasthub.name) + 1);
			goto bleh;
		}
	} else { bleh:
		me.lasthub.name = leetstrdup(main_server->name);
		me.lasthub.addr.s_addr = inet_addr(main_server->host);
		me.lasthub.port = main_server->port;
		servpass = main_server->pass;
	}
	log("HUB: autoconnecting to %s on port %d", me.lasthub.name, me.lasthub.port);
	alldcc("HUB: autoconnecting to %s on port %d", me.lasthub.name, me.lasthub.port);
	me.hub = connect_sock(me.lasthub.name, me.lasthub.addr.s_addr, me.lasthub.port, SOCK_HUB);
}

u_short
self_lookup(void)
{
	char hostbuf[BUFSIZE];
	struct hostent *hp;

	gethostname(hostbuf, BUFSIZE - 1);
	hostbuf[BUFSIZE] = 0;
	if (!(hp = gethostbyname((const char *)&hostbuf)))
		return 0;
	memcpy(&me.ip, hp->h_addr_list[0], hp->h_length);
	return 1;
}

sock_t *
add_sock_to_list(void)
{
	sock_t *sock = leetcalloc(sizeof(sock_t), 1);

	sock->next = main_sock;
	main_sock = sock;
	if (sock->next)
		sock->next->prev = sock;
	return sock;
}

int
nonblock_sock(int _sock)
{
	int flags, err = 0, sock = (_sock == -1 ? socket(AF_INET, SOCK_STREAM, 0) : _sock);

	if (sock == -1)
		err = log("NET: unable to allocate socket: %s", ERR);
	else if ((flags = fcntl(sock, F_GETFL, 0)) == -1 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1)
		err = log("NET: set non-blocking socket failed for %d: %s", sock, ERR);
	return (err ? -1 : sock);
}

sock_t *
connect_sock(char *name, in_addr_t addr, u_short port, u_short flags)
{
	struct sockaddr_in sin;
	int err = 0, _sock;
	sock_t *sock;
	FILE *_fd = NULL;

	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = addr;
	sin.sin_port = htons(port);

	if ((_sock = nonblock_sock(-1)) == -1)
		err = 1;
	else if (!(_fd = fdopen(_sock, "r")))
		err = log("fdopen(%d) failed: %s", _sock, strerror(errno));
	/* add bind() call here */
	else if (connect(_sock, (struct sockaddr *)&sin, sizeof(sin)) == -1 && errno != EINPROGRESS) {
		err = log("%s: connection to %s failed: %s", ((flags & SOCK_HUB) ? "HUB" : "DCC"), name, ERR);
		if (flags & SOCK_HUB)
			alldcc("HUB: connection to %s failed: %s", name, ERR);
	}
	if (err) {
		fclose(_fd);
		close(_sock);
		return NULL;
	}
	sock = add_sock_to_list();
	sock->socket = _sock;
	sock->fd = _fd;
	sock->flags = flags;
	sock->addr.s_addr = addr;
	sock->port = port;
	sock->name = leetstrdup(name);
	sock->buffer = leetcalloc(BUFSIZE, 1);
	return sock;
}

sock_t *
listen_sock(u_short port)
{
	struct sockaddr_in sin;
	int _sock, flags, ospacer = 1;
	sock_t *sock;
	
	memset(&sin, 0, sizeof(sin));
	sin.sin_family = AF_INET;
	sin.sin_addr.s_addr = htonl(INADDR_ANY);
	if (port > 0) {
		sin.sin_port = htons(port);
		flags = SOCK_TELNET;
	} else {
		sin.sin_port = htons(51000);
		flags = SOCK_DCC;
	}
	if ((_sock = nonblock_sock(-1)) == -1 ||
	    setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, (char *)&ospacer, sizeof(ospacer)) == -1)
		goto err;
	while (bind(_sock, (struct sockaddr *)&sin, sizeof(sin)) == -1)
		if (errno == EADDRINUSE)
			sin.sin_port = htons(ntohs(sin.sin_port) + 1);
		else
			goto err;
	if (listen(_sock, 7) == -1)
		goto err;
	if (0) {
err:		log("NET: cannot listen on port %d: %s", port, ERR);
		close(_sock);
		return NULL;
	}
	sock = add_sock_to_list();
	sock->socket = _sock;
	sock->flags |= (flags|SOCK_LISTEN);
	sock->addr = sin.sin_addr;
	sock->port = ntohs(sin.sin_port);
	sock->last = time(NULL);
	return sock;
}

void
cleanup_dcc(void)
{
	time_t now = time(NULL);
	sock_t *sock = main_sock;

	for (; sock; sock = sock->next) {
#if 0
		alldcc("dcc cleanup: checking %s(%d), SOCK_DCC? %s, SOCK_TELNET %s, SOCK_LISTEN %s",
			sock->name ? sock->name : "unknown", sock->socket, (sock->flags & SOCK_DCC ? "yes" : "no"),
			(sock->flags & SOCK_TELNET ? "yes" : "no"),(sock->flags & SOCK_LISTEN ? "yes" : "no"));
#endif
		if (sock->flags & SOCK_DCC || (sock->flags & SOCK_TELNET && !(sock->flags & SOCK_LISTEN))) {
			if (!(sock->flags & SOCK_CONN) && sock->last + CONNTIMEOUT <= now)
				sock->flags |= SOCK_DEL;
			if (!(sock->flags & SOCK_AUTH) && sock->conn + CONNTIMEOUT <= now) {
				tosock(sock, "Login grace time exceeded, closing connection.\n");
				sock->flags |= SOCK_DEL;
			}
		}
	}
}

void
del_dcc(char *name)
{
	sock_t *sock = main_sock;

	for (; sock; sock = sock->next)
		if ((sock->flags & (SOCK_DCC|SOCK_AUTH)) == (SOCK_DCC|SOCK_AUTH) && strcmp(sock->name, name) == 0) {
			del_sock(sock);
			return;
		}
}

void
del_sock(sock_t *sock)
{
	if (!sock)
		for (sock = main_sock; sock; sock = sock->next)
			if (sock->flags & SOCK_HUB)
				break;
	if (!sock)
		return;
	if (sock->fd)
		fclose(sock->fd);
	shutdown(sock->socket, 2);
	close(sock->socket);
	if (sock->prev) {
		sock->prev->next = sock->next;
	} else {
		main_sock = sock->next;
		if (sock->next)
			sock->next->prev = NULL;
	}
	if (sock->next)
		sock->next->prev = sock->prev;
	if (sock->flags & SOCK_HUB && me.crashing != 1) {
		log("HUB: disconnected from %s", sock->name);
		alldcc("HUB: disconnected from %s", sock->name);
		del_all_users();
		add_event(time(NULL) + 7, CONNTIMEOUT / 2, &try_next_hub);
	}
	if (sock->name)
		leetfree(sock->name, strlen(sock->name) + 1);
	if (sock->response)
		leetfree(sock->response, strlen(sock->response) + 1);
	leetfree(sock->buffer, BUFSIZE);
	leetfree(sock, sizeof(sock_t));
}

void
tosocks(u_short flags, char *tosend, ...)
{
	sock_t *sock = main_sock;
	char msgbuf[BUFSIZE];
	va_list ap;

	if (flags == SOCK_ALL)
		flags = SOCK_AUTH;
	flags |= SOCK_CONN;

	va_start(ap, tosend);
	leet_vsprintf(msgbuf, tosend, ap);
	for (; sock; sock = sock->next)
		if ((sock->flags & flags) == flags)
			write(sock->socket, msgbuf, strlen(msgbuf));
	va_end(ap);
}

void
alldcc(char *tosend, ...)
{
	sock_t *sock = main_sock;
	char msgbuf[BUFSIZE], sendbuf[BUFSIZE], *temp = shtime();
	va_list ap;

	va_start(ap, tosend);
	leet_vsprintf(msgbuf, tosend, ap);
	for (; sock; sock = sock->next)
		if (sock->flags & (SOCK_DCC|SOCK_TELNET) && sock->flags & SOCK_AUTH) {
			memset(sendbuf, 0, BUFSIZE);
			snprintf(sendbuf, BUFSIZE, "(%s) %s\n", temp, msgbuf);
			write(sock->socket, sendbuf, strlen(sendbuf));
		}
	va_end(ap);
}

void
dcc_partyline(sock_t *sender, char *msg)
{
        sock_t *sock = main_sock;
        char msgbuf[BUFSIZE];

        snprintf(msgbuf, BUFSIZE, "(%s) (%s) %s\n", shtime(), sender->name, msg);
        for (; sock; sock = sock->next)
                if (sock->flags & (SOCK_DCC|SOCK_TELNET) && sock->flags & SOCK_AUTH &&
                    (sock != sender || sender->flags & SOCK_DCC_ECHO))
                        write(sock->socket, msgbuf, strlen(msgbuf));
}

void
todcc(char *who, char *what)
{
	sock_t *sock = main_sock;

	for (; sock; sock = sock->next)
		if (sock->name && strcasecmp(sock->name, who) == 0) {
			write(sock->socket, what, strlen(what));
			return;
		}
}

void
ping_hub(void)
{
	if (me.conn != 1)
		return;
	if (me.hub->last + SERVTIMEOUT <= time(NULL)) {
		log("HUB: %s is not responding!", me.hub->name);
		alldcc("HUB: %s is not responding!", me.hub->name);
		del_sock(NULL);
	} else {
		toserv(":%s PING :%s\r\n", me.servname, me.servname);
	}
}

void
io_loop(void)
{
	int r, i, errv, errlen = sizeof(errv);
	struct timeval tv = { 1, 0 };
	struct sockaddr_in sin;
	size_t ssin = sizeof(sin);
	char buffer[BUFSIZE];
	sock_t *sock, *tmp;

	memset(buffer, 0, BUFSIZE);
loop:	/* wiz is leet */
	FD_ZERO(&me.readfds);
	FD_ZERO(&me.writefds);
	for (sock = main_sock; sock;) {
		if (sock->flags & SOCK_DEL) {
			tmp = sock->next;
			del_sock(sock);
			sock = tmp;
			continue;
		}
		if (sock->flags & (SOCK_LISTEN|SOCK_CONN))
			FD_SET(sock->socket, &me.readfds);
		else
			FD_SET(sock->socket, &me.writefds);
		sock = sock->next;
	}
	while ((r = select(FD_SETSIZE, &me.readfds, &me.writefds, NULL, &tv)) == -1)
		if (errno != EINTR) {
			log("FATAL! select() == -1: %s", ERR);
			exit(1);
		}
	do_events(time(NULL));
	for (sock = main_sock; r > 0 && sock;) {
		if (FD_ISSET(sock->socket, &me.readfds) && r--) {
			if (sock->flags & SOCK_LISTEN) {
				memset(&sin, 0, sizeof(sin));
				if ((i = accept(sock->socket, (struct sockaddr *)&sin, &ssin)) == -1)
					goto nextsock;
				log("NET: connection from %s:%d", inet_ntoa(sin.sin_addr), ntohs(sin.sin_port));
				tmp = add_sock_to_list();
				if ((tmp->socket = nonblock_sock(i)) == -1) {
					del_sock(tmp);
					goto nextsock;
				} else if (!(tmp->fd = fdopen(tmp->socket, "r"))) {
					log("fdopen(%d) failed: %s", tmp->socket, strerror(errno));
					del_sock(tmp);
					goto nextsock;
				}
				tmp->conn = time(NULL);
				tmp->addr = sin.sin_addr;
				tmp->port = ntohs(sin.sin_port);
				tmp->buffer = leetcalloc(BUFSIZE, 1);
				tmp->flags |= (SOCK_CONN|sock->flags);
				tmp->flags &= ~SOCK_LISTEN;
				greet_dcc(tmp);
				goto nextsock;
			}
			if (!fgets(sock->buffer, BUFSIZE, sock->fd)) {
				errv = 0;
				if (getsockopt(sock->socket, SOL_SOCKET, SO_ERROR, &errv, &errlen) < 0) {
					log("NET: getsockopt(SO_ERROR) failed: %s", ERR);
					goto delsock;
				}
				goto readerr;
			}
			sock->last = time(NULL);
			for(i = 0; i < BUFSIZE; i++)
				if (sock->buffer[i] == '\r' || sock->buffer[i] == '\n')
					sock->buffer[i] = 0;
			if (sock->flags & SOCK_HUB)
				parse(sock->buffer);
			else if (sock->flags & (SOCK_DCC|SOCK_TELNET))
				parse_dcc(sock, sock->buffer);
			memset(sock->buffer, 0, BUFSIZE);
		} else if (FD_ISSET(sock->socket, &me.writefds) && r--) {
			errv = 0;
			if (getsockopt(sock->socket, SOL_SOCKET, SO_ERROR, &errv, &errlen) < 0) {
				log("NET: getsockopt(SO_ERROR) failed: %s", ERR);
				goto delsock;
			}
			if (errv > 0) {
				if (sock->flags & SOCK_HUB) {
					log("HUB: error connecting to %s: %s", sock->name, strerror(errv));
					alldcc("HUB: error connecting to %s: %s", sock->name, strerror(errv));
#if 0
				} else if (sock->flags & SOCK_DCC && find_client(sock->name))
					reply(OS, sock->name, "Error DCC connecting: %s", strerror(errv));
#else
				}
#endif
				goto delsock;
			}
			if (sock->flags & SOCK_HUB) {
				log("HUB: connected to %s", sock->name);
				alldcc("HUB: connected to %s", sock->name);
				del_event_func(&try_next_hub);
				sock->conn = time(NULL);
				me.conn = 1;
				switch (me.servtype) {
					case SERV_IRCNN:
						toserv("PASS :%s\r\n", servpass);
						toserv("SERVER %s 1 %lu %lu J09 :%s\r\n", me.servname, time(NULL), time(NULL), me.servdesc);
						add_linked_server(me.servname, me.servname, me.servdesc);
						intro_nicks(ALL);
						join_channels(OS);
						break;
					case SERV_HYBRD:
						toserv("CAPAB :QS EX IE EOB UNKLN KLN HOPS HUB TBURST\r\n");
						toserv("PASS %s :TS\r\n", servpass);
						toserv("SERVER %s 0 :%s\r\n", me.servname, me.servdesc);
						toserv("SVINFO 5 5 0 :%lu\r\n", time(NULL));
						add_linked_server(me.servname, me.servname, me.servdesc);
						break;
				}
			} else if (sock->flags & SOCK_DCC) {
				log("DCC: %s connected from %s:%d", sock->name, inet_ntoa(sock->addr), sock->port);
				me.dcc++;
			}
			sock->flags |= SOCK_CONN;
		}
		nextsock: {
			sock = sock->next;
			continue;
		}
		readerr: {
			log("NET: read error from %s: %s", (sock->name ? sock->name : inet_ntoa(sock->addr)), strerror(errv));
		}
		delsock: {
			sock->flags |= SOCK_DEL;
			goto nextsock;
		}
	}
	goto loop;
}

void
toserv(char *tosend, ...)
{
	char msgbuf[BUFSIZE];
	va_list ap;

	if (me.conn == 0)
		return;

	va_start(ap, tosend);
	leet_vsprintf(msgbuf, tosend, ap);
	if (write(me.hub->socket, msgbuf, strlen(msgbuf)) < 0) {
		log("HUB: write error to %s: %s", me.hub->name, ERR);
		alldcc("HUB: write error to %s: %s", me.hub->name, ERR);
		del_sock(me.hub);
		me.conn = 0;
	}
#ifdef DEBUGMODE
	if (me.debug)
		printf(">>> %s", msgbuf);
#endif
	va_end(ap);
}

void
tosock(sock_t *sock, char *tosend, ...)
{
	char msgbuf[BUFSIZE];
	va_list ap;

	va_start(ap, tosend);
	leet_vsprintf(msgbuf, tosend, ap);
	if (write(sock->socket, msgbuf, strlen(msgbuf)) < 0)
		del_sock(sock);
	va_end(ap);
}

void
close_all_connections(void)
{
	sock_t *tmp, *sock = main_sock;

	while (sock) {
		tmp = sock->next;
		del_sock(sock);
		sock = tmp;
	}
}
