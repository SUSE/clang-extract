/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION -DCE_KEEP_INCLUDES" }*/

#include "header-with-hg.h"

int x;

#include "header-with-hg.h"

int f(void)
{
  return x;
}

/* { dg-final { scan-tree-dump "#include \"header-with-hg.h\"" } } */
/* { dg-final { scan-tree-dump "int x;\n\n\/\*\* clang-extract" } } */
