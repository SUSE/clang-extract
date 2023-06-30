//#define AA

#ifdef AA
int function(void) {
  int x = 3;
#else
int function(int x) {
#endif
  return x;
}
