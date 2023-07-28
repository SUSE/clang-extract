/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

typedef unsigned long pteval_t;
typedef struct { pteval_t pte; } pte_t;

pte_t (*fptr)(int x);

void *f(void)
{
  return fptr;
}

/* { dg-final { scan-tree-dump "typedef unsigned long pteval_t;" } } */
/* { dg-final { scan-tree-dump "typedef struct { pteval_t pte; } pte_t;" } } */
