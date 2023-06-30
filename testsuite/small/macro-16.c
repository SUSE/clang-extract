/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/
#define __ARG_PLACEHOLDER_1 0,
#define __take_second_arg(__ignored, val, ...) val

#define ___or(x, y) ____or(__ARG_PLACEHOLDER_##x, y)
#define ____or(arg1_or_junk, y) __take_second_arg(arg1_or_junk 1, y)

int x;

int f()
{
  return ___or(1,x);
}

/* { dg-final { scan-tree-dump-not "int x;" } } */
/* { dg-final { scan-tree-dump "#define __ARG_PLACEHOLDER_1 0," } } */
/* { dg-final { scan-tree-dump "#define __take_second_arg\(__ignored, val, ...\) val" } } */
/* { dg-final { scan-tree-dump "#define ___or\(x, y\) ____or\(__ARG_PLACEHOLDER_##x, y\)" } } */
/* { dg-final { scan-tree-dump "#define ____or\(arg1_or_junk, y\) __take_second_arg\(arg1_or_junk 1, y\)" } } */
