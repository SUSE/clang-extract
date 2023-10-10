/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

struct A {
  int a;
};

struct B : public A {
  int b;
};

int f(void)
{
  B b;
  return b.a + b.b;
}

/* { dg-final { scan-tree-dump "struct A \{" } } */
/* { dg-final { scan-tree-dump "struct B : public A \{" } } */
/* { dg-final { scan-tree-dump "b\.a \+ b\.b" } } */
