int x;
int yy;

#define yy x

int f(void) {
  return yy;
}

#undef yy

int g(void) {
  return f() + yy;
}
