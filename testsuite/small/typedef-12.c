/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

struct AA {
  int a;
};

typedef struct AA _A;
typedef struct AA _B;

typedef _A A;
typedef _B A;

int f() {
  A b;
  return b.a;
}


/* { dg-final { scan-tree-dump "struct AA {" } } */
/* { dg-final { scan-tree-dump "typedef struct AA _B;" } } */
/* { dg-final { scan-tree-dump "typedef _B A;" } } */

/* { dg-final { scan-tree-dump-not "typedef _A A;" } } */
/* { dg-final { scan-tree-dump-not "typedef struct AA _A;" } } */
