
#ifndef MOUSE_H
#define MOUSE_H

#include <config.h>

void
mouse_enable ();

void
mouse_disable ();

size_t
mouse_codetokey ();

bool
mouse_keytocodes (int *p);

astr
mouse_chordtodesc (size_t key);

#endif
