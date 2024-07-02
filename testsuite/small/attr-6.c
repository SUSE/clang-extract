/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
/* { dg-skip-on-archs "s390x" }*/
#ifdef __x86_64__
#define REG "rsp"
#elif __aarch64__
#define REG "sp"
#elif __PPC64__
#define REG "r1"
#endif

register unsigned long current_stack_pointer asm(REG);

unsigned long f()
{
  return current_stack_pointer;
}

/* { dg-final { scan-tree-dump "#define REG \"(rsp|sp|r1)\"" } } */
/* { dg-final { scan-tree-dump "current_stack_pointer asm\(REG\)" } } */
/* { dg-final { scan-tree-dump "unsigned long f" } } */
