/* { dg-options "-DCE_EXTRACT_FUNCTIONS=f -DCE_NO_EXTERNALIZATION" }*/

#define SHARED

#define IS_IN(x) 0

#ifndef SHARED
# define GLRO(name) _##name
#else
# if IS_IN (rtld)
#  define GLRO(name) _rtld_local_ro._##name
# else
#  define GLRO(name) _rtld_global_ro._##name
# endif
struct rtld_global_ro
{
#endif
  int field1;
#ifdef SHARED
  int field2;
};
#endif


int f(struct rtld_global_ro *r)
{
  return r->field1;
}

/* { dg-final { scan-tree-dump "struct rtld_global_ro ?\n?{" } } */
