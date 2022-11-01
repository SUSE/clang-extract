struct S { short f[3]; } __attribute__ ((aligned (8)));

short f(struct S* s)
{
  return s->f[0];
}
