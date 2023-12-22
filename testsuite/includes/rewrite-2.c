/* { dg-options "-DCE_EXTRACT_FUNCTIONS=g -DCE_KEEP_INCLUDES -DCE_EXPORT_SYMBOLS=f" }*/

#include "./header-1.h"

int g(void)
{
  return f();
}

/* { dg-final { scan-tree-dump "static int \(\*klpe_f\)\(void\);" } } */
/* { dg-final { scan-tree-dump "\(\*klpe_f\)\(\)" } } */
/* { dg-final { scan-tree-dump-not "#include \"header-1.h\"" } } */
