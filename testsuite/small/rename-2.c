static int g()
{
  volatile int x = 3;
  return x;
}

int f(void)
{
  return g();
}
