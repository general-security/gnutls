/* gc-nettle.c --- Crypto wrappers around Nettle for GC.
 * Copyright (C) 2002, 2003, 2004  Simon Josefsson
 *
 * This file is part of GC.
 *
 * GC is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * GC is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Lesser General
 * Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License License along with GC; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 *
 */

/* Note: This file is only built if GC uses Nettle. */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdlib.h>

/* Get prototype. */
#include <gc.h>

/* For randomize. */
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* XXX GnuTLS still need Libgcrypt. */
#include <gcrypt.h>

int
gc_init (void)
{
  gcry_error_t err;

  /* XXX GnuTLS still need Libgcrypt. */

  err = gcry_control (GCRYCTL_ANY_INITIALIZATION_P);
  if (err == GPG_ERR_NO_ERROR)
    {
      if (gcry_check_version (GCRYPT_VERSION) == NULL)
	return GC_INIT_ERROR;

      err = gcry_control (GCRYCTL_INITIALIZATION_FINISHED, NULL, 0);
      if (err != GPG_ERR_NO_ERROR)
	return GC_INIT_ERROR;
    }

  return GC_OK;
}

void
gc_done (void)
{
  return;
}

/* Randomness. */

static int
randomize (int level, char *data, size_t datalen)
{
  int fd;
  char *device;
  size_t len = 0;
  int rc;

  switch (level)
    {
    case 0:
      device = NAME_OF_NONCE_DEVICE;
      break;

    case 1:
      device = NAME_OF_PSEUDO_RANDOM_DEVICE;
      break;

    default:
      device = NAME_OF_RANDOM_DEVICE;
      break;
    }

  fd = open (device, O_RDONLY);
  if (fd < 0)
    return GC_RANDOM_ERROR;

  do
    {
      ssize_t tmp;

      tmp = read (fd, data, datalen);

      if (tmp < 0)
	return GC_RANDOM_ERROR;

      len += tmp;
    }
  while (len < datalen);

  rc = close (fd);
  if (rc < 0)
    return GC_RANDOM_ERROR;

  return GC_OK;
}

int
gc_nonce (char *data, size_t datalen)
{
  return randomize (0, data, datalen);
}

int
gc_pseudo_random (char *data, size_t datalen)
{
  return randomize (1, data, datalen);
}

int
gc_random (char *data, size_t datalen)
{
  return randomize (2, data, datalen);
}

/* Memory allocation. */

void
gc_set_allocators (gc_malloc_t func_malloc,
		   gc_malloc_t secure_malloc,
		   gc_secure_check_t secure_check,
		   gc_realloc_t func_realloc, gc_free_t func_free)
{
  /* XXX GnuTLS still need Libgcrypt. */
  gcry_set_allocation_handler (func_malloc, secure_malloc, secure_check,
			       func_realloc, func_free);
}

/* Ciphers. */

#include "nettle-meta.h"
#include "aes.h"

#define MAX_BLOCK_SIZE 64

typedef struct cipher_info
{
  int alg;
  int mode;
  const struct nettle_cipher *info;
  void *encrypt_context;
  void *decrypt_context;
  char encrypt_iv[MAX_BLOCK_SIZE];
  char decrypt_iv[MAX_BLOCK_SIZE];
} cinfo;

int
gc_cipher_open (int alg, int mode, gc_cipher * outhandle)
{
  cinfo *cinf;

  cinf = malloc (sizeof (*cinf));
  if (!cinf)
    return GC_MALLOC_ERROR;

  cinf->alg = alg;
  cinf->mode = mode;

  switch (alg)
    {
    case GC_AES256:
      cinf->info = &nettle_aes256;
      break;

    case GC_AES128:
      cinf->info = &nettle_aes128;
      break;

    case GC_3DES:
      cinf->info = &nettle_des3;
      break;

    case GC_DES:
      cinf->info = &nettle_des;
      break;

    case GC_ARCFOUR128:
    case GC_ARCFOUR40:
      cinf->info = &nettle_arcfour128;
      break;

    case GC_ARCTWO40:
      cinf->info = &nettle_pkcs12_arctwo40;
      break;

    default:
      free (cinf);
      return GC_INVALID_CIPHER;
    }

  cinf->encrypt_context = malloc (cinf->info->context_size);
  if (!cinf->encrypt_context)
    {
      free (cinf);
      return GC_MALLOC_ERROR;
    }

  cinf->decrypt_context = malloc (cinf->info->context_size);
  if (!cinf->decrypt_context)
    {
      free (cinf->encrypt_context);
      free (cinf);
      return GC_MALLOC_ERROR;
    }

  memset (cinf->encrypt_context, 0, cinf->info->context_size);
  memset (cinf->decrypt_context, 0, cinf->info->context_size);

  *outhandle = cinf;

  return GC_OK;
}

