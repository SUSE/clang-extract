/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_DEBUGINFO_PATH=../testsuite/decompress/test.gz" } */
int f(void)
{
	return 0;
}

int main(void)
{
	return f();
}

/* { dg-final { scan-tree-dump "int f\(void\)" } } */
