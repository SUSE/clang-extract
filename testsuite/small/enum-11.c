/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

enum consts {
  CST0 = 0,
  CST1,
#include "enums.h"
};

enum consts f(void)
{
  return CST2;
}

/* { dg-final { scan-tree-dump "CST2," } } */
