/* { dg-skip-silent }*/

static int a_function(void)
{
  return 1;
}

static int common_2(void)
{
  return 1337;
}

int function_g(void)
{
  return common_2() + a_function();
}
