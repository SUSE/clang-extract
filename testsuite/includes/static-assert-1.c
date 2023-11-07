/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION -DCE_KEEP_INCLUDES" }*/

int variable;

#include "static-assert-1.h"

int f()
{
  return g();
}

/* { dg-final { scan-tree-dump "#include \"static-assert-1.h\"" } } */
/* { dg-final { scan-tree-dump "int variable;" } } */
