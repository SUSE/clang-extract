/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_LATE_EXTERNALIZE -DCE_EXPORT_SYMBOLS=g -DCE_KEEP_INCLUDES" }*/

#define MACRO int

#include "lateext-4.h"

MACRO f(void)
{
  return g();
}

/* { dg-final { scan-tree-dump "#include \"lateext-4.h\"" } } */
/* { dg-final { scan-tree-dump "static int \(\*klpe_g\)\(void\);" } } */
/* { dg-final { scan-tree-dump "return \(\*klpe_g\)\(\);" } } */
