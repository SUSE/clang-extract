/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION -DCE_KEEP_INCLUDES" }*/

#include "header-8.h"

int should_not_be_here(void)
{
  return 1;
}

int f(void)
{
  return 0;
}

/* { dg-final { scan-tree-dump "#include \"header-8.h\"" } } */
/* { dg-final { scan-tree-dump "int f\(void\)\n\{" } } */
/* { dg-final { scan-tree-dump-not "int should_not_be_here" } } */
