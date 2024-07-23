/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_SYMVERS_PATH=../testsuite/linux/Modules.symvers -DCE_RENAME_SYMBOLS -nostdinc -I../testsuite/linux -DKBUILD_MODNAME=libcrc32c -D__USE_IBT__ -D__KERNEL__ -DCE_KEEP_INCLUDES" } */

typedef unsigned int u32;

u32 crc32c(u32 crc, const void *address, unsigned int length);

int f(void)
{
	u32 lcrc = 0;
	void *addr = 0;
	unsigned int len = 0;

	(void)crc32c(lcrc, addr, len);
	return 0;
}

/* { dg-final { scan-tree-dump "u32 crc32c\(" } } */
/* { dg-final { scan-tree-dump "KLP_RELOC_SYMBOL\(libcrc32c, libcrc32c, crc32c\)" } } */
/* { dg-final { scan-tree-dump-not "\(\*klpe_crc32c\)" } } */
