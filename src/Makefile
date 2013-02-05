#  $Id: Makefile.in,v 1.22 2002/07/30 21:31:15 wiz Exp $
#
CC=	gcc
RM=	/bin/rm
MKDEP=	gcc -MM
SSL_LIBS=  -lcrypto
SSL_INCLUDES= 

# leet cflags
CPPFLAGS=-I../include $(SSL_INCLUDES)
CFLAGS=  -pipe -g -O3 -Wunused -Wall -Wmissing-declarations

SRCS  =	\
	aclone.c\
	channels.c\
	chanserv.c\
	clone.c\
	conf.c\
	dcc.c\
	events.c\
	format.c\
	help.c\
	irc_string.c\
	main.c\
	md5.c\
	mem.c\
	nicks.c\
	nickserv.c\
	net.c\
	oline.c\
	operserv.c\
	parse.c\
	rsa.c\
	sclients.c\
	send.c\
	servers.c\
	snuggle.c\
	users.c

OBJS=${SRCS:.c=.o}

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $<

all: services

services: $(OBJS)
	$(CC) $(CFLAGS) -o services $(OBJS) $(SSL_LIBS)

clean:
	$(RM) -f $(OBJS) services *.core

depend:
	$(MKDEP) $(CPPFLAGS) $(SRCS) > .depend

distclean: clean

install:
	install -c -m700 services ../../test/

include .depend
