/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION -DCE_KEEP_INCLUDES -DCE_EXPAND_INCLUDES=header-5.h" }*/
/* { dg-xfail } */

/* Test currently fails because when expanding a include you must add all Decls
   and macros of that file, but not necessarly those comming from files it
   includes.  The correct way of doing this would be addding an extra pass to
   clang-extract.  */

#include "header-5.h"

int f(void)
{
  return used_function();
}


/* { dg-final { scan-tree-dump "#include \"header-1.h\"" } } */
/* { dg-final { scan-tree-dump "return used_function\(\);" } } */
