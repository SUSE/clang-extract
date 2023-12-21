/* { dg-compile "-fdump-ipa-clones -O2 -flto -g3 $test_dir/a.c $test_dir/b.c"} */
/* { dg-options "-graphviz -compute-closure main" }*/

// Currently we do not distinguish between lto_priv symbols, which results in
// different symbols being merged together.
/* { dg-xfail }*/

int function();
int function_g();

int main(void)
{
  return function() + function_g();
}

/* { dg-scan-tree-dump  } */

/* { dg-final { scan-tree-dump "common.lto_priv.0" } } */
/* { dg-final { scan-tree-dump "common.lto_priv.1" } } */
