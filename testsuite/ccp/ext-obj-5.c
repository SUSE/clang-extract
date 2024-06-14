/* { dg-options "-DCE_EXTRACT_FUNCTIONS=pu_f -DCE_EXPORT_SYMBOLS=ee_o" }*/

#include <assert.h>

static char ee_o[] = "123";

void pu_f(void)
{
        char s[sizeof(ee_o)], t[sizeof(ee_o)][sizeof(ee_o)];
	static_assert(sizeof(s) == 4);
	static_assert(sizeof(t) == 16);
        ee_o;
}

/* { dg-final { scan-tree-dump "static char \(\*klpe_ee_o\)\[4\];" } } */
/* { dg-final { scan-tree-dump "char s\[sizeof\(\(\*klpe_ee_o\)\)\], t\[sizeof\(\(\*klpe_ee_o\)\)\]\[sizeof\(\(\*klpe_ee_o\)\)\];" } } */
