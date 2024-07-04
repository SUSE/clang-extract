/* { dg-options "-DCE_EXTRACT_FUNCTIONS=func_f -DCE_NO_EXTERNALIZATION" }*/
#define MACRO 0

/* Do this sort of blurry definition to confuse out clang-extract into dumping
   the AST of the function.  */
#define DEFINE_FUNCTION(name) __attribute__((noinline)) int func_##name(void) { return MACRO; }
#define DEFINE_VARIABLE(name) int var_##name;
#define DEFINE(f) DEFINE_VARIABLE(f) DEFINE_FUNCTION(f)

DEFINE(f)

/* { dg-final { scan-tree-dump "__attribute__\(\(noinline\)\)" } } */
/* { dg-final { scan-tree-dump-not "#define noinline" } } */
