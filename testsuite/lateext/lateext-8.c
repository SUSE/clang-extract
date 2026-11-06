/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_LATE_EXTERNALIZE -DCE_EXPORT_SYMBOLS=lck" }*/

/* Same as small/rename-30.c, but with late externalization.  */

typedef struct { int x; } lock_t;
void take(lock_t *);

int f(void)
{
  extern lock_t lck __attribute__((visibility("hidden")));
  take(&lck);
  return 0;
}

/* { dg-final { scan-tree-dump "static lock_t \*klpe_lck __attribute__\(\(used\)\);\n+(/\*\*[^\n]*\n)?int f\(void\)|__attribute__\(\(used\)\) static lock_t \*klpe_lck;\n+(/\*\*[^\n]*\n)?int f\(void\)" } } */
/* { dg-final { scan-tree-dump "take\(&\(\*klpe_lck\)\);" } } */
/* { dg-final { scan-tree-dump-not "extern lock_t lck" } } */
/* { dg-final { scan-tree-dump-not "visibility" } } */
