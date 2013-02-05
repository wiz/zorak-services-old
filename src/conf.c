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

#include <fcntl.h>
#include <unistd.h>

#include "irc_string.h"
#include "irc_types.h"
#include "irc_time.h"
#include "defines.h"
#include "config.h"
#include "privs.h"
#include "flags.h"
#include "conf.h"
#include "mem.h"
#include "db.h"
#include "me.h"
#include "servers.h"
#include "oline.h"
#include "client.h"
#include "snuggle.h"
#include "clone.h"
#include "nicks.h"

WIZOSID("$Id: conf.c,v 1.22 2002/10/02 22:08:42 wiz Exp $");

int
read_conf(void)
{
	char c, line[BUFSIZE], *field[10];
	struct privs *pptr;
	u_short i, k = 0;
	FILE *conf;

	if (!(conf = fopen(CONF, "r")))
		return 1;

	while (fgets(line, BUFSIZE, conf)) {
		c = line[0];
		if (c == '#' || c == '\n')
			continue;
		strtok(line, ":");
		for(i = 0; (field[i] = strtok(NULL, ":")); i++);
		field[--i][strlen(field[i]) - 1] = '\0';

		switch (c) {
			case 'C':
			case 'c':
				if (field[0] && field[1] && field[2] && field[3] && atoi(field[4]) > 0)
					add_server(field[0], field[1], field[2], atoi(field[4]));
				if (field[3] && atoi(field[3]) > 0 && me.pid == 0)
					switch (atoi(field[3])) {
						case SERV_HYBRD:
							me.servtype = SERV_HYBRD;
							break;
						default:
							me.servtype = SERV_IRCNN;
							break;
					}
				break;
			case 'O':
			case 'o':
				if (field[0] && field[1] && field[2] && field[3])
					for (pptr = privs; pptr->flag != 0; pptr++)
						if (pptr->flag == *field[3]) {
							add_oline(field[0], field[2], field[1], pptr->privs);
							break;
						}
				break;
			case 'R':
			case 'r':
				if (k < MAXCHANS)
					me.chans[k++] = leetstrdup(field[0]);
				break;
			case 'M': /* M:services2.newnet.net:10.7.9.2:NewNet Operator Services */
			case 'm':
				if (field[0] && me.pid == 0)
					me.servname = leetstrdup(field[0]);
				if (field[1])
					me.ip.s_addr = inet_addr(field[1]);
				if (field[2]) {
					if (me.servdesc)
						leetfree(me.servdesc, strlen(me.servdesc) + 1);
					me.servdesc = leetstrdup(field[2]);
				}
				break;
			case 'N': /* N:NickServ:momo:nickserv is leet:2 */
			case 'n':
				if (field[0] && field[1] && field[2] && field[3] && atoi(field[3]) < ALL &&
					!me.sclients[atoi(field[3])].nick) {
					me.sclients[atoi(field[3])].nick = leetstrdup(field[0]);
					me.sclients[atoi(field[3])].username = leetstrdup(field[1]);
					me.sclients[atoi(field[3])].realname = leetstrdup(field[2]);
				}
				break;
			case 'T':
			case 't':
				if (field[0]) {
					if (atoi(field[0]) > 0)
						me.opernotice = 1;
					else
						me.opernotice = 0;
				}
				break;
		}
	}
	me.chans[k] = NULL;
	fclose(conf);
	return 0;
}

int
read_db(u_short dbnum)
{
	FILE *db;
	char line[BUFSIZE];
	char *field[10];
	int i = -1, j;

	if (dbnum == 0) {
		while (databases[++dbnum])
			if (read_db(dbnum) == -1)
				return dbnum;
		return 0;
	}
	if (!(db = fopen(databases[dbnum], "r")))
		return -1;

	while (fgets(line, BUFSIZE - 1, db)) {
		if (line[0] == '#')
			continue;
		if (isdigit(line[0]))
			j = atoi(strtok(line, ":"));
		else {
			j = line[0];
			strtok(line, ":");
		}
		i = -1;
		while ((field[++i] = strtok(NULL, ":")));
		field[--i][strlen(field[i]) - 1] = '\0';

		switch (j) {
			case 'A':
				if (field[0] && field[1] && atoi(field[1]) > 0)
					add_clone_allow(field[0], atoi(field[1]));
				break;
			case 'S':
				if (field[0] && field[1] && field[2] && field[3] && field[4] && field[5] && field[6] &&
				    atoi(field[6]) > 0 && field[7] && atoi(field[7]) > 0)
					add_snuggle(field[0], field[1], field[2], field[3],
						    field[4], field[5], atoi(field[6]), strtol(field[7], NULL, 10));
				break;
			case 'N':
				if (field[0] && field[1] && field[2] && atoi(field[2]) > 0 && field[3] && atoi(field[3]) > 0 &&
					field[4] && field[5] && field[6] && (atoi(field[6]) > 0 || strcmp(field[6], "0") == 0))
					add_registered_nick(field[0], field[1], strtol(field[2], NULL, 10), strtol(field[3], NULL, 10),
										field[4], field[5], atoi(field[6]));
				break;
		}
	}
	fclose(db);
	return 0;
}

int
write_db(u_short dbnum)
{
	FILE *db;
	clone_t *allow;
	snuggle_t *snuggle;
	nick_t *nick;
	extern clone_t *main_clone_allow;
	extern snuggle_t *main_snuggle;
	extern nick_t *main_nick;

	if (dbnum == 0) {
		while (databases[++dbnum])
			if (write_db(dbnum) == -1)
				return dbnum;
		return 0;
	}
	if (rename(databases[dbnum], "temp.db") < 0)
		return -1;

	if (!(db = fopen(databases[dbnum], "w")))
		return -1;

	switch (dbnum) {
		case DB_ACLONE:
			fprintf(db, "# ACLONE db for %s as of %lu\n", me.servname, time(NULL));
			for (allow = main_clone_allow; allow; allow = allow->next)
				fprintf(db, "A:%s:%d\n", allow->host, allow->clones);
			break;
		case DB_SNUGGLE:
			fprintf(db, "# SNUGGLE db for %s as of %lu\n", me.servname, time(NULL));
			for (snuggle = main_snuggle; snuggle; snuggle = snuggle->next)
				fprintf(db, "S:%s:%s:%s:%s:%s:%s:%d:%lu\n", snuggle->creator, snuggle->nm, snuggle->um, snuggle->hm, snuggle->rm, snuggle->reason, snuggle->type, snuggle->created);
			break;
		case DB_NICK:
			fprintf(db, "# NICK db for %s as of %lu\n", me.servname, time(NULL));
			for (nick = main_nick; nick; nick = nick->next)
				fprintf(db, "N:%s:%s:%lu:%lu:%s:%s:%d\n", nick->nick, nick->passwd, nick->regtime, nick->lasttime,
						nick->info, nick->lasthost, nick->flags);
			break;
	}
	fflush(db);
	fclose(db);
	unlink("temp.db");
	return 0;
}

char
get_flag(u_short flags)
{
	struct privs *pptr;

	for (pptr = privs; pptr->flag != 0; pptr++)
		if (flags & pptr->privs)
			return pptr->flag;
	return '?';
}
