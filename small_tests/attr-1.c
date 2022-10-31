register unsigned long current_stack_pointer asm("rsp");

unsigned long f()
{
  return current_stack_pointer;
}
