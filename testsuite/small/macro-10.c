int a_symbol;

#define thismacro a_symbol

#define concat(a, b) a##b

int f()
{
  return concat(this, macro);
}
