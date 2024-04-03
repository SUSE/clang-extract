/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_EXPORT_SYMBOLS=struct_variable -DCE_RENAME_SYMBOLS" }*/

struct AA {
  int aa;
};

struct AA struct_variable;

#define MACRO(x, y) ((x) + (y))

int f(void)
{
  return MACRO(struct_variable.aa, 1);
}

/* { dg-final { scan-tree-dump "MACRO\(\(\*klpe_struct_variable\)\.aa, 1\);" } } */
