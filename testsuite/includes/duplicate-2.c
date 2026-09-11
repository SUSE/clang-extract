/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION -DCE_KEEP_INCLUDES" }*/

#include "header-2.h"
#include "header-2.h"

int x;

int f(void)
{
  return x;
}

/* { dg-final { scan-tree-dump "#include \"header-2.h\"" } } */
/* { dg-final { scan-tree-dump-not "#include \"header-2.h\"\n#include \"header-2.h\"" } } */
