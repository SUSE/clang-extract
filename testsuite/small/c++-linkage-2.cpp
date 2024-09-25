/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

extern "C" void *gVariable;

void *f(void)
{
  return gVariable;
}

/* { dg-final { scan-tree-dump "extern \"C\" void \*gVariable;" } } */
