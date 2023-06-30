/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

typedef int (*GEN_SESSION_CB)(int *, unsigned char *, unsigned int *);

struct A {
  GEN_SESSION_CB a;
  int y;
};

int f(struct A *a) {
  return a->y;
}

/* { dg-final { scan-tree-dump "typedef int \(\*GEN_SESSION_CB\)\(int \*, unsigned char \*, unsigned int \*\);" } } */
/* { dg-final { scan-tree-dump "struct A {\n *GEN_SESSION_CB a;\n *int y;\n};" } } */
/* { dg-final { scan-tree-dump "int f" } } */
