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

#include "defines.h"

WIZOSID("$Id: rsa.c,v 1.4 2002/07/30 03:23:02 wiz Exp $");

#ifdef HAVE_LIBCRYPTO
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/rsa.h>
#include <openssl/md5.h>
#include <openssl/bn.h>
#include <openssl/evp.h>
#include <openssl/err.h>
#include "oline.h"
#include "rsa.h"
#include "mem.h"

static void
binary_to_hex(unsigned char *bin, char *hex, int length)
{
	char *trans = "0123456789ABCDEF";
	int i = 0;

	for(; i < length; i++)
	{
		hex[i << 1] = trans[bin[i] >> 4];
		hex[(i << 1) + 1] = trans[bin[i] & 0xf];
	}
	hex[i << 1] = '\0';
}

void
read_public_key(oline_t *oper)
{
	BIO *file = BIO_new_file(oper->rsa_public_key_file, "r");
	oper->rsa_public_key = (RSA *)PEM_read_bio_RSA_PUBKEY(file, NULL, 0, NULL);
	BIO_set_close(file, BIO_CLOSE);
	BIO_free(file);
}

int
get_rand(unsigned char *buf, size_t len)
{
	if (RAND_status())
		return RAND_bytes(buf, len);
	else
		return RAND_pseudo_bytes(buf, len);
}

char *
make_challenge(char **respbuf, RSA *rsa)
{
	unsigned long len = RSA_size(rsa);
	unsigned char rand[32], encr[(len << 1) + 1], *challenge;

	get_rand(rand, 32);
	binary_to_hex(rand, (*respbuf ? *respbuf : (*respbuf = leetmalloc(65))), 32);
	if (RSA_public_encrypt(32, rand, encr, rsa, RSA_PKCS1_PADDING) < 0)
		return NULL;

	binary_to_hex(encr, (challenge = leetmalloc((len << 1) + 1)), len);
	return challenge;
}

#endif /* HAVE_LIBCRYPTO */
