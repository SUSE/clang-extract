/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_EXPORT_SYMBOLS=foo_it,bar_var" }*/

/* Check that a symbol pasted with an empty left-hand side is rewritten in place,
   with no macro being emitted for the externalized symbols.  */

#define H(x, y) x##y

extern int foo_it(void);
extern int bar_var;

int f(void)
{
  return H(,foo_it)() + H(,bar_var);
}

/* { dg-final { scan-tree-dump "return H\(,\(\*klpe_foo_it\)\)\(\) \+ H\(,\(\*klpe_bar_var\)\);" } } */
/* { dg-final { scan-tree-dump-not "#define foo_it" } } */
/* { dg-final { scan-tree-dump-not "#define bar_var" } } */
