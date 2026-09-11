/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION -DCE_KEEP_INCLUDES -DCE_NO_DUPLICATED_INCLUDES" }*/

#include "header-2.h"

int x;

#include "header-2.h"

int f(void)
{
  return x;
}

/* { dg-final { scan-tree-dump "#include \"header-2.h\"" } } */
/* { dg-final { scan-tree-dump-not "int x;\n#include \"header-2.h\"" } } */
