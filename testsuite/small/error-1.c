/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
int f()
{
  return a;
}

/* { dg-error "use of undeclared identifier 'a'" }*/
