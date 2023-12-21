/* { dg-skip-silent }*/

static int func_1(void)
{
  return 1;
}

static int common_1(void)
{
  return 42;
}

int function(void)
{
  return common_1() + func_1();
}
