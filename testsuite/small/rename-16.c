/* { dg-options "-DCE_EXTRACT_FUNCTIONS=main,function -DCE_RENAME_SYMBOLS" }*/

struct AA {
  void *fun;
};

int function(void)
{
  return 0;
}

struct AA A = {
  .fun = function,
};

int main(void)
{
  return (int) ((unsigned long)A.fun & 0xFFFFFFFF);
}

/* { dg-final { scan-tree-dump "int klpp_function\(void\)" } } */
/* { dg-final { scan-tree-dump "\.fun = klpp_function," } } */
