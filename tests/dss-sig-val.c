/*
 * Copyright (C) 2017 Red Hat, Inc.
 *
 * Author: Nikos Mavrogiannopoulos
 *
 * This file is part of GnuTLS.
 *
 * GnuTLS is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * GnuTLS is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GnuTLS.  If not, see <https://www.gnu.org/licenses/>.
 */

/* Parts copied from GnuTLS example programs. */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <gnutls/crypto.h>

#include "utils.h"

static void encode(const char *test_name, const gnutls_datum_t * sig,
		   const gnutls_datum_t * r, const gnutls_datum_t * s)
{
	int ret;
	gnutls_datum_t tmp_r, tmp_s, tmp_sig;

	ret = gnutls_decode_rs_value(sig, &tmp_r, &tmp_s);
	if (ret < 0) {
		fail("%s: gnutls_decode_rs_value: %s\n", test_name,
		     gnutls_strerror(ret));
		exit(1);
	}

	if (tmp_r.size != r->size || memcmp(r->data, tmp_r.data, r->size) != 0) {
		fail("%s: gnutls_decode_rs_value: r doesn't match\n",
		     test_name);
		exit(1);
	}

	if (tmp_s.size != s->size || memcmp(s->data, tmp_s.data, s->size) != 0) {
		fail("%s: gnutls_decode_rs_value: s doesn't match\n",
		     test_name);
		exit(1);
	}

	gnutls_free(tmp_r.data);
	gnutls_free(tmp_s.data);

	/* check encoding */
	ret = gnutls_encode_rs_value(&tmp_sig, r, s);
	if (ret < 0) {
		fail("%s: gnutls_encode_rs_value: %s\n", test_name,
		     gnutls_strerror(ret));
		exit(1);
	}

	if (tmp_sig.size != sig->size
	    || memcmp(sig->data, tmp_sig.data, sig->size) != 0) {
		fail("%s: gnutls_encode_rs_value: sig doesn't match\n",
		     test_name);
		exit(1);
	}

	gnutls_free(tmp_sig.data);

	return;
}

struct encode_tests_st {
	const char *name;
	gnutls_datum_t sig;
	gnutls_datum_t r;
	gnutls_datum_t s;
};

