enum A {
  const1 = 1,
};

#define MACRO const1

struct AA {
  int v[MACRO];
};

int f(struct AA *a)
{
  return a->v[0];
}
