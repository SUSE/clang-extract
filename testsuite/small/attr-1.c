/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
register unsigned long current_stack_pointer asm("rsp");

unsigned long f()
{
  return current_stack_pointer;
}

/* { dg-final { scan-tree-dump "current_stack_pointer asm" } } */
/* { dg-final { scan-tree-dump "unsigned long f" } } */
