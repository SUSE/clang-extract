/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_EXPORT_SYMBOLS=foo_it,bar_var" }*/

/* Check that references built by token pasting are externalized even when
   -DCE_MACRO_EXTERNALIZATION is not given.  */

#define GET_IT(x) (x##_it())
#define GET_VAR(x) (x##_var)

extern int foo_it(void);
extern int bar_var;

int f(void)
{
  return GET_IT(foo) + GET_VAR(bar);
}

/* { dg-final { scan-tree-dump "static int \(\*klpe_foo_it\)\(void\)( __attribute__\(\(used\)\))?;" } } */
/* { dg-final { scan-tree-dump "static int \*klpe_bar_var( __attribute__\(\(used\)\))?;" } } */
/* { dg-final { scan-tree-dump "#define foo_it\t\(\*klpe_foo_it\)" } } */
/* { dg-final { scan-tree-dump "#define bar_var\t\(\*klpe_bar_var\)" } } */
/* { dg-final { scan-tree-dump "return GET_IT\(foo\) \+ GET_VAR\(bar\);" } } */
/* { dg-final { scan-tree-dump-not "extern int foo_it\(void\);" } } */
/* { dg-final { scan-tree-dump-not "extern int bar_var;" } } */
