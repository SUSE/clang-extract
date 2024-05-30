/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

void f(void);

void sink(void);

void f(void)
{
  sink();
}

extern void f(void);

/* { dg-final { scan-tree-dump "void sink\(void\);" } } */
