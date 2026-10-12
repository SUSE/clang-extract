/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_EXPORT_SYMBOLS=foo_it,bar_var" }*/

/* Check that a token pasting that results in a macro expanding to the symbol is rewritten in place,
   with no macro being emitted for the externalized symbols.  */

#define foo_alias foo_it
#define bar_alias bar_var
#define K(x) x##_alias

extern int foo_it(void);
extern int bar_var;

int f(void)
{
  return K(foo)() + K(bar);
}

/* { dg-final { scan-tree-dump "#define foo_alias \(\*klpe_foo_it\)" } } */
/* { dg-final { scan-tree-dump "#define bar_alias \(\*klpe_bar_var\)" } } */
/* { dg-final { scan-tree-dump-not "#define foo_it" } } */
/* { dg-final { scan-tree-dump-not "#define bar_var" } } */
