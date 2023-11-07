/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION -DCE_KEEP_INCLUDES -DCE_EXPAND_INCLUDES=assert.h" }*/

int variable;

#include <assert.h>
#include "static-assert-2.h"

int f()
{
  return g();
}

/* { dg-final { scan-tree-dump "#include \"static-assert-2.h\"" } } */
/* { dg-final { scan-tree-dump "int variable;" } } */
