/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

enum {
	TRACE_EVENT_FL_FILTERED_BIT,
	TRACE_EVENT_FL_CAP_ANY_BIT,
	TRACE_EVENT_FL_NO_SET_FILTER_BIT,
	TRACE_EVENT_FL_IGNORE_ENABLE_BIT,
  #define SOME_MACRO (TRACE_EVENT_FL_IGNORE_ENABLE_BIT + 1)
	TRACE_EVENT_FL_TRACEPOINT_BIT,
	TRACE_EVENT_FL_DYNAMIC_BIT,
	TRACE_EVENT_FL_KPROBE_BIT,
	TRACE_EVENT_FL_UPROBE_BIT,
	TRACE_EVENT_FL_EPROBE_BIT,
	TRACE_EVENT_FL_CUSTOM_BIT,
};

struct A {
  int v[SOME_MACRO];
};

int f(struct A *a)
{
  return a->v[0];
}

/* { dg-final { scan-tree-dump "TRACE_EVENT_FL_TRACEPOINT_BIT," } } */
/* { dg-final { scan-tree-dump "#define SOME_MACRO" } } */
/* { dg-final { scan-tree-dump "return a-\>v\[0\];" } } */
