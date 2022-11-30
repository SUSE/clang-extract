int x;

#define erp x
#define __ASM_SEL_RAW(y) y
#define __ASM_REG(reg) __ASM_SEL_RAW(e##reg)

int f()
{
  return __ASM_REG(rp);
}
