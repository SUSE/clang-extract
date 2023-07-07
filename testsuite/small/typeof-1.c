/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
void *malloc(unsigned long);
__typeof(malloc)* malloc_ptr;

void *f(void)
{
  return malloc_ptr;
}

/* { dg-final { scan-tree-dump "__typeof\(malloc\)\* malloc_ptr;" } } */
/* { dg-final { scan-tree-dump "void \*malloc\(unsigned long\);" } } */
/* { dg-final { scan-tree-dump "void \*f\(void\)" } } */