int
gc_cipher_setkey (gc_cipher handle, size_t keylen, const char *key)
{
  cinfo *cinf = (cinfo *) handle;

  cinf->info->set_encrypt_key (cinf->encrypt_context, keylen, key);
  cinf->info->set_decrypt_key (cinf->decrypt_context, keylen, key);

  return GC_OK;
}

int
gc_cipher_setiv (gc_cipher handle, size_t ivlen, const char *iv)
{
  cinfo *cinf = (cinfo *) handle;

  if (ivlen != cinf->info->block_size)
    return GC_INVALID_CIPHER;

  memcpy (cinf->encrypt_iv, iv, ivlen);
  memcpy (cinf->decrypt_iv, iv, ivlen);

  return GC_OK;
}

int
gc_cipher_encrypt_inline (gc_cipher handle, size_t len, char *data)
{
  cinfo *cinf = (cinfo *) handle;

  if (cinf->mode == GC_CBC)
    cbc_encrypt (cinf->encrypt_context, cinf->info->encrypt,
		 cinf->info->block_size, cinf->encrypt_iv, len, data, data);
  else
    cinf->info->encrypt (cinf->encrypt_context, len, data, data);

  return GC_OK;
}

int
gc_cipher_decrypt_inline (gc_cipher handle, size_t len, char *data)
{
  cinfo *cinf = (cinfo *) handle;

  if (cinf->mode == GC_CBC)
    cbc_decrypt (cinf->decrypt_context, cinf->info->decrypt,
		 cinf->info->block_size, cinf->decrypt_iv, len, data, data);
  else
    cinf->info->decrypt (cinf->decrypt_context, len, data, data);

  return GC_OK;
}

int
gc_cipher_close (gc_cipher handle)
{
  cinfo *cinf = (cinfo *) handle;

  free (cinf->encrypt_context);
  free (cinf->decrypt_context);
  free (cinf);

  return GC_OK;
}

/* Hashes. */

#include "nettle-meta.h"
#include "hmac.h"
#include "md5.h"
#include "sha.h"

#define MAX_DIGEST_SIZE 20

typedef struct hash_info
{
  int alg;
  int mode;
  const struct nettle_hash *info;
  void *context;
  void *outer;
  void *inner;
  char digest[MAX_DIGEST_SIZE];
} hinfo;

int
gc_hash_open (int hash, int mode, gc_hash * outhandle)
{
  hinfo *hinf;

  hinf = malloc (sizeof (*hinf));
  if (!hinf)
    return GC_MALLOC_ERROR;

  hinf->alg = hash;
  hinf->mode = mode;

  switch (hash)
    {
    case GC_MD5:
      hinf->info = &nettle_md5;
      break;

    case GC_SHA1:
      hinf->info = &nettle_sha1;
      break;

      /* FIXME: RMD160. */

    default:
      free (hinf);
      return GC_INVALID_HASH;
    }

  hinf->context = malloc (hinf->info->context_size);
  if (!hinf->context)
    {
      free (hinf);
      return GC_MALLOC_ERROR;
    }

  if (mode == GC_HMAC)
    {
      hinf->outer = malloc (hinf->info->context_size);
      if (!hinf->outer)
	{
	  free (hinf->context);
	  free (hinf);
	  return GC_MALLOC_ERROR;
	}

      hinf->inner = malloc (hinf->info->context_size);
      if (!hinf->inner)
	{
	  free (hinf->outer);
	  free (hinf->context);
	  free (hinf);
	  return GC_MALLOC_ERROR;
	}
    }
  else
    hinf->inner = hinf->outer = NULL;

  hinf->info->init (hinf->context);

  *outhandle = hinf;

  return GC_OK;
}

