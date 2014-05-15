-- Copyright (c) 2009-2014 Free Software Foundation, Inc.
--
-- This file is part of GNU Zile.
--
-- This program is free software; you can redistribute it and/or modify it
-- under the terms of the GNU General Public License as published by
-- the Free Software Foundation; either version 3, or (at your option)
-- any later version.
--
-- This program is distributed in the hope that it will be useful, but
-- WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
-- General Public License for more details.
--
-- You should have received a copy of the GNU General Public License
-- along with this program.  If not, see <http://www.gnu.org/licenses/>.

--[[--
 Zemacs Lisp Evaluator.

 Extends the base ZLisp Interpreter with a func-table symbol type,
 that can be called like a regular function, but also contains its own
 metadata.  Additionally, like ELisp, we keep variables in their own
 namespace, and give each buffer it's own local list of variables for
 the various buffer-local variables we want to provide.

 Compared to the basic ZLisp Interpreter, this evaluator has to do
 a lot more work to keep an undo list, allow recording of keyboard
 macros and, again like Emacs, differentiate between interactive and
 non-interactive calls.

 @module zemacs.eval
]]



--[[ ======== ]]--
--[[ Imports. ]]--
--[[ ======== ]]--


local std     = require "std"

local lisp    = require "zile.zlisp"

local chomp   = std.string.chomp
local slurp   = std.io.slurp
local memoize = std.functional.memoize

--- Return a new Cons cell with supplied car and cdr.
-- @function __call
-- @param car first element
-- @param cdr last element
-- @treturn Cons a new cell containing those elements
local Cons = lisp.Cons


--- Call a function on every symbol in @{zile.Symbol.obarray}.
-- If `func` returns `true`, mapatoms returns immediately.
-- @function mapatoms
-- @func func a function that takes a symbol as its argument
-- @tparam[opt=obarray] table symtab a table with symbol values
-- @return `true` if `func` signalled early exit by returning `true`,
--   otherwise `nil`
local mapatoms = lisp.mapatoms


local consp, symbolp = lisp.consp, lisp.symbolp


--[[ ======================== ]]--
--[[ Symbol Table Management. ]]--
--[[ ======================== ]]--


