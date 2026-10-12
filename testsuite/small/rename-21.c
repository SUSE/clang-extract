/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_EXPORT_SYMBOLS=foo_it,bar_var" }*/

/* Check that a symbol passed as a macro argument is rewritten in place,
   with no macro being emitted for the externalized symbols.  */

#define F(x) (x)

extern int foo_it(void);
extern int bar_var;

int f(void)
{
  return F(foo_it()) + F(bar_var);
}

/* { dg-final { scan-tree-dump "return F\(\(\*klpe_foo_it\)\(\)\) \+ F\(\(\*klpe_bar_var\)\);" } } */
/* { dg-final { scan-tree-dump-not "#define foo_it" } } */
/* { dg-final { scan-tree-dump-not "#define bar_var" } } */
