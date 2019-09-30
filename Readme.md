
## **IMPORTANT!: ** 

I abandoned this fork because I found a more functional and complete
alternative: [qemacs](https://github.com/Ergus/Qemacs) and I am
working there.

Qemacs has the following advantages:

1. Does not depend of `gnulib` so not enforced to use autotools or
   other hacks to make it work with cmake.
   
2. Does not need garbage collection, so no `libgc` is needed as a
   dependency.
   
3. Modular design and easy to extend with pure C. (It can load dynamic
   libraries at runtime as modules)

4. 100% C code, no pseudo lisp processor and special macros needed to
   expose to lisp.

5. Vertical split and syntax highlight implemented.

6. dired, term, org and some other modes already implemented and working.

7. Optional X11 integration to provide a gui interface. But can be
   disabled and the terminal version provides a very good experience.

8. The internal buffer storage in memory and memory management is much
   more efficient.
   
9. Available and easier to use in more systems (include MSWindows and
   Haiku).
   
10. Faster to build (as it does not need to build `gnulib` and their
    examples)

GNU Zile
========

GNU Zile is free software, licensed under the GNU GPL.

Copyright (c) 1997-2016 Free Software Foundation, Inc.

**Copying and distribution of this file, with or without modification,
are permitted in any medium without royalty provided the copyright
notice and this notice are preserved.**

------------------------------------------------------------------------

INTRODUCTION
============

GNU Zile (_Zile Is Lossy Emacs_) is a lightweight [Emacs][] clone. Every
Emacs user should feel at home with Zile. Zile is aimed at small
footprint systems (a typical binary is about 100Kb) and quick editing
sessions (it starts up and shuts down instantly).

Zile is written in **ISO C99** using **POSIX.1-2008** APIs, using
[libgc][]. It can be built on many other platforms thanks to its use
of [gnulib][] (version 7.2 or later seems to be required for testing
with Valgrind, but older versions seem to work fine for normal use).

  [Emacs]:   http://www.gnu.org/s/emacs
  [libgc]:   http://www.hpl.hp.com/personal/Hans_Boehm/gc
  [gnulib]:  http://www.gnu.org/s/gnulib

Source Layout
-------------

 * See file `AUTHORS` for the names of maintainers past and present.
 * See file `FAQ` for a selection of a Frequently Answered Questions.
 * See file `NEWS` for a list of major changes in each Zile release.
 * See file `THANKS` for a list of important contributors.

 * Directory `build-aux` contains helper scripts used to build Zile.
 * Directory `doc` contains files used to create Zile's documentation.
 * Directory `gnulib` contains [gnulib]() source modules used by Zile.
 * Directory `src` contains the source code used to build Zile.
 * Directory `tests` contains a small suite of tests used by


Web Pages
---------

Original GNU Zile's home page is at: <http://www.gnu.org/s/zile/>

GNU Zile development is co-ordinated from Zile's project page at GNU
Savannah: <http://savannah.gnu.org/projects/zile/>

OBTAINING THE LATEST SOURCES
============================

To install this version of Zile you need first to get the sources and
the gnulib sources:

```shell
git clone https://github.com/Ergus/Zile.git
cd Zile
git submodule update --init --recursive

```

With the sources you can now build Zile. You need cmake to do so and the
dependencies mentioned before: **libgc**.

```shell
mkdir build
cd build
cmake ..
make
```

The resulting Zile executable will be located inside src.
