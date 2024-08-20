/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

struct iwl_mvm_reorder_buf_entry {
  int frames;
}
#ifndef __CHECKER__
/* sparse doesn't like this construct: "bad integer constant expression" */
__attribute__((aligned(32)))
#endif
;

int f(struct iwl_mvm_reorder_buf_entry x)
{
  return x.frames;
}

/* { dg-final { scan-tree-dump "__attribute__\(\(aligned\(32\)\)\)" } } */
