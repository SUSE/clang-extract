/* { dg-options "-DCE_EXTRACT_FUNCTIONS=g -DCE_KEEP_INCLUDES" }*/

#include "header-1.h"

int g(void)
{
  return f();
}

/* { dg-final { scan-tree-dump "static int \(\*klp_f\)\(void\);" } } */
/* { dg-final { scan-tree-dump "klp_f\(\)" } } */
/* { dg-final { scan-tree-dump-not "#include \"header-1.h\"" } } */