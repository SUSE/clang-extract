/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_EXPORT_SYMBOLS=bbb" }*/
/* { dg-xfail }*/
#define A bbb
#define B bbb

int bbb;

int f()
{
  return A + B;
}
