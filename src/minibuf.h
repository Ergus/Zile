#ifndef MINIBUF_H
#define MINIBUF_H

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
#include <stdbool.h>

#include "gl_xlist.h"

#include "astr.h"
#include "completion.h"

void init_minibuf (void);
_GL_ATTRIBUTE_PURE bool minibuf_no_error (void);
void minibuf_refresh (void);
void minibuf_write (const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void minibuf_error (const char *fmt, ...) __attribute__((format(printf, 1, 2)));
const_astr minibuf_read (const char *fmt, const char *value, ...) __attribute__((format(printf, 1, 3)));
long minibuf_read_number (const char *fmt, ...) __attribute__((format(printf, 1, 2)));

bool minibuf_test_in_completions (const char *ms, gl_list_t completions);
int minibuf_read_yesno (const char *fmt, ...) __attribute__((format(printf, 1, 2)));

const_astr minibuf_read_completion (const char *fmt, const char *value, Completion cp,
                                    History hp, ...) __attribute__((format(printf, 1, 5)));

const_astr minibuf_vread_completion (const char *fmt, const char *value, Completion cp,
                                     History hp, const char *empty_err,
                                     bool (*test) (const char *s, gl_list_t completions),
                                     const char *invalid_err, va_list ap) __attribute__((format(printf, 1, 0)));
const_astr minibuf_read_filename (const char *fmt, const char *value,
                                  const char *file, ...) __attribute__((format(printf, 1, 4)));
void minibuf_clear (void);


#endif
