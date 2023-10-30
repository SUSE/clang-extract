/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

template<bool b> int function(void)
{
  if (b == true) {
    return 1;
  } else {
    return 0;
  }
}

int (*fptr)(void) = function<true>;

int f(void)
{
  return fptr();
}

/* { dg-final { scan-tree-dump "return fptr\(\);" } } */
/* { dg-final { scan-tree-dump "template<bool b>" } } */
/* { dg-final { scan-tree-dump "function<true>" } } */
