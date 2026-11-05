/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

/* The #include "nested-include-2.def" directives are written inside the body
   of a struct declared together with a variable, and of an anonymous struct
   in a typedef.  They are relative to the other/ directory, so they must not
   be copied verbatim into the output.  */
#include "other/nested-include-2.h"

int f(void)
{
  anon_t t;
  t.a = 0;
  return sizeof(_nl_category_names) + t.a;
}

/* { dg-final { scan-tree-dump-not "#include \"nested-include-2.def\"" } } */
/* { dg-final { scan-tree-dump "char str1\[3\];" } } */
/* { dg-final { scan-tree-dump "char str2\[4\];" } } */
/* { dg-final { scan-tree-dump "} _nl_category_names __attribute__\(\(visibility\(\"hidden\"\)\)\);" } } */
/* { dg-final { scan-tree-dump "int a;" } } */
/* { dg-final { scan-tree-dump "int b;" } } */
/* { dg-final { scan-tree-dump "} anon_t;" } } */
