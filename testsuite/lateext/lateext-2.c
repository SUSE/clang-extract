/* { dg-options "-DCE_EXTRACT_FUNCTIONS=h -DCE_LATE_EXTERNALIZE -DCE_EXPORT_SYMBOLS=f -DCE_KEEP_INCLUDES" }*/

#define MACRO
#include "lateext-2.h"
#undef MACRO
#include "lateext-2.h"
int h(void)
{
  return f() + g();
}

/* { dg-final { scan-tree-dump "#define MACRO\n#include \"lateext-2.h\"\n#undef MACRO\n#include \"lateext-2.h\"" } } */
/* { dg-final { scan-tree-dump "static int \(\*klpe_f\)\(void\) __attribute__\(\(used\)\);" } } */
/* { dg-final { scan-tree-dump "return \(\*klpe_f\)\(\) \+ g\(\);" } } */