int
gc_hash_clone (gc_hash handle, gc_hash * outhandle)
{
  hinfo *oldhinf = (hinfo *) handle;
  hinfo *newhinf;

  newhinf = malloc (sizeof (*newhinf));
  if (!newhinf)
    return GC_MALLOC_ERROR;

  newhinf->alg = oldhinf->alg;
  newhinf->mode = oldhinf->mode;
  newhinf->info = oldhinf->info;

  newhinf->context = malloc (newhinf->info->context_size);
  if (!newhinf->context)
    {
      free (newhinf);
      return GC_MALLOC_ERROR;
    }

  if (oldhinf->mode == GC_HMAC)
    {
      newhinf->inner = malloc (newhinf->info->context_size);
      if (!newhinf->inner)
	{
	  free (newhinf->context);
	  free (newhinf);
	  return GC_MALLOC_ERROR;
	}

      newhinf->outer = malloc (newhinf->info->context_size);
      if (!newhinf->outer)
	{
	  free (newhinf->inner);
	  free (newhinf->context);
	  free (newhinf);
	  return GC_MALLOC_ERROR;
	}

      memcpy (newhinf->inner, oldhinf->inner, newhinf->info->context_size);
      memcpy (newhinf->outer, oldhinf->outer, newhinf->info->context_size);
    }

  memcpy (newhinf->context, oldhinf->context, newhinf->info->context_size);
  memcpy (newhinf->digest, oldhinf->digest, MAX_DIGEST_SIZE);

  *outhandle = newhinf;

  return GC_OK;
}

size_t
gc_hash_digest_length (int hash)
{
  switch (hash)
    {
    case GC_MD5:
      return MD5_DIGEST_SIZE;
      break;

    case GC_SHA1:
      return SHA1_DIGEST_SIZE;
      break;

    case GC_RMD160:
      return /* FIXME */ 20;
      break;

    default:
      break;
    }

  return 0;
}

void
gc_hash_hmac_setkey (gc_hash handle, size_t len, const char *key)
{
  hinfo *hinf = (hinfo *) handle;

  hmac_set_key (hinf->outer, hinf->inner, hinf->context,
		hinf->info, (unsigned) len, key);
}

void
gc_hash_write (gc_hash handle, size_t len, const char *data)
{
  hinfo *hinf = (hinfo *) handle;

  hinf->info->update (hinf->context, (size_t) len, data);
}

const char *
gc_hash_read (gc_hash handle)
{
  hinfo *hinf = (hinfo *) handle;

  if (hinf->mode == GC_HMAC)
    hmac_digest (hinf->outer, hinf->inner, hinf->context,
		 hinf->info, (unsigned) hinf->info->digest_size,
		 hinf->digest);
  else
    hinf->info->digest (hinf->context, (unsigned) hinf->info->digest_size,
			hinf->digest);

  return hinf->digest;
}

void
gc_hash_close (gc_hash handle)
{
  hinfo *hinf = (hinfo *) handle;

  if (hinf->mode == GC_HMAC)
    {
      free (hinf->inner);
      free (hinf->outer);
    }
  free (hinf->context);
  free (hinf);
}

int
gc_hash_buffer (int hash, const char *in, size_t inlen, char *out)
{
  switch (hash)
    {
    case GC_MD5:
      {
	struct md5_ctx md5;

	md5_init (&md5);
	md5_update (&md5, inlen, in);
	md5_digest (&md5, GC_MD5_LEN, out);
      }
      break;

    case GC_SHA1:
      {
	struct sha1_ctx sha1;

	sha1_init (&sha1);
	sha1_update (&sha1, inlen, in);
	sha1_digest (&sha1, GC_SHA1_LEN, out);
      }
      break;

      /* FIXME: RMD160. */

    default:
      return GC_INVALID_HASH;
    }

  return GC_OK;
}

/**
 * gc_md5:
 * @in: input character array of data to hash.
 * @inlen: length of input character array of data to hash.
 * @out: newly allocated character array with hash of data.
 *
 * Compute hash of data using MD5.  The @out buffer must be
 * deallocated by the caller.
 *
 * Return value: Returns %GC_OK iff successful.
 **/
int
gc_md5 (const char *in, size_t inlen, char out[GC_MD5_LEN])
{
  struct md5_ctx md5;

  md5_init (&md5);
  md5_update (&md5, inlen, in);
  md5_digest (&md5, GC_MD5_LEN, out);

  return GC_OK;
}

/**
 * gc_hmac_md5:
 * @key: input character array with key to use.
 * @keylen: length of input character array with key to use.
 * @in: input character array of data to hash.
 * @inlen: length of input character array of data to hash.
 * @outhash: newly allocated character array with keyed hash of data.
 *
 * Compute keyed checksum of data using HMAC-MD5.  The @outhash buffer
 * must be deallocated by the caller.
 *
 * Return value: Returns %GC_OK iff successful.
 **/
int
gc_hmac_md5 (const char *key, size_t keylen,
	     const char *in, size_t inlen,
	     char outhash[GC_MD5_LEN])
{
  struct hmac_md5_ctx ctx;

  hmac_md5_set_key (&ctx, keylen, key);
  hmac_md5_update (&ctx, inlen, in);
  hmac_md5_digest (&ctx, GC_MD5_LEN, outhash);

  return GC_OK;
}
