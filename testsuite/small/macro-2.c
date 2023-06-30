#define A 0

int f(void) {
  #define C 3
  return C;
  #undef C
}

#define B 1

int g(void) {
  return B;
}
