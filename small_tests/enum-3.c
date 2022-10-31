enum {
  const1 = 1,
};

struct A {
  int v[const1];
};

int f(struct A *a) {
  return a->v[0];
}
