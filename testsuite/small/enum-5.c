/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
extern enum system_states {
	SYSTEM_BOOTING,
	SYSTEM_SCHEDULING,
	SYSTEM_FREEING_INITMEM,
	SYSTEM_RUNNING,
	SYSTEM_HALT,
	SYSTEM_POWER_OFF,
	SYSTEM_RESTART,
	SYSTEM_SUSPEND,
} system_state;

int f()
{
  return (int) SYSTEM_SUSPEND + system_state;
}

/* { dg-final { scan-tree-dump "enum system_states" } } */
/* { dg-final { scan-tree-dump "SYSTEM_SUSPEND," } } */
/* { dg-final { scan-tree-dump "return \(int\) SYSTEM_SUSPEND \+ system_state;" } } */
