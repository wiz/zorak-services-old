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
 *  $Id: net.h,v 1.5 2002/10/11 04:19:14 wiz Exp $
 */

#ifndef __INCLUDE_NET_H_
#define __INCLUDE_NET_H_

#include "irc_types.h"
#include "defines.h"
#include "oline.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct sock {
	struct sock *next;
	struct sock *prev;
	int socket;
	FILE *fd;
	char *name;
	char *buffer;
	char *response;
	oline_t *oline;
	struct in_addr addr;
	u_short port;
	u_short flags;
	time_t conn, last;
} sock_t;

#define	SOCK_ALL	0x0000
#define	SOCK_HUB	0x0001
#define	SOCK_DCC	0x0002
#define	SOCK_CONN	0x0004
#define	SOCK_LISTEN	0x0008
#define	SOCK_DEL	0x0010
#define	SOCK_AUTH	0x0020
/* 0x0040 - 0x0100 reserved for LEET, CADMIN, ADMIN flags */
#define	SOCK_TELNET	0x0200
#define SOCK_DCC_ECHO   0x0400
 
#define TurnEchoOn(x)   (x->flags |= SOCK_DCC_ECHO)
#define TurnEchoOff(x)  (x->flags &= ~SOCK_DCC_ECHO)

sock_t *	add_sock_to_list	(void);
sock_t *	connect_sock		(char *, in_addr_t, u_short, u_short);
sock_t *	listen_sock		(u_short);
void		del_dcc			(char *);
void		del_sock		(sock_t *);
void		io_loop			(void);
void		try_next_hub		(void);
void		toserv			(char *, ...);
void		tosock			(sock_t *, char *, ...);
void		tosocks			(u_short, char *, ...);
void            dcc_partyline           (sock_t *, char *);
int		nonblock_sock		(int);
void		close_all_connections	(void);
u_short		self_lookup		(void);
void		ping_hub		(void);
void		todcc			(char *, char *);
void		alldcc			(char *, ...);
int		who			(sock_t *, char *, char *, char *, char *);
int		whokill			(sock_t *, char *, char *, char *, char *, char *);
#endif
