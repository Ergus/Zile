/*-------------------------------------------------------------------------
 *
 * memcmp.c
 *  compares memory bytes
 *
 * Portions Copyright (c) 1996-2002, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * This file was taken from NetBSD
 *
 *-------------------------------------------------------------------------
 */

#include <string.h>

/*
 * Compare memory regions.
 */
int memcmp(const void *s1, const void *s2, size_t n)
{
  if (n) {
    const unsigned char *p1 = s1, *p2 = s2;
    do {
      if (*p1++ != *p2++)
        return (*--p1 - *--p2);
    } while (--n);
  }
  return 0;
}
