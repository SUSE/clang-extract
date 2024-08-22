/* { dg-options "-DCE_EXTRACT_FUNCTIONS=g -DCE_EXPORT_SYMBOLS=f" }*/
#include "rename-5.h"

/* { dg-final { scan-tree-dump "static int \(\*klpe_f\)\(\) __attribute__\(\(used\)\);" } } */
/* { dg-final { scan-tree-dump "return \(\*klpe_f\)\(\);" } } */
