/* { dg-options "-DCE_EXTRACT_FUNCTIONS=g -DCE_EXPORT_SYMBOLS=f" }*/

int (*f)(void);

int g(void)
{
  return f();
}

/* { dg-final { scan-tree-dump "static int \(\*\*klp_f\)\(void\);" } } */
/* { dg-final { scan-tree-dump "int g\(void\)\n{\n *return \(\*klp_f\)\(\)" } } */
