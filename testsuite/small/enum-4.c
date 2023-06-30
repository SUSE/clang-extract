typedef enum {
  const1 = 1,
  const2 = 2,
} AA;

struct A {
  int v[const1];
};

int f(struct A *a) {
  return a->v[0];
}
