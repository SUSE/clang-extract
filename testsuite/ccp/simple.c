/* { dg-options "-DCE_EXTRACT_FUNCTIONS=pu_f -DCE_NO_EXTERNALIZATION" }*/

/* The pu_f symbol location should be printed right before the symbol definition */

void pu_f(void)
{
}

/* { dg-final { scan-tree-dump "clang-extract: from .*simple.c" } } */
