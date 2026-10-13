/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

/* Check that a struct declared together with a variable in a macro is not
   lost.  The variable is printed from the AST, which does not include the
   struct definition.  */

#define VAR(n) struct n { int a; } n##_v;
VAR(s9)

int f(struct s9 *p)
{
  return s9_v.a + p->a;
}

/* { dg-final { scan-tree-dump "struct s9 {\n *int a;\n};" } } */
/* { dg-final { scan-tree-dump "struct s9 s9_v;" } } */
