/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

#define __initdata
typedef int initcall_entry_t;

extern initcall_entry_t __initcall0_start[];
extern initcall_entry_t __initcall1_start[];
extern initcall_entry_t __initcall2_start[];
extern initcall_entry_t __initcall3_start[];
extern initcall_entry_t __initcall4_start[];
extern initcall_entry_t __initcall5_start[];
extern initcall_entry_t __initcall6_start[];
extern initcall_entry_t __initcall7_start[];
extern initcall_entry_t __initcall_end[];

static initcall_entry_t *initcall_levels[] __initdata = {
  __initcall0_start,
  __initcall1_start,
  __initcall2_start,
  __initcall3_start,
  __initcall4_start,
  __initcall5_start,
  __initcall6_start,
  __initcall7_start,
  __initcall_end,
};

initcall_entry_t *f(int x)
{
  return initcall_levels[x];
}

/* { dg-final { scan-tree-dump "extern initcall_entry_t __initcall0_start" } } */
/* { dg-final { scan-tree-dump "extern initcall_entry_t __initcall1_start" } } */
/* { dg-final { scan-tree-dump "extern initcall_entry_t __initcall2_start" } } */
/* { dg-final { scan-tree-dump "extern initcall_entry_t __initcall3_start" } } */
/* { dg-final { scan-tree-dump "extern initcall_entry_t __initcall4_start" } } */
/* { dg-final { scan-tree-dump "extern initcall_entry_t __initcall5_start" } } */
/* { dg-final { scan-tree-dump "extern initcall_entry_t __initcall6_start" } } */
/* { dg-final { scan-tree-dump "extern initcall_entry_t __initcall7_start" } } */
/* { dg-final { scan-tree-dump "extern initcall_entry_t __initcall_end" } } */
