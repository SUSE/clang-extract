/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
/* { dg-xfail } */

#include <string>

std::string f(void)
{
  return "x";
}

/* { dg-final { scan-tree-dump "return \"x\";" } } */
