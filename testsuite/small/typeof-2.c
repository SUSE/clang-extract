/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

struct pcpu_hot
{
  int current_task;
};

extern __typeof(struct pcpu_hot) pcpu_hot;

int f(void)
{
  return pcpu_hot.current_task;
}

/* { dg-final { scan-tree-dump "struct pcpu_hot\n\{" } } */
/* { dg-final { scan-tree-dump "extern __typeof\(struct pcpu_hot\) pcpu_hot;" } } */
