/* { dg-options "-DCE_EXTRACT_FUNCTIONS=main -DCE_NO_EXTERNALIZATION -DCE_KEEP_INCLUDES" }*/

#include <stdio.h>

int main(void)
{
  printf("Hello world\n");
  return 0;
}

/* { dg-final { scan-tree-dump "#include <stdio.h>" } } */
/* { dg-final { scan-tree-dump-not "#define __clang_major__" } } */
