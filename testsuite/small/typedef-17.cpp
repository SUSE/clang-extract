/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

/* Check that a struct defined in a typedef inside of a namespace is not
   output twice, which results in a redefinition error.  */

namespace n {
  typedef struct p { int a; } P;
}

int f(n::P *x, struct n::p *y)
{
  return x->a + y->a;
}

/* { dg-final { scan-tree-dump "typedef struct p { int a; } P;" } } */
/* { dg-final { scan-tree-dump-not "struct p { int a; };" } } */
