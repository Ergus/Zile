-- Point facility functions
--
-- Copyright (c) 2010 Free Software Foundation, Inc.
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

function point_new ()
  return {o = 0, n = 0, p = {}}
end

-- FIXME: Remove p member of Point, and use estr functions directly on o member instead.
function make_point (lineno, offset)
  local pt = point_new ()
  pt.p = cur_bp.lines
  pt.n = lineno
  pt.o = offset
  for i = lineno, 1, -1 do
    pt.p = get_line_next (pt.p)
  end
  assert (pt.p)
  return pt
end

function offset_to_point (bp, offset)
  local pt = {p = bp.lines, n = 0}
  assert (pt.p)
  while offset > 0 and offset > #get_line_text (pt.p) do
    offset = offset - #get_line_text (pt.p) - #get_buffer_text (bp).eol
    pt.p = get_line_next (pt.p)
    assert (pt.p)
    pt.n = pt.n + 1
  end
  pt.o = offset
  return pt
end

function point_min ()
  return make_point (0, 0)
end

function point_max ()
  local pt = make_point (cur_bp.last_line, 0)
  pt.o = #get_line_text (pt.p)
  return pt
end

function cmp_point (pt1, pt2)
  if pt1.n < pt2.n then
    return -1
  elseif pt1.n > pt2.n then
    return 1
  end
  return (pt1.o < pt2.o) and -1 or ((pt1.o > pt2.o) and 1 or 0)
end

function line_beginning_position (count)
  -- Copy current point position without offset (beginning of line).
  local pt = table.clone (cur_bp.pt)
  pt.o = 0

  count = count - 1
  while count < 0 and pt.n > 0 do
    pt.p = get_line_prev (pt.p)
    pt.n = pt.n - 1
    count = count + 1
  end

  while count > 0 and pt.n < cur_bp.last_line do
    pt.p = get_line_next (pt.p)
    pt.n = pt.n + 1
    count = count - 1
  end

  return pt
end

function line_end_position (count)
  local pt = line_beginning_position (count)
  pt.o = #get_line_text (pt.p)
  return pt
end

-- Go to coordinates described by pt (ignoring pt.p)
function goto_point (pt)
  if cur_bp.pt.n > pt.n then
    repeat
      execute_function ("previous-line")
    until cur_bp.pt.n == pt.n
  elseif cur_bp.pt.n < pt.n then
    repeat
      execute_function ("next-line")
    until cur_bp.pt.n == pt.n
  end

  if cur_bp.pt.o > pt.o then
    repeat
      execute_function ("backward-char")
    until cur_bp.pt.o == pt.o
  elseif cur_bp.pt.o < pt.o then
    repeat
      execute_function ("forward-char")
    until cur_bp.pt.o == pt.o
  end
end
