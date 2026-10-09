/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_IPACLONES_PATH=$test_dir/ill_formed.c.000i.ipa-clones" }*/

int f(void)
{
  return 0;
}

/* { dg-error "expected number" }*/
