int f(int x)
{
  return x;
}

struct AAA
{
  int a;
};

struct AAA* g(int x)
{
  static struct AAA aa;
  AAA.a = x;
  return &aa;
}
