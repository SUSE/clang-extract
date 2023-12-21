/* { dg-skip-silent }*/

static int func_1(void)
{
  return 1;
}

static __attribute__((noinline)) int common(void)
{
  return 42;
}

int function(void)
{
  return common() + func_1();
}
