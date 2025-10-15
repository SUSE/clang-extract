/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION -include stdlib.h -DCE_KEEP_INCLUDES=system" }*/

void *f()
{
  return NULL;
}

/* { dg-final { scan-tree-dump "return NULL;" } } */
