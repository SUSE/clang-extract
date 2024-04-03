/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

#define SMP_CACHE_BYTES 4
#define ____cacheline_aligned __attribute__((__aligned__(SMP_CACHE_BYTES)))

struct sched_avg {
  int aa;
} ____cacheline_aligned;

struct sched_avg sa;

int f(void)
{
  return sa.aa;
}

/* { dg-final { scan-tree-dump "struct sched_avg {\n *int aa;\n} *____cacheline_aligned;" } } */