struct encode_tests_st encode_tests[] = {
	{
	 .name = "test1",
	 .sig =
	 {(unsigned char *)
	  "\x30\x46\x02\x21\x00\xe8\xa4\x26\x96\x2c\x32\xc6\x92\x55\x71\x31\xd7\x10\x35\x92\x60\x85\x34\xf0\x65\x03\x08\x2e\x38\x2b\xc8\x28\x67\xde\x10\x7c\xf5\x02\x21\x00\xc9\x8e\x56\x4f\xb1\x62\xe1\x74\xbe\x8c\x9d\xff\x04\x88\x75\x76\x63\x91\x8a\xd6\x9d\x41\x76\xef\xe2\xb5\x8e\xbb\xa8\x88\xba\x9f",
	  72},
	 .r =
	 {(unsigned char *)
	  "\x00\xe8\xa4\x26\x96\x2c\x32\xc6\x92\x55\x71\x31\xd7\x10\x35\x92\x60\x85\x34\xf0\x65\x03\x08\x2e\x38\x2b\xc8\x28\x67\xde\x10\x7c\xf5",
	  33},
	 .s =
	 {(unsigned char *)
	  "\x00\xc9\x8e\x56\x4f\xb1\x62\xe1\x74\xbe\x8c\x9d\xff\x04\x88\x75\x76\x63\x91\x8a\xd6\x9d\x41\x76\xef\xe2\xb5\x8e\xbb\xa8\x88\xba\x9f",
	  33}
	 },
	{
	 .name = "test2",
	 .sig =
	 {(unsigned char *)
	  "\x30\x44\x02\x20\x07\xd2\x7f\xd0\xef\x77\xa8\x29\x31\x9b\x46\x01\xb3\xaf\x66\xac\x33\x48\x15\x94\xb8\x80\xa1\x97\x71\x8f\x4a\x32\x43\x12\xec\x1f\x02\x20\x37\x06\x13\xbe\x4c\x6d\xdd\xcd\xa1\x4c\x88\xc5\x66\x85\x81\xf5\x50\x41\xb1\x4a\xee\x29\xce\x73\x2c\x09\xff\xba\xe3\x5e\x8a\x12",
	  70},
	 .r =
	 {(unsigned char *)
	  "\x07\xd2\x7f\xd0\xef\x77\xa8\x29\x31\x9b\x46\x01\xb3\xaf\x66\xac\x33\x48\x15\x94\xb8\x80\xa1\x97\x71\x8f\x4a\x32\x43\x12\xec\x1f",
	  32},
	 .s =
	 {(unsigned char *)
	  "\x37\x06\x13\xbe\x4c\x6d\xdd\xcd\xa1\x4c\x88\xc5\x66\x85\x81\xf5\x50\x41\xb1\x4a\xee\x29\xce\x73\x2c\x09\xff\xba\xe3\x5e\x8a\x12",
	  32}
	 },
	{
	 .name = "test3",
	 .sig =
	 {(unsigned char *)
	  "\x30\x44\x02\x20\x57\x53\x71\xfd\x9c\xb5\x96\xc2\xc5\xc7\x59\xce\x2f\xc2\xbe\x8f\xdc\x22\xf9\xab\x38\xdc\x57\x81\xfb\x34\x78\x12\xae\x88\x9a\x50\x02\x20\x60\x6b\xd4\x5b\xd6\x32\x05\xae\x1f\xe6\x08\xf4\x73\x0f\x42\xbc\x32\x55\x1f\x8d\xea\x81\xec\x2b\x4a\x0b\x8c\x0f\xb1\xbe\x5f\x9b",
	  70},
	 .r =
	 {(unsigned char *)
	  "\x57\x53\x71\xfd\x9c\xb5\x96\xc2\xc5\xc7\x59\xce\x2f\xc2\xbe\x8f\xdc\x22\xf9\xab\x38\xdc\x57\x81\xfb\x34\x78\x12\xae\x88\x9a\x50",
	  32},
	 .s =
	 {(unsigned char *)
	  "\x60\x6b\xd4\x5b\xd6\x32\x05\xae\x1f\xe6\x08\xf4\x73\x0f\x42\xbc\x32\x55\x1f\x8d\xea\x81\xec\x2b\x4a\x0b\x8c\x0f\xb1\xbe\x5f\x9b",
	  32}

	 },
	{
	 .name = "test4",
	 .sig =
	 {(unsigned char *)
	  "\x30\x46\x02\x21\x00\xe7\xdd\x79\x58\x96\xf6\x12\x05\xc1\x7a\x44\xd6\xdf\xde\x83\xe9\xb6\x30\xa7\xff\x02\x85\x97\x34\x5a\xcd\x2f\xae\xbd\xc6\x5d\xde\x02\x21\x00\xb5\x64\xbc\x53\x97\xc5\x64\x6f\x6b\x81\xe7\x4d\xad\x36\x29\x50\xd1\x62\x91\x65\x51\xf0\xc4\xa5\x28\x80\x20\x51\x9b\xce\xcc\xc8",
	  72},
	 .r =
	 {(unsigned char *)
	  "\x00\xe7\xdd\x79\x58\x96\xf6\x12\x05\xc1\x7a\x44\xd6\xdf\xde\x83\xe9\xb6\x30\xa7\xff\x02\x85\x97\x34\x5a\xcd\x2f\xae\xbd\xc6\x5d\xde",
	  33},
	 .s =
	 {(unsigned char *)
	  "\x00\xb5\x64\xbc\x53\x97\xc5\x64\x6f\x6b\x81\xe7\x4d\xad\x36\x29\x50\xd1\x62\x91\x65\x51\xf0\xc4\xa5\x28\x80\x20\x51\x9b\xce\xcc\xc8",
	  33}
	 },
	{
	 .name = "test5",
	 .sig =
	 {(unsigned char *)
	  "\x30\x45\x02\x20\x54\xce\x1b\x8d\x63\x5d\xa4\xdb\x26\x58\x1c\x8f\xf0\xb0\x6d\xf3\x2f\x6a\xdb\x83\xcf\x7e\xe7\xda\x98\x52\xa6\x5f\x1f\xc9\x50\x73\x02\x21\x00\xfa\xb9\x6a\x18\xd4\xc6\x45\x3c\xf2\xbf\xc2\x3d\x15\x0d\xc6\xd4\x0c\x78\x52\x12\xec\xb5\x3c\xf5\xe0\x8a\xbf\x6c\x11\xd3\xfd\x4f",
	  71},
	 .r =
	 {(unsigned char *)
	  "\x54\xce\x1b\x8d\x63\x5d\xa4\xdb\x26\x58\x1c\x8f\xf0\xb0\x6d\xf3\x2f\x6a\xdb\x83\xcf\x7e\xe7\xda\x98\x52\xa6\x5f\x1f\xc9\x50\x73",
	  32},
	 .s =
	 {(unsigned char *)
	  "\x00\xfa\xb9\x6a\x18\xd4\xc6\x45\x3c\xf2\xbf\xc2\x3d\x15\x0d\xc6\xd4\x0c\x78\x52\x12\xec\xb5\x3c\xf5\xe0\x8a\xbf\x6c\x11\xd3\xfd\x4f",
	  33}
	 },
	{
	 .name = "test6",
	 .sig =
	 {(unsigned char *)
	  "\x30\x45\x02\x20\x3b\x77\x03\xcf\x8f\xe7\x92\xf1\x6a\x68\x3b\xcd\xb7\x21\x5b\x80\xcd\x00\x75\x65\x38\x31\x8c\xc2\x9b\x92\x6a\x1a\x02\x4b\xd8\x73\x02\x21\x00\xd9\x88\x28\x68\xae\x31\x5d\x95\xa4\x8b\x5e\x3a\x95\x24\x7b\x0d\x07\xaa\xc4\xe0\xeb\xb1\xf1\x89\xc0\xab\x4d\x88\x15\x84\x5f\x01",
	  71},
	 .r =
	 {(unsigned char *)
	  "\x3b\x77\x03\xcf\x8f\xe7\x92\xf1\x6a\x68\x3b\xcd\xb7\x21\x5b\x80\xcd\x00\x75\x65\x38\x31\x8c\xc2\x9b\x92\x6a\x1a\x02\x4b\xd8\x73",
	  32},
	 .s =
	 {(unsigned char *)
	  "\x00\xd9\x88\x28\x68\xae\x31\x5d\x95\xa4\x8b\x5e\x3a\x95\x24\x7b\x0d\x07\xaa\xc4\xe0\xeb\xb1\xf1\x89\xc0\xab\x4d\x88\x15\x84\x5f\x01",
	  33}
	 },
	{
	 .name = "test7",
	 .sig =
	 {(unsigned char *)
	  "\x30\x45\x02\x20\x22\xa1\xbf\x7d\xcd\x7b\x5e\xab\xa0\x08\x20\xe5\xcc\x22\x34\x61\xf1\x1f\xd0\x3a\x32\xc3\x61\x19\xcf\xe4\xeb\xff\x66\xe6\x51\xa1\x02\x21\x00\x9d\x62\xcb\x04\x3e\xb6\x1f\xc1\x2e\x52\xd4\x73\x7f\xbd\xa5\x9c\x29\x49\x75\x6d\x08\xf9\xcb\x74\xd7\x41\xdc\xcb\x66\x4d\x9a\x77",
	  71},
	 .r =
	 {(unsigned char *)
	  "\x22\xa1\xbf\x7d\xcd\x7b\x5e\xab\xa0\x08\x20\xe5\xcc\x22\x34\x61\xf1\x1f\xd0\x3a\x32\xc3\x61\x19\xcf\xe4\xeb\xff\x66\xe6\x51\xa1",
	  32},
	 .s =
	 {(unsigned char *)
	  "\x00\x9d\x62\xcb\x04\x3e\xb6\x1f\xc1\x2e\x52\xd4\x73\x7f\xbd\xa5\x9c\x29\x49\x75\x6d\x08\xf9\xcb\x74\xd7\x41\xdc\xcb\x66\x4d\x9a\x77",
	  33}
	 },
	{
	 .name = "test8",
	 .sig =
	 {(unsigned char *)
	  "\x30\x44\x02\x20\x41\xc9\x32\x16\x2e\x6f\x0c\x1f\x0f\x81\xc0\x0f\x01\x50\x31\x75\x10\x55\x25\x45\x2a\x75\xb1\xdf\x91\x8e\xdf\x24\x30\xf2\xa8\x5f\x02\x20\x07\x80\xa7\x9b\xb8\x1c\x4f\x6e\xc4\x97\x5c\xb7\x9e\x61\x2a\xf0\xfc\x16\xdd\xe7\xa3\xcc\x15\x99\x68\x08\x8e\x85\x60\x9d\x34\xf8",
	  70},
	 .r =
	 {(unsigned char *)
	  "\x41\xc9\x32\x16\x2e\x6f\x0c\x1f\x0f\x81\xc0\x0f\x01\x50\x31\x75\x10\x55\x25\x45\x2a\x75\xb1\xdf\x91\x8e\xdf\x24\x30\xf2\xa8\x5f",
	  32},
	 .s =
	 {(unsigned char *)
	  "\x07\x80\xa7\x9b\xb8\x1c\x4f\x6e\xc4\x97\x5c\xb7\x9e\x61\x2a\xf0\xfc\x16\xdd\xe7\xa3\xcc\x15\x99\x68\x08\x8e\x85\x60\x9d\x34\xf8",
	  32}

	 },
	{
	 .name = "test9",
	 .sig =
	 {(unsigned char *)
	  "\x30\x46\x02\x21\x00\xbe\xa1\x01\x12\x64\x1d\x66\x5a\x68\x4a\xa0\xd5\x7e\x3e\x0c\x83\x51\xaa\x21\x9a\x0f\x7b\x38\xf0\xc0\x8b\xc5\xba\xfe\x25\x83\x51\x02\x21\x00\xfb\xea\x25\x74\x78\xd3\xaa\x91\x7b\xc8\x49\x26\x22\x26\xc7\x72\x6b\x25\xfd\x05\xac\x71\x5d\xeb\x1d\xc5\xaa\x4e\xc9\x6e\x34\xb5",
	  72},
	 .r =
	 {(unsigned char *)
	  "\x00\xbe\xa1\x01\x12\x64\x1d\x66\x5a\x68\x4a\xa0\xd5\x7e\x3e\x0c\x83\x51\xaa\x21\x9a\x0f\x7b\x38\xf0\xc0\x8b\xc5\xba\xfe\x25\x83\x51",
	  33},
	 .s =
	 {(unsigned char *)
	  "\x00\xfb\xea\x25\x74\x78\xd3\xaa\x91\x7b\xc8\x49\x26\x22\x26\xc7\x72\x6b\x25\xfd\x05\xac\x71\x5d\xeb\x1d\xc5\xaa\x4e\xc9\x6e\x34\xb5",
	  33}

	 }
};

void doit(void)
{
	unsigned i;

	for (i = 0; i < sizeof(encode_tests) / sizeof(encode_tests[0]); i++) {
		encode(encode_tests[i].name, &encode_tests[i].sig,
		       &encode_tests[i].r, &encode_tests[i].s);
	}
}
