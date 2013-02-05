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
#include "defines.h"
#include "main.h"
#include "mem.h"
#include "net.h"
#include "me.h"

WIZOSID("$Id: mem.c,v 1.11 2002/10/02 02:56:20 wiz Exp $");

void *
leetmalloc(size_t size)
{
	void *ptr = malloc(size);

	if (!ptr) {
		log("Unable to allocate memory!");
		exit(1);
	}
	me.memusage += size;

	return ptr;
}

void *
leetcalloc(size_t number, size_t size)
{
	void *ptr = calloc(number, size);

	if (!ptr) {
		log("Unable to callocate memory!");
		exit(1);
	}
	me.memusage += (number * size);

	return ptr;
}

void *
leetrealloc(void *ptr, size_t size)
{
	void *tmp;

	me.memusage -= strlen(ptr) + 1;
	if (!(tmp = realloc(ptr, size))) {
		log("Unable to reallocate memory!");
		exit(1);
	}
	me.memusage += size;

	return tmp;
}

void
leetfree(void *ptr, size_t size)
{
	me.memusage -= size;
	free(ptr);
}

char *
leetstrdup(char *in)
{
	char *out = leetmalloc(strlen(in) + 1);
	strlcpy(out, in, strlen(in) + 1);

	return out;
}

char *
leetrestrdup(char *old, char *new)
{
	char *result = leetrealloc(old, strlen(new) + 1);
	strlcpy(result, new, strlen(new) + 1);

	return result;
}
