/* { dg-options "-DCE_EXTRACT_FUNCTIONS=main -DCE_NO_EXTERNALIZATION" }*/
/* { dg-xfail }*/

// C++ default headers are currently not working.

#include <iostream>

int main(void)
{
  std::cout << "Hello world!" << std::endl;
}

/* { dg-final { scan-tree-dump "std::cout" } } */
