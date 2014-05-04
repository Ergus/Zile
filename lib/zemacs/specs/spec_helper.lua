-- Editor and buffer helpers.
-- Written by Gary V. Vaughan, 2014
--
-- Copyright (c) 2014 Free Software Foundation, Inc.
--
-- This file is part of GNU Zile.
--
-- GNU Zile is free software: you can redistribute it and/or modify
-- it under the terms of the GNU General Public License as published by
-- the Free Software Foundation, either version 3 of the License, or
-- (at your option) any later version.
--
-- GNU Zile is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.

local posix  = require "posix"
local std    = require "specl.std"

local Process = require "specl.shell".Process
local Object, escape_pattern, slurp =
      std.Object, std.string.escape_pattern, std.string.slurp

local EMACS  = os.getenv ("EMACSPROG")
local ZEMACS = "lib/zemacs/zemacs"

-- Don't overwrite _type, so we can also use Process matchers on
-- Editor objects.
local Editor = Process {
  _init = {"status", "output", "errout", "buffer", "minibuf"},
}


local function mktmpfile (content)
  local f = os.tmpname ()
  if content and content ~= "" then
    local h = io.open (f, "w")
    h:write (content .. "\n")
    h:close ()
  end
  return f
end


local function mklisp (self, lisp, fminibuf)
  if not self.batch_mode then
    lisp = lisp ..[[
      ;; copy *Messages* into msgbuf
      (set-buffer "*Messages*")
      (copy-region-as-kill (point-min) (point-max))
      (find-file "]] .. fminibuf .. [[")
      (yank)
    ]]
  end

  return lisp .. [[
    ;; save and exit
    (save-buffers-kill-emacs t)
  ]]
end


local function mkmacro (self, keystr, fminibuf)
  if not self.batch_mode then
    -- in batch mode messages go to stderr, otherwise we save
    -- a copy of *Messages* instead.
    keystr = keystr ..
      [[\C-xb*Messages*\r\C-@\M-<\M-xcopy-region-as-kill\r]] ..
      [[\C-x\C-f]] .. fminibuf .. [[\r\C-y]]
  end

  return [[(execute-kbd-macro "]] .. keystr:gsub ('"', '\\"') .. [[\C-u\C-x\C-c")]]
end


local function edit (self, code, bufcontent)
  local fout     = os.tmpname ()
  local ferr     = os.tmpname ()
  local fbuf     = mktmpfile (bufcontent or "")
  local fminibuf = mktmpfile ""

  self.batch_mode = self.args:match ("%-%-batch") ~= nil

  -- unless `code` begins with a `(`, assume it is a keystr
  local expandfn = not code:match "^%s*%(" and self.mkmacro or self.mklisp
  local fcode    = mktmpfile (expandfn (self, code, fminibuf))

  local redirects = ">" .. fout .. " 2>" .. ferr

  local status = posix.spawn (table.concat ({
    self.command, self.args, fbuf, "--load", fcode, redirects
  }, " "))

  local buffer = Editor {status, slurp (fout), slurp (ferr), slurp (fbuf), slurp (fminibuf)}

  os.remove (fcode)
  os.remove (fminibuf)
  os.remove (fbuf)
  os.remove (ferr)
  os.remove (fout)

  return buffer
end


Zemacs = Object {
  _type = "Zemacs",
  _init = {"args"},

  __index = {
    edit    = edit,
    mklisp  = mklisp,
    mkmacro = mkmacro,
  },

  --- @export
  command = ZEMACS,
}



--[[ ========= ]]--
--[[ Matchers. ]]--
--[[ ========= ]]--


-- Register matchers for editor buffer content.
do
  local matchers = require "specl.matchers"

  local concat, reformat, Matcher, matchers =
        matchers.concat, matchers.reformat, matchers.Matcher, matchers.matchers

  --- Show stderr after a failed expectation.
  -- @tparam Editor result of an edit
  -- @treturn string `editor` results formatted for display
  local function format_actual (self, editor)
    local m = ":" .. reformat (editor.buffer)
    if editor.minibuf and editor.minibuf ~= "" then
      return m .. "\nand *Messages*:" .. reformat (editor.minibuf)
    end
    return m
  end

  matchers.match_minibuf = Matcher {
    function (self, actual, pattern)
      return (string.match (actual.minibuf, pattern) ~= nil)
    end,

    actual_type   = "Process",

    format_actual = function (self, editor)
      return ":" .. reformat (editor.minibuf)
    end,

    format_expect = function (self, expect)
      return " minibuf matching:" .. reformat (expect)
    end,

    format_alternatives = function (self, adaptor, alternatives)
      return " minibuf matching:" .. reformat (alternatives, adaptor)
    end,
  }


  matchers.match_buffer = Matcher {
    function (self, actual, pattern)
      return (string.match (actual.buffer, pattern) ~= nil)
    end,

    actual_type   = "Process",

    format_actual = format_actual,

    format_expect = function (self, expect)
      return " buffer matching:" .. reformat (expect)
    end,

    format_alternatives = function (self, adaptor, alternatives)
      return " buffer matching:" .. reformat (alternatives, adaptor)
    end,
  }


  matchers.write_to_minibuf = Matcher {
    function (self, actual, expect)
      return string.match (actual.minibuf, escape_pattern (expect)) ~= nil
    end,

    actual_type   = "Process",

    format_actual = function (self, editor)
      return ":" .. reformat (editor.minibuf)
    end,

    format_expect = function (self, expect)
      return " minibuffer containing:" .. reformat (expect)
    end,

    format_alternatives = function (self, adaptor, alternatives)
      return " minibuffer containing:" .. reformat (alternatives, adaptor)
    end,
  }


  matchers.write_to_buffer = Matcher {
    function (self, actual, expect)
      return string.match (actual.buffer, escape_pattern (expect)) ~= nil
    end,

    actual_type   = "Process",

    format_actual = format_actual,

    format_expect = function (self, expect)
      return " buffer containing:" .. reformat (expect)
    end,

    format_alternatives = function (self, adaptor, alternatives)
      return " buffer containing:" .. reformat (alternatives, adaptor)
    end,
  }
end
