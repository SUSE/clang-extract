/* { dg-options "-DCE_EXTRACT_FUNCTIONS=main -DCE_NO_EXTERNALIZATION  -DCE_KEEP_INCLUDES=everything -DCE_EXPAND_INCLUDES=stdio.h -DCE_NOT_EXPAND_INCLUDES=bits/wordsize.h" }*/

#include <stdio.h>

int main(void)
{
  printf("Hello world\n");
  return 0;
}

/* { dg-final { scan-tree-dump-not "#include <stdio.h>" } } */
/* { dg-final { scan-tree-dump "#include <bits/wordsize.h>" } } */
/* { dg-final { scan-tree-dump "int main\(void\)" } } */
