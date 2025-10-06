/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
enum
{
  CONST0,
};

enum
{
  ANOTHER_CONST = 1 << CONST0,
};

int f()
{
  return ANOTHER_CONST;
}

/* { dg-final { scan-tree-dump "ANOTHER_CONST = 1 << CONST0" } } */
/* { dg-final { scan-tree-dump "return ANOTHER_CONST;" } } */
/* { dg-final { scan-tree-dump "enum\n? *{\n? *CONST0" } } */
