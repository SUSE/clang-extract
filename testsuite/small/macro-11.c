/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
int a_symbol;

#define thismacro a_symbol

#define concat(a, b) a##b
#define x this
#define y macro

int f()
{
  return concat(x, y);
}

/* { dg-error "use of undeclared identifier 'xy'" } */
