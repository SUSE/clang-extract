/* { dg-options "-DCE_EXTRACT_FUNCTIONS=pu_f -DCE_NO_EXTERNALIZATION" }*/
/* { dg-xfail }*/

/* The pu_f symbol location should be printed right before the symbol definition */

void pu_f(void)
{
}

/* { dg-final { scan-tree-dump "clang-extract: from testsuite/ccp/simple.c" } } */
