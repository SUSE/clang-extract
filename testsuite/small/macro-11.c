int a_symbol;

#define thismacro a_symbol

#define concat(a, b) a##b
#define x this
#define y macro

int f()
{
  return concat(x, y);
}
