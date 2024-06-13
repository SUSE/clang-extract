/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f" } */

typedef struct {
	int id;
	int val;
} st1, st2;

int f(void)
{
	st1 s1;
	st2 s2;

	s1.id = 10;
	s2.val = 5;
}
/* { dg-final { scan-tree-dump "int f\(void\)" } } */
/* { dg-final { scan-tree-dump "} st1, st2;" } } */
/* { dg-final { scan-tree-dump-not "} st1;" } } */
/* { dg-final { scan-tree-dump-not "} st2;" } } */
