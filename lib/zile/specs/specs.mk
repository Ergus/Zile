# Specl specs make rules.
# Writter by Gary V. Vaughan, 2014
#
# Copyright (c) 2014 Free Software Foundation, Inc.
#
# This file is part of GNU Zile.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.


## ------ ##
## Specs. ##
## ------ ##

zile_specdir = $(srcdir)/lib/zile/specs

specl_SPECS +=						\
	$(zile_specdir)/Array_spec.yaml			\
	$(zile_specdir)/MutableString_spec.yaml		\
	$(NOTHING_ELSE)

EXTRA_DIST +=						\
	$(zile_specdir)/spec_helper.lua			\
	$(NOTHING_ELSE)
