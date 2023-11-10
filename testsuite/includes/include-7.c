/* { dg-options "-DCE_EXTRACT_FUNCTIONS=main -DCE_NO_EXTERNALIZATION -DCE_KEEP_INCLUDES -DCE_EXPAND_INCLUDES=stddef.h" }*/

#include <stdio.h>

int main(void)
{
  printf("Hello world\n");
  return 0;
}

/* { dg-final { scan-tree-dump "#define _STDIO_H	1" } } */
/* { dg-final { scan-tree-dump "typedef __SIZE_TYPE__ size_t;" } } */
/* { dg-final { scan-tree-dump "#define __need___va_list" } } */
/* { dg-final { scan-tree-dump "#include <stdarg.h>" } } */
/* { dg-final { scan-tree-dump "#include <bits/types/__fpos_t.h>" } } */
/* { dg-final { scan-tree-dump "#include <bits/types/__fpos64_t.h>" } } */
/* { dg-final { scan-tree-dump "#include <bits/types/__FILE.h>" } } */
/* { dg-final { scan-tree-dump "#include <bits/types/FILE.h>" } } */
/* { dg-final { scan-tree-dump "#include <bits/types/struct_FILE.h>" } } */
/* { dg-final { scan-tree-dump "# include <bits/types/cookie_io_functions_t.h>" } } */
/* { dg-final { scan-tree-dump "#include <bits/stdio_lim.h>" } } */
/* { dg-final { scan-tree-dump "extern int printf \(const char \*__restrict __format, ...\);" } } */
/* { dg-final { scan-tree-dump "#include <bits/floatn.h>" } } */
/* { dg-final { scan-tree-dump "int main\(void\)" } } */
