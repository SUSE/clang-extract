/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
enum
{
  CONST0,
  ANOTHER_CONST = CONST0,
};

int f()
{
  return ANOTHER_CONST;
}

/* { dg-final { scan-tree-dump "CONST0,\n *ANOTHER_CONST = CONST0" } } */
/* { dg-final { scan-tree-dump "return ANOTHER_CONST;" } } */
