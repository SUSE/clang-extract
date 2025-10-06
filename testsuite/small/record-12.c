/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

struct record {
  int field1;
#   include    "fields.h"
  int field4;
};

int f(struct record *r)
{
  return r->field2;
}

/* { dg-final { scan-tree-dump "struct record {" } } */
/* { dg-final { scan-tree-dump "int field2;" } } */
