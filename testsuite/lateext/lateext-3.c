/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_LATE_EXTERNALIZE -DCE_EXPORT_SYMBOLS=var -DCE_KEEP_INCLUDES" }*/

#include "lateext-3.h"

int f(void)
{
  return inline_func();
}

/* { dg-final { scan-tree-dump "static int \*klpe_var __attribute__\(\(used\)\);|__attribute__\(\(used\)\) static int \*klpe_var;" } } */
/* { dg-final { scan-tree-dump-not "#include \"lateext-3.h\"" } } */
