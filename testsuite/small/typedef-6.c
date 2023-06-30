enum CONSTANT {
  CONST1,
  COSST2,
};

enum CONSTANT2 {
  AAAA,
  BBBB,
};

typedef int (*GEN_SESSION_CB)(enum CONSTANT *, enum CONSTANT2 *);

struct A {
  GEN_SESSION_CB a;
  int y;
};

int f(struct A *a) {
  return a->y;
}
