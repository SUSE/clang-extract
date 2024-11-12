/* { dg-options "-DCE_EXTRACT_FUNCTIONS=main -DCE_NO_EXTERNALIZATION" }*/

#include <bits/char_traits.h>

int main(void)
{
  struct std::char_traits<wchar_t> x;
  return 0;
}

/* { dg-final { scan-tree-dump "struct std::char_traits<wchar_t>" } } */
