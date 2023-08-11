/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION -DCE_KEEP_INCLUDES" }*/

#include "header-2.h"

int f(void)
{
  return MACRO;
}

/* { dg-final { scan-tree-dump "#include \"header-2.h\"" } } */
/* { dg-final { scan-tree-dump "return MACRO;" } } */
