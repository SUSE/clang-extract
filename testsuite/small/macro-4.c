#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define ONE 1
#define X MAX(ONE, 0)
#define u 8

int f(void)
{
  return X;
}

#define X 2

int g(void)
{
  return X;
}
