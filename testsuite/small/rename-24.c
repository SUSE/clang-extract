/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_EXPORT_SYMBOLS=foo_it,bar_var" }*/

/* Check that a symbol written in an object-like macro as argument of a function-like macro is rewritten in place,
   with no macro being emitted for the externalized symbols.  */

#define S(x) x
#define T S(foo_it)() + S(bar_var)

extern int foo_it(void);
extern int bar_var;

int f(void)
{
  return T;
}

/* { dg-final { scan-tree-dump "#define T S\(\(\*klpe_foo_it\)\)\(\) \+ S\(\(\*klpe_bar_var\)\)" } } */
/* { dg-final { scan-tree-dump-not "#define foo_it" } } */
/* { dg-final { scan-tree-dump-not "#define bar_var" } } */
