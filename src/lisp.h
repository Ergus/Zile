#ifndef LISP_H
#define LISP_H

typedef le * (*Function) (long uniarg, bool is_uniarg, le * list);

/* Turn a bool into a Lisp boolean */
#define bool_to_lisp(b) ((b) ? leT : leNIL)

void init_lisp (void);
void lisp_loadstring (astr as);
bool lisp_loadfile (const char *file);
bool lisp_to_number (const char *s, long *res);


#endif
