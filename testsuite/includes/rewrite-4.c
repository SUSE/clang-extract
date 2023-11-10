/* { dg-options "-DCE_EXTRACT_FUNCTIONS=g -DCE_KEEP_INCLUDES -DCE_EXPORT_SYMBOLS=f -include header-1.h" }*/

int g(void)
{
  return f();
}

/* { dg-final { scan-tree-dump "static int \(\*klp_f\)\(void\);" } } */
/* { dg-final { scan-tree-dump "klp_f\(\)" } } */
