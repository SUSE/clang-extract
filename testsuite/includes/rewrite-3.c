/* { dg-options "-DCE_EXTRACT_FUNCTIONS=main -DCE_KEEP_INCLUDES -DCE_EXPORT_SYMBOLS=printf" }*/

#include <stdio.h>

int main(void)
{
  printf("Hello, world!\n");
  return 0;
}

/* { dg-final { scan-tree-dump "klp_printf\(\"Hello, world!\\n\"\)" } } */
/* { dg-final { scan-tree-dump-not "#include <stdio.h>" } } */
