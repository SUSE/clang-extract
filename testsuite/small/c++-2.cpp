/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

int printf(const char *, ...);

namespace n {
  int print(const char *str)
  {
    return printf(str);
  }
}

void f(void)
{
  n::print("Hello\n");
}

/* { dg-final { scan-tree-dump "return printf" } } */
