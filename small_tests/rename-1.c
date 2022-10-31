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
  aa.a = x;
  return &aa;
}
