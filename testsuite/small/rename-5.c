/* { dg-options "-DCE_EXTRACT_FUNCTIONS=g" }*/
#include "rename-5.h"

/* { dg-final { scan-tree-dump "static int \(\*klp_f\)\(\);" } } */
/* { dg-final { scan-tree-dump "return klp_f\(\);" } } */
