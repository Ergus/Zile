-- Produce dotzile.sample
--
-- Copyright (c) 2006, 2007, 2009 Free Software Foundation, Inc.
--
-- This file is part of GNU Zile.
--
-- GNU Zile is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3, or (at your option)
-- any later version.
--
-- GNU Zile is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with GNU Zile; see the file COPYING.  If not, write to the
-- Free Software Foundation, Fifth Floor, 51 Franklin Street, Boston,
-- MA 02111-1301, USA.

prog = {
  name = "mkvars"
}

require "lib"
require "texinfo"

h = io.open ("dotzile.sample", "w")
assert (h)

h:write ("; ." .. PACKAGE .. " configuration\n\n")
-- Don't note where the contents of this file comes from or that it's
-- auto-generated, because it's ugly in a user configuration file.

local vars = {}
dofile (arg[1])
for var, val in pairs (main_vars) do
  h:write ("; " .. string.gsub (texi (val.doc), "(\n", "\n; ") .. "\n")
  h:write ("(setq " .. var .. " " .. val.val .. ")\n")
  h:write ("\n")
end

h:close ()
