#define REG "rsp"

register unsigned long current_stack_pointer asm(REG);

unsigned long f()
{
  return current_stack_pointer;
}
