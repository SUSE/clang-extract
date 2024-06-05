/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_RENAME_SYMBOLS -I ../testsuite/small/ -DCE_SYMVERS_PATH=../testsuite/small/Modules.symvers -DKBUILD_MODNAME=crc32c -D__USE_IBT__ -D__KERNEL__" } */

typedef unsigned int u32;

u32 crc32c(u32 crc, const void *address, unsigned int length);

#include "linux/livepatch.h"

int f(void)
{
	u32 lcrc = 0;
	void *addr = 0;
	unsigned int len = 0;

	(void)crc32c(lcrc, addr, len);
	return 0;
}

/* { dg-final { scan-tree-dump "u32 klpe_crc32c" } } */
/* { dg-final { scan-tree-dump "KLP_RELOC_SYMBOL\(crc32c, crc32c, crc32c\)" } } */
/* { dg-final { scan-tree-dump-not "\(\*klpe_crc32c\)" } } */
