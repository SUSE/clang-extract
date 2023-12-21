/* { dg-compile "-fdump-ipa-clones -O2 -flto -g3 $test_dir/a.c $test_dir/b.c"} */
/* { dg-options "-graphviz -compute-closure main" }*/


int function();
int function_g();

int main(void)
{
  return function() + function_g();
}

/* { dg-scan-tree-dump  } */

/* { dg-final { scan-tree-dump "common_1" } } */
/* { dg-final { scan-tree-dump "common_2" } } */
