/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION -DCE_IGNORE_CLANG_ERRORS" }*/

void f(void)
{
  return 3;
}

/* { dg-final { scan-tree-dump "void f" } } */
/* { dg-final { scan-tree-dump "return 3;" } } */
