/* { dg-skip-silent }*/

static int a_function(void)
{
  return 1;
}

static __attribute__((noinline)) int common(void)
{
  return 1337;
}

int function_g(void)
{
  return common() + a_function();
}
