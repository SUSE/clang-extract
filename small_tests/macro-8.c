#define __inline__ inline

static __inline__ int func(int x)
{
  return x + 3;
}

int f(int x)
{
  return func(x);
}
