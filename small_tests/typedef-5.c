typedef int (*GEN_SESSION_CB)(int *, unsigned char *, unsigned int *);

struct A {
  GEN_SESSION_CB a;
  int y;
};

int f(struct A *a) {
  return a->y;
}
