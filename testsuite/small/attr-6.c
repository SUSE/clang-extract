/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
/* { dg-skip-on-archs "s390x" }*/
#ifdef __x86_64__
#define REG "rsp"
#elif __aarch64__
#define REG "sp"
#endif

register unsigned long current_stack_pointer asm(REG);

unsigned long f()
{
  return current_stack_pointer;
}

/* { dg-final { scan-tree-dump "#define REG \"(rsp|sp)\"" } } */
/* { dg-final { scan-tree-dump "current_stack_pointer asm\(REG\)" } } */
/* { dg-final { scan-tree-dump "unsigned long f" } } */
