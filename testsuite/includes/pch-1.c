/* { dg-options "-DCE_EXTRACT_FUNCTIONS=main -DCE_NO_EXTERNALIZATION" }*/

#include "pch-1.h"

DEFINE_FUNCTION;

int main()
{
  function_0();
  return 0;
}

/* { dg-final { scan-tree-dump "function_0" } } */
