/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_EXPORT_SYMBOLS=foo_it,bar_var" }*/

/* Check that a symbol written in an object-like macro body is rewritten in place,
   with no macro being emitted for the externalized symbols.  */

#define G (foo_it() + bar_var)

extern int foo_it(void);
extern int bar_var;

int f(void)
{
  return G;
}

/* { dg-final { scan-tree-dump "#define G \(\(\*klpe_foo_it\)\(\) \+ \(\*klpe_bar_var\)\)" } } */
/* { dg-final { scan-tree-dump-not "#define foo_it" } } */
/* { dg-final { scan-tree-dump-not "#define bar_var" } } */
