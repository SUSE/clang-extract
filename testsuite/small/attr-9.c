/* { dg-options "-DCE_EXTRACT_FUNCTIONS=func_f,g,h -DCE_NO_EXTERNALIZATION" }*/


#define noinline SHOULD_NOT_BE_IN_THE_FINAL_OUTPUT

#undef noinline
#define noinline __attribute__((__noinline__))

/* Do this sort of blurry definition to confuse out clang-extract into dumping
   the AST of the function.  */
#define DEFINE_FUNCTION(name) noinline int func_##name(void) { return 0; }
#define DEFINE_VARIABLE(name) int var_##name;
#define DEFINE(f) DEFINE_VARIABLE(f) DEFINE_FUNCTION(f)

noinline int g()
{
  return 1;
}

DEFINE(f);

noinline int h()
{
  return 2;
}


#undef noinline
#define noinline SHOULD_NOT_BE_IN_THE_FINAL_OUTPUT

/* { dg-final { scan-tree-dump "__attribute__\(\(noinline\)\) int func_f|int func_f\(void\) __attribute__\(\(noinline\)\)" } } */
/* { dg-final { scan-tree-dump "#undef noinline" } } */
/* { dg-final { scan-tree-dump-not "SHOULD_NOT_BE_IN_THE_FINAL_OUTPUT" } } */
