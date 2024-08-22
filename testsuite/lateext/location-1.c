/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_EXPORT_SYMBOLS=global -DCE_KEEP_INCLUDES -DCE_LATE_EXTERNALIZE" }*/

#include "location-1.h"

int f(void)
{
  return global;
}

/* { dg-final { scan-tree-dump "static int \*klpe_global __attribute__\(\(used\)\);" } } */
/* { dg-final { scan-tree-dump "clang-extract: from .*location-1.h:29:1" } } */
