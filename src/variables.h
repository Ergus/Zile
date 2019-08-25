#ifndef VARIABLES_H
#define VARIABLES_H

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

void init_variables (void);
_GL_ATTRIBUTE_FORMAT_PRINTF(1, 2) const_astr minibuf_read_variable_name (const char *fmt, ...);
void set_variable (const char *var, const char *val);
const char *get_variable_doc (const char *var, const char **defval);
const char *get_variable (const char *var);
const char *get_variable_bp (Buffer bp, const char *var);
bool get_variable_bool (const char *var);
void toggle_variable_bool (const char *var);


#endif
