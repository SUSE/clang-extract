/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

typedef unsigned long size_t;
typedef signed long ssize_t;
typedef int unused_t;

size_t f(ssize_t s)
{
  return (unsigned long) s;
}

/* { dg-final { scan-tree-dump "typedef unsigned long size_t;" } */
/* { dg-final { scan-tree-dump "typedef signed long ssize_t;" } */
/* { dg-final { scan-tree-dump-not "typedef int unused_t;" } */
