/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_EXPORT_SYMBOLS=foo_it,bar_var" }*/

/* Check that a symbol passed through __VA_ARGS__ and __VA_OPT__ is rewritten in place,
   with no macro being emitted for the externalized symbols.  */

#define V(...) (0 __VA_OPT__(+) __VA_ARGS__)

extern int foo_it(void);
extern int bar_var;

int f(void)
{
  return V(foo_it()) + V(bar_var);
}

/* { dg-final { scan-tree-dump "return V\(\(\*klpe_foo_it\)\(\)\) \+ V\(\(\*klpe_bar_var\)\);" } } */
/* { dg-final { scan-tree-dump-not "#define foo_it" } } */
/* { dg-final { scan-tree-dump-not "#define bar_var" } } */
