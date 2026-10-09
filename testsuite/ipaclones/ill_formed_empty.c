/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_IPACLONES_PATH=$test_dir/ill_formed_empty.c.000i.ipa-clones" }*/

/* An empty ipa-clones file must not be considered an error.  */

int f(void)
{
  return 0;
}

/* { dg-final { scan-tree-dump "int f\(void\)" } } */
