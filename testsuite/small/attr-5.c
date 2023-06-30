/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
struct S { short f[3]; } __attribute__ ((aligned (8)));

short f(struct S* s)
{
  return s->f[0];
}

/* { dg-final { scan-tree-dump "struct S { short f\[3\]; } __attribute__ \(\(aligned \(8\)\)\);" } } */
