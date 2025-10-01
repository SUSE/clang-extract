/* { dg-options "-DCE_EXTRACT_FUNCTIONS=fn_c -DCE_NO_EXTERNALIZATION -DCE_KEEP_INCLUDES=system" }*/

#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>
#include "header-1.h"

atomic_int cnt_atomic;

void* fn_c(void *thr_data) {
  (void)thr_data;
  for (int i = 0; i < 40000; i++) {
    atomic_fetch_add(&cnt_atomic, 1);
  }
  return NULL;
}

/* { dg-final { scan-tree-dump "#include <stdio.h>" } } */
/* { dg-final { scan-tree-dump "#include <stdatomic.h>" } } */
/* { dg-final { scan-tree-dump "#include <pthread.h>" } } */
/* { dg-final { scan-tree-dump-not "#include \"header-1.h\"" } } */
