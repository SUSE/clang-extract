/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
/* { dg-xfail } */

/* Test fails since LLVM-22 support.  C++ support were not working at this
   stage, so its a regression.  */

/* Unused function should be removed.  */

int printf(const char *, ...);

namespace n {
  int unused_function(void)
  {
    return 0;
  }

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
/* { dg-final { scan-tree-dump-not "unused_function" } } */
