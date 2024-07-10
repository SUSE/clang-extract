/* { dg-options "-DCE_EXTRACT_FUNCTIONS=main -DCE_KEEP_INCLUDES -DCE_LATE_EXTERNALIZE -DCE_EXPORT_SYMBOLS=printf" }*/

#include <stdio.h>

int main(void)
{
  printf("Hello, world!\n");
  return 0;
}

/* { dg-final { scan-tree-dump "#include <stdio.h>" } } */
/* { dg-final { scan-tree-dump "static int \(\*klpe_printf\)" } } */
