/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_LATE_EXTERNALIZE -DCE_EXPORT_SYMBOLS=aaa -DCE_KEEP_INCLUDES" }*/

#include "lateext-7.h"

DEFINE_STATIC_KEY_FALSE(aaa);

int f(void)
{
  return aaa.key;
}

/* { dg-final { scan-tree-dump-not "DEFINE_STATIC_KEY_FALSE\(\*klpe_aaa\)" } } */
/* { dg-final { scan-tree-dump "static struct AA \*klpe_aaa __attribute__\(\(used\)\);|__attribute__\(\(used\)\) static struct AA \*klpe_aaa;" } } */
