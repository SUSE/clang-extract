/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_EXPORT_SYMBOLS=foo_it,bar_var" }*/

/* Check that a symbol in macro body pasted with an empty argument is rewritten in place,
   with no macro being emitted for the externalized symbols.  */

#define I(x) foo_it ## x () + bar_var ## x

extern int foo_it(void);
extern int bar_var;

int f(void)
{
  return I();
}

/* { dg-final { scan-tree-dump "#define I\(x\) \(\*klpe_foo_it\) ## x \(\) \+ \(\*klpe_bar_var\) ## x" } } */
/* { dg-final { scan-tree-dump-not "#define foo_it" } } */
/* { dg-final { scan-tree-dump-not "#define bar_var" } } */
