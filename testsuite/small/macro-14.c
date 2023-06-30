/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
int x;

#define erp x
#define __ASM_SEL_RAW(x) x
#define __ASM_REG(reg) __ASM_SEL_RAW(e##reg)

int f()
{
  return __ASM_REG(rp);
}

/* { dg-final { scan-tree-dump "int x;" } } */
/* { dg-final { scan-tree-dump "#define erp x" } } */
/* { dg-final { scan-tree-dump "#define __ASM_SEL_RAW\(x\) x" } } */
/* { dg-final { scan-tree-dump "#define __ASM_REG\(reg\) __ASM_SEL_RAW\(e##reg\)" } } */
/* { dg-final { scan-tree-dump "return __ASM_REG\(rp\);" } } */