-- Name mangling wrappers for Symbol methods.
-- FIXME: the process of mangling is only here so that the name of
--        a Symbol object as displayed from zemacs is '-' delimited,
--        but the obarray key shared by code in lib/zile/*.lua uses
--        Lua valid '_' delimited strings.  Zile code shouldn't be
--        relying on symbols defined in zemacs/zz, so after factoring
--        that away, we can get rid of the mangling step entirely.


--- Convert a '-' delimited symbol-name to be '_' delimited.
-- @function mangle
-- @string name a '-' delimited symbol-name
-- @treturn string `name` with all '-' transformed into '_'.
local mangle = memoize (function (name)
  return name and name:gsub ("%-", "_")
end)


--- Return a new interned @{symbol} initialised from the given arguments.
-- @string name symbol name
-- @tparam[opt=obarray] table symtab a table of @{symbol}s
--   interned
-- @treturn symbol interned symbol
local function intern (name, ...)
  return lisp.intern (mangle (name), ...)
end


--- Check whether `name` was previously interned.
-- @string name possibly interned name
-- @tparam[opt=obarray] table symtab a table of @{symbol}s
-- @treturn symbol symbol previously interned with `name`, else `nil`
local function intern_soft (name, ...)
  return lisp.intern_soft (mangle (name), ...)
end


local defsubr, eval_expression, marshaller -- forward declarations


------
-- A named symbol and associated metadata.
-- ZLisp symbols are _lisp-2_ style, with separate slots for a function
-- and variable value that can both be associated with the same name at
-- the same time; like elisp.
-- Property list keys can be any string - some internally used keys are:
-- `source-file`, `function-documentation`,`variable-documentation` and
-- `buffer-local-variable`.
-- @table symbol
-- @string name display-name of symbol
-- @func[opt=nil] func function slot
-- @field[opt=nil] value variable slot
-- @tfield[opt={}] table plist property list


--- Define a command in the execution environment for the evaluator.
-- @string name command name
-- @int min minimum number of arguments
-- @int max maximum number of arguments
-- @string doc docstring
-- @bool interactive `true` if this command can be called interactively
-- @func func function to call after marshalling arguments
-- @treturn symbol newly interned symbol
-- @local
function defsubr (name, min, max, doc, interactive, func)
  local symbol = intern (name)
  symbol.name  = name	-- unmangled name
  rawset (symbol, "func", func)
  symbol["marshall-argspec"]       = Cons (min, max)
  symbol["function-documentation"] = chomp (doc)
  symbol["interactive-form"]       = interactive
  getmetatable (symbol).__call = marshaller
  return symbol
end


--- Argument marshalling for zlisp function symbols.
-- Used as the `__call` metamethod for function symbols.
-- @tparam symbol symbol a symbol
-- @tparam zile.Cons arglist arguments for calling this function symbol
-- @return result of calling this function symbol
-- @local
function marshaller (symbol, arglist)
  local argspec = symbol["marshall-argspec"]

  local args = {}
  while arglist and arglist.car ~= nil do
    local arg = arglist.car
    if argspec.cdr ~= "UNEVAL" then arg = eval_expression (arg) end

    -- Lisp 't and 'nil are represented by Lua true and false.
    table.insert (args, arg or false)
    arglist = arglist.cdr
  end

  current_prefix_arg, prefix_arg = prefix_arg, false

  return symbol.func (unpack (args))
end



--[[ ==================== ]]--
--[[ Variable Management. ]]--
--[[ ==================== ]]--


--- Return a string representation of the value of a variable.
-- @tparam symbol symbol a symbol
-- @treturn string string representation, suitable for display
local function display_variable_value (symbol)
  local value = symbol.value
  if value == true then
    return "t"
  elseif value == false then
    return "nil"
  elseif type (value) == "string" then
    return '"' .. value .. '"'
  else
    return tostring (value)
  end
end


--- Define a new variable.
-- Store the value and docstring for a variable for later retrieval.
-- @string name variable name
-- @param value value to store in variable `name`
-- @string doc variable's docstring
-- @treturn symbol newly interned symbol
local function defvar (name, value, doc)
  local symbol = intern (name)
  symbol.name  = name	-- unmangled name
  symbol.value = value
  symbol["variable-documentation"] = chomp (doc or "")
  getmetatable (symbol).__tostring = display_variable_value
  return symbol
end


--- Set a variable's buffer-local behaviour.
-- Any variable marked this way becomes a buffer-local version of the
-- same when set in any way.
-- @string name variable name
-- @tparam bool bool `true` to mark buffer-local, `false` to unmark.
-- @treturn bool the new buffer-local status
local function set_variable_buffer_local (name, bool)
  local symbol = intern (name)
  symbol["buffer-local-variable"] = not not bool or nil
end


--- Return the variable symbol associated with name in buffer.
-- @string name variable name
-- @tparam[opt=current buffer] buffer bp buffer to select
-- @return the value of `name` from buffer `bp`
local function fetch_variable (name, bp)
  local obarray = (bp or cur_bp or {}).obarray
  return obarray and intern_soft (name, obarray) or intern_soft (name)
end


--- Return the value of a variable in a particular buffer.
-- @string name variable name
-- @tparam[opt=current buffer] buffer bp buffer to select
-- @return the value of `name` from buffer `bp`
local function get_variable (name, bp)
  return (fetch_variable (name, bp) or {}).value
end


--- Assign a value to variable in a given buffer.
-- @string symbol_or_name an interned symbol, or name of a symbol
-- @param value value to assign to `symbol_or_name`
-- @tparam[opt=current buffer] buffer bp buffer to select
-- @return the new value of `symbol_or_name` in buffer `bp`
local function set_variable (symbol_or_name, value, bp)
  local symbol = symbol_or_name
  if type (symbol) == "string" then
    symbol = intern (symbol_or_name)
  end

  if symbol and symbol["buffer-local-variable"] then
    bp = bp or cur_bp
    bp.obarray = bp.obarray or {}

    local bufferlocal = intern (symbol.name, bp.obarray)
    bufferlocal.name  = symbol.name	-- unmangled name
    bufferlocal.value = value
    getmetatable (bufferlocal).__index = symbol.plist
    getmetatable (bufferlocal).__tostring = display_variable_value

  elseif symbol then
    symbol.value = value
  end

  return value
end



--[[ ================ ]]--
--[[ ZLisp Evaluator. ]]--
--[[ ================ ]]--


--- Execute a function non-interactively.
-- @tparam symbol|string symbol_or_name symbol or name of function
--   symbol to execute
-- @param[opt=nil] uniarg a single non-table argument for `symbol_or_name`
local function execute_function (symbol_or_name, uniarg)
  local symbol, value = symbol_or_name

  if type (symbol_or_name) ~= "table" then
    symbol = intern_soft (symbol_or_name)
  end

  if uniarg ~= nil and type (uniarg) ~= "table" then
    uniarg = Cons (uniarg)
  end

  command.attach_label (nil)
  value = symbol and symbol (uniarg) or nil
  command.next_label ()

  return value
end


--- Call a zlisp command with arguments, interactively.
-- @tparam symbol symbol a value already passed to @{defsubr}
-- @tparam zile.Cons arglist arguments for `name`
-- @return the result of calling `name` with `arglist`, or else `nil`
local function call_command (symbol, arglist)
  thisflag = {defining_macro = lastflag.defining_macro}

  -- Execute the command.
  command.interactive_enter ()
  local value = execute_function (symbol, arglist)
  command.interactive_exit ()

  -- Only add keystrokes if we were already in macro defining mode
  -- before the function call, to cope with start-kbd-macro.
  if lastflag.defining_macro and thisflag.defining_macro then
    add_cmd_to_macro ()
  end

  if cur_bp and not command.was_labelled (":undo") then
    cur_bp.next_undop = cur_bp.last_undop
  end

  lastflag = thisflag

  return value
end


--- Evaluate a single command expression.
-- @tparam zile.Cons list a cons list, where the first element is a
--   command name.
-- @return the result of evaluating `list`, or else `nil`
local function eval_command (list)
  return list and list.car and call_command (list.car, list.cdr) or nil
end


--- Evaluate one arbitrary expression.
-- This function is required to implement ZLisp special forms, such as
-- `setq`, where some nodes of the AST are evaluated and others are not.
-- @tparam zile.Cons expr a lisp object or Cons list expression
-- @treturn zile.Cons the result of evaluating `expr`
-- @local
function eval_expression (expr)
  if symbolp (expr) then
    return get_variable (expr)
  elseif consp (expr) then
    -- FIXME: quote should be a special form, without special handling
    --        here.
    if expr.car == intern_soft "quote" then
      return expr.cdr.car
    end
    return execute_function (expr.car, expr.cdr)
  end
  return expr
end


--- Evaluate a string of zlisp code.
-- @string s zlisp source
-- @return `true` for success, or else `nil` plus an error string
local function eval_string (s)
  local ok, list = pcall (lisp.parse, s, mangle)
  if not ok then return nil, list end

  local value
  while list do
    value = eval_command (list.car)
    list = list.cdr
  end

  return value
end


--- Evaluate a file of zlisp.
-- @param file path to a file of zlisp code
-- @return `true` for success, or else `nil` plus an error string
local function eval_file (file)
  local s, errmsg = slurp (file)

  local value
  if s then
    value, errmsg = eval_string (s)
  end

  return value, errmsg
end


--- @export
return {
  call_command     = call_command,
  defsubr          = defsubr,
  defvar           = defvar,
  eval_expression  = eval_expression,
  eval_file        = eval_file,
  eval_string      = eval_string,
  execute_function = execute_function,
  fetch_variable   = fetch_variable,
  get_variable     = get_variable,
  intern_soft      = intern_soft,
  mapatoms         = mapatoms,
  set_variable     = set_variable,
  set_variable_buffer_local = set_variable_buffer_local,
}
