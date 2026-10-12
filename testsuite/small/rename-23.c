/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_EXPORT_SYMBOLS=foo_it,bar_var" }*/

/* Check that a symbol passed through nested function-like macros is rewritten in place,
   with no macro being emitted for the externalized symbols.  */

#define F(x) (x)
#define J(x) F(x)

extern int foo_it(void);
extern int bar_var;

int f(void)
{
  return J(foo_it()) + J(bar_var);
}

/* { dg-final { scan-tree-dump "return J\(\(\*klpe_foo_it\)\(\)\) \+ J\(\(\*klpe_bar_var\)\);" } } */
/* { dg-final { scan-tree-dump-not "#define foo_it" } } */
/* { dg-final { scan-tree-dump-not "#define bar_var" } } */
