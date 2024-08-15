/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_SYMVERS_PATH=../testsuite/linux/Modules.symvers -DCE_RENAME_SYMBOLS -nostdinc -I../testsuite/linux -D__USE_IBT__ -D__KERNEL__" } */

int bpf_prog_put(void);

int bpf_prog_put(void)
{
	return 42;
}

int f(void)
{
	return bpf_prog_put();
}

/* { dg-final { scan-tree-dump "int bpf_prog_put\(void\);" } } */
/* { dg-final { scan-tree-dump-not "return 42;" } } */
