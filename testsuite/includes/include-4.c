/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION -DCE_KEEP_INCLUDES" }*/

#include "header-4.h"
#define MACRO 2
#include "header-4.h"

int f(void)
{
  // force compilation error if macro != 2
#if MACRO != 2
# error "MACRO with incorrect value."
#endif
  return MACRO;
}

/* { dg-final { scan-tree-dump "#include \"header-4.h\"\n#define MACRO 2\n#include \"header-4.h\"" } } */
/* { dg-final { scan-tree-dump "return MACRO;" } } */
