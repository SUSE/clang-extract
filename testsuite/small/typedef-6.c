/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

enum CONSTANT {
  CONST1,
  CONST2,
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

/* { dg-final { scan-tree-dump "enum CONSTANT {\n *CONST1,\n *CONST2,?\n};" } } */
/* { dg-final { scan-tree-dump "enum CONSTANT2 {\n *AAAA,\n *BBBB,?\n};" } } */
/* { dg-final { scan-tree-dump "typedef int \(\*GEN_SESSION_CB\)\(enum CONSTANT \*, enum CONSTANT2 \*\);" } } */
/* { dg-final { scan-tree-dump "struct A {\n *GEN_SESSION_CB a;\n *int y;\n};" } } */
/* { dg-final { scan-tree-dump "int f" } } */
