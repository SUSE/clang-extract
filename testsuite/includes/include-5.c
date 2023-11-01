/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION -DCE_KEEP_INCLUDES -DCE_EXPAND_INCLUDES=header-5.h" }*/

#include "header-5.h"

int f(void)
{
  return used_function();
}


/* { dg-final { scan-tree-dump "#include \"header-5-2.h\"" } } */
/* { dg-final { scan-tree-dump "return used_function\(\);" } } */
