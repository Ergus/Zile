#ifndef GETKEY_H
#define GETKEY_H

/*
 * Copyright (C) 2019  Jimmy Aguilar Mena
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* These are not required for POSIX.1-2001, and not always defined in
   the system headers. */
#ifndef timeradd
#  define timeradd(_a, _b, _result)				\
   do {								\
	(_result)->tv_sec  = (_a)->tv_sec + (_b)->tv_sec;	\
	(_result)->tv_usec = (_a)->tv_usec + (_b)->tv_usec;	\
	if ((_result)->tv_usec > 1000000) {			\
	  ++(_result)->tv_sec;					\
	  (_result)->tv_usec -= 1000000;			\
	}							\
   }                                                            \
   while (0)
#endif

#ifndef timercmp
#  define timercmp(_a, _b, _CMP)				\
   (((_a)->tv_sec == (_b)->tv_sec)				\
    ? ((_a)->tv_usec _CMP (_b)->tv_usec)			\
    : ((_a)->tv_sec  _CMP (_b)->tv_sec))
#endif

/* Maximum time to avoid screen updates when catching up with buffered
   input, in milliseconds. */
#define MAX_RESYNC_MS 500

void pushkey (size_t key);
void ungetkey (size_t key);
_GL_ATTRIBUTE_PURE size_t lastkey (void);
size_t getkeystroke (int delay);
size_t getkey (int delay);
size_t getkey_unfiltered (int delay);
void waitkey (void);
void init_getkey (void);
void destroy_getkey (void);


#endif
