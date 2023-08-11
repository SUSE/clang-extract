/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION -DCE_KEEP_INCLUDES" }*/

#include "header-1.h"

int f(void)
{
  return 0;
}


/* { dg-final { scan-tree-dump "#include \"header-1.h\"" } } */
/* { dg-final { scan-tree-dump "return 0;" } } */
