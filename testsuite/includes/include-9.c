/* { dg-options "-DCE_EXTRACT_FUNCTIONS=g -DCE_NO_EXTERNALIZATION -DCE_KEEP_INCLUDES -DCE_EXPAND_INCLUDES=other/other-header-1.h" }*/

#include "other/other-header-1.h"

int g(void)
{
  return f();
}

/* { dg-final { scan-tree-dump "int f\(void\)" } } */
/* { dg-final { scan-tree-dump "return f\(\);" } } */
/* { dg-final { scan-tree-dump-not "#include \"other-header-2.h\"" } } */
