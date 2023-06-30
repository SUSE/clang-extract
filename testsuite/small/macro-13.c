int a_symbol;

#define this_macro a_symbol
#define concat(a, b) this_##b

#define macro concat(this, macro)

int f()
{
  return macro;
}
