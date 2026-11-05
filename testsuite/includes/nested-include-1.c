/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

/* The #include "nested-include-1.def" is written inside the struct body and
   is relative to the other/ directory.  It must not be copied verbatim into
   the output.  */
#include "other/nested-include-1.h"

int f(void)
{
  return sizeof(names.a) + sizeof(names.b);
}

/* { dg-final { scan-tree-dump-not "#include \"nested-include-1.def\"" } } */
/* { dg-final { scan-tree-dump "char a;" } } */
/* { dg-final { scan-tree-dump "char b;" } } */
/* { dg-final { scan-tree-dump "} names;" } } */
