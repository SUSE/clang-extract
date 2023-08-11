/* { dg-options "-DCE_EXTRACT_FUNCTIONS=h -DCE_NO_EXTERNALIZATION -DCE_KEEP_INCLUDES" }*/

#define MACRO
#include "header-3.h"
#undef MACRO
#include "header-3.h"
int h(void)
{
  return f() + g();
}

/* { dg-final { scan-tree-dump "#define MACRO\n#include \"header-3.h\"\n#undef MACRO\n#include \"header-3.h\"" } } */
/* { dg-final { scan-tree-dump "return f\(\) \+ g\(\);" } } */
